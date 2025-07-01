find_path(sparsehash_INCLUDE_DIR
    NAMES sparsehash/sparse_hash_set
    HINTS ${SPARSEHASH_ROOT})

if (sparsehash_INCLUDE_DIR AND NOT TARGET sparsehash::sparsehash)
   include(FindPackageHandleStandardArgs)
   find_package_handle_standard_args(sparsehash REQUIRED_VARS sparsehash_INCLUDE_DIR)
   mark_as_advanced(SPARSEHASH_FOUND SPARSEHASH_INCLUDE_DIR)
   message(STATUS "Found google-sparsehash: ${sparsehash_INCLUDE_DIR}")
   add_library(sparsehash::sparsehash INTERFACE IMPORTED)
   set_target_properties(sparsehash::sparsehash PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${sparsehash_INCLUDE_DIR}")
else()
   message(WARNING "Failed to find google-sparsehash")
endif()

unset(sparsehash_INCLUDE_DIR)
