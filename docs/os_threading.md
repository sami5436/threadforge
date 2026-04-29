# OS & Multithreading Concepts — Interview Reference

---

## Processes vs Threads

| | Process | Thread |
|-|---------|--------|
| Memory | Own address space | Shared address space within a process |
| Cost | Expensive to create (fork) | Cheap to create |
| Communication | IPC (pipes, sockets, shared memory) | Direct (shared variables — carefully) |
| Crash impact | Only kills that process | Can crash the whole process |
| Example | Chrome tabs | Chrome's renderer threads within a tab |

**When to use threads over processes:** When tasks need to share data efficiently and you can manage synchronization. Threads avoid the overhead of serializing data across process boundaries.

---

## What is a Thread?

A thread is an independent sequence of execution within a process. All threads in a process share:
- Heap memory
- Global variables
- File descriptors

Each thread has its own:
- Stack (local variables, function call frames)
- Program counter (where it is in execution)
- Register state

In C++11+:
```cpp
std::thread t([]() {
    std::cout << "I run concurrently\n";
});
t.join();  // wait for t to finish before continuing
```

**`join()` vs `detach()`:**
- `join()` — caller blocks until the thread finishes. Safe, predictable.
- `detach()` — thread runs independently, caller doesn't wait. Dangerous if the thread accesses memory that gets freed.

---

## Race Conditions

A race condition occurs when the result of a program depends on the interleaving of thread execution — and at least one thread writes.

**Classic example:**
```cpp
int counter = 0;

// Thread 1 and Thread 2 both run:
counter++;  // NOT atomic — this is actually 3 operations:
            // 1. load counter into register
            // 2. add 1
            // 3. store back to memory
```

If both threads load `0`, both add `1`, and both store `1` — you get `1` instead of `2`. The increment was lost.

**How to detect:** Data races are undefined behavior in C++. Tools:
- `ThreadSanitizer` (`-fsanitize=thread`) — catches races at runtime
- `Helgrind` (Valgrind tool) — similar

---

## Mutexes (Mutual Exclusion)

A mutex ensures only one thread executes a critical section at a time.

```cpp
std::mutex mtx;
int counter = 0;

void increment() {
    std::lock_guard<std::mutex> lock(mtx);  // locks on construction
    counter++;
    // lock released automatically when lock_guard goes out of scope (RAII)
}
```

**`lock_guard` vs `unique_lock`:**
- `lock_guard` — simple, cannot be manually unlocked, cannot be used with condition variables
- `unique_lock` — more flexible, can unlock/relock, required for condition variables

**Deadlock:** Two threads each hold a lock the other needs, so both wait forever.
```
Thread A holds Lock 1, waits for Lock 2
Thread B holds Lock 2, waits for Lock 1  → deadlock
```

**How to prevent deadlock:**
1. Always acquire locks in the same order across all threads
2. Use `std::lock(m1, m2)` to acquire multiple mutexes atomically
3. Keep critical sections short — hold locks for as little time as possible
4. Use `std::scoped_lock` (C++17) which handles multiple locks safely

---

## Condition Variables

A condition variable lets a thread sleep and wait until another thread signals that something has changed.

```cpp
std::mutex mtx;
std::condition_variable cv;
bool dataReady = false;

// Consumer thread:
std::unique_lock<std::mutex> lock(mtx);
cv.wait(lock, []{ return dataReady; });  // atomically releases lock and sleeps
// wakes up when dataReady == true, re-acquires lock
processData();

// Producer thread:
{
    std::lock_guard<std::mutex> lock(mtx);
    dataReady = true;
}
cv.notify_one();  // wake up one waiting thread
```

**Why not just `while (!dataReady) {}`?** That's a spin loop — it burns 100% CPU doing nothing useful. Condition variables let the OS scheduler sleep the thread and wake it only when needed.

**Spurious wakeups:** A thread can wake up from `cv.wait()` even without a `notify`. Always use a predicate (the lambda) — `cv.wait(lock, predicate)` re-checks it automatically.

---

## std::atomic

For simple types (integers, booleans), `std::atomic` gives you thread-safe reads/writes without a mutex.

```cpp
std::atomic<int> counter{0};
counter++;       // atomic fetch-and-add, no race condition
counter.load();  // atomic read
counter.store(5); // atomic write
```

**When to use atomic vs mutex:**
- `atomic` — single variable, simple operations (increment, flag, status)
- `mutex` — protecting a block of code or multiple related variables that must change together

---

## The Producer-Consumer Pattern

This is the core pattern in ThreadForge (and in most real systems).

```
[Producer threads] → [Shared Queue] → [Consumer/Worker threads]
```

- Producers add tasks to the queue
- Consumers (workers) pull tasks and execute them
- The queue must be thread-safe

Requirements:
- Mutex to protect the queue data structure
- Condition variable to wake workers when a task arrives
- Condition variable (or atomic flag) to signal shutdown

This pattern is how: web servers handle requests, OS kernels schedule syscalls, game engines dispatch physics/render work, message brokers (Kafka, RabbitMQ) work.

---

## RAII (Resource Acquisition Is Initialization)

Not threading-specific, but critical in multithreaded code.

**Idea:** Tie resource lifetime to object lifetime. When the object is constructed, acquire the resource. When it's destroyed (goes out of scope), release it.

```cpp
// Without RAII:
mtx.lock();
doWork();      // if this throws, mtx is never unlocked → deadlock
mtx.unlock();

// With RAII (lock_guard):
{
    std::lock_guard<std::mutex> lock(mtx);
    doWork();  // even if this throws, lock_guard's destructor unlocks mtx
}
```

RAII makes resource management exception-safe and eliminates whole classes of bugs (leaked locks, leaked memory, leaked file handles).

---

## Thread Lifecycle (in this project)

```
main()
  └─ ThreadPool created
       └─ N Worker objects created
            └─ Each Worker spawns a std::thread
                 └─ Thread loops: wait for task → execute → repeat
  └─ Tasks submitted to ThreadSafeQueue
  └─ Workers wake up, pull tasks, execute them
  └─ ThreadPool::shutdown() called
       └─ Sets stop flag, notifies all workers
       └─ Workers exit their loop
       └─ ThreadPool joins all threads (waits for completion)
```

---

## Quick cheat sheet for interviews

| Concept | One-line answer |
|---------|----------------|
| Race condition | Two threads access shared data with no synchronization and at least one writes |
| Mutex | Ensures only one thread enters a critical section at a time |
| Deadlock | Two threads wait on each other's locks forever |
| Condition variable | Lets a thread sleep until another signals a state change |
| Spurious wakeup | A thread waking from cv.wait() without notify — always use a predicate |
| RAII | Bind resource lifetime to object lifetime so cleanup is automatic |
| atomic | Single-variable thread safety without a mutex |
| Thread pool | Fixed set of reusable worker threads to avoid spawn/destroy overhead |
