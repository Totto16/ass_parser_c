

#pragma once

#include "./diagnostics.h"
#include "./types.h"

[[nodiscard]] int parse_font_preset(const char* preset);

void validate_ass_result(AssResult ass_result, ParseSettings settings, Diagnostics* diagnostics);
