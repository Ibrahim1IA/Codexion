*This project has been created as part of the 42 curriculum by iissoufo.*

# Codexion

## Description

Codexion is a concurrency simulation written in C with POSIX threads.

`N` coders sit around a circular table with `N` USB dongles, one between each
pair of neighbours. Each coder loops through three states:
**compile → debug → refactor**. To compile, a coder must hold **both** the
dongle on their left and the dongle on their right. A coder who has not
*started* compiling within `time_to_burnout` ms of their last compile start
(or of the simulation start) **burns out**, and the simulation stops.

The project is a variation of Dijkstra's *Dining Philosophers* problem, with
three extra constraints:

- **Dongle cooldown**: a released dongle cannot be taken again before
  `dongle_cooldown` ms have passed.
- **Fair arbitration**: when several coders want the same dongle, the dongle
  serves them according to a scheduling policy, either `fifo` (arrival order)
  or `edf` (Earliest Deadline First, where
  `deadline = last_compile_start + time_to_burnout`).
- **A custom priority queue** (a binary min-heap) for each dongle that
  implements that policy.

The simulation ends when a coder burns out, or when every coder has compiled at
least `number_of_compiles_required` times.

## Instructions

### Compilation

```bash
make        # builds ./codexion
make clean  # removes object files
make fclean # removes object files and the binary
make re     # full rebuild
```

The project compiles with `cc -Wall -Wextra -Werror -pthread`.

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Meaning |
|---|---|
| `number_of_coders` | Number of coders, which is also the number of dongles (≥ 1) |
| `time_to_burnout` | Max delay in ms between two compile starts |
| `time_to_compile` | Compile duration in ms (both dongles are held) |
| `time_to_debug` | Debug duration in ms |
| `time_to_refactor` | Refactor duration in ms |
| `number_of_compiles_required` | Simulation stops when every coder reaches it |
| `dongle_cooldown` | Delay in ms before a released dongle can be taken again (may be `0`) |
| `scheduler` | `fifo` or `edf` |

All arguments are mandatory. The program rejects:
- a wrong number of arguments;
- negative values and non-numeric values;
- values above `INT_MAX`;
- `0` for every argument except `dongle_cooldown`;
- any scheduler other than `fifo` or `edf`.

### Examples

```bash
./codexion 5 2000 200 200 200 10 0 fifo    # nobody burns out, stops after 10 compiles each
./codexion 5 2000 200 200 200 7 0 edf      # same with edf
./codexion 5 3000 200 200 200 10 400 fifo  # cooldown of 400 ms, nobody burns out
./codexion 200 250 60 60 60 3 60 fifo      # 200 coders, tight timing, survives
./codexion 5 500 200 200 200 10 0 fifo     # infeasible (cycle 600 > 500): burns out at ~500 ms
./codexion 1 800 200 200 200 10 0 fifo     # one coder, one dongle: burns out at ~800 ms
```

### Output format

```
timestamp_in_ms X has taken a dongle
timestamp_in_ms X is compiling
timestamp_in_ms X is debugging
timestamp_in_ms X is refactoring
timestamp_in_ms X burned out
```

### Project layout

| File | Content |
|---|---|
| `includes/codexion.h` | Structures (`t_params`, `t_request`, `t_min_heap`, `t_dongle`, `t_coder`, `t_table`) and prototypes |
| `src/main.c` | Entry point |
| `src/parser.c` | Argument validation and parsing |
| `src/init.c` | Mutex initialisation, simulation start-up, thread joins |
| `src/coder.c` | Coder structures and thread creation |
| `src/routine.c` | Coder thread loop (compile / debug / refactor), precise sleep |
| `src/dongle.c` | Dongle allocation, destruction and release |
| `src/dongle2.c` | Atomic acquisition of both dongles |
| `src/heap.c` | Binary min-heap (priority queue) |
| `src/monitor.c` | Monitor thread: burnout detection and end of simulation |
| `src/utils.c`, `src/utils2.c` | Time helpers, logging, priority comparison, cleanup |

## Blocking cases handled

### Deadlock: Coffman's conditions

A deadlock requires all four Coffman conditions at the same time. Codexion
breaks two of them.

1. **Mutual exclusion**: kept on purpose, since a dongle can only be in one
   coder's hands.
2. **Hold and wait**: **broken.** Both dongles are taken **atomically**. Inside
   one critical section (`dongle_mutex`), a coder checks that both dongles are
   free and out of cooldown, and only then takes both. Otherwise the coder
   takes nothing and waits. A coder never holds one dongle while waiting for
   the other.
3. **No preemption**: kept. Dongles are only released after compiling.
4. **Circular wait**: **broken.** A coder's request is pushed with the *same*
   key (`request_time`, `deadline`, `coder_id`) into the queues of both of its
   dongles. The comparison is a strict total order, because `coder_id` is the
   final tie-breaker. The highest-priority waiting coder is therefore at the
   head of both of its queues, so a cycle of coders waiting on each other
   cannot form.

### Starvation and fairness

- Each dongle has its own priority queue (`t_min_heap`). A coder may take the
  dongles only when they are **at the head of both queues**. A newcomer cannot
  skip ahead of an older request (`fifo`) or of a more urgent one (`edf`).
- `fifo`: ordered by `request_time`, then `coder_id`.
- `edf`: ordered by `deadline`, then `request_time`, then `coder_id`. The
  tie-breaker makes EDF fully deterministic.
- **Staggered start**: even-numbered coders wait `time_to_compile` ms before
  their first request. Odd-numbered coders are then guaranteed to be first in
  their queues, and the table settles into an alternating pattern from the
  beginning. Without this, with many threads, an even coder could queue before
  its odd neighbour, and that delay would cascade along the table.

### Dongle cooldown

When a dongle is released, it stores `cooldown_until = now + dongle_cooldown`.
A coder at the head of both queues whose dongles are still cooling down
sleeps with `pthread_cond_timedwait` until the later of the two
`cooldown_until`. It then re-checks the condition.

### Precise burnout detection

- A dedicated **monitor thread** checks every coder about every 1 ms. A
  burnout is when `now - last_compile_start > time_to_burnout`.
- `last_compile_start` is written by the coder and read by the monitor, both
  under that coder's `coder_mutex`.
- When a burnout is detected, the monitor first sets `simulation_ended`, then
  prints `burned out`. The message appears within a few milliseconds of the
  real deadline, well inside the 10 ms requirement.
- Coders sleep in small slices (`precise_sleep`) and check `simulation_ended`
  between slices, so every thread stops quickly.

### Log serialization

- Every message is printed while holding `log_mutex`. Two lines can never
  interleave.
- The two `has taken a dongle` lines and the `is compiling` line are printed
  together, in a single `log_mutex` critical section (`print_compile_start`).
  No other coder's line can appear between them, so every `is compiling` is
  immediately preceded by exactly two `has taken a dongle` lines of the same
  coder.
- The logging functions also check `simulation_ended` under `end_mutex`. Once
  the simulation has ended, only the `burned out` line can still be printed.
  No message appears after a burnout.

### Other edge cases

- **One coder**: there is only one dongle. The coder takes it, can never get a
  second one, and burns out after `time_to_burnout`.
- **End of simulation while waiting**: `stop_simulation` sets the flag, then
  signals every coder's condition variable under `dongle_mutex`. A waiting
  coder cannot miss this wake-up, because it holds `dongle_mutex` from its
  `simulation_ended` check until `pthread_cond_wait` atomically releases it.
- **Initialisation failures**: if a mutex, condition variable, allocation or
  thread creation fails, everything created so far is destroyed or joined
  before exiting.

## Thread synchronization mechanisms

| Primitive | Owner | Protects / role |
|---|---|---|
| `pthread_mutex_t dongle_mutex` | `t_table` | The state of **all** dongles (`is_available`, `cooldown_until`) and their waiting queues. It makes taking two dongles one atomic operation. |
| `pthread_cond_t cond` | each `t_coder` | Where a coder sleeps while waiting for its dongles. A release only signals the coders at the head of the released dongles' queues, instead of waking every waiting thread. |
| `pthread_mutex_t coder_mutex` | each `t_coder` | `last_compile_start`, `compiles_done` and `is_full`, shared between the coder and the monitor. |
| `pthread_mutex_t end_mutex` | `t_table` | The `simulation_ended` flag, read by every thread. |
| `pthread_mutex_t log_mutex` | `t_table` | Standard output. One line at a time. |
| `pthread_mutex_t start_mutex` | `t_table` | Start barrier. The main thread holds it while creating the threads, so every coder starts from the same `start_time`. |

### How a coder gets its dongles

```
lock(dongle_mutex)
push the same request into the left and right queues
while simulation not ended:
    if not head of both queues, or a dongle is taken:
        cond_wait(my_cond, dongle_mutex)                 # woken by a release
    else if a dongle is still in cooldown:
        cond_timedwait(my_cond, dongle_mutex, cooldown)  # woken by the timeout
    else:
        pop both queues, mark both dongles taken,
        set last_compile_start (under coder_mutex),
        log "has taken a dongle" x2 + "is compiling" in one log_mutex section
        unlock(dongle_mutex) and go compile
unlock(dongle_mutex)
```

Release, after compiling: lock `dongle_mutex`, mark both dongles available
with a new `cooldown_until`, signal the coder at the head of each queue, then
unlock.

### Preventing race conditions

- No variable shared between threads is read or written without its mutex. For
  example, the monitor never reads `last_compile_start` without
  `coder_mutex`, and coders never touch a dongle or a queue without
  `dongle_mutex`.
- **Lock ordering**: nested locks are always taken in the same order:
  `dongle_mutex → coder_mutex` and `dongle_mutex → log_mutex → end_mutex`.
  The reverse order is never used, so locks cannot deadlock.
- The code was checked with `valgrind --tool=helgrind`, `valgrind --tool=drd`
  and `-fsanitize=thread` (no data races, no lock-order errors), and with
  `valgrind --leak-check=full` (no leaks).

### Coder ↔ monitor communication

Coders and the monitor never call each other. They share state that is always
protected:

- coder → monitor: `last_compile_start`, `compiles_done` and `is_full`, under
  `coder_mutex`;
- monitor → coders: `simulation_ended` under `end_mutex`, plus a signal on
  each coder's condition variable so that sleeping coders notice the end
  immediately.

## Resources

- POSIX Threads manual pages: `pthread_mutex_lock(3)`, `pthread_cond_wait(3)`,
  `pthread_cond_timedwait(3)`.
- E. W. Dijkstra, *Hierarchical ordering of sequential processes* (1971), the
  origin of the Dining Philosophers problem.
- E. G. Coffman, M. Elphick, A. Shoshani, *System Deadlocks*, ACM Computing
  Surveys (1971), the four necessary conditions for deadlock.
- C. L. Liu, J. W. Layland, *Scheduling Algorithms for Multiprogramming in a
  Hard-Real-Time Environment* (1973), Earliest Deadline First scheduling.
- T. H. Cormen et al., *Introduction to Algorithms*, chapter "Heapsort"
  (binary heaps and priority queues).
- A. B. Downey, *The Little Book of Semaphores*.
- Valgrind documentation: Helgrind and DRD thread error detectors.

### Use of AI

An AI assistant (Claude) was used as a reviewer and debugging partner:

- to propose the **atomic acquisition** of both dongles (a single
  `dongle_mutex` instead of one mutex per dongle, plus one request pushed into
  both queues), replacing my first version, where a coder took one dongle and
  then waited for the second;
- to **diagnose a burnout** with 200 coders (`./codexion 200 250 60 60 60 3 60
  fifo`), which led to two fixes: the staggered start of even coders, and one
  condition variable per coder instead of a global broadcast;
- to **review the project against the subject and the evaluation scale**,
  mainly the norm, argument parsing, memory and thread checks, and the fact
  that the two `has taken a dongle` lines must be immediately followed by
  `is compiling`;
- to draft this README.

Every suggestion was reviewed, applied by hand, and tested with my own test
cases, Valgrind, Helgrind and DRD before being kept.