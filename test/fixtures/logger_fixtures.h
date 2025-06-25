#ifndef WEBSERVER_TEST_FIXTURES_LOGGER_FIXTURES_H_
#define WEBSERVER_TEST_FIXTURES_LOGGER_FIXTURES_H_

#include "base_fixtures.h"
#include "../../include/Logger.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>

namespace WebServer {
namespace test {

// 日志测试夹具
class LoggerFixture : public ServerTestFixture {
protected:
    void SetUp() override {
        ServerTestFixture::SetUp();
        // 创建临时日志目录
        log_dir_ = CreateTempTestDir();
        log_file_ = log_dir_ + "/test.log";
        
        // 初始化日志系统
        logger_ = std::make_unique<Logger>();
        logger_->init(log_file_);
    }

    void TearDown() override {
        // 确保日志系统正确关闭
        if (logger_) {
            logger_->flush();
            logger_.reset();
        }
        
        // 清理临时文件
        RemoveDirectory(log_dir_);
        
        ServerTestFixture::TearDown();
    }

    // 读取日志文件内容
    std::string ReadLogFile() const {
        std::ifstream file(log_file_);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open log file: " + log_file_);
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // 检查日志文件中是否包含特定内容
    bool LogContains(const std::string& content) const {
        std::string log_content = ReadLogFile();
        return log_content.find(content) != std::string::npos;
    }

    // 使用正则表达式搜索日志内容
    bool LogMatchesPattern(const std::string& pattern) const {
        std::string log_content = ReadLogFile();
        std::regex regex_pattern(pattern);
        return std::regex_search(log_content, regex_pattern);
    }

    // 获取日志文件中的行数
    size_t GetLogLineCount() const {
        std::ifstream file(log_file_);
        return std::count(std::istreambuf_iterator<char>(file),
                         std::istreambuf_iterator<char>(), '\n');
    }

    // 清空日志文件
    void ClearLogFile() {
        std::ofstream file(log_file_, std::ios::trunc);
        file.close();
    }

    // 生成指定数量的日志消息
    void GenerateLogMessages(size_t count, LogLevel level = LogLevel::INFO) {
        for (size_t i = 0; i < count; ++i) {
            std::string message = "Test log message " + std::to_string(i);
            switch (level) {
                case LogLevel::DEBUG:
                    logger_->debug(message);
                    break;
                case LogLevel::INFO:
                    logger_->info(message);
                    break;
                case LogLevel::WARN:
                    logger_->warn(message);
                    break;
                case LogLevel::ERROR:
                    logger_->error(message);
                    break;
                case LogLevel::FATAL:
                    logger_->fatal(message);
                    break;
            }
        }
        logger_->flush();
    }

    // 测试不同日志级别
    void TestAllLogLevels(const std::string& message) {
        logger_->debug(message + " - DEBUG");
        logger_->info(message + " - INFO");
        logger_->warn(message + " - WARN");
        logger_->error(message + " - ERROR");
        logger_->fatal(message + " - FATAL");
        logger_->flush();
    }

    // 性能测试
    void PerformanceTest(size_t message_count) {
        auto start_time = std::chrono::steady_clock::now();
        
        GenerateLogMessages(message_count);
        
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time).count();
        
        std::cout << "Logged " << message_count << " messages in "
                  << duration << " milliseconds" << std::endl;
    }

    // 测试日志文件轮转
    void TestLogRotation(size_t message_count, size_t messages_per_rotation) {
        for (size_t i = 0; i < message_count; ++i) {
            logger_->info("Rotation test message " + std::to_string(i));
            if ((i + 1) % messages_per_rotation == 0) {
                logger_->rotate();
            }
        }
        logger_->flush();
    }

protected:
    std::unique_ptr<Logger> logger_;
    std::string log_dir_;
    std::string log_file_;
};

// 参数化日志测试夹具
class LoggerParameterizedFixture :
    public LoggerFixture,
    public ::testing::WithParamInterface<LogLevel> {
protected:
    void SetUp() override {
        LoggerFixture::SetUp();
        // 设置日志级别
        logger_->setLevel(GetParam());
    }

    // 获取当前测试的日志级别
    LogLevel GetLogLevel() const {
        return GetParam();
    }
};

// 定义常用的日志级别参数
const std::vector<LogLevel> LOG_LEVELS = {
    LogLevel::DEBUG,
    LogLevel::INFO,
    LogLevel::WARN,
    LogLevel::ERROR,
    LogLevel::FATAL
};

} // namespace test
} // namespace WebServer

#endif // WEBSERVER_TEST_FIXTURES_LOGGER_FIXTURES_H_