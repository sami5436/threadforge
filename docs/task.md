# Task — Interview Reference

The `Task` class is the abstract base for any unit of work the scheduler can run. Every concrete task type (PrintTask, ComputeTask, FailingTask, …) inherits from it.

This is the **Open/Closed** principle in action: the rest of the system (queue, workers, pool) only knows about `Task` — adding a new kind of work means writing a new subclass, not modifying the framework.

---

## What `Task` provides

| Member | Purpose |
|--------|---------|
| `name_` | Human-readable identifier — used for logs and the visualizer |
| `priority_` | Integer; higher = more urgent. The pool's priority queue uses this. |
| `status_` | `Pending → Running → Completed/Failed`. Atomic so the UI thread can read while a worker writes. |
| `createdAt_` | `steady_clock` timestamp; useful for measuring queue wait time |
| `execute()` | Pure virtual — the actual work. Subclasses must override. |
| `describe()` | Virtual; default formats the name + priority. Override for richer logs. |
| `markRunning/Completed/Failed()` | Protected; subclasses call these to advance state |

---

## Why each design choice matters (interview talking points)

### Pure virtual `execute()`
```cpp
virtual void execute() = 0;
```
Forces every subclass to define its own work. You can't instantiate a `Task` directly — the compiler stops you. This makes `Task` a **contract**, not a default.

### Virtual destructor
```cpp
virtual ~Task() = default;
```
**Critical.** When you delete a derived object through a base pointer:
```cpp
std::shared_ptr<Task> t = std::make_shared<PrintTask>("hi");
// when t goes out of scope, ~PrintTask must run, then ~Task
```
Without a virtual destructor, only `~Task` would run and `PrintTask`'s members would leak. Any class meant to be inherited from must have a virtual destructor.

### `std::atomic<TaskStatus>` for status
The worker thread writes the status; another thread (a monitor, a UI, the test harness) reads it. Without atomicity that's a data race — undefined behavior. `atomic<enum>` gives lock-free thread-safe reads/writes for a single value, no mutex needed.

### Deleted copy constructor & assignment
```cpp
Task(const Task&)            = delete;
Task& operator=(const Task&) = delete;
```
A task has identity — you don't want two copies of the same task floating around (and you don't want **slicing**, where copying a `PrintTask` into a `Task` strips the derived parts). Deleting copy operations enforces "tasks are managed by `shared_ptr`, not copied."

### Protected state-change helpers
`markRunning()`, `markCompleted()`, `markFailed()` are `protected` — only the task itself (and its subclasses) can transition state. Outside code can only **observe** status. That's encapsulation: the class owns its invariants.

---

## Lifecycle

```
constructor
   │
   ▼
[Pending]  ── pushed onto queue, waiting for a worker
   │
   ▼ (worker pulls task, calls execute())
[Running]  ── markRunning() at top of execute()
   │
   ├──▶ markCompleted() ──▶ [Completed]   normal path
   │
   └──▶ markFailed()    ──▶ [Failed]      caught exception
```

A well-behaved subclass calls `markRunning()` first, then exactly one of `markCompleted()` or `markFailed()`. Skipping these transitions violates the **Liskov Substitution Principle** — outside code that monitors status would see tasks "stuck" in Pending.

---

## Concrete subclasses in this project

| Class | What it does | Why it's here |
|-------|--------------|---------------|
| `PrintTask` | Prints a message and sleeps briefly | Simplest possible task — proves the framework works |
| `ComputeTask` | Sums 1..N | Shows CPU-bound work with a real result |
| `FailingTask` | Throws inside `execute()`, catches it, calls `markFailed()` | Shows the failure path; lets us verify the pool keeps running after a failure |

---

## Common interview questions about this design

**Q: Why use `shared_ptr<Task>` everywhere instead of `Task*` or `unique_ptr<Task>`?**
A: The same task may be referenced by the queue, a worker, and a status monitor at the same time. `shared_ptr` handles that lifetime safely. `unique_ptr` would force an ownership transfer at every step, which makes the API awkward. Raw pointers leak.

**Q: Why is `priority_` not atomic?**
A: It's set once in the constructor and never changes. Immutable data after construction is automatically thread-safe — no synchronization needed.

**Q: What if `execute()` throws?**
A: The worker catches it, marks the task failed, logs it, and moves on. One bad task should not bring down a worker thread. (See `Worker::loop` in the source.)

**Q: Why is `Task` non-copyable but the queue still works?**
A: The queue stores `shared_ptr<Task>`, not `Task` by value. Copying a `shared_ptr` just bumps a refcount — the underlying `Task` is never copied or sliced.
