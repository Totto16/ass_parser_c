

#include "./utf_helper.h"

#include <errno.h>
#include <iconv.h>
#include <utf8proc.h>

CodepointsResult get_codepoints_from_utf8(SizedPtr ptr) {

	if(ptr.data == NULL && ptr.len == 0) {
		return (CodepointsResult){ .has_error = false,
			                       .data = { .result = (Codepoints){ .size = 0, .data = NULL } } };
	}

	utf8proc_int32_t* buffer = malloc(sizeof(utf8proc_int32_t) * ptr.len);

	if(!buffer) {
		return (CodepointsResult){ .has_error = true, .data = { .error = "failed malloc" } };
	}

	utf8proc_ssize_t result = utf8proc_decompose(
	    ptr.data, ptr.len, buffer, ptr.len,
	    0); // NOLINT(cppcoreguidelines-narrowing-conversions,clang-analyzer-optin.core.EnumCastOutOfRange)

	if(result < 0) {
		free(buffer);
		return (CodepointsResult){ .has_error = true,
			                       .data = { .error = utf8proc_errmsg(result) } };
	}

	if((size_t)result != ptr.len) {
		// truncate the buffer
		void* new_buffer = realloc(buffer, sizeof(utf8proc_int32_t) * result);

		if(!new_buffer) {
			free(buffer);
			return (CodepointsResult){ .has_error = true, .data = { .error = "failed realloc" } };
		}
		buffer = new_buffer;
	}

	Codepoints utf8_data = { .size = result, .data = buffer };

	return (CodepointsResult){ .has_error = false, .data = { .result = utf8_data } };
}

#define CHUNK_SIZE_CONVERSION (1 << 14)

[[nodiscard]] static SizedPtr convert_to_utf8_from_format(SizedPtr ptr, const char* format) {
	iconv_t conversion_state = iconv_open("UTF-8", format);
	if(conversion_state == (iconv_t)(-1)) {
		return ptr_error("iconv conversion allocation failed, invalid formats");
	}

	SizedPtr result_ptr = { .data = NULL, .len = 0 };

	void* start_buf = malloc(CHUNK_SIZE_CONVERSION);
	if(!start_buf) {
		iconv_close(conversion_state);
		return ptr_error("allocation error");
	}

	result_ptr.data = start_buf;

	char* inbuf = (char*)ptr.data;
	size_t inbytesleft = ptr.len;

	char* outbuf = (char*)result_ptr.data;
	size_t outbytesleft = CHUNK_SIZE_CONVERSION;

	while(true) {

		size_t result = iconv(conversion_state, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
		if(result == (size_t)(-1)) {
			if(errno == EILSEQ) {
				iconv_close(conversion_state);
				free(result_ptr.data);
				return ptr_error("invalid byte sequence detected, while converting");
			} else if(errno == EINVAL) {
				free(result_ptr.data);
				iconv_close(conversion_state);
				return ptr_error("byte sequence terminated too early, while converting");
			} else if(errno == E2BIG) {
				// more buffer needed!

				result_ptr.len = result_ptr.len + (CHUNK_SIZE_CONVERSION - outbytesleft);

				char* new_buffer =
				    (char*)realloc(result_ptr.data, result_ptr.len + CHUNK_SIZE_CONVERSION);

				if(!new_buffer) {
					iconv_close(conversion_state);
					free(result_ptr.data);
					return ptr_error("realloc error");
				}

				result_ptr.data = new_buffer;
				outbytesleft = CHUNK_SIZE_CONVERSION;
				outbuf = new_buffer + result_ptr.len;
				continue;

			} else {
				iconv_close(conversion_state);
				free(result_ptr.data);
				return ptr_error("unknown error occurred, while converting");
			}
		}

		size_t added_size = result == 0 ? (CHUNK_SIZE_CONVERSION - outbytesleft) : result;

		result_ptr.len = result_ptr.len + added_size;
		break;
	}

	if(result_ptr.len == 0) {
		free(result_ptr.data);
		result_ptr.data = NULL;
	}

	iconv_close(conversion_state);
	return result_ptr;
}

[[nodiscard]] CodepointsResult get_codepoints_from_format(SizedPtr ptr, const char* format) {
	SizedPtr converted_result = convert_to_utf8_from_format(ptr, format);

	if(is_ptr_error(converted_result)) {

		return (CodepointsResult){ .has_error = true,
			                       .data = { .error = ptr_get_error(converted_result) } };
	}

	CodepointsResult result = get_codepoints_from_utf8(converted_result);

	free_sized_ptr(converted_result);

	return result;
}

[[nodiscard]] CodepointsResult get_codepoints_from_utf16(SizedPtr ptr, bool big_endian) {
	return get_codepoints_from_format(ptr, big_endian ? "UTF-16BE" : "UTF-16LE");
}

[[nodiscard]] CodepointsResult get_codepoints_from_utf32(SizedPtr ptr, bool big_endian) {
	return get_codepoints_from_format(ptr, big_endian ? "UTF-16BE" : "UTF-16LE");
}

void free_codepoints(Codepoints data) {
	if(data.data != NULL) {
		free(data.data);
	}
}

#define CHUNK_SIZE_NORMALIZE 256

char* get_normalized_string_from_codepoints(Codepoints codepoints) {

	size_t buffer_size = CHUNK_SIZE_NORMALIZE;
	uint8_t* buffer = (uint8_t*)malloc(buffer_size);

	size_t current_size = 0;

	if(!buffer) {
		return NULL;
	}

	for(size_t i = 0; i < codepoints.size; ++i) {

		if(buffer_size - current_size < 4) {
			buffer_size = buffer_size + CHUNK_SIZE_NORMALIZE;
			uint8_t* new_buffer = (uint8_t*)realloc(buffer, buffer_size);

			if(!new_buffer) {
				free(buffer);
				return NULL;
			}

			buffer = new_buffer;
		}

		long result = utf8proc_encode_char(codepoints.data[i], buffer + current_size);

		if(result <= 0) {
			free(buffer);
			return NULL;
		}

		current_size = current_size + result;
	}

	if(buffer_size - current_size < 1) {
		buffer_size = buffer_size + 1;
		uint8_t* new_buffer = (uint8_t*)realloc(buffer, buffer_size);

		if(!new_buffer) {
			free(buffer);
			return NULL;
		}

		buffer = new_buffer;
	}

	buffer[current_size] = '\0';

	return (char*)buffer;
}
