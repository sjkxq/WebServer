#!/bin/bash

# WebServer项目测试覆盖率报告生成脚本

# 显示帮助信息
show_coverage_help() {
    echo "生成测试覆盖率报告"
    echo ""
    echo "用法: $0 coverage [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help                显示帮助信息"
    echo "  -b, --build-dir DIR       设置构建目录"
    echo "                            默认: build"
    echo "  -o, --output-dir DIR      设置覆盖率报告输出目录"
    echo "                            默认: coverage-report"
    echo "  --html                    生成HTML格式报告"
    echo "  --xml                     生成XML格式报告"
    echo "  --low-memory              低内存模式运行测试"
    echo "  --auto-install            自动安装缺失的依赖（需要sudo权限）"
    echo ""
    echo "示例:"
    echo "  $0 coverage"
    echo "  $0 coverage --html"
    echo "  $0 coverage --html --xml"
    echo "  $0 coverage --auto-install"
    echo ""
}

# 检测操作系统类型
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v apt-get >/dev/null 2>&1; then
            echo "debian"
        elif command -v yum >/dev/null 2>&1; then
            echo "redhat"
        elif command -v pacman >/dev/null 2>&1; then
            echo "arch"
        else
            echo "unknown"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    else
        echo "unknown"
    fi
}

# 尝试自动安装依赖
install_dependencies() {
    local OS_TYPE=$(detect_os)
    echo "检测到操作系统类型: $OS_TYPE"
    
    case $OS_TYPE in
        debian)
            echo "尝试使用 apt-get 安装依赖..."
            if command -v sudo >/dev/null 2>&1; then
                sudo apt-get update
                sudo apt-get install -y lcov gcovr
            else
                echo "警告: 未找到sudo命令，尝试直接安装..."
                apt-get update
                apt-get install -y lcov gcovr
            fi
            ;;
        redhat)
            echo "尝试使用 yum 安装依赖..."
            if command -v sudo >/dev/null 2>&1; then
                sudo yum install -y lcov gcovr
            else
                echo "警告: 未找到sudo命令，尝试直接安装..."
                yum install -y lcov gcovr
            fi
            ;;
        arch)
            echo "尝试使用 pacman 安装依赖..."
            if command -v sudo >/dev/null 2>&1; then
                sudo pacman -Syu lcov gcovr --noconfirm
            else
                echo "警告: 未找到sudo命令，尝试直接安装..."
                pacman -Syu lcov gcovr --noconfirm
            fi
            ;;
        macos)
            echo "尝试使用 brew 安装依赖..."
            if command -v brew >/dev/null 2>&1; then
                brew install lcov gcovr
            else
                echo "错误: 未找到brew命令，请先安装Homebrew"
                return 1
            fi
            ;;
        *)
            echo "错误: 不支持的操作系统或包管理器"
            echo "请手动安装以下依赖:"
            echo "  - lcov"
            echo "  - gcovr"
            return 1
            ;;
    esac
}

# 检查依赖
check_dependencies() {
    local AUTO_INSTALL=false
    
    # 检查是否有--auto-install参数
    for arg in "$@"; do
        if [[ "$arg" == "--auto-install" ]]; then
            AUTO_INSTALL=true
            break
        fi
    done
    
    # 检查gcov是否已安装
    if ! command -v gcov >/dev/null 2>&1; then
        echo "错误: gcov 未安装"
        if [[ "$AUTO_INSTALL" == true ]]; then
            echo "尝试自动安装依赖..."
            if ! install_dependencies; then
                echo "自动安装失败，请手动安装依赖"
                exit 1
            fi
        else
            echo "请安装gcov后再运行此脚本"
            echo "提示: 可使用 --auto-install 参数尝试自动安装依赖"
            exit 1
        fi
    fi
    
    # 检查lcov是否已安装
    if ! command -v lcov >/dev/null 2>&1; then
        echo "错误: lcov 未安装"
        if [[ "$AUTO_INSTALL" == true ]]; then
            echo "尝试自动安装依赖..."
            if ! install_dependencies; then
                echo "自动安装失败，请手动安装依赖"
                exit 1
            fi
        else
            echo "请安装lcov后再运行此脚本"
            echo "提示: 可使用 --auto-install 参数尝试自动安装依赖"
            exit 1
        fi
    fi
    
    # 检查genhtml是否已安装
    if ! command -v genhtml >/dev/null 2>&1; then
        echo "错误: genhtml 未安装"
        if [[ "$AUTO_INSTALL" == true ]]; then
            echo "尝试自动安装依赖..."
            if ! install_dependencies; then
                echo "自动安装失败，请手动安装依赖"
                exit 1
            fi
        else
            echo "请安装lcov (包含genhtml)后再运行此脚本"
            echo "提示: 可使用 --auto-install 参数尝试自动安装依赖"
            exit 1
        fi
    fi
    
    echo "所有依赖检查通过"
}

# 生成测试覆盖率报告主函数
generate_coverage() {
    local SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
    local BUILD_DIR="$PROJECT_DIR/build"
    local OUTPUT_DIR="$PROJECT_DIR/coverage-report"
    local GENERATE_HTML=false
    local GENERATE_XML=false
    local LOW_MEMORY_MODE=false
    local AUTO_INSTALL=false
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_coverage_help
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
            -o|--output-dir)
                if [[ -z "$2" || "$2" == -* ]]; then
                    echo "错误: --output-dir 选项需要一个参数"
                    exit 1
                fi
                OUTPUT_DIR="$2"
                shift 2
                ;;
            --html)
                GENERATE_HTML=true
                shift
                ;;
            --xml)
                GENERATE_XML=true
                shift
                ;;
            --low-memory)
                LOW_MEMORY_MODE=true
                shift
                ;;
            --auto-install)
                AUTO_INSTALL=true
                shift
                ;;
            *)
                echo "未知选项: $1"
                show_coverage_help
                exit 1
                ;;
        esac
    done
    
    echo "开始生成测试覆盖率报告..."
    
    # 检查依赖
    check_dependencies "$@"
    
    # 如果没有指定格式，则默认生成HTML报告
    if [[ "$GENERATE_HTML" == false && "$GENERATE_XML" == false ]]; then
        GENERATE_HTML=true
    fi
    
    # 创建构建目录（如果不存在）
    if [[ ! -d "$BUILD_DIR" ]]; then
        echo "创建构建目录: $BUILD_DIR"
        mkdir -p "$BUILD_DIR"
    fi
    
    # 进入构建目录
    cd "$BUILD_DIR"
    
    # 配置CMake以启用覆盖率
    echo "配置CMake以启用测试覆盖率..."
    cmake -DCMAKE_BUILD_TYPE=Debug \
          -DBUILD_COVERAGE=true \
          -DCMAKE_CXX_FLAGS="--coverage" \
          -DCMAKE_C_FLAGS="--coverage" \
          "$PROJECT_DIR"
    
    # 构建项目
    echo "构建项目..."
    if [[ "$LOW_MEMORY_MODE" == true ]]; then
        cmake --build . --parallel 1
    else
        cmake --build . --parallel
    fi
    
    # 运行测试以生成覆盖率数据
    echo "运行测试以生成覆盖率数据..."
    if [[ "$LOW_MEMORY_MODE" == true ]]; then
        GTEST_ALSO_RUN_DISABLED_TESTS=1 ctest --output-on-failure -j1
    else
        GTEST_ALSO_RUN_DISABLED_TESTS=1 ctest --output-on-failure
    fi
    
    # 捕获覆盖率数据
    echo "捕获覆盖率数据..."
    lcov --capture --directory . --output-file coverage.info
    
    # 过滤掉系统头文件和第三方库
    echo "过滤系统和第三方库的覆盖率数据..."
    lcov --remove coverage.info \
         '/usr/*' \
         '*/third_party/*' \
         '*/build/*' \
         '*/test/*' \
         '*/googletest/*' \
         '*/nlohmann/*' \
         --output-file coverage_filtered.info
    
    # 创建输出目录
    mkdir -p "$OUTPUT_DIR"
    
    # 生成HTML报告
    if [[ "$GENERATE_HTML" == true ]]; then
        echo "生成HTML格式覆盖率报告..."
        genhtml coverage_filtered.info \
                 --output-directory "$OUTPUT_DIR/html" \
                 --title "WebServer 测试覆盖率报告" \
                 --legend \
                 --show-details
        
        echo "HTML格式覆盖率报告已生成到: $OUTPUT_DIR/html"
    fi
    
    # 生成XML报告
    if [[ "$GENERATE_XML" == true ]]; then
        echo "生成XML格式覆盖率报告..."
        lcov --list coverage_filtered.info --rc lcov_branch_coverage=1 > "$OUTPUT_DIR/coverage.xml"
        echo "XML格式覆盖率报告已生成到: $OUTPUT_DIR/coverage.xml"
    fi
    
    # 显示覆盖率摘要
    echo ""
    echo "测试覆盖率摘要:"
    echo "================"
    lcov --list coverage_filtered.info
    
    echo ""
    echo "测试覆盖率报告生成完成!"
    
    if [[ "$GENERATE_HTML" == true ]]; then
        echo "HTML报告位置: $OUTPUT_DIR/html/index.html"
    fi
    
    if [[ "$GENERATE_XML" == true ]]; then
        echo "XML报告位置: $OUTPUT_DIR/coverage.xml"
    fi
}