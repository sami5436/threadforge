#pragma once

// Priority queue of tasks, thread-safe. This is what the ThreadPool actually
// uses: workers always pull the highest-priority pending task.
//
// Why a separate class instead of a templated priority queue? Two reasons:
//   1. The priority comparator is task-specific (reads task->getPriority()).
//   2. Keeping it concrete makes it easy to read and to talk through in
//      an interview ("here's the synchronization, here's the priority order").
//
// Synchronization: same pattern as ThreadSafeQueue — mutex + cv + shutdown
// flag. The only difference is std::priority_queue underneath instead of
// std::queue, with a Compare that ranks higher priority first.

#include "Task.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

class ThreadSafeTaskQueue {
public:
    void push(std::shared_ptr<Task> task) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            queue_.push(std::move(task));
        }
        cv_.notify_one();
    }

    // Blocks until a task is available or the queue is shut down.
    // Returns true with the highest-priority task in `out`, or false on
    // shutdown-with-empty-queue.
    bool wait_and_pop(std::shared_ptr<Task>& out) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return !queue_.empty() || shutdown_; });
        if (queue_.empty()) return false;
        out = queue_.top();
        queue_.pop();
        return true;
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            shutdown_ = true;
        }
        cv_.notify_all();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }

private:
    // priority_queue is a max-heap by default; "less" returns true when a is
    // *lower* priority than b — so b sits above a, meaning higher priority
    // pops first. That's what we want.
    struct Compare {
        bool operator()(const std::shared_ptr<Task>& a,
                        const std::shared_ptr<Task>& b) const {
            return a->getPriority() < b->getPriority();
        }
    };

    mutable std::mutex      mtx_;
    std::condition_variable cv_;
    std::priority_queue<
        std::shared_ptr<Task>,
        std::vector<std::shared_ptr<Task>>,
        Compare
    > queue_;
    bool shutdown_ = false;
};
