

#include <my_malloc/my_malloc.h>

#include "./stdlib.h"
#include "./string.h"

__attribute__((export_name("malloc"))) void* malloc(size_t size) {
	return my_malloc(size);
}

__attribute__((export_name("free"))) void free(void* ptr) {
	my_free(ptr);
}

__attribute__((export_name("realloc"))) void* realloc(void* ptr, size_t size) {
	return my_realloc(ptr, size);
}

__attribute__((constructor)) void stdlib_constructor(void) {
	my_allocator_init();
}
