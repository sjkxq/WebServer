#ifndef WEBSERVER_TEST_FIXTURES_THREAD_POOL_FIXTURES_H_
#define WEBSERVER_TEST_FIXTURES_THREAD_POOL_FIXTURES_H_

#include "base_fixtures.h"
#include "../../include/ThreadPool.h"
#include <vector>
#include <future>
#include <atomic>

namespace WebServer {
namespace test {

// 线程池基础测试夹具
class ThreadPoolFixture : public ServerTestFixture {
protected:
    void SetUp() override {
        ServerTestFixture::SetUp();
        // 创建默认线程池
        thread_pool_ = std::make_unique<ThreadPool>(default_thread_count_);
    }

    void TearDown() override {
        // 确保线程池被正确关闭
        if (thread_pool_) {
            thread_pool_->shutdown();
            thread_pool_.reset();
        }
        ServerTestFixture::TearDown();
    }

    // 提交多个任务并等待它们完成
    template<typename F>
    std::vector<std::future<typename std::result_of<F()>::type>>
    SubmitTasks(const F& task, size_t task_count) {
        using ReturnType = typename std::result_of<F()>::type;
        std::vector<std::future<ReturnType>> futures;
        futures.reserve(task_count);

        for (size_t i = 0; i < task_count; ++i) {
            futures.push_back(thread_pool_->submit(task));
        }

        return futures;
    }

    // 测试线程池的负载能力
    void StressTest(size_t task_count, std::chrono::milliseconds task_duration) {
        std::atomic<size_t> completed_tasks{0};
        auto task = [&completed_tasks, task_duration]() {
            std::this_thread::sleep_for(task_duration);
            ++completed_tasks;
            return true;
        };

        auto futures = SubmitTasks(task, task_count);
        
        // 等待所有任务完成
        for (auto& future : futures) {
            future.wait();
        }

        EXPECT_EQ(completed_tasks.load(), task_count);
    }

    // 获取线程池的当前活跃线程数
    size_t GetActiveThreadCount() const {
        return thread_pool_->getActiveThreadCount();
    }

    // 获取线程池的任务队列大小
    size_t GetTaskQueueSize() const {
        return thread_pool_->getTaskQueueSize();
    }

protected:
    std::unique_ptr<ThreadPool> thread_pool_;
    const size_t default_thread_count_ = 4;
};

// 参数化线程池测试夹具
class ThreadPoolParameterizedFixture : 
    public ThreadPoolFixture,
    public ::testing::WithParamInterface<size_t> {
protected:
    void SetUp() override {
        ServerTestFixture::SetUp();  // 注意：不调用ThreadPoolFixture::SetUp()
        // 使用参数化的线程数创建线程池
        thread_pool_ = std::make_unique<ThreadPool>(GetParam());
    }

    // 获取当前测试的线程数参数
    size_t GetThreadCount() const {
        return GetParam();
    }
};

// 定义常用的线程数参数
const std::vector<size_t> THREAD_COUNTS = {1, 2, 4, 8, 16};

} // namespace test
} // namespace WebServer

#endif // WEBSERVER_TEST_FIXTURES_THREAD_POOL_FIXTURES_H_