#pragma once

#include <string>
#include <chrono>
#include <atomic>

// Lifecycle of a task through the scheduler
enum class TaskStatus {
    Pending,
    Running,
    Completed,
    Failed
};

// Abstract base class for all units of work the scheduler can run.
// Derive from this and implement execute() to create a concrete task.
class Task {
public:
    explicit Task(std::string name, int priority = 0);
    virtual ~Task() = default;

    // The work this task performs. Must be implemented by derived classes.
    // Should be safe to call from any thread.
    virtual void execute() = 0;

    // Human-readable description for logging — optional override
    virtual std::string describe() const;

    std::string     getName()     const { return name_; }
    int             getPriority() const { return priority_; }
    TaskStatus      getStatus()   const { return status_.load(); }
    std::chrono::steady_clock::time_point getCreatedAt() const { return createdAt_; }

protected:
    // Derived classes call these to update their own status
    void markRunning()   { status_.store(TaskStatus::Running); }
    void markCompleted() { status_.store(TaskStatus::Completed); }
    void markFailed()    { status_.store(TaskStatus::Failed); }

private:
    std::string   name_;
    int           priority_;  // higher = more urgent (used by Scheduler later)
    std::atomic<TaskStatus> status_;
    std::chrono::steady_clock::time_point createdAt_;

    // Non-copyable — tasks have identity, they shouldn't be sliced or duplicated
    Task(const Task&)            = delete;
    Task& operator=(const Task&) = delete;
};
