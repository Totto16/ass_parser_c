

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "./sized_ptr.h"

typedef struct {
	const int32_t* data_const;
	int32_t* data_readable;
} CodePointsData;

typedef struct {
	uint64_t size;
	CodePointsData data;
} Codepoints;

typedef struct {
	uint64_t size;
	const int32_t* data;
} RawCodepoints;

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

void free_codepoints(Codepoints codepoints);

[[nodiscard]] char* get_normalized_string_from_codepoints(RawCodepoints codepoints);
