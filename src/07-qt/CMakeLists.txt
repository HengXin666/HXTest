# QT Start
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 一些配置
if(WIN32)
    # Qt编译器路径设置
    set(QT_COMPILER_PATH "D:/MyApp/Qt/6.8.2/msvc2022_64")

    # vcpkg下载的包的路径
    set(LIB_ROOT "D:/MyApp/vcpkg/installed/x64-windows")
endif()

add_subdirectory(01-qml)