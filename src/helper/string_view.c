

#include "./string_view.h"
#include "./macros.h"

#include <stdio.h>
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

#define LINE_ENDING_CRLF "\r\n"

[[nodiscard]] static const char* get_str_for_linetype(LineType line_type) {
	switch(line_type) {
		case LineTypeCrLf: return LINE_ENDING_CRLF;
		case LineTypeLf: return "\n";
		case LineTypeCr: return "\r";
		default: assert("UNREACHABLE");
	}
}

[[nodiscard]] bool str_view_expect_newline(StrView* str_view, LineType line_type) {

	const char* to_compare = get_str_for_linetype(line_type);

	return str_view_expect_ascii(str_view, to_compare);
}

[[nodiscard]] static bool char_delimiter(int32_t code_point, void* data_ptr) {

	char* data = (char*)data_ptr;

	return is_utf8_char_eq_to_ascii_char(code_point, *data);
}

typedef bool (*DelimiterFn)(int32_t code_point, void* data_ptr);

[[nodiscard]] static bool str_view_get_substring_by_delimiter(StrView* str_view,
                                                              ConstStrView* result,
                                                              DelimiterFn delimit_fn,
                                                              void* data_ptr, bool allow_eof) {

	size_t size = 0;
	bool got_delimter = false;

	for(size_t i = 0;; ++i) {
		if(str_view->offset + i >= str_view->length) {
			if(allow_eof) {
				size = i;
				break;
			}
			return false;
		}

		int32_t current_codepoint = str_view->start[str_view->offset + i];

		if(delimit_fn(current_codepoint, data_ptr)) {
			size = i;
			got_delimter = true;
			break;
		}
	}

	result->length = size;
	result->start = str_view->start + str_view->offset;

	return str_view_advance(str_view, size + (got_delimter ? 1 : 0));
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

[[nodiscard]] bool str_view_eq_str_view(ConstStrView const_str_view1,
                                        ConstStrView const_str_view2) {

	if(const_str_view1.length != const_str_view2.length) {
		return false;
	}

	for(size_t i = 0; i < const_str_view1.length; ++i) {

		if(const_str_view1.start[i] != const_str_view2.start[i]) {
			return false;
		}
	}

	return true;
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

[[nodiscard]] static bool str_view_skip_while(StrView* str_view, DelimiterFn delimit_fn,
                                              void* data_ptr) {

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

[[nodiscard]] bool str_view_get_substring_by_char_delimiter(StrView* str_view, ConstStrView* result,
                                                            char delimiter, bool allow_eof) {

	char local_char = delimiter;

	return str_view_get_substring_by_delimiter(str_view, result, char_delimiter, &local_char,
	                                           allow_eof);
}

[[nodiscard]] bool str_view_get_substring_until_eol(StrView* str_view, ConstStrView* result,
                                                    LineType line_type, bool allow_eof) {

	if(line_type == LineTypeCr || line_type == LineTypeLf) {

		char char_delimiter = line_type == LineTypeCr ? '\r' : '\n';

		return str_view_get_substring_by_char_delimiter(str_view, result, char_delimiter,
		                                                allow_eof);
	}

	assert(line_type == LineTypeCrLf);

#define LINE_CHARACTER_SIZE 2

	const char line_characters[LINE_CHARACTER_SIZE] = LINE_ENDING_CRLF;

	size_t size = 0;
	bool got_delimter = false;

	for(size_t i = 0;; ++i) {
		if(str_view->offset + i >= str_view->length) {
			if(allow_eof) {
				size = i;
				break;
			}
			return false;
		}

		int32_t current_codepoint = str_view->start[str_view->offset + i];

		if(is_utf8_char_eq_to_ascii_char(current_codepoint, line_characters[0])) {
			size = i;

			if(str_view->offset + i + 1 >= str_view->length) {
				// ended in the middle of the separator
				return false;
			}

			int32_t next_codepoint = str_view->start[str_view->offset + i + 1];

			if(!is_utf8_char_eq_to_ascii_char(next_codepoint, line_characters[1])) {
				// invalid next byte to separator
				return false;
			}

			got_delimter = true;
			break;
		}
	}

	result->length = size;
	result->start = str_view->start + str_view->offset;

	return str_view_advance(str_view, size + (got_delimter ? LINE_CHARACTER_SIZE : 0));
}

[[nodiscard]] LineType get_line_type(ConstStrView str_view, char** error_ptr) {

#define LINETYPE_CRLF_INDEX 0
#define LINETYPE_LF_INDEX 1
#define LINETYPE_CR_INDEX 2

	size_t counters[3] = { 0, 0, 0 };

	for(size_t i = 0; i < str_view.length; ++i) {
		int32_t codepoint = str_view.start[i];

		if(is_utf8_char_eq_to_ascii_char(codepoint, '\r')) {

			if(i + 1 >= str_view.length) {
				counters[LINETYPE_CR_INDEX]++;
				continue;
			}

			int32_t next_codepoint = str_view.start[i + 1];

			if(is_utf8_char_eq_to_ascii_char(next_codepoint, '\n')) {
				counters[LINETYPE_CRLF_INDEX]++;
				++i;
			} else {
				counters[LINETYPE_CR_INDEX]++;
			}
		}

		if(is_utf8_char_eq_to_ascii_char(codepoint, '\n')) {
			counters[LINETYPE_LF_INDEX]++;
		}
	}

	size_t sum = counters[0] + counters[1] + counters[2];

	if(counters[LINETYPE_CRLF_INDEX] != 0) {
		if(sum != counters[LINETYPE_CRLF_INDEX]) {
			goto error_cond;
		}

		return LineTypeCrLf;
	}

	if(counters[LINETYPE_LF_INDEX] != 0) {
		if(sum != counters[LINETYPE_LF_INDEX]) {
			goto error_cond;
		}

		return LineTypeLf;
	}

	if(counters[LINETYPE_CR_INDEX] != 0) {
		if(sum != counters[LINETYPE_CR_INDEX]) {
			goto error_cond;
		}

		return LineTypeCr;
	}

	*error_ptr = strdup("Unreachable");
	return LineTypeCrLf;

error_cond:

	char* result_buffer = NULL;
	FORMAT_STRING_DEFAULT(
	    &result_buffer,
	    "got multiple line endings in file: total: %zu, \\r\\n: %zu, \\r: %zu, \\n: %zu\n", sum,
	    counters[LINETYPE_CRLF_INDEX], counters[LINETYPE_CR_INDEX], counters[LINETYPE_LF_INDEX]);

	*error_ptr = result_buffer;

	return LineTypeCrLf;
}
