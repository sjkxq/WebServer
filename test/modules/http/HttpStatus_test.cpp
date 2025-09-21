#include "HttpStatus.hpp"
#include <gtest/gtest.h>

TEST(HttpStatusTest, GetStatusMessage) {
    auto& handler = webserver::HttpStatusHandler::getInstance();
    
    // 测试一些常见状态码
    EXPECT_EQ("OK", handler.getStatusMessage(webserver::HttpStatus::OK));
    EXPECT_EQ("Not Found", handler.getStatusMessage(webserver::HttpStatus::NOT_FOUND));
    EXPECT_EQ("Internal Server Error", handler.getStatusMessage(webserver::HttpStatus::INTERNAL_SERVER_ERROR));
    
    // 使用整数状态码
    EXPECT_EQ("OK", handler.getStatusMessage(200));
    EXPECT_EQ("Not Found", handler.getStatusMessage(404));
    EXPECT_EQ("Internal Server Error", handler.getStatusMessage(500));
    
    // 测试未知状态码
    EXPECT_EQ("Unknown Status", handler.getStatusMessage(999));
}

TEST(HttpStatusTest, StatusClassification) {
    // 测试信息性状态码
    EXPECT_TRUE(webserver::HttpStatusHandler::isInformational(webserver::HttpStatus::CONTINUE));
    EXPECT_TRUE(webserver::HttpStatusHandler::isInformational(webserver::HttpStatus::SWITCHING_PROTOCOLS));
    EXPECT_FALSE(webserver::HttpStatusHandler::isInformational(webserver::HttpStatus::OK));
    
    // 测试成功状态码
    EXPECT_TRUE(webserver::HttpStatusHandler::isSuccessful(webserver::HttpStatus::OK));
    EXPECT_TRUE(webserver::HttpStatusHandler::isSuccessful(webserver::HttpStatus::CREATED));
    EXPECT_FALSE(webserver::HttpStatusHandler::isSuccessful(webserver::HttpStatus::NOT_FOUND));
    
    // 测试重定向状态码
    EXPECT_TRUE(webserver::HttpStatusHandler::isRedirection(webserver::HttpStatus::MOVED_PERMANENTLY));
    EXPECT_TRUE(webserver::HttpStatusHandler::isRedirection(webserver::HttpStatus::FOUND));
    EXPECT_FALSE(webserver::HttpStatusHandler::isRedirection(webserver::HttpStatus::OK));
    
    // 测试客户端错误状态码
    EXPECT_TRUE(webserver::HttpStatusHandler::isClientError(webserver::HttpStatus::BAD_REQUEST));
    EXPECT_TRUE(webserver::HttpStatusHandler::isClientError(webserver::HttpStatus::NOT_FOUND));
    EXPECT_FALSE(webserver::HttpStatusHandler::isClientError(webserver::HttpStatus::OK));
    
    // 测试服务器错误状态码
    EXPECT_TRUE(webserver::HttpStatusHandler::isServerError(webserver::HttpStatus::INTERNAL_SERVER_ERROR));
    EXPECT_TRUE(webserver::HttpStatusHandler::isServerError(webserver::HttpStatus::BAD_GATEWAY));
    EXPECT_FALSE(webserver::HttpStatusHandler::isServerError(webserver::HttpStatus::NOT_FOUND));
}