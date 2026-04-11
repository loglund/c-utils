#ifndef JSON_OBJECT_H
#define JSON_OBJECT_H

#include <stdbool.h>

#include <json/value.h>

static inline void json_object_init(JsonObject* object) {
    hash_table_init(&object->members, 0);
}

static inline void json_object_destroy(JsonObject* object) {
    hash_table_destroy(&object->members, NULL);
}

static inline bool json_object_set(JsonObject* object, const char* key, JsonValue* value) {
    return hash_table_set(&object->members, key, (void*)value);
}

static inline JsonValue* json_object_get(const JsonObject* object, const char* key) {
    return hash_table_get_as(JsonValue, &object->members, key);
}

static inline bool json_object_remove(JsonObject* object, const char* key) {
    return hash_table_remove(&object->members, key, NULL);
}

static inline size_t json_object_size(const JsonObject* object) {
    return hash_table_size(&object->members);
}

#endif
