

#include "./string.h"
#include "./ctype.h"
#include "./stdlib.h"

void* memcpy(void* dest, const void* src, size_t n) {
	return __builtin_memcpy(dest, src, n);
}

void* memset(void* s, int c, size_t n) {
	return __builtin_memset(s, c, n);
}

void* memmove(void* dest, const void* src, size_t n) {
	return __builtin_memmove(dest, src, n);
}

int memcmp(const void* s1, const void* s2, size_t n) {
	return __builtin_memcmp(s1, s2, n);
}

size_t strlen(const char* s) {
	return __builtin_strlen(s);
}

int strcmp(const char* s1, const char* s2) {
	return __builtin_strcmp(s1, s2);
}

int strcasecmp(const char* s1, const char* s2) {
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
char* strcasestr(const char* haystack, const char* needle) {
	size_t l = strlen(needle);
	for(; *haystack; haystack++)
		if(!strncasecmp(haystack, needle, l)) return (char*)haystack;
	return 0;
}

// from musl:
// https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/string/strdup.c#L5

char* strdup(const char* s) {

	size_t l = strlen(s);
	char* d = malloc(l + 1);
	if(!d) return NULL;
	return memcpy(d, s, l + 1);
}
