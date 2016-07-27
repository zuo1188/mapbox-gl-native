include(ExternalProject)

# Macro for cloning and downloading our header-only dependencies
macro(add_dep package tag git)
    externalproject_add(${package}
        URL https://github.com/${git}/archive/${tag}.zip
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
    )

    add_dependencies(mbgl-core ${package})

    externalproject_get_property(${package} SOURCE_DIR)
    target_include_directories(mbgl-core PRIVATE ${SOURCE_DIR})
    target_include_directories(mbgl-core PRIVATE ${SOURCE_DIR}/include)

    set(${package}_SOURCE_DIR ${SOURCE_DIR})
endmacro()

add_custom_command(
    OUTPUT ${MBGL_GENERATED}/include/mbgl/util/version.hpp
    DEPENDS ${CMAKE_SOURCE_DIR}/package.json
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/build-version.py ${MBGL_GENERATED}
    VERBATIM
)
