add_library(murmur OBJECT MurmurHash2.h MurmurHash2.cpp)
target_compile_features(murmur PRIVATE cxx_std_17)

add_library(binarywrapper OBJECT binarywrapper.h binarywrapper.cpp)
target_compile_features(binarywrapper PUBLIC cxx_std_17)
target_link_libraries(binarywrapper PUBLIC sparsehash::sparsehash)

add_executable(ptrie_insert_bm ptrie_insert_bm.cpp)
target_link_libraries(ptrie_insert_bm PRIVATE binarywrapper ptrie murmur benchmark::benchmark_main)
add_benchmark(ptrie_insert_bm --logx --logy)

add_executable(int_ptrie_insert_bm int_ptrie_insert_bm.cpp)
target_link_libraries(int_ptrie_insert_bm PRIVATE binarywrapper ptrie murmur benchmark::benchmark_main)
add_benchmark(int_ptrie_insert_bm --logx --logy)

add_executable(ptrie_insert ptrie_insert.cpp utils.h)
target_link_libraries(ptrie_insert PRIVATE binarywrapper ptrie murmur)

set(ELEMENT_COUNT 1000000)
add_test(NAME ptrie COMMAND ptrie_insert ptrie ${ELEMENT_COUNT})
add_test(NAME std COMMAND ptrie_insert std ${ELEMENT_COUNT})
add_test(NAME sparse COMMAND ptrie_insert sparse ${ELEMENT_COUNT})
add_test(NAME dense COMMAND ptrie_insert dense ${ELEMENT_COUNT})

add_executable(int_ptrie_insert int_ptrie_insert.cpp utils.h binarywrapper.cpp)
target_link_libraries(int_ptrie_insert PRIVATE sparsehash::sparsehash ptrie murmur)

add_test(NAME int_ptrie COMMAND int_ptrie_insert ptrie ${ELEMENT_COUNT})
add_test(NAME int_stable COMMAND int_ptrie_insert ptrie-stable ${ELEMENT_COUNT})
add_test(NAME int_map COMMAND int_ptrie_insert ptrie-map ${ELEMENT_COUNT})
add_test(NAME int_std COMMAND int_ptrie_insert std ${ELEMENT_COUNT})
add_test(NAME int_redblack COMMAND int_ptrie_insert redblack ${ELEMENT_COUNT})
add_test(NAME int_sparse COMMAND int_ptrie_insert sparse ${ELEMENT_COUNT})
add_test(NAME int_dense COMMAND int_ptrie_insert dense ${ELEMENT_COUNT})

set_tests_properties(ptrie std sparse dense int_ptrie int_std int_sparse int_dense PROPERTIES RUN_SERIAL TRUE)
