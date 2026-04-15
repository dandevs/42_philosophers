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
    python3 run_tests.py --watch      # watch src/tests and rerun impacted tests
    python3 run_tests.py --timeout-ms 3000  # set per-test timeout to 3 seconds
"""

import fcntl
import glob
import hashlib
import json
import queue
import os
import shutil
import subprocess
import sys
import threading
import time
from dataclasses import dataclass, field


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
WATCH_EXTENSIONS = {".c", ".h"}
DEFAULT_TEST_TIMEOUT_MS = 10000
SPINNER_TICK_SECONDS = 0.05


@dataclass
class RunnerOptions:
    do_clean: bool = False
    max_parallel: int = 1
    use_ccache: bool = True
    watch_mode: bool = False
    watch_initial: bool = True
    debounce_ms: int = 100
    poll_interval_ms: int = 250
    timeout_ms: int = DEFAULT_TEST_TIMEOUT_MS
    requested_suites: list = None


class _ChangedFilesBuffer:
    def __init__(self):
        self._lock = threading.Lock()
        self._paths = set()

    def add(self, rel_path):
        if not rel_path:
            return
        with self._lock:
            self._paths.add(rel_path)

    def drain(self):
        with self._lock:
            if not self._paths:
                return set()
            paths = set(self._paths)
            self._paths.clear()
            return paths


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
                db = json.load(f)
            for test_id, entry in db.items():
                compile_error = entry.get("compile_error")
                if not compile_error or not _is_make_noop_block(compile_error):
                    continue
                binary_path = os.path.join(build_dir, test_id)
                entry["compile_error"] = None
                entry["duration_ms"] = 0
                entry["binary_mtime"] = (
                    os.path.getmtime(binary_path)
                    if os.path.exists(binary_path)
                    else None
                )
                if os.path.exists(binary_path):
                    entry["status"] = "passed"
                else:
                    entry["status"] = "pending"
            return db
        except (json.JSONDecodeError, OSError):
            pass
    return {}


def _save_db(build_dir, db):
    path = os.path.join(build_dir, "db.json")
    with open(path, "w") as f:
        json.dump(db, f, indent=2)


def _is_make_noop_line(line):
    normalized = line.strip().lower()
    if not normalized:
        return True
    return (
        "is up to date." in normalized
        or "nothing to be done for" in normalized
        or "entering directory" in normalized
        or "leaving directory" in normalized
    )


def _is_make_noop_block(block):
    lines = [line for line in block.splitlines() if line.strip()]
    if not lines:
        return False
    return all(_is_make_noop_line(line) for line in lines)


def _parse_cli_args(argv):
    options = RunnerOptions(requested_suites=[])
    i = 1
    while i < len(argv):
        arg = argv[i]
        if arg in ("--help", "-h"):
            _print_help()
            sys.exit(0)
        if arg == "--clean":
            options.do_clean = True
            i += 1
            continue
        if arg == "--disable-ccache":
            options.use_ccache = False
            i += 1
            continue
        if arg == "--watch":
            options.watch_mode = True
            i += 1
            continue
        if arg == "--watch-no-initial":
            options.watch_mode = True
            options.watch_initial = False
            i += 1
            continue
        if arg == "--max-parallel":
            if i + 1 >= len(argv):
                print("--max-parallel requires a positive integer value")
                sys.exit(1)
            raw_value = argv[i + 1]
            try:
                options.max_parallel = int(raw_value)
            except ValueError:
                print(f"Invalid --max-parallel value: {raw_value}")
                sys.exit(1)
            if options.max_parallel < 1:
                print("--max-parallel must be >= 1")
                sys.exit(1)
            i += 2
            continue
        if arg == "--debounce-ms":
            if i + 1 >= len(argv):
                print("--debounce-ms requires a positive integer value")
                sys.exit(1)
            raw_value = argv[i + 1]
            try:
                options.debounce_ms = int(raw_value)
            except ValueError:
                print(f"Invalid --debounce-ms value: {raw_value}")
                sys.exit(1)
            if options.debounce_ms < 1:
                print("--debounce-ms must be >= 1")
                sys.exit(1)
            i += 2
            continue
        if arg == "--poll-interval-ms":
            if i + 1 >= len(argv):
                print("--poll-interval-ms requires a positive integer value")
                sys.exit(1)
            raw_value = argv[i + 1]
            try:
                options.poll_interval_ms = int(raw_value)
            except ValueError:
                print(f"Invalid --poll-interval-ms value: {raw_value}")
                sys.exit(1)
            if options.poll_interval_ms < 1:
                print("--poll-interval-ms must be >= 1")
                sys.exit(1)
            i += 2
            continue
        if arg == "--timeout-ms":
            if i + 1 >= len(argv):
                print("--timeout-ms requires a positive integer value")
                sys.exit(1)
            raw_value = argv[i + 1]
            try:
                options.timeout_ms = int(raw_value)
            except ValueError:
                print(f"Invalid --timeout-ms value: {raw_value}")
                sys.exit(1)
            if options.timeout_ms < 1:
                print("--timeout-ms must be >= 1")
                sys.exit(1)
            i += 2
            continue
        if arg.startswith("--"):
            print(f"Unknown option: {arg}")
            sys.exit(1)
        options.requested_suites.append(arg)
        i += 1

    if options.use_ccache and shutil.which("ccache") is None:
        options.use_ccache = False
    return options


def _resolve_requested_suites(requested):
    all_suites = discover_suites()
    suites = list(all_suites)
    if requested:
        suites, unknown = _match_requested_suites(all_suites, requested)
        if unknown:
            print(f"Unknown suites: {', '.join(sorted(set(unknown)))}")
            print(f"Available: {', '.join(discover_suites())}")
            sys.exit(1)
    return suites


def _build_compile_jobs(suites):
    compile_jobs = []
    binaries_by_id = {}
    for suite in suites:
        tests = discover_tests(suite)
        for test_file in tests:
            test_name = os.path.splitext(os.path.basename(test_file))[0]
            target_name = _target_name_for_suite_test(suite, test_name)
            binary = os.path.join(BUILD_DIR, target_name)
            compile_jobs.append((suite, test_name, test_file, binary))
            test_id = os.path.basename(binary)
            binaries_by_id[test_id] = binary
    return compile_jobs, binaries_by_id


def _normalize_to_project_rel(path):
    normalized = os.path.normpath(path)
    try:
        rel = os.path.relpath(normalized, PROJECT_ROOT)
    except ValueError:
        return None
    rel = rel.replace("\\", "/")
    if rel.startswith("../"):
        return None
    return rel


def _is_watch_candidate(rel_path):
    ext = os.path.splitext(rel_path)[1].lower()
    if ext in WATCH_EXTENSIONS:
        return True
    return False


def _path_under(base, path):
    try:
        return os.path.commonpath([base, path]) == base
    except ValueError:
        return False


def _collect_polling_snapshot():
    snapshot = {}
    for root in (SRC_DIR, TESTS_DIR):
        if not os.path.isdir(root):
            continue
        for current_root, _, files in os.walk(root):
            for name in files:
                abs_path = os.path.join(current_root, name)
                rel = _normalize_to_project_rel(abs_path)
                if rel is None or not _is_watch_candidate(rel):
                    continue
                try:
                    snapshot[rel] = os.path.getmtime(abs_path)
                except OSError:
                    continue
    return snapshot


def _poll_snapshot_changes(snapshot):
    current = _collect_polling_snapshot()
    changed = set()

    for rel, mtime in current.items():
        if rel not in snapshot or snapshot[rel] != mtime:
            changed.add(rel)

    for rel in snapshot:
        if rel not in current:
            changed.add(rel)

    return changed, current


def _wait_for_changes(get_changes_fn, debounce_ms, poll_interval_ms):
    pending = set()
    last_change_time = None
    debounce_seconds = debounce_ms / 1000.0
    poll_seconds = poll_interval_ms / 1000.0

    while True:
        fresh = get_changes_fn()
        now = time.time()
        if fresh:
            pending.update(fresh)
            last_change_time = now
        elif pending and last_change_time is not None:
            if now - last_change_time >= debounce_seconds:
                return pending
        time.sleep(poll_seconds)


def _parse_dep_file(dep_path):
    if not os.path.exists(dep_path):
        return set()

    try:
        with open(dep_path) as f:
            raw = f.read()
    except OSError:
        return set()

    merged = raw.replace("\\\n", " ").replace("\n", " ")
    if ":" not in merged:
        return set()

    _, deps_part = merged.split(":", 1)
    deps = set()
    for token in deps_part.split():
        item = token.strip()
        if not item:
            continue
        if item.endswith(":"):
            item = item[:-1]
        if not item:
            continue
        abs_dep = os.path.normpath(os.path.join(BUILD_DIR, item))
        rel_dep = _normalize_to_project_rel(abs_dep)
        if rel_dep is not None:
            deps.add(rel_dep)
    return deps


def _collect_source_dep_index(project_sources):
    index = {}
    for source in project_sources:
        rel_source_from_src = os.path.relpath(source, SRC_DIR)
        dep_path = os.path.join(
            BUILD_DIR, "obj", os.path.splitext(rel_source_from_src)[0] + ".d"
        )
        rel_source = _normalize_to_project_rel(source)
        deps = _parse_dep_file(dep_path)
        if rel_source is not None:
            deps.add(rel_source)
            index[rel_source] = deps
    return index


def _collect_test_dep_index(compile_jobs):
    index = {}
    for suite, test_name, test_file, binary in compile_jobs:
        test_id = os.path.basename(binary)
        dep_path = os.path.join(BUILD_DIR, test_id + ".d")
        deps = _parse_dep_file(dep_path)
        rel_test = _normalize_to_project_rel(test_file)
        if rel_test is not None:
            deps.add(rel_test)
        index[test_id] = deps
    return index


def _collect_dependency_index(compile_jobs, project_sources):
    source_dep_index = _collect_source_dep_index(project_sources)
    test_dep_index = _collect_test_dep_index(compile_jobs)

    header_to_sources = {}
    source_headers = {}
    for source_rel, deps in source_dep_index.items():
        headers = {dep for dep in deps if dep.endswith(".h")}
        source_headers[source_rel] = headers
        for header in headers:
            header_to_sources.setdefault(header, set()).add(source_rel)

    merged = {}
    for test_id, deps in test_dep_index.items():
        combined = set(deps)
        test_headers = {dep for dep in deps if dep.endswith(".h")}
        source_matches = set()
        for header in test_headers:
            source_matches.update(header_to_sources.get(header, set()))
        for source_rel in source_matches:
            combined.add(source_rel)
            combined.update(source_headers.get(source_rel, set()))
        merged[test_id] = sorted(combined)
    return merged


def _max_mtime_for_deps(dep_paths):
    if not dep_paths:
        return 0
    max_mtime = 0
    for rel_path in dep_paths:
        abs_path = os.path.join(PROJECT_ROOT, rel_path)
        try:
            mtime = os.path.getmtime(abs_path)
        except OSError:
            continue
        if mtime > max_mtime:
            max_mtime = mtime
    return max_mtime


def _resolve_impacted_test_ids(changed_files, compile_jobs, db):
    ordered_ids = [os.path.basename(binary) for _, _, _, binary in compile_jobs]
    if not changed_files:
        return []

    dep_to_tests = {}
    test_file_to_id = {}
    for suite, test_name, test_file, binary in compile_jobs:
        test_id = os.path.basename(binary)
        rel_test = _normalize_to_project_rel(test_file)
        if rel_test is not None:
            test_file_to_id[rel_test] = test_id
        entry = db.get(test_id, {})
        deps = entry.get("deps") or []
        for dep in deps:
            dep_to_tests.setdefault(dep, set()).add(test_id)

    impacted = set()
    src_unmapped = False
    tests_unmapped = False

    for changed in changed_files:
        impacted.update(dep_to_tests.get(changed, set()))
        if changed in test_file_to_id:
            impacted.add(test_file_to_id[changed])
        if changed.startswith("src/") and changed not in dep_to_tests:
            src_unmapped = True
        if changed.startswith("tests/") and changed not in dep_to_tests:
            ext = os.path.splitext(changed)[1].lower()
            if ext in WATCH_EXTENSIONS and changed not in test_file_to_id:
                tests_unmapped = True

    if src_unmapped or tests_unmapped:
        return ordered_ids

    if not impacted:
        return ordered_ids

    return [test_id for test_id in ordered_ids if test_id in impacted]


def _resolve_impacted_from_dep_index(changed_files, compile_jobs, dep_index):
    ordered_ids = [os.path.basename(binary) for _, _, _, binary in compile_jobs]
    if not changed_files:
        return []

    dep_to_tests = {}
    test_file_to_id = {}
    for suite, test_name, test_file, binary in compile_jobs:
        test_id = os.path.basename(binary)
        rel_test = _normalize_to_project_rel(test_file)
        if rel_test is not None:
            test_file_to_id[rel_test] = test_id
        deps = dep_index.get(test_id, [])
        for dep in deps:
            dep_to_tests.setdefault(dep, set()).add(test_id)

    impacted = set()
    src_unmapped = False
    tests_unmapped = False

    for changed in changed_files:
        impacted.update(dep_to_tests.get(changed, set()))
        if changed in test_file_to_id:
            impacted.add(test_file_to_id[changed])
        if changed.startswith("src/") and changed not in dep_to_tests:
            src_unmapped = True
        if changed.startswith("tests/") and changed not in dep_to_tests:
            ext = os.path.splitext(changed)[1].lower()
            if ext in WATCH_EXTENSIONS and changed not in test_file_to_id:
                tests_unmapped = True

    if src_unmapped or tests_unmapped:
        return ordered_ids
    if not impacted:
        return ordered_ids
    return [test_id for test_id in ordered_ids if test_id in impacted]


def _terminate_process(proc):
    if proc.poll() is not None:
        return
    try:
        proc.terminate()
        proc.wait(timeout=0.2)
    except Exception:
        try:
            proc.kill()
            proc.wait(timeout=0.2)
        except Exception:
            pass


def _try_start_watchdog(buffer):
    try:
        from watchdog.events import FileSystemEventHandler
        from watchdog.observers import Observer
    except ImportError:
        return None, "watchdog-not-installed"

    class _Handler(FileSystemEventHandler):
        def _add(self, path):
            if not path:
                return
            abs_path = os.path.abspath(path)
            if not (_path_under(SRC_DIR, abs_path) or _path_under(TESTS_DIR, abs_path)):
                return
            rel = _normalize_to_project_rel(abs_path)
            if rel is None or not _is_watch_candidate(rel):
                return
            buffer.add(rel)

        def on_created(self, event):
            if not event.is_directory:
                self._add(event.src_path)

        def on_modified(self, event):
            if not event.is_directory:
                self._add(event.src_path)

        def on_deleted(self, event):
            if not event.is_directory:
                self._add(event.src_path)

        def on_moved(self, event):
            if not event.is_directory:
                self._add(event.src_path)
                self._add(event.dest_path)

    observer = Observer()
    handler = _Handler()
    try:
        if os.path.isdir(SRC_DIR):
            observer.schedule(handler, SRC_DIR, recursive=True)
        if os.path.isdir(TESTS_DIR):
            observer.schedule(handler, TESTS_DIR, recursive=True)
        observer.start()
    except Exception:
        try:
            observer.stop()
            observer.join(timeout=1)
        except Exception:
            pass
        return None, "watchdog-start-failed"
    return observer, "watchdog"


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
        if _is_make_noop_block(block):
            return
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


TERMINAL_STATUSES = frozenset(
    (
        "passed",
        "failed",
        "segfault",
        "timeout",
        "compile_fail",
        "compile_fail_cached",
    )
)

RUNNABLE_STATUSES = frozenset(("pending_ready", "running"))


class TestItem:
    __slots__ = (
        "test_id",
        "suite",
        "test_name",
        "test_file",
        "binary",
        "status",
        "time_status_changed",
        "run_token",
        "duration_ms",
        "output",
        "proc",
        "start_time",
    )

    def __init__(
        self, test_id, suite, test_name, test_file, binary, status, time_status_changed
    ):
        self.test_id = test_id
        self.suite = suite
        self.test_name = test_name
        self.test_file = test_file
        self.binary = binary
        self.status = status
        self.time_status_changed = time_status_changed
        self.run_token = 0
        self.duration_ms = 0.0
        self.output = ""
        self.proc = None
        self.start_time = 0.0


class SchedulerState:
    def __init__(self, max_parallel, timeout_ms, binaries_by_id):
        self.tests = {}
        self.suites_order = []
        self.suite_tests = {}
        self.available_runners = max_parallel
        self.max_parallel = max_parallel
        self.timeout_ms = timeout_ms
        self.grand_start_time = None
        self.grand_ms = 0.0
        self.all_done = False
        self.any_failed = False
        self._dirty = False
        self.spinner_idx = 0
        self._tick_event = threading.Event()
        self._run_token_counter = 0
        self._completion_queue = queue.Queue()
        self._binaries_by_id = binaries_by_id

    def advance_spinner(self):
        self.spinner_idx = (self.spinner_idx + 1) % len(SPINNER_FRAMES)

    @property
    def spinner(self):
        return SPINNER_FRAMES[self.spinner_idx % len(SPINNER_FRAMES)]

    def state_changed(self):
        self._drain_completions()
        self._dispatch_pending()
        self._check_all_done()
        self._dirty = True
        self._tick_event.set()

    def _drain_completions(self):
        while True:
            try:
                item = self._completion_queue.get_nowait()
            except queue.Empty:
                break
            test_id, run_token, returncode, output, duration_ms = item
            test = self.tests.get(test_id)
            if test is None or test.run_token != run_token:
                continue
            test.proc = None
            test.duration_ms = duration_ms
            test.output = output
            test.time_status_changed = time.perf_counter()
            if returncode == 139:
                test.status = "segfault"
            elif returncode != 0:
                test.status = "failed"
            else:
                test.status = "passed"
            if test.status != "passed":
                self.any_failed = True
            self.available_runners += 1

    def _dispatch_pending(self):
        pending = sorted(
            (t for t in self.tests.values() if t.status == "pending_ready"),
            key=lambda t: t.time_status_changed,
        )
        for test in pending:
            if self.available_runners <= 0:
                break
            self.available_runners -= 1
            self._start_test(test)

    def _start_test(self, test):
        self._run_token_counter += 1
        test.run_token = self._run_token_counter
        test.status = "running"
        run_start = time.perf_counter()
        test.start_time = run_start
        test.time_status_changed = run_start
        try:
            proc = subprocess.Popen(
                [test.binary],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )
        except Exception as e:
            test.status = "failed"
            test.output = str(e)
            self.available_runners += 1
            self.any_failed = True
            return
        test.proc = proc
        threading.Thread(
            target=self._monitor_test,
            args=(test.test_id, test.run_token, proc, run_start),
            daemon=True,
        ).start()

    def _monitor_test(self, test_id, run_token, proc, start_time):
        try:
            out, err = proc.communicate()
        except Exception as e:
            out, err = "", str(e)
        duration_ms = (time.perf_counter() - start_time) * 1000
        output = (out or "") + (err or "")
        self._completion_queue.put(
            (test_id, run_token, proc.returncode, output, duration_ms)
        )
        self._tick_event.set()

    def check_timeouts(self):
        now = time.perf_counter()
        for test in self.tests.values():
            if test.status != "running":
                continue
            elapsed_ms = (now - test.start_time) * 1000
            if elapsed_ms > self.timeout_ms:
                _terminate_process(test.proc)
                test.proc = None
                test.run_token += 1
                test.status = "timeout"
                test.output = f"Test exceeded {self.timeout_ms}ms limit"
                test.duration_ms = elapsed_ms
                test.time_status_changed = now
                self.available_runners += 1
                self.any_failed = True
        self._dirty = True
        self._tick_event.set()

    def reprioritize(self, impacted_ids):
        now = time.perf_counter()
        for i, test_id in enumerate(impacted_ids):
            test = self.tests.get(test_id)
            if test is None:
                continue
            if test.status in TERMINAL_STATUSES:
                continue
            if test.status == "running":
                _terminate_process(test.proc)
                test.proc = None
                self.available_runners += 1
            test.run_token += 1
            test.status = "pending_ready"
            test.time_status_changed = now + 0.001 * (i + 1)
        self.state_changed()

    def _check_all_done(self):
        for test in self.tests.values():
            if test.status in RUNNABLE_STATUSES:
                return
        self.all_done = True

    def suite_guide_style(self, suite):
        test_ids = self.suite_tests.get(suite, [])
        has_active = any(
            self.tests.get(tid) is not None
            and self.tests[tid].status in RUNNABLE_STATUSES
            for tid in test_ids
        )
        if has_active:
            return "cyan"
        has_fail = any(
            self.tests.get(tid) is not None
            and self.tests[tid].status not in RUNNABLE_STATUSES
            and self.tests[tid].status != "passed"
            for tid in test_ids
        )
        return "red" if has_fail else "green"

    def suite_done(self, suite):
        test_ids = self.suite_tests.get(suite, [])
        return all(
            self.tests.get(tid) is not None
            and self.tests[tid].status not in RUNNABLE_STATUSES
            for tid in test_ids
        )

    def suite_total_ms(self, suite):
        test_ids = self.suite_tests.get(suite, [])
        if not self.suite_done(suite):
            return None
        return sum(
            self.tests[tid].duration_ms
            for tid in test_ids
            if self.tests.get(tid) is not None
        )

    def suite_start_time(self, suite):
        test_ids = self.suite_tests.get(suite, [])
        if not test_ids:
            return None
        runnable = [
            self.tests[tid]
            for tid in test_ids
            if self.tests.get(tid) is not None
            and self.tests[tid].status in RUNNABLE_STATUSES
        ]
        if not runnable:
            pending = [
                self.tests[tid] for tid in test_ids if self.tests.get(tid) is not None
            ]
            if not pending:
                return None
            return min(t.time_status_changed for t in pending)
        return min(t.time_status_changed for t in runnable)


def _build_error_panel(test):
    if test.status in ("compile_fail", "compile_fail_cached"):
        content = Text.from_ansi(test.output) if test.output else Text("")
        title = "Compilation Error"
        if test.status == "compile_fail_cached":
            title = "Compilation Error (cached)"
        return Panel(
            content,
            title=title,
            border_style="red",
            title_align="left",
        )
    if test.status == "segfault":
        msg = test.output.strip() if test.output.strip() else ""
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
    if test.status == "timeout":
        return Panel(
            test.output.strip()
            if test.output.strip()
            else "Test exceeded timeout limit",
            title="Timeout",
            border_style="yellow",
            title_align="left",
        )
    content = test.output.strip() if test.output.strip() else "(no output)"
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
        is_done = state.suite_done(suite)
        guide_style = state.suite_guide_style(suite)

        if is_done:
            total_ms = state.suite_total_ms(suite) or 0
            label = f"{suite} ── [{total_ms:.0f}ms]"
        else:
            s_start = state.suite_start_time(suite) or state.grand_start_time or now
            elapsed_ms = (now - s_start) * 1000
            label = f"{suite} {state.spinner} ── [{elapsed_ms:.0f}ms]"

        tree = Tree(label, guide_style=guide_style)

        max_name_len = max(
            (
                len(state.tests.get(tid).test_name)
                if state.tests.get(tid)
                else len(tid)
                for tid in test_ids
            ),
            default=0,
        )

        for tid in test_ids:
            test = state.tests.get(tid)
            name = test.test_name if test else tid
            padded = name.ljust(max_name_len)
            if test is None or test.status in RUNNABLE_STATUSES:
                tree.add(Text(f"{state.spinner} {padded}  pending", style="yellow"))
            elif test.status == "passed":
                tree.add(Text(f"✓ {padded}  [{test.duration_ms:.0f}ms]", style="green"))
            else:
                if test.status == "compile_fail_cached":
                    fail_node = tree.add(
                        Text(f"✗ {padded}  [cached]", style="bold red")
                    )
                else:
                    fail_node = tree.add(
                        Text(
                            f"✗ {padded}  [{test.duration_ms:.0f}ms]", style="bold red"
                        )
                    )
                fail_node.add(_build_error_panel(test))

        elements.append(tree)

    total_count = len(state.tests)
    total_passed = sum(1 for t in state.tests.values() if t.status == "passed")
    total_failed = sum(
        1
        for t in state.tests.values()
        if t.status not in RUNNABLE_STATUSES and t.status != "passed"
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


def _build_watch_renderable(status_markup, state=None):
    status = Text.from_markup(status_markup)
    if state is None:
        return Group(status)
    return Group(status, build_renderable(state))


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
    {BOLD}--watch{RESET}         Watch src/tests and rerun only impacted tests
    {BOLD}--watch-no-initial{RESET}  Watch without running an initial test pass
    {BOLD}--debounce-ms N{RESET}  Debounce watch events ({GREEN}default: 100{RESET})
    {BOLD}--poll-interval-ms N{RESET}  Poll interval fallback ({GREEN}default: 250{RESET})
    {BOLD}--timeout-ms N{RESET}  Per-test timeout in milliseconds ({GREEN}default: 10000{RESET})
  {BOLD}--help{RESET}           Show this help message

{CYAN}EXAMPLES{RESET}
    python3 run_tests.py                        {DIM}# incremental build + run all{RESET}
    python3 run_tests.py --clean                {DIM}# full rebuild{RESET}
    python3 run_tests.py table_create           {DIM}# run a specific suite{RESET}
    python3 run_tests.py foo_tests              {DIM}# run nested suites under foo_tests{RESET}
    python3 run_tests.py foo_tests/a_tests      {DIM}# run one nested suite path{RESET}
    python3 run_tests.py --max-parallel 5       {DIM}# run tests in parallel{RESET}
    python3 run_tests.py --disable-ccache       {DIM}# skip ccache{RESET}
    python3 run_tests.py --watch                {DIM}# watch and rerun impacted tests{RESET}
    python3 run_tests.py --watch --debounce-ms 500  {DIM}# slower debounce window{RESET}
    python3 run_tests.py --timeout-ms 3000      {DIM}# set per-test timeout to 3s{RESET}
""")


def _run_cycle(
    options,
    suites,
    compile_jobs,
    binaries_by_id,
    do_clean,
    console,
    live=None,
    watch_status_getter=None,
    selected_test_ids=None,
    change_during_run_getter=None,
):
    if not compile_jobs:
        print("No tests found for selected suites.")
        return True, set()

    queued_changes = set()

    project_sources = get_project_sources()
    project_sources_mtime = max(
        (os.path.getmtime(path) for path in project_sources if os.path.exists(path)),
        default=0,
    )

    if do_clean:
        subprocess.run(["make", "-C", BUILD_DIR, "clean"], capture_output=True)
        _save_db(BUILD_DIR, {})

    if _makefile_needs_regen(
        BUILD_DIR, compile_jobs, project_sources, options.use_ccache
    ):
        _generate_makefile(
            BUILD_DIR,
            SRC_DIR,
            project_sources,
            compile_jobs,
            options.use_ccache,
        )

    selected_set = None
    if selected_test_ids is not None:
        selected_set = set(selected_test_ids)

    db = _load_db(BUILD_DIR)
    state = SchedulerState(
        max_parallel=options.max_parallel,
        timeout_ms=options.timeout_ms,
        binaries_by_id=binaries_by_id,
    )
    state.suites_order = list(suites)
    now = time.perf_counter()

    for i, (suite, test_name, test_file, binary) in enumerate(compile_jobs):
        test_id = os.path.basename(binary)
        state.suite_tests.setdefault(suite, []).append(test_id)
        status = "pending_ready"
        if selected_set is not None and test_id not in selected_set:
            cached = db.get(test_id)
            if cached:
                status = cached.get("status", "passed")
            else:
                status = "passed"
        test = TestItem(
            test_id=test_id,
            suite=suite,
            test_name=test_name,
            test_file=test_file,
            binary=binary,
            status=status,
            time_status_changed=now + 0.001 * i,
        )
        if selected_set is not None and test_id not in selected_set:
            cached = db.get(test_id)
            if cached:
                test.duration_ms = cached.get("duration_ms", 0)
                test.output = cached.get("compile_error") or ""
        state.tests[test_id] = test

    if live is not None:
        if watch_status_getter is None:
            live.update(build_renderable(state))
        else:
            live.update(_build_watch_renderable(watch_status_getter(), state))

    cpu_count = os.cpu_count() or 4
    targets = []
    for _, _, _, binary in compile_jobs:
        test_id = os.path.basename(binary)
        if selected_set is not None and test_id not in selected_set:
            continue
        targets.append(test_id)
    if not targets:
        return False, set()
    make_result = subprocess.run(
        ["make", "-C", BUILD_DIR, f"-j{cpu_count}", *targets],
        capture_output=True,
        text=True,
    )

    dep_index = _collect_dependency_index(compile_jobs, project_sources)

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
    if _is_make_noop_block(generic_build_error):
        generic_build_error = "Build failed without actionable compiler diagnostics"

    for suite, test_name, test_file, binary in compile_jobs:
        test_id = os.path.basename(binary)
        deps = dep_index.get(test_id, [])
        test = state.tests[test_id]

        if selected_set is not None and test_id not in selected_set:
            cached = db.get(test_id)
            if cached is not None:
                test.status = cached.get("status", "pending_ready")
                test.duration_ms = cached.get("duration_ms", 0)
                test.output = cached.get("compile_error") or ""
                cached["deps"] = dep_index.get(test_id, cached.get("deps", []))
            continue

        test_source_mtime = (
            os.path.getmtime(test_file) if os.path.exists(test_file) else 0
        )
        binary_mtime = os.path.getmtime(binary) if os.path.exists(binary) else 0
        input_mtime = _max_mtime_for_deps(deps)
        if input_mtime == 0:
            input_mtime = max(test_source_mtime, project_sources_mtime)

        if test_id in make_errors:
            db[test_id] = {
                "status": "compile_fail",
                "duration_ms": 0,
                "compile_error": make_errors[test_id],
                "source_mtime": input_mtime,
                "binary_mtime": None,
                "deps": deps,
            }
            test.status = "compile_fail"
            test.output = make_errors[test_id]
            if os.path.exists(binary):
                os.remove(binary)
            continue

        if build_failed:
            db[test_id] = {
                "status": "compile_fail",
                "duration_ms": 0,
                "compile_error": generic_build_error,
                "source_mtime": input_mtime,
                "binary_mtime": None,
                "deps": deps,
            }
            test.status = "compile_fail"
            test.output = generic_build_error
            if os.path.exists(binary):
                os.remove(binary)
            continue

        if os.path.exists(binary) and input_mtime > binary_mtime:
            entry = db.get(test_id)
            if (
                entry
                and entry.get("compile_error")
                and entry.get("source_mtime") == input_mtime
            ):
                test.status = "compile_fail_cached"
                test.output = entry["compile_error"]
                os.remove(binary)
                continue
            if entry is not None:
                entry["compile_error"] = None
            os.remove(binary)
            test.status = "compile_fail"
            test.output = "Binary is stale (source modified) but no cached error"
            continue

        if not os.path.exists(binary):
            entry = db.get(test_id)
            if entry and entry.get("compile_error"):
                cached_source_mtime = entry.get("source_mtime", 0)
                if input_mtime == cached_source_mtime:
                    test.status = "compile_fail_cached"
                    test.output = entry["compile_error"]
                    continue
                entry["compile_error"] = None
                test.status = "compile_fail"
                test.output = "No binary produced but source differs from cached error"
                continue
            test.status = "compile_fail"
            test.output = "No binary found"
            continue

        db[test_id] = {
            "status": "passed",
            "duration_ms": 0,
            "compile_error": None,
            "source_mtime": input_mtime,
            "binary_mtime": binary_mtime,
            "deps": deps,
        }

    grand_start = time.perf_counter()
    state.grand_start_time = grand_start

    def _render_state():
        if watch_status_getter is None:
            return build_renderable(state)
        return _build_watch_renderable(watch_status_getter(), state)

    owned_live = None
    active_live = live
    if active_live is None:
        owned_live = Live(console=console, refresh_per_second=10, transient=True)
        owned_live.start()
        active_live = owned_live

    try:
        state.state_changed()
        active_live.update(_render_state())

        while not state.all_done:
            state._tick_event.wait(timeout=SPINNER_TICK_SECONDS)
            state._tick_event.clear()

            state.check_timeouts()

            if change_during_run_getter is not None:
                fresh_changes = set(change_during_run_getter() or set())
                if fresh_changes:
                    queued_changes.update(fresh_changes)
                    impacted_now = _resolve_impacted_from_dep_index(
                        fresh_changes,
                        compile_jobs,
                        dep_index,
                    )
                    if selected_set is not None:
                        impacted_now = [
                            test_id
                            for test_id in impacted_now
                            if test_id in selected_set
                        ]
                    state.reprioritize(impacted_now)

            state.state_changed()

            if state._dirty:
                state._dirty = False
                state.advance_spinner()
                active_live.update(_render_state())

        active_live.update(_render_state())
    finally:
        if owned_live is not None:
            owned_live.stop()

    grand_end = time.perf_counter()
    state.grand_ms = (grand_end - grand_start) * 1000
    state.all_done = True

    for suite, test_name, test_file, binary in compile_jobs:
        test_id = os.path.basename(binary)
        if selected_set is not None and test_id not in selected_set:
            continue
        test = state.tests.get(test_id)
        if test and test.status not in ("compile_fail", "compile_fail_cached"):
            if test_id in db:
                db[test_id]["status"] = test.status
                db[test_id]["duration_ms"] = test.duration_ms
                db[test_id]["compile_error"] = None
                db[test_id]["binary_mtime"] = (
                    os.path.getmtime(binary) if os.path.exists(binary) else None
                )
                db[test_id]["deps"] = dep_index.get(
                    test_id, db[test_id].get("deps", [])
                )

    _save_db(BUILD_DIR, db)
    if live is None:
        console.print(build_renderable(state))
    return state.any_failed, queued_changes


def main():
    options = _parse_cli_args(sys.argv)
    suites = _resolve_requested_suites(options.requested_suites)
    if not suites:
        print("No test suites found.")
        sys.exit(1)

    lock_fd = _acquire_lock(BUILD_DIR)
    console = Console()
    observer = None
    any_failed = False
    clean_pending = options.do_clean

    try:
        compile_jobs, binaries_by_id = _build_compile_jobs(suites)
        if not options.watch_mode:
            any_failed, _ = _run_cycle(
                options,
                suites,
                compile_jobs,
                binaries_by_id,
                clean_pending,
                console,
            )
            clean_pending = False
            sys.exit(1 if any_failed else 0)
        change_buffer = _ChangedFilesBuffer()
        observer, backend = _try_start_watchdog(change_buffer)
        polling_snapshot = None
        if observer is None:
            backend = "polling"
            polling_snapshot = _collect_polling_snapshot()

        def _drain_watch_changes():
            nonlocal polling_snapshot
            if backend == "polling":
                changes, polling_snapshot = _poll_snapshot_changes(polling_snapshot)
                return changes
            return change_buffer.drain()

        watch_status = {
            "text": "[cyan]watch[/cyan] waiting for file changes...",
        }
        if backend == "polling":
            watch_status["text"] = (
                "[yellow]watch[/yellow] polling backend active (watchdog unavailable)"
            )
        else:
            watch_status["text"] = "[green]watch[/green] watchdog backend active"

        def _get_watch_status():
            return watch_status["text"]

        with Live(
            console=console, refresh_per_second=10, transient=False
        ) as watch_live:
            queued_watch_changes = set()
            if options.watch_initial:
                watch_status["text"] = (
                    f"{_get_watch_status()} | compiling initial test pass"
                )
                cycle_failed, cycle_changes = _run_cycle(
                    options,
                    suites,
                    compile_jobs,
                    binaries_by_id,
                    clean_pending,
                    console,
                    live=watch_live,
                    watch_status_getter=_get_watch_status,
                    change_during_run_getter=_drain_watch_changes,
                )
                if cycle_failed:
                    any_failed = True
                queued_watch_changes.update(cycle_changes)
                clean_pending = False
            else:
                watch_status["text"] = (
                    f"{_get_watch_status()} | waiting for file changes..."
                )
                watch_live.update(_build_watch_renderable(_get_watch_status()))

            while True:
                if queued_watch_changes:
                    changed = sorted(queued_watch_changes)
                    queued_watch_changes.clear()
                else:
                    changed = _wait_for_changes(
                        _drain_watch_changes,
                        options.debounce_ms,
                        options.poll_interval_ms,
                    )
                    changed = sorted(changed)
                if not changed:
                    continue

                suites = _resolve_requested_suites(options.requested_suites)
                if not suites:
                    watch_status["text"] = (
                        "[yellow]watch[/yellow] no suites found after change"
                    )
                    watch_live.update(_build_watch_renderable(_get_watch_status()))
                    continue

                compile_jobs, binaries_by_id = _build_compile_jobs(suites)
                if not compile_jobs:
                    watch_status["text"] = (
                        "[yellow]watch[/yellow] no tests discovered after change"
                    )
                    watch_live.update(_build_watch_renderable(_get_watch_status()))
                    continue

                db = _load_db(BUILD_DIR)
                impacted_ids = _resolve_impacted_test_ids(changed, compile_jobs, db)
                impacted_set = set(impacted_ids)

                preview = ", ".join(changed[:4])
                if len(changed) > 4:
                    preview += ", ..."
                watch_status["text"] = (
                    "[cyan]watch[/cyan] change detected "
                    f"({preview}) -> compiling {len(impacted_set)} test(s)"
                )

                cycle_failed, cycle_changes = _run_cycle(
                    options,
                    suites,
                    compile_jobs,
                    binaries_by_id,
                    clean_pending,
                    console,
                    live=watch_live,
                    watch_status_getter=_get_watch_status,
                    selected_test_ids=impacted_set,
                    change_during_run_getter=_drain_watch_changes,
                )
                queued_watch_changes.update(cycle_changes)
                clean_pending = False
                if cycle_failed:
                    any_failed = True

    except KeyboardInterrupt:
        print()
        console.print("[cyan]watch[/cyan] stopped")
    finally:
        if observer is not None:
            try:
                observer.stop()
                observer.join(timeout=1)
            except Exception:
                pass
        try:
            fcntl.flock(lock_fd, fcntl.LOCK_UN)
            lock_fd.close()
        except Exception:
            pass

    sys.exit(1 if any_failed else 0)


if __name__ == "__main__":
    main()
