

#pragma once

#include "./parser.h"
#include "./warnings.h"


[[nodiscard]] double parse_str_as_double(ConstStrView value, ErrorStruct* error_ptr,
                                         Warnings* warnings);

[[nodiscard]] size_t parse_str_as_unsigned_number(ConstStrView value, ErrorStruct* error_ptr,
                                                  Warnings* warnings);

[[nodiscard]] bool parse_str_as_bool(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] bool parse_str_as_str_bool(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] AssColor parse_str_as_color(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] BorderStyle parse_str_as_border_style(ConstStrView value, ErrorStruct* error_ptr,
                                                    Warnings* warnings);

[[nodiscard]] AssAlignment parse_str_as_style_alignment(ConstStrView value, ErrorStruct* error_ptr,
                                                        Warnings* warnings);

[[nodiscard]] size_t parse_str_as_unsigned_number_with_option(ConstStrView value,
                                                              ErrorStruct* error_ptr,
                                                              bool allow_number_truncating,
                                                              Warnings* warnings);

[[nodiscard]] MarginValue parse_str_as_margin_value(ConstStrView value, ErrorStruct* error_ptr,
                                                    Warnings* warnings);

[[nodiscard]] AssTime parse_str_as_time(ConstStrView value, ErrorStruct* error_ptr,
                                        Warnings* warnings);

[[nodiscard]] ScriptType parse_str_as_script_type(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] WrapStyle parse_str_as_wrap_style(ConstStrView value, ErrorStruct* error_ptr,
                                                Warnings* warnings);
