
#include "./sized_ptr.h"

[[nodiscard]] bool is_ptr_error(SizedPtr ptr) {
	return ptr.len == 0 && ptr.data != NULL;
}

[[nodiscard]] SizedPtr ptr_error(const char* message) {
	return (SizedPtr){ .data = (void*)message, .len = 0 };
}

[[nodiscard]] const char* ptr_get_error(SizedPtr ptr) {
	return (const char*)ptr.data;
}
