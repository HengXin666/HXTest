cmake_minimum_required(VERSION 3.20.0)
# 定义工程名称, 并指定语言为C++
project(HXTest LANGUAGES CXX)

# 是否启用 Address Sanitizer
set(HX_DEBUG_BY_ADDRESS_SANITIZER ON)

# 使用 Address Sanitizer
if(HX_DEBUG_BY_ADDRESS_SANITIZER AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
    message("=-=-=-=-=-=-= 启用 Address Sanitizer [-fsanitize=address] =-=-=-=-=-=-=")
endif()

# 改用 clang 编译器
# set(CMAKE_C_COMPILER clang)
# set(CMAKE_CXX_COMPILER clang++)

# 如果没有定义CMAKE_BUILD_TYPE，则设置为Release
# 可以使用 cmake -DCMAKE_BUILD_TYPE=Debug .. 来指定
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message("=-=-=-=-=-=-= 当前是 [Release] 模式 =-=-=-=-=-=-=")
else()
    message("=-=-=-=-=-=-= 当前是 [DEBUG] 模式 =-=-=-=-=-=-=")
    add_definitions(-D_HX_DEBUG_) # 添加一个 _HX_DEBUG_ 宏
    message("已添加宏: _HX_DEBUG_")
endif()

set(CMAKE_CXX_STANDARD 20) # 设置C++标准为C++20
set(CMAKE_C_STANDARD 11)   # 设置C语言标准为C11
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 指定C++标准是必需的
set(CMAKE_CXX_EXTENSIONS OFF) # 禁用编译器的扩展
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines") # 显式启动C++协程

# 设置CMake模块路径, 包含当前目录下的cmake文件夹以及之前的路径
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}")

add_compile_options(
    -Wall                     # 启用所有警告
    -Wextra                   # 启用额外的警告
    -Werror=return-type       # 将函数返回类型的警告视为错误, 确保所有函数都有返回值
    -Wnon-virtual-dtor        # 当类有虚函数但没有虚析构函数时发出警告
    -Wdelete-non-virtual-dtor # 当试图删除有虚函数但没有虚析构函数的对象时发出警告
)

# 定义生成的库的存放目录
set(LIB_PATH ${PROJECT_SOURCE_DIR}/lib)

set(ROOT_PATH "${PROJECT_SOURCE_DIR}") # 设置根项目的根目录
# 在camke .. 的时候会输出提示目录路径 (可选)
message(STATUS "Prefix dir is ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Binary dir is ${PROJECT_BINARY_DIR}")
message(STATUS "Source dir is ${PROJECT_SOURCE_DIR}")
message(STATUS "ROOT_PATH dir is ${ROOT_PATH}")

# 设置编译出来的可执行程序的名称
set(appName HXTest)

#####        ##############
# 添加子项目, 然后选择性编译#
########       ###########
# === begin === 添加子目录 === begin === #
option(BUILD_01_REFLECTION "Build 01-reflection" ON)

if(BUILD_01_REFLECTION)
    add_subdirectory(src/01-reflection)
    message("=-=-=-=-=-=-= Build 01-reflection =-=-=-=-=-=-=")
endif()

option(BUILD_01_HANSYA_LIVE "Build 01-hansya-live" ON)

if(BUILD_01_HANSYA_LIVE)
    add_subdirectory(src/01-hansya-live)
    message("=-=-=-=-=-=-= Build 01-hansya-live =-=-=-=-=-=-=")
endif()

# === end === 添加子目录 === end === #

# 设置头文件路径
# include_directories(${ROOT_PATH}/include)

# 将include目录添加到HXTest库的公共包含目录中
# target_include_directories(HXTest PUBLIC include)