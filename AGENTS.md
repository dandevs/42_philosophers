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
