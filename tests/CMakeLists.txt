enable_testing()

file(GLOB tests_subset   "test_*.c")

list(APPEND
    test_paths
    ${tests_subset}
)

foreach(test_path ${test_paths})

    get_filename_component(name ${test_path} NAME_WLE)

    add_executable(${name} ${test_path})

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
            # -fsanitize=address
            -fdiagnostics-color=always)
    # target_link_options(${name} PRIVATE -fsanitize=address)

    add_test(
        NAME              ${name}
        COMMAND           ${name}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

endforeach()
