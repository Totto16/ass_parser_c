/* zvec.h
    based on https://github.com/Zuhaitz-dev/zvec.h/blob/e015d0eadf07e1720866f732da3703de9bead051/zvec.h

    modified to suit my needs

By: Zuhaitz-dev

Modifications by: Totto16
*/

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef enum  : bool{
    ZvecResultErr = false,
    ZvecResultOk = true,
}ZvecResult;


// maybe some visibility things later, but i just removed the static inline
#define ZVEC_FUN_ATTRIBUTES 

#define ZVEC_TYPENAME(TypeName) zvec_##TypeName

#define ZVEC_DEFINE_VEC_TYPE(T)                                                             \
typedef struct {                                                                            \
    T *data;                                                                                \
    size_t length;                                                                          \
    size_t capacity;                                                                        \
} ZVEC_TYPENAME(T);                                                                            \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] ZVEC_TYPENAME(T) zvec_init_capacity_##T(size_t cap);                       \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] ZvecResult zvec_reserve_##T(ZVEC_TYPENAME(T) *v, size_t new_cap);                 \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] bool zvec_is_empty_##T(ZVEC_TYPENAME(T) v);                                \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] ZvecResult zvec_push_##T(ZVEC_TYPENAME(T) *v, T value);                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] ZvecResult zvec_extend_##T(ZVEC_TYPENAME(T) *v, const T *items, size_t count);    \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_pop_##T(ZVEC_TYPENAME(T) *v);                                    \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] T zvec_pop_get_##T(ZVEC_TYPENAME(T) *v);                                   \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_shrink_to_fit_##T(ZVEC_TYPENAME(T) *v);                          \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]]T zvec_at_##T(ZVEC_TYPENAME(T) v, size_t index);                          \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] T* zvec_get_at_mut_##T( ZVEC_TYPENAME(T) *v, size_t index);                 \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] const T* zvec_get_at_##T(const ZVEC_TYPENAME(T) *v, size_t index);                 \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] T* zvec_data_##T(ZVEC_TYPENAME(T) *v);                                     \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] T* zvec_last_##T(ZVEC_TYPENAME(T) *v);                                     \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_remove_##T(ZVEC_TYPENAME(T) *v, size_t index);                   \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_swap_remove_##T(ZVEC_TYPENAME(T) *v, size_t index);              \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_clear_##T(ZVEC_TYPENAME(T) *v);                                  \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_free_##T(ZVEC_TYPENAME(T) *v);                                   \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_reverse_##T(ZVEC_TYPENAME(T) *v);                                \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_sort_##T(ZVEC_TYPENAME(T) *v,                                    \
                                                int (*compar)(const T *, const T *));       \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] T* zvec_bsearch_##T(ZVEC_TYPENAME(T) *v, const void *key,                  \
                                 int (*compar)(const T *, const T *));

#define ZVEC_EMPTY(TypeName) ((ZVEC_TYPENAME(TypeName)){.data=NULL, .length=0, .capacity=0})

#define ZVEC_LENGTH(v) (v).length

#define ZVEC_PUSH_ENTRY(T)       ZVEC_TYPENAME(T)*: zvec_push_##T,
#define ZVEC_EXTEND_ENTRY(T)     ZVEC_TYPENAME(T)*: zvec_extend_##T,
#define ZVEC_RESERVE_ENTRY(T)    ZVEC_TYPENAME(T)*: zvec_reserve_##T,
#define ZVEC_IS_EMPTY_ENTRY(T)   ZVEC_TYPENAME(T)*: zvec_is_empty_##T,
#define ZVEC_AT_ENTRY(T)         ZVEC_TYPENAME(T)*: zvec_at_##T,
#define ZVEC_GET_AT_ENTRY(T)     ZVEC_TYPENAME(T)*: zvec_get_at_##T,
#define ZVEC_GET_AT_MUT_ENTRY(T) ZVEC_TYPENAME(T)*: zvec_get_at_mut_##T,
#define ZVEC_DATA_ENTRY(T)       ZVEC_TYPENAME(T)*: zvec_data_##T,
#define ZVEC_LAST_ENTRY(T)       ZVEC_TYPENAME(T)*: zvec_last_##T,
#define ZVEC_FREE_ENTRY(T)       ZVEC_TYPENAME(T)*: zvec_free_##T,
#define ZVEC_POP_ENTRY(T)        ZVEC_TYPENAME(T)*: zvec_pop_##T,
#define ZVEC_POP_GET_ENTRY(T)    ZVEC_TYPENAME(T)*: zvec_pop_get_##T,
#define ZVEC_SHRINK_ENTRY(T)     ZVEC_TYPENAME(T)*: zvec_shrink_to_fit_##T,
#define ZVEC_REMOVE_ENTRY(T)     ZVEC_TYPENAME(T)*: zvec_remove_##T,
#define ZVEC_SWAP_REM_ENTRY(T)   ZVEC_TYPENAME(T)*: zvec_swap_remove_##T,
#define ZVEC_CLEAR_ENTRY(T)      ZVEC_TYPENAME(T)*: zvec_clear_##T,
#define ZVEC_REVERSE_ENTRY(T)    ZVEC_TYPENAME(T)*: zvec_reverse_##T,
#define ZVEC_SORT_ENTRY(T)       ZVEC_TYPENAME(T)*: zvec_sort_##T,
#define ZVEC_BSEARCH_ENTRY(T)    ZVEC_TYPENAME(T)*: zvec_bsearch_##T,

#define ZVEC_PUSH(T, v, val)           zvec_push_##T(v, val)
#define ZVEC_EXTEND(T, v, arr, count)  zvec_extend_##T(v, arr, count)
#define ZVEC_RESERVE(T, v, cap)        zvec_reserve_##T(v, cap)
#define ZVEC_IS_EMPTY(T, v)            zvec_is_empty_##T(v)
#define ZVEC_AT(T, v, idx)             zvec_at_##T(v, idx)
#define ZVEC_GET_AT(T, v, idx)         zvec_get_at_##T(v, idx)
#define ZVEC_GET_AT_MUT(T, v, idx)     zvec_get_at_mut_##T(v, idx)
#define ZVEC_DATA(T, v)                zvec_data_##T(v)
#define ZVEC_LAST(T, v)                zvec_last_##T(v)
#define ZVEC_FREE(T, v)                zvec_free_##T(v)
#define ZVEC_POP(T, v)                 zvec_pop_##T(v)
#define ZVEC_POP_GET(T, v)             zvec_pop_get_##T(v)
#define ZVEC_SHRINK_TO_FIT(T, v)       zvec_shrink_to_fit_##T(v)
#define ZVEC_REMOVE(T, v, i)           zvec_remove_##T(v, i)
#define ZVEC_SWAP_REMOVE(T, v, i)      zvec_swap_remove_##T(v, i)
#define ZVEC_CLEAR(T, v)               zvec_clear_##T(v)
#define ZVEC_REVERSE(T, v)             zvec_reverse_##T(v)
#define ZVEC_SORT(T, v, cmp)           zvec_sort_##T(v, cmp)
#define ZVEC_BSEARCH(T, v, key, cmp)   zvec_bsearch_##T(v, key, cmp)

#define zvec_push(v, val)          _Generic((v), REGISTER_TYPES(ZVEC_PUSH_ENTRY)      default: 0)      (v, val)
#define zvec_extend(v, arr, count) _Generic((v), REGISTER_TYPES(ZVEC_EXTEND_ENTRY)    default: 0)      (v, arr, count)
#define zvec_reserve(v, cap)       _Generic((v), REGISTER_TYPES(ZVEC_RESERVE_ENTRY)   default: 0)      (v, cap)
#define zvec_is_empty(v)           _Generic((v), REGISTER_TYPES(ZVEC_IS_EMPTY_ENTRY)  default: 0)      (v)
#define zvec_at(v, idx)            _Generic((v), REGISTER_TYPES(ZVEC_AT_ENTRY)        default: (void)0)(v, idx)
#define zvec_get_at(v, idx)        _Generic((v), REGISTER_TYPES(ZVEC_GET_AT_ENTRY)    default: (void)0)(v, idx)
#define zvec_get_at_mut(v, idx)    _Generic((v), REGISTER_TYPES(ZVEC_GET_AT_MUT_ENTRY)default: (void)0)(v, idx)
#define zvec_data(v)               _Generic((v), REGISTER_TYPES(ZVEC_DATA_ENTRY)      default: (void)0)(v)
#define zvec_last(v)               _Generic((v), REGISTER_TYPES(ZVEC_LAST_ENTRY)      default: (void)0)(v)
#define zvec_free(v)               _Generic((v), REGISTER_TYPES(ZVEC_FREE_ENTRY)      default: (void)0)(v)
#define zvec_pop(v)                _Generic((v), REGISTER_TYPES(ZVEC_POP_ENTRY)       default: (void)0)(v)
#define zvec_pop_get(v)            _Generic((v), REGISTER_TYPES(ZVEC_POP_GET_ENTRY)   default: (void)0)(v)
#define zvec_shrink_to_fit(v)      _Generic((v), REGISTER_TYPES(ZVEC_SHRINK_ENTRY)    default: (void)0)(v)
#define zvec_remove(v, i)          _Generic((v), REGISTER_TYPES(ZVEC_REMOVE_ENTRY)    default: (void)0)(v, i)
#define zvec_swap_remove(v, i)     _Generic((v), REGISTER_TYPES(ZVEC_SWAP_REM_ENTRY)  default: (void)0)(v, i)
#define zvec_clear(v)              _Generic((v), REGISTER_TYPES(ZVEC_CLEAR_ENTRY)     default: (void)0)(v)
#define zvec_reverse(v)            _Generic((v), REGISTER_TYPES(ZVEC_REVERSE_ENTRY)   default: (void)0)(v)
#define zvec_sort(v, cmp)          _Generic((v), REGISTER_TYPES(ZVEC_SORT_ENTRY)      default: (void)0)(v, cmp)
#define zvec_bsearch(v, key, cmp)  _Generic((v), REGISTER_TYPES(ZVEC_BSEARCH_ENTRY)   default: (void)0)(v, key, cmp)

#define ZVEC_INIT(T) ZVEC_EMPTY(T)
#define ZVEC_INIT_WITH_CAP(T, cap) zvec_init_capacity_##T(cap)

#define ZVEC_CAT(a, b) a##b
#define ZVEC_NAME(a, b) ZVEC_CAT(a, b)

#define zvec_foreach(v, iter) \
    for (size_t ZVEC_NAME(_i_, __LINE__) = 0; \
         ZVEC_NAME(_i_, __LINE__) < (v)->length && ((iter) = &(v)->data[ZVEC_NAME(_i_, __LINE__)]); \
         ++ZVEC_NAME(_i_, __LINE__))


#define ZVEC_IMPLEMENT_VEC_TYPE(T)                                                          \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] ZVEC_TYPENAME(T) zvec_init_capacity_##T(size_t cap) {                      \
    ZVEC_TYPENAME(T) v = ZVEC_EMPTY(T);                                                                  \
    if (cap > 0) {                                                                          \
        v.data = calloc(cap, sizeof(T));                                                    \
        v.capacity = v.data ? cap : 0;                                                      \
    }                                                                                       \
    return v;                                                                               \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] ZvecResult zvec_reserve_##T(ZVEC_TYPENAME(T) *v, size_t new_cap) {                \
    if (new_cap <= v->capacity) return ZvecResultOk;                                              \
    T *new_data = realloc(v->data, new_cap * sizeof(T));                                    \
    if (!new_data) return ZvecResultErr;                                                          \
    v->data = new_data;                                                                     \
    v->capacity = new_cap;                                                                  \
    return ZvecResultOk;                                                                          \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] bool zvec_is_empty_##T(ZVEC_TYPENAME(T) v) {                               \
    return v.length == 0;                                                                  \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] ZvecResult zvec_push_##T(ZVEC_TYPENAME(T) *v, T value) {                          \
    if (v->length >= v->capacity) {                                                         \
        size_t new_cap = v->capacity == 0 ? 8 : v->capacity * 2;                            \
        if (zvec_reserve_##T(v, new_cap) != ZvecResultOk) return ZvecResultErr;                          \
    }                                                                                       \
    v->data[v->length++] = value;                                                           \
    return ZvecResultOk;                                                                          \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] ZvecResult zvec_extend_##T(ZVEC_TYPENAME(T) *v, const T *items, size_t count) {   \
    if (v->length + count > v->capacity) {                                                  \
        size_t new_cap = v->capacity == 0 ? 8 : v->capacity;                                \
        while (new_cap < v->length + count) new_cap *= 2;                                   \
        if (zvec_reserve_##T(v, new_cap) != ZvecResultOk) return ZvecResultErr;                          \
    }                                                                                       \
    memcpy(v->data + v->length, items, count * sizeof(T));                                  \
    v->length += count;                                                                     \
    return ZvecResultOk;                                                                          \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_pop_##T(ZVEC_TYPENAME(T) *v) {                                   \
    assert(v->length > 0 && "Popping empty vector");                                        \
    v->length--;                                                                            \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] T zvec_pop_get_##T(ZVEC_TYPENAME(T) *v) {                                  \
    assert(v->length > 0 && "Vector is empty, cannot pop!");                                \
    return v->data[--v->length];                                                            \
}                                                                                           \
ZVEC_FUN_ATTRIBUTES void zvec_shrink_to_fit_##T(ZVEC_TYPENAME(T) *v) {                         \
    if (v->length == 0) {                                                                   \
        free(v->data);                                                                      \
        *v = ZVEC_EMPTY(T);                                                            \
        return;                                                                             \
    }                                                                                       \
    if (v->length == v->capacity) return;                                                   \
    T *new_data = realloc(v->data, v->length * sizeof(T));                                  \
    if (!new_data) return;                                                                  \
    v->data = new_data;                                                                     \
    v->capacity = v->length;                                                                \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] T zvec_at_##T(ZVEC_TYPENAME(T) v, size_t index) {                         \
    assert(index < v.length && "Vector index out of bounds!");                             \
    return v.data[index];                                                                  \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] T* zvec_get_at_mut_##T( ZVEC_TYPENAME(T) *v, size_t index){ \
    return (index < v->length) ? &v->data[index] : NULL;                                    \
}                                                                                            \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] const T* zvec_get_at_##T(const ZVEC_TYPENAME(T) *v, size_t index){ \
    return (index < v->length) ? &v->data[index] : NULL;                                    \
}                                                                                            \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] T* zvec_data_##T(ZVEC_TYPENAME(T) *v) {                                    \
    return v->data;                                                                         \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] T* zvec_last_##T(ZVEC_TYPENAME(T) *v) {                                    \
    return (v->length > 0) ? &v->data[v->length - 1] : NULL;                                \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_remove_##T(ZVEC_TYPENAME(T) *v, size_t index) {                  \
    if (index >= v->length) return;                                                         \
    memmove(&v->data[index], &v->data[index + 1],                                           \
            (v->length - index - 1) * sizeof(T));                                           \
    v->length--;                                                                            \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_swap_remove_##T(ZVEC_TYPENAME(T) *v, size_t index) {             \
    if (index >= v->length) return;                                                         \
    v->data[index] = v->data[--v->length];                                                  \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_clear_##T(ZVEC_TYPENAME(T) *v) {                                 \
    v->length = 0;                                                                          \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_free_##T(ZVEC_TYPENAME(T) *v) {                                  \
    free(v->data);                                                                          \
    *v = ZVEC_EMPTY(T);                                                                \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_reverse_##T(ZVEC_TYPENAME(T) *v) {                               \
    if (v->length < 2) return;                                                              \
    size_t i = 0, j = v->length - 1;                                                        \
    while (i < j) {                                                                         \
        T temp = v->data[i];                                                                \
        v->data[i] = v->data[j];                                                            \
        v->data[j] = temp;                                                                  \
        i++; j--;                                                                           \
    }                                                                                       \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES void zvec_sort_##T(ZVEC_TYPENAME(T) *v,                                    \
                                        int (*compar)(const T *, const T *)) {              \
    if (v->length > 1) {                                                                    \
        int (*qsort_cmp)(const void *, const void *) =                                      \
            (int (*)(const void *, const void *))compar;                                    \
        qsort(v->data, v->length, sizeof(T), qsort_cmp);                                    \
    }                                                                                       \
}                                                                                           \
                                                                                            \
ZVEC_FUN_ATTRIBUTES [[nodiscard]] T* zvec_bsearch_##T(ZVEC_TYPENAME(T) *v, const void *key,                  \
                                 int (*compar)(const T *, const T *)) {                     \
    if (v->length == 0) return NULL;                                                        \
    int (*bsearch_cmp)(const void *, const void *) =                                        \
        (int (*)(const void *, const void *))compar;                                        \
    return (T*) bsearch(key, v->data, v->length, sizeof(T), bsearch_cmp);                   \
}

#define ZVEC_DEFINE_AND_IMPLEMENT_VEC_TYPE(T) \
    ZVEC_DEFINE_VEC_TYPE(T)                   \
    ZVEC_IMPLEMENT_VEC_TYPE(T)
