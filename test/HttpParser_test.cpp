#include "HttpParser.hpp"
#include "gtest/gtest.h"
#include <map>

namespace webserver {

class HttpParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化代码
    }

    void TearDown() override {
        // 清理代码
    }
};

// 测试基本GET请求解析
TEST_F(HttpParserTest, ParseSimpleGetRequest) {
    std::string request = 
        "GET /index.html HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: test-agent\r\n"
        "\r\n";
    
    auto [method, path, headers, body] = HttpParser::parseRequest(request);
    
    EXPECT_EQ(method, "GET");
    EXPECT_EQ(path, "/index.html");
    EXPECT_EQ(headers["Host"], "example.com");
    EXPECT_EQ(headers["User-Agent"], "test-agent");
    EXPECT_TRUE(body.empty());
}

// 测试POST请求与内容体解析
TEST_F(HttpParserTest, ParsePostRequestWithBody) {
    std::string request = 
        "POST /submit HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 16\r\n"  // 更新为实际长度
        "\r\n"
        "{\"key\":\"value\"}\r\n";
    
    auto [method, path, headers, body] = HttpParser::parseRequest(request);
    
    EXPECT_EQ(method, "POST");
    EXPECT_EQ(path, "/submit");
    EXPECT_EQ(headers["Host"], "example.com");
    EXPECT_EQ(headers["Content-Type"], "application/json");
    EXPECT_EQ(headers["Content-Length"], "16");
    EXPECT_EQ(body, "{\"key\":\"value\"}\r\n");
}

// 测试响应构建
TEST_F(HttpParserTest, BuildSimpleResponse) {
    std::string expected = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 12\r\n"
        "Connection: close\r\n"
        "\r\n"
        "Hello World!";
    
    std::string actual = HttpParser::buildResponse(
        HttpStatus::OK, 
        "Hello World!", 
        "text/plain"
    );
    
    EXPECT_EQ(actual, expected);
}

// 测试带自定义头部的响应构建
TEST_F(HttpParserTest, BuildResponseWithCustomHeaders) {
    std::string expected = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 16\r\n"  // 更新为实际长度
        "X-Custom-Header: value\r\n"
        "Connection: close\r\n"
        "\r\n"
        "{\"status\":\"ok\"}";
    
    std::map<std::string, std::string> headers;
    headers["X-Custom-Header"] = "value";
    
    std::string actual = HttpParser::buildResponse(
        HttpStatus::OK, 
        "{\"status\":\"ok\"}", 
        headers,
        "application/json"
    );
    
    EXPECT_EQ(actual, expected);
}

// 测试超大头部处理
TEST_F(HttpParserTest, ParseRequestWithLargeHeaders) {
    std::string request = 
        "GET /large-headers HTTP/1.1\r\n"
        "Host: example.com\r\n";
    
    // 添加100个自定义头部
    for (int i = 0; i < 100; ++i) {
        request += "X-Custom-Header-" + std::to_string(i) + ": value-" + std::to_string(i) + "\r\n";
    }
    request += "\r\n";
    
    auto [method, path, headers, body] = HttpParser::parseRequest(request);
    
    EXPECT_EQ(method, "GET");
    EXPECT_EQ(path, "/large-headers");
    EXPECT_EQ(headers["Host"], "example.com");
    for (int i = 0; i < 100; ++i) {
        std::string header = "X-Custom-Header-" + std::to_string(i);
        EXPECT_EQ(headers[header], "value-" + std::to_string(i));
    }
    EXPECT_TRUE(body.empty());
}

// 测试无效HTTP方法
TEST_F(HttpParserTest, ParseInvalidHttpMethod) {
    std::string request = 
        "INVALID /path HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";
    
    EXPECT_THROW(
        HttpParser::parseRequest(request),
        std::invalid_argument
    );
}

// 测试缺少Host头部
TEST_F(HttpParserTest, ParseRequestWithoutHostHeader) {
    std::string request = 
        "GET /no-host HTTP/1.1\r\n"
        "User-Agent: test-agent\r\n"
        "\r\n";
    
    EXPECT_THROW(
        HttpParser::parseRequest(request),
        std::invalid_argument
    );
}

// 测试内容长度与实际体不匹配
TEST_F(HttpParserTest, ParseRequestWithMismatchedContentLength) {
    std::string request = 
        "POST /mismatch HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 10\r\n"
        "\r\n"
        "short";
    
    EXPECT_THROW(
        HttpParser::parseRequest(request),
        std::invalid_argument
    );
}

} // namespace webserver