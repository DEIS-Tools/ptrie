cmake_minimum_required(VERSION 3.28.3)
project(ptrie VERSION 1.1.3)

option(PTRIE_BuildTests "Build the unit tests" ON)
option(PTRIE_BuildBenchmark "Build the simple benchmark suite" ON)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/warnings.cmake)
include(cmake/sanitizers.cmake) # CMake options: SSP, UBSAN, ASAN, LSAN, TSAN, etc
include(GNUInstallDirs) # CMAKE_INSTALL_BINDIR, CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_INCLUDEDIR etc

# Header-only library
add_library(ptrie INTERFACE)
add_library(ptrie::ptrie ALIAS ptrie)
target_compile_features(ptrie INTERFACE cxx_std_20) # Require C++20 features.
target_include_directories(ptrie INTERFACE
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # depends on include(GNUInstallDirs)
)

if (PTRIE_BuildTests OR PTRIE_Benchmarks)
    enable_testing()
endif()

if(PTRIE_BuildTests)
    include(cmake/doctest.cmake)
    add_subdirectory(test)
endif(PTRIE_BuildTests)

if(PTRIE_BuildBenchmark)
    include(cmake/benchmark.cmake)
    include(cmake/sparsehash.cmake)
    add_subdirectory(benchmark)
endif(PTRIE_BuildBenchmark)

include(CMakePackageConfigHelpers)

set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# Define which directories to install with the ptrie library.
install(DIRECTORY include/ptrie/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ptrie)

install(TARGETS ptrie
        EXPORT ${PROJECT_NAME}-config)

configure_package_config_file(cmake/${PROJECT_NAME}-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
        INSTALL_DESTINATION ${INSTALL_CONFIGDIR})
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

install(EXPORT ${PROJECT_NAME}-config
        FILE ptrie-config.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${INSTALL_CONFIGDIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
        DESTINATION ${INSTALL_CONFIGDIR})
