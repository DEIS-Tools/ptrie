cmake_minimum_required(VERSION 3.28.3)

cmake_policy(SET CMP0048 NEW)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(PTRIE_VERSION 1.1.2)
project(ptrie VERSION ${PTRIE_VERSION} LANGUAGES CXX)

# MAIN_PROJECT CHECK
# Determine if ptrie is built as a subproject (using add_subdirectory) or if it is the main project
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(PTRIE_MAIN_PROJECT ON)
else ()
    set(PTRIE_MAIN_PROJECT OFF)
endif()
message(STATUS "PTRIE_MAIN_PROJECT=${PTRIE_MAIN_PROJECT}")

option(PTRIE_BuildTests "Build the unit tests when BUILD_TESTING is enabled." ${PTRIE_MAIN_PROJECT})
option(PTRIE_BuildBenchmark "Build the simple benchmark suite" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/warnings.cmake)

if(NOT CMAKE_BUILD_TYPE MATCHES Prebuild)
    add_subdirectory(src)
    if(PTRIE_BuildTests)
        #testing
        enable_testing()
        add_subdirectory(test)
    endif()
    if(PTRIE_BuildBenchmark)
        #benchmark
        enable_testing()
        add_subdirectory(benchmark)
    endif()
else()
    add_custom_target(prebuild)
    add_dependencies(prebuild boost-ext)
endif()
