

#include <walloc/walloc.h>

#include "./stdlib.h"
#include "./string.h"

__attribute__((export_name("malloc"))) void* malloc(size_t size) {
	return w_malloc(size);
}

__attribute__((export_name("free"))) void free(void* ptr) {
	w_free(ptr);
}

// TODO: use custom allocator that also has realloc, not walloc, that hasn't it
__attribute__((export_name("realloc"))) void* realloc(void* ptr, size_t size) {

	if(ptr == NULL) {
		return w_malloc(size);
	}

	if(size == 0) {
		w_free(ptr);
		return NULL;
	}

	size_t old_size = w_get_allocated_size(ptr);

	if(size < old_size) {
		return ptr;
	}

	void* new_ptr = malloc(size);
	if(new_ptr == NULL) {
		return NULL;
	}

	size_t copy_size = (size < old_size) ? size : old_size;
	memcpy(new_ptr, ptr, copy_size);

	free(ptr);

	return new_ptr;
}
