

#include "./string.h"
#include "./ctype.h"
#include "./limits.h"
#include "./stdlib.h"

// see:
// https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/string/memcpy.c
// using only simplified version
void* memcpy(void* dest, const void* src, size_t n) {
	unsigned char* d = dest;
	const unsigned char* s = src;

	for(; n; n--)
		*d++ = *s++;
	return dest;
}

// see:
// https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/string/memset.c
void* memset(void* dest, int c, size_t n) {
	unsigned char* s = dest;
	size_t k;

	/* Fill head and tail with minimal branching. Each
	 * conditional ensures that all the subsequently used
	 * offsets are well-defined and in the dest region. */

	if(!n) return dest;
	s[0] = s[n - 1] = c;
	if(n <= 2) return dest;
	s[1] = s[n - 2] = c;
	s[2] = s[n - 3] = c;
	if(n <= 6) return dest;
	s[3] = s[n - 4] = c;
	if(n <= 8) return dest;

	/* Advance pointer to align it at a 4-byte boundary,
	 * and truncate n to a multiple of 4. The previous code
	 * already took care of any head/tail that get cut off
	 * by the alignment. */

	k = -(uintptr_t)s & 3;
	s += k;
	n -= k;
	n &= -4;

	for(; n; n--, s++)
		*s = c;

	return dest;
}

// see:
// https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/string/memmove.c
#define WT size_t
#define WS (sizeof(WT))

void* memmove(void* dest, const void* src, size_t n) {
	char* d = dest;
	const char* s = src;

	if(d == s) return d;
	if(s + n <= d || d + n <= s) return memcpy(d, s, n);

	if(d < s) {
		if((uintptr_t)s % WS == (uintptr_t)d % WS) {
			while((uintptr_t)d % WS) {
				if(!n--) return dest;
				*d++ = *s++;
			}
			for(; n >= WS; n -= WS, d += WS, s += WS)
				*(WT*)d = *(WT*)s;
		}
		for(; n; n--)
			*d++ = *s++;
	} else {
		if((uintptr_t)s % WS == (uintptr_t)d % WS) {
			while((uintptr_t)(d + n) % WS) {
				if(!n--) return dest;
				d[n] = s[n];
			}
			while(n >= WS)
				n -= WS, *(WT*)(d + n) = *(WT*)(s + n);
		}
		while(n)
			n--, d[n] = s[n];
	}

	return dest;
}

// see:
//  https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/string/memcmp.c
int memcmp(const void* s1, const void* s2, size_t n) {
	const unsigned char *l = s1, *r = s2;
	for(; n && *l == *r; n--, l++, r++)
		;
	return n ? *l - *r : 0;
}

// see:
// https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/string/strlen.c
#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1 / UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX / 2 + 1))
#define HASZERO(x) ((x) - ONES & ~(x) & HIGHS)

size_t strlen(const char* s) {
	const char* a = s;
	const size_t* w;
	for(; (uintptr_t)s % ALIGN; s++)
		if(!*s) return s - a;
	for(w = (const void*)s; !HASZERO(*w); w++)
		;
	for(s = (const void*)w; *s; s++)
		;
	return s - a;
}

// see:
// https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/string/strcmp.c
int strcmp(const char* s1, const char* s2) {
	for(; *s1 == *s2 && *s1; s1++, s2++)
		;
	return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// see:
// https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/string/strcasecmp.c
int strcasecmp(const char* s1, const char* s2) {
	const unsigned char *l = (void*)s1, *r = (void*)s2;
	for(; *l && *r && (*l == *r || tolower(*l) == tolower(*r)); l++, r++)
		;
	return tolower(*l) - tolower(*r);
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
