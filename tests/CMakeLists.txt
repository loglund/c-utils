add_executable(utils_ds_tests
    ds/array_test.cpp
    ds/list_test.cpp
    ds/deque_test.cpp
)

target_link_libraries(utils_ds_tests
    PRIVATE
    c-utils::c-utils
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(utils_ds_tests)
