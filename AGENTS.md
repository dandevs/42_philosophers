## Project Structure

```
philo/
├── Makefile
├── *.h
└── *.c

philo_bonus/ (optional)
├── Makefile
├── *.h
└── *.c
```

---

## Common Instructions

### Language & Standards
- **Language**: C
- **Compliance**: Must follow 42 Norm
- **Compiler**: cc with flags `-Wall`, `-Wextra`, `-Werror`
- **Libft**: Not authorized for this project

### Stability Requirements
- No unexpected quits (segfault, bus error, double free, etc.)
- All heap-allocated memory must be properly freed
- No memory leaks tolerated

### Makefile Requirements
Mandatory rules:
- `$(NAME)` - builds the executable
- `all` - default target
- `clean` - remove object files
- `fclean` - remove object files and executable
- `re` - fclean + all

For bonus:
- `bonus` rule - builds bonus executable
- Bonus files named `*_bonus.{c/h}`

### External Functions Allowed (Mandatory)
- `memset`, `printf`, `malloc`, `free`, `write`
- `usleep`, `gettimeofday`
- `pthread_create`, `pthread_detach`, `pthread_join`
- `pthread_mutex_init`, `pthread_mutex_destroy`, `pthread_mutex_lock`, `pthread_mutex_unlock`

### External Functions Allowed (Bonus)
- All mandatory functions
- `fork`, `kill`, `exit`, `waitpid`
- `sem_open`, `sem_close`, `sem_post`, `sem_wait`, `sem_unlink`

### (DANGEROUS) TEMPORARY FUNCTIONS
Remove these before evaluation!
- `atoi`

---

## Problem Overview

The Dining Philosophers Problem is a classic synchronization problem:

- Philosophers sit at a round table with a bowl of spaghetti
- Each philosopher cycles through: **eating → sleeping → thinking**
- Number of forks equals number of philosophers
- A philosopher needs **two forks** (left and right) to eat
- Simulation ends when a philosopher dies of starvation

### Key Constraints
- Philosophers don't communicate with each other
- Philosophers don't know if others are about to die
- No global variables allowed
- Must avoid data races

---

## Global Rules

### Arguments
```
./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```

| Argument | Description |
|----------|-------------|
| `number_of_philosophers` | Number of philosophers AND forks |
| `time_to_die` (ms) | Max time without eating before death |
| `time_to_eat` (ms) | Time spent eating (holding two forks) |
| `time_to_sleep` (ms) | Time spent sleeping |
| `number_of_times_each_philosopher_must_eat` (optional) | Stop when all philosophers eat this many times |

### Seating Arrangement
- Philosophers numbered 1 to N
- Philosopher 1 sits next to philosopher N
- Philosopher N sits between N-1 and N+1

### Log Format
All state changes must be logged with timestamps:
```
timestamp_in_ms X has taken a fork
timestamp_in_ms X is eating
timestamp_in_ms X is sleeping
timestamp_in_ms X is thinking
timestamp_in_ms X died
```

### Log Requirements
- No overlapping messages
- Death announcement within 10ms of actual death
- Timestamps in milliseconds
- X = philosopher number (1-indexed)

---

## Mandatory Part

### Implementation Requirements
- Each philosopher = separate **thread**
- One fork between each pair of philosophers
- Each fork's state protected by a **mutex**
- Single philosopher has access to only one fork (will die)

### Program Name
`philo`

---

## Bonus Part

### Implementation Requirements
- All forks in the middle of the table
- Fork availability represented by a **semaphore**
- Each philosopher = separate **process**
- Main process does NOT act as a philosopher

### Program Name
`philo_bonus`

### Important Note
Bonus is only evaluated if mandatory part is PERFECT (all requirements met, no malfunctions).

---

## README Requirements

Must include at least:

1. **First line** (italicized): *This project has been created as part of the 42 curriculum by <login1>[, <login2>[, ...]].*

2. **Description**: Project goal and brief overview

3. **Instructions**: Compilation, installation, execution

4. **Resources**: 
   - Classic references (documentation, articles, tutorials)
   - AI usage description (which tasks, which parts)

Additional sections may include: usage examples, feature list, technical choices.

Language: English

---

## Submission & Evaluation

### Directories
- Mandatory: `philo/`
- Bonus: `philo_bonus/`

### Evaluation Notes
- Only work in Git repository will be evaluated
- Brief modifications may be requested during evaluation
- Modifications test understanding of specific parts
- Must be feasible within a few minutes

---

## Key Technical Considerations

### Synchronization
- Prevent race conditions when accessing shared data
- Avoid deadlocks (especially with fork pickup order)
- Ensure proper mutex/lock ordering

### Timing
- Use `gettimeofday()` for accurate timestamps
- Handle `usleep()` granularity issues
- Death must be detected within 10ms

### Edge Cases
- Single philosopher (will die)
- Very short times (approaching zero)
- Large number of philosophers
- Optional meal count reached

---

## Common Pitfalls

1. **Data races** - All shared state must be protected
2. **Deadlocks** - Implement consistent fork pickup order
3. **Timing precision** - Death detection must be accurate
4. **Memory leaks** - Clean up all resources on exit
5. **Message overlap** - Protect printf calls with mutex
6. **Philosopher starvation** - Ensure fair fork access

---

## Gotchas & Pitfalls (from 42 student experience)

### Timing & Precision

- **`usleep()` is not precise.** `usleep(200000)` may wake up at 205ms or later. The OS scheduler doesn't guarantee exact wake-up. Use a polling loop (check + short `usleep(500)`) instead of one long sleep when precision matters.
- **Death detection must be within 10ms.** If your monitor loop checks every 10ms, you're already at the limit. Check every ~0.5ms (500μs). This is the #1 reason projects fail evaluation.
- **`gettimeofday()` returns microseconds, arguments are in milliseconds.** Mixing these up causes off-by-1000x bugs. Always convert: `tv.tv_sec * 1000 + tv.tv_usec / 1000`.
- **Timestamps must be relative to simulation start**, not epoch time. Compute once at start, subtract on every print.

### Thread Start Synchronization

- **All philosophers must not start eating at the exact same instant.** Without staggering, every philosopher grabs their left fork simultaneously → instant deadlock with odd-count tables. Use a small initial delay for even-numbered philosophers (`usleep(1000)`).
- **`start_time` must be recorded BEFORE launching threads**, and every philosopher's `last_meal_time` initialized to `start_time`. If you record start time after thread creation, philosophers may already be running and calculating wrong death times.
- **Thread creation is not instantaneous.** By the time `pthread_create` returns for philosopher 5, philosopher 1 may have already been running for milliseconds. This asymmetry can cause false deaths if `last_meal_time` isn't properly initialized.

### Death & Termination

- **A philosopher who dies during `usleep` must still be detected.** If you use raw `usleep()` for eating/sleeping, the death won't be caught until the sleep finishes — which may be way past the 10ms deadline. The monitor thread solves this, but only if philosopher sleeps are polling-based so they can exit early.
- **The death message must still print even after `someone_died` is set.** The philosopher that triggers death sets the flag, but its own print must go through. A common bug: checking `is_dead()` before printing causes the dying philosopher's own death message to be suppressed.
- **Only one death message allowed.** If two philosophers die nearly simultaneously, only the first one should print. Protect the death check-and-print with a mutex so the second one sees `someone_died == 1` and stays silent.
- **Philosophers must stop their routine after death is detected**, not just the monitor. If `someone_died` is set but philosopher threads keep looping, they may try to lock/unlock mutexes during cleanup → undefined behavior.

### Fork Pickup & Deadlocks

- **Symmetric pickup order causes deadlock.** If every philosopher always grabs left fork first, right fork second → circular wait. Even/odd asymmetric ordering or always locking the lower-indexed fork first breaks the cycle.
- **Holding one fork while waiting for the other can starve neighbors.** A philosopher holding its left fork and blocking on the right fork prevents its left neighbor from eating. Minimize the window between first and second fork acquisition.
- **Single philosopher edge case.** With 1 philosopher and 1 fork, the philosopher can never eat. Must die after `time_to_die` ms. Handle this as a special case before threading — don't create threads at all.
- **2 philosophers, 2 forks.** Both compete for the same pair. Without stagger start or asymmetric ordering, one will starve. Test this case explicitly.

### Race Conditions (Non-Obvious)

- **`meals_eaten++` and `last_meal_time = get_time_ms()` are NOT atomic.** The monitor reads these while the philosopher writes them. Without a per-philosopher mutex, the monitor can read a half-written value (torn read).
- **The `someone_died` flag needs a mutex too.** It's written by the monitor and read by every philosopher on every loop iteration. An unprotected `int` read/write is technically a data race even on single-core systems (C standard says it's undefined behavior).
- **`printf` is not atomic.** Two threads calling `printf` at the same time can produce interleaved characters. The output "100 2 is ea100 3 is sleepingting\n" is a real possibility. Protect with a print mutex.
- **Don't check `someone_died` then act — check and act atomically.** The TOCTOU (time-of-check-to-time-of-use) gap between checking `is_dead()` and locking a fork means a death might have occurred in between.

### Cleanup & Resource Leaks

- **Every `pthread_mutex_init` must have a matching `pthread_mutex_destroy`.** Forgetting to destroy even one mutex is a leak. Track them systematically (e.g., destroy in reverse order of creation).
- **`pthread_join` all threads before destroying mutexes.** If you destroy a mutex while a thread is still running and trying to lock it, you get undefined behavior. Join everything first, then destroy.
- **Named semaphores persist after process exit (bonus).** They live in `/dev/shm/` on Linux. If your program crashes without calling `sem_unlink`, stale semaphores remain and cause "semaphore already exists" errors on next run. Always clean up at start too: call `sem_unlink` before `sem_open`.
- **Child processes must be reaped (bonus).** Every `fork()` needs a matching `waitpid()`. Unreaped children become zombies. If the parent exits first, children become orphans adopted by init — still running and holding semaphore references.

### Evaluation-Specific

- **Evaluators will test `./philo 1 800 200 200`** — must die, must print one "taken fork" then death. No threads needed for this case.
- **Evaluators will test impossible timing** — `./philo 4 310 200 100` (time_to_die < time_to_eat + time_to_sleep). Someone must die.
- **Evaluators will test `./philo 5 800 200 200 5`** and verify all 5 philosophers eat exactly 5 times before the simulation stops cleanly.
- **Evaluators will run with 200 philosophers** — must not deadlock, must not crash. Stress tests reveal race conditions that don't appear with 5 philosophers.
- **Evaluators may request live modifications during evaluation** — you must understand every line of your code. Be prepared to change the death check interval or fork pickup order on the spot.
- **42 Norm compliance is checked.** Functions >25 lines, variable declarations at top of scope, no `for` loops — all the usual rules. Norm errors on a working project can still cause a failing grade.
