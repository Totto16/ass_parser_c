#pragma once

#include <stdint.h>

void* my_malloc(uint64_t size);
void my_free(void* ptr);
void* my_realloc(void* ptr, uint64_t size);

void my_allocator_init(void);
