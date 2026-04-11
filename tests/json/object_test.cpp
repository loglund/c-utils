#include <gtest/gtest.h>

#include <json/object.h>

TEST(JsonObject, SetGetAndSize) {
    JsonObject object;
    json_object_init(&object);

    JsonValue value;
    value.type = JSON_TYPE_NUMBER;
    value.as.number = 3.14;

    ASSERT_TRUE(json_object_set(&object, "pi", &value));
    EXPECT_EQ(json_object_size(&object), 1u);

    JsonValue* found = json_object_get(&object, "pi");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->type, JSON_TYPE_NUMBER);
    EXPECT_DOUBLE_EQ(found->as.number, 3.14);

    json_object_destroy(&object);
}

TEST(JsonObject, ReplaceAndRemove) {
    JsonObject object;
    json_object_init(&object);

    JsonValue old_value;
    old_value.type = JSON_TYPE_BOOL;
    old_value.as.boolean = false;

    JsonValue new_value;
    new_value.type = JSON_TYPE_BOOL;
    new_value.as.boolean = true;

    ASSERT_TRUE(json_object_set(&object, "flag", &old_value));
    ASSERT_TRUE(json_object_set(&object, "flag", &new_value));

    JsonValue* found = json_object_get(&object, "flag");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->type, JSON_TYPE_BOOL);
    EXPECT_TRUE(found->as.boolean);
    EXPECT_EQ(json_object_size(&object), 1u);

    ASSERT_TRUE(json_object_remove(&object, "flag"));
    EXPECT_EQ(json_object_get(&object, "flag"), nullptr);
    EXPECT_EQ(json_object_size(&object), 0u);

    json_object_destroy(&object);
}
