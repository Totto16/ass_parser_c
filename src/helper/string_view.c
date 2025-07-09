

#include "./string_view.h"
#include "./macros.h"

#include <string.h>
#include <utf8proc.h>

[[nodiscard]] StrView str_view_from_data(Codepoints data) {
	return (StrView){ .start = data.data, .offset = 0, .length = data.size };
}

[[nodiscard]] bool str_view_advance(StrView* str_view, size_t len) {

	if(str_view->offset + len > str_view->length) {
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

[[nodiscard]] static bool str_view_starts_with_ascii_sized(StrView str_view, const char* ascii_str,
                                                           size_t ascii_length) {

	if(ascii_length + str_view.offset > str_view.length) {
		return false;
	}

	for(size_t i = 0; i < ascii_length; ++i) {

		if(!is_utf8_char_eq_to_ascii_char(str_view.start[str_view.offset + i], ascii_str[i])) {
			return false;
		}
	}

	return true;
}

[[nodiscard]] bool str_view_starts_with_ascii(StrView str_view, const char* ascii_str) {

	size_t ascii_length = strlen(ascii_str);

	return str_view_starts_with_ascii_sized(str_view, ascii_str, ascii_length);
}

[[nodiscard]] bool str_view_expect_ascii(StrView* str_view, const char* ascii_str) {

	size_t ascii_length = strlen(ascii_str);

	if(!str_view_starts_with_ascii_sized(*str_view, ascii_str, ascii_length)) {
		return false;
	}

	if(!str_view_advance(str_view, ascii_length)) {
		return false;
	}

	return true;
}

[[nodiscard]] bool str_view_expect_newline(StrView* str_view) {
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

[[nodiscard]] bool newline_delimiter(int32_t code_point, void* data_ptr) {
	assert(data_ptr == NULL);

	if(is_utf8_char_eq_to_ascii_char(code_point, '\n')) {
		return true;
	}

	if(is_utf8_char_eq_to_ascii_char(code_point, '\r')) {
		return true;
	}

	return false;
}

[[nodiscard]] bool char_delimiter(int32_t code_point, void* data_ptr) {

	const char* data = (const char*)data_ptr;

	assert(strlen(data) == 1);

	return is_utf8_char_eq_to_ascii_char(code_point, data[0]);
}

[[nodiscard]] bool str_view_get_substring_by_delimiter(StrView* str_view, ConstStrView* result,
                                                       DelimiterFn delimit_fn, bool multiple,
                                                       void* data_ptr) {

	size_t size = 0;
	size_t collected_delimiters = 0;

	for(size_t i = 0;; ++i) {
		if(str_view->offset + i >= str_view->length) {
			if(multiple && collected_delimiters != 0) {
				break;
			}
			return false;
		}

		int32_t current_codepoint = str_view->start[str_view->offset + i];

		if(delimit_fn(current_codepoint, data_ptr)) {
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
	result->start = str_view->start + str_view->offset;

	return str_view_advance(str_view, size + collected_delimiters);
}

[[nodiscard]] StrView get_str_view_from_const_str_view(ConstStrView input) {

	return (StrView){ .offset = 0, .length = input.length, .start = input.start };
}

[[nodiscard]] bool str_view_eq_ascii(ConstStrView const_str_view, const char* ascii_str) {

	size_t ascii_length = strlen(ascii_str);

	if(const_str_view.length != ascii_length) {
		return false;
	}

	StrView str_view = get_str_view_from_const_str_view(const_str_view);

	return str_view_starts_with_ascii_sized(str_view, ascii_str, ascii_length);
}

[[nodiscard]] bool str_view_is_eof(StrView str_view) {
	return str_view.offset >= str_view.length;
}

[[nodiscard]] bool str_view_get_substring_until_eof(StrView* str_view, ConstStrView* result) {

	if(str_view->offset > str_view->length) {
		return false;
	}

	if(str_view->offset == str_view->length) {
		result->length = 0;
		result->start = str_view->start + str_view->offset;
		return true;
	}

	result->length = str_view->length - str_view->offset;
	result->start = str_view->start + str_view->offset;

	str_view->offset = str_view->length;

	return true;
}

[[nodiscard]] bool str_view_skip_while(StrView* str_view, DelimiterFn delimit_fn, void* data_ptr) {

	while(true) {
		if(str_view_is_eof(*str_view)) {
			return true;
		}

		int32_t current_codepoint = str_view->start[str_view->offset];

		if(!delimit_fn(current_codepoint, data_ptr)) {
			return true;
		}

		str_view->offset++;
	}

	return true;
}

[[nodiscard]] bool category_delimiter(int32_t code_point, void* data_ptr) {

	return utf8proc_category(code_point) == (*(utf8proc_category_t*)data_ptr);
}

[[nodiscard]] bool str_view_skip_optional_whitespace(StrView* str_view) {

	utf8proc_category_t cat = UTF8PROC_CATEGORY_ZS;

	return str_view_skip_while(str_view, category_delimiter, (void*)&cat);
}

[[nodiscard]] ConstStrView get_const_str_view_from_str_view(StrView input) {

	return (ConstStrView){ .start = input.start + input.offset,
		                   .length = input.length - input.offset };
}

[[nodiscard]] char* get_normalized_string(ConstStrView str_view) {
	return get_normalized_string_from_codepoints(
	    (Codepoints){ .data = str_view.start, .size = str_view.length });
}

[[nodiscard]] bool str_view_get_substring_by_amount(StrView* str_view, ConstStrView* result,
                                                    size_t amount) {

	if(str_view_is_eof(*str_view)) {
		return false;
	}

	if(str_view->offset + amount > str_view->length) {
		return false;
	}

	result->length = amount;
	result->start = str_view->start + str_view->offset;

	str_view->offset = str_view->offset + amount;

	return true;
}
