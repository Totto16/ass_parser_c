

#define ASS_PARSER_C_INTERNAL_USAGE

#include "./helper.h"
#include "../helper/macros.h"

#undef ASS_PARSER_C_INTERNAL_USAGE

#include <math.h>
#include <stb/ds.h>
#include <stdio.h>

[[nodiscard]] double parse_str_as_double(ConstStrView value, MessageStruct* message_ptr,
                                         Diagnostics* diagnostics) {

	StrView value_view = get_str_view_from_const_str_view(value);

	double final_value = 1.0F;

	if(str_view_starts_with_ascii(value_view, "-")) {
		final_value = 1.0F;
		if(!str_view_expect_ascii(&value_view, "-")) {
			*message_ptr = STATIC_MESSAGE_STRUCT("implementation error");
			return 0.0;
		}
	}

	ConstStrView prefix = {};
	if(!str_view_get_substring_by_char_delimiter(&value_view, &prefix, '.', false)) {

		size_t num = parse_str_as_unsigned_number(get_const_str_view_from_str_view(value_view),
		                                          message_ptr, diagnostics);

		if(message_ptr->message != NULL) {
			return 0.0;
		}

		*message_ptr = EMPTY_MESSAGE_STRUCT();
		return (double)num;
	}

	size_t prefix_num = parse_str_as_unsigned_number(prefix, message_ptr, diagnostics);

	if(message_ptr->message != NULL) {
		return 0.0;
	}

	final_value = final_value * (double)prefix_num;

	if(str_view_is_eof(value_view)) {
		*message_ptr = EMPTY_MESSAGE_STRUCT();
		return final_value;
	}

	ConstStrView suffix = {};
	if(!str_view_get_substring_until_eof(&value_view, &suffix, false, NO_LINE_TYPE)) {
		*message_ptr = STATIC_MESSAGE_STRUCT("implementation error");
		return 0.0;
	}

	size_t suffix_num = parse_str_as_unsigned_number(suffix, message_ptr, diagnostics);

	if(message_ptr->message != NULL) {
		return 0.0;
	}

	size_t suffix_power_of_10 = suffix.length;

	final_value = final_value + (double)suffix_num / (pow(10.0, (double)suffix_power_of_10));

	*message_ptr = EMPTY_MESSAGE_STRUCT();
	return final_value;
}

[[nodiscard]] size_t parse_str_as_unsigned_number_with_option(ConstStrView value,
                                                              MessageStruct* message_ptr,
                                                              bool allow_number_truncating,
                                                              Diagnostics* diagnostics) {
	size_t result = 0;

	for(size_t i = 0; i < value.length; ++i) {
		int32_t current_codepoint = value.start[i];

		if(current_codepoint < (unsigned char)'0' || current_codepoint > (unsigned char)'9') {

			char* value_name = get_normalized_string(value);

			if(!value_name) {
				*message_ptr = STATIC_MESSAGE_STRUCT("allocation error");
				return 0;
			}

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer, "error, not a valid decimal number: %s",
			                      value_name);

			if(allow_number_truncating) {
				assert(diagnostics != NULL);
				// check if the number is not empty
				if(i > 0) {

					// check if its a double

					MessageStruct local_error = EMPTY_MESSAGE_STRUCT();

					double _unused = parse_str_as_double(value, &local_error, diagnostics);
					UNUSED(_unused);

					if(local_error.message == NULL) {

						DiagnosticEntry warning = { .type = DiagnosticTypeSimple,
							                        .data = { .simple = DYNAMIC_MESSAGE_STRUCT(
							                                      result_buffer) },
							                        .position = value.file_pos };

						stbds_arrput(diagnostics->entries, warning);

						free_message_struct(local_error);
						free(value_name);

						// return the truncated number
						*message_ptr = EMPTY_MESSAGE_STRUCT();
						return result;
					}

					free_message_struct(local_error);
				}
			}

			free(value_name);

			*message_ptr = DYNAMIC_MESSAGE_STRUCT(result_buffer);
			return 0;
		}
		result = (result * 10) + (current_codepoint - '0');
	}

	*message_ptr = EMPTY_MESSAGE_STRUCT();
	return result;
}

[[nodiscard]] size_t parse_str_as_unsigned_number(ConstStrView value, MessageStruct* message_ptr,
                                                  Diagnostics* diagnostics) {
	return parse_str_as_unsigned_number_with_option(value, message_ptr, false, diagnostics);
}

[[nodiscard]] bool parse_str_as_bool(ConstStrView value, MessageStruct* message_ptr) {

	if(str_view_eq_ascii(value, "-1")) {
		*message_ptr = EMPTY_MESSAGE_STRUCT();
		return true;
	}

	if(str_view_eq_ascii(value, "0")) {
		*message_ptr = EMPTY_MESSAGE_STRUCT();
		return false;
	}

	*message_ptr = STATIC_MESSAGE_STRUCT("error, not a valid bool");
	return false;
}

[[nodiscard]] bool parse_str_as_str_bool(ConstStrView value, MessageStruct* message_ptr) {

	if(str_view_eq_ascii_case_insensitive(value, "yes")) {
		*message_ptr = EMPTY_MESSAGE_STRUCT();
		return true;
	}

	if(str_view_eq_ascii_case_insensitive(value, "no")) {
		*message_ptr = EMPTY_MESSAGE_STRUCT();
		return false;
	}

	*message_ptr = STATIC_MESSAGE_STRUCT("error, not a valid str bool");
	return false;
}

[[nodiscard]] AssColor parse_str_as_color(ConstStrView value, MessageStruct* message_ptr) {

	AssColor color = {};

	if(value.length != 10) {
		*message_ptr = STATIC_MESSAGE_STRUCT("error, not a valid color, not correct length");
		return color;
	}

	StrView value_view = get_str_view_from_const_str_view(value);

	if(!str_view_expect_ascii(&value_view, "&H")) {
		*message_ptr = STATIC_MESSAGE_STRUCT("error, not a valid color, invalid prefix");
		return color;
	}

	for(size_t i = 0; i < 4; ++i) {

		uint8_t component = 0;

		for(size_t j = 0; j < 2; ++j) {

			int32_t current_codepoint = value_view.start[value_view.position.offset + (i * 2) + j];

			uint8_t current_value = 0;

			if(current_codepoint >= (unsigned char)'0' && current_codepoint <= (unsigned char)'9') {
				current_value = (current_codepoint - '0');
			} else if(current_codepoint >= (unsigned char)'a' &&
			          current_codepoint <= (unsigned char)'f') {
				current_value = (current_codepoint - 'a') + 10;
			} else if(current_codepoint >= (unsigned char)'A' &&
			          current_codepoint <= (unsigned char)'F') {
				current_value = (current_codepoint - 'A') + 10;
			} else {
				*message_ptr = STATIC_MESSAGE_STRUCT("error, not a valid hex color number");
				return color;
			}
			component = (component << 4) + current_value;
		}

		switch(i) {
			case 0: {
				color.a = component;
				break;
			}
			case 1: {
				color.b = component;
				break;
			}
			case 2: {
				color.g = component;
				break;
			}
			case 3: {
				color.r = component;
				break;
			}
			default: {
				*message_ptr =
				    STATIC_MESSAGE_STRUCT("error, not a valid color, implementation error");
				return color;
			}
		}
	}

	*message_ptr = EMPTY_MESSAGE_STRUCT();
	return color;
}

[[nodiscard]] BorderStyle parse_str_as_border_style(ConstStrView value, MessageStruct* message_ptr,
                                                    Diagnostics* diagnostics) {
	size_t num = parse_str_as_unsigned_number(value, message_ptr, diagnostics);

	if(message_ptr->message != NULL) {
		return BorderStyleOutline;
	}

	switch(num) {
		case 1: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return BorderStyleOutline;
		}
		case 3: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return BorderStyleOpaqueBox;
		}
		default: {
			*message_ptr = STATIC_MESSAGE_STRUCT("invalid border style value");
			return BorderStyleOutline;
		}
	}
}

[[nodiscard]] AssAlignment parse_str_as_style_alignment(ConstStrView value,
                                                        MessageStruct* message_ptr,
                                                        Diagnostics* diagnostics) {
	size_t num = parse_str_as_unsigned_number(value, message_ptr, diagnostics);

	if(message_ptr->message != NULL) {
		return AssAlignmentBL;
	}

	switch(num) {
		// bottom
		case 1: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return AssAlignmentBL;
		}
		case 2: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return AssAlignmentBC;
		}
		case 3: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return AssAlignmentBR;
		}
			// middle
		case 4: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return AssAlignmentML;
		}
		case 5: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return AssAlignmentMC;
		}
		case 6: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return AssAlignmentMR;
		}
			// top
		case 7: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return AssAlignmentTL;
		}
		case 8: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return AssAlignmentTC;
		}
		case 9: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return AssAlignmentTR;
		}
		default: {
			*message_ptr = STATIC_MESSAGE_STRUCT("invalid alignment value");
			return AssAlignmentBL;
		}
	}
}

[[nodiscard]] MarginValue parse_str_as_margin_value(ConstStrView value, MessageStruct* message_ptr,
                                                    Diagnostics* diagnostics) {
	MarginValue result = { .is_default = true };

	// spec: 4-figure Margin override. The values are in pixels. All zeroes means the default
	// margins defined by the style are used.

	// TODO: 0000 is used in older ass files, but is "0" also the default?
	if(str_view_eq_ascii(value, "0000")) {
		result.is_default = true;
		*message_ptr = EMPTY_MESSAGE_STRUCT();

		return result;
	}

	size_t num = parse_str_as_unsigned_number(value, message_ptr, diagnostics);
	if(message_ptr->message != NULL) {
		return result;
	}

	result.is_default = false;
	result.data.value = num;
	*message_ptr = EMPTY_MESSAGE_STRUCT();

	return result;
}

[[nodiscard]] AssTime parse_str_as_time(ConstStrView value, MessageStruct* message_ptr,
                                        Diagnostics* diagnostics) {

	// spec: in 0:00:00:00 format ie. Hrs:Mins:Secs:hundredths. Note that there is a single digit
	// for the hours!

	AssTime time = {};

	if(value.length != 10) {
		*message_ptr = STATIC_MESSAGE_STRUCT("error, not a valid time, not correct length");
		return time;
	}

	StrView value_view = get_str_view_from_const_str_view(value);

	{
		// hour

		ConstStrView hour_str = {};
		if(!str_view_get_substring_by_amount(&value_view, &hour_str, 1)) {
			*message_ptr = STATIC_MESSAGE_STRUCT("error, couldn't get hour value");
			return time;
		}

		size_t num = parse_str_as_unsigned_number(hour_str, message_ptr, diagnostics);

		if(message_ptr->message != NULL) {
			return time;
		}

		time.hour = (uint8_t)num;

		if(!str_view_expect_ascii(&value_view, ":")) {
			*message_ptr = STATIC_MESSAGE_STRUCT("error, not a valid time, missing ':'");
			return time;
		}
	}

	{
		// min

		ConstStrView min_str = {};
		if(!str_view_get_substring_by_amount(&value_view, &min_str, 2)) {
			*message_ptr = STATIC_MESSAGE_STRUCT("error, couldn't get min value");
			return time;
		}

		size_t num = parse_str_as_unsigned_number(min_str, message_ptr, diagnostics);

		if(message_ptr->message != NULL) {
			return time;
		}

		time.min = (uint8_t)num;

		if(!str_view_expect_ascii(&value_view, ":")) {
			*message_ptr = STATIC_MESSAGE_STRUCT("error, not a valid time, missing ':'");
			return time;
		}
	}

	{
		// sec

		ConstStrView sec_str = {};
		if(!str_view_get_substring_by_amount(&value_view, &sec_str, 2)) {
			*message_ptr = STATIC_MESSAGE_STRUCT("error, couldn't get sec value");
			return time;
		}

		size_t num = parse_str_as_unsigned_number(sec_str, message_ptr, diagnostics);

		if(message_ptr->message != NULL) {
			return time;
		}

		time.sec = (uint8_t)num;

		if(!str_view_expect_ascii(&value_view, ":")) {
			// note: specs defines ":" but in the wild we mostly get "."
			if(!str_view_expect_ascii(&value_view, ".")) {
				*message_ptr =
				    STATIC_MESSAGE_STRUCT("error, not a valid time, missing ':' or '.' after secs");
				return time;
			}
		}
	}

	{
		// hundred

		ConstStrView hundred_str = {};
		if(!str_view_get_substring_by_amount(&value_view, &hundred_str, 2)) {
			*message_ptr = STATIC_MESSAGE_STRUCT("error, couldn't get hundred value");
			return time;
		}

		size_t num = parse_str_as_unsigned_number(hundred_str, message_ptr, diagnostics);

		if(message_ptr->message != NULL) {
			return time;
		}

		time.hundred = (uint8_t)num;

		if(!str_view_is_eof(value_view)) {
			*message_ptr =
			    STATIC_MESSAGE_STRUCT("error, not a valid time, more data then expected");
			return time;
		}
	}

	*message_ptr = EMPTY_MESSAGE_STRUCT();
	return time;
}

[[nodiscard]] ScriptType parse_str_as_script_type(ConstStrView value, MessageStruct* message_ptr) {

	if(str_view_eq_ascii(value, "V4.00") || str_view_eq_ascii(value, "v4.00")) {
		*message_ptr = EMPTY_MESSAGE_STRUCT();
		return ScriptTypeV4;
	} else if(str_view_eq_ascii(value, "V4.00+") || str_view_eq_ascii(value, "v4.00+")) {
		*message_ptr = EMPTY_MESSAGE_STRUCT();
		return ScriptTypeV4Plus;
	} else {
		*message_ptr = STATIC_MESSAGE_STRUCT("invalid script type value");
		return ScriptTypeV4;
	}
}

[[nodiscard]] WrapStyle parse_str_as_wrap_style(ConstStrView value, MessageStruct* message_ptr,
                                                Diagnostics* diagnostics) {
	size_t num = parse_str_as_unsigned_number(value, message_ptr, diagnostics);

	if(message_ptr->message != NULL) {
		return WrapStyleSmart;
	}

	switch(num) {
		case 0: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return WrapStyleSmart;
		}
		case 1: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return WrapStyleEOL;
		}
		case 2: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return WrapStyleNoWrap;
		}
		case 3: {
			*message_ptr = EMPTY_MESSAGE_STRUCT();
			return WrapStyleSmartLow;
		}
		default: {
			*message_ptr = STATIC_MESSAGE_STRUCT("invalid wrap style value");
			return WrapStyleSmart;
		}
	}
}
