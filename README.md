*This project has been created as part of the 42 curriculum by danimend*

# Description

**philo** is a simulation of the classic [dining philosophers problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem): N philosophers sit around a table with N forks between them, alternating between *thinking*, *eating*, and *sleeping*. A philosopher needs two forks to eat, must not starve, and no two neighbors may eat at the same time.

The goal of the project is to solve this concurrency problem safely in C:

- **Mandatory part** (`philo/`): one thread per philosopher and one mutex per fork, using only POSIX threads (`pthread`) and mutexes.
- The simulation ends either when a philosopher dies of starvation or, if an optional meal limit is given, when every philosopher has eaten the required number of meals.

Key design choices:

- **No hold-and-wait**: forks are always locked and checked together; a philosopher that cannot take both forks immediately backs off, which makes circular wait (deadlock) impossible.
- **Centralized monitor**: the main thread watches every philosopher's last-meal time and is the only place a death is declared and printed, guaranteeing the `died` message is always the last one.
- **Norm of 42**: no globals, no external libraries, 25-line functions.

# Instructions

## Compilation

Requires `cc`, `make`, and POSIX threads.

```bash
cd philo
make          # build ./philo
make debug    # build with -g -O0 (debugging)
make clean    # remove object files
make fclean   # remove objects + executable
make re       # fclean + all
```

## Usage

```
./philo <philo_count> <time_to_die> <time_to_eat> <time_to_sleep> [<meals_required>]
```

| Argument | Meaning |
|---|---|
| `philo_count` | Number of philosophers (and forks) at the table |
| `time_to_die` | ms a philosopher survives without eating |
| `time_to_eat` | ms a philosopher spends eating (holding both forks) |
| `time_to_sleep` | ms a philosopher spends sleeping |
| `meals_required` | Optional: simulation stops cleanly once everyone ate this many meals |

Every state change is printed with a millisecond timestamp relative to the simulation start:

```
timestamp_in_ms X has taken a fork
timestamp_in_ms X is eating
timestamp_in_ms X is sleeping
timestamp_in_ms X is thinking
timestamp_in_ms X died
```

Death is always the last printed message.

## Example runs

```bash
./philo 1 800 200 200        # single philosopher: takes one fork, dies at ~800ms
./philo 4 310 200 100        # dies: time_to_die < eat + sleep
./philo 5 800 200 200 7      # everyone eats exactly 7 meals, clean stop
./philo 200 800 200 200      # stress: no deadlock, no death
```

# Resources

## References

- [Dining philosophers problem — Wikipedia](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- Multiple 42 repos on github for philosophers to see what they did.

## AI usage

- Fix norminette issues.
- Used in place of `man` for research and understanding of methods.