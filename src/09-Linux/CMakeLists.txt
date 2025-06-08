project(09-Linux LANGUAGES CXX)

file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS 
    ./demo/*.cpp
)

if(NOT WIN32)
    set(HX_DEBUG_BY_ADDRESS_SANITIZER TRUE)
endif()


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

    # 设置 FOLDER 属性, 使其按所在 demo 子目录分类
    set_target_properties(${TEST_NAME} PROPERTIES FOLDER 09-Linux/${PARENT_DIR})

    # 使用 Address Sanitizer
    if(HX_DEBUG_BY_ADDRESS_SANITIZER)
        target_compile_options(${TEST_NAME} PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address>)

        target_link_options(${TEST_NAME} PRIVATE
            $<$<CONFIG:Debug>:-fsanitize=address>)
    endif()
endforeach()
