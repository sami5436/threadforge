// ThreadForge demo
// -----------------
// Stand up a ThreadPool, submit a mix of tasks at different priorities,
// then let them drain. Watch the order: higher-priority tasks execute first
// even if they were submitted later.

#include "Tasks.h"
#include "ThreadPool.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

int main() {
    constexpr size_t kNumWorkers = 4;
    std::cout << "ThreadForge starting with " << kNumWorkers << " workers\n";

    ThreadPool pool(kNumWorkers);

    // Submit a mix of tasks. Note priorities: 0 = default, higher = sooner.
    pool.submit(std::make_shared<PrintTask>("low priority hello", /*priority=*/0));
    pool.submit(std::make_shared<ComputeTask>(1'000'000, /*priority=*/0));
    pool.submit(std::make_shared<PrintTask>("URGENT log line", /*priority=*/10));
    pool.submit(std::make_shared<FailingTask>(/*priority=*/5));
    pool.submit(std::make_shared<ComputeTask>(500'000, /*priority=*/3));
    pool.submit(std::make_shared<PrintTask>("normal traffic", /*priority=*/1));
    pool.submit(std::make_shared<PrintTask>("ALSO URGENT", /*priority=*/10));

    // Give the pool a moment to drain (in real code you'd use futures or a
    // wait_for_idle() helper — kept simple for the demo).
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    pool.shutdown();
    std::cout << "ThreadForge done\n";
    return 0;
}
