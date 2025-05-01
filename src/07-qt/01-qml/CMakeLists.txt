set(projectName "HX-01-QML")

project(${projectName}
    VERSION 0.0.1
    DESCRIPTION ${projectName}
    HOMEPAGE_URL "https://github.com/HengXin666/HXTest"
    LANGUAGES CXX
)

file(GLOB_RECURSE src_files CONFIGURE_DEPENDS 
    src/*.cpp
    include/*.h
    include/*.hpp
)

file(GLOB_RECURSE qrc_files CONFIGURE_DEPENDS 
    resources/*.qrc
)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick)

qt_add_executable(${projectName}
    ${src_files}
    ${qrc_files}
)

target_compile_features(${projectName} PUBLIC cxx_std_20)

target_link_libraries(${projectName}
    PRIVATE Qt::Core
    PRIVATE Qt::Gui
    PRIVATE Qt::Widgets
    PRIVATE Qt::Qml
    PRIVATE Qt::Quick
)

# 添加 QML 文件所在目录作为资源路径（推荐方式）
qt_add_qml_module(${projectName}
    URI ${projectName}  # QML 中 import 的名字
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

# Qt拓展 (PDF操作 与 渲染)
# find_package(Qt6 REQUIRED COMPONENTS Pdf PdfWidgets)
# target_link_libraries(${projectName} PRIVATE Qt6::Pdf Qt::PdfWidgets)

# Qt拓展 (音频播放)
# find_package(Qt6 REQUIRED COMPONENTS Multimedia)
# target_link_libraries(${projectName} PRIVATE Qt::Multimedia)

# Qt拓展 (SVG)
# find_package(Qt6 REQUIRED COMPONENTS Svg)
# target_link_libraries(${projectName} PRIVATE Qt::Svg)

# Qt拓展 (XML)
# find_package(Qt6 REQUIRED COMPONENTS Xml)
# target_link_libraries(${projectName} PRIVATE Qt::Xml)

# Qt拓展 (编码)
# find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
# target_link_libraries(${projectName} PRIVATE Qt::Core5Compat)

# Qt拓展 (并行库)
# find_package(Qt6 REQUIRED COMPONENTS Concurrent)
# target_link_libraries(${projectName} PRIVATE Qt::Concurrent)

# Qt拓展 (网络库)
# find_package(Qt6 REQUIRED COMPONENTS Network)
# target_link_libraries(${projectName} PRIVATE Qt::Network)

if (WIN32)
    # 解决路径问题, 确保 windeployqt.exe 存在
    set(QT_BIN_DIR "${QT_COMPILER_PATH}/bin")
    if(NOT EXISTS "${QT_BIN_DIR}/windeployqt.exe")
        message(FATAL_ERROR "Error: windeployqt.exe not found in ${QT_BIN_DIR}")
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Debug")
        add_custom_command(TARGET ${projectName} POST_BUILD
            COMMAND "${QT_BIN_DIR}/windeployqt.exe" --debug "$<TARGET_FILE:${projectName}>"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
        )
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Release")
        add_custom_command(TARGET ${projectName} POST_BUILD
            COMMAND "${QT_BIN_DIR}/windeployqt.exe" --release "$<TARGET_FILE:${projectName}>"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Release"
        )
    endif()
endif()

set_target_properties(${projectName} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE OFF
    WIN32_EXECUTABLE OFF # 这里需要为 OFF 才可以让vscode在控制台中输出...
)

include(GNUInstallDirs)
install(TARGETS ${projectName}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)