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
    echo ""
    echo "示例:"
    echo "  $0 coverage"
    echo "  $0 coverage --html"
    echo "  $0 coverage --html --xml"
    echo ""
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
            *)
                echo "未知选项: $1"
                show_coverage_help
                exit 1
                ;;
        esac
    done
    
    echo "开始生成测试覆盖率报告..."
    
    # 检查gcov和lcov是否已安装
    if ! command -v gcov >/dev/null 2>&1; then
        echo "错误: gcov 未安装，请先安装 gcov"
        exit 1
    fi
    
    if ! command -v lcov >/dev/null 2>&1; then
        echo "错误: lcov 未安装，请先安装 lcov"
        exit 1
    fi
    
    if ! command -v genhtml >/dev/null 2>&1; then
        echo "错误: genhtml 未安装，请先安装 lcov"
        exit 1
    fi
    
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
          -DCOVERAGE=true \
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