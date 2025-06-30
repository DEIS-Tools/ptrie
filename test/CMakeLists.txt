cmake_policy(SET CMP0167 NEW)
find_package (Boost 1.83 REQUIRED COMPONENTS unit_test_framework)

add_executable (Delete delete.cpp)
add_executable (Set set.cpp)
add_executable (Map map.cpp)
add_executable (StableSet stable_set.cpp)

target_link_libraries(Set       PRIVATE ptrie Boost::unit_test_framework)
target_link_libraries(Delete    PRIVATE ptrie Boost::unit_test_framework)
target_link_libraries(StableSet PRIVATE ptrie Boost::unit_test_framework)
target_link_libraries(Map       PRIVATE ptrie Boost::unit_test_framework)

add_test(NAME Set       COMMAND Set)
add_test(NAME Delete    COMMAND Delete)
add_test(NAME StableSet COMMAND StableSet)
add_test(NAME Map       COMMAND Map)
