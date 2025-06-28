#pragma once

#include <string>
#include <ostream>
#include <memory>

namespace webserver::color {

/**
 * @brief 文本颜色枚举
 */
enum class Color {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    DEFAULT
};

/**
 * @brief 背景颜色枚举
 */
enum class Background {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    DEFAULT
};

/**
 * @brief 文本样式枚举
 */
enum class Style {
    RESET,
    BOLD,
    DIM,
    ITALIC,
    UNDERLINE,
    BLINK,
    REVERSE,
    HIDDEN
};

/**
 * @brief 颜色处理器接口
 * 
 * 定义了处理颜色输出的抽象接口，不同平台可以提供不同的实现
 */
class ColorHandler {
public:
    virtual ~ColorHandler() = default;
    
    /**
     * @brief 应用文本颜色
     * @param color 颜色值
     * @return 格式化后的字符串
     */
    virtual std::string applyColor(Color color) const = 0;
    
    /**
     * @brief 应用背景颜色
     * @param bg 背景色值
     * @return 格式化后的字符串
     */
    virtual std::string applyBackground(Background bg) const = 0;
    
    /**
     * @brief 应用文本样式
     * @param style 样式值
     * @return 格式化后的字符串
     */
    virtual std::string applyStyle(Style style) const = 0;
    
    /**
     * @brief 检查当前环境是否支持颜色输出
     * @return 如果支持颜色输出则返回true，否则返回false
     */
    virtual bool supportsColor() const = 0;
};

/**
 * @brief 空颜色处理器
 * 
 * 不执行任何颜色处理，用于不支持颜色的环境
 */
class NullColorHandler : public ColorHandler {
public:
    std::string applyColor(Color) const override { return ""; }
    std::string applyBackground(Background) const override { return ""; }
    std::string applyStyle(Style) const override { return ""; }
    bool supportsColor() const override { return false; }
};

/**
 * @brief ANSI颜色处理器
 * 
 * 使用ANSI转义序列实现颜色输出
 */
class AnsiColorHandler : public ColorHandler {
public:
    std::string applyColor(Color color) const override;
    std::string applyBackground(Background bg) const override;
    std::string applyStyle(Style style) const override;
    bool supportsColor() const override;
};

#ifdef _WIN32
/**
 * @brief Windows控制台颜色处理器
 * 
 * 使用Windows API实现颜色输出
 */
class WindowsColorHandler : public ColorHandler {
public:
    std::string applyColor(Color color) const override;
    std::string applyBackground(Background bg) const override;
    std::string applyStyle(Style style) const override;
    bool supportsColor() const override;
};
#endif

/**
 * @brief 颜色输出管理器
 * 
 * 管理颜色处理器并提供全局配置
 */
class ColorOutput {
public:
    /**
     * @brief 获取单例实例
     * @return ColorOutput单例引用
     */
    static ColorOutput& getInstance();
    
    /**
     * @brief 设置是否启用颜色输出
     * @param enabled 是否启用
     */
    void setEnabled(bool enabled);
    
    /**
     * @brief 检查颜色输出是否启用
     * @return 如果启用则返回true，否则返回false
     */
    bool isEnabled() const;
    
    /**
     * @brief 获取当前颜色处理器
     * @return 颜色处理器指针
     */
    const ColorHandler* getHandler() const;
    
    /**
     * @brief 设置自定义颜色处理器
     * @param handler 自定义处理器
     */
    void setHandler(std::unique_ptr<ColorHandler> handler);
    
    /**
     * @brief 自动检测并设置最合适的颜色处理器
     */
    void autoDetectHandler();
    
private:
    ColorOutput();
    ~ColorOutput() = default;
    ColorOutput(const ColorOutput&) = delete;
    ColorOutput& operator=(const ColorOutput&) = delete;
    
    bool m_enabled = true;
    std::unique_ptr<ColorHandler> m_handler;
};

/**
 * @brief 颜色格式化器
 * 
 * 用于创建带颜色的字符串
 */
class ColorFormatter {
public:
    /**
     * @brief 使用指定颜色格式化文本
     * @param text 要格式化的文本
     * @param color 文本颜色
     * @return 格式化后的字符串
     */
    static std::string colorize(const std::string& text, Color color);
    
    /**
     * @brief 使用指定颜色和背景色格式化文本
     * @param text 要格式化的文本
     * @param color 文本颜色
     * @param bg 背景颜色
     * @return 格式化后的字符串
     */
    static std::string colorize(const std::string& text, Color color, Background bg);
    
    /**
     * @brief 使用指定颜色、背景色和样式格式化文本
     * @param text 要格式化的文本
     * @param color 文本颜色
     * @param bg 背景颜色
     * @param style 文本样式
     * @return 格式化后的字符串
     */
    static std::string colorize(const std::string& text, Color color, Background bg, Style style);
};

/**
 * @brief 颜色输出流操作符
 * @param os 输出流
 * @param color 颜色值
 * @return 输出流引用
 */
std::ostream& operator<<(std::ostream& os, Color color);

/**
 * @brief 背景色输出流操作符
 * @param os 输出流
 * @param bg 背景色值
 * @return 输出流引用
 */
std::ostream& operator<<(std::ostream& os, Background bg);

/**
 * @brief 样式输出流操作符
 * @param os 输出流
 * @param style 样式值
 * @return 输出流引用
 */
std::ostream& operator<<(std::ostream& os, Style style);

} // namespace webserver::color