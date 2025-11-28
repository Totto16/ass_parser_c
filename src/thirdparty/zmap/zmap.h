/*
 * zmap.h
 *
 * This file is part of the z-libs collection: https://github.com/z-libs
 * Licensed under the MIT License.
 * 
 * based on https://github.com/z-libs/zmap.h/blob/743b276cfccf65b8f4a07aa1175c12fc5359b525/zmap.h
 * 
 * modified to suit my needs
 * 
 * Modifications by: Totto16
 */

#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Return Codes.
typedef enum : bool {
    ZvecResultErr = false,
    ZvecResultOk = true,
} ZvecResult;

// Memory Macros.
// If the user hasn't defined their own allocator, use the standard one.
#ifndef Z_MALLOC
    #define Z_MALLOC(sz)       malloc(sz)
    #define Z_CALLOC(n, sz)    calloc(n, sz)
    #define Z_REALLOC(p, sz)   realloc(p, sz)
    #define Z_FREE(p)          free(p)
#endif

// Compiler Extensions (Optional).
// We check for GCC/Clang features to enable RAII-style cleanup.
// Define Z_NO_EXTENSIONS to disable this manually.
#if !defined(Z_NO_EXTENSIONS) && (defined(__GNUC__) || defined(__clang__))
    #define Z_HAS_CLEANUP 1
    #define Z_CLEANUP(func) __attribute__((cleanup(func)))
#else
    #define Z_HAS_CLEANUP 0
    #define Z_CLEANUP(func) 
#endif


#ifndef Z_MAP_MALLOC
    #define Z_MAP_MALLOC(sz)      Z_MALLOC(sz)
#endif
#ifndef Z_MAP_CALLOC
    #define Z_MAP_CALLOC(n, sz)   Z_CALLOC(n, sz)
#endif
#ifndef Z_MAP_FREE
    #define Z_MAP_FREE(p)         Z_FREE(p)
#endif


// maybe some visibility things later, but I just removed the ZMAP_FUN_ATTRIBUTES
#define ZMAP_FUN_ATTRIBUTES 

ZMAP_FUN_ATTRIBUTES uint32_t zmap_default_hash(const void *key, size_t len);

ZMAP_FUN_ATTRIBUTES uint32_t zmap_stbds_hash_string(char* str);

ZMAP_FUN_ATTRIBUTES uint32_t zmap_stbds_hash_bytes(const void *key, size_t len);


typedef enum 
{
    ZMAP_EMPTY = 0,
    ZMAP_OCCUPIED,
    ZMAP_DELETED
} zmap_state;

#define ZMAP_HASH_SCALAR(k)          zmap_default_hash(&(k), sizeof(k))
#define ZMAP_HASH_STR(k)             zmap_stbds_hash_string(k)
#define ZMAP_HASH_BYTES(start, len)  zmap_stbds_hash_bytes((start), len)

#define ZMAP_TYPENAME_BUCKET(TypeName) zmap_bucket_##Name

#define ZMAP_TYPENAME_MAP(TypeName) zmap_##Name

#define ZMAP_DEFINE_MAP_TYPE(KeyT, ValT, Name)                                                                       \
                                                                                                                \
typedef struct {                                                                                                \
    KeyT key;                                                                                                   \
    ValT value;                                                                                                 \
    zmap_state state;                                                                                          \
} ZMAP_TYPENAME_BUCKET(Name);                                                                                           \
                                                                                                                \
typedef struct {                                                                                                \
    ZMAP_TYPENAME_BUCKET(Name) *buckets;                                                                                \
    size_t capacity;                                                                                            \
    size_t count;                                                                                               \
    size_t occupied;                                                                                            \
    uint32_t (*hash_func)(KeyT);                                                                                \
    int      (*cmp_func)(KeyT, KeyT);                                                                           \
} ZMAP_TYPENAME_MAP(Name);                                                                                                   \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES void zmap_free_##Name(ZMAP_TYPENAME_MAP(Name) *m) ;                                          \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES ZMAP_TYPENAME_MAP(Name) zmap_init_##Name(uint32_t (*h)(KeyT), int (*c)(KeyT, KeyT));        \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES ZvecResult zmap_resize_##Name(ZMAP_TYPENAME_MAP(Name) *m, size_t new_cap);                   \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES ZvecResult zmap_put_##Name(ZMAP_TYPENAME_MAP(Name) *m, KeyT key, ValT val);                 \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES ValT* zmap_get_##Name(ZMAP_TYPENAME_MAP(Name) *m, KeyT key);                                \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES void zmap_remove_##Name(ZMAP_TYPENAME_MAP(Name) *m, KeyT key)                               \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES size_t zmap_size_##Name(ZMAP_TYPENAME_MAP(Name) *m);                                       \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES void zmap_clear_##Name(ZMAP_TYPENAME_MAP(Name) *m);

#define M_PUT_ENTRY(K, V, N)    zmap_##N*: zmap_put_##N,
#define M_GET_ENTRY(K, V, N)    zmap_##N*: zmap_get_##N,
#define M_REM_ENTRY(K, V, N)    zmap_##N*: zmap_remove_##N,
#define M_FREE_ENTRY(K, V, N)   zmap_##N*: zmap_free_##N,
#define M_SIZE_ENTRY(K, V, N)   zmap_##N*: zmap_size_##N,
#define M_CLEAR_ENTRY(K, V, N)  zmap_##N*: zmap_clear_##N,

#define zmap_init(Name, h_func, c_func) zmap_init_##Name(h_func, c_func)

#if defined(Z_HAS_CLEANUP) && Z_HAS_CLEANUP
    #define zmap_autofree(Name)  Z_CLEANUP(map_free_##Name) ZMAP_TYPENAME_MAP(Name)
#endif

#define zmap_put(m, k, v)   _Generic((m), REGISTER_MAP_TYPES(M_PUT_ENTRY)  default: 0) (m, k, v)
#define zmap_get(m, k)      _Generic((m), REGISTER_MAP_TYPES(M_GET_ENTRY)  default: (void*)0) (m, k)
#define zmap_remove(m, k)   _Generic((m), REGISTER_MAP_TYPES(M_REM_ENTRY)  default: (void)0) (m, k)
#define zmap_free(m)        _Generic((m), REGISTER_MAP_TYPES(M_FREE_ENTRY) default: (void)0) (m)
#define zmap_size(m)        _Generic((m), REGISTER_MAP_TYPES(M_SIZE_ENTRY) default: 0) (m)
#define zmap_clear(m)       _Generic((m), REGISTER_MAP_TYPES(M_CLEAR_ENTRY) default: (void)0) (m)



#define ZMAP_IMPLEMENT_MAP_TYPE(KeyT, ValT, Name)                                                                       \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES void zmap_free_##Name(ZMAP_TYPENAME_MAP(Name) *m) {                                                             \
    Z_MAP_FREE(m->buckets);                                                                                     \
    *m = (ZMAP_TYPENAME_MAP(Name)){0};                                                                                       \
}                                                                                                               \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES ZMAP_TYPENAME_MAP(Name) zmap_init_##Name(uint32_t (*h)(KeyT), int (*c)(KeyT, KeyT)) {                           \
    return (ZMAP_TYPENAME_MAP(Name)){ .hash_func = h, .cmp_func = c };                                                       \
}                                                                                                               \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES ZvecResult zmap_resize_##Name(ZMAP_TYPENAME_MAP(Name) *m, size_t new_cap) {                                            \
    ZMAP_TYPENAME_BUCKET(Name) *new_buckets = Z_MAP_CALLOC(new_cap, sizeof(ZMAP_TYPENAME_BUCKET(Name)));                        \
    if (!new_buckets) { return ZvecResultErr };                                                                             \
                                                                                                                \
    for (size_t i = 0; i < m->capacity; i++) {                                                                  \
            if (m->buckets[i].state == ZMAP_OCCUPIED) {                                                         \
                uint32_t hash = m->hash_func(m->buckets[i].key);                                                \
                size_t idx = hash % new_cap;                                                                    \
                while (new_buckets[idx].state == ZMAP_OCCUPIED) {                                               \
                    idx = (idx + 1) % new_cap;                                                                  \
                }                                                                                               \
                new_buckets[idx] = m->buckets[i];                                                               \
            }                                                                                                   \
        }                                                                                                       \
        Z_MAP_FREE(m->buckets);                                                                                 \
        m->buckets = new_buckets;                                                                               \
        m->capacity = new_cap;                                                                                  \
        m->occupied = m->count;                                                                                 \
        return ZvecResultOk;                                                                                            \
}                                                                                                               \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES ZvecResult zmap_put_##Name(ZMAP_TYPENAME_MAP(Name) *m, KeyT key, ValT val) {                                           \
        if (m->occupied >= m->capacity * 0.75) {                                                                \
            size_t new_cap = m->capacity == 0 ? 16 : m->capacity * 2;                                           \
            if (map_resize_##Name(m, new_cap) != ZvecResultOk) { return ZvecResultErr; }                                            \
        }                                                                                                       \
        uint32_t hash = m->hash_func(key);                                                                      \
        size_t idx = hash % m->capacity;                                                                        \
        size_t deleted_idx = SIZE_MAX;                                                                          \
                                                                                                                \
        for (size_t i = 0; i < m->capacity; i++) {                                                              \
            zmap_state s = m->buckets[idx].state;                                                               \
            if (s == ZMAP_EMPTY) {                                                                              \
                if (deleted_idx != SIZE_MAX) idx = deleted_idx;                                                 \
                else m->occupied++;                                                                             \
                m->buckets[idx] = (ZMAP_TYPENAME_BUCKET(Name)){ .key = key, .value = val, .state = ZMAP_OCCUPIED };     \
                m->count++;                                                                                     \
                return ZvecResultOk;                                                                                    \
            }                                                                                                   \
            if (s == ZMAP_DELETED) {                                                                            \
                if (deleted_idx == SIZE_MAX) deleted_idx = idx;                                                 \
            }                                                                                                   \
            else if (m->cmp_func(m->buckets[idx].key, key) == 0) {                                              \
                m->buckets[idx].value = val;                                                                    \
                return ZvecResultOk;                                                                                    \
            }                                                                                                   \
            idx = (idx + 1) % m->capacity;                                                                      \
        }                                                                                                       \
        return ZvecResultErr;                                                                                           \
    }                                                                                                           \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES ValT* zmap_get_##Name(ZMAP_TYPENAME_MAP(Name) *m, KeyT key) {                                               \
        if (m->count == 0) return NULL;                                                                         \
        uint32_t hash = m->hash_func(key);                                                                      \
        size_t idx = hash % m->capacity;                                                                        \
                                                                                                                \
        for (size_t i = 0; i < m->capacity; i++) {                                                              \
            zmap_state s = m->buckets[idx].state;                                                               \
            if (s == ZMAP_EMPTY) return NULL;                                                                   \
            if (s == ZMAP_OCCUPIED && m->cmp_func(m->buckets[idx].key, key) == 0) {                             \
                return &m->buckets[idx].value;                                                                  \
            }                                                                                                   \
            idx = (idx + 1) % m->capacity;                                                                      \
        }                                                                                                       \
        return NULL;                                                                                            \
    }                                                                                                           \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES void zmap_remove_##Name(ZMAP_TYPENAME_MAP(Name) *m, KeyT key) {                                             \
        if (m->count == 0) return;                                                                              \
        uint32_t hash = m->hash_func(key);                                                                      \
        size_t idx = hash % m->capacity;                                                                        \
                                                                                                                \
        for (size_t i = 0; i < m->capacity; i++) {                                                              \
            zmap_state s = m->buckets[idx].state;                                                               \
            if (s == ZMAP_EMPTY) return;                                                                        \
            if (s == ZMAP_OCCUPIED && m->cmp_func(m->buckets[idx].key, key) == 0) {                             \
                m->buckets[idx].state = ZMAP_DELETED;                                                           \
                m->count--;                                                                                     \
                return;                                                                                         \
            }                                                                                                   \
            idx = (idx + 1) % m->capacity;                                                                      \
        }                                                                                                       \
    }                                                                                                           \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES size_t zmap_size_##Name(ZMAP_TYPENAME_MAP(Name) *m) { return m->count; }                                    \
                                                                                                                \
ZMAP_FUN_ATTRIBUTES void zmap_clear_##Name(ZMAP_TYPENAME_MAP(Name) *m) {                                                        \
        if (m->capacity > 0) {                                                                                  \
             memset(m->buckets, 0, m->capacity * sizeof(ZMAP_TYPENAME_BUCKET(Name)));                                   \
        }                                                                                                       \
        m->count = 0;                                                                                           \
        m->occupied = 0;                                                                                        \
    }
