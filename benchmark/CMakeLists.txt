add_library(murmur OBJECT MurmurHash2.h MurmurHash2.cpp)

add_executable(benchmark benchmark.cpp utils.h binarywrapper.cpp)
target_link_libraries(benchmark PRIVATE ptrie murmur)
add_test(NAME bench_ptrie_10M COMMAND benchmark ptrie 10000000)
add_test(NAME bench_std_10M COMMAND benchmark std 10000000)
add_test(NAME bench_sparse_10M COMMAND benchmark sparse 10000000)
add_test(NAME bench_dense_10M COMMAND benchmark dense 10000000)

add_executable(int_benchmark int_benchmark.cpp utils.h binarywrapper.cpp)
target_link_libraries(int_benchmark PRIVATE ptrie murmur)

add_test(NAME ibench_ptrie_10M COMMAND int_benchmark ptrie 10000000)
add_test(NAME ibench_std_10M COMMAND int_benchmark std 10000000)
add_test(NAME ibench_sparse_10M COMMAND int_benchmark sparse 10000000)
add_test(NAME ibench_dense_10M COMMAND int_benchmark dense 10000000)

set_tests_properties(
        bench_ptrie_10M bench_std_10M bench_sparse_10M bench_dense_10M
        ibench_ptrie_10M ibench_std_10M ibench_sparse_10M ibench_dense_10M
        PROPERTIES RUN_SERIAL TRUE)