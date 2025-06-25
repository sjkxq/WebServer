#!/bin/bash

# 构建脚本
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
    echo "WebServer构建脚本"
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

# 配置项目
echo "配置项目 (构建类型: $BUILD_TYPE)"
cmake "${CMAKE_ARGS[@]}" ..

# 构建项目
echo "构建项目 (使用 $JOBS 个作业)"
if [ "$VERBOSE" = true ]; then
    cmake --build . --config "$BUILD_TYPE" --verbose -j "$JOBS"
else
    cmake --build . --config "$BUILD_TYPE" -j "$JOBS"
fi

echo "构建完成!"
echo "可执行文件位于: $BUILD_DIR/bin/webserver"
echo ""
echo "要运行测试，请执行:"
echo "  cd $BUILD_DIR && ctest"
echo ""
echo "要安装项目，请执行:"
echo "  cd $BUILD_DIR && sudo cmake --install ."
echo ""