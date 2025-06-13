# C++ WebServer

一个基于C++的高性能Web服务器实现，支持多线程处理和彩色日志输出。

## 功能特性

- 基于C++14标准开发
- 使用现代CMake构建系统
- 支持HTTP/1.1协议
- 多线程处理请求，基于线程池实现
- 可配置的日志系统，支持多种级别(ERROR/WARNING/INFO/DEBUG)
- 彩色终端输出支持，增强日志可读性
- 内置性能基准测试
- 全面的单元测试覆盖

## 依赖项

- CMake 3.10+
- GCC 7+/Clang 5+ (支持C++14)
- Google Test (用于单元测试)
- Google Benchmark (用于性能测试)
- nlohmann/json (用于配置解析)

## 日志系统使用示例

```cpp
#include "Logger.hpp"

// 设置日志级别(默认为INFO)
Logger::setMinLevel(Logger::Level::DEBUG);

// 记录不同级别日志
LOG_ERROR("This is an error message");
LOG_WARNING("This is a warning message"); 
LOG_INFO("This is an info message");
LOG_DEBUG("This is a debug message");
```

日志格式示例：
```
2023-11-15 14:30:45.123 [ERROR] This is an error message
2023-11-15 14:30:45.124 [WARNING] This is a warning message
```

## 性能基准测试

项目包含Google Benchmark性能测试，用于评估线程池和日志系统的性能。

## 单元测试

项目使用Google Test框架进行单元测试，覆盖核心组件功能。

### 构建和运行测试

```bash
# 在项目根目录下
mkdir -p build && cd build
cmake .. -DBUILD_TESTS=ON
make
ctest --output-on-failure  # 运行所有测试
```

### 测试覆盖范围

- WebServer核心功能测试
- 线程池任务调度测试
- 日志系统格式和级别测试
- 配置文件加载测试

### 添加新测试

1. 在`test/`目录下创建新的测试文件
2. 遵循现有测试模式
3. 包含成功和失败场景的测试用例
4. 如需添加新测试源文件，请更新CMakeLists.txt

### 构建和运行测试

```bash
# 在项目根目录下
mkdir -p build && cd build
cmake .. -DBUILD_BENCHMARK=ON
make
./benchmark/webserver_benchmark
```

### 测试内容

1. 线程池性能测试：
   - 任务提交速度
   - 多线程任务吞吐量

2. 日志系统性能测试：
   - 不同日志级别的性能开销
   - 多线程环境下的日志性能

### 示例输出格式
```
--------------------------------------------------------------------
Benchmark                          Time             CPU   Iterations
--------------------------------------------------------------------
BM_ThreadPoolSubmit/4           1254 ns         1254 ns       560000
BM_MultiThreadLogger/8         32451 ns        32451 ns        21500
```

## 构建方法

```bash
mkdir build
cd build
cmake ..
make
```

## 使用方法

运行编译后的可执行文件：
```bash
./webserver
```

默认服务器将在本地8080端口启动。

## 日志系统

- INFO: 一般运行信息
- DEBUG: 调试信息(当前已禁用)

## 彩色输出功能

提供完整的终端彩色文本输出支持，用于增强日志和消息的可读性。

### 主要特性
- 支持标准ANSI 256色
- 自动检测终端颜色支持能力
- 预定义16种常用颜色常量
- 支持前景色、背景色和文本样式组合
- 线程安全的输出操作
- 支持颜色嵌套和重置

### 基本使用方法

```cpp
#include "ColorOutput.hpp"

// 简单彩色输出
ColorOutput::println("错误信息", ColorOutput::Color::RED);
ColorOutput::print("警告信息", ColorOutput::Color::YELLOW);

// 组合样式
ColorOutput::println("重要信息", 
    ColorOutput::Color::WHITE, 
    ColorOutput::Color::RED,
    ColorOutput::Style::BOLD
);

// 嵌套使用
{
    ColorOutput::ScopedColor scoped(ColorOutput::Color::GREEN);
    std::cout << "这部分文本是绿色的" << std::endl;
    // 自动恢复原颜色
}

// 格式化输出
ColorOutput::printf(ColorOutput::Color::CYAN, "CPU使用率: %.2f%%\n", 75.32);
```

### 预定义颜色和样式

#### 基础颜色
| 常量 | 描述 | 示例 |
|------|------|------|
| BLACK | 黑色 | ![#000000](https://placehold.co/15x15/000000/000000.png) |
| RED | 红色 | ![#FF0000](https://placehold.co/15x15/FF0000/FF0000.png) |
| GREEN | 绿色 | ![#00FF00](https://placehold.co/15x15/00FF00/00FF00.png) |
| YELLOW | 黄色 | ![#FFFF00](https://placehold.co/15x15/FFFF00/FFFF00.png) |
| BLUE | 蓝色 | ![#0000FF](https://placehold.co/15x15/0000FF/0000FF.png) |
| MAGENTA | 洋红 | ![#FF00FF](https://placehold.co/15x15/FF00FF/FF00FF.png) |
| CYAN | 青色 | ![#00FFFF](https://placehold.co/15x15/00FFFF/00FFFF.png) |
| WHITE | 白色 | ![#FFFFFF](https://placehold.co/15x15/FFFFFF/FFFFFF.png) |

#### 亮色变体
所有基础颜色都有对应的`BRIGHT_`前缀亮色版本，如`BRIGHT_RED`。

#### 文本样式
| 常量 | 效果 |
|------|------|
| BOLD | 加粗文本 |
| DIM | 淡化文本 |
| ITALIC | 斜体文本 |
| UNDERLINE | 下划线 |
| BLINK | 闪烁文本 |
| REVERSE | 反转前景/背景色 |

### 高级功能

1. **自定义颜色**:
```cpp
// 使用RGB值创建颜色
ColorOutput::Color customColor = ColorOutput::rgb(100, 200, 50);
ColorOutput::println("自定义颜色文本", customColor);
```

2. **颜色主题**:
```cpp
// 定义主题
const ColorOutput::Color ERROR_THEME = ColorOutput::BRIGHT_RED;
const ColorOutput::Color WARNING_THEME = ColorOutput::YELLOW;

// 应用主题
ColorOutput::println("系统错误", ERROR_THEME);
```

### 最佳实践

1. **日志系统集成**:
```cpp
void logError(const std::string& message) {
    ColorOutput::println("[ERROR] " + message, ColorOutput::BRIGHT_RED);
}
```

2. **性能敏感场景**:
```cpp
// 提前检查颜色支持
if(ColorOutput::isColorSupported()) {
    ColorOutput::println("状态正常", ColorOutput::GREEN);
}
```

3. **跨平台兼容**:
- Windows: 需要启用VT100转义序列支持
- Linux/macOS: 原生支持

### 注意事项

1. 在不支持颜色的终端中会自动回退到普通文本
2. 可通过`ColorOutput::setColorEnabled(false)`全局禁用颜色
3. 建议日志文件中禁用颜色输出
4. 颜色代码会增加输出内容的长度
5. 某些终端可能不支持所有样式(如闪烁)
6. 考虑色盲用户的体验，不要仅依赖颜色传递信息