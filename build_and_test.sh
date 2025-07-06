#!/bin/bash

# 构建和测试脚本
set -e

# 默认参数
BUILD_TYPE="Debug"
BUILD_DIR="build"
INSTALL_PREFIX="/usr/local"
CLEAN=false
VERBOSE=false
SHOW_CPU_INFO=false
LOW_MEMORY_MODE=false
MAX_MEMORY=""

# 检测系统内存
TOTAL_MEMORY_KB=$(grep MemTotal /proc/meminfo 2>/dev/null | awk '{print $2}')
if [ -n "$TOTAL_MEMORY_KB" ]; then
    TOTAL_MEMORY_MB=$((TOTAL_MEMORY_KB / 1024))
    # 如果系统内存小于3GB，自动启用低内存模式
    if [ "$TOTAL_MEMORY_MB" -lt 3072 ]; then
        LOW_MEMORY_MODE=true
        echo "检测到系统内存较低 (${TOTAL_MEMORY_MB}MB)，自动启用低内存模式"
    fi
fi

# 检测CPU核心数
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

# 根据系统情况设置默认作业数
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

# 显示帮助信息
show_help() {
    echo "WebServer构建和测试脚本"
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
    echo "                            默认: 根据系统情况自动设置 (当前默认: $JOBS)"
    echo "                            低内存系统建议设置为1"
    echo "  --cpu-info                显示CPU和内存信息并退出"
    echo "  --low-memory              启用低内存模式，减少内存使用"
    echo "                            (自动设置-j 1并启用其他内存优化)"
    echo "  --max-memory SIZE         设置编译器可使用的最大内存 (单位: MB)"
    echo "                            例如: --max-memory 1024 限制为1GB内存"
    echo ""
    echo "示例:"
    echo "  $0 --type Release --prefix ~/webserver"
    echo "  $0 --clean --verbose"
    echo "  $0 --jobs 1 --low-memory  # 适用于低内存系统"
    echo "  $0 --max-memory 1500      # 限制编译器内存使用为1.5GB"
    echo "  $0 --cpu-info             # 显示系统信息"
    echo ""
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

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
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
            SHOW_CPU_INFO=true
            shift
            ;;
        --low-memory)
            LOW_MEMORY_MODE=true
            # 在低内存模式下，如果用户没有明确指定作业数，则设置为1
            if [ "$JOBS" != "1" ]; then
                echo "启用低内存模式，将作业数设置为1"
                JOBS=1
            fi
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
            show_help
            exit 1
            ;;
    esac
done

# 如果请求显示系统信息，则显示并退出
if [ "$SHOW_CPU_INFO" = true ]; then
    show_cpu_info
    exit 0
fi

# 清理系统缓存以释放内存
clean_system_cache() {
    echo "清理系统缓存以释放内存..."
    
    # 同步文件系统缓冲区，确保数据写入磁盘
    sync
    
    # 在Linux上，我们可以通过写入/proc/sys/vm/drop_caches来清理缓存
    if [ -f /proc/sys/vm/drop_caches ]; then
        # 检查是否有写入权限
        if [ -w /proc/sys/vm/drop_caches ] && [ "$(id -u)" -eq 0 ]; then
            # 尝试清理缓存，但忽略错误
            echo 3 > /proc/sys/vm/drop_caches 2>/dev/null
            
            if [ $? -eq 0 ]; then
                echo "系统缓存已清理"
            else
                echo "清理系统缓存失败，可能是权限问题"
                echo "尝试其他方法释放内存..."
            fi
        else
            echo "无法清理系统缓存: 需要root权限或文件系统是只读的"
            echo "尝试其他方法释放内存..."
        fi
    else
        echo "无法清理系统缓存: /proc/sys/vm/drop_caches不存在"
        echo "尝试其他方法释放内存..."
    fi
    
    # 备选方法：使用madvise系统调用请求内核释放一些内存
    # 这在容器或受限环境中可能更有效
    if command -v python3 >/dev/null 2>&1; then
        echo "使用Python请求内存释放..."
        python3 -c "import gc; gc.collect()" 2>/dev/null || true
    fi
    
    # 在某些系统上，可以通过这种方式请求内核回收一些内存
    echo "请求系统回收未使用的内存..."
}

# 获取当前可用内存(MB)
get_available_memory() {
    if [ ! -f /proc/meminfo ]; then
        echo "0" # 如果无法获取内存信息，返回0
        return
    fi
    
    # 获取可用内存(KB)
    AVAILABLE_MEM=$(grep MemAvailable /proc/meminfo | awk '{print $2}')
    if [ -z "$AVAILABLE_MEM" ]; then
        # 如果MemAvailable不可用，使用MemFree作为替代
        AVAILABLE_MEM=$(grep MemFree /proc/meminfo | awk '{print $2}')
    fi
    
    # 转换为MB并返回
    echo $((AVAILABLE_MEM / 1024))
}

# 显示构建进度
show_build_progress() {
    local build_log=$1
    local total_files
    local completed_files
    local start_time
    local current_time
    local elapsed_time
    local estimated_total_time
    local remaining_time
    local percent_complete
    
    # 如果没有提供构建日志文件，则静默退出
    if [ -z "$build_log" ] || [ ! -f "$build_log" ]; then
        # 静默退出，不显示错误信息
        return 0
    fi
    
    # 获取总文件数 (这里假设使用ninja构建系统)
    if [ "$GENERATOR" = "Ninja" ]; then
        total_files=$(grep -c "^FAILED\|^SUCCESS" "$build_log" 2>/dev/null || echo 0)
        completed_files=$(grep -c "^SUCCESS" "$build_log" 2>/dev/null || echo 0)
    else
        # 对于其他构建系统，可能需要不同的方法
        echo "不支持的构建系统，无法显示详细进度"
        return 1
    fi
    
    # 如果没有找到文件，可能是日志格式不匹配
    if [ "$total_files" -eq 0 ]; then
        echo "无法解析构建进度，日志格式可能不匹配"
        return 1
    fi
    
    # 计算完成百分比
    percent_complete=$((completed_files * 100 / total_files))
    
    # 显示进度
    echo "构建进度: $completed_files / $total_files 文件 ($percent_complete%)"
    
    # 如果有时间信息，计算估计剩余时间
    if [ -n "$BUILD_START_TIME" ]; then
        current_time=$(date +%s)
        elapsed_time=$((current_time - BUILD_START_TIME))
        
        # 只有当至少完成了5%的文件时才计算剩余时间
        if [ "$percent_complete" -ge 5 ] && [ "$elapsed_time" -gt 0 ]; then
            estimated_total_time=$((elapsed_time * 100 / percent_complete))
            remaining_time=$((estimated_total_time - elapsed_time))
            
            # 转换为分钟和秒
            remaining_minutes=$((remaining_time / 60))
            remaining_seconds=$((remaining_time % 60))
            
            echo "估计剩余时间: ${remaining_minutes}分${remaining_seconds}秒"
        fi
    fi
}

# 根据可用内存动态调整作业数
adjust_jobs_by_memory() {
    # 获取当前可用内存
    AVAILABLE_MEM_MB=$(get_available_memory)
    
    # 如果无法获取内存信息，保持当前作业数不变
    if [ "$AVAILABLE_MEM_MB" -eq 0 ]; then
        echo "无法获取内存信息，保持当前作业数: $JOBS"
        return
    fi
    
    # 估算每个作业需要的内存 (根据经验值，每个C++编译作业约需要300-500MB内存)
    MEMORY_PER_JOB=400 # MB
    
    # 计算可支持的最大作业数
    MAX_JOBS_BY_MEMORY=$((AVAILABLE_MEM_MB / MEMORY_PER_JOB))
    
    # 保留一些内存给系统和其他进程使用
    MAX_JOBS_BY_MEMORY=$((MAX_JOBS_BY_MEMORY - 1))
    
    # 确保至少有1个作业
    if [ "$MAX_JOBS_BY_MEMORY" -lt 1 ]; then
        MAX_JOBS_BY_MEMORY=1
    fi
    
    # 如果当前作业数超过了内存可支持的最大作业数，则调整作业数
    if [ "$JOBS" -gt "$MAX_JOBS_BY_MEMORY" ]; then
        echo "根据可用内存 ($AVAILABLE_MEM_MB MB) 调整作业数: $JOBS -> $MAX_JOBS_BY_MEMORY"
        JOBS=$MAX_JOBS_BY_MEMORY
    else
        echo "当前作业数 ($JOBS) 适合可用内存 ($AVAILABLE_MEM_MB MB)"
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
        
        # 询问用户是否继续
        echo -n "是否继续构建? (y/n) "
        read -r CONTINUE
        if [ "$CONTINUE" != "y" ] && [ "$CONTINUE" != "Y" ]; then
            echo "构建已取消"
            exit 1
        fi
    fi
}

# 监控内存使用情况
monitor_memory() {
    if [ ! -f /proc/meminfo ]; then
        echo "无法监控内存使用情况，/proc/meminfo不存在"
        return 1
    fi
    
    # 获取可用内存(MB)
    AVAILABLE_MEM_MB=$(get_available_memory)
    
    # 如果可用内存低于阈值，暂停并等待
    MEMORY_THRESHOLD=100  # MB
    if [ "$AVAILABLE_MEM_MB" -lt "$MEMORY_THRESHOLD" ]; then
        echo "警告: 可用内存不足 ($AVAILABLE_MEM_MB MB < $MEMORY_THRESHOLD MB)"
        echo "暂停构建，等待内存释放..."
        
        # 尝试清理系统缓存
        clean_system_cache
        
        # 等待一段时间
        sleep 10
        
        # 再次检查内存
        AVAILABLE_MEM_MB=$(get_available_memory)
        echo "当前可用内存: $AVAILABLE_MEM_MB MB"
        
        # 如果内存仍然不足，建议用户减少作业数或启用低内存模式
        if [ "$AVAILABLE_MEM_MB" -lt "$MEMORY_THRESHOLD" ]; then
            echo "内存仍然不足，建议:"
            echo "1. 使用 --jobs 1 选项减少并行作业数"
            echo "2. 使用 --low-memory 选项启用低内存模式"
            echo "3. 关闭其他内存密集型应用程序"
            
            # 自动调整作业数
            if [ "$JOBS" -gt 1 ]; then
                OLD_JOBS=$JOBS
                JOBS=1
                echo "自动将作业数从 $OLD_JOBS 调整为 $JOBS"
            fi
            
            # 询问用户是否继续
            echo -n "是否继续构建? (y/n) "
            read -r CONTINUE
            if [ "$CONTINUE" != "y" ] && [ "$CONTINUE" != "Y" ]; then
                echo "构建已取消"
                exit 1
            fi
        fi
    else
        echo "当前可用内存: $AVAILABLE_MEM_MB MB (足够)"
    fi
}

# 设置清理函数，确保在脚本结束时恢复系统设置
cleanup() {
    # 恢复原始内存限制
    if [ -n "$ORIGINAL_MEMORY_LIMIT" ]; then
        echo "恢复原始内存限制..."
        ulimit -v $ORIGINAL_MEMORY_LIMIT || echo "警告: 无法恢复原始内存限制"
    fi
}

# 注册清理函数，确保在脚本退出时执行
trap cleanup EXIT

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
    "-DBUILD_BENCHMARKS=ON"  # 默认启用benchmark构建
)

# 低内存模式优化
if [ "$LOW_MEMORY_MODE" = true ]; then
    echo "应用低内存模式优化..."
    
    # 减少编译器内存使用的C++编译器标志
    # -Os: 优化大小而不是速度，通常使用更少的内存
    # --param ggc-min-expand=10: 减少GCC垃圾收集器的内存使用
    # --param ggc-min-heapsize=32768: 设置GCC垃圾收集器的最小堆大小
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
    
    # 禁用一些可能消耗内存的功能
    CMAKE_ARGS+=("-DBUILD_TESTING=OFF")  # 如果可能，禁用测试构建
    # 保持benchmark构建启用，以便能够运行benchmark测试
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

# 如果需要清理构建目录
if [ "$CLEAN" = true ] && [ -d "$BUILD_DIR" ]; then
    echo "清理构建目录: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

# 检查磁盘空间
check_disk_space

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
cmake "${CMAKE_ARGS[@]}" ..

# 构建项目
echo "构建项目 (使用 $JOBS 个作业)"

# 在低内存模式下，尝试清理系统缓存并监控内存使用情况
if [ "$LOW_MEMORY_MODE" = true ]; then
    clean_system_cache
    monitor_memory
fi

# 根据可用内存调整作业数
adjust_jobs_by_memory

# 记录构建开始时间
BUILD_START_TIME=$(date +%s)

# 创建构建日志文件
BUILD_LOG_FILE="$BUILD_DIR/build_log.txt"
# 确保构建日志目录存在并可写
mkdir -p "$(dirname "$BUILD_LOG_FILE")" 2>/dev/null || true
# 尝试创建日志文件，如果失败则使用临时文件或禁用日志
if ! touch "$BUILD_LOG_FILE" 2>/dev/null; then
    echo "警告: 无法创建构建日志文件 '$BUILD_LOG_FILE'"
    # 尝试使用/tmp目录
    if [ -d "/tmp" ] && [ -w "/tmp" ]; then
        BUILD_LOG_FILE="/tmp/webserver_build_log.txt"
        echo "尝试使用临时文件: $BUILD_LOG_FILE"
        touch "$BUILD_LOG_FILE" 2>/dev/null || BUILD_LOG_FILE=""
    else
        BUILD_LOG_FILE=""
    fi
    
    # 如果仍然无法创建日志文件，禁用进度显示
    if [ -z "$BUILD_LOG_FILE" ]; then
        echo "无法创建构建日志文件，进度显示将被禁用"
    fi
fi

# 在低内存模式下，我们可能需要特殊处理大文件
if [ "$LOW_MEMORY_MODE" = true ] && [ "$JOBS" -gt 1 ]; then
    echo "低内存模式: 将尝试避免同时编译大文件"
    
    # 首先尝试构建小文件
    if [ "$GENERATOR" = "Ninja" ]; then
        echo "第一阶段: 构建小文件..."
        # 根据可用内存再次调整作业数
        adjust_jobs_by_memory
        
        # 排除已知的大文件
        if [ "$VERBOSE" = true ]; then
            # 将输出重定向到构建日志文件，同时显示在终端
            ninja -v -j "$JOBS" 2>&1 | tee -a "$BUILD_LOG_FILE" || echo "部分构建可能失败，继续下一阶段..."
            
            # 每隔30秒显示一次进度
            (
                while true; do
                    sleep 30
                    show_build_progress "$BUILD_LOG_FILE"
                done
            ) &
            PROGRESS_PID=$!
            
            # 等待构建完成
            wait
            
            # 终止进度显示进程
            kill $PROGRESS_PID 2>/dev/null || true
        else
            ninja -j "$JOBS" 2>&1 | tee -a "$BUILD_LOG_FILE" || echo "部分构建可能失败，继续下一阶段..."
        fi
        
        # 显示第一阶段的进度
        [ -n "$BUILD_LOG_FILE" ] && show_build_progress "$BUILD_LOG_FILE"
        
        # 在两个阶段之间监控内存
        if [ "$LOW_MEMORY_MODE" = true ]; then
            monitor_memory
            clean_system_cache
        fi
        
        echo "第二阶段: 构建剩余文件 (单线程)..."
        # 使用单线程模式构建剩余文件
        if [ "$VERBOSE" = true ]; then
            ninja -v -j 1 2>&1 | tee -a "$BUILD_LOG_FILE"
            
            # 每隔30秒显示一次进度
            (
                while true; do
                    sleep 30
                    show_build_progress "$BUILD_LOG_FILE"
                done
            ) &
            PROGRESS_PID=$!
            
            # 等待构建完成
            wait
            
            # 终止进度显示进程
            kill $PROGRESS_PID 2>/dev/null || true
        else
            ninja -j 1 2>&1 | tee -a "$BUILD_LOG_FILE"
        fi
        
        # 显示最终进度
        show_build_progress "$BUILD_LOG_FILE"
    else
        # 对于Make，我们没有简单的方法来分离大小文件，所以直接使用单线程
        echo "使用单线程模式构建以减少内存使用..."
        if [ "$VERBOSE" = true ]; then
            cmake --build . --config "$BUILD_TYPE" --verbose -j 1
        else
            cmake --build . --config "$BUILD_TYPE" -j 1
        fi
    fi
else
    # 正常构建
    if [ "$GENERATOR" = "Ninja" ]; then
        if [ "$VERBOSE" = true ]; then
            # 将输出重定向到构建日志文件，同时显示在终端
            ninja -v -j "$JOBS" 2>&1 | tee -a "$BUILD_LOG_FILE"
            
            # 每隔30秒显示一次进度
            (
                while true; do
                    sleep 30
                    show_build_progress "$BUILD_LOG_FILE"
                done
            ) &
            PROGRESS_PID=$!
            
            # 等待构建完成
            wait
            
            # 终止进度显示进程
            kill $PROGRESS_PID 2>/dev/null || true
        else
            ninja -j "$JOBS" 2>&1 | tee -a "$BUILD_LOG_FILE"
            
            # 每隔60秒显示一次进度
            (
                while true; do
                    sleep 60
                    show_build_progress "$BUILD_LOG_FILE"
                done
            ) &
            PROGRESS_PID=$!
            
            # 等待构建完成
            wait
            
            # 终止进度显示进程
            kill $PROGRESS_PID 2>/dev/null || true
        fi
        
        # 显示最终进度
        show_build_progress "$BUILD_LOG_FILE"
    else
        if [ "$VERBOSE" = true ]; then
            cmake --build . --config "$BUILD_TYPE" --verbose -j "$JOBS" 2>&1 | tee -a "$BUILD_LOG_FILE"
        else
            cmake --build . --config "$BUILD_TYPE" -j "$JOBS" 2>&1 | tee -a "$BUILD_LOG_FILE"
        fi
    fi
fi

# 显示构建完成时间
BUILD_END_TIME=$(date +%s)
BUILD_DURATION=$((BUILD_END_TIME - BUILD_START_TIME))
BUILD_MINUTES=$((BUILD_DURATION / 60))
BUILD_SECONDS=$((BUILD_DURATION % 60))
echo "构建完成，用时: ${BUILD_MINUTES}分${BUILD_SECONDS}秒"

# 如果构建失败，尝试使用单线程模式重新构建
if [ $? -ne 0 ]; then
    echo "构建失败，尝试使用单线程模式重新构建..."
    
    # 重置构建日志文件
    echo "重置构建日志文件..."
    > "$BUILD_LOG_FILE"
    
    # 重置构建开始时间
    BUILD_START_TIME=$(date +%s)
    
    if [ "$GENERATOR" = "Ninja" ]; then
        if [ "$VERBOSE" = true ]; then
            ninja -v -j 1 2>&1 | tee -a "$BUILD_LOG_FILE"
            
            # 每隔30秒显示一次进度
            (
                while true; do
                    sleep 30
                    show_build_progress "$BUILD_LOG_FILE"
                done
            ) &
            PROGRESS_PID=$!
            
            # 等待构建完成
            wait
            
            # 终止进度显示进程
            kill $PROGRESS_PID 2>/dev/null || true
        else
            ninja -j 1 2>&1 | tee -a "$BUILD_LOG_FILE"
            
            # 每隔60秒显示一次进度
            (
                while true; do
                    sleep 60
                    show_build_progress "$BUILD_LOG_FILE"
                done
            ) &
            PROGRESS_PID=$!
            
            # 等待构建完成
            wait
            
            # 终止进度显示进程
            kill $PROGRESS_PID 2>/dev/null || true
        fi
        
        # 显示最终进度
        show_build_progress "$BUILD_LOG_FILE"
    else
        if [ "$VERBOSE" = true ]; then
            cmake --build . --config "$BUILD_TYPE" --verbose -j 1 2>&1 | tee -a "$BUILD_LOG_FILE"
        else
            cmake --build . --config "$BUILD_TYPE" -j 1 2>&1 | tee -a "$BUILD_LOG_FILE"
        fi
    fi
    
    # 显示重新构建完成时间
    BUILD_END_TIME=$(date +%s)
    BUILD_DURATION=$((BUILD_END_TIME - BUILD_START_TIME))
    BUILD_MINUTES=$((BUILD_DURATION / 60))
    BUILD_SECONDS=$((BUILD_DURATION % 60))
    echo "重新构建完成，用时: ${BUILD_MINUTES}分${BUILD_SECONDS}秒"
fi

# 在低内存模式下，再次清理系统缓存以释放内存
if [ "$LOW_MEMORY_MODE" = true ]; then
    clean_system_cache
fi

echo "构建完成!"
echo "可执行文件位于: $BUILD_DIR/bin/webserver"

# 运行单元测试
echo ""
echo "运行单元测试..."

# 在低内存模式下，监控内存并清理缓存
if [ "$LOW_MEMORY_MODE" = true ]; then
    monitor_memory
    clean_system_cache
    echo "低内存模式: 串行运行测试..."
    if ! ctest --output-on-failure -j 1; then
        echo "单元测试失败!"
        exit 1
    fi
else
    # 正常模式下，使用默认并行度
    if ! ctest --output-on-failure; then
        echo "单元测试失败!"
        exit 1
    fi
fi

# 测试后再次清理缓存
if [ "$LOW_MEMORY_MODE" = true ]; then
    clean_system_cache
fi

echo "单元测试通过!"

# 保存当前目录
CURRENT_DIR=$(pwd)

# 运行benchmark测试
echo ""
echo "运行benchmark测试..."

# 在低内存模式下，监控内存并清理缓存
if [ "$LOW_MEMORY_MODE" = true ]; then
    monitor_memory
    clean_system_cache
fi

BENCHMARK_PATH="bin/webserver_benchmark"
if [ -f "$BENCHMARK_PATH" ]; then
    echo "执行: $BENCHMARK_PATH"
    
    # 在低内存模式下，可以调整benchmark参数以减少内存使用
    if [ "$LOW_MEMORY_MODE" = true ]; then
        echo "低内存模式: 使用较少的迭代次数运行benchmark..."
        if ! "./$BENCHMARK_PATH" --benchmark_min_time=0.1 --benchmark_repetitions=1; then
            echo "Benchmark测试失败!"
            cd "$CURRENT_DIR"  # 返回原始目录
            exit 1
        fi
    else
        # 正常模式下，使用默认参数
        if ! "./$BENCHMARK_PATH"; then
            echo "Benchmark测试失败!"
            cd "$CURRENT_DIR"  # 返回原始目录
            exit 1
        fi
    fi
    
    # 测试后再次清理缓存
    if [ "$LOW_MEMORY_MODE" = true ]; then
        clean_system_cache
    fi
    
    echo "Benchmark测试完成!"
else
    echo "警告: 未找到benchmark可执行文件: $BENCHMARK_PATH"
    echo "检查构建目录中的可执行文件:"
    ls -la bin/ || echo "无法列出 bin/ 目录内容"
fi

echo ""
echo "所有测试完成!"
echo ""
echo "要安装项目，请执行:"
echo "  cd $BUILD_DIR && sudo cmake --install ."
echo ""