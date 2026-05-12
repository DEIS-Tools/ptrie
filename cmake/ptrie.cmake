# Ensures that ptrie is installed: find_package or fetch from source
include(FetchContent)
option(PTRIE_REPO "The repository to fetch PTRIE if find_package fails" https://github.com/DEIS-Tools/ptrie)
option(PTRIE_BRANCH "The branch to fetch PTRIE if find_package fails" main)
FetchContent_Declare(ptrie
    GIT_REPOSITORY ${PTRIE_REPO}
    GIT_TAG ${PTRIE_BRANCH}
    GIT_SHALLOW TRUE  # download specific revision only (git clone --depth 1)
    GIT_PROGRESS TRUE # show download progress in Ninja
    USES_TERMINAL_DOWNLOAD TRUE
    EXCLUDE_FROM_ALL # don't build if not used
    FIND_PACKAGE_ARGS 1.1.2) # minimum version for find_package

set(PTRIE_BuildTests OFF CACHE BOOL "Build the unit tests when BUILD_TESTING is enabled.")
set(PTRIE_BuildBenchmark OFF CACHE BOOL "Build the simple benchmark suite")
FetchContent_MakeAvailable(ptrie)

if (ptrie_FOUND) # find_package
    message(STATUS "Found ptrie: ${ptrie_DIR}")
else (ptrie_FOUND) # FetchContent
    message(STATUS "Fetched ptrie: ${ptrie_SOURCE_DIR}")
endif (ptrie_FOUND)

if (TARGET ptrie::ptrie)
    message(STATUS "    Available target: ptrie::ptrie")
endif ()
