#ifndef JSON_CODEC_H
#define JSON_CODEC_H

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <json/object.h>

static inline void json_value_free(JsonValue* value);

typedef struct {
    char* data;
    size_t len;
    size_t cap;
} JsonStringBuilder;

static inline bool json_sb_reserve_(JsonStringBuilder* sb, size_t needed_extra) {
    size_t needed = sb->len + needed_extra + 1;
    if (needed <= sb->cap) {
        return true;
    }

    size_t new_cap = (sb->cap == 0) ? 64 : sb->cap;
    while (new_cap < needed) {
        new_cap *= 2;
    }

    char* new_data = (char*)realloc(sb->data, new_cap);
    if (!new_data) {
        return false;
    }

    sb->data = new_data;
    sb->cap = new_cap;
    return true;
}

static inline bool json_sb_append_char_(JsonStringBuilder* sb, char c) {
    if (!json_sb_reserve_(sb, 1)) {
        return false;
    }
    sb->data[sb->len++] = c;
    sb->data[sb->len] = '\0';
    return true;
}

static inline bool json_sb_append_n_(JsonStringBuilder* sb, const char* s, size_t n) {
    if (!json_sb_reserve_(sb, n)) {
        return false;
    }
    memcpy(sb->data + sb->len, s, n);
    sb->len += n;
    sb->data[sb->len] = '\0';
    return true;
}

static inline bool json_sb_append_(JsonStringBuilder* sb, const char* s) {
    return json_sb_append_n_(sb, s, strlen(s));
}

static inline bool json_append_utf8_codepoint_(JsonStringBuilder* sb, uint32_t cp) {
    if (cp <= 0x7F) {
        return json_sb_append_char_(sb, (char)cp);
    }
    if (cp <= 0x7FF) {
        return json_sb_append_char_(sb, (char)(0xC0 | (cp >> 6))) &&
               json_sb_append_char_(sb, (char)(0x80 | (cp & 0x3F)));
    }
    if (cp <= 0xFFFF) {
        return json_sb_append_char_(sb, (char)(0xE0 | (cp >> 12))) &&
               json_sb_append_char_(sb, (char)(0x80 | ((cp >> 6) & 0x3F))) &&
               json_sb_append_char_(sb, (char)(0x80 | (cp & 0x3F)));
    }
    if (cp <= 0x10FFFF) {
        return json_sb_append_char_(sb, (char)(0xF0 | (cp >> 18))) &&
               json_sb_append_char_(sb, (char)(0x80 | ((cp >> 12) & 0x3F))) &&
               json_sb_append_char_(sb, (char)(0x80 | ((cp >> 6) & 0x3F))) &&
               json_sb_append_char_(sb, (char)(0x80 | (cp & 0x3F)));
    }
    return false;
}

static inline void json_skip_ws_(const char** p) {
    while (**p != '\0' && isspace((unsigned char)**p)) {
        (*p)++;
    }
}

static inline bool json_parse_hex4_(const char* p, uint32_t* out) {
    uint32_t v = 0;
    for (int i = 0; i < 4; ++i) {
        char c = p[i];
        v <<= 4;
        if (c >= '0' && c <= '9') {
            v |= (uint32_t)(c - '0');
        } else if (c >= 'a' && c <= 'f') {
            v |= (uint32_t)(10 + c - 'a');
        } else if (c >= 'A' && c <= 'F') {
            v |= (uint32_t)(10 + c - 'A');
        } else {
            return false;
        }
    }
    *out = v;
    return true;
}

static inline char* json_parse_string_raw_(const char** p) {
    if (**p != '"') {
        return NULL;
    }
    (*p)++;

    JsonStringBuilder sb = {0};

    while (**p != '\0') {
        unsigned char c = (unsigned char)**p;
        (*p)++;

        if (c == '"') {
            return sb.data ? sb.data : (char*)calloc(1, 1);
        }

        if (c == '\\') {
            char esc = **p;
            if (esc == '\0') {
                free(sb.data);
                return NULL;
            }
            (*p)++;
            switch (esc) {
                case '"': if (!json_sb_append_char_(&sb, '"')) goto fail; break;
                case '\\': if (!json_sb_append_char_(&sb, '\\')) goto fail; break;
                case '/': if (!json_sb_append_char_(&sb, '/')) goto fail; break;
                case 'b': if (!json_sb_append_char_(&sb, '\b')) goto fail; break;
                case 'f': if (!json_sb_append_char_(&sb, '\f')) goto fail; break;
                case 'n': if (!json_sb_append_char_(&sb, '\n')) goto fail; break;
                case 'r': if (!json_sb_append_char_(&sb, '\r')) goto fail; break;
                case 't': if (!json_sb_append_char_(&sb, '\t')) goto fail; break;
                case 'u': {
                    uint32_t cp = 0;
                    if (!json_parse_hex4_(*p, &cp)) {
                        goto fail;
                    }
                    *p += 4;
                    if (cp >= 0xD800 && cp <= 0xDBFF) {
                        if ((*p)[0] == '\\' && (*p)[1] == 'u') {
                            uint32_t low = 0;
                            if (!json_parse_hex4_((*p) + 2, &low)) {
                                goto fail;
                            }
                            if (low < 0xDC00 || low > 0xDFFF) {
                                goto fail;
                            }
                            *p += 6;
                            cp = 0x10000u + (((cp - 0xD800u) << 10u) | (low - 0xDC00u));
                        } else {
                            goto fail;
                        }
                    }
                    if (!json_append_utf8_codepoint_(&sb, cp)) {
                        goto fail;
                    }
                    break;
                }
                default:
                    goto fail;
            }
        } else {
            if (c < 0x20) {
                goto fail;
            }
            if (!json_sb_append_char_(&sb, (char)c)) {
                goto fail;
            }
        }
    }

fail:
    free(sb.data);
    return NULL;
}

static inline JsonValue* json_value_new_(JsonType type) {
    JsonValue* v = (JsonValue*)calloc(1, sizeof(JsonValue));
    if (!v) {
        return NULL;
    }
    v->type = type;
    return v;
}

static inline JsonValue* json_parse_value_(const char** p);

static inline JsonValue* json_parse_array_(const char** p) {
    if (**p != '[') {
        return NULL;
    }
    (*p)++;

    JsonValue* arr = json_value_new_(JSON_TYPE_ARRAY);
    if (!arr) {
        return NULL;
    }

    arr->as.array.items = arr_init(JsonValue*);
    if (!arr->as.array.items) {
        json_value_free(arr);
        return NULL;
    }

    json_skip_ws_(p);
    if (**p == ']') {
        (*p)++;
        return arr;
    }

    while (**p != '\0') {
        json_skip_ws_(p);
        JsonValue* item = json_parse_value_(p);
        if (!item) {
            json_value_free(arr);
            return NULL;
        }

        arr_push(arr->as.array.items, item);

        json_skip_ws_(p);
        if (**p == ',') {
            (*p)++;
            continue;
        }
        if (**p == ']') {
            (*p)++;
            return arr;
        }

        json_value_free(arr);
        return NULL;
    }

    json_value_free(arr);
    return NULL;
}

static inline JsonValue* json_parse_object_(const char** p) {
    if (**p != '{') {
        return NULL;
    }
    (*p)++;

    JsonValue* obj = json_value_new_(JSON_TYPE_OBJECT);
    if (!obj) {
        return NULL;
    }
    json_object_init(&obj->as.object);

    json_skip_ws_(p);
    if (**p == '}') {
        (*p)++;
        return obj;
    }

    while (**p != '\0') {
        json_skip_ws_(p);
        char* key = json_parse_string_raw_(p);
        if (!key) {
            json_value_free(obj);
            return NULL;
        }

        json_skip_ws_(p);
        if (**p != ':') {
            free(key);
            json_value_free(obj);
            return NULL;
        }
        (*p)++;

        json_skip_ws_(p);
        JsonValue* item = json_parse_value_(p);
        if (!item) {
            free(key);
            json_value_free(obj);
            return NULL;
        }

        if (!json_object_set(&obj->as.object, key, item)) {
            free(key);
            json_value_free(item);
            json_value_free(obj);
            return NULL;
        }
        free(key);

        json_skip_ws_(p);
        if (**p == ',') {
            (*p)++;
            continue;
        }
        if (**p == '}') {
            (*p)++;
            return obj;
        }

        json_value_free(obj);
        return NULL;
    }

    json_value_free(obj);
    return NULL;
}

static inline JsonValue* json_parse_string_(const char** p) {
    char* s = json_parse_string_raw_(p);
    if (!s) {
        return NULL;
    }

    JsonValue* v = json_value_new_(JSON_TYPE_STRING);
    if (!v) {
        free(s);
        return NULL;
    }
    v->as.string = s;
    return v;
}

static inline JsonValue* json_parse_number_(const char** p) {
    errno = 0;
    char* end_ptr = NULL;
    double n = strtod(*p, &end_ptr);
    if (end_ptr == *p || errno == ERANGE) {
        return NULL;
    }

    JsonValue* v = json_value_new_(JSON_TYPE_NUMBER);
    if (!v) {
        return NULL;
    }
    v->as.number = n;
    *p = end_ptr;
    return v;
}

static inline JsonValue* json_parse_value_(const char** p) {
    json_skip_ws_(p);

    if (**p == '"') {
        return json_parse_string_(p);
    }
    if (**p == '{') {
        return json_parse_object_(p);
    }
    if (**p == '[') {
        return json_parse_array_(p);
    }
    if (strncmp(*p, "true", 4) == 0) {
        JsonValue* v = json_value_new_(JSON_TYPE_BOOL);
        if (!v) return NULL;
        v->as.boolean = true;
        *p += 4;
        return v;
    }
    if (strncmp(*p, "false", 5) == 0) {
        JsonValue* v = json_value_new_(JSON_TYPE_BOOL);
        if (!v) return NULL;
        v->as.boolean = false;
        *p += 5;
        return v;
    }
    if (strncmp(*p, "null", 4) == 0) {
        JsonValue* v = json_value_new_(JSON_TYPE_NULL);
        if (!v) return NULL;
        *p += 4;
        return v;
    }

    if (**p == '-' || isdigit((unsigned char)**p)) {
        return json_parse_number_(p);
    }

    return NULL;
}

static inline JsonValue* json_parse(const char* json) {
    if (!json) {
        return NULL;
    }

    const char* p = json;
    JsonValue* v = json_parse_value_(&p);
    if (!v) {
        return NULL;
    }

    json_skip_ws_(&p);
    if (*p != '\0') {
        json_value_free(v);
        return NULL;
    }

    return v;
}

static inline bool json_dump_string_(JsonStringBuilder* sb, const char* s) {
    if (!json_sb_append_char_(sb, '"')) {
        return false;
    }

    for (const unsigned char* p = (const unsigned char*)s; *p != '\0'; ++p) {
        unsigned char c = *p;
        switch (c) {
            case '"': if (!json_sb_append_(sb, "\\\"")) return false; break;
            case '\\': if (!json_sb_append_(sb, "\\\\")) return false; break;
            case '\b': if (!json_sb_append_(sb, "\\b")) return false; break;
            case '\f': if (!json_sb_append_(sb, "\\f")) return false; break;
            case '\n': if (!json_sb_append_(sb, "\\n")) return false; break;
            case '\r': if (!json_sb_append_(sb, "\\r")) return false; break;
            case '\t': if (!json_sb_append_(sb, "\\t")) return false; break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04X", (unsigned)c);
                    if (!json_sb_append_(sb, buf)) {
                        return false;
                    }
                } else {
                    if (!json_sb_append_char_(sb, (char)c)) {
                        return false;
                    }
                }
        }
    }

    return json_sb_append_char_(sb, '"');
}

static inline bool json_dump_value_(JsonStringBuilder* sb, const JsonValue* v) {
    if (!v) {
        return json_sb_append_(sb, "null");
    }

    switch (v->type) {
        case JSON_TYPE_NULL:
            return json_sb_append_(sb, "null");
        case JSON_TYPE_BOOL:
            return json_sb_append_(sb, v->as.boolean ? "true" : "false");
        case JSON_TYPE_NUMBER: {
            char numbuf[64];
            int n = snprintf(numbuf, sizeof(numbuf), "%.17g", v->as.number);
            if (n <= 0) {
                return false;
            }
            return json_sb_append_n_(sb, numbuf, (size_t)n);
        }
        case JSON_TYPE_STRING:
            return json_dump_string_(sb, v->as.string ? v->as.string : "");
        case JSON_TYPE_ARRAY: {
            if (!json_sb_append_char_(sb, '[')) return false;
            size_t count = json_array_len(&v->as.array);
            for (size_t i = 0; i < count; ++i) {
                if (i > 0 && !json_sb_append_char_(sb, ',')) return false;
                if (!json_dump_value_(sb, v->as.array.items[i])) return false;
            }
            return json_sb_append_char_(sb, ']');
        }
        case JSON_TYPE_OBJECT: {
            if (!json_sb_append_char_(sb, '{')) return false;
            bool first = true;
            const HashTable* t = &v->as.object.members;
            for (size_t i = 0; i < t->capacity; ++i) {
                const HashTableEntry* e = &t->entries[i];
                if (e->state != HASH_TABLE_ENTRY_OCCUPIED) {
                    continue;
                }
                if (!first && !json_sb_append_char_(sb, ',')) return false;
                first = false;
                if (!json_dump_string_(sb, e->key)) return false;
                if (!json_sb_append_char_(sb, ':')) return false;
                if (!json_dump_value_(sb, (const JsonValue*)e->value)) return false;
            }
            return json_sb_append_char_(sb, '}');
        }
        default:
            return false;
    }
}

static inline char* json_dump(const JsonValue* value) {
    JsonStringBuilder sb = {0};
    if (!json_dump_value_(&sb, value)) {
        free(sb.data);
        return NULL;
    }
    return sb.data;
}

static inline void json_value_free(JsonValue* value) {
    if (!value) {
        return;
    }

    switch (value->type) {
        case JSON_TYPE_STRING:
            free(value->as.string);
            break;
        case JSON_TYPE_ARRAY:
            if (value->as.array.items) {
                size_t count = arr_len(value->as.array.items);
                for (size_t i = 0; i < count; ++i) {
                    json_value_free(value->as.array.items[i]);
                }
                arr_free(value->as.array.items);
            }
            break;
        case JSON_TYPE_OBJECT: {
            HashTable* t = &value->as.object.members;
            if (t->entries) {
                for (size_t i = 0; i < t->capacity; ++i) {
                    HashTableEntry* e = &t->entries[i];
                    if (e->state == HASH_TABLE_ENTRY_OCCUPIED) {
                        json_value_free((JsonValue*)e->value);
                    }
                }
            }
            hash_table_destroy(t, NULL);
            break;
        }
        default:
            break;
    }

    free(value);
}

#endif
