#include <gtest/gtest.h>
#include "Logger.hpp"
#include <sstream>
#include <thread>

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger.setLogLevel(Logger::Level::DEBUG);
        logger.setStream(output);
    }

    void TearDown() override {
        logger.setStream(std::cout);
    }

    Logger logger;
    std::ostringstream output;
};

TEST_F(LoggerTest, LogLevelFiltering) {
    logger.log(Logger::Level::ERROR, "Error message");
    logger.log(Logger::Level::WARNING, "Warning message");
    logger.log(Logger::Level::INFO, "Info message");
    logger.log(Logger::Level::DEBUG, "Debug message");
    
    logger.setLogLevel(Logger::Level::WARNING);
    logger.log(Logger::Level::INFO, "This should not appear");
    
    auto logContent = output.str();
    EXPECT_TRUE(logContent.find("Error message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Warning message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Info message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Debug message") != std::string::npos);
    EXPECT_TRUE(logContent.find("This should not appear") == std::string::npos);
}

TEST_F(LoggerTest, LogFormat) {
    logger.log(Logger::Level::ERROR, "Test message");
    auto logContent = output.str();
    EXPECT_TRUE(logContent.find("[ERROR] Test message") != std::string::npos);
}

TEST_F(LoggerTest, ThreadSafety) {
    const int threadCount = 10;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back([this, i] {
            logger.log(Logger::Level::INFO, "Thread " + std::to_string(i));
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
    std::ostringstream testStream;
    logger.setStream(testStream);
    logger.log(Logger::Level::INFO, "Stream test");
    
    EXPECT_TRUE(testStream.str().find("Stream test") != std::string::npos);
    EXPECT_TRUE(output.str().empty());  // 原始流不应有输出
}