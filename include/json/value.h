#ifndef JSON_VALUE_H
#define JSON_VALUE_H

#include <stdbool.h>
#include <stddef.h>

#include <ds/dynarr.h>
#include <ds/hashtable.h>

typedef enum {
    JSON_TYPE_NULL = 0,
    JSON_TYPE_BOOL,
    JSON_TYPE_NUMBER,
    JSON_TYPE_STRING,
    JSON_TYPE_ARRAY,
    JSON_TYPE_OBJECT
} JsonType;

struct JsonValue;
typedef struct JsonValue JsonValue;

typedef struct {
    JsonValue** items;
} JsonArray;

static inline size_t json_array_len(const JsonArray* array) {
    if (!array || !array->items) {
        return 0;
    }
    return arr_len(array->items);
}

typedef struct {
    HashTable members;
} JsonObject;

struct JsonValue {
    JsonType type;
    union {
        bool boolean;
        double number;
        char* string;
        JsonArray array;
        JsonObject object;
    } as;
};

#endif
