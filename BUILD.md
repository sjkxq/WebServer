# 构建说明

## 系统要求
- CMake 3.15+
- C++20 兼容编译器
- Ninja 1.10+ (推荐) 或 Make

## 依赖安装

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git
```

### macOS
```bash
brew update
brew install \
    cmake \
    ninja \
    git
```

## 构建步骤

### 使用构建脚本 (推荐)
```bash
./build_and_test.sh
```

### 手动构建 (Ninja)
```bash
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
```

### 手动构建 (Make)
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## 构建选项
可通过CMake选项自定义构建：

| 选项 | 描述 | 默认值 |
|------|------|--------|
| CMAKE_BUILD_TYPE | 构建类型 (Debug/Release) | Debug |
| BUILD_TESTS | 是否构建测试 | ON |
| BUILD_DOCS | 是否构建文档 | OFF |

示例：
```bash
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_DOCS=ON ..
```

## 测试
构建完成后运行测试：
```bash
cd build && ctest -V
```

## 常见问题

### CMake找不到编译器
确保已安装构建工具链：
- Ubuntu: `build-essential`
- macOS: Xcode命令行工具

### Ninja构建失败
清理构建目录后重试：
```bash
rm -rf build && mkdir build
```