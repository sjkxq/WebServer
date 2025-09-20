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
    echo "  --ignore-errors           忽略geninfo错误"
    echo "  --atomic-profile          使用原子配置文件更新（解决负计数问题）"
    echo ""
    echo "示例:"
    echo "  $0 coverage"
    echo "  $0 coverage --html"
    echo "  $0 coverage --html --xml"
    echo "  $0 coverage --auto-install"
    echo "  $0 coverage --ignore-errors --atomic-profile"
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
    
    # 检查参数中是否包含--auto-install
    for param in "$@"; do
        if [[ "$param" == "--auto-install" ]]; then
            AUTO_INSTALL=true
            break
        fi
    done
    
    local missing_deps=()
    
    # 检查gcov是否已安装
    if ! command -v gcov >/dev/null 2>&1; then
        missing_deps+=("gcov")
    fi
    
    # 检查lcov是否已安装
    if ! command -v lcov >/dev/null 2>&1; then
        missing_deps+=("lcov")
    fi
    
    # 检查genhtml是否已安装
    if ! command -v genhtml >/dev/null 2>&1; then
        missing_deps+=("genhtml")
    fi
    
    # 如果有缺失的依赖
    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        echo "错误: 以下依赖未安装: ${missing_deps[*]}"
        
        if [[ "$AUTO_INSTALL" == true ]]; then
            echo "尝试自动安装依赖..."
            if ! install_dependencies; then
                echo "自动安装失败，请手动安装依赖"
                exit 1
            fi
            
            # 重新检查依赖
            echo "重新检查依赖..."
            local still_missing=()
            if ! command -v gcov >/dev/null 2>&1; then
                still_missing+=("gcov")
            fi
            if ! command -v lcov >/dev/null 2>&1; then
                still_missing+=("lcov")
            fi
            if ! command -v genhtml >/dev/null 2>&1; then
                still_missing+=("genhtml")
            fi
            
            if [[ ${#still_missing[@]} -gt 0 ]]; then
                echo "依赖安装后仍然存在问题，请手动检查安装: ${still_missing[*]}"
                exit 1
            fi
        else
            echo "请安装缺失的依赖后再运行此脚本"
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
    local IGNORE_ERRORS=false
    local ATOMIC_PROFILE=false
    
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
            --ignore-errors)
                IGNORE_ERRORS=true
                shift
                ;;
            --atomic-profile)
                ATOMIC_PROFILE=true
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
    
    # 检查依赖，传递所有原始参数
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
    
    # 构建CMake命令
    local CMAKE_CMD="cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_COVERAGE=true"
    
    if [[ "$ATOMIC_PROFILE" == true ]]; then
        CMAKE_CMD="$CMAKE_CMD -DCMAKE_CXX_FLAGS=\"--coverage -fprofile-update=atomic\" -DCMAKE_C_FLAGS=\"--coverage -fprofile-update=atomic\""
    else
        CMAKE_CMD="$CMAKE_CMD -DCMAKE_CXX_FLAGS=\"--coverage\" -DCMAKE_C_FLAGS=\"--coverage\""
    fi
    
    CMAKE_CMD="$CMAKE_CMD \"$PROJECT_DIR\""
    
    # 使用eval执行命令以正确处理引号
    eval $CMAKE_CMD
    
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
    local LCOV_FLAGS=""
    if [[ "$IGNORE_ERRORS" == true ]]; then
        LCOV_FLAGS="--ignore-errors mismatch,gcov,negative,unused"
    else
        # 默认忽略mismatch、gcov和unused错误，但不忽略negative错误
        LCOV_FLAGS="--ignore-errors mismatch,gcov,unused"
    fi
    
    # 正确配置geninfo参数
    local GENINFO_FLAGS="--rc geninfo_unexecuted_blocks=1"
    if [[ "$ATOMIC_PROFILE" == true ]]; then
        GENINFO_FLAGS="$GENINFO_FLAGS --rc geninfo_gcov_tool='gcov --profile-update=atomic'"
    fi
    
    # 正确导出环境变量
    export LC_GCOV_ARGS="geninfo_unexecuted_blocks=1"
    if [[ "$ATOMIC_PROFILE" == true ]]; then
        export LC_GCOV_ARGS="geninfo_unexecuted_blocks=1 --rc geninfo_gcov_tool='gcov --profile-update=atomic'"
    fi
    
    # 尝试捕获覆盖率数据
    if ! lcov --capture --directory . --output-file coverage.info $LCOV_FLAGS $GENINFO_FLAGS; then
        echo "警告: 覆盖率数据捕获失败，尝试使用忽略所有错误模式..."
        if [[ "$IGNORE_ERRORS" == false ]]; then
            echo "重新尝试并忽略所有错误..."
            if ! lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch,gcov,negative,unused $GENINFO_FLAGS; then
                echo "错误: 即使忽略错误也无法捕获覆盖率数据"
                exit 1
            fi
        else
            echo "错误: 覆盖率数据捕获失败"
            exit 1
        fi
    fi
    
    # 过滤掉系统头文件和第三方库
    echo "过滤系统和第三方库的覆盖率数据..."
    local REMOVE_FLAGS="--ignore-errors unused"
    
    if ! lcov --remove coverage.info \
         '/usr/include/*' \
         '*/build/_deps/*' \
         '*/test/*' \
         '*/nlohmann/*' \
         --output-file coverage_filtered.info $REMOVE_FLAGS; then
        echo "警告: 过滤覆盖率数据时出现问题，尝试继续..."
    fi
    
    # 创建输出目录
    mkdir -p "$OUTPUT_DIR"
    
    # 生成HTML报告
    if [[ "$GENERATE_HTML" == true ]]; then
        echo "生成HTML格式覆盖率报告..."
        if genhtml coverage_filtered.info \
                 --output-directory "$OUTPUT_DIR/html" \
                 --title "WebServer 测试覆盖率报告" \
                 --legend \
                 --show-details; then
            echo "HTML格式覆盖率报告已生成到: $OUTPUT_DIR/html"
        else
            echo "警告: HTML报告生成过程中出现问题"
        fi
    fi
    
    # 生成XML报告
    if [[ "$GENERATE_XML" == true ]]; then
        echo "生成XML格式覆盖率报告..."
        if lcov --list coverage_filtered.info --rc lcov_branch_coverage=1 > "$OUTPUT_DIR/coverage.xml"; then
            echo "XML格式覆盖率报告已生成到: $OUTPUT_DIR/coverage.xml"
        else
            echo "警告: XML报告生成过程中出现问题"
        fi
    fi
    
    # 显示覆盖率摘要
    echo ""
    echo "测试覆盖率摘要:"
    echo "================"
    if ! lcov --list coverage_filtered.info; then
        echo "警告: 无法显示覆盖率摘要"
    fi
    
    echo ""
    echo "测试覆盖率报告生成完成!"
    
    if [[ "$GENERATE_HTML" == true ]]; then
        echo "HTML报告位置: $OUTPUT_DIR/html/index.html"
    fi
    
    if [[ "$GENERATE_XML" == true ]]; then
        echo "XML报告位置: $OUTPUT_DIR/coverage.xml"
    fi
}