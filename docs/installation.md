# 安装指南

## 依赖项

- CMake 3.10+
- GCC 7+/Clang 5+ (支持C++14)
- Google Test (用于单元测试)
- Google Benchmark (用于性能测试)
- nlohmann/json (用于配置解析)

## 构建方法

### 使用构建脚本（推荐）

项目提供了一个便捷的zsh构建脚本，可以一键完成构建和测试：

```bash
# 基本用法（Debug模式构建并运行测试）
./build.sh

# 使用Release模式构建
./build.sh --release

# 清理构建目录并重新构建
./build.sh --clean

# 构建但跳过测试
./build.sh --no-tests

# 显示详细的构建输出
./build.sh --verbose

# 设置并行构建任务数
./build.sh --jobs 8

# 显示帮助信息
./build.sh --help
```

### 手动构建

如果你不想使用构建脚本，也可以手动执行构建步骤：

```bash
mkdir build
cd build
cmake ..
make
```