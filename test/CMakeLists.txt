add_executable (Delete delete.cpp)
add_executable (Set set.cpp)
add_executable (Map map.cpp)
add_executable (StableSet stable_set.cpp)

target_link_libraries(Set       PRIVATE ptrie doctest::doctest_with_main)
target_link_libraries(Delete    PRIVATE ptrie doctest::doctest_with_main)
target_link_libraries(StableSet PRIVATE ptrie doctest::doctest_with_main)
target_link_libraries(Map       PRIVATE ptrie doctest::doctest_with_main)

add_test(NAME Set       COMMAND Set)
add_test(NAME Delete    COMMAND Delete)
add_test(NAME StableSet COMMAND StableSet)
add_test(NAME Map       COMMAND Map)
