

#include "./string.h"
#include "./stdlib.h"
#include "./ctype.h"

__attribute__((export_name("memcpy"))) void* memcpy(void* dest, const void* src, size_t n) {
	return __builtin_memcpy(dest, src, n);
}

__attribute__((export_name("memset"))) void* memset(void* s, int c, size_t n) {
	return __builtin_memset(s, c, n);
}

__attribute__((export_name("memmove"))) void* memmove(void* dest, const void* src, size_t n) {
	return __builtin_memmove(dest, src, n);
}

__attribute__((export_name("memcmp"))) int memcmp(const void* s1, const void* s2, size_t n) {
	return __builtin_memcmp(s1, s2, n);
}

__attribute__((export_name("strlen"))) size_t strlen(const char* s) {
	return __builtin_strlen(s);
}

__attribute__((export_name("strcmp"))) int strcmp(const char* s1, const char* s2) {
	return __builtin_strcmp(s1, s2);
}

__attribute__((export_name("strcasecmp"))) int strcasecmp(const char* s1, const char* s2) {
	return __builtin_strcasecmp(s1, s2);
}

// see:
// https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/string/strncasecmp.c
static int strncasecmp(const char* _l, const char* _r, size_t n) {
	const unsigned char *l = (void*)_l, *r = (void*)_r;
	if(!n--) return 0;
	for(; *l && *r && n && (*l == *r || tolower(*l) == tolower(*r)); l++, r++, n--)
		;
	return tolower(*l) - tolower(*r);
}

// see
// https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/string/strcasestr.c
__attribute__((export_name("strcasestr"))) char* strcasestr(const char* haystack,
                                                            const char* needle) {
	size_t l = strlen(needle);
	for(; *haystack; haystack++)
		if(!strncasecmp(haystack, needle, l)) return (char*)haystack;
	return 0;
}

// from musl:
// https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/string/strdup.c#L5

__attribute__((export_name("strdup"))) char* strdup(const char* s) {

	size_t l = strlen(s);
	char* d = malloc(l + 1);
	if(!d) return NULL;
	return memcpy(d, s, l + 1);
}
