

#pragma once

#include "../helper/sized_ptr.h"

#include <stdint.h>

typedef enum : uint8_t {
	AssSourceTypeFile,
	AssSourceTypeStr,
} AssSourceType;

typedef struct {
	AssSourceType type;
	union {
		const char* file;
		SizedPtr str;
	} data;

} AssSource;

typedef struct {
	bool allow_missing_fields;
	bool allow_duplicate_fields;
} ScriptInfoStrictSettings;

typedef struct {
	ScriptInfoStrictSettings script_info;
	bool allow_additional_fields;
	bool allow_number_truncating;
	bool allow_unrecognized_file_encoding;
} StrictSettings;

typedef struct {
	StrictSettings strict_settings;
} ParseSettings;

typedef struct AssParseResultImpl AssParseResult;

[[nodiscard]] AssParseResult* parse_ass(AssSource source, ParseSettings settings);

[[nodiscard]] bool parse_result_is_error(AssParseResult* result);

[[nodiscard]] char* parse_result_get_error(AssParseResult* result);

void free_parse_result(AssParseResult* result);
