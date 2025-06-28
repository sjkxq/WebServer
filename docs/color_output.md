# ColorOutput 模块文档

## 概述

ColorOutput 是一个跨平台的终端颜色输出模块，提供了简单易用的 API 来为终端输出添加颜色和样式。该模块支持多种平台（包括 ANSI 终端和 Windows 控制台），并提供了自动检测颜色支持的功能。

## 主要特性

- **跨平台支持**：自动适配 ANSI 终端和 Windows 控制台
- **丰富的颜色选项**：支持前景色、背景色和文本样式
- **自动检测**：能够检测终端是否支持颜色输出
- **可配置**：可以全局启用或禁用颜色输出
- **易于扩展**：支持自定义颜色处理器
- **多种使用方式**：支持流操作符和格式化函数

## 基本用法

### 使用流操作符

```cpp
#include "webserver/color/output.hpp"
#include <iostream>

using namespace webserver::color;

int main() {
    // 基本颜色输出
    std::cout << Color::RED << "This is red text" << Style::RESET << std::endl;
    
    // 组合颜色和样式
    std::cout << Color::BLUE << Background::WHITE << Style::BOLD 
              << "Bold blue text on white background" 
              << Style::RESET << std::endl;
              
    // 重置所有格式
    std::cout << "Back to normal text" << std::endl;
    
    return 0;
}
```

### 使用 ColorFormatter

```cpp
#include "webserver/color/output.hpp"
#include <iostream>

using namespace webserver::color;

int main() {
    // 基本颜色格式化
    std::string redText = ColorFormatter::colorize("This is red text", Color::RED);
    std::cout << redText << std::endl;
    
    // 组合颜色和背景
    std::string highlightedText = ColorFormatter::colorize(
        "Warning message", Color::BLACK, Background::YELLOW);
    std::cout << highlightedText << std::endl;
    
    // 完整格式化（颜色、背景和样式）
    std::string errorText = ColorFormatter::colorize(
        "Error: File not found", Color::RED, Background::DEFAULT, Style::BOLD);
    std::cout << errorText << std::endl;
    
    return 0;
}
```

## 全局配置

### 启用/禁用颜色输出

```cpp
// 禁用颜色输出
ColorOutput::getInstance().setEnabled(false);

// 启用颜色输出（默认）
ColorOutput::getInstance().setEnabled(true);

// 检查是否启用
bool isEnabled = ColorOutput::getInstance().isEnabled();
```

### 自动检测颜色支持

```cpp
// 自动检测并设置最合适的颜色处理器
ColorOutput::getInstance().autoDetectHandler();
```

## 自定义颜色处理器

您可以通过继承 `ColorHandler` 接口来创建自定义的颜色处理器：

```cpp
class MyCustomColorHandler : public ColorHandler {
public:
    std::string applyColor(Color color) const override {
        // 自定义颜色处理逻辑
        return "...";
    }
    
    std::string applyBackground(Background bg) const override {
        // 自定义背景色处理逻辑
        return "...";
    }
    
    std::string applyStyle(Style style) const override {
        // 自定义样式处理逻辑
        return "...";
    }
    
    bool supportsColor() const override {
        // 检查是否支持颜色
        return true;
    }
};

// 设置自定义处理器
ColorOutput::getInstance().setHandler(std::make_unique<MyCustomColorHandler>());
```

## 与日志系统集成

ColorOutput 模块可以轻松集成到日志系统中：

```cpp
#include "webserver/color/output.hpp"
#include "Logger.hpp"

void Logger::log(LogLevel level, const std::string& message) {
    using namespace webserver::color;
    
    std::string formattedMessage;
    switch (level) {
        case LogLevel::ERROR:
            formattedMessage = ColorFormatter::colorize(message, Color::RED, Background::DEFAULT, Style::BOLD);
            break;
        case LogLevel::WARNING:
            formattedMessage = ColorFormatter::colorize(message, Color::YELLOW);
            break;
        case LogLevel::INFO:
            formattedMessage = ColorFormatter::colorize(message, Color::GREEN);
            break;
        case LogLevel::DEBUG:
            formattedMessage = ColorFormatter::colorize(message, Color::BLUE);
            break;
        default:
            formattedMessage = message;
    }
    
    // 输出格式化后的消息
    std::cout << formattedMessage << std::endl;
}
```

## 平台特定注意事项

### ANSI 终端

- 大多数 Unix/Linux 终端和 macOS 终端支持 ANSI 颜色代码
- 某些终端可能支持扩展的 256 色或 RGB 颜色（当前版本未实现）

### Windows 控制台

- Windows 10 以上版本的控制台支持 ANSI 颜色代码
- 旧版 Windows 使用 Windows API 实现颜色支持
- Windows 控制台对样式支持有限（如斜体、闪烁等可能不支持）

## 性能考虑

- 颜色处理会增加少量开销，对于性能关键的应用，可以考虑在发布版本中禁用颜色输出
- 使用 `ColorFormatter` 进行预格式化可以减少重复的颜色代码生成

## 故障排除

如果颜色输出不正常，请检查：

1. 确认终端/控制台支持颜色输出
2. 检查 `ColorOutput::isEnabled()` 是否返回 `true`
3. 验证是否使用了正确的颜色处理器
4. 在 Windows 上，确认控制台没有被重定向到不支持颜色的设备

## 示例应用

### 创建彩色进度条

```cpp
#include "webserver/color/output.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace webserver::color;

void showProgress(int percentage) {
    const int barWidth = 50;
    int pos = barWidth * percentage / 100;
    
    std::cout << "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << Color::GREEN << "=" << Style::RESET;
        else if (i == pos) std::cout << Color::YELLOW << ">" << Style::RESET;
        else std::cout << " ";
    }
    
    std::cout << "] " << Color::CYAN << percentage << "%" << Style::RESET << "\r";
    std::cout.flush();
}

int main() {
    for (int i = 0; i <= 100; ++i) {
        showProgress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::cout << std::endl;
    return 0;
}
```

### 彩色表格输出

```cpp
#include "webserver/color/output.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

using namespace webserver::color;

void printTableRow(const std::vector<std::string>& cells, bool isHeader = false) {
    for (const auto& cell : cells) {
        if (isHeader) {
            std::cout << Color::WHITE << Background::BLUE << Style::BOLD
                      << std::setw(15) << cell << Style::RESET << " ";
        } else {
            std::cout << std::setw(15) << cell << " ";
        }
    }
    std::cout << std::endl;
}

int main() {
    // 表头
    printTableRow({"Name", "Age", "City"}, true);
    
    // 数据行
    printTableRow({"John", "25", "New York"});
    printTableRow({"Alice", "30", "London"});
    printTableRow({"Bob", "22", "Paris"});
    
    return 0;
}
```