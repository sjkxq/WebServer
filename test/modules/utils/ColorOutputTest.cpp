#include <gtest/gtest.h>
#include <sstream>
#include "webserver/color/output.hpp"

using namespace webserver::color;

// 测试自定义颜色处理器
class TestColorHandler : public ColorHandler {
public:
    std::string applyColor(Color color) const override {
        return "[COLOR:" + std::to_string(static_cast<int>(color)) + "]";
    }
    
    std::string applyBackground(Background bg) const override {
        return "[BG:" + std::to_string(static_cast<int>(bg)) + "]";
    }
    
    std::string applyStyle(Style style) const override {
        return "[STYLE:" + std::to_string(static_cast<int>(style)) + "]";
    }
    
    bool supportsColor() const override {
        return true;
    }
};

class ColorOutputTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 保存原始启用状态
        originalEnabled = ColorOutput::getInstance().isEnabled();
        
        // 设置测试处理器
        ColorOutput::getInstance().setHandler(std::make_unique<TestColorHandler>());
        ColorOutput::getInstance().setEnabled(true);
    }
    
    void TearDown() override {
        // 恢复原始状态
        ColorOutput::getInstance().autoDetectHandler();
        ColorOutput::getInstance().setEnabled(originalEnabled);
    }
    
    bool originalEnabled = true;
};

// 测试颜色输出流操作符
TEST_F(ColorOutputTest, StreamOperators) {
    std::stringstream ss;
    
    ss << Color::RED << "Red Text" << Style::RESET;
    EXPECT_EQ(ss.str(), "[COLOR:1]Red Text[STYLE:0]");
    
    ss.str("");
    ss << Background::BLUE << "Blue Background" << Style::RESET;
    EXPECT_EQ(ss.str(), "[BG:4]Blue Background[STYLE:0]");
    
    ss.str("");
    ss << Style::BOLD << "Bold Text" << Style::RESET;
    EXPECT_EQ(ss.str(), "[STYLE:1]Bold Text[STYLE:0]");
}

// 测试颜色格式化器
TEST_F(ColorOutputTest, ColorFormatter) {
    std::string result = ColorFormatter::colorize("Test", Color::GREEN);
    EXPECT_EQ(result, "[COLOR:2]Test[STYLE:0]");
    
    result = ColorFormatter::colorize("Test", Color::RED, Background::YELLOW);
    EXPECT_EQ(result, "[COLOR:1][BG:3]Test[STYLE:0]");
    
    result = ColorFormatter::colorize("Test", Color::BLUE, Background::WHITE, Style::UNDERLINE);
    EXPECT_EQ(result, "[COLOR:4][BG:7][STYLE:4]Test[STYLE:0]");
}

// 测试禁用颜色输出
TEST_F(ColorOutputTest, DisabledOutput) {
    ColorOutput::getInstance().setEnabled(false);
    
    std::stringstream ss;
    ss << Color::RED << "Red Text" << Style::RESET;
    EXPECT_EQ(ss.str(), "Red Text");
    
    std::string result = ColorFormatter::colorize("Test", Color::GREEN);
    EXPECT_EQ(result, "Test");
}

// 测试空颜色处理器
TEST_F(ColorOutputTest, NullColorHandler) {
    ColorOutput::getInstance().setHandler(std::make_unique<NullColorHandler>());
    
    std::stringstream ss;
    ss << Color::RED << "Red Text" << Style::RESET;
    EXPECT_EQ(ss.str(), "Red Text");
    
    std::string result = ColorFormatter::colorize("Test", Color::GREEN);
    EXPECT_EQ(result, "Test");
}

// 测试颜色处理器自动检测
TEST_F(ColorOutputTest, AutoDetectHandler) {
    ColorOutput::getInstance().autoDetectHandler();
    EXPECT_NE(ColorOutput::getInstance().getHandler(), nullptr);
}

// 测试ANSI颜色处理器
TEST(AnsiColorHandlerTest, ApplyColor) {
    AnsiColorHandler handler;
    EXPECT_EQ(handler.applyColor(Color::RED), "\033[31m");
    EXPECT_EQ(handler.applyBackground(Background::BLUE), "\033[44m");
    EXPECT_EQ(handler.applyStyle(Style::BOLD), "\033[1m");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}