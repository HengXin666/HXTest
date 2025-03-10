#!/bin/bash
echo "=== 生成独立CMake子项目[快捷指引] (By Heng_Xin) ==="
read -p "请输入项目名称(如: 09-loli): " NAME

# 将项目名称转换为大写并将 '-' 替换为 '_'
BUILD_NAME=$(echo "$NAME" | tr '[:lower:]-' '[:upper:]_')

mkdir -p "./src/$NAME/demo/00-example"

echo "project($NAME LANGUAGES CXX)

file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS 
    ./demo/*.cpp
)

# 遍历每个 .cpp 文件, 生成可执行文件
foreach(TEST_FILE \${TEST_FILES})
    # 提取 .cpp 文件名作为目标名 (去掉路径和扩展名)
    get_filename_component(TEST_NAME \${TEST_FILE} NAME_WE)

    # 获取 .cpp 文件所在的目录 (相对路径)
    get_filename_component(TEST_DIR \${TEST_FILE} DIRECTORY)
    
    # 获取 TEST_DIR 的最后一级目录名 (即父文件夹名)
    get_filename_component(PARENT_DIR \${TEST_DIR} NAME)
    
    # 添加测试可执行文件
    add_executable(\${TEST_NAME} \${TEST_FILE})

    # 示例: 添加std线程依赖
    # find_package(Threads REQUIRED)
    # target_link_libraries(\${TEST_NAME} Threads::Threads)

    # 设置 FOLDER 属性, 使其按所在 demo 子目录分类
    set_target_properties(\${TEST_NAME} PROPERTIES FOLDER "$NAME/\${PARENT_DIR}")

    # 使用 Address Sanitizer
    if(HX_DEBUG_BY_ADDRESS_SANITIZER AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(\${TEST_NAME} PRIVATE -fsanitize=address)
    endif()
endforeach()" > ./src/$NAME/CMakeLists.txt

echo "int main() {
    return 0;
}" > ./src/$NAME/demo/00-example/01_main.cpp

echo "
已自动添加配置到 [cmake/subDir.cmake] 尾部"

echo "
option(BUILD_${BUILD_NAME} \"Build $NAME\" ON)

if(BUILD_${BUILD_NAME})
    add_subdirectory(src/$NAME)
    message(\"=-=-=-=-=-=-= Build $NAME =-=-=-=-=-=-=\")
endif()" >> ./cmake/subDir.cmake

echo "子项目已生成: $NAME"
