
#pragma once

#include <stddef.h>

typedef struct {
	void* data;
	size_t len;
} SizedPtr;

[[nodiscard]] bool is_ptr_error(SizedPtr ptr);

[[nodiscard]] SizedPtr ptr_error(const char* message);

[[nodiscard]] const char* ptr_get_error(SizedPtr ptr);
