

#pragma once

#include "../helper/decl.h"
#include <stddef.h>

// expose allocators

PUBLIC("malloc") void* malloc(size_t size);
PUBLIC("free") void free(void* ptr);
PUBLIC("realloc") void* realloc(void* ptr, size_t size);
