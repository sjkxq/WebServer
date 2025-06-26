# 开发指南

## 开发环境设置

### 推荐工具
- IDE: VSCode 或 CLion
- 插件:
  - CMake Tools
  - C/C++
  - GitLens

### 环境配置
1. 安装开发依赖:
```bash
# Ubuntu
sudo apt-get install -y clang-format cppcheck

# macOS
brew install clang-format cppcheck
```

2. 配置VSCode:
- 安装推荐插件
- 创建`.vscode/settings.json`:
```json
{
  "cmake.configureOnOpen": true,
  "editor.formatOnSave": true,
  "C_Cpp.clang_format_style": "file"
}
```

## 代码规范

### 代码风格
- 遵循Google C++ Style Guide
- 使用clang-format格式化代码
- 配置文件: `.clang-format`

### 静态检查
```bash
cppcheck --enable=all --inconclusive --std=c++20 src/
```

## Git工作流

### 分支策略
- `main`: 稳定版本
- `dev`: 开发分支
- `feature/*`: 功能分支
- `bugfix/*`: 修复分支

### 提交规范
```bash
git commit -m "类型: 简短描述

详细描述（可选）"
```
类型包括: feat, fix, docs, style, refactor, test, chore

## 测试策略

### 单元测试
```bash
cd build && ctest -V
```

### 覆盖率报告
```bash
mkdir build_coverage && cd build_coverage
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_COVERAGE=ON ..
make
make coverage
```

## 调试指南

### GDB调试
```bash
gdb ./build/webserver
```

### VSCode调试
1. 创建`.vscode/launch.json`
2. 使用CMake Tools配置调试目标

### 常见问题
- 内存泄漏检查:
```bash
valgrind --leak-check=full ./build/webserver
```