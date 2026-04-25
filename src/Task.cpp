#include "Task.h"
#include <sstream>

Task::Task(std::string name, int priority)
    : name_(std::move(name))
    , priority_(priority)
    , status_(TaskStatus::Pending)
    , createdAt_(std::chrono::steady_clock::now())
{}

std::string Task::describe() const {
    std::ostringstream oss;
    oss << "[Task: " << name_ << " | priority=" << priority_ << "]";
    return oss.str();
}
