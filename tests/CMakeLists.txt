add_executable(data_structures_ds_tests
    ds/dynarr_test.cpp
    ds/hashtable_test.cpp
)

target_link_libraries(data_structures_ds_tests
    PRIVATE
    c-utils::c-utils
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(data_structures_ds_tests)

add_executable(data_structures_json_tests
    json/object_test.cpp
    json/codec_test.cpp
)

target_link_libraries(data_structures_json_tests
    PRIVATE
    c-utils::c-utils
        GTest::gtest_main
)

target_compile_definitions(data_structures_json_tests
    PRIVATE
    C_UTILS_TESTS_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

gtest_discover_tests(data_structures_json_tests)
