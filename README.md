# ThreadForge

A small, interview-focused C++ thread pool with a priority-aware task queue. Built as a study project for a Software Engineer I role on a real-time C++/Linux team — every class is a chance to talk about an OOP or threading concept.

## What's in here

```
threadforge/
├── include/
│   ├── Task.h                  # abstract base class for all work units
│   ├── Tasks.h                 # PrintTask / ComputeTask / FailingTask
│   ├── ThreadSafeQueue.h       # generic FIFO (mutex + cv pattern reference)
│   ├── ThreadSafeTaskQueue.h   # priority queue used by the pool
│   ├── Worker.h                # one std::thread, pulls + executes tasks
│   └── ThreadPool.h            # owns workers + queue, public API
├── src/
│   ├── Task.cpp
│   ├── Worker.cpp
│   ├── ThreadPool.cpp
│   └── main.cpp                # demo
├── docs/
│   ├── task.md                 # the Task contract + interview Q&A
│   ├── oop_principles.md       # the four pillars + SOLID
│   └── os_threading.md         # mutex, cv, atomic, RAII, deadlock
├── viz/
│   └── index.html              # interactive web visualizer (open in browser)
└── CMakeLists.txt
```

## Build & run

With clang directly (no extra tools):
```sh
clang++ -std=c++17 -Wall -Wextra -Wpedantic -Iinclude \
    src/main.cpp src/Task.cpp src/Worker.cpp src/ThreadPool.cpp \
    -o build/threadforge -pthread
./build/threadforge
```

With CMake (if you have it; gives clangd a `compile_commands.json`):
```sh
cmake -S . -B build && cmake --build build
./build/threadforge
```

Expected output (priority ordering may shuffle slightly — that's the point):
```
ThreadForge starting with 4 workers
[PrintTask] URGENT log line
[PrintTask] ALSO URGENT
[ComputeTask] sum(1..1000000) = 500000500000
[FailingTask] caught: simulated failure
[ComputeTask] sum(1..500000) = 125000250000
[PrintTask] normal traffic
ThreadForge done
```

## Visualize it

Open `viz/index.html` in any browser — no server needed. Tabs:
- **Live Simulator** — animated thread pool: submit tasks, watch them flow through the priority queue into workers and out to "completed"
- **OOP Pillars** — encapsulation / abstraction / inheritance / polymorphism, mapped to this codebase
- **Threading** — race conditions, mutexes, deadlock, condition variables — animated
- **Project Map** — class diagram of how Task/Queue/Worker/ThreadPool fit together
- **Code** — every header and source file with syntax highlighting

## Interview talking points by file

| File | What to talk about |
|------|--------------------|
| `Task.h` | abstract base class, pure virtual, **virtual destructor**, atomic status, deleted copy |
| `Tasks.h` | runtime polymorphism, override keyword, why FailingTask exists |
| `ThreadSafeTaskQueue.h` | producer-consumer, mutex + cv, predicate to handle spurious wakeups, shutdown signaling |
| `Worker.cpp` | exception safety inside the loop, why `wait_and_pop` returning `false` is the shutdown signal |
| `ThreadPool.cpp` | RAII (destructor calls shutdown), `std::unique_ptr<Worker>` for ownership, why pool > spawning per task |
| `main.cpp` | priority ordering — higher-priority tasks jump ahead even when submitted later |
