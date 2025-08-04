

#include "./string_view.h"
#include "./macros.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <utf8proc.h>

[[nodiscard]] bool is_empty_pos(FilePos pos) {
	return pos.line == EMPTY_POS_VAL && pos.column == EMPTY_POS_VAL;
}

[[nodiscard]] StrView str_view_from_data(Codepoints data) {
	return (StrView){ .start = data.data.data_const,
		              .length = data.size,
		              .position = (StrViewPos){ .offset = 0,
		                                        .file_pos = (FilePos){ .column = 0, .line = 0 } } };
}

inline static void str_view_advance_unchecked(StrView* str_view, size_t len) {

	str_view->position.offset += len;
	str_view->position.file_pos.column += len;
}

[[nodiscard]] bool str_view_advance(StrView* str_view, size_t len) {

	if(str_view->position.offset + len > str_view->length) {
		return false;
	}

	str_view_advance_unchecked(str_view, len);

	return true;
}

[[nodiscard]] bool str_view_advance_from_end(StrView* str_view, size_t len) {

	if(len > str_view->length) {
		return false;
	}

	if(str_view->position.offset > str_view->length - len) {
		return false;
	}

	str_view->length -= len;
	return true;
}

[[nodiscard]] static bool is_utf8_char_eq_to_ascii_char(int32_t utf8_char, char ascii_char) {
	// ascii_char is coneverted to unsigned char, so that no sign extension can occur (since
	// everything is casted to int, before doing a comparison)
	return utf8_char == (unsigned char)ascii_char;
}

#define ASCII_MAX_NUMBER 0x80

[[nodiscard]] static bool is_utf8_char_eq_to_ascii_char_case_insensitive(int32_t utf8_char,
                                                                         char ascii_char) {

	if(utf8_char >= ASCII_MAX_NUMBER) {
		return false;
	}

	return tolower((char)utf8_char) == tolower(ascii_char);
}

typedef bool (*CharCompareFn)(int32_t utf8_char, char ascii_char);

[[nodiscard]] static bool str_view_starts_with_sized(StrView str_view, const char* ascii_str,
                                                     size_t ascii_length,
                                                     CharCompareFn compare_fn) {

	if(ascii_length + str_view.position.offset > str_view.length) {
		return false;
	}

	for(size_t i = 0; i < ascii_length; ++i) {

		if(!compare_fn(str_view.start[str_view.position.offset + i], ascii_str[i])) {
			return false;
		}
	}

	return true;
}

[[nodiscard]] bool str_view_starts_with_ascii(StrView str_view, const char* ascii_str) {

	size_t ascii_length = strlen(ascii_str);

	return str_view_starts_with_sized(str_view, ascii_str, ascii_length,
	                                  is_utf8_char_eq_to_ascii_char);
}

[[nodiscard]] static bool str_view_ends_with_sized(StrView str_view, const char* ascii_str,
                                                   size_t ascii_length, CharCompareFn compare_fn) {

	if(ascii_length + str_view.position.offset > str_view.length) {
		return false;
	}

	size_t start_point = str_view.length - ascii_length;

	for(size_t i = 0; i < ascii_length; ++i) {
		size_t offset = start_point + i;

		if(!compare_fn(str_view.start[offset], ascii_str[i])) {
			return false;
		}
	}

	return true;
}

[[nodiscard]] bool str_view_ends_with_ascii(StrView str_view, const char* ascii_str) {
	size_t ascii_length = strlen(ascii_str);

	return str_view_ends_with_sized(str_view, ascii_str, ascii_length,
	                                is_utf8_char_eq_to_ascii_char);
}

[[nodiscard]] bool str_view_expect_ascii(StrView* str_view, const char* ascii_str) {

	size_t ascii_length = strlen(ascii_str);

	if(!str_view_starts_with_sized(*str_view, ascii_str, ascii_length,
	                               is_utf8_char_eq_to_ascii_char)) {
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
		default: UNREACHABLE();
	}
}

inline static void str_view_advance_line_count(StrView* str_view) {
	str_view->position.file_pos.line++;
	str_view->position.file_pos.column = 0;
}

[[nodiscard]] bool str_view_expect_newline(StrView* str_view, LineType line_type) {

	const char* to_compare = get_str_for_linetype(line_type);

	bool result = str_view_expect_ascii(str_view, to_compare);

	if(result) {
		str_view_advance_line_count(str_view);
	}

	return result;
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
	bool got_delimiter = false;

	for(size_t i = 0;; ++i) {
		if(str_view->position.offset + i >= str_view->length) {
			if(allow_eof) {
				size = i;
				break;
			}
			return false;
		}

		int32_t current_codepoint = str_view->start[str_view->position.offset + i];

		if(delimit_fn(current_codepoint, data_ptr)) {
			size = i;
			got_delimiter = true;
			break;
		}
	}

	result->length = size;
	result->start = str_view->start + str_view->position.offset;
	result->file_pos = str_view->position.file_pos;

	str_view_advance_unchecked(str_view, size + (got_delimiter ? 1 : 0));
	return true;
}

[[nodiscard]] StrView get_str_view_from_const_str_view(ConstStrView input) {

	return (StrView){ .length = input.length,
		              .start = input.start,
		              .position = { .offset = 0, .file_pos = input.file_pos } };
}

[[nodiscard]] bool str_view_eq_ascii_case_insensitive(ConstStrView const_str_view,
                                                      const char* ascii_str) {

	size_t ascii_length = strlen(ascii_str);

	if(const_str_view.length != ascii_length) {
		return false;
	}

	StrView str_view = get_str_view_from_const_str_view(const_str_view);

	return str_view_starts_with_sized(str_view, ascii_str, ascii_length,
	                                  is_utf8_char_eq_to_ascii_char_case_insensitive);
}

[[nodiscard]] bool str_view_eq_ascii(ConstStrView const_str_view, const char* ascii_str) {
	size_t ascii_length = strlen(ascii_str);

	if(const_str_view.length != ascii_length) {
		return false;
	}

	StrView str_view = get_str_view_from_const_str_view(const_str_view);

	return str_view_starts_with_sized(str_view, ascii_str, ascii_length,
	                                  is_utf8_char_eq_to_ascii_char);
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
	return str_view.position.offset >= str_view.length;
}

[[nodiscard]] bool str_view_get_substring_until_eof(StrView* str_view, ConstStrView* result,
                                                    bool process_newlines, LineType line_type) {

	if(str_view->position.offset > str_view->length) {
		return false;
	}

	if(str_view->position.offset == str_view->length) {
		result->length = 0;
		result->start = str_view->start + str_view->position.offset;
		result->file_pos = str_view->position.file_pos;
		return true;
	}

	result->length = str_view->length - str_view->position.offset;
	result->start = str_view->start + str_view->position.offset;
	result->file_pos = str_view->position.file_pos;

	str_view->position.offset = str_view->length;

	if(process_newlines) {
		ConstStrView temp = {};

		while(true) {
			bool result = str_view_get_substring_until_eol(str_view, &temp, line_type, true);
			if(!result) {
				return false;
			}
		}

	} else {
		str_view->position.file_pos = EMPTY_POS(); // stub out, so to make it immediately visible,
		                                           // that we didn't process newlines
	}

	return true;
}

[[nodiscard]] static bool str_view_skip_while(StrView* str_view, DelimiterFn delimit_fn,
                                              void* data_ptr) {

	while(true) {
		if(str_view_is_eof(*str_view)) {
			return true;
		}

		int32_t current_codepoint = str_view->start[str_view->position.offset];

		if(!delimit_fn(current_codepoint, data_ptr)) {
			return true;
		}

		str_view->position.offset++;
		str_view->position.file_pos.column++;
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

	return (ConstStrView){ .start = input.start + input.position.offset,
		                   .length = input.length - input.position.offset,
		                   .file_pos = input.position.file_pos };
}

[[nodiscard]] char* get_normalized_string(ConstStrView str_view) {
	return get_normalized_string_from_codepoints(
	    (RawCodepoints){ .data = str_view.start, .size = str_view.length });
}

[[nodiscard]] bool str_view_get_substring_by_amount(StrView* str_view, ConstStrView* result,
                                                    size_t amount) {

	if(str_view_is_eof(*str_view)) {
		return false;
	}

	if(str_view->position.offset + amount > str_view->length) {
		return false;
	}

	result->length = amount;
	result->start = str_view->start + str_view->position.offset;
	result->file_pos = str_view->position.file_pos;

	str_view_advance_unchecked(str_view, amount);
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

		bool final_result =
		    str_view_get_substring_by_char_delimiter(str_view, result, char_delimiter, allow_eof);

		if(final_result) {
			str_view_advance_line_count(str_view);
		}

		return final_result;
	}

	ASSERT(line_type == LineTypeCrLf, "linetype is crlf");

#define LINE_CHARACTER_SIZE 2

	const char line_characters[LINE_CHARACTER_SIZE] = { '\r', '\n' };

	size_t size = 0;
	bool got_delimiter = false;

	for(size_t i = 0;; ++i) {
		if(str_view->position.offset + i >= str_view->length) {
			if(allow_eof) {
				size = i;
				break;
			}
			return false;
		}

		int32_t current_codepoint = str_view->start[str_view->position.offset + i];

		if(is_utf8_char_eq_to_ascii_char(current_codepoint, line_characters[0])) {
			size = i;

			if(str_view->position.offset + i + 1 >= str_view->length) {
				// ended in the middle of the separator
				return false;
			}

			int32_t next_codepoint = str_view->start[str_view->position.offset + i + 1];

			if(!is_utf8_char_eq_to_ascii_char(next_codepoint, line_characters[1])) {
				// invalid next byte to separator
				return false;
			}

			got_delimiter = true;
			break;
		}
	}

	result->length = size;
	result->start = str_view->start + str_view->position.offset;
	result->file_pos = str_view->position.file_pos;

	str_view_advance_unchecked(str_view, size + (got_delimiter ? LINE_CHARACTER_SIZE : 0));
	str_view_advance_line_count(str_view);

	return true;
}

[[nodiscard]] LineType get_line_type(ConstStrView str_view, MessageStruct* msg_ptr) {

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

	UNREACHABLE();

error_cond:

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		*msg_ptr = STATIC_MESSAGE_STRUCT(message); \
		return LineTypeCrLf; \
	} while(false)

	char* result_buffer = NULL;
	FORMAT_STRING_PROPAGATE_ERROR(
	    &result_buffer,
	    "got multiple line endings in file: total: %zu, \\r\\n: %zu, \\r: %zu, \\n: %zu\n", sum,
	    counters[LINETYPE_CRLF_INDEX], counters[LINETYPE_CR_INDEX], counters[LINETYPE_LF_INDEX]);

	*msg_ptr = DYNAMIC_MESSAGE_STRUCT(result_buffer);

	return LineTypeCrLf;
}

#undef PROPAGATE_ERROR_IMPL
