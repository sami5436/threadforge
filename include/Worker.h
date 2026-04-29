#pragma once

// Worker — owns one std::thread that loops:
//   1. block on the queue waiting for a task
//   2. execute it (catching anything it throws)
//   3. repeat
//
// The worker is loosely coupled to the pool: it holds a reference to the
// shared task queue and nothing else. When the queue is shut down, the
// blocking wait returns false and the worker's loop exits cleanly.
//
// Single Responsibility: Worker doesn't know how tasks were produced, what
// the priority comparator is, or how many other workers exist. It just runs
// the consume loop.

#include "ThreadSafeTaskQueue.h"

#include <atomic>
#include <thread>

class Worker {
public:
    Worker(int id, ThreadSafeTaskQueue& queue);
    ~Worker();

    // Non-copyable: a worker owns a thread; copying that doesn't make sense.
    Worker(const Worker&)            = delete;
    Worker& operator=(const Worker&) = delete;

    void start();   // spawns the underlying std::thread
    void join();    // waits for the thread to exit (after queue shutdown)

    int  getId()        const { return id_; }
    bool isBusy()       const { return busy_.load(); }
    long tasksRun()     const { return tasksRun_.load(); }

private:
    void loop();

    int                   id_;
    ThreadSafeTaskQueue&  queue_;
    std::thread           thread_;
    std::atomic<bool>     busy_{false};
    std::atomic<long>     tasksRun_{0};
};
