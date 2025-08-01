

#pragma once

#include "./diagnostics.h"
#include "./types.h"

[[nodiscard]] int parse_font_preset(const char* preset);

void validate_fonts_of_result(AssResult ass_result, bool allow_validation_errors, FontPreset preset,
                              Diagnostics* diagnostics);

void validate_ass_result(AssResult ass_result, ParseSettings settings, Diagnostics* diagnostics);
