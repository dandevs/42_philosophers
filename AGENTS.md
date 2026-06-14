> **Keep me updated!** After any significant code changes (new modules, renamed functions, changed patterns), update this file so agents stay aligned with the actual codebase.

## Build

```
make          # build philo
make debug    # build with -g -O0
make fclean   # clean objects + executable
```

- **CFLAGS strict flags are commented out** in Makefile (`-Wall -Wextra -Werror -pthread`). Uncomment before evaluation. (All source currently compiles clean under these flags.)
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

## File Map

| File | Role |
|---|---|
| `src/main.c` | Entry: `parse_arguments` → `table_create` → `table_main_routine` → `table_free` |
| `src/lib.h` | Core structs (`t_philosopher`, `t_config`, `t_table`) + shared prototypes |
| `src/fork.h`, `src/fork/fork.c` | `t_fork` + `fork_init` |
| `src/mutex_utils.h`, `src/mutex_utils/mutex_utils.c` | Typed mutex get/set helpers |
| `src/parse.c` | `parse_arguments`, `parse_int`, `parse_ulong`, `is_valid_number`, `ft_atoul` (custom, no `atoi`) |
| `src/utils.c` | `get_time_ms`, `philo_log` |
| `src/philosopher/utils.h`, `src/philosopher/utils.c` | `philo_init`, `philo_init_time` |
| `src/philosopher/philo_main_routine.c` | Philosopher thread routine |
| `src/table/table.h` | Table function prototypes |
| `src/table/table_create.c` | `table_create` (+ static `initialize_forks`, `init_philosopher`) |
| `src/table/table_main_routine.c` | `table_main_routine` (+ static `create_threads`) |
| `src/table/table_monitor.c` | `someone_died` (+ static `check_death`), `check_all_done`, `stop_threads` |
| `src/table/table_utils.c` | `table_free` |

## Architecture

### Data Structures

- **`t_fork`** (`src/fork.h`): `{ pthread_mutex_t mutex; int available; }`. `fork_init` sets `available = 1` and inits the mutex. The `available` flag is guarded by the fork's own mutex.
- **`t_config`** (`src/lib.h`): `philo_count`, `meals_required` (or `-1`), `time_to_die_ms`, `time_to_eat_ms`, `time_to_sleep_ms`. (No think-time field — the thinking phase is instant.)
- **`t_philosopher`** (`src/lib.h`): `mutex` (embedded), `fork_left`/`fork_right` (`t_fork *` — shared into `table->forks[]`), `table`, `thread`, `index`, `eat_count`, `done`, `alive`, `time_began_eating`, `start_time`.
- **`t_table`** (`src/lib.h`): `philosophers` array, `forks` (`t_fork *` — `malloc`'d array), `printf_mutex` (embedded), `mutex` (embedded), `config`, `start_time`, `alive`.

### Key Patterns

- **Typed mutex get/set** (`src/mutex_utils.h`). **Signature order is field-ptr first, mutex last:**
  - `m_set_int(int *ptr, int new_value, pthread_mutex_t *mutex)` / `m_get_int(int *ptr, pthread_mutex_t *mutex)`
  - `m_set_ulong(unsigned long *ptr, unsigned long new_value, pthread_mutex_t *mutex)` / `m_get_ulong(unsigned long *ptr, pthread_mutex_t *mutex)`
  - Call sites look like `m_set_int(&philo->done, 1, &philo->mutex)`.
  - Type-safe: no `(void*)` casts, no size mismatch (the old `set_with_mutex` wrote `sizeof(void*)` bytes into `int` fields, corrupting adjacent memory on 64-bit — these replace it).
- **`philo_log(philo, msg)`** (`src/utils.c`): no-op if `table->alive == 0`; else prints `<ms_since_start> <philo_id> <msg>\n` under `table->printf_mutex`. Philo id is `index + 1` (1-indexed).
- **`get_time_ms()`** (`src/utils.c`): `gettimeofday` → `tv.tv_sec * 1000 + tv.tv_usec / 1000`.
- `POLLING_RATE = 100` (µs) defined in `lib.h`.

### Mutex Embedding Rule

Per-object mutexes (`philo->mutex`, `table->mutex`, `table->printf_mutex`) are **embedded** (`pthread_mutex_t`, not pointer) — no separate malloc, accessed via `&philo->mutex`.

Shared forks MUST remain pointers (`t_fork *fork_left/right` in philo, `t_fork *forks` in table) because adjacent philosophers share fork references and the fork array is dynamically sized.

### Philosopher Lifecycle

- Each philosopher runs in a **joinable** thread (created in `create_threads`). Threads are joined by `stop_threads` at shutdown — **not** detached.
- Routine in `src/philosopher/philo_main_routine.c`:
  - `while (alive)`: small startup stagger `usleep(((eat_count + index) * 200) % 5000)` → `get_both_forks()` → `philo_eat()` (returns 1 to stop when `meals_required` reached) → `philo_log("is sleeping")` → `usleep(time_to_sleep)` → `philo_log("is thinking")` → repeat.
  - **No death self-check** in the routine — death is detected solely by the monitor. Philo loops only re-check `alive` (set by `stop_threads`).
- **`get_both_forks`**: single-philo special case (`fork_left == fork_right`) logs one "has taken a fork" then busy-waits on `alive` until the monitor kills it (returns 0, loop breaks). Otherwise loops `try_lock_forks` until success or death.
- **`try_lock_forks`** (no hold-and-wait): lock right→left fork mutexes, check both `available`; if both free set both `available = 0`, unlock both, log "has taken a fork" ×2, return 1; else unlock both, return 0. Because both mutexes are always taken/released together, no circular wait → no deadlock.
- **`philo_eat`**: `time_began_eating = now` → log "is eating" → `usleep(time_to_eat)` → `unlock_both_forks` → `eat_count++` → if `meals_required != -1 && eat_count >= meals_required`, set `done = 1` and return 1.
- **`unlock_both_forks`**: single-philo case busy-waits forever (it's dying); otherwise sets both `available = 1` under the fork mutexes.
- Philosopher fields (`alive`, `eat_count`, `done`, `time_began_eating`) protected by per-philosopher `mutex`.
- `philo_init` — sets `index`, `alive = 1`, `eat_count = 0`, `done = 0`, inits mutex.
- `philo_init_time` — sets `table->start_time` and each philo's `time_began_eating` / `start_time` to `get_time_ms()`.

### Table Lifecycle

- `table_create` (`src/table/table_create.c`): `initialize_forks` (`malloc` `t_fork[]`, `fork_init` each), `malloc` philosophers, `init_philosopher` each (assigns `fork_left = &forks[i]`, `fork_right = &forks[(i+1) % n]`, calls `philo_init`), inits `printf_mutex` + `mutex`, sets `alive = 1`, stores `config`.
- `table_main_routine` (`src/table/table_main_routine.c`): `philo_init_time` → `create_threads` (`pthread_create` each) → monitor loop → `stop_threads` → returns 1.
- `table_free` (`src/table/table_utils.c`): frees `forks` + `philosophers` arrays. (Single definition — no duplicate.)

### Monitoring Architecture

- **Main thread runs the monitor** (no separate monitor thread), in `table_main_routine`.
- Loop: `while (!done)` check `someone_died` / `check_all_done`, else `usleep(POLLING_RATE)`.
- **`someone_died`** → static `check_death` per philo: reads `time_began_eating` via `m_get_ulong`, `elapsed = now - time_began_eating`; if `elapsed >= time_to_die_ms`, sets `table->alive = 0` under table mutex, prints `"<t> <id> died\n"` under `printf_mutex`, returns 1. (Variable is named `last_meal` locally but reads `time_began_eating`.)
- **`check_all_done`**: returns 0 immediately if `meals_required == -1`; else returns 1 only when every philo has `done == 1`.
- **`stop_threads`**: sets `table->alive = 0`, sets each philo `alive = 0`, then `pthread_join`s every thread. After it returns, `table_main_routine` returns to `main`, which calls `table_free`.
- Death is the **last printed message** (the `philo_log` no-op gate + `printf_mutex` ordering enforce this).

### Current State

- Simulation is **fully wired and working**. All four evaluator cases pass (see below): single-philo death, starvation death, `meals_required` clean stop, and 200-philosopher stress (no deadlock/crash/death over long runs).
- Compiles clean under `-Wall -Wextra -Werror -pthread`.
- **Remaining pre-evaluation tasks:**
  - Uncomment the strict `CFLAGS` line in `Makefile`.
  - Write the root `README.md` (italicized first line `*...by <login>*`, Description, Instructions, Resources).
  - Implement `philo_bonus/` (processes + semaphores, files named `*_bonus.{c,h}`).

## 42 Constraints

- **Norm**: 25 lines/func, no `for`, vars at top of scope, `while` loops.
- **No globals**, **no external libs**, **no atomics**.
- **Allowed (mandatory)**: `memset`, `printf`, `malloc`, `free`, `write`, `usleep`, `gettimeofday`, `pthread_create`, `pthread_detach`, `pthread_join`, `pthread_mutex_init`, `pthread_mutex_destroy`, `pthread_mutex_lock`, `pthread_mutex_unlock`. (Currently uses `pthread_join`; `pthread_detach` is unused.)
- **Allowed (bonus)**: above + `fork`, `kill`, `exit`, `waitpid`, `sem_open`, `sem_close`, `sem_post`, `sem_wait`, `sem_unlink`
- **Norm violations (current)**: none known.

## Program Specification

### Arguments
```
./philo philo_count time_to_die time_to_eat time_to_sleep [meals_required]
```
`meals_required = -1` when unset (simulation runs until someone dies). All args are validated as positive integers via `parse_int`/`parse_ulong`.

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
- **Mandatory**: threads + mutexes (current `src/`, builds to `./philo`).
- **Bonus**: processes + semaphores in `philo_bonus/`, files named `*_bonus.{c/h}`. Not yet started.

## Critical Gotchas

### Timing
- `usleep()` takes µs: `usleep(200 * 1000)` = 200ms.
- `gettimeofday()` → ms: `tv.tv_sec * 1000 + tv.tv_usec / 1000`.

### Deadlock Prevention
- **No hold-and-wait**: `try_lock_forks` always locks/releases both fork mutexes together, claiming both only if both `available`, else backs off for `POLLING_RATE`. No circular wait is possible.
- **Startup stagger** `usleep(((eat_count + index) * 200) % 5000)` desynchronizes philosophers to reduce spin contention.
- **Single philo** (`fork_left == fork_right`): handled in `get_both_forks` — logs one "has taken a fork", then waits for death (cannot eat with one fork).

### Data Races
- Every shared variable written/read across threads is mutex-protected.
- `eat_count` / `done` / `alive` / `time_began_eating` (philo): per-philosopher `mutex` (use the `m_*` helpers).
- `table->alive`: table `mutex`.
- All `printf` output: shared `printf_mutex` (via `philo_log`, which also gates on `table->alive`).
- `volatile` does NOT fix data races — only mutexes do.

### Cleanup Order
- Threads are joinable. `stop_threads` flips `alive` flags (waking any busy-waiting philo loops) and `pthread_join`s every thread before `table_main_routine` returns.
- `main` then calls `table_free` (frees the two arrays). No `pthread_mutex_destroy` calls — on Linux (NPTL), default mutexes don't allocate heap, so Valgrind reports no leaks from skipping destroy.
- Single-philo `unlock_both_forks` intentionally never returns (infinite `usleep`) — that philosopher is dying and never releases its lone fork; the process exits shortly after.

### Key Evaluator Test Cases
```
./philo 1 800 200 200          # dies: 1 "taken fork" + death ~800ms
./philo 4 310 200 100          # dies (time_to_die < eat + sleep); death ~310ms
./philo 5 800 200 200 7        # all eat exactly 7 times, clean stop, no death
./philo 200 800 200 200        # stress: no deadlock, no crash, no death
```
All four verified passing after the dead-code cleanup (2026-06-14).

## Submission Structure
- Mandatory: `philo/` (current repo root builds `./philo`).
- Bonus: `philo_bonus/`.
- Root `README.md` required: italicized first line (`*...by <login>*`), Description, Instructions, Resources.
