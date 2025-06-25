#ifndef WEBSERVER_TEST_FIXTURES_BASE_FIXTURES_H_
#define WEBSERVER_TEST_FIXTURES_BASE_FIXTURES_H_

#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <chrono>
#include <functional>

namespace WebServer {
namespace test {

// 基础测试夹具类，提供所有测试通用的功能
class ServerTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // 记录测试开始时间
        test_start_time_ = std::chrono::steady_clock::now();
    }

    void TearDown() override {
        // 计算测试执行时间
        auto test_end_time = std::chrono::steady_clock::now();
        test_duration_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            test_end_time - test_start_time_).count();
    }

    // 通用辅助方法

    // 获取测试执行时间（毫秒）
    long long GetTestDuration() const {
        return test_duration_;
    }

    // 创建临时测试目录
    std::string CreateTempTestDir() const {
        char template_path[] = "/tmp/webserver_test_XXXXXX";
        char* temp_dir = mkdtemp(template_path);
        if (temp_dir == nullptr) {
            throw std::runtime_error("Failed to create temporary test directory");
        }
        return std::string(temp_dir);
    }

    // 删除目录及其内容
    void RemoveDirectory(const std::string& path) const {
        std::string cmd = "rm -rf " + path;
        system(cmd.c_str());
    }

    // 执行带超时的操作
    template<typename F>
    bool ExecuteWithTimeout(F func, std::chrono::milliseconds timeout) {
        std::atomic<bool> completed(false);
        auto future = std::async(std::launch::async, [&]() {
            func();
            completed = true;
        });

        if (future.wait_for(timeout) == std::future_status::timeout) {
            return false;  // 超时
        }
        return completed;
    }

    // 生成随机字符串
    std::string GenerateRandomString(size_t length) const {
        const char charset[] = "0123456789"
                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                             "abcdefghijklmnopqrstuvwxyz";
        const size_t charset_size = sizeof(charset) - 1;
        
        std::string result;
        result.reserve(length);
        
        for (size_t i = 0; i < length; ++i) {
            result += charset[rand() % charset_size];
        }
        
        return result;
    }

private:
    std::chrono::steady_clock::time_point test_start_time_;
    long long test_duration_{0};
};

} // namespace test
} // namespace WebServer

#endif // WEBSERVER_TEST_FIXTURES_BASE_FIXTURES_H_