

#include "./fonts.h"
#include "../helper/macros.h"

#include <zmap/zmap.h>

#include <tvec.h>

typedef unsigned char Byte;

#define MIN_MACRO(a, b) ((a) > (b) ? (b) : (a))

#define VALUE_OFFSET 33

TVEC_DEFINE_VEC_TYPE(char)

[[nodiscard]] SizedPtr uu_encode(SizedPtr input) {

	if(is_ptr_error(input)) {
		return input;
	}

	size_t input_size = input.len;

	if(input_size == 0) {
		return (SizedPtr){ .data = NULL, .len = 0 };
	}

	// this is according to rfc, but the code was inspired (only slightly) (not copied) from
	// aegisubs implementation. see:
	// https://github.com/TypesettingTools/Aegisub/blob/master/libaegisub/ass/uuencode.cpp#L30

	size_t max_size = ((input_size * 4) + 2) / 3;

	TVEC_TYPENAME(char) final_chars = TVEC_INIT_WITH_CAP(char, max_size);

	Byte* input_data = (Byte*)input.data;

	for(size_t i = 0; i < input_size; i += 3) {

		Byte src[3] = { '\0', '\0', '\0' };

		memcpy(src, input_data + i, MIN_MACRO(3U, input_size - i));

		char result[4] = { (char)(src[0] >> 2),
			               (char)(((src[0] & 0x3) << 4) | ((src[1] & 0xF0) >> 4)),
			               (char)(((src[1] & 0xF) << 2) | ((src[2] & 0xC0) >> 6)),
			               (char)(src[2] & 0x3F) };

		for(size_t j = 0; j < MIN_MACRO(4U, input_size - i + 1); ++j) {
			char val = (char)(result[j] + VALUE_OFFSET);
			TVEC_PUSH_AND_ASSERT(char, &final_chars, val);
		}
	}

	// make stbds_array into sizedptr
	// here we can use strdup, as no \0 characters can be in the result
	char* result = strdup(TVEC_DATA(char, &final_chars));

	if(!result) {
		TVEC_FREE(char, &final_chars);
		return ptr_error("allocation error");
	}

	size_t final_length = TVEC_LENGTH(final_chars);
	TVEC_FREE(char, &final_chars);

	return (SizedPtr){ .data = result, .len = final_length };
}

TVEC_DEFINE_AND_IMPLEMENT_VEC_TYPE(Byte)

[[nodiscard]] SizedPtr uu_decode(SizedPtr input) {

	if(is_ptr_error(input)) {
		return input;
	}

	size_t input_size = input.len;

	if(input_size == 0) {
		return (SizedPtr){ .data = NULL, .len = 0 };
	}

	// this is according to rfc, but the code was inspired (only slightly) (not copied) from
	// aegisubs implementation. see:
	// https://github.com/TypesettingTools/Aegisub/blob/master/libaegisub/ass/uuencode.cpp#L60

	size_t max_size = (input_size * 3) / 4;

	TVEC_TYPENAME(Byte) final_bytes = TVEC_INIT_WITH_CAP(Byte, max_size);

	char* input_data = (char*)input.data;

	for(size_t i = 0; i < input_size; i += 4) {

		char src[4] = { '\0', '\0', '\0', '\0' };

		for(size_t j = 0; j < MIN_MACRO(4U, input_size - i); ++j) {
			src[j] = (char)(input_data[i] - VALUE_OFFSET);
		}

		Byte result[3] = { (Byte)((src[0] << 2) | (src[1] >> 4)),
			               (Byte)(((src[1] & 0xF) << 4) | (src[2] >> 2)),
			               (Byte)(((src[2] & 0x3) << 6) | (src[3])) };

		for(size_t j = 0; j < MIN_MACRO(3U, input_size - i - 1); ++j) {
			Byte val = result[j];
			TVEC_PUSH_AND_ASSERT(Byte, &final_bytes, val);
		}
	}

	// make stbds_array into sizedptr
	size_t final_length = TVEC_LENGTH(final_bytes);

	Byte* result = malloc(final_length);

	if(!result) {
		TVEC_FREE(Byte, &final_bytes);
		return ptr_error("allocation error");
	}

	memcpy(result, TVEC_DATA(Byte, &final_bytes), final_length);

	TVEC_FREE(Byte, &final_bytes);

	return (SizedPtr){ .data = result, .len = final_length };
}
