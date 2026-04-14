#!/usr/bin/env python3
"""
Test runner for 42_philosophers project.

Discovers test suites under tests/, compiles each test in parallel,
then runs them sequentially with colored output.

Usage:
    python3 run_tests.py              # run all tests
    python3 run_tests.py table_create # run only the table_create suite
"""

import glob
import os
import shutil
import subprocess
import sys
from concurrent.futures import ProcessPoolExecutor

RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
CYAN = "\033[36m"
BOLD = "\033[1m"
RESET = "\033[0m"

PROJECT_ROOT = os.path.dirname(os.path.abspath(__file__))
SRC_DIR = os.path.join(PROJECT_ROOT, "src")
TESTS_DIR = os.path.join(PROJECT_ROOT, "tests")
BUILD_DIR = os.path.join(PROJECT_ROOT, "test_build")

CC = "cc"
CFLAGS = ["-I" + SRC_DIR]


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


def compile_test(test_file, project_sources, output_binary):
    cmd = [CC] + CFLAGS + [test_file] + project_sources + ["-o", output_binary]
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.returncode == 0, result.stderr


def run_test(binary_path):
    try:
        result = subprocess.run(
            [binary_path], capture_output=True, text=True, timeout=10
        )
        return result.returncode, result.stdout, False, False
    except subprocess.TimeoutExpired:
        return -1, "", False, True
    except Exception:
        return 139, "", True, False


def main():
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

    compile_jobs = []
    for suite in suites:
        tests = discover_tests(suite)
        for test_file in tests:
            test_name = os.path.splitext(os.path.basename(test_file))[0]
            binary = os.path.join(BUILD_DIR, f"{suite}_{test_name}")
            compile_jobs.append((suite, test_name, test_file, binary))

    cpu_count = os.cpu_count() or 4
    compile_results = {}
    with ProcessPoolExecutor(max_workers=cpu_count) as executor:
        futures = {}
        for suite, test_name, test_file, binary in compile_jobs:
            future = executor.submit(compile_test, test_file, project_sources, binary)
            futures[future] = (suite, test_name, binary)

        for future in futures:
            suite, test_name, binary = futures[future]
            ok, stderr = future.result()
            compile_results[(suite, test_name)] = (ok, stderr, binary)

    total = 0
    passed = 0
    fail_count = 0

    for suite in suites:
        tests = discover_tests(suite)
        if not tests:
            continue

        print(f"\n  {BOLD}{CYAN}═══ {suite} ═══{RESET}\n")

        for test_file in tests:
            test_name = os.path.splitext(os.path.basename(test_file))[0]
            total += 1

            ok, stderr, binary = compile_results.get(
                (suite, test_name), (False, "not compiled", "")
            )

            if not ok:
                print(f"  {RED}✗ {test_name}   COMPILE FAIL{RESET}")
                if stderr.strip():
                    for line in stderr.strip().split("\n"):
                        print(f"      {YELLOW}→ {line}{RESET}")
                fail_count += 1
                continue

            rc, stdout, segfault, timeout = run_test(binary)

            if timeout:
                print(f"  {YELLOW}✗ {test_name}   TIMEOUT{RESET}")
                fail_count += 1
            elif segfault or rc == 139:
                print(f"  {RED}✗ {test_name}   SEGFAULT{RESET}")
                if stdout.strip():
                    for line in stdout.strip().split("\n"):
                        print(f"      {YELLOW}→ {line}{RESET}")
                fail_count += 1
            elif rc != 0:
                print(f"  {RED}✗ {test_name}   FAIL{RESET}")
                if stdout.strip():
                    for line in stdout.strip().split("\n"):
                        print(f"      {YELLOW}→ {line}{RESET}")
                fail_count += 1
            else:
                print(f"  {GREEN}✓ {test_name}   PASS{RESET}")
                passed += 1

    shutil.rmtree(BUILD_DIR, ignore_errors=True)

    print(f"\n  {BOLD}{CYAN}═══ Results ═══{RESET}\n")
    print(f"  {GREEN}{passed}/{total} passed{RESET}")
    if fail_count > 0:
        print(f"  {RED}✗ {fail_count} failed{RESET}")
        sys.exit(1)
    else:
        print(f"\n  {GREEN}All tests passed!{RESET}")
        sys.exit(0)


if __name__ == "__main__":
    main()
