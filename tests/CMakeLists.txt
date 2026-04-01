enable_testing()

file(GLOB tests_subset   "test_*.c")

list(APPEND
    test_paths
    ${tests_subset}
)

foreach(test_path ${test_paths})

    get_filename_component(name ${test_path} NAME_WLE)

    add_executable(${name} ${test_path})

    target_link_libraries(${name}
        PRIVATE
            stz2
    )

    set_property(TARGET ${name} PROPERTY C_STANDARD 23)

    target_compile_options(${name}
        PRIVATE
            -O0
            -g
            -Wall
            -Wextra
            -fno-exceptions
            -fdiagnostics-color=always
    )

    add_test(
        NAME              ${name}
        COMMAND           ${name}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

endforeach()
