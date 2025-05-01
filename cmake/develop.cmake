set(CMAKE_CXX_STANDARD 20) # 设置C++标准为C++20
set(CMAKE_C_STANDARD 11)   # 设置C语言标准为C11
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 指定C++标准是必需的
set(CMAKE_CXX_EXTENSIONS OFF) # 禁用编译器的扩展

# 警告
option(ENABLE_WARNING "Enable warning for all project" ON)
if(ENABLE_WARNING) # 如果用户在配置时启用了警告选项
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # 判断是否为 MSVC 编译器
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
        message("启动 c++20")
        # 为 MSVC 编译器添加警告选项
        list(APPEND MSVC_OPTIONS "/W3") # 启用中等级别警告 (MSVC 默认支持 /W1 ~ /W4, /W3 是常用的平衡选项)
        if(MSVC_VERSION GREATER 1900) # 如果是 MSVC 2015 或更高版本
            list(APPEND MSVC_OPTIONS "/WX") # 将所有警告视为错误
        endif()
        # 使用utf-8编译
        add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
        add_compile_options(${MSVC_OPTIONS}) # 添加这些选项到编译命令中
    else()
        # 为 GCC/Clang 编译器添加警告选项
        add_compile_options(
            -Wall                     # 启用所有基础警告 (最常见的潜在问题)
            -Wextra                   # 启用额外的警告 (如未使用的变量、不推荐的语法等)
            -Wconversion              # 启用类型转换相关的警告 (可能导致隐式数据丢失)
            -pedantic                 # 强制遵守标准, 非标准扩展将会警告
            -Werror                   # 将所有警告视为错误 (确保代码在警告级别下完全无误)
            -Wfatal-errors            # 在第一个错误后立即停止编译 (减少无意义的编译时间)

            # 显式启用警告
            -Werror=return-type       # 将函数返回类型的警告视为错误, 确保所有函数都有返回值
            -Wnon-virtual-dtor        # 当类有虚函数但没有虚析构函数时发出警告
            -Wdelete-non-virtual-dtor # 当试图删除有虚函数但没有虚析构函数的对象时发出警告
        )
    endif()
endif()
message(STATUS "--------------------------------------------")