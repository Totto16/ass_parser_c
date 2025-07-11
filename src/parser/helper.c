
#include "./helper.h"

#include "../helper/log.h"
#include "../helper/macros.h"

#include <math.h>
#include <stdio.h>

void free_error_struct(ErrorStruct error) {
	if(error.dynamic) {
		free(error.message);
	}
}

[[nodiscard]] double parse_str_as_double(ConstStrView value, ErrorStruct* error_ptr) {

	StrView value_view = get_str_view_from_const_str_view(value);

	double final_value = 1.0F;

	if(str_view_starts_with_ascii(value_view, "-")) {
		final_value = 1.0F;
		if(!str_view_expect_ascii(&value_view, "-")) {
			*error_ptr = STATIC_ERROR("implementation error");
			return 0.0;
		}
	}

	ConstStrView prefix = {};
	if(!str_view_get_substring_by_char_delimiter(&value_view, &prefix, '.', false)) {

		size_t num =
		    parse_str_as_unsigned_number(get_const_str_view_from_str_view(value_view), error_ptr);

		if(error_ptr->message != NULL) {
			return 0.0;
		}

		*error_ptr = NO_ERROR();
		return (double)num;
	}

	size_t prefix_num = parse_str_as_unsigned_number(prefix, error_ptr);

	if(error_ptr->message != NULL) {
		return 0.0;
	}

	final_value = final_value * (double)prefix_num;

	if(str_view_is_eof(value_view)) {
		*error_ptr = NO_ERROR();
		return final_value;
	}

	ConstStrView suffix = {};
	if(!str_view_get_substring_until_eof(&value_view, &suffix)) {
		*error_ptr = STATIC_ERROR("implementation error");
		return 0.0;
	}

	size_t suffix_num = parse_str_as_unsigned_number(suffix, error_ptr);

	if(error_ptr->message != NULL) {
		return 0.0;
	}

	size_t suffix_power_of_10 = suffix.length;

	final_value = final_value + (double)suffix_num / (pow(10.0, (double)suffix_power_of_10));

	*error_ptr = NO_ERROR();
	return final_value;
}

[[nodiscard]] size_t parse_str_as_unsigned_number_with_option(ConstStrView value,
                                                              ErrorStruct* error_ptr,
                                                              bool allow_number_truncating) {
	size_t result = 0;

	for(size_t i = 0; i < value.length; ++i) {
		int32_t current_codepoint = value.start[i];

		if(current_codepoint < (unsigned char)'0' || current_codepoint > (unsigned char)'9') {

			char* value_name = get_normalized_string(value);

			if(!value_name) {
				*error_ptr = STATIC_ERROR("allocation error");
				return 0;
			}

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer, "error, not a valid decimal number: %s",
			                      value_name);

			if(allow_number_truncating) {
				// check if the number is not empty
				if(i > 0) {

					// check if its a double

					ErrorStruct local_error = NO_ERROR();

					double _unused = parse_str_as_double(value, &local_error);
					UNUSED(_unused);

					if(local_error.message == NULL) {

						LOG_MESSAGE(LogLevelWarn, "%s\n", result_buffer);

						free(result_buffer);
						free_error_struct(local_error);
						free(value_name);

						// return the truncated number
						*error_ptr = NO_ERROR();
						return result;
					}

					free_error_struct(local_error);
				}
			}

			free(value_name);

			*error_ptr = DYNAMIC_ERROR(result_buffer);
			return 0;
		}
		result = (result * 10) + (current_codepoint - '0');
	}

	*error_ptr = NO_ERROR();
	return result;
}

[[nodiscard]] size_t parse_str_as_unsigned_number(ConstStrView value, ErrorStruct* error_ptr) {
	return parse_str_as_unsigned_number_with_option(value, error_ptr, false);
}

[[nodiscard]] bool parse_str_as_bool(ConstStrView value, ErrorStruct* error_ptr) {

	if(str_view_eq_ascii(value, "-1")) {
		*error_ptr = NO_ERROR();
		return true;
	}

	if(str_view_eq_ascii(value, "0")) {
		*error_ptr = NO_ERROR();
		return false;
	}

	*error_ptr = STATIC_ERROR("error, not a valid bool");
	return false;
}

[[nodiscard]] bool parse_str_as_str_bool(ConstStrView value, ErrorStruct* error_ptr) {

	if(str_view_eq_ascii(value, "yes")) {
		*error_ptr = NO_ERROR();
		return true;
	}

	if(str_view_eq_ascii(value, "no")) {
		*error_ptr = NO_ERROR();
		return false;
	}

	*error_ptr = STATIC_ERROR("error, not a valid str bool");
	return false;
}

[[nodiscard]] AssColor parse_str_as_color(ConstStrView value, ErrorStruct* error_ptr) {

	AssColor color = {};

	if(value.length != 10) {
		*error_ptr = STATIC_ERROR("error, not a valid color, not correct length");
		return color;
	}

	StrView value_view = get_str_view_from_const_str_view(value);

	if(!str_view_expect_ascii(&value_view, "&H")) {
		*error_ptr = STATIC_ERROR("error, not a valid color, invalid prefix");
		return color;
	}

	for(size_t i = 0; i < 4; ++i) {

		uint8_t component = 0;

		for(size_t j = 0; j < 2; ++j) {

			int32_t current_codepoint = value_view.start[value_view.offset + (i * 2) + j];

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
				*error_ptr = STATIC_ERROR("error, not a valid hex color number");
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
				*error_ptr = STATIC_ERROR("error, not a valid color, implementation error");
				return color;
			}
		}
	}

	*error_ptr = NO_ERROR();
	return color;
}

[[nodiscard]] BorderStyle parse_str_as_border_style(ConstStrView value, ErrorStruct* error_ptr) {
	size_t num = parse_str_as_unsigned_number(value, error_ptr);

	if(error_ptr->message != NULL) {
		return BorderStyleOutline;
	}

	switch(num) {
		case 1: {
			*error_ptr = NO_ERROR();
			return BorderStyleOutline;
		}
		case 3: {
			*error_ptr = NO_ERROR();
			return BorderStyleOpaqueBox;
		}
		default: {
			*error_ptr = STATIC_ERROR("invalid border style value");
			return BorderStyleOutline;
		}
	}
}

[[nodiscard]] AssAlignment parse_str_as_style_alignment(ConstStrView value,
                                                        ErrorStruct* error_ptr) {
	size_t num = parse_str_as_unsigned_number(value, error_ptr);

	if(error_ptr->message != NULL) {
		return AssAlignmentBL;
	}

	switch(num) {
		// bottom
		case 1: {
			*error_ptr = NO_ERROR();
			return AssAlignmentBL;
		}
		case 2: {
			*error_ptr = NO_ERROR();
			return AssAlignmentBC;
		}
		case 3: {
			*error_ptr = NO_ERROR();
			return AssAlignmentBR;
		}
			// middle
		case 4: {
			*error_ptr = NO_ERROR();
			return AssAlignmentML;
		}
		case 5: {
			*error_ptr = NO_ERROR();
			return AssAlignmentMC;
		}
		case 6: {
			*error_ptr = NO_ERROR();
			return AssAlignmentMR;
		}
			// top
		case 7: {
			*error_ptr = NO_ERROR();
			return AssAlignmentTL;
		}
		case 8: {
			*error_ptr = NO_ERROR();
			return AssAlignmentTC;
		}
		case 9: {
			*error_ptr = NO_ERROR();
			return AssAlignmentTR;
		}
		default: {
			*error_ptr = STATIC_ERROR("invalid alignment value");
			return AssAlignmentBL;
		}
	}
}
