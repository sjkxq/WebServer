# 彩色输出功能文档

## 功能概述

日志系统支持彩色终端输出，不同日志级别会显示不同颜色：
- ERROR: 红色
- WARNING: 黄色
- INFO: 绿色
- DEBUG: 蓝色

## 启用/禁用彩色输出

```cpp
// 启用彩色输出（默认启用）
Logger::getInstance().setColorOutput(true);

// 禁用彩色输出
Logger::getInstance().setColorOutput(false);
```

## 自定义颜色

可以通过修改`Logger.cpp`中的颜色定义来调整颜色方案：

```cpp
// 颜色定义示例
const std::string ERROR_COLOR = "\033[1;31m";   // 红色
const std::string WARNING_COLOR = "\033[1;33m"; // 黄色
const std::string INFO_COLOR = "\033[1;32m";    // 绿色
const std::string DEBUG_COLOR = "\033[1;34m";   // 蓝色
const std::string RESET_COLOR = "\033[0m";      // 重置颜色
```

## 注意事项

1. 彩色输出仅在支持ANSI转义码的终端中有效
2. 在重定向输出到文件时建议禁用彩色输出
3. 某些IDE的控制台可能不支持所有颜色