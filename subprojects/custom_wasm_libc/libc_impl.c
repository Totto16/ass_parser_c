

#include "./string.h"

__attribute__((export_name("memcpy"))) void memcpy(void* dest, const void* src, size_t n) {
	__builtin_memcpy(dest, src, n);
}

__attribute__((export_name("memset"))) void* memset(void* s, int c, size_t n) {
	return __builtin_memset(s, c, n);
}
