option(BUILD_01_REFLECTION "Build 01-reflection" ON)

if(BUILD_01_REFLECTION)
    add_subdirectory(src/01-reflection)
    message("=-=-=-=-=-=-= Build 01-reflection =-=-=-=-=-=-=")
endif()

option(BUILD_01_HANSYA_LIVE "Build 01-hansya-live" ON)

if(BUILD_01_HANSYA_LIVE)
    add_subdirectory(src/01-hansya-live)
    message("=-=-=-=-=-=-= Build 01-hansya-live =-=-=-=-=-=-=")
endif()

option(BUILD_02_LINK_PROPERTY "Build 02-link-property" ON)

if(BUILD_02_LINK_PROPERTY)
    add_subdirectory(src/02-link-property)
    message("=-=-=-=-=-=-= Build 02-link-property =-=-=-=-=-=-=")
endif()

option(BUILD_03_RANDOM "Build 03-random" ON)

if(BUILD_03_RANDOM)
    add_subdirectory(src/03-random)
    message("=-=-=-=-=-=-= Build 03-random =-=-=-=-=-=-=")
endif()

option(BUILD_04_TBB "Build 04-tbb" ON)

if(BUILD_04_TBB)
    add_subdirectory(src/04-tbb)
    message("=-=-=-=-=-=-= Build 04-tbb =-=-=-=-=-=-=")
endif()

option(BUILD_05_EXCEPTION "Build 05-exception" ON)

if(BUILD_05_EXCEPTION)
    add_subdirectory(src/05-exception)
    message("=-=-=-=-=-=-= Build 05-exception =-=-=-=-=-=-=")
endif()

option(BUILD_06_STD_ANALYSE "Build 06-std-analyse" ON)

if(BUILD_06_STD_ANALYSE)
    add_subdirectory(src/06-std-analyse)
    message("=-=-=-=-=-=-= Build 06-std-analyse =-=-=-=-=-=-=")
endif()

option(BUILD_07_QT "Build 07-QT" ON)

if(BUILD_07_QT)
    add_subdirectory(src/07-qt)
    message("=-=-=-=-=-=-= Build 07-QT =-=-=-=-=-=-=")
endif()
