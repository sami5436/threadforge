#pragma once

// Concrete Task subclasses — examples of polymorphism in action.
// Each one inherits from Task and provides its own execute() implementation.
// The ThreadPool calls task->execute() through a Task* — runtime dispatch via
// the vtable picks the right one.

#include "Task.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

// ---------------------------------------------------------------------------
// PrintTask — simplest possible task: prints a message, sleeps a beat to
// simulate I/O, then completes.
// ---------------------------------------------------------------------------
class PrintTask : public Task {
public:
    explicit PrintTask(std::string msg, int priority = 0)
        : Task("PrintTask", priority)
        , msg_(std::move(msg))
    {}

    void execute() override {
        markRunning();
        std::cout << "[PrintTask] " << msg_ << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        markCompleted();
    }

    std::string describe() const override {
        return "[PrintTask: \"" + msg_ + "\"]";
    }

private:
    std::string msg_;
};

// ---------------------------------------------------------------------------
// ComputeTask — CPU-bound work: sum 1..n. Demonstrates a task that produces
// a result.
// ---------------------------------------------------------------------------
class ComputeTask : public Task {
public:
    explicit ComputeTask(int n, int priority = 0)
        : Task("ComputeTask", priority)
        , n_(n)
    {}

    void execute() override {
        markRunning();
        long long sum = 0;
        for (int i = 1; i <= n_; ++i) sum += i;
        std::cout << "[ComputeTask] sum(1.." << n_ << ") = " << sum << "\n";
        markCompleted();
    }

private:
    int n_;
};

// ---------------------------------------------------------------------------
// FailingTask — deliberately throws to demonstrate the failure path.
// The worker also has a top-level catch, but a well-behaved task catches its
// own exceptions and reports them via markFailed() so callers can inspect
// status afterwards.
// ---------------------------------------------------------------------------
class FailingTask : public Task {
public:
    explicit FailingTask(int priority = 0)
        : Task("FailingTask", priority)
    {}

    void execute() override {
        markRunning();
        try {
            throw std::runtime_error("simulated failure");
        } catch (const std::exception& e) {
            std::cerr << "[FailingTask] caught: " << e.what() << "\n";
            markFailed();
        }
    }
};
