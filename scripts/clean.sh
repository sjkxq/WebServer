#!/bin/bash

# WebServer项目清理脚本

# 显示帮助信息
show_clean_help() {
    echo "清理构建目录"
    echo ""
    echo "用法: $0 clean [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help                显示帮助信息"
    echo "  -b, --build-dir DIR       设置构建目录"
    echo "                            默认: build"
    echo ""
    echo "示例:"
    echo "  $0 clean"
    echo "  $0 clean --build-dir mybuild"
    echo ""
}

# 清理构建目录主函数
clean_build() {
    local SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
    local BUILD_DIR="$PROJECT_DIR/build"
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_clean_help
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
            *)
                echo "错误: 未知选项 $1"
                show_clean_help
                exit 1
                ;;
        esac
    done
    
    # 清理构建目录
    if [ -d "$BUILD_DIR" ]; then
        echo "清理构建目录: $BUILD_DIR"
        rm -rf "$BUILD_DIR"
        echo "清理完成!"
    else
        echo "构建目录 '$BUILD_DIR' 不存在，无需清理"
    fi
}