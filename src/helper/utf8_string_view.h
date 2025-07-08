

#pragma once

#include "./utf8_helper.h"

typedef struct {
	int32_t* start;
	size_t offset;
	size_t length;
} Utf8StrView;

typedef struct {
	int32_t* start;
	size_t length;
} ConstUtf8StrView;

[[nodiscard]] Utf8StrView str_view_from_data(Utf8Data data);

[[nodiscard]] bool str_view_advance(Utf8StrView* str_view, size_t len);

[[nodiscard]] bool str_view_starts_with_ascii(Utf8StrView str_view, const char* ascii_str);

[[nodiscard]] bool str_view_expect_ascii(Utf8StrView* str_view, const char* ascii_str);

[[nodiscard]] bool str_view_expect_newline(Utf8StrView* str_view);

typedef bool (*DelimiterFn)(int32_t code_point, void* data_ptr);

[[nodiscard]] bool newline_delimiter(int32_t code_point, void* data_ptr);

[[nodiscard]] bool char_delimiter(int32_t code_point, void* data_ptr);

[[nodiscard]] bool str_view_get_substring_by_delimiter(Utf8StrView* str_view,
                                                       ConstUtf8StrView* result,
                                                       DelimiterFn delimit_fn, bool multiple,
                                                       void* data_ptr);

[[nodiscard]] Utf8StrView get_str_view_from_const_str_view(ConstUtf8StrView input);

[[nodiscard]] bool str_view_eq_ascii(ConstUtf8StrView const_str_view, const char* ascii_str);

[[nodiscard]] bool str_view_is_eof(Utf8StrView str_view);
