#pragma once

// ThreadPool — owns a fixed set of Worker threads and a shared task queue.
//
// Why a pool? Spawning a thread for every task is expensive on Linux
// (clone(2), kernel bookkeeping, stack allocation). A pool of long-lived
// workers amortizes that cost: the threads exist once, and tasks just
// flow through the queue.
//
// API:
//   submit(task)  — enqueue work; returns immediately
//   shutdown()    — signal workers to drain the queue and exit; joins them
//
// Tasks are dispatched in priority order (see ThreadSafeTaskQueue).

#include "ThreadSafeTaskQueue.h"
#include "Worker.h"

#include <cstddef>
#include <memory>
#include <vector>

class Task;   // forward declared above via ThreadSafeTaskQueue.h

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    ThreadPool(const ThreadPool&)            = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // Add a task to be executed by some worker. Higher priority runs first.
    void submit(std::shared_ptr<Task> task);

    // Stop accepting new work, drain remaining queue, join all workers.
    // Idempotent — safe to call twice (the destructor calls it).
    void shutdown();

    size_t pendingCount() const { return queue_.size(); }
    size_t workerCount()  const { return workers_.size(); }

private:
    ThreadSafeTaskQueue                  queue_;
    std::vector<std::unique_ptr<Worker>> workers_;
    bool                                 stopped_ = false;
};
