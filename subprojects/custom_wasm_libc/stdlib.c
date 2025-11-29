

#include <my_malloc/my_malloc.h>

#include "./stdlib.h"
#include "./string.h"

void* malloc(size_t size) {
	return my_malloc(size);
}

void free(void* ptr) {
	my_free(ptr);
}

void* realloc(void* ptr, size_t size) {
	return my_realloc(ptr, size);
}

__attribute__((constructor)) void stdlib_constructor(void) {
	my_allocator_init();
}

#include "./assert.h"

// from musl: https://github.com/esmil/musl/blob/master/src/malloc/calloc.c
void* calloc(size_t nmemb, size_t size) {
	void* p;
	size_t* z;

	if(size && nmemb > (size_t)-1 / size) {
		// NOTE: errno is not supported!
		// errno = ENOMEM;
		PANIC("ENOMEM in calloc");
		return NULL;
	}

	size *= nmemb;

	p = malloc(size);

	if(!p) {
		return NULL;
	}

	/* Only do this for non-mmapped chunks */
	if(((size_t*)p)[-1] & 7) {
		/* Only write words that are not already zero */
		nmemb = (size + sizeof *z - 1) / sizeof *z;
		for(z = p; nmemb; nmemb--, z++) {
			if(*z) {
				*z = 0;
			}
		}
	}
	return p;
}

__attribute__((noreturn)) void exit(int status) {
	platform_exit(status);
}
