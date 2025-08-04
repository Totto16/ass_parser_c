#pragma once

typedef __SIZE_TYPE__ size_t;
typedef __UINTPTR_TYPE__ uintptr_t;
typedef __UINT8_TYPE__ uint8_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

void* w_malloc(size_t size);

void w_free(void* ptr);

size_t w_get_allocated_size(void* ptr);
