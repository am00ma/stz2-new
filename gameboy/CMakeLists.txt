enable_testing()

file(GLOB games_subset   "*/game_*.c")

list(APPEND
    game_paths
    ${games_subset}
)

foreach(game_path ${game_paths})

    get_filename_component(name ${game_path} NAME_WLE)

    add_executable(${name} ${game_path})

    set_target_properties(${name} PROPERTIES LANGUAGES       C)
    set_target_properties(${name} PROPERTIES LINKER_LANGUAGE C)
    set_target_properties(${name} PROPERTIES C_STANDARD      23)

    target_include_directories(${name} PUBLIC ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${name} PUBLIC stz2)

    target_compile_options(${name}
        PRIVATE
            -O0
            -ggdb
            -Wall
            -Wextra
            -fdiagnostics-color=always)

endforeach()
