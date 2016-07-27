# This file is to be reused by target platforms that don't
# support `mason` (i.e. Yocto). Do not add any `mason` macro.

option(WITH_QT_DECODERS "Use builtin Qt image decoders" OFF)
option(WITH_QT_4        "Use Qt4 instead of Qt5"        OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -D__QT__")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fvisibility=hidden -D__QT__")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(MBGL_QT_FILES
    # File source
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/asset_file_source.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/default_file_source.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/online_file_source.cpp

    # Offline
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/mbgl/storage/offline.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/mbgl/storage/offline_database.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/mbgl/storage/offline_database.hpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/mbgl/storage/offline_download.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/mbgl/storage/offline_download.hpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/sqlite3.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/sqlite3.hpp

    # Misc
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/log_stderr.cpp

    # Headless headless_view_cgl
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/headless_display.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/headless_view.cpp

    # Platform integration
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/async_task.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/async_task_impl.hpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/http_file_source.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/http_file_source.hpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/http_request.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/http_request.hpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/image.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/qmapbox.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/qmapboxgl.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/qmapboxgl_p.hpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/run_loop.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/run_loop_impl.hpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/string_stdlib.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/timer.cpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/src/timer_impl.hpp

    # Qt resources
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/qmapbox.qrc

    # Public headers
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/include/qmapbox.hpp
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/include/qmapboxgl.hpp
)

include_directories(
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/include
)

# C++ app
add_executable(mbgl-qt
    ${CMAKE_SOURCE_DIR}/platform/qt/app/main.cpp
    ${CMAKE_SOURCE_DIR}/platform/qt/app/mapwindow.cpp
    ${CMAKE_SOURCE_DIR}/platform/qt/app/mapwindow.hpp
    ${CMAKE_SOURCE_DIR}/platform/qt/app/source.qrc
)

if(WITH_QT_4)
    include(${CMAKE_SOURCE_DIR}/platform/qt/qt4.cmake)
else()
    include(${CMAKE_SOURCE_DIR}/platform/qt/qt5.cmake)
endif()

# OS specific configurations
if (BUILD_PLATFORM STREQUAL "macos")
    list(APPEND MBGL_QT_FILES
        PRIVATE ${CMAKE_SOURCE_DIR}/platform/darwin/src/nsthread.mm
        PRIVATE ${CMAKE_SOURCE_DIR}/platform/darwin/src/headless_view_cgl.cpp
    )
    list(APPEND MBGL_QT_LIBRARIES
        PRIVATE "-framework Foundation"
        PRIVATE "-framework OpenGL"
    )
else()
    list(APPEND MBGL_QT_FILES
        PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/thread.cpp
        PRIVATE ${CMAKE_SOURCE_DIR}/platform/default/headless_view_glx.cpp
    )
    list(APPEND MBGL_QT_LIBRARIES
        PRIVATE -lGL
        PRIVATE -lX11
    )
endif()
