

#pragma once

#include "./diagnostics.h"
#include "./types.h"

typedef struct AssParseResultImpl AssParseResult;

[[nodiscard]] AssParseResult* parse_ass(AssSource source, ParseSettings settings);

[[nodiscard]] Diagnostics get_diagnostics_from_result(AssParseResult* result);

[[nodiscard]] bool parse_result_is_error(AssParseResult* result);

[[nodiscard]] AssResult parse_result_get_value(AssParseResult* result);

void free_parse_result(AssParseResult* result);

[[nodiscard]] const char* get_script_type_name(ScriptType script_type);

[[nodiscard]] const char* get_file_type_name(FileType file_type);
