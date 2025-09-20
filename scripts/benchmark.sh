#!/bin/bash

# WebServer项目性能测试脚本

# 显示帮助信息
show_benchmark_help() {
    echo "运行性能测试"
    echo ""
    echo "用法: $0 benchmark [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help                显示帮助信息"
    echo "  -b, --build-dir DIR       设置构建目录"
    echo "                            默认: build"
    echo "  --low-memory              低内存模式运行测试"
    echo ""
    echo "示例:"
    echo "  $0 benchmark"
    echo "  $0 benchmark --low-memory"
    echo ""
}

# 运行性能测试主函数
run_benchmark() {
    local SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
    local BUILD_DIR="$PROJECT_DIR/build"
    local LOW_MEMORY_MODE=false
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_benchmark_help
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
            --low-memory)
                LOW_MEMORY_MODE=true
                shift
                ;;
            *)
                echo "错误: 未知选项 $1"
                show_benchmark_help
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
    
    # 运行benchmark测试
    echo "运行benchmark测试..."
    
    BENCHMARK_PATH="bin/webserver_benchmark"
    if [ -f "$BENCHMARK_PATH" ]; then
        echo "执行: $BENCHMARK_PATH"
        
        # 在低内存模式下，可以调整benchmark参数以减少内存使用
        if [ "$LOW_MEMORY_MODE" = true ]; then
            echo "低内存模式: 使用较少的迭代次数运行benchmark..."
            if ! "./$BENCHMARK_PATH" --benchmark_min_time=0.1 --benchmark_repetitions=1; then
                echo "Benchmark测试失败!"
                exit 1
            fi
        else
            # 正常模式下，使用默认参数
            if ! "./$BENCHMARK_PATH"; then
                echo "Benchmark测试失败!"
                exit 1
            fi
        fi
        
        echo "Benchmark测试完成!"
    else
        echo "警告: 未找到benchmark可执行文件: $BENCHMARK_PATH"
        echo "检查构建目录中的可执行文件:"
        ls -la bin/ || echo "无法列出 bin/ 目录内容"
    fi
}