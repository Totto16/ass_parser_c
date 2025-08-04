

#pragma once

#include "./stddef.h"

void* memcpy(void* dest, const void* src, size_t n);

void* memset(void* s, int c, size_t n);

void* memmove(void* dest, const void* src, size_t n);

int memcmp(const void* s1, const void* s2, size_t n);

size_t strlen(const char* s);

int strcmp(const char* s1, const char* s2);

int strcasecmp(const char* s1, const char* s2);

char* strcasestr(const char* haystack, const char* needle);

char* strdup(const char* s);
