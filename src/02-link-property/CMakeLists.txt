project(02-link-property LANGUAGES CXX)

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS 
    *.cpp 
    *.h
    *.hpp
)

# 编译可执行文件
add_executable(02-link-property ${srcs})

# 使用 Address Sanitizer
if(HX_DEBUG_BY_ADDRESS_SANITIZER AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(02-link-property PRIVATE -fsanitize=address)
endif()
