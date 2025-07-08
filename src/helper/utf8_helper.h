

#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
	int32_t* data;
	uint64_t size;
} Utf8Data;

typedef struct {
	bool has_error;
	union {
		Utf8Data result;
		const char* error;
	} data;
} Utf8DataResult;

[[nodiscard]] Utf8DataResult get_utf8_string(const void* data, size_t size);

void free_utf8_data(Utf8Data data);

char* get_normalized_string(int32_t* data, size_t size);
