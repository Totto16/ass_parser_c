#pragma once

#define UNUSED(v) ((void)(v))

// cool trick from here:
// https://stackoverflow.com/questions/777261/avoiding-unused-variables-warnings-when-using-assert-in-a-release-build
#ifdef NDEBUG
#define assert(x) /* NOLINT(readability-identifier-naming) */ \
	do { \
		UNUSED((x)); \
	} while(false)
#else

#include <assert.h>

#endif

#define FORMAT_STRING(to_store, error_statement, format, ...) \
	{ \
		char* internal_buffer = *to_store; \
		if(internal_buffer != NULL) { \
			free(internal_buffer); \
		} \
		int to_write = snprintf(NULL, 0, format, __VA_ARGS__) + 1; \
		internal_buffer = (char*)malloc(to_write * sizeof(char)); \
		if(!internal_buffer) { \
			fprintf(stderr, "Couldn't allocate memory for %d bytes!\n", to_write); \
			error_statement; \
		} \
		int written = snprintf(internal_buffer, to_write, format, __VA_ARGS__); \
		if(written >= to_write) { \
			fprintf(stderr, \
			        "Snprint did write more bytes then it had space in the buffer, available " \
			        "space:'%d', actually written:'%d'!\n", \
			        (to_write) - 1, written); \
			free(internal_buffer); \
			error_statement; \
		} \
		*to_store = internal_buffer; \
	} \
	if(*to_store == NULL) { \
		fprintf(stderr, "snprintf Macro gone wrong: '%s' is pointing to NULL!\n", #to_store); \
		error_statement; \
	}

#define FORMAT_STRING_DEFAULT(to_store, format, ...) \
	FORMAT_STRING(to_store, exit(EXIT_FAILURE), format, __VA_ARGS__)

#define STRINGIFY(a) STR_IMPL(a)
#define STR_IMPL(a) #a
