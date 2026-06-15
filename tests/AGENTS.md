# Writing Tests for `ctester`

## Test Structure

Each test is a single `.c` file with its own `main`:

- **Include** project headers via the `src/` path: `#include "lib.h"`, `#include "table/table.h"`, etc.
- **Include** `#include "ctest.h"` for assertion macros.
- **On success**: `return (0)`, print nothing.
- **On failure**: use `ASSERT_*` / `EXPECT_*` macros from `ctest.h` — they print a structured failure line and `return (1)`.

```c
#include "lib.h"
#include "table/table.h"
#include "ctest.h"

int	main(void)
{
	t_table		table;
	t_config	config;

	config = (t_config){0};
	config.philo_count = 5;
	config.time_to_die_ms = 800;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 200;
	ASSERT_TRUE(table_create(&table, config));
	ASSERT_EQ(table.config.philo_count, 5);
	table_free(&table);
	return (0);
}
```

### Available Macros (defined in `tests/ctest.h`)

**Fatal — return 1 immediately on failure:**
`ASSERT_TRUE`, `ASSERT_FALSE`, `ASSERT_EQ`, `ASSERT_NE`, `ASSERT_GT`, `ASSERT_GE`, `ASSERT_LT`, `ASSERT_LE`, `ASSERT_STREQ`, `ASSERT_STRNE`, `ASSERT_NULL`, `ASSERT_NOT_NULL`, `TEST_FAIL`

**Soft — record failure and continue (end main with `return TEST_RESULT()`):**
`EXPECT_TRUE`, `EXPECT_FALSE`, `EXPECT_EQ`, `EXPECT_NE`, `EXPECT_STREQ`

### Key Rules
- Return `0` = pass, non-zero = fail.
- No manual `printf` on failure — use the assertion macros.
- Each test is a standalone executable — define your own `main`.
- Tests using `dup`/`freopen` to suppress stdout (e.g. integration tests) still need `#include <stdio.h>` alongside `ctest.h`.

## Directory Layout

```
tests/
├── AGENTS.md              # This file
├── ctest.h                # Assertion header
└── suite_name/            # One directory per test suite
    ├── behavior_a.c
    ├── behavior_b.c
    └── ...
```

- A **suite** is any subdirectory of `tests/`. The directory name becomes the suite name.
- Every `.c` file inside a suite directory is a standalone test.
- The runner discovers suites and tests automatically — no registration needed.

## Adding a New Suite

1. Create a directory under `tests/`: `mkdir tests/my_feature`
2. Add one or more `.c` test files: `tests/my_feature/basic.c`
3. Run: `ctester`

No other setup required.

## Best Practices

- **One test per behavior.** Don't assert multiple unrelated things in one file.
- **Keep tests focused.** A failing test should point to exactly one problem.
- **Clean up resources.** Call destroy/free functions so the test doesn't leak. Leaks can mask real bugs.
- **Test edge cases.** Cover 0, 1, 2, and large values explicitly.
- **Don't print on success.** Silent = passing. The assertion macros handle failure output.