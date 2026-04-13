cmake_minimum_required(VERSION 3.15)

# =========================================================================
#                       Project
# =========================================================================
project(
    "stz2"
    VERSION 0.0.1
    DESCRIPTION "Standard library for C"
    LANGUAGES C)

# =========================================================================
#                      Options
# =========================================================================
option(STZ_BUILD_TESTS "Build tests" ON)

# =========================================================================
#                      stz
# =========================================================================
set(name stz2)

add_library(${name} STATIC)

set_target_properties(${name} PROPERTIES LANGUAGES       C)
set_target_properties(${name} PROPERTIES LINKER_LANGUAGE C)
set_target_properties(${name} PROPERTIES C_STANDARD      23)

target_sources(${name}
    PRIVATE
        src/path.c
        src/args.c
        src/cmd.c
        src/db.c

        src/rt/ring.c
        src/rt/atomic.c

        src/md/toc.c

        src/gui/SDL_FontCache.c
        src/gui/draw.c
        src/gui/types.c

        src/web/registry.c

    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
        include
    FILES
        include/stz2/types.h
        include/stz2/test.h
        include/stz2/parse.h
        include/stz2/args.h
        include/stz2/cmd.h
        include/stz2/db.h
        include/stz2/table.h

        include/stz2/rt/ring.h
        include/stz2/rt/atomic.h

        include/stz2/md/toc.h

        include/stz2/gui/assets.h

        include/stz2/gui/SDL_FontCache.h
        include/stz2/gui/draw.h
        include/stz2/gui/types.h

        include/stz2/web/registry.h

)

target_include_directories(${name}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${name}
    PUBLIC
        m
        md4c
        sqlite3
        SDL2
        SDL2_ttf
        SDL2_image
)

target_compile_options(${name}
    PRIVATE
        -O0
        -ggdb
        -Wall
        -Wextra
        # -fsanitize=address
        -fdiagnostics-color=always)
# target_link_options(${name} PRIVATE -fsanitize=address)

# =========================================================================
#                      Installation
# =========================================================================
install(
    TARGETS ${name}
    EXPORT ${name}Targets
    FILE_SET HEADERS
)

include(GNUInstallDirs)

install(
    EXPORT ${name}Targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${name}
    NAMESPACE ${name}::
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${name}ConfigVersion.cmake
    COMPATIBILITY ExactVersion
)

install(
    FILES
        cmake/${name}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${name}ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${name}
)

# =========================================================================
#                      tests
# =========================================================================
if(STZ_BUILD_TESTS)
    add_subdirectory(tests)
endif()
