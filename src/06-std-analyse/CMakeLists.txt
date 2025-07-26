# target名格式: {name}___{dir}.cpp, 比如 main___thread
# 对 demo 和 test 的 cpp 文件生成可执行文件

project(06-std-analyse LANGUAGES CXX)

file(GLOB_RECURSE DEMO_FILES CONFIGURE_DEPENDS 
    ./demo/*.cpp
)

file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS 
    ./test/*.cpp
)

include_directories(../09-os/include)

function(add_targets_from_files FILE_LIST BASE_FOLDER)
    foreach(TEST_FILE ${FILE_LIST})
        # 文件名 (无扩展名)
        get_filename_component(FILE_NAME ${TEST_FILE} NAME_WE)

        # 父目录名 (所在目录最后一级)
        get_filename_component(TEST_DIR ${TEST_FILE} DIRECTORY)
        get_filename_component(PARENT_DIR ${TEST_DIR} NAME)

        # 拼接 target 名: {name}___{dir}
        set(TARGET_NAME "${FILE_NAME}___${PARENT_DIR}")

        # 添加可执行文件
        add_executable(${TARGET_NAME} ${TEST_FILE})

        # 设置 VS FOLDER 分类
        set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${PARENT_DIR}")

        # Address Sanitizer 支持
        if(HX_DEBUG_BY_ADDRESS_SANITIZER)
            target_compile_options(${TARGET_NAME} PRIVATE
                $<$<CONFIG:Debug>:-fsanitize=address>)
            target_link_options(${TARGET_NAME} PRIVATE
                $<$<CONFIG:Debug>:-fsanitize=address>)
        endif()
    endforeach()
endfunction()

add_targets_from_files("${DEMO_FILES}" "06-std-analyse/demo")
add_targets_from_files("${TEST_FILES}" "06-std-analyse/test")
