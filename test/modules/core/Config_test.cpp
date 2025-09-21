 #include <gtest/gtest.h>
 #include <filesystem>
#include "Config.hpp"
#include <fstream>
#include <filesystem>

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时JSON配置文件
        std::ofstream configFile("test_config.json");
        configFile << "{\n";
        configFile << "  \"port\": 8080,\n";
        configFile << "  \"threads\": 4,\n";
        configFile << "  \"timeout\": 30\n";
        configFile << "}\n";
        configFile.close();
    }

    void TearDown() override {
        // 删除临时配置文件
        std::filesystem::remove("test_config.json");
    }
};

TEST_F(ConfigTest, LoadConfig) {
    webserver::Config config;
    EXPECT_TRUE(config.loadFromFile("test_config.json"));
}

// 测试Config类的get方法，验证配置文件中各项参数的值是否正确
// 该测试用例测试了端口、线程数和超时时间三个参数
TEST_F(ConfigTest, GetValues) {
    webserver::Config config;
    // 从test_config.json文件加载配置
    config.loadFromFile("test_config.json");
    // 验证端口值是否为预期的8080
    EXPECT_EQ(config.get<int>("port"), 8080);
    // 验证线程数是否为预期的4
    EXPECT_EQ(config.get<int>("threads"), 4);
    // 验证超时时间是否为预期的30秒
    EXPECT_EQ(config.get<int>("timeout"), 30);
}

TEST_F(ConfigTest, DefaultValues) {
    webserver::Config config;
    config.loadFromFile("test_config.json");
    
    // 测试不存在的配置项返回默认值
    EXPECT_EQ(config.get<int>("nonexistent", 999), 999);
    EXPECT_EQ(config.get<std::string>("nonexistent", "default"), "default");
}

TEST_F(ConfigTest, InvalidConfig) {
    webserver::Config config;
    EXPECT_FALSE(config.loadFromFile("nonexistent.json"));
}