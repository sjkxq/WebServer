# 编译器和构建选项配置

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wconversion
        -Wshadow
        -Wdouble-promotion
        -Wsign-conversion
        -Wnull-dereference
        -Wold-style-cast
        -Wcast-align
        -Wno-unused-parameter
        -Wno-missing-field-initializers
    )
    
    # Debug模式下的额外选项
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    endif()
    
    # Release模式下的优化选项
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
    endif()
endif()

# MSVC特定选项
if(MSVC)
    add_compile_options(
        /W4
        /WX
        /w44265  # 有符号/无符号不匹配
        /w44289  # 常量条件表达式
        /w44296  # 运算符'&&'的求值顺序
        /w45263  # 成员函数不匹配
        /w14545  # 表达式在算术转换前求值
        /w14546  # 表达式在算术转换后求值
        /w14547  # 声明隐藏了全局声明
        /w14549  # 声明隐藏了类成员
        /w14555  # 表达式没有副作用
        /w14619  # 枚举值转换
        /wd4100  # 未引用的形参
        /wd4505  # 未引用的函数
    )
    
    # 启用多处理器编译
    add_compile_options(/MP)
    
    # 使用UTF-8编码
    add_compile_options(/utf-8)
endif()

# 启用IPO/LTO（如果可用）
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED)
if(IPO_SUPPORTED AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

# 设置可用的构建类型
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
)

# 添加自定义编译定义
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()

# 设置运行时库
if(MSVC)
    # 在Debug模式下使用MTd，在其他模式下使用MT
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    endif()
endif()