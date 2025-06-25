#include <gtest/gtest.h>
#include "fixtures/base_fixtures.h"
#include "fixtures/thread_pool_fixtures.h"
#include "fixtures/memory_pool_fixtures.h"
#include "fixtures/logger_fixtures.h"
#include "fixtures/network_fixtures.h"
#include "fixtures/fixture_factory.h"

using namespace WebServer::test;

// 基础测试夹具示例
TEST_F(ServerTestFixture, BasicFixtureExample) {
    // 使用基础测试夹具中的辅助方法
    std::string random_str = GenerateRandomString(10);
    EXPECT_EQ(random_str.length(), 10);
    
    // 测试执行时间测量
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_GE(GetTestDuration(), 100);
}

// 线程池测试夹具示例
TEST_F(ThreadPoolFixture, ThreadPoolBasicOperations) {
    // 提交任务并验证结果
    auto future = thread_pool_->submit([]() {
        return 42;
    });
    
    EXPECT_EQ(future.get(), 42);
}

// 参数化线程池测试示例
TEST_P(ThreadPoolParameterizedFixture, ThreadCountParameterized) {
    // 获取参数化的线程数
    size_t thread_count = GetThreadCount();
    
    // 提交与线程数相同数量的任务
    std::atomic<int> counter{0};
    std::vector<std::future<void>> futures;
    
    for (size_t i = 0; i < thread_count; ++i) {
        futures.push_back(thread_pool_->submit([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            counter++;
        }));
    }
    
    // 等待所有任务完成
    for (auto& future : futures) {
        future.wait();
    }
    
    EXPECT_EQ(counter.load(), thread_count);
}

// 实例化参数化测试
INSTANTIATE_TEST_SUITE_P(
    ThreadCounts,
    ThreadPoolParameterizedFixture,
    ::testing::ValuesIn(THREAD_COUNTS)
);

// 内存池测试夹具示例
TEST_F(MemoryPoolFixture, MemoryPoolAllocateAndDeallocate) {
    // 分配内存并验证
    const size_t size = 128;
    void* ptr = AllocateAndVerify(size);
    ASSERT_NE(ptr, nullptr);
    
    // 释放内存
    memory_pool_->deallocate(ptr);
}

// 多级内存池测试夹具示例
TEST_F(MultiLevelMemoryPoolFixture, MultiLevelAllocation) {
    // 测试不同级别的内存分配
    TestAllocationAtLevel(32, 10);
    TestAllocationAtLevel(64, 10);
    TestAllocationAtLevel(128, 10);
    
    // 测试跨级别分配
    TestCrossLevelAllocation();
}

// 日志测试夹具示例
TEST_F(LoggerFixture, LoggerBasicOperations) {
    // 生成日志消息
    logger_->info("This is an info message");
    logger_->error("This is an error message");
    logger_->flush();
    
    // 验证日志内容
    EXPECT_TRUE(LogContains("This is an info message"));
    EXPECT_TRUE(LogContains("This is an error message"));
}

// 参数化日志测试示例
TEST_P(LoggerParameterizedFixture, LogLevelFiltering) {
    LogLevel current_level = GetLogLevel();
    
    // 生成所有级别的日志消息
    logger_->debug("Debug message");
    logger_->info("Info message");
    logger_->warn("Warning message");
    logger_->error("Error message");
    logger_->fatal("Fatal message");
    logger_->flush();
    
    // 验证日志级别过滤
    bool should_contain_debug = current_level <= LogLevel::DEBUG;
    bool should_contain_info = current_level <= LogLevel::INFO;
    bool should_contain_warn = current_level <= LogLevel::WARN;
    bool should_contain_error = current_level <= LogLevel::ERROR;
    bool should_contain_fatal = current_level <= LogLevel::FATAL;
    
    EXPECT_EQ(LogContains("Debug message"), should_contain_debug);
    EXPECT_EQ(LogContains("Info message"), should_contain_info);
    EXPECT_EQ(LogContains("Warning message"), should_contain_warn);
    EXPECT_EQ(LogContains("Error message"), should_contain_error);
    EXPECT_EQ(LogContains("Fatal message"), should_contain_fatal);
}

// 实例化参数化日志测试
INSTANTIATE_TEST_SUITE_P(
    LogLevels,
    LoggerParameterizedFixture,
    ::testing::ValuesIn(LOG_LEVELS)
);

// 网络测试夹具示例
TEST_F(NetworkFixture, BasicSocketOperations) {
    // 启动监听
    StartListening();
    
    // 创建客户端连接
    int client_sock = CreateClientConnection();
    ASSERT_NE(client_sock, -1);
    
    // 接受连接
    int server_client = AcceptConnection();
    ASSERT_NE(server_client, -1);
    
    // 发送和接收数据
    const std::string test_data = "Hello, Server!";
    ASSERT_TRUE(SendData(client_sock, test_data));
    
    std::string received = ReceiveData(server_client);
    EXPECT_EQ(received, test_data);
    
    // 清理
    close(client_sock);
    close(server_client);
}

// HTTP测试夹具示例 (注意：这个测试需要实际的HTTP服务器运行)
// 这里只是演示如何使用夹具，实际测试可能需要模拟HTTP服务器
TEST_F(HttpFixture, DISABLED_HttpRequestBuilding) {
    // 构建HTTP请求
    std::string request = BuildHttpRequest(
        "GET", "/api/test",
        "", {{"User-Agent", "Test Client"}}
    );
    
    EXPECT_TRUE(request.find("GET /api/test HTTP/1.1") != std::string::npos);
    EXPECT_TRUE(request.find("User-Agent: Test Client") != std::string::npos);
}

// 测试夹具工厂示例
TEST(FixtureFactoryTest, CreateFixtures) {
    // 创建基础测试夹具
    auto base_fixture = ServerTestFixtureFactory::CreateFixture();
    EXPECT_NE(base_fixture, nullptr);
    
    // 创建线程池测试夹具
    FixtureOptions thread_options;
    thread_options.thread_count = 8;
    auto thread_fixture = ServerTestFixtureFactory::CreateThreadPoolFixture(thread_options);
    EXPECT_NE(thread_fixture, nullptr);
    
    // 创建日志测试夹具
    FixtureOptions log_options;
    log_options.log_level = LogLevel::DEBUG;
    auto logger_fixture = ServerTestFixtureFactory::CreateLoggerFixture(log_options);
    EXPECT_NE(logger_fixture, nullptr);
    
    // 根据类型创建测试夹具
    auto memory_fixture = ServerTestFixtureFactory::CreateFixtureByType("memory_pool");
    EXPECT_NE(memory_fixture, nullptr);
    
    // 创建组合测试夹具
    auto composite_fixture = ServerTestFixtureFactory::CreateCompositeFixture(
        {"thread_pool", "logger", "network"}
    );
    EXPECT_NE(composite_fixture, nullptr);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}