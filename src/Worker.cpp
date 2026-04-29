#include "Worker.h"

#include <exception>
#include <iostream>
#include <memory>

Worker::Worker(int id, ThreadSafeTaskQueue& queue)
    : id_(id)
    , queue_(queue)
{}

Worker::~Worker() {
    // Defensive cleanup: if the owner forgot to join() us, do it now.
    // A std::thread that's still joinable when destroyed calls std::terminate,
    // so this avoids a crash on accidental misuse.
    if (thread_.joinable()) {
        thread_.join();
    }
}

void Worker::start() {
    thread_ = std::thread([this] { loop(); });
}

void Worker::join() {
    if (thread_.joinable()) thread_.join();
}

void Worker::loop() {
    std::shared_ptr<Task> task;
    while (queue_.wait_and_pop(task)) {
        busy_.store(true);
        try {
            task->execute();
        } catch (const std::exception& e) {
            // Last-resort catch: a well-behaved task handles its own errors,
            // but a misbehaving one shouldn't kill the worker thread.
            std::cerr << "[Worker " << id_ << "] task threw: "
                      << e.what() << "\n";
        } catch (...) {
            std::cerr << "[Worker " << id_ << "] task threw unknown exception\n";
        }
        tasksRun_.fetch_add(1);
        busy_.store(false);
        task.reset();   // drop our refcount on the task
    }
    // wait_and_pop returned false → queue shut down with no work left. Exit.
}
