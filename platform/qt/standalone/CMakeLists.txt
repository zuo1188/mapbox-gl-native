cmake_minimum_required(VERSION 3.2)
project(qmapboxgl LANGUAGES CXX C)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../..)

set(MBGL_GENERATED ${CMAKE_BINARY_DIR}/generated)

include(${CMAKE_SOURCE_DIR}/cmake/core-files.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/shaders.cmake)
include(${CMAKE_SOURCE_DIR}/platform/qt/qt.cmake)
include(${CMAKE_SOURCE_DIR}/platform/qt/standalone/standalone.cmake)

add_library(mbgl-core STATIC
    ${MBGL_CORE_FILES}
)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/platform/default
    ${CMAKE_SOURCE_DIR}/platform/qt/include
    ${CMAKE_SOURCE_DIR}/src
    ${MBGL_GENERATED}/include
)

add_dep(earcut            662398 mapbox/earcut.hpp) # v0.11 + header fix
add_dep(geojson-cpp       v0.1.4 mapbox/geojson-cpp)
add_dep(geojson-vt-cpp    v6.1.0 mapbox/geojson-vt-cpp)
add_dep(geometry.hpp      v0.8.0 mapbox/geometry.hpp)
add_dep(mapbox-gl-shaders 4d1f89 mapbox/mapbox-gl-shaders)
add_dep(protozero         v1.3.0 mapbox/protozero)
add_dep(rapidjson         v1.0.2 miloyip/rapidjson)
add_dep(unique_resource   cba309 okdshin/unique_resource)
add_dep(variant           v1.1.1 mapbox/variant)

add_dep(boost_algorithm  boost-1.60.0 boostorg/algorithm)
add_dep(boost_functional boost-1.60.0 boostorg/functional)
add_dep(boost_geometry   boost-1.60.0 boostorg/geometry)
add_dep(boost_iterator   boost-1.60.0 boostorg/iterator)
add_dep(boost_regex      boost-1.60.0 boostorg/regex)
add_dep(boost_spirit     boost-1.60.0 boostorg/spirit)

target_sources(mbgl-core
    ${MBGL_QT_FILES}
    PRIVATE ${CMAKE_SOURCE_DIR}/platform/qt/standalone/geojson-hack.cpp
)

add_custom_target(mbgl-headers DEPENDS
    ${MBGL_SHADER_FILES}
    ${MBGL_GENERATED}/include/mbgl/util/version.hpp
)

add_dependencies(mbgl-core
    mbgl-headers
)

target_compile_definitions(mbgl-core
    PRIVATE -DQT_IMAGE_DECODERS
)

# FIXME: Qt should not depend on sqlite3
target_link_libraries(mbgl-core
    ${MBGL_QT_LIBRARIES}
    PRIVATE -lsqlite3
)
