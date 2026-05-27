add_executable(Delete delete.cpp)
add_executable(Set set.cpp)
add_executable(Map map.cpp)
add_executable(StableSet stable_set.cpp)

target_compile_options(Delete PRIVATE ${PTRIE_WARNINGS})
target_compile_options(Set PRIVATE ${PTRIE_WARNINGS})
target_compile_options(Map PRIVATE ${PTRIE_WARNINGS})
target_compile_options(StableSet PRIVATE ${PTRIE_WARNINGS})

target_enable_coverage(Delete)
target_enable_coverage(Set)
target_enable_coverage(Map)
target_enable_coverage(StableSet)

target_link_libraries(Set       PRIVATE ptrie doctest::doctest_with_main)
target_link_libraries(Delete    PRIVATE ptrie doctest::doctest_with_main)
target_link_libraries(StableSet PRIVATE ptrie doctest::doctest_with_main)
target_link_libraries(Map       PRIVATE ptrie doctest::doctest_with_main)

add_heaptrack_test(NAME Set       COMMAND Set)
add_heaptrack_test(NAME Delete    COMMAND Delete)
add_heaptrack_test(NAME StableSet COMMAND StableSet)
add_heaptrack_test(NAME Map       COMMAND Map)
