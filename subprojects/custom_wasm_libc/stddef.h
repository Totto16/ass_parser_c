

#pragma once

#define NULL ((void*)0)

#ifdef __PTRDIFF_TYPE__

typedef __INTPTR_TYPE__ intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;

typedef __SIZE_TYPE__ size_t;

typedef __PTRDIFF_TYPE__ ptrdiff_t;

#else

typedef int32_t intptr_t;
typedef uint32_t uintptr_t;

typedef unsigned long size_t;

typedef intptr_t ptrdiff_t;

#endif
