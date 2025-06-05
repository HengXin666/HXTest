project(08-OpenGL LANGUAGES C CXX)

file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS 
    ./demo/*.cpp
)

function(disable_warnings_if_target_exists target)
    if (TARGET ${target})
        if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options(${target} PRIVATE -w)
        elseif (MSVC)
            target_compile_options(${target} PRIVATE /w)
        endif()
    endif()
endfunction()

# 添加依赖
add_subdirectory(lib/glm)
add_subdirectory(lib/glfw)
add_subdirectory(lib/glad)

# 针对 C 库 target 禁用警告
disable_warnings_if_target_exists(glfw)
disable_warnings_if_target_exists(glad)

# 遍历每个 .cpp 文件, 生成可执行文件
foreach(TEST_FILE ${TEST_FILES})
    # 提取 .cpp 文件名作为目标名 (去掉路径和扩展名)
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

    # 获取 .cpp 文件所在的目录 (相对路径)
    get_filename_component(TEST_DIR ${TEST_FILE} DIRECTORY)
    
    # 获取 TEST_DIR 的最后一级目录名 (即父文件夹名)
    get_filename_component(PARENT_DIR ${TEST_DIR} NAME)
    
    # 添加测试可执行文件
    add_executable(${TEST_NAME} ${TEST_FILE})

    # 示例: 添加std线程依赖
    # find_package(Threads REQUIRED)
    # target_link_libraries(${TEST_NAME} Threads::Threads)

    target_link_libraries(${TEST_NAME} PUBLIC glm)
    target_link_libraries(${TEST_NAME} PUBLIC glfw)
    target_link_libraries(${TEST_NAME} PUBLIC glad)

    # 设置 FOLDER 属性, 使其按所在 demo 子目录分类
    set_target_properties(${TEST_NAME} PROPERTIES FOLDER 08-OpenGL/${PARENT_DIR})

    # 使用 Address Sanitizer
    if(HX_DEBUG_BY_ADDRESS_SANITIZER AND CMAKE_BUILD_TYPE STREQUAL Debug)
        target_compile_options(${TEST_NAME} PRIVATE -fsanitize=address)
    endif()
endforeach()
