#!/bin/bash
echo "=== 生成独立CMake子项目[快捷指引] (By Heng_Xin) ==="
read -p "请输入项目名称(如: 09-loli): " NAME

# 将项目名称转换为大写并将 '-' 替换为 '_'
BUILD_NAME=$(echo "$NAME" | tr '[:lower:]-' '[:upper:]_')

mkdir -p "./src/$NAME"

echo "project($NAME LANGUAGES CXX)

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS 
    *.cpp 
    *.h
    *.hpp
)

# 编译可执行文件
add_executable($NAME \${srcs})

# 使用 Address Sanitizer
if(HX_DEBUG_BY_ADDRESS_SANITIZER AND CMAKE_BUILD_TYPE STREQUAL \"Debug\")
    target_compile_options($NAME PRIVATE -fsanitize=address)
endif()" > ./src/$NAME/CMakeLists.txt

echo "int main() {
    return 0;
}" > ./src/$NAME/main.cpp

echo "
请将下面内容复制并且添加到到[根CMakeLists.txt]:"

echo "
option(BUILD_${BUILD_NAME} \"Build $NAME\" ON)

if(BUILD_${BUILD_NAME})
    add_subdirectory(src/$NAME)
endif()
"

echo "子项目已生成: $NAME"
