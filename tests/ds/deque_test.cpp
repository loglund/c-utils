#include <gtest/gtest.h>

#include <ds/deque.h>

TEST(Deque, init)
{
    Deque* deque = deque_init();
    EXPECT_NE(deque, nullptr);
    EXPECT_EQ(deque->size, 0u);
    EXPECT_EQ(deque->head, nullptr);
    EXPECT_EQ(deque->tail, nullptr);
    deque_free(deque);
}

TEST(Deque, push_back)
{
    Deque* deque = deque_init();
    int a = 10, b = 20, c = 30;
    deque_push_back(deque, a);
    deque_push_back(deque, b);
    deque_push_back(deque, c);

    EXPECT_EQ(deque_len(deque), 3u);
    EXPECT_EQ(deque_get(deque, 0, int), 10);
    EXPECT_EQ(deque_get(deque, 1, int), 20);
    EXPECT_EQ(deque_get(deque, 2, int), 30);

    deque_free(deque);
}

TEST(Deque, get_out_of_bounds)
{
    Deque* deque = deque_init();
    int a = 42;
    deque_push_back(deque, a);
    EXPECT_DEATH((void)deque_get(deque, 1, int), "index 1 out of bounds");
    deque_free(deque);
}

TEST(Deque, remove)
{
    Deque* deque = deque_init();
    int a = 1, b = 2, c = 3;
    deque_push_back(deque, a);
    deque_push_back(deque, b);
    deque_push_back(deque, c);

    EXPECT_EQ(deque_remove(deque, 1, int), 2);
    EXPECT_EQ(deque_len(deque), 2u);
    EXPECT_EQ(deque_remove(deque, 0, int), 1);
    EXPECT_EQ(deque_len(deque), 1u);
    EXPECT_EQ(deque_remove(deque, 0, int), 3);
    EXPECT_EQ(deque_len(deque), 0u);

    deque_free(deque);
}

TEST(Deque, iteration)
{
    Deque* deque = deque_init();
    for (int i = 0; i < 10; i++) {
        deque_push_back(deque, i);
    }

    EXPECT_EQ(deque_len(deque), 10u);
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(deque_get(deque, i, int), i);
    }

    deque_free(deque);
}

TEST(Deque, pop_back)
{
    Deque* deque = deque_init();
    int a = 5, b = 10, c = 15;
    deque_push_back(deque, a);
    deque_push_back(deque, b);
    deque_push_back(deque, c);

    EXPECT_EQ(deque_pop_back(deque, int), 15);
    EXPECT_EQ(deque_len(deque), 2u);
    EXPECT_EQ(deque_pop_back(deque, int), 10);
    EXPECT_EQ(deque_len(deque), 1u);
    EXPECT_EQ(deque_pop_back(deque, int), 5);
    EXPECT_EQ(deque_len(deque), 0u);

    deque_free(deque);
}

TEST(Deque, push_front)
{
    Deque* deque = deque_init();
    int a = 10, b = 20, c = 30;
    deque_push_front(deque, a);
    deque_push_front(deque, b);
    deque_push_front(deque, c);

    EXPECT_EQ(deque_len(deque), 3u);
    EXPECT_EQ(deque_get(deque, 0, int), 30);
    EXPECT_EQ(deque_get(deque, 1, int), 20);
    EXPECT_EQ(deque_get(deque, 2, int), 10);

    deque_free(deque);
}

TEST(Deque, pop_front)
{
    Deque* deque = deque_init();
    int a = 5, b = 10, c = 15;
    deque_push_back(deque, a);
    deque_push_back(deque, b);
    deque_push_back(deque, c);

    EXPECT_EQ(deque_pop_front(deque, int), 5);
    EXPECT_EQ(deque_len(deque), 2u);
    EXPECT_EQ(deque_pop_front(deque, int), 10);
    EXPECT_EQ(deque_len(deque), 1u);
    EXPECT_EQ(deque_pop_front(deque, int), 15);
    EXPECT_EQ(deque_len(deque), 0u);

    deque_free(deque);
}