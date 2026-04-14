#!/usr/bin/env python3
"""
Rich TUI test runner for 42_philosophers project.

Compiles project sources once into a static archive, then links each
test against it — reducing compilation from N×S to S+N.
Tests run in parallel with a live animated display.

Usage:
    python3 run_tests.py                          # run all, auto ccache, max 5 parallel
    python3 run_tests.py table_create             # run specific suite
    python3 run_tests.py --disable-ccache         # no ccache
    python3 run_tests.py --max-parallel 10         # 10 parallel tests
    python3 run_tests.py --max-parallel 1          # sequential
"""

import glob
import os
import re
import shutil
import subprocess
import sys
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
from dataclasses import dataclass, field

try:
    from rich.console import Console, Group
    from rich.live import Live
    from rich.text import Text
except ImportError:
    subprocess.check_call(
        [sys.executable, "-m", "pip", "install", "rich"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    print("Installed [rich] package.")
    from rich.console import Console, Group
    from rich.text import Text

SPINNER_FRAMES = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"]

PROJECT_ROOT = os.path.dirname(os.path.abspath(__file__))
SRC_DIR = os.path.join(PROJECT_ROOT, "src")
TESTS_DIR = os.path.join(PROJECT_ROOT, "tests")
BUILD_DIR = os.path.join(PROJECT_ROOT, "test_build")

TERM_WIDTH = 80


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


def build_project_lib(project_sources, build_dir, use_ccache):
    cc = ["ccache", "cc"] if use_ccache else ["cc"]
    obj_dir = os.path.join(build_dir, "obj")
    os.makedirs(obj_dir, exist_ok=True)

    obj_files = []
    for src in project_sources:
        rel = os.path.relpath(src, SRC_DIR)
        obj = os.path.join(obj_dir, os.path.splitext(rel)[0] + ".o")
        os.makedirs(os.path.dirname(obj), exist_ok=True)
        cmd = cc + ["-O0", "-I" + SRC_DIR, "-c", src, "-o", obj]
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            return None, result.stderr
        obj_files.append(obj)

    lib_path = os.path.join(build_dir, "libproject.a")
    cmd = ["ar", "rcs", lib_path] + obj_files
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        return None, result.stderr
    return lib_path, None


def compile_test(test_file, lib_path, output_binary, use_ccache):
    cc = ["ccache", "cc"] if use_ccache else ["cc"]
    cmd = cc + ["-O0", "-I" + SRC_DIR, test_file, lib_path, "-o", output_binary]
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.returncode == 0, result.stderr


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


_ERROR_KEYWORDS = {
    "expected",
    "got",
    "wrong",
    "NULL",
    "returned",
    "error",
    "fail",
    "failed",
    "timeout",
    "segfault",
}

_TOKEN_RE = re.compile(r"""("(?:[^"\\]|\\.)*"|'(?:[^'\\]|\\.)*'|\d+|[a-zA-Z_]\w*)""")

_KEYWORDS_UPPER = {k.upper() for k in _ERROR_KEYWORDS}


def highlight_error(msg: str) -> Text:
    text = Text()
    last_end = 0
    for m in _TOKEN_RE.finditer(msg):
        if m.start() > last_end:
            text.append(msg[last_end : m.start()], style="dim white")
        token = m.group()
        if (token.startswith('"') and token.endswith('"')) or (
            token.startswith("'") and token.endswith("'")
        ):
            text.append(token, style="green")
        elif token.isdigit():
            text.append(token, style="cyan")
        elif token in _ERROR_KEYWORDS or token.upper() in _KEYWORDS_UPPER:
            text.append(token, style="bold yellow")
        else:
            text.append(token, style="dim white")
        last_end = m.end()
    if last_end < len(msg):
        text.append(msg[last_end:], style="dim white")
    return text


@dataclass
class TestResult:
    status: str = "pending"
    duration_ms: float = 0.0
    error_lines: list = field(default_factory=list)


def _header_line(label, is_running, spinner_idx, ms):
    spinner = SPINNER_FRAMES[spinner_idx % len(SPINNER_FRAMES)]
    if is_running:
        inner = f" {label} {spinner} ── [{ms:.0f}ms] "
    else:
        inner = f" {label} ── [{ms:.0f}ms] "
    prefix = "╭─"
    fill = max(0, TERM_WIDTH - len(prefix) - len(inner))
    return prefix + inner + "─" * fill


def _footer_line():
    return "╰" + "─" * max(0, TERM_WIDTH - 1)


class TestDisplay:
    def __init__(self):
        self.results = {}
        self.spinner_idx = 0
        self.suite_order = []
        self.suite_test_order = {}
        self.suite_total_ms = {}
        self.suite_done = set()
        self.suite_start_times = {}
        self.grand_ms = 0.0
        self.grand_start_time = None
        self.all_done = False

    def advance_spinner(self):
        self.spinner_idx = (self.spinner_idx + 1) % len(SPINNER_FRAMES)

    def _suite_color(self, suite):
        if suite in self.suite_done:
            has_fail = any(
                self.results.get(tn) is not None
                and self.results[tn].status not in ("pending", "running", "passed")
                for tn in self.suite_test_order.get(suite, [])
            )
            return "red" if has_fail else "green"
        return "cyan"

    def render(self):
        elements = []
        now = time.perf_counter()

        for suite in self.suite_order:
            test_names = self.suite_test_order.get(suite, [])
            is_running = suite not in self.suite_done
            color = self._suite_color(suite)

            if is_running:
                start_time = self.suite_start_times.get(suite, now)
                elapsed_ms = (now - start_time) * 1000
                header = _header_line(suite, True, self.spinner_idx, elapsed_ms)
            else:
                total_ms = self.suite_total_ms.get(suite, 0)
                header = _header_line(suite, False, self.spinner_idx, total_ms)
            elements.append(Text(header, style=color))
            elements.append(Text("│", style=color))

            max_name_len = max((len(tn) for tn in test_names), default=0)
            total_tests = len(test_names)
            spinner_char = SPINNER_FRAMES[self.spinner_idx % len(SPINNER_FRAMES)]

            for idx, tn in enumerate(test_names):
                is_last = idx == total_tests - 1
                tree = "╰─" if is_last else "├─"
                res = self.results.get(tn)

                if res is None or res.status in ("pending", "running"):
                    line = Text()
                    line.append("│  ", style=color)
                    line.append(tree + " ", style="yellow")
                    line.append(spinner_char + " ", style="yellow")
                    line.append(tn, style="yellow")
                    line.append("  running...", style="dim yellow")
                    elements.append(line)
                elif res.status == "passed":
                    line = Text()
                    line.append("│  ", style=color)
                    line.append(tree + " ", style="green")
                    line.append("✓ ", style="bold green")
                    line.append(tn.ljust(max_name_len), style="green")
                    line.append(f"  [{res.duration_ms:.0f}ms]", style="dim green")
                    elements.append(line)
                else:
                    label = res.status
                    if label == "compile_fail":
                        label = "compile fail"
                    line = Text()
                    line.append("│  ", style=color)
                    line.append(tree + " ", style="red")
                    line.append("✗ ", style="bold red")
                    line.append(tn.ljust(max_name_len), style="red")
                    line.append(f"  [{res.duration_ms:.0f}ms]", style="dim red")
                    elements.append(line)
                    for err_text in res.error_lines:
                        err_line = Text()
                        err_line.append("│       ", style=color)
                        err_line.append("→ ", style="dim yellow")
                        err_line.append(highlight_error(err_text))
                        elements.append(err_line)

            elements.append(Text("│", style=color))
            elements.append(Text(_footer_line(), style=color))

        total_passed = sum(1 for r in self.results.values() if r.status == "passed")
        total_count = len(self.results)
        total_failed = sum(
            1
            for r in self.results.values()
            if r.status not in ("pending", "running", "passed")
        )

        if self.grand_start_time is not None and not self.all_done:
            results_ms = (now - self.grand_start_time) * 1000
        else:
            results_ms = self.grand_ms

        results_inner = f" Results ── [{results_ms:.0f}ms] "
        prefix = "╭─"
        fill = max(0, TERM_WIDTH - len(prefix) - len(results_inner))
        results_header = prefix + results_inner + "─" * fill
        elements.append(Text(results_header, style="bold white"))
        elements.append(Text("│", style="bold white"))

        if total_count > 0:
            passed_line = Text()
            passed_line.append("│  ", style="bold white")
            passed_line.append(
                f"{total_passed}/{total_count} passed", style="bold green"
            )
            elements.append(passed_line)
            if total_failed > 0:
                fail_line = Text()
                fail_line.append("│  ", style="bold white")
                fail_line.append("✗ ", style="bold red")
                fail_line.append(f"{total_failed} failed", style="bold red")
                elements.append(fail_line)

        elements.append(Text("│", style="bold white"))
        elements.append(Text(_footer_line(), style="bold white"))

        return Group(*elements)


def main():
    global TERM_WIDTH

    max_parallel = 5
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
    os.makedirs(BUILD_DIR, exist_ok=True)

    console = Console()
    display = TestDisplay()

    try:
        TERM_WIDTH = os.get_terminal_size().columns
    except OSError:
        TERM_WIDTH = 80
    TERM_WIDTH = min(TERM_WIDTH, 100)

    lib_path, lib_error = build_project_lib(project_sources, BUILD_DIR, use_ccache)
    if lib_path is None:
        console.print("[red]Failed to build project library:[/red]")
        if lib_error and lib_error.strip():
            for line in lib_error.strip().split("\n"):
                console.print(f"  [yellow]→ {line}[/yellow]")
        shutil.rmtree(BUILD_DIR, ignore_errors=True)
        sys.exit(1)

    compile_jobs = []
    for suite in suites:
        tests = discover_tests(suite)
        for test_file in tests:
            test_name = os.path.splitext(os.path.basename(test_file))[0]
            binary = os.path.join(BUILD_DIR, f"{suite}_{test_name}")
            compile_jobs.append((suite, test_name, test_file, binary))
            display.results[test_name] = TestResult(status="pending")
            display.suite_test_order.setdefault(suite, []).append(test_name)
    display.suite_order = suites

    cpu_count = os.cpu_count() or 4
    compile_results = {}
    with ProcessPoolExecutor(max_workers=cpu_count) as executor:
        futures = {}
        for suite, test_name, test_file, binary in compile_jobs:
            future = executor.submit(
                compile_test, test_file, lib_path, binary, use_ccache
            )
            futures[future] = (suite, test_name, binary)
        for future in futures:
            suite, test_name, binary = futures[future]
            ok, stderr = future.result()
            compile_results[(suite, test_name)] = (ok, stderr, binary)

    any_failed = False
    grand_start = time.perf_counter()
    display.grand_start_time = grand_start

    with Live(console=console, refresh_per_second=10, transient=True) as live:
        live.update(display.render())

        for suite in suites:
            tests = discover_tests(suite)
            if not tests:
                display.suite_total_ms[suite] = 0
                display.suite_done.add(suite)
                live.update(display.render())
                continue

            test_items = []
            for test_file in tests:
                test_name = os.path.splitext(os.path.basename(test_file))[0]
                ok, stderr, binary = compile_results.get(
                    (suite, test_name), (False, "not compiled", "")
                )
                test_items.append((test_name, ok, stderr, binary))

            suite_start = time.perf_counter()
            display.suite_start_times[suite] = suite_start

            run_futures = {}
            with ProcessPoolExecutor(max_workers=max_parallel) as executor:
                for test_name, ok, stderr, binary in test_items:
                    if not ok:
                        res = TestResult(status="compile_fail", duration_ms=0)
                        res.error_lines = (
                            stderr.strip().split("\n") if stderr.strip() else []
                        )
                        display.results[test_name] = res
                        any_failed = True
                        live.update(display.render())
                        continue

                    display.results[test_name].status = "running"
                    display.advance_spinner()
                    live.update(display.render())
                    future = executor.submit(run_single_test, binary)
                    run_futures[future] = test_name

                for future in as_completed(run_futures):
                    test_name = run_futures[future]
                    rc, output, duration_ms, segfault, timeout = future.result()

                    if timeout:
                        res = TestResult(status="timeout", duration_ms=duration_ms)
                        res.error_lines = ["test timed out (10s limit)"]
                    elif segfault or rc == 139:
                        res = TestResult(status="segfault", duration_ms=duration_ms)
                        res.error_lines = (
                            output.strip().split("\n") if output.strip() else []
                        )
                    elif rc != 0:
                        res = TestResult(status="failed", duration_ms=duration_ms)
                        res.error_lines = (
                            output.strip().split("\n") if output.strip() else []
                        )
                    else:
                        res = TestResult(status="passed", duration_ms=duration_ms)

                    display.results[test_name] = res
                    display.advance_spinner()
                    if res.status != "passed":
                        any_failed = True
                    live.update(display.render())

            suite_end = time.perf_counter()
            display.suite_total_ms[suite] = (suite_end - suite_start) * 1000
            display.suite_done.add(suite)
            live.update(display.render())

    grand_end = time.perf_counter()
    display.grand_ms = (grand_end - grand_start) * 1000
    display.all_done = True

    console.print(display.render())
    shutil.rmtree(BUILD_DIR, ignore_errors=True)
    sys.exit(1 if any_failed else 0)


if __name__ == "__main__":
    main()
