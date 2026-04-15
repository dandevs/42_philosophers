#!/usr/bin/env python3
"""
Rich TUI test runner for 42_philosophers project.

Uses make for incremental builds — only recompiles changed files.
Tests run in parallel with a live animated display.

Usage:
    python3 run_tests.py              # incremental build + run all tests
    python3 run_tests.py --clean      # full rebuild from scratch
    python3 run_tests.py table_create  # run specific suite
    python3 run_tests.py --max-parallel 5    # parallel test execution (default: 1)
    python3 run_tests.py --disable-ccache    # skip ccache
"""

import fcntl
import glob
import hashlib
import json
import os
import shutil
import subprocess
import sys
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
from dataclasses import dataclass


def _prompt_install_rich():
    """Interactive prompt to install rich. Returns True if installed."""
    BOLD = "\033[1m"
    DIM = "\033[2m"
    RED = "\033[31m"
    GREEN = "\033[32m"
    RESET = "\033[0m"

    print()
    print(f"  {BOLD}rich{RESET} is required for the test runner but is not installed.")
    print()
    print(f"  How would you like to install it?")
    print(f"    {BOLD}1){RESET} pip install rich")
    print(f"    {DIM}2){RESET} pip3 install rich")
    print(f"    {DIM}3){RESET} uv pip install rich")
    print(f"    {RED}4){RESET} Abort")
    print()

    try:
        choice = input(f"  Enter choice [{BOLD}1{RESET}] (default: pip): ").strip()
    except EOFError:
        print(f"  {RED}No interactive input available.{RESET}")
        return False

    commands = {
        "1": [sys.executable, "-m", "pip", "install", "rich"],
        "2": ["pip3", "install", "rich"],
        "3": ["uv", "pip", "install", "rich"],
        "4": None,
    }

    if choice == "" or choice == "1":
        cmd = commands["1"]
    elif choice in commands:
        cmd = commands[choice]
    else:
        print(f"  {RED}Invalid choice: {choice}{RESET}")
        return False

    if cmd is None:
        print(f"  Aborted.")
        return False

    print()
    try:
        subprocess.check_call(cmd)
        print(f"\n  {GREEN}✓ rich installed successfully{RESET}")
        return True
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        print(f"\n  {RED}✗ Installation failed: {e}{RESET}")
        return False


def _try_import_rich():
    """Try to import rich. Returns the modules or None."""
    try:
        from rich.console import Console, Group
        from rich.live import Live
        from rich.panel import Panel
        from rich.text import Text
        from rich.tree import Tree

        return Console, Group, Live, Panel, Text, Tree
    except ImportError:
        return None


_rich = _try_import_rich()
if _rich is None:
    if not _prompt_install_rich():
        print()
        print("  Cannot run tests without rich. Please install it manually:")
        print("    pip install rich")
        sys.exit(1)
    _rich = _try_import_rich()
    if _rich is None:
        print()
        print("  Installation succeeded but rich still cannot be imported.")
        print("  Try restarting the test runner.")
        sys.exit(1)

Console, Group, Live, Panel, Text, Tree = _rich

SPINNER_FRAMES = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"]

PROJECT_ROOT = os.path.dirname(os.path.abspath(__file__))
SRC_DIR = os.path.join(PROJECT_ROOT, "src")
TESTS_DIR = os.path.join(PROJECT_ROOT, "tests")
BUILD_DIR = os.path.join(PROJECT_ROOT, "test_build")


def _normalize_suite_id(value):
    return value.replace("\\", "/").strip("/")


def _sanitize_target_fragment(value):
    safe = []
    for char in value:
        if char.isalnum() or char in ("_", "-"):
            safe.append(char)
        else:
            safe.append("_")
    return "".join(safe) or "test"


def _target_name_for_suite_test(suite, test_name):
    suite_fragment = _sanitize_target_fragment(suite.replace("/", "__"))
    test_fragment = _sanitize_target_fragment(test_name)
    digest = hashlib.sha1(f"{suite}/{test_name}".encode("utf-8")).hexdigest()[:10]
    return f"{suite_fragment}__{test_fragment}__{digest}"


def _match_requested_suites(all_suites, requested):
    matched = []
    seen = set()
    unknown = []

    for raw in requested:
        normalized = _normalize_suite_id(raw)
        if not normalized:
            unknown.append(raw)
            continue
        if normalized in all_suites:
            candidates = [normalized]
        else:
            prefix = normalized + "/"
            candidates = [suite for suite in all_suites if suite.startswith(prefix)]
        if not candidates:
            unknown.append(raw)
            continue
        for suite in candidates:
            if suite in seen:
                continue
            matched.append(suite)
            seen.add(suite)

    return matched, unknown


def get_project_sources():
    sources = []
    for path in glob.glob(os.path.join(SRC_DIR, "**", "*.c"), recursive=True):
        if os.path.basename(path) != "main.c":
            sources.append(path)
    return sources


def discover_suites():
    suites = []
    if not os.path.isdir(TESTS_DIR):
        return suites

    for root, _, files in os.walk(TESTS_DIR):
        has_tests = any(name.endswith(".c") for name in files)
        if not has_tests:
            continue
        rel_path = os.path.relpath(root, TESTS_DIR)
        if rel_path == ".":
            continue
        normalized = _normalize_suite_id(rel_path)
        if normalized:
            suites.append(normalized)

    return sorted(suites)


def discover_tests(suite):
    suite_dir = os.path.join(TESTS_DIR, *suite.split("/"))
    return sorted(glob.glob(os.path.join(suite_dir, "*.c")))


def _generate_makefile(build_dir, src_dir, project_sources, compile_jobs, use_ccache):
    """Generate a Makefile in build_dir. Returns path to Makefile."""
    rel_src = os.path.relpath(src_dir, build_dir)

    objs = []
    for src in project_sources:
        rel = os.path.relpath(src, src_dir)
        obj = "obj/" + os.path.splitext(rel)[0] + ".o"
        objs.append(obj)

    cc_var = "CC = ccache cc" if use_ccache else "CC ?= cc"
    test_names = []
    test_targets = []
    for suite, test_name, test_file, binary in compile_jobs:
        target = os.path.basename(binary)
        test_names.append(target)
        test_targets.append((target, test_file))

    lines = []
    lines.append(f"{cc_var}")
    lines.append(f"CFLAGS = -O0 -fdiagnostics-color=always -MMD -MP -I{rel_src}")
    lines.append(f"SRC_DIR = {rel_src}")
    lines.append("")
    lines.append(f"OBJS = {' '.join(objs)}")
    lines.append("LIB = libproject.a")
    lines.append("")
    lines.append(f"TESTS = {' '.join(test_names)}")
    lines.append("")
    lines.append(".PHONY: all clean")
    lines.append("")
    lines.append("all: $(LIB) $(TESTS)")
    lines.append("")
    lines.append("$(LIB): $(OBJS)")
    lines.append("\tar rcs $@ $^")
    lines.append("")
    lines.append("obj/%.o: $(SRC_DIR)/%.c")
    lines.append("\t@mkdir -p $(dir $@)")
    lines.append("\t$(CC) $(CFLAGS) -c $< -o $@")
    lines.append("")

    for target, test_file in test_targets:
        rel_test = os.path.relpath(test_file, build_dir)
        lines.append(f"{target}: {rel_test} $(LIB)")
        lines.append(f"\t$(CC) $(CFLAGS) $< $(LIB) -o $@")
        lines.append("")

    lines.append("clean:")
    lines.append("\trm -rf obj $(LIB) $(TESTS)")
    lines.append("")
    lines.append("-include $(OBJS:.o=.d)")

    makefile_path = os.path.join(build_dir, "Makefile")
    with open(makefile_path, "w") as f:
        f.write("\n".join(lines))
    return makefile_path


def _makefile_needs_regen(build_dir, compile_jobs, project_sources, use_ccache):
    """Check if Makefile needs regeneration by comparing manifest."""
    manifest_path = os.path.join(build_dir, ".manifest")
    jobs_manifest = "\n".join(
        f"{suite}:{test_file}:{os.path.basename(binary)}"
        for suite, test_name, test_file, binary in compile_jobs
    )
    sources_manifest = "\n".join(
        sorted(os.path.relpath(path, build_dir) for path in project_sources)
    )
    current = (
        f"use_ccache={1 if use_ccache else 0}\n"
        f"sources:\n{sources_manifest}\n"
        f"jobs:\n{jobs_manifest}"
    )
    if os.path.exists(manifest_path):
        with open(manifest_path) as f:
            if f.read() == current:
                return False
    with open(manifest_path, "w") as f:
        f.write(current)
    return True


def _acquire_lock(build_dir):
    """Acquire exclusive file lock. Returns lock fd or exits."""
    os.makedirs(build_dir, exist_ok=True)
    lock_path = os.path.join(build_dir, ".lock")
    lock_fd = open(lock_path, "w")
    try:
        fcntl.flock(lock_fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
        lock_fd.write(str(os.getpid()))
        lock_fd.flush()
        return lock_fd
    except (IOError, OSError):
        print("Another test run is in progress. Skipping.")
        sys.exit(2)


def _load_db(build_dir):
    path = os.path.join(build_dir, "db.json")
    if os.path.exists(path):
        try:
            with open(path) as f:
                return json.load(f)
        except (json.JSONDecodeError, OSError):
            pass
    return {}


def _save_db(build_dir, db):
    path = os.path.join(build_dir, "db.json")
    with open(path, "w") as f:
        json.dump(db, f, indent=2)


def _parse_make_errors(make_output, compile_jobs, build_dir):
    if not make_output or not make_output.strip():
        return {}, ""

    job_entries = []
    basename_to_ids = {}
    for suite, test_name, test_file, binary in compile_jobs:
        job_id = os.path.basename(binary)
        rel_test = os.path.normpath(os.path.relpath(test_file, build_dir)).replace(
            "\\", "/"
        )
        abs_test = os.path.normpath(test_file).replace("\\", "/")
        basename = os.path.basename(test_file)
        basename_to_ids.setdefault(basename, []).append(job_id)
        job_entries.append(
            {
                "job_id": job_id,
                "binary": os.path.basename(binary),
                "rel_test": rel_test,
                "abs_test": abs_test,
                "basename": basename,
            }
        )

    errors = {}
    unassigned = []

    def _append_block(target_id, lines):
        if not lines:
            return
        block = "\n".join(lines)
        if target_id is None:
            unassigned.append(block)
            return
        if target_id in errors and errors[target_id]:
            errors[target_id] = errors[target_id] + "\n" + block
        else:
            errors[target_id] = block

    def _match_job_id(line):
        normalized = line.replace("\\", "/")

        for entry in job_entries:
            if entry["binary"] in line:
                return entry["job_id"]
        for entry in job_entries:
            if entry["rel_test"] in normalized or entry["abs_test"] in normalized:
                return entry["job_id"]
        for basename, ids in basename_to_ids.items():
            if len(ids) == 1 and basename in normalized:
                return ids[0]
        return None

    current_id = None
    current_lines = []
    for line in make_output.splitlines():
        matched_id = _match_job_id(line)
        if matched_id is not None and matched_id != current_id:
            _append_block(current_id, current_lines)
            current_id = matched_id
            current_lines = [line]
        else:
            current_lines.append(line)

    _append_block(current_id, current_lines)
    return errors, "\n".join(unassigned).strip()


def run_single_test(binary_path):
    start = time.perf_counter()
    try:
        result = subprocess.run(
            [binary_path], capture_output=True, text=True, timeout=10
        )
        duration_ms = (time.perf_counter() - start) * 1000
        output = (result.stdout or "") + (result.stderr or "")
        return result.returncode, output, duration_ms, False, False
    except subprocess.TimeoutExpired:
        duration_ms = (time.perf_counter() - start) * 1000
        return -1, "", duration_ms, False, True
    except (FileNotFoundError, PermissionError, OSError) as e:
        duration_ms = (time.perf_counter() - start) * 1000
        return 1, str(e), duration_ms, False, False
    except Exception as e:
        duration_ms = (time.perf_counter() - start) * 1000
        return 1, str(e), duration_ms, False, False


@dataclass
class TestResult:
    status: str = "pending"
    duration_ms: float = 0.0
    output: str = ""


class TestState:
    def __init__(self):
        self.results = {}
        self.test_labels = {}
        self.suites_order = []
        self.suite_tests = {}
        self.suite_done = set()
        self.suite_total_ms = {}
        self.suite_start_times = {}
        self.grand_ms = 0.0
        self.grand_start_time = None
        self.all_done = False
        self.spinner_idx = 0

    def advance_spinner(self):
        self.spinner_idx = (self.spinner_idx + 1) % len(SPINNER_FRAMES)

    @property
    def spinner(self):
        return SPINNER_FRAMES[self.spinner_idx % len(SPINNER_FRAMES)]

    def _suite_guide_style(self, suite):
        if suite not in self.suite_done:
            return "cyan"
        has_fail = any(
            self.results.get(tn) is not None
            and self.results[tn].status not in ("pending", "running", "passed")
            for tn in self.suite_tests.get(suite, [])
        )
        return "red" if has_fail else "green"


def _build_error_panel(result):
    if result.status in ("compile_fail", "compile_fail_cached"):
        content = Text.from_ansi(result.output) if result.output else Text("")
        title = "Compilation Error"
        if result.status == "compile_fail_cached":
            title = "Compilation Error (cached)"
        return Panel(
            content,
            title=title,
            border_style="red",
            title_align="left",
        )
    if result.status == "segfault":
        msg = result.output.strip() if result.output.strip() else ""
        lines = []
        if msg:
            lines.append(msg)
        lines.append("Process terminated with signal 11 (segmentation fault)")
        return Panel(
            "\n".join(lines),
            title="Segfault",
            border_style="red",
            title_align="left",
        )
    if result.status == "timeout":
        return Panel(
            "Test exceeded 10 second limit",
            title="Timeout",
            border_style="yellow",
            title_align="left",
        )
    content = result.output.strip() if result.output.strip() else "(no output)"
    return Panel(
        content,
        title="Runtime Error",
        border_style="red",
        title_align="left",
    )


def build_renderable(state):
    elements = []
    now = time.perf_counter()

    for suite in state.suites_order:
        test_ids = state.suite_tests.get(suite, [])
        is_done = suite in state.suite_done
        guide_style = state._suite_guide_style(suite)

        if is_done:
            total_ms = state.suite_total_ms.get(suite, 0)
            label = f"{suite} ── [{total_ms:.0f}ms]"
        else:
            start_time = state.suite_start_times.get(suite, now)
            elapsed_ms = (now - start_time) * 1000
            label = f"{suite} {state.spinner} ── [{elapsed_ms:.0f}ms]"

        tree = Tree(label, guide_style=guide_style)

        max_name_len = max(
            (len(state.test_labels.get(test_id, test_id)) for test_id in test_ids),
            default=0,
        )

        for test_id in test_ids:
            label = state.test_labels.get(test_id, test_id)
            res = state.results.get(test_id)
            if res is None or res.status in ("pending", "running"):
                padded = label.ljust(max_name_len)
                tree.add(Text(f"{state.spinner} {padded}  pending", style="yellow"))
            elif res.status == "passed":
                padded = label.ljust(max_name_len)
                tree.add(Text(f"✓ {padded}  [{res.duration_ms:.0f}ms]", style="green"))
            else:
                padded = label.ljust(max_name_len)
                if res.status == "compile_fail_cached":
                    fail_node = tree.add(
                        Text(f"✗ {padded}  [cached]", style="bold red")
                    )
                else:
                    fail_node = tree.add(
                        Text(f"✗ {padded}  [{res.duration_ms:.0f}ms]", style="bold red")
                    )
                fail_node.add(_build_error_panel(res))

        elements.append(tree)

    total_count = len(state.results)
    total_passed = sum(1 for r in state.results.values() if r.status == "passed")
    total_failed = sum(
        1
        for r in state.results.values()
        if r.status not in ("pending", "running", "passed")
    )

    if state.grand_start_time is not None and not state.all_done:
        results_ms = (now - state.grand_start_time) * 1000
    else:
        results_ms = state.grand_ms

    results_tree = Tree(f"Results ── [{results_ms:.0f}ms]", guide_style="dim")
    if total_count > 0:
        results_tree.add(Text(f"{total_passed}/{total_count} passed", style="green"))
        if total_failed > 0:
            results_tree.add(Text(f"✗ {total_failed} failed", style="bold red"))

    elements.append(results_tree)
    return Group(*elements)


def _print_help():
    BOLD = "\033[1m"
    DIM = "\033[2m"
    CYAN = "\033[36m"
    GREEN = "\033[32m"
    RESET = "\033[0m"

    print(f"""
{BOLD}Philosophers Test Runner{RESET}

{CYAN}USAGE{RESET}
  python3 run_tests.py {DIM}[OPTIONS] [SUITE...]{RESET}

{CYAN}DESCRIPTION{RESET}
  Discovers and runs C test suites with incremental builds via make.
  Only recompiles files that have changed since the last run.

{CYAN}ARGUMENTS{RESET}
  {BOLD}SUITE...{RESET}        Test suite(s) to run (default: all)
                  Available: {", ".join(discover_suites()) or "(none)"}

{CYAN}OPTIONS{RESET}
  {BOLD}--clean{RESET}         Full rebuild from scratch ({DIM}make clean{RESET} + rebuild)
  {BOLD}--max-parallel N{RESET}  Max concurrent test processes ({GREEN}default: 1{RESET})
  {BOLD}--disable-ccache{RESET}  Disable ccache even if available
  {BOLD}--help{RESET}           Show this help message

{CYAN}EXAMPLES{RESET}
    python3 run_tests.py                        {DIM}# incremental build + run all{RESET}
    python3 run_tests.py --clean                {DIM}# full rebuild{RESET}
    python3 run_tests.py table_create           {DIM}# run a specific suite{RESET}
    python3 run_tests.py foo_tests              {DIM}# run nested suites under foo_tests{RESET}
    python3 run_tests.py foo_tests/a_tests      {DIM}# run one nested suite path{RESET}
    python3 run_tests.py --max-parallel 5       {DIM}# run tests in parallel{RESET}
    python3 run_tests.py --disable-ccache       {DIM}# skip ccache{RESET}
""")


def main():
    if "--help" in sys.argv or "-h" in sys.argv:
        _print_help()
        sys.exit(0)

    do_clean = "--clean" in sys.argv
    if do_clean:
        sys.argv.remove("--clean")

    max_parallel = 1
    if "--max-parallel" in sys.argv:
        idx = sys.argv.index("--max-parallel")
        if idx + 1 >= len(sys.argv):
            print("--max-parallel requires a positive integer value")
            sys.exit(1)
        raw_value = sys.argv[idx + 1]
        try:
            max_parallel = int(raw_value)
        except ValueError:
            print(f"Invalid --max-parallel value: {raw_value}")
            sys.exit(1)
        if max_parallel < 1:
            print("--max-parallel must be >= 1")
            sys.exit(1)
        del sys.argv[idx : idx + 2]

    use_ccache = "--disable-ccache" not in sys.argv
    if "--disable-ccache" in sys.argv:
        sys.argv.remove("--disable-ccache")

    if use_ccache:
        if shutil.which("ccache") is None:
            use_ccache = False

    all_suites = discover_suites()
    suites = list(all_suites)
    if len(sys.argv) > 1:
        requested = sys.argv[1:]
        suites, unknown = _match_requested_suites(all_suites, requested)
        if unknown:
            print(f"Unknown suites: {', '.join(sorted(set(unknown)))}")
            print(f"Available: {', '.join(discover_suites())}")
            sys.exit(1)

    if not suites:
        print("No test suites found.")
        sys.exit(1)

    project_sources = get_project_sources()
    project_sources_mtime = max(
        (os.path.getmtime(path) for path in project_sources if os.path.exists(path)),
        default=0,
    )

    compile_jobs = []
    binaries_by_id = {}
    state = TestState()
    for suite in suites:
        tests = discover_tests(suite)
        for test_file in tests:
            test_name = os.path.splitext(os.path.basename(test_file))[0]
            target_name = _target_name_for_suite_test(suite, test_name)
            binary = os.path.join(BUILD_DIR, target_name)
            compile_jobs.append((suite, test_name, test_file, binary))
            test_id = os.path.basename(binary)
            binaries_by_id[test_id] = binary
            state.results[test_id] = TestResult(status="pending")
            state.test_labels[test_id] = test_name
            state.suite_tests.setdefault(suite, []).append(test_id)
    state.suites_order = suites

    lock_fd = _acquire_lock(BUILD_DIR)

    try:
        if do_clean:
            subprocess.run(["make", "-C", BUILD_DIR, "clean"], capture_output=True)
            _save_db(BUILD_DIR, {})

        if _makefile_needs_regen(BUILD_DIR, compile_jobs, project_sources, use_ccache):
            _generate_makefile(
                BUILD_DIR, SRC_DIR, project_sources, compile_jobs, use_ccache
            )

        cpu_count = os.cpu_count() or 4
        make_result = subprocess.run(
            ["make", "-C", BUILD_DIR, f"-j{cpu_count}"],
            capture_output=True,
            text=True,
        )

        db = _load_db(BUILD_DIR)

        make_output = "\n".join(
            part for part in (make_result.stderr, make_result.stdout) if part
        )
        build_failed = make_result.returncode != 0
        make_errors = {}
        unassigned_make_errors = ""
        if build_failed:
            make_errors, unassigned_make_errors = _parse_make_errors(
                make_output, compile_jobs, BUILD_DIR
            )
        generic_build_error = (
            unassigned_make_errors
            or make_output.strip()
            or "Build failed without diagnostic output"
        )

        for suite, test_name, test_file, binary in compile_jobs:
            test_id = os.path.basename(binary)
            test_source_mtime = (
                os.path.getmtime(test_file) if os.path.exists(test_file) else 0
            )
            binary_mtime = os.path.getmtime(binary) if os.path.exists(binary) else 0
            input_mtime = max(test_source_mtime, project_sources_mtime)

            if test_id in make_errors:
                entry = {
                    "status": "compile_fail",
                    "duration_ms": 0,
                    "compile_error": make_errors[test_id],
                    "source_mtime": input_mtime,
                    "binary_mtime": None,
                }
                db[test_id] = entry
                res = TestResult(status="compile_fail", duration_ms=0)
                res.output = make_errors[test_id]
                state.results[test_id] = res
                if os.path.exists(binary):
                    os.remove(binary)
                continue

            if build_failed:
                entry = {
                    "status": "compile_fail",
                    "duration_ms": 0,
                    "compile_error": generic_build_error,
                    "source_mtime": input_mtime,
                    "binary_mtime": None,
                }
                db[test_id] = entry
                res = TestResult(status="compile_fail", duration_ms=0)
                res.output = generic_build_error
                state.results[test_id] = res
                if os.path.exists(binary):
                    os.remove(binary)
                continue

            if os.path.exists(binary) and input_mtime > binary_mtime:
                entry = db.get(test_id)
                if entry and entry.get("compile_error"):
                    res = TestResult(status="compile_fail_cached", duration_ms=0)
                    res.output = entry["compile_error"]
                    state.results[test_id] = res
                    os.remove(binary)
                    continue
                else:
                    os.remove(binary)
                    res = TestResult(status="compile_fail", duration_ms=0)
                    res.output = "Binary is stale (source modified) but no cached error"
                    state.results[test_id] = res
                    continue

            if not os.path.exists(binary):
                entry = db.get(test_id)
                if entry and entry.get("compile_error"):
                    cached_source_mtime = entry.get("source_mtime", 0)
                    if input_mtime == cached_source_mtime:
                        res = TestResult(status="compile_fail_cached", duration_ms=0)
                        res.output = entry["compile_error"]
                        state.results[test_id] = res
                        continue
                    else:
                        res = TestResult(status="compile_fail", duration_ms=0)
                        res.output = (
                            "No binary produced but source differs from cached error"
                        )
                        state.results[test_id] = res
                        continue
                res = TestResult(status="compile_fail", duration_ms=0)
                res.output = "No binary found"
                state.results[test_id] = res
                continue

            db[test_id] = {
                "status": "passed",
                "duration_ms": 0,
                "compile_error": None,
                "source_mtime": input_mtime,
                "binary_mtime": binary_mtime,
            }

        console = Console()

        any_failed = any(
            r.status in ("compile_fail", "compile_fail_cached")
            for r in state.results.values()
        )
        grand_start = time.perf_counter()
        state.grand_start_time = grand_start

        with Live(console=console, refresh_per_second=10, transient=True) as live:
            live.update(build_renderable(state))

            for suite in suites:
                test_ids = state.suite_tests.get(suite, [])
                if not test_ids:
                    state.suite_total_ms[suite] = 0
                    state.suite_done.add(suite)
                    live.update(build_renderable(state))
                    continue

                suite_start = time.perf_counter()
                state.suite_start_times[suite] = suite_start

                run_futures = {}
                with ProcessPoolExecutor(max_workers=max_parallel) as executor:
                    for test_id in test_ids:
                        binary = binaries_by_id.get(test_id)
                        if binary is None:
                            any_failed = True
                            res = TestResult(status="failed", duration_ms=0)
                            res.output = "Internal error: missing binary mapping"
                            state.results[test_id] = res
                            continue
                        if state.results[test_id].status in (
                            "compile_fail",
                            "compile_fail_cached",
                        ):
                            any_failed = True
                            continue
                        state.results[test_id].status = "running"
                        state.advance_spinner()
                        live.update(build_renderable(state))
                        future = executor.submit(run_single_test, binary)
                        run_futures[future] = test_id

                    for future in as_completed(run_futures):
                        test_id = run_futures[future]
                        try:
                            rc, output, duration_ms, segfault, timeout = future.result()
                        except Exception as e:
                            res = TestResult(status="failed", duration_ms=0)
                            res.output = f"Worker execution failed: {e}"
                            state.results[test_id] = res
                            state.advance_spinner()
                            any_failed = True
                            live.update(build_renderable(state))
                            continue

                        if timeout:
                            res = TestResult(status="timeout", duration_ms=duration_ms)
                        elif segfault or rc == 139:
                            res = TestResult(status="segfault", duration_ms=duration_ms)
                            res.output = output.strip() if output.strip() else ""
                        elif rc != 0:
                            res = TestResult(status="failed", duration_ms=duration_ms)
                            res.output = output.strip() if output.strip() else ""
                        else:
                            res = TestResult(status="passed", duration_ms=duration_ms)

                        state.results[test_id] = res
                        state.advance_spinner()
                        if res.status != "passed":
                            any_failed = True
                        live.update(build_renderable(state))

                suite_end = time.perf_counter()
                state.suite_total_ms[suite] = (suite_end - suite_start) * 1000
                state.suite_done.add(suite)
                live.update(build_renderable(state))

        grand_end = time.perf_counter()
        state.grand_ms = (grand_end - grand_start) * 1000
        state.all_done = True

        for suite, test_name, test_file, binary in compile_jobs:
            test_id = os.path.basename(binary)
            res = state.results.get(test_id)
            if res and res.status not in ("compile_fail", "compile_fail_cached"):
                if test_id in db:
                    db[test_id]["status"] = res.status
                    db[test_id]["duration_ms"] = res.duration_ms
                    db[test_id]["compile_error"] = None
                    db[test_id]["binary_mtime"] = (
                        os.path.getmtime(binary) if os.path.exists(binary) else None
                    )

        _save_db(BUILD_DIR, db)

        console.print(build_renderable(state))
        sys.exit(1 if any_failed else 0)
    finally:
        try:
            fcntl.flock(lock_fd, fcntl.LOCK_UN)
            lock_fd.close()
        except Exception:
            pass


if __name__ == "__main__":
    main()
