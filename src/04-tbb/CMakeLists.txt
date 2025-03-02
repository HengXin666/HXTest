project(04-tbb LANGUAGES CXX)

file(GLOB TEST_FILES CONFIGURE_DEPENDS 
    ./demo/*.cpp
)

# 遍历每个 .cpp 文件, 生成可执行文件
foreach(TEST_FILE ${TEST_FILES})
    # 提取 .cpp 文件名作为目标名 (去掉路径和扩展名)
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

    # 添加测试可执行文件
    add_executable("04-tbb-${TEST_NAME}" ${TEST_FILE})

    # 添加std线程依赖
    find_package(Threads REQUIRED)
    target_link_libraries("04-tbb-${TEST_NAME}" Threads::Threads)

    # 添加TBB依赖
    find_package(TBB REQUIRED)
    target_link_libraries("04-tbb-${TEST_NAME}" TBB::tbb)

    # 使用 Address Sanitizer
    if(HX_DEBUG_BY_ADDRESS_SANITIZER AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options("04-tbb-${TEST_NAME}" PRIVATE -fsanitize=address)
    endif()
endforeach()
