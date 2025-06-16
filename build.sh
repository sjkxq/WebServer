#!/bin/zsh

# 设置错误时退出
set -e

# 颜色定义
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# 默认值
BUILD_TYPE="Debug"
RUN_TESTS=true
CLEAN=false
VERBOSE=false
TEST_FILTER=""
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# 显示帮助信息
show_help() {
  echo "${BLUE}用法: $0 [选项]${NC}"
  echo
  echo "选项:"
  echo "  ${YELLOW}-h, --help${NC}        显示此帮助信息"
  echo "  ${YELLOW}-c, --clean${NC}       清理构建目录"
  echo "  ${YELLOW}-t, --no-tests${NC}    跳过运行测试"
  echo "  ${YELLOW}-r, --release${NC}     使用Release模式构建（默认为Debug）"
  echo "  ${YELLOW}-v, --verbose${NC}     显示详细的构建输出"
  echo "  ${YELLOW}-j, --jobs N${NC}      设置并行构建任务数（默认: 系统CPU核心数）"
  echo "  ${YELLOW}-f, --test-filter${NC} 指定要运行的测试（例如：HttpStatusTest.*）"
  echo
}

# 解析命令行参数
while [[ $# -gt 0 ]]; do
  case $1 in
    -h|--help)
      show_help
      exit 0
      ;;
    -c|--clean)
      CLEAN=true
      shift
      ;;
    -t|--no-tests)
      RUN_TESTS=false
      shift
      ;;
    -r|--release)
      BUILD_TYPE="Release"
      shift
      ;;
    -v|--verbose)
      VERBOSE=true
      shift
      ;;
    -j|--jobs)
      JOBS="$2"
      shift 2
      ;;
    -f|--test-filter)
      TEST_FILTER="$2"
      shift 2
      ;;
    *)
      echo "${RED}错误: 未知选项 $1${NC}" >&2
      show_help
      exit 1
      ;;
  esac
done

# 如果需要清理，则删除build目录
if $CLEAN; then
  echo "${YELLOW}清理构建目录...${NC}"
  rm -rf build
fi

# 创建build目录（如果不存在）
mkdir -p build

# 进入build目录
cd build

echo "${BLUE}配置项目 (${BUILD_TYPE} 模式)...${NC}"
# 运行CMake配置
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..

echo "${BLUE}编译项目...${NC}"
# 编译项目
if $VERBOSE; then
  cmake --build . --config ${BUILD_TYPE} -j ${JOBS} -- VERBOSE=1
else
  cmake --build . --config ${BUILD_TYPE} -j ${JOBS}
fi

# 运行测试
if $RUN_TESTS; then
  echo "${BLUE}运行测试...${NC}"
  if [[ -n "$TEST_FILTER" ]]; then
    echo "${YELLOW}使用过滤器: $TEST_FILTER${NC}"
    ./webserver_tests --gtest_filter="$TEST_FILTER"
  else
    ctest --output-on-failure
  fi
else
  echo "${YELLOW}跳过测试${NC}"
fi

# 返回到原始目录
cd ..

echo "${GREEN}✅ 构建完成 (${BUILD_TYPE} 模式)${NC}"