

#pragma once

#include "../helper/decl.h"
#include "./diagnostics.h"
#include "./types.h"

[[nodiscard]] PUBLIC("parse_font_preset") int parse_font_preset(const char* preset);

PUBLIC("validate_fonts_of_result")
void validate_fonts_of_result(AssResult ass_result, bool allow_validation_errors, FontPreset preset,
                              Diagnostics* diagnostics);

PUBLIC("validate_ass_result")
void validate_ass_result(AssResult ass_result, ParseSettings settings, Diagnostics* diagnostics);
