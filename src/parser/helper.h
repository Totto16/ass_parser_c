

#pragma once

#ifndef ASS_PARSER_C_INTERNAL_USAGE
#error "This header is reserved for internal usage only"
#endif

#include "./diagnostics.h"
#include "./parser.h"

[[nodiscard]] double parse_str_as_double(ConstStrView value, MessageStruct* message_ptr,
                                         Diagnostics* diagnostics);

[[nodiscard]] size_t parse_str_as_unsigned_number(ConstStrView value, MessageStruct* message_ptr,
                                                  Diagnostics* diagnostics);

[[nodiscard]] bool parse_str_as_bool(ConstStrView value, MessageStruct* message_ptr);

[[nodiscard]] bool parse_str_as_str_bool(ConstStrView value, MessageStruct* message_ptr);

[[nodiscard]] AssColor parse_str_as_color(ConstStrView value, MessageStruct* message_ptr);

[[nodiscard]] BorderStyle parse_str_as_border_style(ConstStrView value, MessageStruct* message_ptr,
                                                    Diagnostics* diagnostics);

[[nodiscard]] AssAlignment parse_str_as_style_alignment(ConstStrView value,
                                                        MessageStruct* message_ptr,
                                                        Diagnostics* diagnostics);

[[nodiscard]] size_t parse_str_as_unsigned_number_with_option(ConstStrView value,
                                                              MessageStruct* message_ptr,
                                                              bool allow_number_truncating,
                                                              Diagnostics* diagnostics);

[[nodiscard]] MarginValue parse_str_as_margin_value(ConstStrView value, MessageStruct* message_ptr,
                                                    Diagnostics* diagnostics);

[[nodiscard]] AssTime parse_str_as_time(ConstStrView value, MessageStruct* message_ptr,
                                        Diagnostics* diagnostics);

[[nodiscard]] ScriptType parse_str_as_script_type(ConstStrView value, MessageStruct* message_ptr);

[[nodiscard]] WrapStyle parse_str_as_wrap_style(ConstStrView value, MessageStruct* message_ptr,
                                                Diagnostics* diagnostics);
