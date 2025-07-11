

#pragma once

#include "./parser.h"

typedef struct {
	char* message;
	bool dynamic;
} ErrorStruct;

#define STATIC_ERROR(error) ((ErrorStruct){ .message = (char*)(error), .dynamic = false })

#define DYNAMIC_ERROR(error) ((ErrorStruct){ .message = (error), .dynamic = true })

#define NO_ERROR() STATIC_ERROR(NULL)

void free_error_struct(ErrorStruct error);

[[nodiscard]] double parse_str_as_double(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] size_t parse_str_as_unsigned_number(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] bool parse_str_as_bool(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] bool parse_str_as_str_bool(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] AssColor parse_str_as_color(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] BorderStyle parse_str_as_border_style(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] AssAlignment parse_str_as_style_alignment(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] size_t parse_str_as_unsigned_number_with_option(ConstStrView value,
                                                              ErrorStruct* error_ptr,
                                                              bool allow_number_truncating);
