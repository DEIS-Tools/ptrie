find_package(sparsehash 2.0.4 QUIET)
if (sparsehash_FOUND)
    message(STATUS "Found ptrie: ${sparsehash_SOURCE_DIR}")
else (sparsehash_FOUND)
    message(STATUS "Failed to find sparsehash, going to fetch from source")
    set(FETCHCONTENT_QUIET ON)
    set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
    include(FetchContent)
    FetchContent_Declare(sparsehash
            GIT_REPOSITORY https://github.com/sparsehash/sparsehash.git
            GIT_TAG sparsehash-2.0.4
            GIT_SHALLOW TRUE  # download specific revision only (git clone --depth 1)
            GIT_PROGRESS TRUE # show download progress in Ninja
            FIND_PACKAGE_ARGS NAMES sparsehash
            USES_TERMINAL_DOWNLOAD TRUE)
    FetchContent_MakeAvailable(sparsehash)
    message(STATUS "Got ptrie: ${sparsehash_SOURCE_DIR}")
    # Workaround until ptrie exports proper cmake config:
    add_library(::ptrie INTERFACE IMPORTED GLOBAL)
    target_include_directories(ptrie::ptrie INTERFACE ${ptrie_SOURCE_DIR}/src)
endif (ptrie_FOUND)
