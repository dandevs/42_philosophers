## Build

```
make          # build philo
make debug    # build with -g -O0
make fclean   # clean objects + executable
```

- **CFLAGS strict flags are commented out** in Makefile (`-Wall -Wextra -Werror -pthread`). Uncomment before evaluation.
- `CC = cc`. Sources auto-discovered: any `.c`/`.h` in `src/` subtree.
- Required targets: `$(NAME)`, `all`, `clean`, `fclean`, `re`.

## Testing

Custom test runner: `ctester`

### Test Structure

Each test is a standalone `.c` file with its own `main`:
- Include headers via `src/` path: `#include "lib.h"`, `#include "table/table.h"`
- Return `0` on success (silent), non-zero + `fprintf(stderr, ...)` on failure.

### Adding Tests
1. `mkdir tests/my_suite`
2. Add `.c` files: `tests/my_suite/test_basic.c`
3. Run: `ctester`

## Architecture

### Data Structures (`src/lib.h`)

- **`t_lock`** (`src/lock.h`): Mutex wrapper with `locked` flag. Functions: `lock_init`, `lock_destroy`, `lock_lock`, `lock_unlock`. Forks are `t_lock *`. `lock_unlock` checks `locked` flag before unlocking (safe for cleanup when already unlocked).
- **`t_config`**: Holds `philo_count`, `time_to_die_ms`, `time_to_eat_ms`, `time_to_sleep_ms`, `meals_required` (or -1).
- **`t_philosopher`**: `fork_left`/`fork_right` (`t_lock *`), `mutex` (`pthread_mutex_t *`), `table` (`t_table *`), `thread`, `index`, `alive`, `eat_count`, `done`, `time_last_meal`, `time_began_eating`, `time_began_sleep`.
- **`t_table`**: `philosophers` array, `forks` (`pthread_mutex_t *`), `printf_mutex`, `mutex`, `config`, `start_time`, `alive`, `threads_created`.

### Key Patterns

- **Typed mutex get/set** (`src/mutex_utils.h`):
  - `set_int(mutex, &field, val)` / `get_int(mutex, &field)` — for `int` fields.
  - `set_ulong(mutex, &field, val)` / `get_ulong(mutex, &field)` — for `unsigned long` fields.
  - Type-safe: no `(void*)` casts, no size mismatch (unlike the old `set_with_mutex` which wrote `sizeof(void*)` bytes into `int` fields, corrupting adjacent memory on 64-bit).
- **Philosopher mutex helpers** (`src/philosopher/utils.h`):
  - `with_philo_mutex(philo, func)` — lock philo's mutex, call func, unlock.
  - `mutex_philo_table_lock(philo)` / `mutex_philo_table_unlock(philo)` — lock/unlock both philo mutex and table mutex (for atomic reads of table state + philo state).
  - `mutex_forks_lock(philo)` / `mutex_forks_unlock(philo)` — lock/unlock both forks via `t_lock`.
  - `philo_mutexes_unlock(philo)` — unlock both forks (no lock check).
- **Generic array iterators** (`src/utils.c`):
  - `for_each(arr, len, func)` — call `func` on each element.
  - `all(arr, len, predicate)` — return 1 if all elements satisfy predicate.
- `POLLING_RATE` = 100 (microseconds) defined in `lib.h`.

### Philosopher Lifecycle

- Philosopher threads run polling loop: acquire philo + table mutex via `mutex_philo_table_lock(philo)`, sleep `POLLING_RATE`.
- Philosopher fields (`alive`, `eat_count`, `done`, `time_last_meal`, `time_began_eating`, `time_began_sleep`) protected by per-philosopher `mutex`.
- `philo_init` — sets `index`, `alive = 1`, `eat_count = 0`, `done = 0`, inits mutex.
- `philo_init_time` — sets all time fields to `get_time_ms()`.

### Table Lifecycle

- `table_create` — allocates philosophers, calls `philo_init` for each.
- `table_main_routine` — calls `philo_init_time` (sets start time for all philos).
- **Duplicate `table_create`** in `table_utils.c` (incomplete, `init_forks` stub) — will conflict with `table_create.c`.

### Current State (WIP)

- `main.c` returns 0 — simulation not wired up.
- `table_main_routine.c` only calls `philo_init_time` — no monitor loop, no thread creation.
- `philo_main_routine.c` has a bare polling loop — no eat/sleep/think logic.
- `table_create` duplicated in `table_create.c` and `table_utils.c` — linker conflict.

## 42 Constraints

- **Norm**: 25 lines/func, no `for`, vars at top of scope, `while` loops.
- **No globals**, **no external libs**, **no atomics**.
- **Allowed (mandatory)**: `memset`, `printf`, `malloc`, `free`, `write`, `usleep`, `gettimeofday`, `pthread_create`, `pthread_detach`, `pthread_join`, `pthread_mutex_init`, `pthread_mutex_destroy`, `pthread_mutex_lock`, `pthread_mutex_unlock`
- **Allowed (bonus)**: above + `fork`, `kill`, `exit`, `waitpid`, `sem_open`, `sem_close`, `sem_post`, `sem_wait`, `sem_unlink`
- **Norm violations in current code**: `table_create.c` uses `for` loop; `philo_main_routine.c` has mid-scope var decl.

## Program Specification

### Arguments
```
./philo philo_count time_to_die time_to_eat time_to_sleep [meals_required]
```
`meals_required = -1` when unset.

### Log Format (ms relative to sim start, 1-indexed philo numbers)
```
timestamp X has taken a fork
timestamp X is eating
timestamp X is sleeping
timestamp X is thinking
timestamp X died
```
Death is the **last printed message**. No output after death. Death within 10ms.

### Mandatory vs Bonus
- **Mandatory**: threads + mutexes in `philo/`.
- **Bonus**: processes + semaphores in `philo_bonus/`, files named `*_bonus.{c/h}`.

## Critical Gotchas

### Timing
- `usleep()` takes µs: `usleep(200 * 1000)` = 200ms. Polling loop for precise waits: `usleep(POLLING_RATE)` + time check. Polling needed for eating and sleeping (death can interrupt).
- Death detection ≤ 10ms. Monitor polls every `POLLING_RATE` (100µs).
- `gettimeofday()` → ms: `tv.tv_sec * 1000 + tv.tv_usec / 1000`.

### Deadlock Prevention
- Asymmetric fork pickup: even IDs grab left→right, odd IDs grab right→left → avoids circular wait.
- Stagger start: even philos `usleep(1000)` before first action.
- Single philo (1 fork): no threads — print "taken fork", polling sleep `time_to_die`, print "died".

### Data Races
- Every shared variable written/read across threads must be mutex-protected.
- `eat_count` / `time_last_meal` / `alive` / `done`: per-philosopher mutex.
- `table->alive`: table mutex.
- All `printf` output: shared print mutex. Death message prints even after death flag set.
- `volatile` does NOT fix data races — only mutexes do.

### Cleanup Order
- `pthread_join` ALL threads before destroying any mutex.
- Every `pthread_mutex_init` → matching `pthread_mutex_destroy`.
- Bonus: `sem_unlink` before `sem_open`, `waitpid` all children.
- Dying philosopher holding fork mutexes must unlock them so neighbors aren't stuck on `pthread_mutex_lock` (which would hang `pthread_join`).

### Key Evaluator Test Cases
```
./philo 1 800 200 200          # dies, 1 "taken fork" + death msg
./philo 4 310 200 100          # dies (time_to_die < eat + sleep)
./philo 5 800 200 200 5        # all eat exactly 5 times, clean stop
./philo 200 800 200 200        # stress: no deadlock, no crash
```

## Submission Structure
- Mandatory: `philo/` (current repo root).
- Bonus: `philo_bonus/`.
- Root `README.md` required: italicized first line (`*...by <login>*`), Description, Instructions, Resources.

## TEMPORARY — Remove Before Evaluation

- `atoi()` in `src/utils.c:parse_argument()` must be replaced with custom implementation.
- Remove `for` loop in `table_create.c` before eval (norm violation).
- Fix duplicate `table_create` definition (`table_create.c` vs `table_utils.c`).
- Fix mid-scope variable declarations (`philo_main_routine.c:23`).
- Remove `#include <stdarg.h>` in `src/utils.c` (unused, not allowed).
