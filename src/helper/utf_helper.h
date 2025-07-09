

#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
	int32_t* data;
	uint64_t size;
} Codepoints;

typedef struct {
	bool has_error;
	union {
		Codepoints result;
		const char* error;
	} data;
} CodepointsResult;

[[nodiscard]] CodepointsResult get_codepints_from_utf8(const void* data, size_t size);

void free_codepoints(Codepoints data);

[[nodiscard]] char* get_normalized_string_from_codepoints(Codepoints codepoints);
