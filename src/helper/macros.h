#pragma once

#define UNUSED(v) ((void)(v))

// cool trick from here:
// https://stackoverflow.com/questions/777261/avoiding-unused-variables-warnings-when-using-assert-in-a-release-build
#ifdef NDEBUG
#define ASSERT(x, msg) /* NOLINT(readability-identifier-naming) */ \
	do { \
		UNUSED((x)); \
		UNUSED((msg)); \
	} while(false)

#else

#include "./assert.h"
#define ASSERT(cond, message) custom_assert(__FILE__, __LINE__, cond, message)

#endif

#define FORMAT_STRING(to_store, error_statement, format, ...) \
	{ \
		if(to_store == NULL) { \
			fprintf(stderr, "FORMAT_STRING macro gone wrong: '%s' is NULL!\n", #to_store); \
			error_statement; \
		} \
		char* internal_buffer = *to_store; \
		if(internal_buffer != NULL) { \
			free(internal_buffer); \
		} \
		int to_write = snprintf(NULL, 0, format, __VA_ARGS__) + 1; \
		internal_buffer = (char*)malloc(to_write * sizeof(char)); \
		if(internal_buffer == NULL) { \
			fprintf(stderr, "Couldn't allocate memory for %d bytes!\n", to_write); \
			error_statement; \
		} \
		int written = snprintf(internal_buffer, to_write, format, __VA_ARGS__); \
		if(written >= to_write) { \
			fprintf(stderr, \
			        "Snprint did write more bytes then it had space in the buffer, available " \
			        "space: '%d', actually written: '%d'!\n", \
			        (to_write) - 1, written); \
			free(internal_buffer); \
			error_statement; \
		} \
		*to_store = internal_buffer; \
	}

#define FORMAT_STRING_DEFAULT(to_store, format, ...) \
	FORMAT_STRING(to_store, exit(EXIT_FAILURE), format, __VA_ARGS__)

#define FORMAT_STRING_PROPAGATE_ERROR(to_store, format, ...) \
	FORMAT_STRING(to_store, PROPAGATE_ERROR_IMPL("string format allocation error"), format, \
	              __VA_ARGS__)

#ifdef NDEBUG
#define UNREACHABLE() \
	do { \
		fprintf(stderr, "[%s %s:%d]: UNREACHABLE", __func__, __FILE__, __LINE__); \
		exit(EXIT_FAILURE); \
	} while(false)
#else

#define UNREACHABLE() \
	do { \
		ASSERT(false, "UNREACHABLE"); /*NOLINT(cert-dcl03-c,misc-static-assert)*/ \
	} while(false)

#endif
