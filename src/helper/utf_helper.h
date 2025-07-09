

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "./sized_ptr.h"

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

[[nodiscard]] CodepointsResult get_codepoints_from_utf8(SizedPtr ptr);

[[nodiscard]] CodepointsResult get_codepoints_from_utf16(SizedPtr ptr, bool big_endian);

[[nodiscard]] CodepointsResult get_codepoints_from_utf32(SizedPtr ptr, bool big_endian);

void free_codepoints(Codepoints data);

[[nodiscard]] char* get_normalized_string_from_codepoints(Codepoints codepoints);
