# OOP Principles — Interview Reference

These are the four pillars of OOP plus SOLID. For each one: what it is, why it matters, and how it shows up in this project.

---

## The Four Pillars

### 1. Encapsulation
**What:** Bundle data and the methods that operate on it inside a class. Hide internal state from the outside world.

**Why:** Prevents outside code from putting an object into an invalid state. You control *how* state changes.

**In C++:**
```cpp
class Task {
private:
    std::atomic<TaskStatus> status_;  // hidden — no one sets this directly

protected:
    void markRunning() { status_.store(TaskStatus::Running); }  // only subclasses can transition

public:
    TaskStatus getStatus() const { return status_.load(); }  // read-only to the outside
};
```

**Interview answer:** "Encapsulation means the class owns its invariants. Outside code can observe state through a public interface, but can't corrupt it by writing directly to internal fields."

---

### 2. Abstraction
**What:** Expose *what* something does, not *how* it does it. Define an interface, hide the implementation.

**Why:** Callers depend on the interface, not the concrete class. You can swap implementations without changing callers.

**In C++:** Abstract classes with pure virtual methods.
```cpp
class Task {
public:
    virtual void execute() = 0;  // pure virtual — defines the contract
};

// The ThreadPool calls task->execute() without knowing what the task does
```

**Key distinction from encapsulation:** Encapsulation hides *data*. Abstraction hides *complexity*.

---

### 3. Inheritance
**What:** A derived class acquires the properties and behavior of a base class, then extends or overrides them.

**Why:** Reuse code and model "is-a" relationships.

**In C++:**
```cpp
class PrintTask : public Task {           // PrintTask IS-A Task
public:
    PrintTask(std::string msg)
        : Task("PrintTask"), msg_(std::move(msg)) {}

    void execute() override {             // override keyword catches typos at compile time
        markRunning();
        std::cout << msg_ << "\n";
        markCompleted();
    }
private:
    std::string msg_;
};
```

**Watch out for:** Prefer composition over inheritance when the relationship is "has-a" not "is-a". Deep inheritance hierarchies become fragile.

---

### 4. Polymorphism
**What:** One interface, many forms. A base class pointer/reference can refer to any derived type, and the right method gets called at runtime.

**Types:**
- **Runtime (dynamic) polymorphism:** `virtual` functions, resolved via vtable at runtime.
- **Compile-time (static) polymorphism:** Templates, resolved at compile time.

**In C++:**
```cpp
std::vector<std::shared_ptr<Task>> tasks;
tasks.push_back(std::make_shared<PrintTask>("hello"));
tasks.push_back(std::make_shared<ComputeTask>(42));

for (auto& t : tasks) {
    t->execute();  // correct execute() called for each type — runtime dispatch via vtable
}
```

**Interview answer:** "Polymorphism lets you write code against an abstraction. The scheduler holds `shared_ptr<Task>` and calls `execute()` — it doesn't know or care whether it's a PrintTask or a NetworkTask. Adding a new task type requires zero changes to the scheduler."

---

## SOLID Principles

### S — Single Responsibility
Each class has one reason to change.

- `Task` — defines what a job is and tracks its state
- `ThreadSafeQueue` — manages concurrent access to a queue
- `Worker` — runs a thread that pulls and executes tasks
- `ThreadPool` — manages the lifecycle of workers
- `Scheduler` — decides priority and dispatches to the pool

If the queue logic lived inside Worker, changing the queue implementation would force changes to Worker. Separating them isolates the impact of change.

---

### O — Open/Closed
Open for extension, closed for modification.

We can add a `NetworkTask`, `DatabaseTask`, or `TimedTask` without touching `Task.h`, `Worker.cpp`, or `ThreadPool.cpp`. The base class defines the contract; new behavior comes from new subclasses.

---

### L — Liskov Substitution
Any derived class must be usable wherever the base class is expected, without breaking correctness.

A `PrintTask` passed as a `Task*` must behave as a valid `Task` — it calls `markRunning()` before work and `markCompleted()` or `markFailed()` after. If a subclass *skips* these transitions, it violates LSP and breaks any code that monitors task status.

---

### I — Interface Segregation
Don't force classes to implement interfaces they don't need.

If we had one giant `ISchedulable` interface with `execute()`, `pause()`, `resume()`, `cancel()`, `serialize()`... most tasks would have to stub out methods they don't use. Better to keep the base interface minimal (`execute()` + `describe()`) and add opt-in interfaces as needed.

---

### D — Dependency Inversion
High-level modules should not depend on low-level modules. Both should depend on abstractions.

`ThreadPool` depends on `Task` (the abstraction), not on `PrintTask` (the concrete class). If we later change how tasks are implemented, `ThreadPool` doesn't change.

---

## Quick mental model

```
Encapsulation  → protect state from outside corruption
Abstraction    → hide complexity behind an interface
Inheritance    → reuse and extend behavior ("is-a")
Polymorphism   → one interface, correct behavior at runtime

SOLID          → guidelines to keep OOP code maintainable as it grows
```

---

## Common interview traps

| Question | Wrong answer | Right answer |
|----------|-------------|--------------|
| "What's the difference between abstraction and encapsulation?" | "They're the same thing" | "Abstraction hides complexity (the what). Encapsulation hides data (the how it's stored)." |
| "Why use virtual functions?" | "For inheritance" | "For runtime polymorphism — so you can call the right method through a base class pointer without knowing the concrete type." |
| "What's a vtable?" | "I don't know" | "A per-class table of function pointers that the compiler generates. When you call a virtual function, the CPU looks up the function pointer in the vtable of the actual object's type." |
| "When NOT to use inheritance?" | "Always use it" | "When the relationship is 'has-a' not 'is-a'. Prefer composition — it's more flexible and avoids the fragile base class problem." |
