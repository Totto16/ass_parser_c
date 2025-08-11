

#pragma once

#include "./stddef.h"

// expose allocators

__attribute__((export_name("malloc"))) void* malloc(size_t size);
__attribute__((export_name("free"))) __attribute__((annotate("is_free_fn"))) void free(void* ptr);
__attribute__((export_name("realloc"))) void* realloc(void* ptr, size_t size);
