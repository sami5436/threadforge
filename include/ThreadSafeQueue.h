#pragma once

// A bounded-blocking-free thread-safe FIFO queue.
//
// Pattern: producer-consumer.
//   Producers call push(value); the queue notifies one waiter.
//   Consumers call wait_and_pop(out); they sleep on the condition variable
//   until either an item is available OR shutdown() has been called.
//
// Synchronization primitives:
//   - mutex   protects the underlying std::queue from concurrent access
//   - cv      lets consumers sleep instead of spin when the queue is empty
//   - flag    lets shutdown() wake up consumers waiting on an empty queue
//
// This is a generic FIFO. The thread pool uses a *priority* variant
// (ThreadSafeTaskQueue) instead — but the synchronization pattern is
// identical, and this template is a useful reference for interviews.

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

template <typename T>
class ThreadSafeQueue {
public:
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            queue_.push(std::move(value));
        }
        cv_.notify_one();   // wake exactly one waiter (we only added one item)
    }

    // Blocks until an item is available or shutdown() is called.
    // Returns true on item, false if the queue was shut down empty.
    bool wait_and_pop(T& out) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return !queue_.empty() || shutdown_; });
        if (queue_.empty()) return false;   // shutdown path
        out = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            shutdown_ = true;
        }
        cv_.notify_all();   // wake every waiter so they can exit
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }

private:
    mutable std::mutex      mtx_;
    std::condition_variable cv_;
    std::queue<T>           queue_;
    bool                    shutdown_ = false;
};
