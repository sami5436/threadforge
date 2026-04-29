#include "ThreadPool.h"

#include "Task.h"

#include <utility>

ThreadPool::ThreadPool(size_t numThreads) {
    workers_.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        auto w = std::make_unique<Worker>(static_cast<int>(i), queue_);
        w->start();
        workers_.push_back(std::move(w));
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}

void ThreadPool::submit(std::shared_ptr<Task> task) {
    if (stopped_) return;   // refuse work after shutdown
    queue_.push(std::move(task));
}

void ThreadPool::shutdown() {
    if (stopped_) return;
    stopped_ = true;

    // 1. Tell the queue no more work is coming. Any worker waiting on an
    //    empty queue wakes up and gets `false` from wait_and_pop, exiting
    //    its loop. Any worker mid-task finishes, then sees the queue empty
    //    + shutdown flag and exits.
    queue_.shutdown();

    // 2. Wait for every worker thread to exit.
    for (auto& w : workers_) {
        w->join();
    }
}
