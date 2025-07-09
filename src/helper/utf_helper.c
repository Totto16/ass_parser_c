

#include "./utf_helper.h"

#include <utf8proc.h>

CodepointsResult get_codepints_from_utf8(const void* data, size_t size) {

	utf8proc_int32_t* buffer = malloc(sizeof(utf8proc_int32_t) * size);

	if(!buffer) {
		return (CodepointsResult){ .has_error = true, .data = { .error = "failed malloc" } };
	}

	utf8proc_ssize_t result = utf8proc_decompose(
	    data, size, buffer, size,
	    0); // NOLINT(cppcoreguidelines-narrowing-conversions,clang-analyzer-optin.core.EnumCastOutOfRange)

	if(result < 0) {
		free(buffer);
		return (CodepointsResult){ .has_error = true,
			                       .data = { .error = utf8proc_errmsg(result) } };
	}

	if((size_t)result != size) {
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

void free_codepoints(Codepoints data) {
	free(data.data);
}

#define CHUNK_SIZE 256

char* get_normalized_string_from_codepoints(Codepoints codepoints) {

	size_t buffer_size = CHUNK_SIZE;
	uint8_t* buffer = (uint8_t*)malloc(buffer_size);

	uint8_t* current_buffer = buffer;
	size_t current_size = 0;

	if(!buffer) {
		return NULL;
	}

	for(size_t i = 0; i < codepoints.size; ++i) {

		if(buffer_size - current_size < 4) {
			buffer_size = buffer_size + CHUNK_SIZE;
			uint8_t* new_buffer = (uint8_t*)realloc(buffer, buffer_size);

			if(!new_buffer) {
				free(buffer);
				return NULL;
			}
		}

		long result = utf8proc_encode_char(codepoints.data[i], current_buffer);

		if(result <= 0) {
			free(buffer);
			return NULL;
		}

		current_buffer = current_buffer + result;
		current_size = current_size + result;
	}

	if(buffer_size - current_size < 1) {
		buffer_size = buffer_size + 1;
		uint8_t* new_buffer = (uint8_t*)realloc(buffer, buffer_size);

		if(!new_buffer) {
			free(buffer);
			return NULL;
		}
	}

	buffer[current_size] = '\0';

	return (char*)buffer;
}
