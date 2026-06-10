## Build

```
make          # build philo
make debug    # build with -g -O0
make fclean   # clean objects + executable
```

- **CFLAGS strict flags are commented out** in Makefile (`-Wall -Wextra -Werror -pthread`). Uncomment before evaluation. The `-pthread` flag (needed for linking) is inside that commented-out line.
- Must use `cc` compiler (currently `CC = cc`). Makefile must not perform unnecessary relinking — currently OK since `$(NAME)` has proper object prerequisites.
- Required targets: `$(NAME)`, `all`, `clean`, `fclean`, `re`.
- Sources auto-discovered: any `.c`/`.h` in `src/` subtree is compiled. Adding files works with no Makefile changes.

## Testing

Custom test runner builds via auto-generated `test_build/Makefile`. Run: `ctester`

### Test Structure

Each test is a standalone `.c` file with its own `main`:

- **Include** project headers via the `src/` path: `#include "lib.h"`, `#include "table/table.h"`
- **On success**: return `0`, print nothing.
- **On failure**: return non-zero and print error to stderr via `fprintf(stderr, ...)`. No "FAIL:" or "Error:" prefixes.
- **No print on success** — silent = passing.

### Adding Tests

1. Create directory: `mkdir tests/my_suite`
2. Add `.c` files: `tests/my_suite/test_basic.c`
3. Run: `ctester`

## Architecture

- Each philosopher thread runs autonomously: eat → sleep → think loop. The table never orchestrates philosopher actions.
- **The monitor detects death and prints it.** The philosopher never self-diagnoses or prints its own death. The monitor (main thread or separate thread) checks each philosopher's `last_meal_time` against `time_to_die`.
- `death_flag` (mutex-protected) is the sole coordination point: the monitor sets it, philosophers check it between actions to know when to exit.
- **No thread killing**: `pthread_cancel`, `pthread_kill`, `exit()` are not allowed. Only cooperative exit via `death_flag` checks.
- A philosopher that dies while eating (holding both fork mutexes) must release those forks on exit. If it doesn't, neighboring philosophers stuck on `pthread_mutex_lock` will never unblock and `pthread_join` in the main thread will hang forever.
- A philosopher blocked in `pthread_mutex_lock` when another philosopher dies stays stuck until that fork's mutex is unlocked by the dying philosopher's cleanup (see previous bullet).
- Circular table: philosopher 1 sits next to philosopher N (where N = `number_of_philosophers`). Philosopher N sits between N-1 and 1. For 0-indexed code: philo 0 = philosopher 1 (1-indexed).
- Asymmetric fork pickup prevents deadlock: even IDs grab left→right, odd IDs grab right→left.
- Even-numbered philosophers stagger start with `usleep(1000)` before first action to avoid thundering herd.
- Single philosopher: no threads — print "taken fork", polling sleep `time_to_die`, print "died".
- One death stops the entire simulation (`death_flag` → all philosophers exit).
- `meals_eaten` and `last_meal_time` are written by the philosopher and read by the monitor — protect with per-philosopher mutex.
- All `printf` output is protected by a shared print mutex to prevent interleaved messages.

## 42 Constraints (not obvious from code)

- **Norm**: max 25 lines/function, no `for` loops, variable declarations at top of scope. Use `while` loops.
- **No global variables**.
- **No external libraries** (Libft not authorized).
- **No atomics**: `<stdatomic.h>` is not in the allowed functions list. Mutexes only for synchronization.
- **Allowed functions (mandatory)**: `memset`, `printf`, `malloc`, `free`, `write`, `usleep`, `gettimeofday`, `pthread_create`, `pthread_detach`, `pthread_join`, `pthread_mutex_init`, `pthread_mutex_destroy`, `pthread_mutex_lock`, `pthread_mutex_unlock`
- **Allowed functions (bonus)**: all mandatory + `fork`, `kill`, `exit`, `waitpid`, `sem_open`, `sem_close`, `sem_post`, `sem_wait`, `sem_unlink`
- Using anything outside these lists is a grading failure.

## TEMPORARY — Remove Before Evaluation

`atoi()` in `src/utils.c:parse_argument()` must be replaced with a custom implementation.

## Program Specification

### Arguments
```
./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```
All values in milliseconds. `meals_required = -1` when unset.

### Log Format (exact, timestamps in ms relative to sim start)
```
timestamp_in_ms X has taken a fork
timestamp_in_ms X is eating
timestamp_in_ms X is sleeping
timestamp_in_ms X is thinking
timestamp_in_ms X died
```
X = philosopher number (1-indexed). No overlapping messages. Death within 10ms.
Death is the **last printed message**. No "thinking" or "sleeping" appears after a death line.

### Mandatory vs Bonus
- **Mandatory**: threads + mutexes. Executable: `philo`, files in `philo/` directory.
- **Bonus**: processes + semaphores. Executable: `philo_bonus`, files in `philo_bonus/` directory named `*_bonus.{c/h}`.
- Single philosopher (1 fork): can't eat, must die. Handle without threads.
- **Bonus specifics**: All forks in center of table, available forks tracked via semaphore (init to `number_of_philosophers`). Each philosopher is a separate `fork()` process. Main process does NOT act as a philosopher. Bonus evaluated only if mandatory is perfect.

## Critical Gotchas

### Timing
- `usleep()` takes **microseconds** (µs). 1ms = 1000µs. Convert: `usleep(200 * 1000)` for 200ms.
- `usleep()` overshoots by ms. Never use it for precise waits — use polling loop (`usleep(500)` + time check) instead.
- Polling sleep is needed for **both eating and sleeping**: if death is detected mid-meal, the philosopher must wake up, see `death_flag`, release forks, and exit. A blocking `usleep(time_to_eat * 1000)` prevents this.
- Death detection must be within 10ms. Monitor loop checks every ~0.5ms.
- `gettimeofday()` returns µs, args are ms. Convert: `tv.tv_sec * 1000 + tv.tv_usec / 1000`.
- Record `start_time` BEFORE launching threads. Set each philosopher's `last_meal_time = start_time` before their thread starts.

### Deadlock Prevention
- Symmetric left→right pickup causes circular wait. Use even/odd asymmetric ordering: even IDs grab left first, odd IDs grab right first.
- Stagger start: even-numbered philosophers `usleep(1000)` before first action.
- Single philosopher: no threads — print "taken fork", sleep `time_to_die`, print "died".

### Data Races & Race Conditions
- Subject explicitly requires **zero data races**. Every shared variable written by one thread and read by another must be mutex-protected.
- `meals_eaten` and `last_meal_time` need per-philosopher mutex — monitor reads while philosopher writes.
- `someone_died` flag needs its own mutex. Unprotected int read/write is undefined behavior in C.
- **`volatile` does NOT fix data races.** It only prevents compiler caching — writes can still tear or be reordered. Only mutexes (or atomics, which are not allowed in 42) prevent data races.
- `printf` is not atomic — protect all output with a print mutex.
- Death message is the exception: must print even after `someone_died` is set.

### Cleanup Order
- `pthread_join` ALL threads before destroying any mutex.
- Every `pthread_mutex_init` must have matching `pthread_mutex_destroy`.
- Bonus: `sem_unlink` before `sem_open` (stale semaphores in `/dev/shm/`), `waitpid` all children.

### Key Evaluator Test Cases
```
./philo 1 800 200 200          # dies, one "taken fork" + death msg
./philo 4 310 200 100          # dies (time_to_die < eat + sleep)
./philo 5 800 200 200 5        # all eat exactly 5 times, clean stop
./philo 200 800 200 200        # stress: no deadlock, no crash
```

### Death Guarantees
- **`time_to_die < time_to_eat`**: guaranteed death — philosopher cannot finish eating before timer expires. Monitor catches them mid-meal.
- **`time_to_die < time_to_eat + time_to_sleep`**: typically dead — eating + sleeping exceeds the death window, even without fork contention.
- **`time_to_die > time_to_eat`**: not automatically dead. Survival depends on sleeping duration and fork contention.

## Submission Structure
- Mandatory part directory: `philo/` (current project at repo root is `philo/`)
- Bonus part directory: `philo_bonus/`

## README Requirements (subject mandate)
Root `README.md` required with:
- First line italicized: `*This project has been created as part of the 42 curriculum by <login>*`
- **Description** section — goal and overview
- **Instructions** section — compilation, installation, execution
- **Resources** section — classic references + how AI was used and for which tasks

## 42 Header Dates

To get the correct timestamp for `Created:` / `Updated:` in 42 headers:
```
date '+%Y/%m/%d %H:%M:%S'
```

## Reference Docs

- `FUNCTIONS.md` — API reference for all allowed functions with implementation patterns and code examples

## Philosopher Utilities (`src/philosopher/utils.h`, `src/philosopher/utils.c`)

Shared helper functions for fork manipulation in `philo_main_routine`:

- **`with_philo_lock(t_philosopher *philo, void (*func)(t_philosopher *))`** — Locks `philo->mutex`, calls `func(philo)`, unlocks. Used to atomically modify philosopher fields from the philosopher thread.
- **`take_left_fork(t_philosopher *philo)`** — Sets `fork_left->available = 0`, `has_fork_left = 1`.
- **`take_right_fork(t_philosopher *philo)`** — Sets `fork_right->available = 0`, `has_fork_right = 1`.
- **`release_left_fork(t_philosopher *philo)`** — Sets `fork_left->available = 1`, `has_fork_left = 0`.
- **`release_right_fork(t_philosopher *philo)`** — Sets `fork_right->available = 1`, `has_fork_right = 0`.

Include: `#include "philosopher/utils.h"`
