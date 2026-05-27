# Code coverage instrumentation and reporting.
#   - GCC:            gcov/lcov (--coverage)
#   - Clang/AppleClang: source-based coverage via llvm-cov (-fprofile-instr-generate -fcoverage-mapping)
#
# Usage:
#   cmake --preset multi -DPTRIE_COVERAGE=ON
#   cmake --build --preset release-deb --target coverage
# The `coverage` target runs the instrumented executables and produces an HTML
# report under ${CMAKE_BINARY_DIR}/coverage-html plus a textual summary.
option(PTRIE_COVERAGE "Instrument ptrie targets for code coverage (gcov/lcov for GCC, llvm-cov for Clang)" OFF)

set(PTRIE_COVERAGE_COMPILE_OPTIONS)
set(PTRIE_COVERAGE_LINK_OPTIONS)

# Excludes system headers, fetched dependencies and test/benchmark sources from llvm-cov reports.
set(PTRIE_COVERAGE_IGNORE_REGEX "(/usr/|/_deps/|/test/|/benchmark/)")

if (PTRIE_COVERAGE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(PTRIE_COVERAGE_COMPILE_OPTIONS --coverage -O0 -g)
        set(PTRIE_COVERAGE_LINK_OPTIONS --coverage)
        message(STATUS "Enabled code coverage (gcov/lcov)")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang") # matches both Clang and AppleClang
        set(PTRIE_COVERAGE_COMPILE_OPTIONS -fprofile-instr-generate -fcoverage-mapping -O0 -g)
        set(PTRIE_COVERAGE_LINK_OPTIONS -fprofile-instr-generate)
        message(STATUS "Enabled code coverage (llvm-cov)")
    else ()
        message(WARNING "Code coverage is not supported for ${CMAKE_CXX_COMPILER_ID}, PTRIE_COVERAGE ignored")
        set(PTRIE_COVERAGE OFF)
    endif ()
endif (PTRIE_COVERAGE)

# Applies coverage instrumentation to the given target and registers it so that
# the `coverage` report target knows which executables to run. No-op when PTRIE_COVERAGE is OFF.
function(target_enable_coverage target)
    if (PTRIE_COVERAGE)
        target_compile_options(${target} PRIVATE ${PTRIE_COVERAGE_COMPILE_OPTIONS})
        target_link_options(${target} PRIVATE ${PTRIE_COVERAGE_LINK_OPTIONS})
        set_property(GLOBAL APPEND PROPERTY PTRIE_COVERAGE_TARGETS ${target})
    endif ()
endfunction()

# Derives the matching gcov tool for the active GCC (e.g. gcov-14) so lcov reads
# the .gcda format produced by that compiler version.
function(_ptrie_find_gcov out_var)
    string(REGEX MATCH "^[0-9]+" _major "${CMAKE_CXX_COMPILER_VERSION}")
    find_program(GCOV_TOOL NAMES "gcov-${_major}" gcov)
    set(${out_var} "${GCOV_TOOL}" PARENT_SCOPE)
endfunction()

# Creates the `coverage` custom target. Call once from the top-level CMakeLists
# after all instrumented targets have been registered via target_enable_coverage().
function(ptrie_add_coverage_report)
    if (NOT PTRIE_COVERAGE)
        return()
    endif ()
    get_property(_targets GLOBAL PROPERTY PTRIE_COVERAGE_TARGETS)
    if (NOT _targets)
        message(STATUS "Coverage: no instrumented targets registered, `coverage` target not created")
        return()
    endif ()

    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        find_program(LCOV lcov)
        find_program(GENHTML genhtml)
        if (NOT LCOV OR NOT GENHTML)
            message(WARNING "Coverage: lcov/genhtml not found, `coverage` target not created")
            return()
        endif ()
        _ptrie_find_gcov(_gcov)
        # lcov >= 2.0 treats a --remove pattern that matches nothing as a fatal error
        # (e.g. */benchmark/* never appears because benchmarks are not instrumented),
        # so downgrade that to a warning where the option is supported.
        set(_lcov_ignore)
        execute_process(COMMAND ${LCOV} --version OUTPUT_VARIABLE _lcov_ver ERROR_QUIET)
        string(REGEX MATCH "([0-9]+)\\.([0-9]+)" _lcov_ver "${_lcov_ver}")
        if (_lcov_ver AND _lcov_ver VERSION_GREATER_EQUAL "2.0")
            set(_lcov_ignore --ignore-errors unused)
        endif ()
        set(_run_cmds)
        foreach (_t IN LISTS _targets)
            list(APPEND _run_cmds COMMAND $<TARGET_FILE:${_t}>)
        endforeach ()
        add_custom_target(coverage
            ${_run_cmds}
            COMMAND ${LCOV} --quiet --gcov-tool ${_gcov} --directory ${CMAKE_BINARY_DIR}
                    --capture --output-file coverage.info
            COMMAND ${LCOV} --quiet --remove coverage.info ${_lcov_ignore}
                    "/usr/*" "*/_deps/*" "*/test/*" "*/benchmark/*"
                    --output-file coverage.info
            COMMAND ${LCOV} --list coverage.info
            COMMAND ${GENHTML} --quiet coverage.info --output-directory coverage-html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ${_targets}
            COMMENT "Generating gcov/lcov coverage report in ${CMAKE_BINARY_DIR}/coverage-html"
            VERBATIM)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        string(REGEX MATCH "^[0-9]+" _major "${CMAKE_CXX_COMPILER_VERSION}")
        find_program(LLVM_PROFDATA NAMES "llvm-profdata-${_major}" llvm-profdata)
        find_program(LLVM_COV NAMES "llvm-cov-${_major}" llvm-cov)
        if (NOT LLVM_PROFDATA OR NOT LLVM_COV)
            message(WARNING "Coverage: llvm-profdata/llvm-cov not found, `coverage` target not created")
            return()
        endif ()
        # Run each executable so it writes its own raw profile, then merge.
        set(_run_cmds)
        set(_profraws)
        set(_objects)
        foreach (_t IN LISTS _targets)
            list(APPEND _run_cmds
                COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${_t}.profraw $<TARGET_FILE:${_t}>)
            list(APPEND _profraws ${_t}.profraw)
            list(APPEND _objects -object $<TARGET_FILE:${_t}>)
        endforeach ()
        add_custom_target(coverage
            ${_run_cmds}
            COMMAND ${LLVM_PROFDATA} merge -sparse ${_profraws} -o coverage.profdata
            COMMAND ${LLVM_COV} report ${_objects} -instr-profile=coverage.profdata
                    -ignore-filename-regex=${PTRIE_COVERAGE_IGNORE_REGEX}
            COMMAND ${LLVM_COV} show ${_objects} -instr-profile=coverage.profdata
                    -format=html -output-dir=coverage-html
                    -ignore-filename-regex=${PTRIE_COVERAGE_IGNORE_REGEX}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ${_targets}
            COMMENT "Generating llvm-cov coverage report in ${CMAKE_BINARY_DIR}/coverage-html"
            VERBATIM)
    endif ()
endfunction()
