
#include "./sized_ptr.h"

#include <stdlib.h>

[[nodiscard]] bool is_ptr_error(SizedPtr ptr) {
	return ptr.len == 0 && ptr.data != NULL;
}

[[nodiscard]] SizedPtr ptr_error(const char* message) {
	return (SizedPtr){ .data = (void*)message, .len = 0 };
}

[[nodiscard]] const char* ptr_get_error(SizedPtr ptr) {
	return (const char*)ptr.data;
}

void free_sized_ptr(SizedPtr ptr) {
	free(ptr.data);
}
