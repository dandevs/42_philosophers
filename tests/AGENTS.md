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
	t_table		table;
	t_config	config;

	config = (t_config){0};
	config.philo_count = 5;
	config.time_to_die_ms = 800;
	config.time_to_eat_ms = 200;
	config.time_to_sleep_ms = 200;
	if (!table_create(&table, config))
	{
		printf("table_create returned 0");
		return (1);
	}
	if (table.config.philo_count != 5)
	{
		printf("expected count 5, got %d", table.config.philo_count);
		return (1);
	}
	table_free(&table);
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
- **Don't print on success.** Silent = passing. Only print the failure reason when returning non-zero.