find_program(HEAPTRACK NAMES heaptrack)
if (HEAPTRACK)
    if (ASAN OR LSAN)
        message(STATUS "Disabled heaptrack (incompatible with sanitizers")
        set(HEAPTRACK OFF)
    else ()
        find_program(HEAPTRACK_PRINT NAMES heaptrack_print)
        if (HEAPTRACK_PRINT)
            message(STATUS "Found heaptrack and heaptrack_print, enabling heap tracking tests")
        else (HEAPTRACK_PRINT)
            message(STATUS "Failed to find heaptrack_print, heap tracking is going to be disabled")
            set(HEAPTRACK OFF)
        endif (HEAPTRACK_PRINT)
    endif ()
else (HEAPTRACK)
    message(STATUS "Failed to find heaptrack, heap tracking is going to be disabled")
endif (HEAPTRACK)

# Adds heaptracking test with arguments just like add_test
macro(add_heaptrack)
    if (HEAPTRACK)
        #message(STATUS "add_heaptrack(${ARGN}")
        cmake_parse_arguments(ARG "" "NAME" "COMMAND" "CONFIGURATIONS" "WORKING_DIRECTORY" "COMMAND_EXPAND_LISTS" ${ARGN})
        add_test(NAME ${ARG_NAME} COMMAND ${HEAPTRACK} --record-only -o ${ARG_NAME} ${ARG_COMMAND})
        if (HEAPTRACK_PRINT)
            add_test(NAME ${ARG_NAME}_heap COMMAND
                ${HEAPTRACK_PRINT} --print-peaks=0 --print-allocators=0 --print-temporary=0 --print-leaks=1 --suppressions ${PROJECT_SOURCE_DIR}/cmake/heaptrack_suppress.txt ${ARG_NAME}.zst
                 CONFIGURATIONS ${ARG_CONFIGURATIONS}
                 WORKING_DIRECTORY ${ARG_WORKING_DIRECTORY}
                 COMMAND_EXPAND_LISTS ${ARG_COMMAND_EXPAND_LISTS}
            )
            set_tests_properties(${ARG_NAME}_heap PROPERTIES DEPENDS ${ARG_NAME}
                PASS_REGULAR_EXPRESSION "total memory leaked: 0B")
        endif (HEAPTRACK_PRINT)
    endif (HEAPTRACK)
endmacro()

# Adds heaptracking test if heaptracking is available, otherwise falls back to simple add_test
macro(add_heaptrack_test)
    #message(STATUS "add_heaptrack_test(${ARGN})")
    cmake_parse_arguments(ARG "" "NAME" "COMMAND" "CONFIGURATIONS" "WORKING_DIRECTORY" "COMMAND_EXPAND_LISTS" ${ARGN})
    if (HEAPTRACK)
        add_heaptrack(NAME ${ARG_NAME} COMMAND ${ARG_COMMAND}
            CONFIGURATIONS ${ARG_CONFIGURATIONS}
            WORKING_DIRECTORY ${ARG_WORKING_DIRECTORY}
            COMMAND_EXPAND_LISTS ${ARG_COMMAND_EXPAND_LISTS})
    else()
        add_test(NAME ${ARG_NAME} COMMAND ${ARG_COMMAND}
                CONFIGURATIONS ${ARG_CONFIGURATIONS}
                WORKING_DIRECTORY ${ARG_WORKING_DIRECTORY}
                COMMAND_EXPAND_LISTS ${ARG_COMMAND_EXPAND_LISTS})
    endif (HEAPTRACK)
endmacro()
