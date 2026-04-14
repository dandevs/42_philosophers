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

    choice = input(f"  Enter choice [{BOLD}1{RESET}] (default: pip): ").strip()

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


def get_project_sources():
    sources = []
    for path in glob.glob(os.path.join(SRC_DIR, "**", "*.c"), recursive=True):
        if os.path.basename(path) != "main.c":
            sources.append(path)
    return sources


def discover_suites():
    suites = []
    for entry in sorted(os.listdir(TESTS_DIR)):
        full = os.path.join(TESTS_DIR, entry)
        if os.path.isdir(full):
            suites.append(entry)
    return suites


def discover_tests(suite):
    suite_dir = os.path.join(TESTS_DIR, suite)
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


def _makefile_needs_regen(build_dir, compile_jobs):
    """Check if Makefile needs regeneration by comparing manifest."""
    manifest_path = os.path.join(build_dir, ".manifest")
    current = "\n".join(
        f"{suite}:{test_file}:{os.path.basename(binary)}"
        for suite, test_name, test_file, binary in compile_jobs
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
        sys.exit(0)


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
    except Exception:
        duration_ms = (time.perf_counter() - start) * 1000
        return 139, "", duration_ms, True, False


@dataclass
class TestResult:
    status: str = "pending"
    duration_ms: float = 0.0
    output: str = ""


class TestState:
    def __init__(self):
        self.results = {}
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
    if result.status == "compile_fail":
        content = Text.from_ansi(result.output) if result.output else Text("")
        return Panel(
            content,
            title="Compilation Error",
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
        test_names = state.suite_tests.get(suite, [])
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

        max_name_len = max((len(tn) for tn in test_names), default=0)

        for tn in test_names:
            res = state.results.get(tn)
            if res is None or res.status in ("pending", "running"):
                padded = tn.ljust(max_name_len)
                tree.add(Text(f"{state.spinner} {padded}  running...", style="yellow"))
            elif res.status == "passed":
                padded = tn.ljust(max_name_len)
                tree.add(Text(f"✓ {padded}  [{res.duration_ms:.0f}ms]", style="green"))
            else:
                padded = tn.ljust(max_name_len)
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


def main():
    do_clean = "--clean" in sys.argv
    if do_clean:
        sys.argv.remove("--clean")

    max_parallel = 1
    if "--max-parallel" in sys.argv:
        idx = sys.argv.index("--max-parallel")
        if idx + 1 < len(sys.argv):
            try:
                max_parallel = int(sys.argv[idx + 1])
            except ValueError:
                pass
            sys.argv.pop(idx + 1)
        sys.argv.pop(idx)

    use_ccache = "--disable-ccache" not in sys.argv
    if "--disable-ccache" in sys.argv:
        sys.argv.remove("--disable-ccache")

    if use_ccache:
        if shutil.which("ccache") is None:
            use_ccache = False

    suites = discover_suites()
    if len(sys.argv) > 1:
        requested = sys.argv[1:]
        suites = [s for s in suites if s in requested]
        unknown = set(requested) - set(suites)
        if unknown:
            print(f"Unknown suites: {', '.join(unknown)}")
            print(f"Available: {', '.join(discover_suites())}")
            sys.exit(1)

    if not suites:
        print("No test suites found.")
        sys.exit(1)

    project_sources = get_project_sources()

    compile_jobs = []
    state = TestState()
    for suite in suites:
        tests = discover_tests(suite)
        for test_file in tests:
            test_name = os.path.splitext(os.path.basename(test_file))[0]
            binary = os.path.join(BUILD_DIR, f"{suite}_{test_name}")
            compile_jobs.append((suite, test_name, test_file, binary))
            state.results[test_name] = TestResult(status="pending")
            state.suite_tests.setdefault(suite, []).append(test_name)
    state.suites_order = suites

    lock_fd = _acquire_lock(BUILD_DIR)

    try:
        if do_clean:
            subprocess.run(["make", "-C", BUILD_DIR, "clean"], capture_output=True)

        if _makefile_needs_regen(BUILD_DIR, compile_jobs):
            _generate_makefile(
                BUILD_DIR, SRC_DIR, project_sources, compile_jobs, use_ccache
            )

        cpu_count = os.cpu_count() or 4
        make_result = subprocess.run(
            ["make", "-C", BUILD_DIR, f"-j{cpu_count}"],
            capture_output=True,
            text=True,
        )

        if make_result.returncode != 0:
            for suite, test_name, test_file, binary in compile_jobs:
                if not os.path.exists(binary):
                    test_basename = os.path.basename(test_file)
                    relevant = []
                    for line in make_result.stderr.splitlines():
                        if test_basename in line or test_file in line:
                            relevant.append(line)
                    if not relevant:
                        relevant = make_result.stderr.splitlines()
                    error_output = "\n".join(relevant)
                    res = TestResult(status="compile_fail", duration_ms=0)
                    res.output = (
                        error_output.strip()
                        if error_output.strip()
                        else make_result.stderr.strip()
                    )
                    if test_name in state.results:
                        state.results[test_name] = res

        console = Console()

        any_failed = any(r.status == "compile_fail" for r in state.results.values())
        grand_start = time.perf_counter()
        state.grand_start_time = grand_start

        with Live(console=console, refresh_per_second=10, transient=True) as live:
            live.update(build_renderable(state))

            for suite in suites:
                test_names = state.suite_tests.get(suite, [])
                if not test_names:
                    state.suite_total_ms[suite] = 0
                    state.suite_done.add(suite)
                    live.update(build_renderable(state))
                    continue

                suite_start = time.perf_counter()
                state.suite_start_times[suite] = suite_start

                run_futures = {}
                with ProcessPoolExecutor(max_workers=max_parallel) as executor:
                    for tn in test_names:
                        binary = None
                        for s, t_name, t_file, b in compile_jobs:
                            if t_name == tn and s == suite:
                                binary = b
                                break
                        if binary is None:
                            continue
                        if state.results[tn].status == "compile_fail":
                            any_failed = True
                            continue
                        state.results[tn].status = "running"
                        state.advance_spinner()
                        live.update(build_renderable(state))
                        future = executor.submit(run_single_test, binary)
                        run_futures[future] = tn

                    for future in as_completed(run_futures):
                        test_name = run_futures[future]
                        rc, output, duration_ms, segfault, timeout = future.result()

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

                        state.results[test_name] = res
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
