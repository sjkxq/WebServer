#include <gtest/gtest.h>
#include "Logger.hpp"
#include <sstream>
#include <thread>

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto& logger = webserver::Logger::getInstance();
        logger.setLogLevel(webserver::Logger::Level::DEBUG_LEVEL);
        logger.setStream(output);
    }

    void TearDown() override {
        auto& logger = webserver::Logger::getInstance();
        logger.setStream(std::cout);
    }

    std::ostringstream output;
};

TEST_F(LoggerTest, LogLevelFiltering) {
    auto& logger = webserver::Logger::getInstance();
    logger.log(webserver::Logger::Level::ERROR, "Error message");
    logger.log(webserver::Logger::Level::WARNING, "Warning message");
    logger.log(webserver::Logger::Level::INFO, "Info message");
    logger.log(webserver::Logger::Level::DEBUG_LEVEL, "Debug message");
    
    logger.setLogLevel(webserver::Logger::Level::WARNING);
    logger.log(webserver::Logger::Level::INFO, "This should not appear");
    
    auto logContent = output.str();
    EXPECT_TRUE(logContent.find("Error message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Warning message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Info message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Debug message") != std::string::npos);
    EXPECT_TRUE(logContent.find("This should not appear") == std::string::npos);
}

TEST_F(LoggerTest, LogFormat) {
    auto& logger = webserver::Logger::getInstance();
    logger.log(webserver::Logger::Level::ERROR, "Test message");
    auto logContent = output.str();
    EXPECT_TRUE(logContent.find("[ERROR] Test message") != std::string::npos);
}

TEST_F(LoggerTest, ThreadSafety) {
    auto& logger = webserver::Logger::getInstance();
    const int threadCount = 10;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back([&logger, i] {
            logger.log(webserver::Logger::Level::INFO, "Thread " + std::to_string(i));
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto logContent = output.str();
    for (int i = 0; i < threadCount; ++i) {
        EXPECT_TRUE(logContent.find("Thread " + std::to_string(i)) != std::string::npos);
    }
}

TEST_F(LoggerTest, StreamRedirection) {
    auto& logger = webserver::Logger::getInstance();
    std::ostringstream testStream;
    logger.setStream(testStream);
    logger.log(webserver::Logger::Level::INFO, "Stream test");
    
    EXPECT_TRUE(testStream.str().find("Stream test") != std::string::npos);
    EXPECT_TRUE(output.str().empty());  // 原始流不应有输出
}