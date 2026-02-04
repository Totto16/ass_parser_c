

#pragma once

#include "./stddef.h"

// expose allocators

__attribute__((export_name("malloc"))) __attribute__((annotate("category:POINTER"))) void*
malloc(size_t size);
__attribute__((export_name("free"))) __attribute__((annotate("category:VOID")))
__attribute__((annotate("is_free_fn"))) void
free(void* ptr);
__attribute__((export_name("realloc"))) __attribute__((annotate("category:POINTER"))) void*
realloc(void* ptr, size_t size);

__attribute__((export_name("calloc"))) __attribute__((annotate("category:POINTER"))) void*
calloc(size_t nmemb, size_t size);

__attribute__((export_name("bsearch"))) __attribute__((annotate("category:POINTER"))) void*
bsearch(const void*, const void*, size_t, size_t, int (*)(const void*, const void*));
__attribute__((export_name("qsort"))) __attribute__((annotate("category:VOID"))) void
qsort(void*, size_t, size_t, int (*)(const void*, const void*));

__attribute__((export_name("exit"))) __attribute__((annotate("category:VOID")))
__attribute__((noreturn)) void
exit(int status);

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1 /* nonzero */
