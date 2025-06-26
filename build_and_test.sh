#!/bin/bash

# 构建和测试脚本
set -e

# 默认参数
BUILD_TYPE="Debug"
BUILD_DIR="build"
INSTALL_PREFIX="/usr/local"
CLEAN=false
VERBOSE=false
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

# 显示帮助信息
show_help() {
    echo "WebServer构建和测试脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help                显示帮助信息"
    echo "  -t, --type TYPE           设置构建类型 (Debug|Release|RelWithDebInfo|MinSizeRel)"
    echo "                            默认: Debug"
    echo "  -b, --build-dir DIR       设置构建目录"
    echo "                            默认: build"
    echo "  -p, --prefix PREFIX       设置安装前缀"
    echo "                            默认: /usr/local"
    echo "  -c, --clean               清理构建目录"
    echo "  -v, --verbose             启用详细输出"
    echo "  -j, --jobs N              并行构建作业数"
    echo "                            默认: 自动检测"
    echo ""
    echo "示例:"
    echo "  $0 --type Release --prefix ~/webserver"
    echo "  $0 --clean --verbose"
    echo ""
}

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -p|--prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        *)
            echo "错误: 未知选项 $1"
            show_help
            exit 1
            ;;
    esac
done

# 验证构建类型
if [[ ! "$BUILD_TYPE" =~ ^(Debug|Release|RelWithDebInfo|MinSizeRel)$ ]]; then
    echo "错误: 无效的构建类型 '$BUILD_TYPE'"
    echo "有效的构建类型: Debug, Release, RelWithDebInfo, MinSizeRel"
    exit 1
fi

# 设置CMake参数
CMAKE_ARGS=(
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    "-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX"
)

# 如果需要清理构建目录
if [ "$CLEAN" = true ] && [ -d "$BUILD_DIR" ]; then
    echo "清理构建目录: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

# 创建构建目录
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 检查Ninja是否可用
if command -v ninja >/dev/null 2>&1; then
    GENERATOR="Ninja"
    echo "检测到Ninja可用，将使用Ninja构建系统"
    CMAKE_ARGS+=("-G" "Ninja")
else
    GENERATOR="Unix Makefiles"
    echo "警告: Ninja未安装，将使用默认Make构建系统"
fi

# 配置项目
echo "配置项目 (构建类型: $BUILD_TYPE, 生成器: $GENERATOR)"
cmake "${CMAKE_ARGS[@]}" ..

# 构建项目
echo "构建项目 (使用 $JOBS 个作业)"
if [ "$GENERATOR" = "Ninja" ]; then
    if [ "$VERBOSE" = true ]; then
        ninja -v -j "$JOBS"
    else
        ninja -j "$JOBS"
    fi
else
    if [ "$VERBOSE" = true ]; then
        cmake --build . --config "$BUILD_TYPE" --verbose -j "$JOBS"
    else
        cmake --build . --config "$BUILD_TYPE" -j "$JOBS"
    fi
fi

echo "构建完成!"
echo "可执行文件位于: $BUILD_DIR/bin/webserver"

# 运行单元测试
echo ""
echo "运行单元测试..."
if ! ctest --output-on-failure; then
    echo "单元测试失败!"
    exit 1
fi
echo "单元测试通过!"

# 保存当前目录
CURRENT_DIR=$(pwd)

# 运行benchmark测试
echo ""
echo "运行benchmark测试..."
BENCHMARK_PATH="bin/webserver_benchmark"
if [ -f "$BENCHMARK_PATH" ]; then
    echo "执行: $BENCHMARK_PATH"
    if ! "./$BENCHMARK_PATH"; then
        echo "Benchmark测试失败!"
        cd "$CURRENT_DIR"  # 返回原始目录
        exit 1
    fi
    echo "Benchmark测试完成!"
else
    echo "警告: 未找到benchmark可执行文件: $BENCHMARK_PATH"
    echo "检查构建目录中的可执行文件:"
    ls -la bin/ || echo "无法列出 bin/ 目录内容"
fi

echo ""
echo "所有测试完成!"
echo ""
echo "要安装项目，请执行:"
echo "  cd $BUILD_DIR && sudo cmake --install ."
echo ""