## Build

```
make          # build philo
make debug    # build with -g -O0
make fclean   # clean objects + executable
```

- **CFLAGS strict flags are commented out** in Makefile (`-Wall -Wextra -Werror -pthread`). Uncomment before evaluation. The `-pthread` flag (needed for linking) is inside that commented-out line.
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
- A philosopher blocked in `pthread_mutex_lock` when it dies stays stuck until the fork is released by another philosopher. This is expected — the death was already printed by the monitor, and the philosopher will exit on its next `death_flag` check.
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

### Mandatory vs Bonus
- **Mandatory**: threads + mutexes. Executable: `philo`.
- **Bonus**: processes + semaphores. Executable: `philo_bonus`, files named `*_bonus.{c/h}`.
- Single philosopher (1 fork): can't eat, must die. Handle without threads.

## Critical Gotchas

### Timing
- `usleep()` overshoots by ms. Use polling loop (`usleep(500)` + time check) instead of one long sleep.
- Death detection must be within 10ms. Monitor loop checks every ~0.5ms.
- `gettimeofday()` returns µs, args are ms. Convert: `tv.tv_sec * 1000 + tv.tv_usec / 1000`.
- Record `start_time` BEFORE launching threads. Set each philosopher's `last_meal_time = start_time` before their thread starts.

### Deadlock Prevention
- Symmetric left→right pickup causes circular wait. Use even/odd asymmetric ordering: even IDs grab left first, odd IDs grab right first.
- Stagger start: even-numbered philosophers `usleep(1000)` before first action.
- Single philosopher: no threads — print "taken fork", sleep `time_to_die`, print "died".

### Race Conditions
- `meals_eaten` and `last_meal_time` need per-philosopher mutex — monitor reads while philosopher writes.
- `someone_died` flag needs its own mutex. Unprotected int read/write is undefined behavior in C.
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

## Reference Docs

- `FUNCTIONS.md` — API reference for all allowed functions with implementation patterns and code examples
