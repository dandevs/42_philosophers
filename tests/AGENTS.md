# Writing Tests for run_tests.py

## Test Structure

Each test is a single `.c` file with its own `main`:

- **Include** project headers via the `src/` path: `#include "lib.h"`, `#include "table/table.h"`, etc.
- **On success**: return `0`, print nothing.
- **On failure**: return non-zero (typically `1`) and print only the error message to stdout. No prefix — the runner adds formatting.

```c
#include "lib.h"
#include "table/table.h"
#include <stdlib.h>
#include <stdio.h>

int	main(void)
{
	t_table	table;

	if (!table_create(&table, 5))
	{
		printf("table_create returned 0");
		return (1);
	}
	if (table.count != 5)
	{
		printf("expected count 5, got %d", table.count);
		return (1);
	}
	table_destroy(&table);
	return (0);
}
```

Key rules:
- Return `0` = pass, non-zero = fail.
- Print only on failure. No "FAIL:", "Error:", or similar prefixes.
- Each test is a standalone executable — define your own `main`.

## Directory Layout

```
tests/
├── AGENTS.md              # This file
└── suite_name/            # One directory per test suite
    ├── test_behavior_a.c
    ├── test_behavior_b.c
    └── ...
```

- A **suite** is any subdirectory of `tests/`. The directory name becomes the suite name.
- Every `.c` file inside a suite directory is a standalone test.
- The runner discovers suites and tests automatically — no registration needed.

## Compilation

Project sources are compiled once into a static archive (`libproject.a`), then each test links against it:

```
# Once (shared):
cc -O0 -Isrc/ -c src/utils.c -o obj/utils.o
cc -O0 -Isrc/ -c src/table/table.c -o obj/table/table.o
ar rcs libproject.a obj/utils.o obj/table/table.o

# Per test:
cc -O0 -Isrc/ test_file.c libproject.a -o binary
```

- All `.c` files under `src/` are included **except** `src/main.c` (avoiding duplicate `main`).
- Include path is `src/`, so use `#include "lib.h"` etc.
- `ccache` is auto-detected and used if available.
- If a test fails to compile, the runner reports `COMPILE FAIL` with compiler output.

## Running Tests

```sh
python3 run_tests.py                        # run all suites (auto-detects ccache)
python3 run_tests.py suite_name             # run one suite
python3 run_tests.py suite_a suite_b        # run multiple suites
python3 run_tests.py --disable-ccache       # force disable ccache
python3 run_tests.py --max-parallel 1       # sequential execution
python3 run_tests.py --max-parallel 10      # up to 10 tests in parallel
```

- Project sources compile once into a static library.
- Test compilation runs in parallel across CPU cores.
- Test execution runs in parallel (default: 5 concurrent, configurable via `--max-parallel`).
- Each test has a 10-second timeout.
- Runner exit code: `0` if all pass, `1` if any fail.
- Requires the `rich` Python package (auto-installed if missing).

## Output

| Symbol | Meaning | Color |
|--------|---------|-------|
| `⠋ test_name` | Test running (animated spinner) | Yellow |
| `✓ test_name  [Nms]` | Test passed | Green |
| `✗ test_name  [Nms]` | Test failed; stdout shown with `→` prefix | Red |
| `✗ test_name  COMPILE FAIL` | Compilation failed; compiler errors shown | Red |
| `✗ test_name  SEGFAULT` | Crashed (signal 11 / exit 139) | Red |
| `✗ test_name  TIMEOUT` | Exceeded 10s limit | Yellow |

## Adding a New Suite

1. Create a directory under `tests/`: `mkdir tests/my_feature`
2. Add one or more `.c` test files: `tests/my_feature/test_basic.c`
3. Run: `python3 run_tests.py my_feature`

No other setup required.

## Best Practices

- **One test per behavior.** Don't assert multiple unrelated things in one file.
- **Keep tests focused.** A failing test should point to exactly one problem.
- **Clean up resources.** Call destroy/free functions so the test doesn't leak. Leaks can mask real bugs.
- **Test edge cases.** Cover 0, 1, 2, and large values explicitly.
- **Don't print on success.** Silent = passing. Only print the failure reason when returning non-zero.
- **Follow 42 Norm where practical.** No `for` loops, declarations at top of scope, functions ≤ 25 lines, no unused includes.
