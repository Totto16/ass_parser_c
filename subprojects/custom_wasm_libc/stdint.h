

#pragma once

#ifdef __SIZE_TYPE__

typedef __INT8_TYPE__ int8_t;
typedef __UINT8_TYPE__ uint8_t;
typedef __INT16_TYPE__ int16_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __INT32_TYPE__ int32_t;
typedef __UINT32_TYPE__ uint32_t;

typedef __INT64_TYPE__ int64_t;
typedef __UINT64_TYPE__ uint64_t;


typedef __INTPTR_TYPE__ intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;

typedef __SIZE_TYPE__ size_t;

#define PTRDIFF_MAX __PTRDIFF_MAX__
#define SIZE_MAX __SIZE_MAX__

#define SSIZE_MAX PTRDIFF_MAX

#else

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int int16_t;
typedef unsigned short int uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;

typedef signed long long int32_t;
typedef unsigned long long int uint32_t;

typedef int32_t intptr_t;
typedef uint32_t uintptr_t;

typedef unsigned long size_t;

#define SSIZE_MAX INT32_MAX

#endif
