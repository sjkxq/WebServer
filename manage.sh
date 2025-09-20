#!/bin/bash

# WebServer项目统一管理脚本
# 用途: 统一管理WebServer项目的构建、测试、清理等操作

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

show_help() {
    echo "WebServer项目管理脚本"
    echo ""
    echo "用法: $0 [命令] [选项]"
    echo ""
    echo "命令:"
    echo "  init         为所有脚本授予执行权限"
    echo "  build        构建项目"
    echo "  test         运行测试"
    echo "  coverage     生成测试覆盖率报告"
    echo "  clean        清理构建目录"
    echo "  benchmark    运行性能测试"
    echo "  install      安装项目"
    echo "  help         显示帮助信息"
    echo ""
    echo "示例:"
    echo "  $0 init"
    echo "  $0 build"
    echo "  $0 test"
    echo "  $0 coverage"
    echo "  $0 clean"
    echo ""
}

# 加载其他脚本模块
load_modules() {
    if [[ -f "$SCRIPT_DIR/scripts/build.sh" ]]; then
        source "$SCRIPT_DIR/scripts/build.sh"
    fi
    
    if [[ -f "$SCRIPT_DIR/scripts/test.sh" ]]; then
        source "$SCRIPT_DIR/scripts/test.sh"
    fi
    
    if [[ -f "$SCRIPT_DIR/scripts/clean.sh" ]]; then
        source "$SCRIPT_DIR/scripts/clean.sh"
    fi
    
    if [[ -f "$SCRIPT_DIR/scripts/benchmark.sh" ]]; then
        source "$SCRIPT_DIR/scripts/benchmark.sh"
    fi
    
    if [[ -f "$SCRIPT_DIR/scripts/install.sh" ]]; then
        source "$SCRIPT_DIR/scripts/install.sh"
    fi
    
    if [[ -f "$SCRIPT_DIR/scripts/coverage.sh" ]]; then
        source "$SCRIPT_DIR/scripts/coverage.sh"
    fi
}

# 初始化脚本权限
init_scripts() {
    echo "为所有脚本授予执行权限..."
    
    # 为所有脚本文件授予执行权限
    chmod +x "$SCRIPT_DIR"/*.sh 2>/dev/null || echo "警告: 无法为某些脚本文件授予权限"
    
    echo "脚本权限初始化完成!"
}

# 主函数
main() {
    # 加载模块
    load_modules
    
    # 解析命令行参数
    case "$1" in
        init)
            shift
            init_scripts "$@"
            ;;
        build)
            shift
            build_project "$@"
            ;;
        test)
            shift
            run_tests "$@"
            ;;
        coverage)
            shift
            generate_coverage "$@"
            ;;
        clean)
            shift
            clean_build "$@"
            ;;
        benchmark)
            shift
            run_benchmark "$@"
            ;;
        install)
            shift
            install_project "$@"
            ;;
        help|--help|-h)
            show_help
            ;;
        "")
            show_help
            ;;
        *)
            echo "未知命令: $1"
            show_help
            exit 1
            ;;
    esac
}

# 如果直接运行此脚本，则执行main函数
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi