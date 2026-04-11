#include <gtest/gtest.h>

#include <ds/dynarr.h>

TEST(DynArr, PushIncreasesLength) {
    int* arr = arr_init(int);

    arr_push(arr, 10);
    arr_push(arr, 20);
    arr_push(arr, 30);

    EXPECT_EQ(arr_len(arr), 3u);
    EXPECT_EQ(arr[0], 10);
    EXPECT_EQ(arr[1], 20);
    EXPECT_EQ(arr[2], 30);

    arr_free(arr);
}

TEST(DynArr, PopReturnsLastElement) {
    int* arr = arr_init(int);

    arr_push(arr, 7);
    arr_push(arr, 9);

    EXPECT_EQ(arr_pop(arr), 9);
    EXPECT_EQ(arr_len(arr), 1u);

    arr_free(arr);
}

TEST(DynArr, RemoveShiftsElementsLeft) {
    int* arr = arr_init(int);

    arr_push(arr, 1);
    arr_push(arr, 2);
    arr_push(arr, 3);

    arr_remove(arr, 1);

    EXPECT_EQ(arr_len(arr), 2u);
    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[1], 3);

    arr_free(arr);
}
