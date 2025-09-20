#!/bin/bash

# WebServer项目安装脚本

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

# 显示帮助信息
show_install_help() {
    echo "安装项目"
    echo ""
    echo "用法: $0 install [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help                显示帮助信息"
    echo "  -b, --build-dir DIR       设置构建目录"
    echo "                            默认: build"
    echo "  -p, --prefix PREFIX       设置安装前缀"
    echo "                            默认: /usr/local"
    echo ""
    echo "示例:"
    echo "  $0 install"
    echo "  $0 install --prefix ~/webserver"
    echo ""
}

# 安装项目主函数
install_project() {
    local INSTALL_PREFIX="/usr/local"
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_install_help
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
            -p|--prefix)
                if [[ -z "$2" || "$2" == -* ]]; then
                    echo "错误: --prefix 选项需要一个参数"
                    exit 1
                fi
                INSTALL_PREFIX="$2"
                shift 2
                ;;
            *)
                echo "错误: 未知选项 $1"
                show_install_help
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
    
    # 安装项目
    echo "安装项目到 $INSTALL_PREFIX ..."
    
    if ! cmake --install . --prefix "$INSTALL_PREFIX"; then
        echo "安装失败!"
        exit 1
    fi
    
    echo "安装完成!"
}