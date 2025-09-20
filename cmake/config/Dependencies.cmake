# 依赖管理

# 设置选项
option(BUILD_TESTS "构建测试" ON)
option(BUILD_BENCHMARKS "构建基准测试" ON)
option(BUILD_COVERAGE "启用代码覆盖率" OFF)

# 设置CMP0069策略以解决GoogleTest的警告
# 这个策略控制是否强制执行INTERPROCEDURAL_OPTIMIZATION属性
if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()

# 引入外部依赖
include(FetchContent)

# 添加Google Test
if(BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.11.0
    )
    # 避免覆盖父项目的编译器/链接器设置
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    # 为Google Test目标添加编译选项，以解决空指针解引用警告问题
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(gtest PRIVATE 
            -Wno-error=null-dereference
            -Wno-null-dereference
        )
        target_compile_options(gtest_main PRIVATE 
            -Wno-error=null-dereference
            -Wno-null-dereference
        )
        target_compile_options(gmock PRIVATE 
            -Wno-error=null-dereference
            -Wno-null-dereference
        )
        target_compile_options(gmock_main PRIVATE 
            -Wno-error=null-dereference
            -Wno-null-dereference
        )
    endif()
    
    # 禁用Google Test目标的IPO以避免CMP0069警告
    if(POLICY CMP0069)
        set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION FALSE)
    endif()
endif()

# 添加Google Benchmark
if(BUILD_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.7.1
    )
    set(BENCHMARK_ENABLE_TESTING OFF)
    FetchContent_MakeAvailable(benchmark)

    # 为benchmark目标禁用特定严格警告
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
      target_compile_options(benchmark PRIVATE 
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
      )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      target_compile_options(benchmark PRIVATE 
        /wd4244  # 转换警告
        /wd4267  # size_t到更小类型的转换
        /wd4800  # 强制转换为bool
      )
    endif()
endif()

# 添加JSON库
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# 代码覆盖率配置
if((BUILD_COVERAGE OR COVERAGE) AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # 检查是否支持-fprofile-update=atomic选项
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-fprofile-update=atomic" HAS_PROFILE_UPDATE_ATOMIC)
    
    if(HAS_PROFILE_UPDATE_ATOMIC)
        # 支持atomic profile update
        add_compile_options(--coverage -fprofile-update=atomic -O0)
    else()
        add_compile_options(--coverage -O0)
    endif()
    
    add_link_options(--coverage)
endif()

# 查找线程库
find_package(Threads REQUIRED)

# 查找zlib库
find_package(ZLIB REQUIRED)

# 查找OpenSSL库
find_package(OpenSSL REQUIRED)