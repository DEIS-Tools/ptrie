find_package(benchmark QUIET) # benchmarkConfigVersion may have garbage version numbers

if (benchmark_FOUND)
  set(benchmark_SOURCE_DIR ${benchmark_DIR})
  message(STATUS "Found benchmark: ${benchmark_SOURCE_DIR}")
else(benchmark_FOUND)
  include(FetchContent)
  message(STATUS "Failed to find benchmark, going to compile from source.")
  if (FIND_FATAL)
    message(FATAL_ERROR "Failed to find benchmark with CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}")
  endif(FIND_FATAL)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable tests")
  set(BENCHMARK_ENABLE_EXCEPTIONS ON CACHE BOOL "Enable the use of exceptions in the benchmark library. Default ON")
  set(BENCHMARK_ENABLE_LTO OFF CACHE BOOL "Enable link time optimisation of the benchmark library. Default OFF")
  set(BENCHMARK_USE_LIBCXX OFF CACHE BOOL "Build and test using libc++ as the standard library. Default OFF")
  set(BENCHMARK_ENABLE_WERROR ON CACHE BOOL "Build Release candidates with -Werror. Default ON")
  set(BENCHMARK_FORCE_WERROR OFF CACHE BOOL "Build Release candidates with -Werror regardless of compiler issues. Default OFF")
  FetchContent_Declare(benchmark
    GIT_REPOSITORY https://github.com/google/benchmark
    GIT_TAG v1.9.4
    GIT_SHALLOW TRUE # get only the last commit version
    GIT_PROGRESS TRUE # show progress of download
    FIND_PACKAGE_ARGS NAMES benchmark
    USES_TERMINAL_DOWNLOAD TRUE # show progress in ninja generator
    USES_TERMINAL_CONFIGURE ON
    USES_TERMINAL_BUILD ON
    USES_TERMINAL_INSTALL ON
    EXCLUDE_FROM_ALL
    )
  FetchContent_GetProperties(benchmark)
  if (benchmark_POPULATED)
    message(STATUS "Found populated benchmark: ${benchmark_SOURCE_DIR}")
  else (benchmark_POPULATED)
    FetchContent_MakeAvailable(benchmark)
    message(STATUS "Got benchmark: ${benchmark_SOURCE_DIR}")
  endif()
endif(benchmark_FOUND)

if (benchmark_SOURCE_DIR)
  find_program(python_CMD NAMES python3 python)
  if (python_CMD)
    set(benchmark_cmp python3 ${benchmark_SOURCE_DIR}/tools/compare.py)
    set(benchmark_plot python3 ${CMAKE_SOURCE_DIR}/cmake/plot.py)
    message(STATUS "Enabled benchmark comparison (assuming python: scipy, pandas, matplotlib are installed)")
  else (python_CMD)
    message(WARNING "Failed to find python3: disabled benchmark visualization")
  endif (python_CMD)
endif (benchmark_SOURCE_DIR)