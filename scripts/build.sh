#!/bin/bash

# WebServer项目构建脚本

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# 默认参数
BUILD_TYPE="Debug"
BUILD_DIR="$PROJECT_DIR/build"
INSTALL_PREFIX="/usr/local"
CLEAN=false
VERBOSE=false
SHOW_CPU_INFO=false
LOW_MEMORY_MODE=false
MAX_MEMORY=""
JOBS=""

# 检测系统内存
detect_memory() {
    TOTAL_MEMORY_KB=$(grep MemTotal /proc/meminfo 2>/dev/null | awk '{print $2}' || echo "")
    if [ -n "$TOTAL_MEMORY_KB" ]; then
        TOTAL_MEMORY_MB=$((TOTAL_MEMORY_KB / 1024))
        # 如果系统内存小于3GB，自动启用低内存模式
        if [ "$TOTAL_MEMORY_MB" -lt 3072 ]; then
            LOW_MEMORY_MODE=true
            echo "检测到系统内存较低 (${TOTAL_MEMORY_MB}MB)，自动启用低内存模式"
        fi
    fi
}

# 检测CPU核心数
detect_cpu_cores() {
    if command -v lscpu >/dev/null 2>&1; then
        # Linux with lscpu
        PHYSICAL_CORES=$(lscpu -p | grep -v '^#' | sort -u -t, -k 2,4 | wc -l)
        LOGICAL_CORES=$(nproc 2>/dev/null)
        # 如果物理核心数检测失败，使用逻辑核心数
        CORES=${PHYSICAL_CORES:-$LOGICAL_CORES}
    elif command -v sysctl >/dev/null 2>&1; then
        # macOS or BSD
        PHYSICAL_CORES=$(sysctl -n hw.physicalcpu 2>/dev/null)
        LOGICAL_CORES=$(sysctl -n hw.ncpu 2>/dev/null)
        # 如果物理核心数检测失败，使用逻辑核心数
        CORES=${PHYSICAL_CORES:-$LOGICAL_CORES}
    else
        # 默认值
        CORES=1
        echo "警告: 无法检测CPU核心数，默认使用1个核心"
    fi
}

# 根据系统情况设置默认作业数
set_default_jobs() {
    if [ "$LOW_MEMORY_MODE" = true ]; then
        # 低内存模式下，默认使用1个作业，无论有多少核心
        JOBS=1
        echo "低内存模式: 默认使用1个编译作业以减少内存使用"
    else
        # 正常模式下，使用核心数作为默认作业数
        JOBS=$CORES
        # 如果内存充足且核心数大于1，可以考虑使用核心数+1的作业数
        if [ "$CORES" -gt 1 ] && [ -n "$TOTAL_MEMORY_MB" ] && [ "$TOTAL_MEMORY_MB" -gt 4096 ]; then
            JOBS=$((CORES + 1))
        fi
    fi
}

# 显示系统信息
show_cpu_info() {
    echo "系统信息:"
    
    # CPU信息
    echo "== CPU信息 =="
    if command -v lscpu >/dev/null 2>&1; then
        lscpu | grep -E "^CPU\(s\)|Core|Thread|Model name"
    elif command -v sysctl >/dev/null 2>&1; then
        sysctl -n machdep.cpu.brand_string 2>/dev/null || echo "CPU型号: 未知"
        echo "物理核心数: $(sysctl -n hw.physicalcpu 2>/dev/null || echo "未知")"
        echo "逻辑核心数: $(sysctl -n hw.ncpu 2>/dev/null || echo "未知")"
    else
        echo "无法获取详细CPU信息，未找到lscpu或sysctl命令"
    fi
    
    # 内存信息
    echo ""
    echo "== 内存信息 =="
    if [ -f /proc/meminfo ]; then
        TOTAL_MEM=$(grep MemTotal /proc/meminfo | awk '{print $2}')
        FREE_MEM=$(grep MemFree /proc/meminfo | awk '{print $2}')
        AVAILABLE_MEM=$(grep MemAvailable /proc/meminfo | awk '{print $2}' 2>/dev/null)
        
        if [ -n "$TOTAL_MEM" ]; then
            echo "总内存: $((TOTAL_MEM / 1024)) MB"
        fi
        if [ -n "$FREE_MEM" ]; then
            echo "空闲内存: $((FREE_MEM / 1024)) MB"
        fi
        if [ -n "$AVAILABLE_MEM" ]; then
            echo "可用内存: $((AVAILABLE_MEM / 1024)) MB"
        fi
    elif command -v sysctl >/dev/null 2>&1; then
        # macOS
        TOTAL_MEM=$(sysctl -n hw.memsize 2>/dev/null)
        if [ -n "$TOTAL_MEM" ]; then
            echo "总内存: $((TOTAL_MEM / 1024 / 1024)) MB"
        fi
    else
        echo "无法获取内存信息"
    fi
    
    # 构建建议
    echo ""
    echo "== 构建建议 =="
    echo "当前自动检测的并行作业数: $JOBS"
    
    if [ "$LOW_MEMORY_MODE" = true ]; then
        echo "检测到低内存系统，建议使用以下选项:"
        echo "  --low-memory --jobs 1"
        echo "  --max-memory $(( (TOTAL_MEM / 1024) * 3 / 4 )) # 限制为总内存的75%"
    elif [ -n "$TOTAL_MEM" ] && [ "$TOTAL_MEM" -lt 4194304 ]; then
        # 如果内存小于4GB
        echo "系统内存较小，建议:"
        echo "  --jobs 1 或 --jobs 2 (取决于可用内存)"
        echo "  考虑使用 --low-memory 选项减少内存使用"
    else
        echo "提示: 设置为CPU核心数或核心数+1通常效果最佳"
        echo "      对于内存受限的系统，可以考虑减少作业数"
    fi
    echo ""
}

# 显示帮助信息
show_build_help() {
    echo "构建项目"
    echo ""
    echo "用法: $0 build [选项]"
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
    echo "                            默认: 根据系统情况自动设置 (当前默认: $JOBS)"
    echo "                            低内存系统建议设置为1"
    echo "  --cpu-info                显示CPU和内存信息并退出"
    echo "  --low-memory              启用低内存模式，减少内存使用"
    echo "                            (自动设置-j 1并启用其他内存优化)"
    echo "  --max-memory SIZE         设置编译器可使用的最大内存 (单位: MB)"
    echo "                            例如: --max-memory 1024 限制为1GB内存"
    echo ""
    echo "示例:"
    echo "  $0 build --type Release --prefix ~/webserver"
    echo "  $0 build --clean --verbose"
    echo "  $0 build --jobs 1 --low-memory  # 适用于低内存系统"
    echo "  $0 build --max-memory 1500      # 限制编译器内存使用为1.5GB"
    echo "  $0 build --cpu-info             # 显示系统信息"
    echo ""
}

# 验证构建类型
validate_build_type() {
    if [[ ! "$BUILD_TYPE" =~ ^(Debug|Release|RelWithDebInfo|MinSizeRel)$ ]]; then
        echo "错误: 无效的构建类型 '$BUILD_TYPE'"
        echo "有效的构建类型: Debug, Release, RelWithDebInfo, MinSizeRel"
        exit 1
    fi
}

# 设置CMake参数
set_cmake_args() {
    CMAKE_ARGS=(
        "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
        "-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX"
        "-DBUILD_BENCHMARKS=ON"  # 默认启用benchmark构建"
    )

    # 低内存模式优化
    if [ "$LOW_MEMORY_MODE" = true ]; then
        echo "应用低内存模式优化..."
        
        # 减少编译器内存使用的C++编译器标志
        if [ "$BUILD_TYPE" = "Debug" ]; then
            # 在Debug模式下，我们可以减少一些调试信息以节省内存
            CMAKE_ARGS+=("-DCMAKE_CXX_FLAGS=-Os -g1 --param ggc-min-expand=10 --param ggc-min-heapsize=32768")
            CMAKE_ARGS+=("-DCMAKE_C_FLAGS=-Os -g1 --param ggc-min-expand=10 --param ggc-min-heapsize=32768")
        else
            # 在其他构建模式下，保持优化级别不变，但仍然应用内存优化
            CMAKE_ARGS+=("-DCMAKE_CXX_FLAGS_EXTRA=--param ggc-min-expand=10 --param ggc-min-heapsize=32768")
            CMAKE_ARGS+=("-DCMAKE_C_FLAGS_EXTRA=--param ggc-min-expand=10 --param ggc-min-heapsize=32768")
        fi
        
        # 减少CMake自身的内存使用
        CMAKE_ARGS+=("-DCMAKE_EXPORT_COMPILE_COMMANDS=OFF")
    fi

    # 设置内存限制
    if [ -n "$MAX_MEMORY" ]; then
        # 将MB转换为KB (ulimit使用KB)
        MAX_MEMORY_KB=$((MAX_MEMORY * 1024))
        echo "设置编译器内存限制为 ${MAX_MEMORY}MB (${MAX_MEMORY_KB}KB)"
        
        # 在Linux上，我们可以使用ulimit来限制内存使用
        if command -v ulimit >/dev/null 2>&1; then
            # 保存当前限制以便稍后恢复
            ORIGINAL_MEMORY_LIMIT=$(ulimit -v)
            
            # 设置虚拟内存限制
            ulimit -v $MAX_MEMORY_KB || echo "警告: 无法设置内存限制"
        else
            echo "警告: 无法设置内存限制，ulimit命令不可用"
        fi
    fi
}

# 检查磁盘空间
check_disk_space() {
    # 获取构建目录的父目录
    BUILD_DIR_PARENT=$(dirname "$BUILD_DIR")
    if [ ! -d "$BUILD_DIR_PARENT" ]; then
        echo "无法检查磁盘空间，目录 $BUILD_DIR_PARENT 不存在"
        return 1
    fi
    
    # 获取可用磁盘空间(KB)
    DISK_SPACE_KB=$(df -k "$BUILD_DIR_PARENT" | tail -1 | awk '{print $4}')
    
    # 转换为MB
    DISK_SPACE_MB=$((DISK_SPACE_KB / 1024))
    
    echo "构建目录可用磁盘空间: $DISK_SPACE_MB MB"
    
    # 检查磁盘空间是否足够
    DISK_THRESHOLD=500  # MB
    if [ "$DISK_SPACE_MB" -lt "$DISK_THRESHOLD" ]; then
        echo "警告: 磁盘空间不足 ($DISK_SPACE_MB MB < $DISK_THRESHOLD MB)"
        echo "构建可能会失败或系统可能会变得不稳定"
    fi
}

# 清理构建目录
clean_build_dir() {
    if [ "$CLEAN" = true ] && [ -d "$BUILD_DIR" ]; then
        echo "清理构建目录: $BUILD_DIR"
        rm -rf "$BUILD_DIR"
    fi
}

# 构建项目主函数
build_project() {
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_build_help
                return 0
                ;;
            -t|--type)
                if [[ -z "$2" || "$2" == -* ]]; then
                    echo "错误: --type 选项需要一个参数"
                    exit 1
                fi
                BUILD_TYPE="$2"
                shift 2
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
            -c|--clean)
                CLEAN=true
                shift
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            -j|--jobs)
                if [[ -z "$2" || "$2" == -* ]]; then
                    echo "错误: --jobs 选项需要一个数字参数"
                    exit 1
                fi
                if ! [[ "$2" =~ ^[0-9]+$ ]]; then
                    echo "错误: --jobs 参数必须是一个正整数"
                    exit 1
                fi
                JOBS="$2"
                shift 2
                ;;
            --cpu-info)
                detect_memory
                detect_cpu_cores
                set_default_jobs
                show_cpu_info
                return 0
                ;;
            --low-memory)
                LOW_MEMORY_MODE=true
                shift
                ;;
            --max-memory)
                if [[ -z "$2" || "$2" == -* ]]; then
                    echo "错误: --max-memory 选项需要一个数字参数"
                    exit 1
                fi
                if ! [[ "$2" =~ ^[0-9]+$ ]]; then
                    echo "错误: --max-memory 参数必须是一个正整数"
                    exit 1
                fi
                MAX_MEMORY="$2"
                shift 2
                ;;
            *)
                echo "错误: 未知选项 $1"
                show_build_help
                exit 1
                ;;
        esac
    done

    # 检测系统资源
    detect_memory
    detect_cpu_cores
    set_default_jobs
    
    # 验证参数
    validate_build_type
    
    # 设置CMake参数
    set_cmake_args
    
    # 检查磁盘空间
    check_disk_space
    
    # 清理构建目录
    clean_build_dir
    
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
    cmake "${CMAKE_ARGS[@]}" "$PROJECT_DIR"
    
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
}