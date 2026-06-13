> **Keep me updated!** After any significant code changes (new modules, renamed functions, changed patterns), update this file so agents stay aligned with the actual codebase.

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

- **`t_lock`** (`src/lock.h`): Mutex wrapper with `locked` flag. Functions: `lock_init`, `lock_destroy`, `lock_lock`, `lock_unlock`. `lock_unlock` checks `locked` flag before unlocking (safe for cleanup when already unlocked).
- **`t_config`**: Holds `philo_count`, `time_to_die_ms`, `time_to_eat_ms`, `time_to_sleep_ms`, `time_to_think_ms`, `meals_required` (or -1).
- **`t_philosopher`**: `fork_left`/`fork_right` (`pthread_mutex_t *` — shared pointers into `forks[]`), `mutex` (`pthread_mutex_t` — embedded per-philo), `table` (`t_table *`), `thread`, `index`, `alive`, `eat_count`, `done`, `time_last_meal`, `time_began_eating`, `time_began_sleep`, `start_time`.
- **`t_table`**: `philosophers` array, `forks` (`pthread_mutex_t *` — `malloc`'d array), `printf_mutex` (`pthread_mutex_t` — embedded), `mutex` (`pthread_mutex_t` — embedded), `config`, `start_time`, `alive`, `threads_created`.

### Key Patterns

- **Typed mutex get/set** (`src/mutex_utils.h`):
  - `m_set_int(mutex, &field, val)` / `m_get_int(mutex, &field)` — for `int` fields.
  - `m_set_ulong(mutex, &field, val)` / `m_get_ulong(mutex, &field)` — for `unsigned long` fields.
  - Type-safe: no `(void*)` casts, no size mismatch (unlike the old `set_with_mutex` which wrote `sizeof(void*)` bytes into `int` fields, corrupting adjacent memory on 64-bit).
- **Lock/unlock helpers** (`src/philosopher/utils.h`):
  - `with_philo_mutex(philo, func)` — lock philo's mutex, call func, unlock.
  - `mutex_philo_table_lock(philo)` / `mutex_philo_table_unlock(philo)` — lock/unlock both philo mutex and table mutex (for atomic reads of table state + philo state).
  - `mutex_forks_lock(philo)` / `mutex_forks_unlock(philo)` — lock/unlock both forks via `t_lock`.
  - `philo_mutexes_unlock(philo)` — unlock both forks (no lock check).
- **Generic array iterators** (`src/utils.c`):
  - `for_each(arr, len, func)` — call `func` on each element.
  - `all(arr, len, predicate)` — return 1 if all elements satisfy predicate.
- **`philo_log(philo, msg)`** (`src/utils.c`): Prints `<timestamp> <philo_id> <msg>\n` via printf mutex. Renamed from `log` to avoid `math.h` name conflict.
- `POLLING_RATE` = 200 (microseconds) defined in `lib.h`.

### Mutex Embedding Rule

Per-object mutexes (`philo->mutex`, `table->mutex`, `table->printf_mutex`) are **embedded** (`pthread_mutex_t`, not pointer) — no separate malloc, accessed via `&philo->mutex`.

Shared fork mutexes MUST remain pointers (`pthread_mutex_t *fork_left/right` in philo, `pthread_mutex_t *forks` in table) because adjacent philosophers share fork references, and the fork array is dynamically sized.

### Philosopher Lifecycle

- Each philosopher runs in a detached thread (`pthread_detach` — no join needed).
- Routine in `src/philosopher/philo_main_routine.c`:
  - Loop: `lock_forks()` (asymmetric pickup) → record `time_began_eating` → print "eating" (via `philo_log`) → `unlock_forks()` → record `time_last_meal` → print "sleeping" → `usleep(time_to_sleep)` → print "thinking" → `usleep(time_to_think)` → repeat.
  - No `check_death()` in philo routine — death detection is handled solely by the monitor (main thread).
- Philosopher fields (`alive`, `eat_count`, `done`, `time_last_meal`, `time_began_eating`, `time_began_sleep`) protected by per-philosopher `mutex`.
- `philo_init` — sets `index`, `alive = 1`, `eat_count = 0`, `done = 0`, inits mutex.
- `philo_init_time` — sets all time fields to `get_time_ms()`.

### Table Lifecycle

- `table_create` (`src/table/table_create.c`):
  - `malloc` philosophers array, `philo_init` each.
  - `pthread_mutex_init` for `table->printf_mutex` and `table->mutex`.
  - Sets `table->alive = 1` and `table->config`.
- `table_main_routine` — calls `philo_init_time` (sets start time for all philos). To be expanded: create philo threads (detached), then run monitor loop detecting death or `meals_required` completion.
- **Duplicate `table_create`** in `table_utils.c` (incomplete, `init_forks` stub) — will conflict with `table_create.c`.

### Monitoring Architecture

- **Main thread runs monitor** (no separate monitor thread).
- Monitor polls all philosophers at `POLLING_RATE` (200µs) checking `time_last_meal` vs `time_to_die`.
- Death detected → prints "died" via `philo_log` → `main()` returns → process exits, OS kills all detached threads.
- Clean termination for `meals_required` similarly: monitor detects all ate enough → clean exit.
- Since `main()` returning terminates the entire process, there is no need for philosophers to self-check death or release forks on death. Polling not needed in eat/sleep — blocking `usleep()` is fine.

### Current State (WIP)

- `main.c` returns 0 — simulation not wired up.
- `table_main_routine.c` only calls `philo_init_time` — no philo thread creation, no monitor loop.
- `philo_main_routine.c` has eat/sleep/think loop with fork lock/unlock but no monitor interaction.
- `table_create` duplicated in `table_create.c` and `table_utils.c` — linker conflict.
- `parse_argument` type mismatch error (passing `unsigned long *` to `int *`) in `src/utils.c`.

## 42 Constraints

- **Norm**: 25 lines/func, no `for`, vars at top of scope, `while` loops.
- **No globals**, **no external libs**, **no atomics**.
- **Allowed (mandatory)**: `memset`, `printf`, `malloc`, `free`, `write`, `usleep`, `gettimeofday`, `pthread_create`, `pthread_detach`, `pthread_join`, `pthread_mutex_init`, `pthread_mutex_destroy`, `pthread_mutex_lock`, `pthread_mutex_unlock`
- **Allowed (bonus)**: above + `fork`, `kill`, `exit`, `waitpid`, `sem_open`, `sem_close`, `sem_post`, `sem_wait`, `sem_unlink`
- **Norm violations (current)**: N/A (previous violations fixed: `for` → `while` in `table_create.c`, wrapper functions removed in `philo_main_routine.c`).

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
- `usleep()` takes µs: `usleep(200 * 1000)` = 200ms.
- `gettimeofday()` → ms: `tv.tv_sec * 1000 + tv.tv_usec / 1000`.

### Deadlock Prevention
- Asymmetric fork pickup: even IDs grab left→right, odd IDs grab right→left → avoids circular wait.
- Single philo (1 fork): special case needed — grab fork, wait `time_to_die`, die.

### Data Races
- Every shared variable written/read across threads must be mutex-protected.
- `eat_count` / `time_last_meal` / `alive` / `done`: per-philosopher mutex.
- `table->alive`: table mutex.
- All `printf` output: shared print mutex (via `philo_log`).
- `volatile` does NOT fix data races — only mutexes do.

### Cleanup Order
- Detached threads: no `pthread_join` needed.
- When `main()` returns, OS kills all threads and reclaims all resources (mutexes, memory). No need to call `pthread_mutex_destroy` on exit.
- On Linux (NPTL), `pthread_mutex_init`/`destroy` for default mutexes don't allocate/free heap memory — they just modify struct fields. Valgrind will not report leaks from missing `pthread_mutex_destroy` calls.

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
- Fix duplicate `table_create` definition (`table_create.c` vs `table_utils.c`).
- Remove `#include <stdarg.h>` in `src/utils.c` (unused, not allowed).
- Fix `parse_argument` type mismatch — change signature from `int *value` to `unsigned long *value` (or cast).
- Finalize `table_main_routine.c`: add philo thread creation, monitor loop.
- Wire up `main.c`: call `table_create`, `table_main_routine`, clean up, return.
- Review `src/philosopher/utils.c` for `t_lock *`/`pthread_mutex_t *` type mismatch on fork fields.
