#include <gtest/gtest.h>
#include "ThreadPool.hpp"
#include <atomic>
#include <chrono>

class ThreadPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        pool = std::make_unique<ThreadPool>(4);
    }

    void TearDown() override {
        pool.reset(); // Replace shutdown() with pool destruction
    }

    std::unique_ptr<ThreadPool> pool;
    std::atomic<int> counter{0};
};

TEST_F(ThreadPoolTest, BasicTaskExecution) {
    auto future = pool->enqueue([] { return 42; });
    EXPECT_EQ(future.get(), 42);
}

TEST_F(ThreadPoolTest, ConcurrentTaskExecution) {
    const int taskCount = 100;
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < taskCount; ++i) {
        futures.push_back(pool->enqueue([this] { counter++; }));
    }
    
    for (auto& f : futures) {
        f.get();
    }
    
    EXPECT_EQ(counter, taskCount);
}

TEST_F(ThreadPoolTest, ShutdownBehavior) {
    // Test that pool can be safely destroyed
    EXPECT_NO_THROW({
        pool.reset();
    });
    
    // Create a new pool for the next test
    pool = std::make_unique<ThreadPool>(4);
}