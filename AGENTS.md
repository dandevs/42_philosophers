## Build

```
make          # build philo
make debug    # build with -g -O0
make fclean   # clean objects + executable
```

- **CFLAGS strict flags are commented out** in Makefile (`-Wall -Wextra -Werror -pthread`). Uncomment before evaluation.
- `-pthread` linker flag needed once threading code is added.
- Sources auto-discovered: any `.c`/`.h` in `src/` subtree is compiled. Adding files to `src/table/` (or new subdirs) works with no Makefile changes.

## Testing

Tests in `tests/`. Each test = standalone `.c` with its own `main`. Custom test runner builds via auto-generated `test_build/Makefile`. See `tests/AGENTS.md` for full guide.

- **Return**: `0` = pass, non-zero = fail
- **Error output**: use `fprintf(stderr, "...")` (not `printf`) so the runner captures it correctly
- **No print on success** — silent = passing

## 42 Constraints (not obvious from code)

- **Norm**: max 25 lines/function, no `for` loops, variable declarations at top of scope. Norm errors fail evaluation even on working code. Use `while` loops, not `for`.
- **No global variables**.
- **Libft not authorized**. No external libraries.
- **Allowed functions (mandatory)**: `memset`, `printf`, `malloc`, `free`, `write`, `usleep`, `gettimeofday`, `pthread_create`, `pthread_detach`, `pthread_join`, `pthread_mutex_init`, `pthread_mutex_destroy`, `pthread_mutex_lock`, `pthread_mutex_unlock`
- **Allowed functions (bonus)**: all mandatory + `fork`, `kill`, `exit`, `waitpid`, `sem_open`, `sem_close`, `sem_post`, `sem_wait`, `sem_unlink`
- Using anything outside these lists is a grading failure.

## TEMPORARY — Remove Before Evaluation

`atoi()` in `src/utils.c:parse_argument()` must be replaced with a custom implementation.

## Current Implementation Status

**Done:** Argument parsing & validation, struct skeletons, table allocation + circular fork assignment.
**Not started:** Mutex init, thread creation, philosopher routine, monitor thread, timestamp logging, print mutex, meal counting, single-philosopher edge case, thread joining + cleanup.

**Struct fields still needed:**
- `t_philosopher` (currently only `fork_left`/`fork_right`): add `pthread_t thread`, `int id`, back-pointer to table, `unsigned long last_meal_time`, `int meals_eaten`, `pthread_mutex_t meal_mutex`
- `t_table` (currently only `philosophers`/`forks`/`count`): add `t_config`, `pthread_mutex_t print_mutex`, `pthread_mutex_t death_mutex`, `unsigned long start_time`, `int someone_died`

**Fork assignment** is symmetric (philo i → forks[i] left, forks[(i+1)%n] right) — needs asymmetric pickup order to avoid deadlock (see gotchas below).

## Program Specification

### Arguments
```
./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```
All values in milliseconds. Optional 5th arg: stop when all philosophers eat N times. `meals_required = -1` when unset.

### Log Format (exact, timestamps in ms relative to sim start)
```
timestamp_in_ms X has taken a fork
timestamp_in_ms X is eating
timestamp_in_ms X is sleeping
timestamp_in_ms X is thinking
timestamp_in_ms X died
```
X = philosopher number (1-indexed). No overlapping messages. Death announcement within 10ms of actual death.

### Mandatory vs Bonus
- **Mandatory**: threads + mutexes. Executable: `philo`.
- **Bonus**: processes + semaphores. Executable: `philo_bonus`, files named `*_bonus.{c/h}`. Only evaluated if mandatory is perfect.
- Single philosopher (1 fork): can't eat, must die. Handle without threads.

## Critical Gotchas

### Timing
- `usleep()` overshoots by ms. Use polling loop (`usleep(500)` + time check) instead of one long sleep.
- Death detection must be within 10ms. Monitor loop checks every ~0.5ms.
- `gettimeofday()` returns µs, args are ms. Convert: `tv.tv_sec * 1000 + tv.tv_usec / 1000`.
- Record `start_time` BEFORE launching threads. Set each philosopher's `last_meal_time = start_time` before their thread starts (thread creation is not instantaneous — early threads run before late ones are created).

### Deadlock Prevention
- Symmetric left→right pickup causes circular wait. Use even/odd asymmetric ordering: even IDs grab left first, odd IDs grab right first.
- Stagger start: even-numbered philosophers `usleep(1000)` before first action.
- Single philosopher: no threads needed — just print "taken fork", sleep `time_to_die`, print "died".

### Race Conditions
- `meals_eaten` and `last_meal_time` need per-philosopher mutex — monitor reads them while philosopher writes.
- `someone_died` flag needs its own mutex. Unprotected int read/write is undefined behavior in C.
- `printf` is not atomic — protect all output with a print mutex.
- Death message is the exception: must print even after `someone_died` is set (the dying philosopher sets the flag then prints).

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
- `tests/AGENTS.md` — test writing guide
