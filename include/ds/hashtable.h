#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef HASH_TABLE_INITIAL_CAPACITY
#define HASH_TABLE_INITIAL_CAPACITY 16u
#endif

#ifndef HASH_TABLE_MAX_LOAD_FACTOR
#define HASH_TABLE_MAX_LOAD_FACTOR 0.70
#endif

typedef enum {
    HASH_TABLE_ENTRY_EMPTY = 0,
    HASH_TABLE_ENTRY_OCCUPIED = 1,
    HASH_TABLE_ENTRY_TOMBSTONE = 2
} HashTableEntryState;

typedef struct {
    char* key;
    void* value;
    HashTableEntryState state;
} HashTableEntry;

typedef struct {
    size_t size;
    size_t tombstones;
    size_t capacity;
    HashTableEntry* entries;
} HashTable;

typedef void (*HashTableValueFreeFn)(void* value);

static inline uint64_t hash_djb2(const char* s) {
    uint64_t hash = 5381;
    unsigned char c;
    while ((c = (unsigned char)*s++) != '\0') {
        hash = ((hash << 5u) + hash) + (uint64_t)c;
    }
    return hash;
}

static inline char* hash_table_strdup_(const char* src) {
    size_t len = strlen(src);
    char* out = (char*)malloc(len + 1);
    if (!out) {
        return NULL;
    }
    memcpy(out, src, len + 1);
    return out;
}

static inline void hash_table_init(HashTable* table, size_t initial_capacity) {
    if (!table) {
        return;
    }

    if (initial_capacity == 0) {
        initial_capacity = HASH_TABLE_INITIAL_CAPACITY;
    }

    if (initial_capacity < 8) {
        initial_capacity = 8;
    }

    table->entries = (HashTableEntry*)calloc(initial_capacity, sizeof(HashTableEntry));
    table->size = 0;
    table->tombstones = 0;
    table->capacity = table->entries ? initial_capacity : 0;
}

static inline void hash_table_clear(HashTable* table, HashTableValueFreeFn free_value) {
    if (!table || !table->entries) {
        return;
    }

    for (size_t i = 0; i < table->capacity; ++i) {
        HashTableEntry* e = &table->entries[i];
        if (e->state == HASH_TABLE_ENTRY_OCCUPIED) {
            free(e->key);
            if (free_value) {
                free_value(e->value);
            }
            e->key = NULL;
            e->value = NULL;
            e->state = HASH_TABLE_ENTRY_EMPTY;
        } else if (e->state == HASH_TABLE_ENTRY_TOMBSTONE) {
            e->key = NULL;
            e->value = NULL;
            e->state = HASH_TABLE_ENTRY_EMPTY;
        }
    }

    table->size = 0;
    table->tombstones = 0;
}

static inline void hash_table_destroy(HashTable* table, HashTableValueFreeFn free_value) {
    if (!table) {
        return;
    }

    hash_table_clear(table, free_value);
    free(table->entries);
    table->entries = NULL;
    table->capacity = 0;
}

static inline size_t hash_table_probe_index_(const HashTable* table, const char* key, bool* found) {
    uint64_t hash = hash_djb2(key);
    size_t idx = (size_t)(hash % table->capacity);
    size_t first_tombstone = (size_t)-1;

    for (;;) {
        const HashTableEntry* e = &table->entries[idx];
        if (e->state == HASH_TABLE_ENTRY_EMPTY) {
            *found = false;
            return (first_tombstone != (size_t)-1) ? first_tombstone : idx;
        }

        if (e->state == HASH_TABLE_ENTRY_TOMBSTONE) {
            if (first_tombstone == (size_t)-1) {
                first_tombstone = idx;
            }
        } else if (strcmp(e->key, key) == 0) {
            *found = true;
            return idx;
        }

        idx = (idx + 1) % table->capacity;
    }
}

static inline bool hash_table_rehash_(HashTable* table, size_t new_capacity) {
    HashTableEntry* new_entries = (HashTableEntry*)calloc(new_capacity, sizeof(HashTableEntry));
    if (!new_entries) {
        return false;
    }

    HashTableEntry* old_entries = table->entries;
    size_t old_capacity = table->capacity;

    table->entries = new_entries;
    table->capacity = new_capacity;
    table->size = 0;
    table->tombstones = 0;

    for (size_t i = 0; i < old_capacity; ++i) {
        HashTableEntry* old = &old_entries[i];
        if (old->state == HASH_TABLE_ENTRY_OCCUPIED) {
            bool found = false;
            size_t idx = hash_table_probe_index_(table, old->key, &found);
            (void)found;
            table->entries[idx].key = old->key;
            table->entries[idx].value = old->value;
            table->entries[idx].state = HASH_TABLE_ENTRY_OCCUPIED;
            table->size++;
        }
    }

    free(old_entries);
    return true;
}

static inline bool hash_table_reserve_(HashTable* table) {
    if (!table || table->capacity == 0) {
        return false;
    }

    double load = (double)(table->size + table->tombstones + 1) / (double)table->capacity;
    if (load > HASH_TABLE_MAX_LOAD_FACTOR) {
        return hash_table_rehash_(table, table->capacity * 2);
    }

    if (table->tombstones > table->size && table->capacity > 16) {
        return hash_table_rehash_(table, table->capacity);
    }

    return true;
}

static inline bool hash_table_set(HashTable* table, const char* key, void* value) {
    if (!table || !key) {
        return false;
    }

    if (!table->entries) {
        hash_table_init(table, HASH_TABLE_INITIAL_CAPACITY);
        if (!table->entries) {
            return false;
        }
    }

    if (!hash_table_reserve_(table)) {
        return false;
    }

    bool found = false;
    size_t idx = hash_table_probe_index_(table, key, &found);
    HashTableEntry* e = &table->entries[idx];

    if (found) {
        e->value = value;
        return true;
    }

    char* key_copy = hash_table_strdup_(key);
    if (!key_copy) {
        return false;
    }

    if (e->state == HASH_TABLE_ENTRY_TOMBSTONE) {
        table->tombstones--;
    }

    e->key = key_copy;
    e->value = value;
    e->state = HASH_TABLE_ENTRY_OCCUPIED;
    table->size++;
    return true;
}

static inline void* hash_table_get(const HashTable* table, const char* key) {
    if (!table || !table->entries || !key || table->capacity == 0) {
        return NULL;
    }

    bool found = false;
    size_t idx = hash_table_probe_index_(table, key, &found);
    if (!found) {
        return NULL;
    }

    return table->entries[idx].value;
}

static inline bool hash_table_contains(const HashTable* table, const char* key) {
    return hash_table_get(table, key) != NULL;
}

static inline bool hash_table_remove(HashTable* table, const char* key, HashTableValueFreeFn free_value) {
    if (!table || !table->entries || !key || table->capacity == 0) {
        return false;
    }

    bool found = false;
    size_t idx = hash_table_probe_index_(table, key, &found);
    if (!found) {
        return false;
    }

    HashTableEntry* e = &table->entries[idx];
    free(e->key);
    if (free_value) {
        free_value(e->value);
    }
    e->key = NULL;
    e->value = NULL;
    e->state = HASH_TABLE_ENTRY_TOMBSTONE;

    table->size--;
    table->tombstones++;
    return true;
}

static inline size_t hash_table_size(const HashTable* table) {
    return table ? table->size : 0;
}

#define hash_table_get_as(type, table_ptr, key_str) ((type*)hash_table_get((table_ptr), (key_str)))

#endif
