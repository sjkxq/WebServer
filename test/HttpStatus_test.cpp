#include "HttpStatus.hpp"
#include <gtest/gtest.h>

TEST(HttpStatusTest, GetStatusMessage) {
    auto& handler = HttpStatusHandler::getInstance();
    
    // 测试一些常见状态码
    EXPECT_EQ("OK", handler.getStatusMessage(HttpStatus::OK));
    EXPECT_EQ("Not Found", handler.getStatusMessage(HttpStatus::NOT_FOUND));
    EXPECT_EQ("Internal Server Error", handler.getStatusMessage(HttpStatus::INTERNAL_SERVER_ERROR));
    
    // 使用整数状态码
    EXPECT_EQ("OK", handler.getStatusMessage(200));
    EXPECT_EQ("Not Found", handler.getStatusMessage(404));
    EXPECT_EQ("Internal Server Error", handler.getStatusMessage(500));
    
    // 测试未知状态码
    EXPECT_EQ("Unknown Status", handler.getStatusMessage(999));
}

TEST(HttpStatusTest, StatusClassification) {
    // 测试信息性状态码
    EXPECT_TRUE(HttpStatusHandler::isInformational(HttpStatus::CONTINUE));
    EXPECT_TRUE(HttpStatusHandler::isInformational(HttpStatus::SWITCHING_PROTOCOLS));
    EXPECT_FALSE(HttpStatusHandler::isInformational(HttpStatus::OK));
    
    // 测试成功状态码
    EXPECT_TRUE(HttpStatusHandler::isSuccessful(HttpStatus::OK));
    EXPECT_TRUE(HttpStatusHandler::isSuccessful(HttpStatus::CREATED));
    EXPECT_FALSE(HttpStatusHandler::isSuccessful(HttpStatus::NOT_FOUND));
    
    // 测试重定向状态码
    EXPECT_TRUE(HttpStatusHandler::isRedirection(HttpStatus::MOVED_PERMANENTLY));
    EXPECT_TRUE(HttpStatusHandler::isRedirection(HttpStatus::FOUND));
    EXPECT_FALSE(HttpStatusHandler::isRedirection(HttpStatus::OK));
    
    // 测试客户端错误状态码
    EXPECT_TRUE(HttpStatusHandler::isClientError(HttpStatus::BAD_REQUEST));
    EXPECT_TRUE(HttpStatusHandler::isClientError(HttpStatus::NOT_FOUND));
    EXPECT_FALSE(HttpStatusHandler::isClientError(HttpStatus::OK));
    
    // 测试服务器错误状态码
    EXPECT_TRUE(HttpStatusHandler::isServerError(HttpStatus::INTERNAL_SERVER_ERROR));
    EXPECT_TRUE(HttpStatusHandler::isServerError(HttpStatus::BAD_GATEWAY));
    EXPECT_FALSE(HttpStatusHandler::isServerError(HttpStatus::NOT_FOUND));
}