#include <gtest/gtest.h>

#include <ds/array.h>

TEST(Array, PushIncreasesLength) {
    int* arr = array_init(int);

    array_push(arr, 10);
    array_push(arr, 20);
    array_push(arr, 30);
    EXPECT_EQ(array_len(arr), 3u);
    EXPECT_EQ(arr[0], 10);
    EXPECT_EQ(arr[1], 20);
    EXPECT_EQ(arr[2], 30);

    array_free(arr);
}

TEST(Array, PopReturnsLastElement) {
    int* arr = array_init(int);

    array_push(arr, 7);
    array_push(arr, 9);

    EXPECT_EQ(array_pop(arr), 9);
    EXPECT_EQ(array_len(arr), 1u);

    array_free(arr);
}

TEST(Array, RemoveShiftsElementsLeft) {
    int* arr = array_init(int);

    array_push(arr, 1);
    array_push(arr, 2);
    array_push(arr, 3);

    array_remove(arr, 1);

    EXPECT_EQ(array_len(arr), 2u);
    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[1], 3);

    array_free(arr);
}

TEST(Array, BoundsCheck) {
    int* arr = array_init(int);

    array_push(arr, 42);
    EXPECT_EQ(array_len(arr), 1u);
    EXPECT_DEATH(array_bounds_check(arr, 1), "index 1 out of bounds");
    EXPECT_DEATH(array_remove(arr, 1), "index 1 out of bounds");
    array_pop(arr);
    EXPECT_DEATH((void)array_pop(arr), "pop from empty array");

    array_free(arr);
}

TEST(Array, Resize) {
    int* arr = array_init(int);

    for (int i = 0; i < 1000; i++) {
        array_push(arr, i);
    }

    EXPECT_EQ(array_len(arr), 1000u);
    for (int i = 0; i < 1000; i++) {
        EXPECT_EQ(arr[i], i);
    }

    array_free(arr);
}

TEST(Array, FreeNull) {
    array_free(NULL);
}

TEST(Array, Nesting) {
    int** arr = array_init(int*);

    for (int i = 0; i < 10; i++) {
        int* inner = array_init(int);
        for (int j = 0; j < 5; j++) {
            array_push(inner, i * 10 + j);
        }
        array_push(arr, inner);
    }

    EXPECT_EQ(array_len(arr), 10u);
    for (int i = 0; i < 10; i++) {
        int* inner = arr[i];
        EXPECT_EQ(array_len(inner), 5u);
        for (int j = 0; j < 5; j++) {
            EXPECT_EQ(inner[j], i * 10 + j);
        }
        array_free(inner);
    }

    array_free(arr);
}

TEST(Array, structs) {
    typedef struct {
        int x;
        int y;
    } Point;

    Point* arr = array_init(Point);

    for (int i = 0; i < 5; i++) {
        Point p = {i, i * 2};
        array_push(arr, p);
    }

    EXPECT_EQ(array_len(arr), 5u);
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(arr[i].x, i);
        EXPECT_EQ(arr[i].y, i * 2);
    }

    array_free(arr);
}