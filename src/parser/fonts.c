

#include "./fonts.h"

#include <stb/ds.h>

typedef unsigned char Byte;

#define MIN_MACRO(a, b) ((a) > (b) ? (b) : (a))

#define VALUE_OFFSET 33

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

	STBDS_ARRAY(char) final_chars = STBDS_ARRAY_EMPTY;

	size_t max_size = ((input_size * 4) + 2) / 3;

	stbds_arrsetcap(final_chars, max_size);

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
			stbds_arrput(final_chars, val);
		}
	}

	// make stbds_array into sizedptr
	// here we can use strdup, as no \0 characters can be in the result
	char* result = strdup(final_chars);

	if(!result) {
		stbds_arrfree(final_chars);
		return ptr_error("allocation error");
	}

	size_t final_length = stbds_arrlenu(final_chars);
	stbds_arrfree(final_chars);

	return (SizedPtr){ .data = result, .len = final_length };
}

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

	STBDS_ARRAY(Byte) final_bytes = STBDS_ARRAY_EMPTY;

	size_t max_size = (input_size * 3) / 4;

	stbds_arrsetcap(final_bytes, max_size);

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
			stbds_arrput(final_bytes, val);
		}
	}

	// make stbds_array into sizedptr
	size_t final_length = stbds_arrlenu(final_bytes);

	Byte* result = malloc(final_length);

	if(!result) {
		stbds_arrfree(final_bytes);
		return ptr_error("allocation error");
	}

	memcpy(result, final_bytes, final_length);

	stbds_arrfree(final_bytes);

	return (SizedPtr){ .data = result, .len = final_length };
}
