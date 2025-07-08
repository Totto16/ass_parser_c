

#pragma once

#include "../helper/sized_ptr.h"

typedef enum {
	AssSourceTypeFile,
	AssSourceTypeStr,
} AssSourceType;

typedef struct {
	AssSourceType type;
	union {
		char* file;
		SizedPtr str;
	} data;

} AssSource;

typedef struct AssParseResultImpl AssParseResult;

[[nodiscard]] AssParseResult* parse_ass(AssSource source);

[[nodiscard]] bool parse_result_is_error(AssParseResult* result);

[[nodiscard]] const char* parse_result_get_error(AssParseResult* result);
