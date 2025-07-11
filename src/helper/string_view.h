

#pragma once

#include "./utf_helper.h"

typedef struct {
	int32_t* start;
	size_t offset;
	size_t length;
} StrView;

typedef struct {
	int32_t* start;
	size_t length;
} ConstStrView;

typedef enum : uint8_t {
	LineTypeCrLf,
	LineTypeLf,
	LineTypeCr,
} LineType;

[[nodiscard]] StrView str_view_from_data(Codepoints data);

[[nodiscard]] bool str_view_advance(StrView* str_view, size_t len);

[[nodiscard]] bool str_view_starts_with_ascii(StrView str_view, const char* ascii_str);

[[nodiscard]] bool str_view_expect_ascii(StrView* str_view, const char* ascii_str);

[[nodiscard]] bool str_view_expect_newline(StrView* str_view, LineType line_type);

[[nodiscard]] StrView get_str_view_from_const_str_view(ConstStrView input);

[[nodiscard]] bool str_view_eq_ascii(ConstStrView const_str_view, const char* ascii_str);

[[nodiscard]] bool str_view_eq_str_view(ConstStrView const_str_view1, ConstStrView const_str_view2);

[[nodiscard]] bool str_view_is_eof(StrView str_view);

[[nodiscard]] bool str_view_get_substring_until_eof(StrView* str_view, ConstStrView* result);

[[nodiscard]] bool str_view_skip_optional_whitespace(StrView* str_view);

[[nodiscard]] ConstStrView get_const_str_view_from_str_view(StrView input);

[[nodiscard]] char* get_normalized_string(ConstStrView str_view);

[[nodiscard]] bool str_view_get_substring_by_amount(StrView* str_view, ConstStrView* result,
                                                    size_t amount);

[[nodiscard]] bool str_view_get_substring_by_char_delimiter(StrView* str_view, ConstStrView* result,
                                                            char delimiter, bool allow_eof);

[[nodiscard]] bool str_view_get_substring_until_eol(StrView* str_view, ConstStrView* result,
                                                    LineType line_type, bool allow_eof);

[[nodiscard]] LineType get_line_type(ConstStrView str_view, char** error_ptr);
