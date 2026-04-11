#include <gtest/gtest.h>

#include <ds/hashtable.h>

TEST(HashTable, SetAndGetValue) {
    HashTable table;
    hash_table_init(&table, 0);

    int answer = 42;
    ASSERT_TRUE(hash_table_set(&table, "answer", &answer));

    int* found = hash_table_get_as(int, &table, "answer");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(*found, 42);
    EXPECT_EQ(hash_table_size(&table), 1u);

    hash_table_destroy(&table, NULL);
}

TEST(HashTable, ReplaceExistingKey) {
    HashTable table;
    hash_table_init(&table, 0);

    int v1 = 1;
    int v2 = 2;

    ASSERT_TRUE(hash_table_set(&table, "k", &v1));
    ASSERT_TRUE(hash_table_set(&table, "k", &v2));

    int* found = hash_table_get_as(int, &table, "k");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(*found, 2);
    EXPECT_EQ(hash_table_size(&table), 1u);

    hash_table_destroy(&table, NULL);
}

TEST(HashTable, RemoveKey) {
    HashTable table;
    hash_table_init(&table, 0);

    int value = 7;
    ASSERT_TRUE(hash_table_set(&table, "tmp", &value));
    ASSERT_TRUE(hash_table_remove(&table, "tmp", NULL));

    EXPECT_EQ(hash_table_get(&table, "tmp"), nullptr);
    EXPECT_EQ(hash_table_size(&table), 0u);

    hash_table_destroy(&table, NULL);
}

TEST(HashTable, RehashesAsItGrows) {
    HashTable table;
    hash_table_init(&table, 8);

    int values[200];
    char key[32];

    for (int i = 0; i < 200; ++i) {
        values[i] = i;
        snprintf(key, sizeof(key), "key_%d", i);
        ASSERT_TRUE(hash_table_set(&table, key, &values[i]));
    }

    EXPECT_EQ(hash_table_size(&table), 200u);

    for (int i = 0; i < 200; ++i) {
        snprintf(key, sizeof(key), "key_%d", i);
        int* found = hash_table_get_as(int, &table, key);
        ASSERT_NE(found, nullptr);
        EXPECT_EQ(*found, i);
    }

    hash_table_destroy(&table, NULL);
}
