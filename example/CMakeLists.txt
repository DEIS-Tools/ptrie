cmake_minimum_required(VERSION 3.28)
project(example)

set(CMAKE_CXX_EXTENSIONS OFF) # use -std=c++20 instead of -std=gnu++20
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # helps with linking
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # build system details for linters

#find_package(ptrie REQUIRED 1.1.2) # simple when already installed
include(../cmake/ptrie.cmake) # universal: find_package or fetch from sources

enable_testing()

add_executable(example example.cpp)
target_link_libraries(example PRIVATE ptrie::ptrie)
add_test(NAME example COMMAND example)
