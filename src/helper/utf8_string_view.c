

#include "./utf8_string_view.h"

#include <string.h>

[[nodiscard]] Utf8StrView str_view_from_data(Utf8Data data) {
	return (Utf8StrView){ .start = data.data, .offset = 0, .length = data.size };
}

[[nodiscard]] bool str_view_advance(Utf8StrView* str_view, size_t len) {

	if(str_view->offset + len >= str_view->length) {
		return false;
	}

	str_view->offset = str_view->offset + len;

	return true;
}

[[nodiscard]] static inline bool is_utf8_char_eq_to_ascii_char(int32_t utf8_char, char ascii_char) {
	// ascii_char is coneverted to unsigned char, so that no sign extension can occur (since
	// everything is casted to int, before doing a comparison)
	return utf8_char == (unsigned char)ascii_char;
}

#include <stdio.h>

[[nodiscard]] static bool
str_view_starts_with_ascii_sized(Utf8StrView str_view, const char* ascii_str, size_t ascii_length) {

	if(ascii_length + str_view.offset >= str_view.length) {
		return false;
	}

	for(size_t i = 0; i < ascii_length; ++i) {

		if(!is_utf8_char_eq_to_ascii_char(str_view.start[str_view.offset + i], ascii_str[i])) {
			return false;
		}
	}

	return true;
}

[[nodiscard]] bool str_view_starts_with_ascii(Utf8StrView str_view, const char* ascii_str) {

	size_t ascii_length = strlen(ascii_str);

	return str_view_starts_with_ascii_sized(str_view, ascii_str, ascii_length);
}

[[nodiscard]] bool str_view_expect_ascii(Utf8StrView* str_view, const char* ascii_str) {

	size_t ascii_length = strlen(ascii_str);

	if(!str_view_starts_with_ascii_sized(*str_view, ascii_str, ascii_length)) {
		return false;
	}

	if(!str_view_advance(str_view, ascii_length)) {
		return false;
	}

	return true;
}

[[nodiscard]] bool str_view_expect_newline(Utf8StrView* str_view) {
	if(str_view_expect_ascii(str_view, "\n")) {
		return true;
	}

	if(str_view_expect_ascii(str_view, "\r\n")) {
		return true;
	}

	if(str_view_expect_ascii(str_view, "\r")) {
		return true;
	}

	return false;
}

[[nodiscard]] bool newline_delimiter(int32_t code_point) {

	if(is_utf8_char_eq_to_ascii_char(code_point, '\n')) {
		return true;
	}

	if(is_utf8_char_eq_to_ascii_char(code_point, '\r')) {
		return true;
	}

	return false;
}

[[nodiscard]] bool str_view_get_substring_by_delimiter(Utf8StrView* str_view, Utf8StrView* result,
                                                       DelimiterFn delimit_fn, bool multiple) {

	size_t size = 0;
	size_t collected_delimiters = 0;

	for(size_t i = 0;; ++i) {
		if(str_view->offset + i >= str_view->length) {
			return false;
		}

		int32_t current_codepoint = str_view->start[str_view->offset + i];

		if(delimit_fn(current_codepoint)) {
			if(collected_delimiters == 0) {
				size = i;
				if(!multiple) {
					collected_delimiters = 1;
					break;
				}
			}

			collected_delimiters++;
			continue;
		}

		if(collected_delimiters != 0) {
			break;
		}
	}

	result->length = size;
	result->offset = 0;
	result->start = str_view->start + str_view->offset;

	return str_view_advance(str_view, size + collected_delimiters);
}
