#include <gtest/gtest.h>

#include <ds/list.h>

TEST(List, init)
{
    LinkedList* list = list_init();
    EXPECT_NE(list, nullptr);
    EXPECT_EQ(list->size, 0u);
    EXPECT_EQ(list->head, nullptr);
    list_free(list);
}

TEST(List, push)
{
    LinkedList* list = list_init();
    int a = 10, b = 20, c = 30;
    list_push(list, a);
    list_push(list, b);
    list_push(list, c);

    EXPECT_EQ(list_len(list), 3u);
    EXPECT_EQ(list_get(list, 0, int), 10);
    EXPECT_EQ(list_get(list, 1, int), 20);
    EXPECT_EQ(list_get(list, 2, int), 30);

    list_free(list);
}

TEST(List, get_out_of_bounds)
{
    LinkedList* list = list_init();
    int a = 42;
    list_push(list, a);
    EXPECT_DEATH((void)list_get(list, 1, int), "index 1 out of bounds");
    list_free(list);
}

TEST(List, remove)
{
    LinkedList* list = list_init();
    int a = 1, b = 2, c = 3;
    list_push(list, a);
    list_push(list, b);
    list_push(list, c);

    EXPECT_EQ(list_remove(list, 1, int), 2);
    EXPECT_EQ(list_len(list), 2u);
    EXPECT_EQ(list_get(list, 0, int), 1);
    EXPECT_EQ(list_get(list, 1, int), 3);

    list_free(list);
}

TEST(List, iteration)
{
    LinkedList* list = list_init();
    for (int i = 0; i < 10; i++) {
        list_push(list, i);
    }

    int expected = 0;
    for (int i = 0; i < list_len(list); i++) {
        EXPECT_EQ(list_get(list, i, int), expected++);
    }

    list_free(list);
}

TEST(List, push_unsafe)
{
    LinkedList* list = list_init();
    int a = 10, b = 20, c = 30;
    list_push_unsafe(list, &a);
    list_push_unsafe(list, &b);
    list_push_unsafe(list, &c);

    EXPECT_EQ(list_len(list), 3u);
    EXPECT_EQ(list_get(list, 0, int), 10);
    EXPECT_EQ(list_get(list, 1, int), 20);
    EXPECT_EQ(list_get(list, 2, int), 30);

    list_free(list);
}

TEST(List, structs)
{
    struct Point {
        int x, y;
    };

    LinkedList* list = list_init();
    struct Point p1 = {1, 2};
    struct Point p2 = {3, 4};
    struct Point p3 = {5, 6};

    list_push(list, p1);
    list_push(list, p2);
    list_push(list, p3);
    EXPECT_EQ(list_len(list), 3u);
    EXPECT_EQ(list_get(list, 0, struct Point).x, 1);
    EXPECT_EQ(list_get(list, 0, struct Point).y, 2);
    EXPECT_EQ(list_get(list, 1, struct Point).x, 3);
    EXPECT_EQ(list_get(list, 1, struct Point).y, 4);
    EXPECT_EQ(list_get(list, 2, struct Point).x, 5);
    EXPECT_EQ(list_get(list, 2, struct Point).y, 6);
}