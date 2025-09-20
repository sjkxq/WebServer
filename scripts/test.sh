#!/bin/bash

# WebServer项目测试脚本

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

# 显示帮助信息
show_test_help() {
    echo "运行测试"
    echo ""
    echo "用法: $0 test [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help                显示帮助信息"
    echo "  -b, --build-dir DIR       设置构建目录"
    echo "                            默认: build"
    echo "  -v, --verbose             启用详细输出"
    echo "  --low-memory              低内存模式运行测试"
    echo ""
    echo "示例:"
    echo "  $0 test"
    echo "  $0 test --verbose"
    echo ""
}

# 运行测试主函数
run_tests() {
    local VERBOSE=false
    local LOW_MEMORY_MODE=false
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_test_help
                return 0
                ;;
            -b|--build-dir)
                if [[ -z "$2" || "$2" == -* ]]; then
                    echo "错误: --build-dir 选项需要一个参数"
                    exit 1
                fi
                BUILD_DIR="$2"
                shift 2
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            --low-memory)
                LOW_MEMORY_MODE=true
                shift
                ;;
            *)
                echo "错误: 未知选项 $1"
                show_test_help
                exit 1
                ;;
        esac
    done
    
    # 检查构建目录是否存在
    if [ ! -d "$BUILD_DIR" ]; then
        echo "错误: 构建目录 '$BUILD_DIR' 不存在"
        echo "请先运行构建命令: $0 build"
        exit 1
    fi
    
    cd "$BUILD_DIR"
    
    # 运行单元测试
    echo "运行单元测试..."
    
    if [ "$LOW_MEMORY_MODE" = true ]; then
        echo "低内存模式: 串行运行测试..."
        if [ "$VERBOSE" = true ]; then
            ctest --output-on-failure -j 1
        else
            ctest -j 1
        fi
    else
        # 正常模式下，使用默认并行度
        if [ "$VERBOSE" = true ]; then
            ctest --output-on-failure
        else
            ctest
        fi
    fi
    
    if [ $? -ne 0 ]; then
        echo "单元测试失败!"
        exit 1
    fi
    
    echo "单元测试通过!"
}