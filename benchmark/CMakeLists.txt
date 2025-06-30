add_library(murmur OBJECT MurmurHash2.h MurmurHash2.cpp)
add_library(binarywrapper OBJECT binarywrapper.h binarywrapper.cpp)

add_executable(ptrie_insert_bm ptrie_insert_bm.cpp)
target_link_libraries(ptrie_insert_bm PRIVATE binarywrapper ptrie murmur benchmark::benchmark_main)
add_test(NAME ptrie_insert_bm COMMAND ptrie_insert_bm --benchmark_out_format=json --benchmark_out=${CMAKE_CURRENT_BINARY_DIR}/ptrie_insert_bm.json)

add_executable(int_ptrie_insert_bm int_ptrie_insert_bm.cpp)
target_link_libraries(int_ptrie_insert_bm PRIVATE binarywrapper ptrie murmur benchmark::benchmark_main)
add_test(NAME int_ptrie_insert_bm COMMAND int_ptrie_insert_bm --benchmark_out_format=json --benchmark_out=${CMAKE_CURRENT_BINARY_DIR}/int_ptrie_insert_bm.json)

add_executable(ptrie_insert ptrie_insert.cpp utils.h)
target_link_libraries(ptrie_insert PRIVATE binarywrapper ptrie murmur)

set(ELEMENT_COUNT 1000000)
add_test(NAME ptrie COMMAND ptrie_insert ptrie ${ELEMENT_COUNT})
add_test(NAME std COMMAND ptrie_insert std ${ELEMENT_COUNT})
add_test(NAME sparse COMMAND ptrie_insert sparse ${ELEMENT_COUNT})
add_test(NAME dense COMMAND ptrie_insert dense ${ELEMENT_COUNT})

add_executable(int_ptrie_insert int_ptrie_insert.cpp utils.h binarywrapper.cpp)
target_link_libraries(int_ptrie_insert PRIVATE ptrie murmur)

add_test(NAME int_ptrie COMMAND int_ptrie_insert ptrie ${ELEMENT_COUNT})
add_test(NAME int_stable COMMAND int_ptrie_insert ptrie-stable ${ELEMENT_COUNT})
add_test(NAME int_map COMMAND int_ptrie_insert ptrie-map ${ELEMENT_COUNT})
add_test(NAME int_std COMMAND int_ptrie_insert std ${ELEMENT_COUNT})
add_test(NAME int_redblack COMMAND int_ptrie_insert redblack ${ELEMENT_COUNT})
add_test(NAME int_sparse COMMAND int_ptrie_insert sparse ${ELEMENT_COUNT})
add_test(NAME int_dense COMMAND int_ptrie_insert dense ${ELEMENT_COUNT})

set_tests_properties(
        ptrie_insert_bm int_ptrie_insert_bm
        ptrie std sparse dense int_ptrie int_std int_sparse int_dense
        PROPERTIES RUN_SERIAL TRUE)

if (python_CMD)
    add_test(NAME int_ptrie_insert_plot COMMAND ${benchmark_plot} --title "int_ptrie_insert_bm" --logx --logy ${CMAKE_CURRENT_BINARY_DIR}/int_ptrie_insert_bm.json)
    set_tests_properties(int_ptrie_insert_plot PROPERTIES DEPENDS int_ptrie_insert_bm)

    add_test(NAME ptrie_insert_plot COMMAND ${benchmark_plot} --title "ptrie_insert_bm" --logx --logy ${CMAKE_CURRENT_BINARY_DIR}/ptrie_insert_bm.json)
    set_tests_properties(ptrie_insert_plot PROPERTIES DEPENDS ptrie_insert_bm)
endif (python_CMD)
