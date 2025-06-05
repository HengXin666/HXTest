cmake_minimum_required(VERSION 3.21.0)
# 定义工程名称, 并指定语言为C++
project(HXTest LANGUAGES CXX)

if(NOT WIN32)
    # 是否启用 Address Sanitizer
    # set(HX_DEBUG_BY_ADDRESS_SANITIZER ON)

    # 使用 Address Sanitizer
    # if(HX_DEBUG_BY_ADDRESS_SANITIZER AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    #     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    #     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
    #     message("=-=-=-=-=-=-= 启用 Address Sanitizer [-fsanitize=address] =-=-=-=-=-=-=")
    # endif()

    # 改用 clang 编译器
    # set(CMAKE_C_COMPILER clang)
    # set(CMAKE_CXX_COMPILER clang++)
endif()

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

# 设置CMake模块路径, 包含当前目录下的cmake文件夹以及之前的路径
# set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}")

# 定义生成的库的存放目录
# set(LIB_PATH ${PROJECT_SOURCE_DIR}/lib)

set(ROOT_PATH "${PROJECT_SOURCE_DIR}") # 设置根项目的根目录
# 在camke .. 的时候会输出提示目录路径 (可选)
message(STATUS "Prefix dir is ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Binary dir is ${PROJECT_BINARY_DIR}")
message(STATUS "Source dir is ${PROJECT_SOURCE_DIR}")
message(STATUS "ROOT_PATH dir is ${ROOT_PATH}")

include_directories(include)

if(CMAKE_PROJECT_NAME STREQUAL "HXTest")
    include(cmake/develop.cmake)
    include(cmake/subDir.cmake)
endif()

# 将include目录添加到HXTest库的公共包含目录中
# target_include_directories(HXTest PUBLIC include)