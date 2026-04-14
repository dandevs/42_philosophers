# External Functions Reference - Philosophers Project

This document describes all external functions allowed for the Philosophers project, including their purpose and usage examples.

---

## Table of Contents

- [Mandatory Functions](#mandatory-functions)
  - [Standard Library](#standard-library)
  - [Time Functions](#time-functions)
  - [Thread Functions](#thread-functions)
  - [Mutex Functions](#mutex-functions)
- [Bonus Functions](#bonus-functions)
  - [Process Functions](#process-functions)
  - [Semaphore Functions](#semaphore-functions)

---

## Mandatory Functions

### Standard Library

#### `memset`

**Description:**  
Fills a block of memory with a specified value.

**Purpose:**  
Created to efficiently initialize memory regions to a known value (typically zero). Before `memset`, programmers had to write loops to initialize memory, which was error-prone and less optimized. It solves the problem of quickly zeroing out or setting memory for structs, arrays, and buffers.

**Usage:**
```c
#include <string.h>

typedef struct s_philo {
    int id;
    int meals_eaten;
} t_philo;

t_philo philosopher;
memset(&philosopher, 0, sizeof(t_philo));  // Zero out all fields
// philosopher.id = 0, philosopher.meals_eaten = 0

char buffer[100];
memset(buffer, 'A', sizeof(buffer));  // Fill buffer with 'A' characters
```

---

#### `printf`

**Description:**  
Formats and prints output to stdout.

**Purpose:**  
Created to provide formatted output capability (like the original `printf` from BCPL/early C). It solves the problem of converting various data types (integers, strings, pointers) into human-readable text with customizable formatting. Without it, programmers would need to manually convert numbers to strings and handle different types separately.

**Usage:**
```c
#include <stdio.h>

int philo_id = 3;
long timestamp = 1234567;

// Basic logging for philosopher states
printf("%ld %d has taken a fork\n", timestamp, philo_id);
printf("%ld %d is eating\n", timestamp, philo_id);
printf("%ld %d is sleeping\n", timestamp, philo_id);
printf("%ld %d died\n", timestamp, philo_id);

// Format specifiers
printf("Philosopher %d ate %d meals\n", id, meal_count);
```

---

#### `malloc`

**Description:**  
Allocates memory dynamically from the heap.

**Purpose:**  
Created to solve the problem of unknown or variable memory requirements at compile time. Before dynamic allocation, all memory had to be statically declared (fixed sizes), wasting memory or limiting functionality. `malloc` enables programs to request exactly the memory they need at runtime.

**Usage:**
```c
#include <stdlib.h>

typedef struct s_data {
    int num_philos;
    pthread_mutex_t *forks;
} t_data;

// Allocate array for 5 philosophers
t_data *data = malloc(sizeof(t_data));
if (!data)
    return (NULL);

// Allocate array of mutexes for forks
data->forks = malloc(sizeof(pthread_mutex_t) * num_philosophers);
if (!data->forks) {
    free(data);
    return (NULL);
}
```

---

#### `free`

**Description:**  
Deallocates memory previously allocated by `malloc`.

**Purpose:**  
Created to solve the memory leak problem - without `free`, allocated memory would remain reserved until program termination. This allows programs to reuse memory, prevent exhaustion, and properly clean up resources. It's essential for long-running programs.

**Usage:**
```c
#include <stdlib.h>

typedef struct s_philo {
    pthread_t thread;
    int id;
} t_philo;

t_philo *philos = malloc(sizeof(t_philo) * num_philosophers);

// ... use philosophers ...

// Cleanup when done
free(philos);  // Release the memory
philos = NULL; // Good practice: prevent dangling pointer
```

---

#### `write`

**Description:**  
Writes data to a file descriptor.

**Purpose:**  
A low-level system call created to output data to files, devices, or communication channels. Unlike `printf`, it works directly with bytes, making it more atomic (important for thread safety). It solves the problem of outputting raw data without formatting overhead.

**Usage:**
```c
#include <unistd.h>

// Thread-safe writing (more atomic than printf)
char msg[50];
int len = sprintf(msg, "%ld %d is eating\n", timestamp, id);
write(1, msg, len);  // 1 = stdout file descriptor

// Writing to stderr for errors
write(2, "Error: malloc failed\n", 21);  // 2 = stderr
```

---

### Time Functions

#### `gettimeofday`

**Description:**  
Gets the current time with microsecond precision.

**Purpose:**  
Created to provide high-resolution timestamps for measuring time intervals. Unlike `time()` which only gives seconds, this solves the problem of needing millisecond/microsecond accuracy for performance measurement, timeouts, and precise logging.

**Usage:**
```c
#include <sys/time.h>

// Get current time in milliseconds
long get_current_time(void)
{
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

// Usage in simulation
long start_time = get_current_time();
// ... philosopher eats ...
long elapsed = get_current_time() - start_time;
```

---

#### `usleep`

**Description:**  
Suspends execution for microseconds.

**Purpose:**  
Created to allow programs to pause without consuming CPU cycles (unlike busy-waiting loops). It solves the problem of implementing delays while being resource-efficient. Essential for simulating time passing (eating, sleeping) without wasting processor time.

**Usage:**
```c
#include <unistd.h>

// Sleep for time_to_eat milliseconds
void precise_sleep(int milliseconds)
{
    usleep(milliseconds * 1000);  // usleep takes microseconds
}

// In philosopher routine
printf("%ld %d is eating\n", get_time(), id);
usleep(time_to_eat * 1000);
printf("%ld %d is sleeping\n", get_time(), id);
usleep(time_to_sleep * 1000);
```

---

### Thread Functions

#### `pthread_create`

**Description:**  
Creates a new thread of execution.

**Purpose:**  
Created by POSIX to enable concurrent execution within a single process. Before threads, achieving concurrency required multiple processes (heavyweight). `pthread_create` solves the problem of lightweight parallelism - sharing memory while executing independently, essential for running multiple philosophers simultaneously.

**Usage:**
```c
#include <pthread.h>

typedef struct s_philo {
    pthread_t thread;
    int id;
} t_philo;

void *philosopher_routine(void *arg)
{
    t_philo *philo = (t_philo *)arg;
    // ... eat, sleep, think ...
    return (NULL);
}

// Create threads for each philosopher
t_philo philos[5];
for (int i = 0; i < 5; i++) {
    philos[i].id = i + 1;
    pthread_create(&philos[i].thread, NULL, 
                   philosopher_routine, &philos[i]);
}
```

---

#### `pthread_detach`

**Description:**  
Marks a thread as detached, allowing automatic cleanup on exit.

**Purpose:**  
Created to solve the resource cleanup problem for threads that don't need to be joined. Without detaching or joining, terminated threads remain as "zombies" consuming resources. Detaching is useful for "fire and forget" threads that clean up their own resources.

**Usage:**
```c
#include <pthread.h>

// Create a detached thread (alternative approach)
pthread_t thread;
pthread_create(&thread, NULL, routine, arg);
pthread_detach(thread);
// Thread will clean itself up when done, no need to join

// Or set detach state before creation
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
pthread_create(&thread, &attr, routine, arg);
pthread_attr_destroy(&attr);
```

---

#### `pthread_join`

**Description:**  
Waits for a thread to terminate and retrieves its return value.

**Purpose:**  
Created to provide synchronization between threads - allowing one thread to wait for another to complete. It solves the problem of knowing when a thread has finished and prevents the main program from exiting before threads complete their work.

**Usage:**
```c
#include <pthread.h>

// Wait for all philosophers to finish
for (int i = 0; i < num_philosophers; i++) {
    pthread_join(philos[i].thread, NULL);
    // Main thread blocks here until philos[i] terminates
}

// With return value
void *result;
pthread_join(thread, &result);
// result now contains what the thread returned
```

---

### Mutex Functions

#### `pthread_mutex_init`

**Description:**  
Initializes a mutex with specified attributes.

**Purpose:**  
Created to prepare a mutex for use. Mutexes (mutual exclusion) were invented to solve the critical section problem - preventing race conditions when multiple threads access shared data. This function sets up the synchronization primitive.

**Usage:**
```c
#include <pthread.h>

pthread_mutex_t fork_mutex;

// Initialize with default attributes
pthread_mutex_init(&fork_mutex, NULL);

// Or initialize multiple forks
pthread_mutex_t forks[5];
for (int i = 0; i < 5; i++) {
    pthread_mutex_init(&forks[i], NULL);
}
```

---

#### `pthread_mutex_destroy`

**Description:**  
Destroys a mutex and releases its resources.

**Purpose:**  
Created to clean up mutex resources when they're no longer needed. Without proper destruction, system resources (memory, kernel objects) could leak. It solves the resource cleanup problem for synchronization primitives.

**Usage:**
```c
#include <pthread.h>

pthread_mutex_t forks[5];

// ... use forks ...

// Cleanup when simulation ends
for (int i = 0; i < 5; i++) {
    pthread_mutex_destroy(&forks[i]);
}

// Also needed for single mutex
pthread_mutex_t print_mutex;
pthread_mutex_init(&print_mutex, NULL);
// ... use it ...
pthread_mutex_destroy(&print_mutex);
```

---

#### `pthread_mutex_lock`

**Description:**  
Acquires a mutex lock, blocking if necessary.

**Purpose:**  
The core synchronization primitive - created to enforce mutual exclusion. When a thread locks a mutex, it gains exclusive access to the protected resource. Other threads block until unlock. This solves the race condition problem by ensuring only one thread accesses critical code/data at a time.

**Usage:**
```c
#include <pthread.h>

pthread_mutex_t forks[5];

void take_forks(int philo_id)
{
    int left = philo_id - 1;
    int right = philo_id % num_philosophers;
    
    pthread_mutex_lock(&forks[left]);   // Acquire left fork
    printf("%d has taken a fork\n", philo_id);
    
    pthread_mutex_lock(&forks[right]);  // Acquire right fork
    printf("%d has taken a fork\n", philo_id);
}

// Protecting shared print
pthread_mutex_t print_lock;
pthread_mutex_lock(&print_lock);
printf("%ld %d is eating\n", time, id);
pthread_mutex_unlock(&print_lock);
```

---

#### `pthread_mutex_unlock`

**Description:**  
Releases a mutex lock, allowing other threads to acquire it.

**Purpose:**  
Created as the counterpart to lock - it solves the problem of releasing exclusive access so other threads can proceed. Without unlock, threads would deadlock forever waiting for a resource. Proper lock/unlock pairing is essential for correct synchronization.

**Usage:**
```c
#include <pthread.h>

pthread_mutex_t forks[5];

void put_forks(int philo_id)
{
    int left = philo_id - 1;
    int right = philo_id % num_philosophers;
    
    pthread_mutex_unlock(&forks[left]);   // Release left fork
    pthread_mutex_unlock(&forks[right]);  // Release right fork
    printf("%d put down forks\n", philo_id);
}
    
// Safe eating routine
pthread_mutex_lock(&left_fork);
pthread_mutex_lock(&right_fork);
eat();
pthread_mutex_unlock(&right_fork);
pthread_mutex_unlock(&left_fork);
```

---

## Bonus Functions

### Process Functions

#### `fork`

**Description:**  
Creates a new process by duplicating the current one.

**Purpose:**  
One of the original UNIX system calls, created to enable multitasking. It solves the problem of creating independent execution units that don't share memory (unlike threads). Each philosopher as a process gets its own memory space, preventing accidental shared state corruption.

**Usage:**
```c
#include <unistd.h>
#include <sys/wait.h>

pid_t pids[5];

for (int i = 0; i < num_philosophers; i++) {
    pids[i] = fork();
    
    if (pids[i] == -1) {
        perror("fork failed");
        exit(1);
    }
    else if (pids[i] == 0) {
        // Child process - this philosopher's code
        philosopher_routine(i + 1);
        exit(0);
    }
    // Parent continues to fork next philosopher
}

// Parent waits for children
for (int i = 0; i < num_philosophers; i++) {
    waitpid(pids[i], NULL, 0);
}
```

---

#### `kill`

**Description:**  
Sends a signal to a process or process group.

**Purpose:**  
Created to provide inter-process communication and control through signals. It solves the problem of one process notifying or controlling another - particularly useful for stopping all philosophers when one dies.

**Usage:**
```c
#include <signal.h>
#include <unistd.h>

pid_t philo_pids[5];

// When a philosopher dies, kill all others
void stop_all_philosophers(int dead_id)
{
    for (int i = 0; i < num_philosophers; i++) {
        if (i + 1 != dead_id) {
            kill(philo_pids[i], SIGTERM);  // Send termination signal
        }
    }
}

// Alternative: use SIGKILL for immediate termination
kill(pid, SIGKILL);  // Cannot be caught or ignored
```

---

#### `exit`

**Description:**  
Terminates the calling process immediately.

**Purpose:**  
Created to provide a clean way to terminate a process with a status code. It solves the problem of returning control to the operating system and reporting success/failure. Unlike returning from main, `exit` can be called from anywhere.

**Usage:**
```c
#include <stdlib.h>

// Child process cleanup
if (philo_died) {
    printf("%d died\n", id);
    exit(1);  // Exit with error status
}

// Normal completion
exit(0);  // Success

// Error handling
if (!data) {
    write(2, "Error: initialization failed\n", 29);
    exit(EXIT_FAILURE);
}
```

---

#### `waitpid`

**Description:**  
Waits for a specific child process to change state.

**Purpose:**  
Created to allow parent processes to synchronize with and reap child processes. It solves the zombie process problem - without waitpid, terminated children remain in the process table. It also lets the parent know when children finish.

**Usage:**
```c
#include <sys/wait.h>
#include <unistd.h>

pid_t pid = fork();
if (pid == 0) {
    // Child
    philosopher_work();
    exit(0);
}

// Parent waits for specific child
int status;
waitpid(pid, &status, 0);

// Check how child exited
if (WIFEXITED(status)) {
    printf("Child exited with status %d\n", WEXITSTATUS(status));
}

// Non-blocking wait
waitpid(pid, &status, WNOHANG);  // Returns immediately if no child exited
```

---

### Semaphore Functions

#### `sem_open`

**Description:**  
Opens/creates a named semaphore.

**Purpose:**  
Created to provide named, system-wide synchronization primitives that persist beyond process lifetime. Unlike mutexes (thread-only), named semaphores solve the inter-process synchronization problem - multiple unrelated processes can share a semaphore by name.

**Usage:**
```c
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

// Create/open a named semaphore
sem_t *forks = sem_open("/philo_forks", 
                         O_CREAT,           // Create if doesn't exist
                         0644,              // Permissions
                         num_philosophers); // Initial value

if (forks == SEM_FAILED) {
    perror("sem_open failed");
    exit(1);
}

// Per-philosopher semaphore for meal counting
char sem_name[20];
sprintf(sem_name, "/philo_%d", id);
sem_t *meal_sem = sem_open(sem_name, O_CREAT, 0644, 0);
```

---

#### `sem_close`

**Description:**  
Closes a semaphore previously opened by `sem_open`.

**Purpose:**  
Created to release the process's reference to a semaphore. It solves the resource cleanup problem - each process that opens a semaphore should close it. The semaphore persists until `sem_unlink` is called and all references are closed.

**Usage:**
```c
#include <semaphore.h>

sem_t *forks = sem_open("/philo_forks", 0);
sem_t *print_sem = sem_open("/print_lock", 0);

// ... use semaphores ...

// Cleanup before exit
sem_close(forks);
sem_close(print_sem);

// In child process before exit
sem_close(forks);
exit(0);
```

---

#### `sem_post`

**Description:**  
Increments (unlocks) a semaphore.

**Purpose:**  
The "V" operation (from Dijkstra's original notation) - increases semaphore value. It solves the problem of releasing a resource, signaling that it's available. When value > 0, waiting processes can proceed. This is how philosophers "put back" forks.

**Usage:**
```c
#include <semaphore.h>

sem_t *forks = sem_open("/philo_forks", 0);

// Philosopher puts down forks (releases 2 resources)
sem_post(forks);  // Put back left fork
sem_post(forks);  // Put back right fork

// Signal that philosopher ate
sem_t *meal_sem = sem_open("/meal_count", 0);
sem_post(meal_sem);  // Increment meal counter

// Unlock print semaphore
sem_t *print_lock = sem_open("/print_lock", 0);
sem_post(print_lock);  // Allow others to print
```

---

#### `sem_wait`

**Description:**  
Decrements (locks) a semaphore, blocking if value is 0.

**Purpose:**  
The "P" operation (from Dijkstra's "proberen" = test) - decreases semaphore value. It solves the problem of acquiring a resource: if value > 0, decrement and proceed; if 0, block until another process posts. This prevents race conditions and implements resource counting.

**Usage:**
```c
#include <semaphore.h>

sem_t *forks = sem_open("/philo_forks", 0);

// Philosopher takes forks (needs 2)
sem_wait(forks);  // Take first fork (blocks if none available)
printf("Has taken a fork\n");
sem_wait(forks);  // Take second fork
printf("Has taken a fork\n");

// Eat
eat();

// Release forks
sem_post(forks);
sem_post(forks);
```

---

#### `sem_unlink`

**Description:**  
Removes a named semaphore from the system.

**Purpose:**  
Created to solve the persistence problem of named semaphores. Unlike regular variables, named semaphores persist after all processes exit (stored in /dev/shm/ on Linux). `sem_unlink` removes the name so future opens won't find it, and the semaphore is destroyed when last reference closes.

**Usage:**
```c
#include <semaphore.h>

// Cleanup at program end (usually parent process)
sem_unlink("/philo_forks");
sem_unlink("/print_lock");
sem_unlink("/death_lock");

// Cleanup per-philosopher semaphores
for (int i = 0; i < num_philosophers; i++) {
    char name[20];
    sprintf(name, "/philo_%d", i);
    sem_unlink(name);
}

// Note: semaphores still work for processes that already opened them
// They just can't be opened by new processes after unlink
```

---

## Quick Reference Table

| Category | Function | Core Purpose |
|----------|----------|--------------|
| Memory | `memset` | Initialize memory blocks |
| Memory | `malloc`/`free` | Dynamic memory allocation |
| I/O | `printf`/`write` | Output text/data |
| Time | `gettimeofday` | High-res timestamps |
| Time | `usleep` | Precise delays |
| Threads | `pthread_create` | Create concurrent threads |
| Threads | `pthread_join` | Wait for thread completion |
| Threads | `pthread_detach` | Auto-cleanup threads |
| Sync | `pthread_mutex_*` | Thread mutual exclusion |
| Process | `fork` | Create new process |
| Process | `exit` | Terminate process |
| Process | `kill` | Signal other processes |
| Process | `waitpid` | Reap child processes |
| Sync | `sem_open`/`close` | Create/close semaphores |
| Sync | `sem_wait`/`post` | Semaphore lock/unlock |
| Sync | `sem_unlink` | Remove named semaphore |

---

## Key Differences: Mutex vs Semaphore

| Aspect | Mutex | Semaphore |
|--------|-------|-----------|
| Scope | Threads in one process | Any process (named) or threads |
| Value | Binary (locked/unlocked) | Counting (0 to N) |
| Owner | Must be unlocked by locker | Can be posted by any process |
| Use Case | Exclusive resource access | Resource counting, signaling |
| Lifetime | Process | System-wide (until unlinked) |

---

## Practical Implementation Patterns

Real-world patterns observed across high-scoring 42 student submissions (125/100). These sections cover how the allowed functions are composed together to solve the dining philosophers problem.

---

### Core Data Structures

Every successful implementation uses two key structs. The shared `t_data` holds simulation-wide state, while `t_philo` holds per-philosopher state:

```c
typedef struct s_data {
    int             nb_philos;
    int             time_to_die;
    int             time_to_eat;
    int             time_to_sleep;
    int             must_eat_count;
    unsigned long   start_time;
    int             someone_died;
    pthread_mutex_t *forks;
    pthread_mutex_t print_mutex;
    pthread_mutex_t death_mutex;
    t_philo         *philos;
    pthread_t       monitor_thread;
} t_data;

typedef struct s_philo {
    int             id;
    int             meals_eaten;
    unsigned long   last_meal_time;
    pthread_mutex_t meal_mutex;
    pthread_t       thread;
    t_data          *data;
} t_philo;
```

**Mutex roles:**

| Mutex | Protects | Why |
|-------|----------|-----|
| `forks[i]` | Fork i's availability | Only one philosopher holds a fork at a time |
| `print_mutex` | `printf` output | Prevents interleaved messages from concurrent threads |
| `death_mutex` | `someone_died` flag | Monitor sets it, philosophers read it — data race prevention |
| `meal_mutex` (per philo) | `last_meal_time`, `meals_eaten` | Monitor reads these while philosopher writes them |

---

### Smart `usleep` — The Polling Sleep Pattern

`usleep()` is imprecise — it may overshoot by several milliseconds. More critically, a philosopher sleeping for `time_to_sleep` ms must still react immediately if another philosopher dies. The solution is a polling loop:

```c
unsigned long get_time_ms(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void smart_sleep(unsigned long duration_ms, t_philo *philo)
{
    unsigned long start;

    start = get_time_ms();
    while (get_time_ms() - start < duration_ms)
    {
        if (is_dead(philo))
            return;
        usleep(500);
    }
}
```

**Why `usleep(500)` (0.5ms) per iteration?**
- Small enough for <10ms death detection accuracy
- Large enough to avoid burning CPU in a tight loop
- The monitor thread checks every ~0.5ms, and `smart_sleep` also exits within 0.5ms of a death

---

### Fork Pickup Order — Deadlock Avoidance

The classic deadlock scenario: every philosopher grabs their left fork simultaneously, then waits forever for the right fork. Every 42 implementation solves this with **asymmetric pickup order** based on philosopher ID parity:

```c
void take_forks(t_philo *philo)
{
    int left = philo->id - 1;
    int right = philo->id % philo->data->nb_philos;
    pthread_mutex_t *first;
    pthread_mutex_t *second;

    if (philo->id % 2 == 0)
    {
        first = &philo->data->forks[left];
        second = &philo->data->forks[right];
    }
    else
    {
        first = &philo->data->forks[right];
        second = &philo->data->forks[left];
    }
    pthread_mutex_lock(first);
    print_status(philo, "has taken a fork");
    pthread_mutex_lock(second);
    print_status(philo, "has taken a fork");
}

void put_forks(t_philo *philo)
{
    int left = philo->id - 1;
    int right = philo->id % philo->data->nb_philos;

    pthread_mutex_unlock(&philo->data->forks[left]);
    pthread_mutex_unlock(&philo->data->forks[right]);
}
```

**Why this works:** It breaks the "circular wait" condition — one of the four necessary conditions for deadlock. Even-numbered philosophers go left→right, odd-numbered go right→left, so no circular dependency chain forms.

**Alternative approach (lower-indexed fork first):**
```c
if (left < right)
{
    pthread_mutex_lock(&forks[left]);
    pthread_mutex_lock(&forks[right]);
}
else
{
    pthread_mutex_lock(&forks[right]);
    pthread_mutex_lock(&forks[left]);
}
```

---

### Thread-Safe Printing

`printf` is not atomic — concurrent calls produce interleaved output. Every implementation protects it with a mutex:

```c
void print_status(t_philo *philo, char *msg)
{
    pthread_mutex_lock(&philo->data->print_mutex);
    if (!is_dead(philo))
        printf("%lu %d %s\n",
               get_time_ms() - philo->data->start_time,
               philo->id, msg);
    pthread_mutex_unlock(&philo->data->print_mutex);
}
```

**Death message exception:** The death message must print even when `someone_died == 1`, because the philosopher that died is the one setting the flag. Check explicitly:

```c
void print_status(t_philo *philo, char *msg)
{
    pthread_mutex_lock(&philo->data->print_mutex);
    if (!is_dead(philo) || strcmp(msg, "died") == 0)
        printf("%lu %d %s\n",
               get_time_ms() - philo->data->start_time,
               philo->id, msg);
    pthread_mutex_unlock(&philo->data->print_mutex);
}
```

---

### Death Monitor — Separate Thread Pattern

A dedicated monitor thread continuously checks whether any philosopher has starved. This runs in the main thread or a separate thread:

```c
void *monitor(void *arg)
{
    t_data *data = (t_data *)arg;
    int i;

    while (1)
    {
        i = 0;
        while (i < data->nb_philos)
        {
            pthread_mutex_lock(&data->philos[i].meal_mutex);
            if (get_time_ms() - data->philos[i].last_meal_time
                >= (unsigned long)data->time_to_die)
            {
                pthread_mutex_unlock(&data->philos[i].meal_mutex);
                pthread_mutex_lock(&data->death_mutex);
                data->someone_died = 1;
                pthread_mutex_unlock(&data->death_mutex);
                print_status(&data->philos[i], "died");
                return (NULL);
            }
            pthread_mutex_unlock(&data->philos[i].meal_mutex);
            i++;
        }
        if (all_ate_enough(data))
            return (NULL);
        usleep(500);
    }
}
```

**Key timing insight:** The monitor must read `last_meal_time` while the philosopher thread writes it — this is a data race. The per-philosopher `meal_mutex` prevents this.

---

### Philosopher Routine — Full Lifecycle

The main loop that each philosopher thread executes:

```c
void *routine(void *arg)
{
    t_philo *philo = (t_philo *)arg;

    if (philo->id % 2 == 0)
        usleep(1000);
    while (!is_dead(philo))
    {
        take_forks(philo);
        pthread_mutex_lock(&philo->meal_mutex);
        philo->last_meal_time = get_time_ms();
        philo->meals_eaten++;
        pthread_mutex_unlock(&philo->meal_mutex);
        print_status(philo, "is eating");
        smart_sleep(philo->data->time_to_eat, philo);
        put_forks(philo);
        if (philo->data->must_eat_count > 0
            && philo->meals_eaten >= philo->data->must_eat_count)
            break;
        print_status(philo, "is sleeping");
        smart_sleep(philo->data->time_to_sleep, philo);
        print_status(philo, "is thinking");
    }
    return (NULL);
}
```

**Why `usleep(1000)` for even IDs?** Staggering start prevents all philosophers from rushing for forks at the same instant, reducing contention and the chance of starvation for odd-numbered philosophers.

---

### Single Philosopher — Special Case

With only 1 philosopher, there is only 1 fork. The philosopher can never eat and must die:

```c
if (data.nb_philos == 1)
{
    print_status(&data.philos[0], "has taken a fork");
    smart_sleep(data.time_to_die, &data.philos[0]);
    print_status(&data.philos[0], "died");
    // cleanup and exit
}
```

---

### Main Thread Launch Sequence

The recommended order for thread creation and joining:

```c
// 1. Record start time
data.start_time = get_time_ms();

// 2. Initialize last_meal_time for all philosophers
for (int i = 0; i < data.nb_philos; i++)
    data.philos[i].last_meal_time = data.start_time;

// 3. Create philosopher threads
for (int i = 0; i < data.nb_philos; i++)
    pthread_create(&data.philos[i].thread, NULL, routine, &data.philos[i]);

// 4. Create monitor thread
pthread_create(&data.monitor_thread, NULL, monitor, &data);

// 5. Join all threads (blocks until simulation ends)
for (int i = 0; i < data.nb_philos; i++)
    pthread_join(data.philos[i].thread, NULL);
pthread_join(data.monitor_thread, NULL);

// 6. Destroy all mutexes, free all memory
```

**Order matters:** The monitor thread must be created *after* all philosopher threads to avoid checking uninitialized philosophers. Joining blocks the main thread until everything finishes.

---

### Cleanup — Preventing Leaks

Every mutex must be destroyed, every allocation freed. A systematic approach:

```c
void cleanup(t_data *data)
{
    int i;

    i = 0;
    while (i < data->nb_philos)
    {
        pthread_mutex_destroy(&data->philos[i].meal_mutex);
        i++;
    }
    i = 0;
    while (i < data->nb_philos)
    {
        pthread_mutex_destroy(&data->forks[i]);
        i++;
    }
    pthread_mutex_destroy(&data->print_mutex);
    pthread_mutex_destroy(&data->death_mutex);
    free(data->forks);
    free(data->philos);
}
```

---

## Common Pitfalls & Solutions

### 1. Data Race on `last_meal_time`

```c
// WRONG — no protection, monitor reads while philosopher writes
philo->last_meal_time = get_time_ms();

// CORRECT — mutex-protected
pthread_mutex_lock(&philo->meal_mutex);
philo->last_meal_time = get_time_ms();
philo->meals_eaten++;
pthread_mutex_unlock(&philo->meal_mutex);
```

### 2. Printing Death After Another Thread Already Printed Death

```c
// WRONG — every thread prints death
if (someone_died)
    printf("died\n");

// CORRECT — only the first thread to detect death prints
pthread_mutex_lock(&print_mutex);
if (!someone_died)
{
    someone_died = 1;
    printf("%lu %d died\n", elapsed, id);
}
pthread_mutex_unlock(&print_mutex);
```

### 3. Deadlock From Symmetric Fork Pickup

```c
// WRONG — all philosophers grab left then right = circular wait
pthread_mutex_lock(&forks[left]);
pthread_mutex_lock(&forks[right]);

// CORRECT — use asymmetric or index-based ordering (see Fork Pickup section)
```

### 4. `usleep` Overshoot Causing False Deaths

```c
// WRONG — usleep(200000) may sleep 205ms, causing death detection miss
usleep(time_to_eat * 1000);

// CORRECT — polling sleep that checks death flag
smart_sleep(time_to_eat, philo);
```

### 5. Starvation With Odd Philosopher Counts

With 3+ philosophers, some may never get both forks. The stagger start (`usleep(1000)` for even IDs) plus the even/odd fork ordering ensures fairness. Some implementations also add a small "thinking" delay to prevent a fast philosopher from monopolizing forks.

### 6. Joining Threads Before Monitor Finishes

If the monitor detects death and returns, philosopher threads might still be running. Always join *all* threads including the monitor:

```c
for (int i = 0; i < data.nb_philos; i++)
    pthread_join(data.philos[i].thread, NULL);
pthread_join(data.monitor_thread, NULL);
```

---

## Testing Commands

```bash
# Should not die — plenty of time
./philo 5 800 200 200

# Should not die — tight but survivable
./philo 4 410 200 200

# Should die — 310 < 200 + 100
./philo 4 310 200 100

# Single philosopher — must die (only one fork)
./philo 1 800 200 200

# With meal limit — stops when all ate 7 times
./philo 5 800 200 200 7

# Stress test — many philosophers
./philo 200 800 200 200

# Two philosophers — borderline
./philo 2 400 200 200

# Very short times — test timing precision
./philo 3 60 60 60
```
