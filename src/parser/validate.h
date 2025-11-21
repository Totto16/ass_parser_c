

#pragma once

#include "../helper/decl.h"
#include "./diagnostics.h"
#include "./types.h"

[[nodiscard]] PUBLIC("parse_font_preset") CATEGORY_LITERAL
    int parse_font_preset(const char* preset);

PUBLIC("validate_fonts_of_result")
CATEGORY_VOID void validate_fonts_of_result(AssResult ass_result, bool allow_validation_errors,
                                            FontPreset preset, Diagnostics* diagnostics);

PUBLIC("validate_ass_result")
CATEGORY_VOID void validate_ass_result(AssResult ass_result, ParseSettings settings,
                                       Diagnostics* diagnostics);
