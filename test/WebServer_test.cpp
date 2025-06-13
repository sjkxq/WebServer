#include <gtest/gtest.h>
#include <fstream>
#include "WebServer.hpp"
#include "Config.hpp"

class WebServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时配置文件
        std::ofstream tmpFile("test_config.json");
        tmpFile << "{\"server\":{\"port\":8080}}" << std::endl;
        tmpFile.close();
        
        // 加载配置文件
        testConfig.loadFromFile("test_config.json");
    }

    void TearDown() override {
        // 清理测试
    }
    
    Config testConfig;
};

TEST_F(WebServerTest, Initialization) {
    WebServer server(testConfig);
    EXPECT_EQ(testConfig.get<int>("server.port", 8080), 8080);
    // 可以添加更多初始化测试
}

TEST_F(WebServerTest, RequestHandling) {
    // 测试请求处理逻辑
    // 需要mock网络请求
}