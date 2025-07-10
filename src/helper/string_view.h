

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

[[nodiscard]] StrView str_view_from_data(Codepoints data);

[[nodiscard]] bool str_view_advance(StrView* str_view, size_t len);

[[nodiscard]] bool str_view_starts_with_ascii(StrView str_view, const char* ascii_str);

[[nodiscard]] bool str_view_expect_ascii(StrView* str_view, const char* ascii_str);

[[nodiscard]] bool str_view_expect_newline(StrView* str_view);

typedef bool (*DelimiterFn)(int32_t code_point, void* data_ptr);

[[nodiscard]] bool newline_delimiter(int32_t code_point, void* data_ptr);

[[nodiscard]] bool char_delimiter(int32_t code_point, void* data_ptr);

[[nodiscard]] bool str_view_get_substring_by_delimiter(StrView* str_view, ConstStrView* result,
                                                       DelimiterFn delimit_fn, bool multiple,
                                                       void* data_ptr);

[[nodiscard]] StrView get_str_view_from_const_str_view(ConstStrView input);

[[nodiscard]] bool str_view_eq_ascii(ConstStrView const_str_view, const char* ascii_str);

[[nodiscard]] bool str_view_eq(ConstStrView const_str_view1, ConstStrView const_str_view2);

[[nodiscard]] bool str_view_is_eof(StrView str_view);

[[nodiscard]] bool str_view_get_substring_until_eof(StrView* str_view, ConstStrView* result);

[[nodiscard]] bool str_view_skip_while(StrView* str_view, DelimiterFn delimit_fn, void* data_ptr);

[[nodiscard]] bool str_view_skip_optional_whitespace(StrView* str_view);

[[nodiscard]] bool category_delimiter(int32_t code_point, void* data_ptr);

[[nodiscard]] ConstStrView get_const_str_view_from_str_view(StrView input);

[[nodiscard]] char* get_normalized_string(ConstStrView str_view);

[[nodiscard]] bool str_view_get_substring_by_amount(StrView* str_view, ConstStrView* result,
                                                    size_t amount);
