#include <gtest/gtest.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <json/codec.h>

static char* read_text_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }

    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return NULL;
    }

    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }

    char* buf = (char*)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t nread = fread(buf, 1, (size_t)size, f);
    fclose(f);

    if (nread != (size_t)size) {
        free(buf);
        return NULL;
    }

    buf[size] = '\0';
    return buf;
}

static char* strip_json_ws_outside_strings(const char* src) {
    if (!src) {
        return NULL;
    }

    size_t len = strlen(src);
    char* out = (char*)malloc(len + 1);
    if (!out) {
        return NULL;
    }

    size_t j = 0;
    int in_string = 0;
    int escaped = 0;

    for (size_t i = 0; i < len; ++i) {
        char c = src[i];

        if (in_string) {
            out[j++] = c;
            if (escaped) {
                escaped = 0;
            } else if (c == '\\') {
                escaped = 1;
            } else if (c == '"') {
                in_string = 0;
            }
            continue;
        }

        if (c == '"') {
            in_string = 1;
            out[j++] = c;
            continue;
        }

        if (!isspace((unsigned char)c)) {
            out[j++] = c;
        }
    }

    out[j] = '\0';
    return out;
}

static bool json_values_equal(const JsonValue* a, const JsonValue* b) {
    if (a == b) {
        return true;
    }
    if (!a || !b) {
        return false;
    }
    if (a->type != b->type) {
        return false;
    }

    switch (a->type) {
        case JSON_TYPE_NULL:
            return true;
        case JSON_TYPE_BOOL:
            return a->as.boolean == b->as.boolean;
        case JSON_TYPE_NUMBER:
            return a->as.number == b->as.number;
        case JSON_TYPE_STRING:
            if (!a->as.string || !b->as.string) {
                return a->as.string == b->as.string;
            }
            return strcmp(a->as.string, b->as.string) == 0;
        case JSON_TYPE_ARRAY:
            if (json_array_len(&a->as.array) != json_array_len(&b->as.array)) {
                return false;
            }
            for (size_t i = 0; i < json_array_len(&a->as.array); ++i) {
                if (!json_values_equal(a->as.array.items[i], b->as.array.items[i])) {
                    return false;
                }
            }
            return true;
        case JSON_TYPE_OBJECT: {
            const HashTable* ta = &a->as.object.members;
            const HashTable* tb = &b->as.object.members;

            if (hash_table_size(ta) != hash_table_size(tb)) {
                return false;
            }

            for (size_t i = 0; i < ta->capacity; ++i) {
                const HashTableEntry* e = &ta->entries[i];
                if (e->state != HASH_TABLE_ENTRY_OCCUPIED) {
                    continue;
                }

                JsonValue* other = hash_table_get_as(JsonValue, tb, e->key);
                if (!other) {
                    return false;
                }
                if (!json_values_equal((const JsonValue*)e->value, other)) {
                    return false;
                }
            }
            return true;
        }
        default:
            return false;
    }
}

TEST(JsonCodec, ParseObjectWithEscapedStrings) {
    const char* src = "{\"name\":\"mhl\",\"escaped\":\"line1\\nline2\\t\\\"x\\\"\\\\\",\"ok\":true}";

    JsonValue* root = json_parse(src);
    ASSERT_NE(root, nullptr);
    ASSERT_EQ(root->type, JSON_TYPE_OBJECT);

    JsonValue* name = json_object_get(&root->as.object, "name");
    ASSERT_NE(name, nullptr);
    ASSERT_EQ(name->type, JSON_TYPE_STRING);
    EXPECT_STREQ(name->as.string, "mhl");

    JsonValue* escaped = json_object_get(&root->as.object, "escaped");
    ASSERT_NE(escaped, nullptr);
    ASSERT_EQ(escaped->type, JSON_TYPE_STRING);
    EXPECT_STREQ(escaped->as.string, "line1\nline2\t\"x\"\\");

    JsonValue* ok = json_object_get(&root->as.object, "ok");
    ASSERT_NE(ok, nullptr);
    ASSERT_EQ(ok->type, JSON_TYPE_BOOL);
    EXPECT_TRUE(ok->as.boolean);

    json_value_free(root);
}

TEST(JsonCodec, DumpAndParseRoundTripObject) {
    JsonValue* root = (JsonValue*)calloc(1, sizeof(JsonValue));
    ASSERT_NE(root, nullptr);
    root->type = JSON_TYPE_OBJECT;
    json_object_init(&root->as.object);

    JsonValue* s = (JsonValue*)calloc(1, sizeof(JsonValue));
    ASSERT_NE(s, nullptr);
    s->type = JSON_TYPE_STRING;
    s->as.string = (char*)malloc(16);
    ASSERT_NE(s->as.string, nullptr);
    strcpy(s->as.string, "hello\\nworld");

    JsonValue* n = (JsonValue*)calloc(1, sizeof(JsonValue));
    ASSERT_NE(n, nullptr);
    n->type = JSON_TYPE_NUMBER;
    n->as.number = 42.5;

    ASSERT_TRUE(json_object_set(&root->as.object, "message", s));
    ASSERT_TRUE(json_object_set(&root->as.object, "value", n));

    char* dumped = json_dump(root);
    ASSERT_NE(dumped, nullptr);

    JsonValue* reparsed = json_parse(dumped);
    ASSERT_NE(reparsed, nullptr);
    ASSERT_EQ(reparsed->type, JSON_TYPE_OBJECT);

    JsonValue* message = json_object_get(&reparsed->as.object, "message");
    ASSERT_NE(message, nullptr);
    ASSERT_EQ(message->type, JSON_TYPE_STRING);
    EXPECT_STREQ(message->as.string, "hello\\nworld");

    JsonValue* value = json_object_get(&reparsed->as.object, "value");
    ASSERT_NE(value, nullptr);
    ASSERT_EQ(value->type, JSON_TYPE_NUMBER);
    EXPECT_DOUBLE_EQ(value->as.number, 42.5);

    free(dumped);
    json_value_free(reparsed);
    json_value_free(root);
}

TEST(JsonCodec, ParseValidJsonFile) {
    const char* path = C_UTILS_TESTS_SOURCE_DIR "/json/files/valid_object.json";
    char* content = read_text_file(path);
    ASSERT_NE(content, nullptr);

    JsonValue* root = json_parse(content);
    ASSERT_NE(root, nullptr);
    ASSERT_EQ(root->type, JSON_TYPE_OBJECT);

    JsonValue* name = json_object_get(&root->as.object, "name");
    ASSERT_NE(name, nullptr);
    ASSERT_EQ(name->type, JSON_TYPE_STRING);
    EXPECT_STREQ(name->as.string, "data-structures");

    JsonValue* active = json_object_get(&root->as.object, "active");
    ASSERT_NE(active, nullptr);
    ASSERT_EQ(active->type, JSON_TYPE_BOOL);
    EXPECT_TRUE(active->as.boolean);

    JsonValue* meta = json_object_get(&root->as.object, "meta");
    ASSERT_NE(meta, nullptr);
    ASSERT_EQ(meta->type, JSON_TYPE_OBJECT);

    char* dumped = json_dump(root);
    ASSERT_NE(dumped, nullptr);

    char* file_compact = strip_json_ws_outside_strings(content);
    ASSERT_NE(file_compact, nullptr);
    EXPECT_STREQ(file_compact, dumped);

    JsonValue* reparsed = json_parse(dumped);
    ASSERT_NE(reparsed, nullptr);
    EXPECT_TRUE(json_values_equal(root, reparsed));

    free(file_compact);
    free(content);
    free(dumped);
    json_value_free(reparsed);
    json_value_free(root);
}

TEST(JsonCodec, ParseInvalidJsonFileFails) {
    const char* path = C_UTILS_TESTS_SOURCE_DIR "/json/files/invalid_object.json";
    char* content = read_text_file(path);
    ASSERT_NE(content, nullptr);

    JsonValue* root = json_parse(content);
    EXPECT_EQ(root, nullptr);
    free(content);
}
