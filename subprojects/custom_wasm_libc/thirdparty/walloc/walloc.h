#pragma once

typedef __SIZE_TYPE__ size_t;
typedef __UINTPTR_TYPE__ uintptr_t;
typedef __UINT8_TYPE__ uint8_t;

#define NULL ((void *) 0)

void* w_malloc(size_t size);

void w_free(void* ptr);
