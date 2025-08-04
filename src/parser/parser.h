

#pragma once

#include "../helper/decl.h"
#include "./diagnostics.h"
#include "./types.h"

typedef struct AssParseResultImpl AssParseResult;

[[nodiscard]] PUBLIC("parse_ass") AssParseResult* parse_ass(AssSource source,
                                                            ParseSettings settings);

[[nodiscard]] PUBLIC("get_diagnostics_from_result") Diagnostics
    get_diagnostics_from_result(AssParseResult* result);

[[nodiscard]] PUBLIC("parse_result_is_error") bool parse_result_is_error(AssParseResult* result);

[[nodiscard]] PUBLIC("parse_result_get_value") AssResult
    parse_result_get_value(AssParseResult* result);

PUBLIC("free_parse_result") void free_parse_result(AssParseResult* result);

[[nodiscard]] PUBLIC("get_script_type_name") const
    char* get_script_type_name(ScriptType script_type);

[[nodiscard]] PUBLIC("get_file_type_name") const char* get_file_type_name(FileType file_type);
