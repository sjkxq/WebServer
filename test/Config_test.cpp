 #include <gtest/gtest.h>
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
        std::filesystem::remove("test_config.ini");
    }
};

TEST_F(ConfigTest, LoadConfig) {
    Config config;
    EXPECT_TRUE(config.loadFromFile("test_config.json"));
}

TEST_F(ConfigTest, GetValues) {
    Config config;
    config.loadFromFile("test_config.json");
    EXPECT_EQ(config.get<int>("port"), 8080);
    EXPECT_EQ(config.get<int>("threads"), 4);
    EXPECT_EQ(config.get<int>("timeout"), 30);
}

TEST_F(ConfigTest, DefaultValues) {
    Config config;
    config.loadFromFile("test_config.json");
    
    // 测试不存在的配置项返回默认值
    EXPECT_EQ(config.get<int>("nonexistent", 999), 999);
    EXPECT_EQ(config.get<std::string>("nonexistent", "default"), "default");
}

TEST_F(ConfigTest, InvalidConfig) {
    Config config;
    EXPECT_FALSE(config.loadFromFile("nonexistent.json"));
}