

#include "./parser.h"
#include "../helper/io.h"
#include "../helper/macros.h"
#include "../helper/utf8_helper.h"
#include "../helper/utf8_string_view.h"

#include <math.h>
#include <stb/ds.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef ConstUtf8StrView FinalStr;

typedef struct {
	int todo;
} AssScriptInfo;

typedef enum {
	AssStyleFormatName,
	AssStyleFormatFontname,
	AssStyleFormatFontsize,
	AssStyleFormatPrimaryColour,
	AssStyleFormatSecondaryColour,
	AssStyleFormatOutlineColour, // note: old name was TertiaryColour
	AssStyleFormatBackColour,
	AssStyleFormatBold,
	AssStyleFormatItalic,
	AssStyleFormatUnderline,
	AssStyleFormatStrikeOut,
	AssStyleFormatScaleX,
	AssStyleFormatScaleY,
	AssStyleFormatSpacing,
	AssStyleFormatAngle,
	AssStyleFormatBorderStyle,
	AssStyleFormatOutline,
	AssStyleFormatShadow,
	AssStyleFormatAlignment,
	AssStyleFormatMarginL,
	AssStyleFormatMarginR,
	AssStyleFormatMarginV,
	// AssStyleFormatAlphaLevel, //note: not supported in ass v4+
	AssStyleFormatEncoding,
} AssStyleFormat;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} AssColor;

typedef enum {
	BorderStyleOutline = 1,
	BorderStyleOpaqueBox = 3,
} BorderStyle;

typedef enum {
	// bottom
	AssAlignmentBL = 1,
	AssAlignmentBC,
	AssAlignmentBR,
	// middle
	AssAlignmentML,
	AssAlignmentMC,
	AssAlignmentMR,
	// top
	AssAlignmentTL,
	AssAlignmentTC,
	AssAlignmentTR,
} AssAlignment;

typedef struct {
	FinalStr name;
	FinalStr fontname;
	size_t fontsize;
	AssColor primary_colour;
	AssColor secondary_colour;
	AssColor outline_colour;
	AssColor back_colour;
	bool bold;
	bool italic;
	bool underline;
	bool strike_out;
	size_t scale_x;
	size_t scale_y;
	double spacing;
	double angle;
	BorderStyle border_style;
	double outline;
	double shadow;
	AssAlignment alignment;
	size_t margin_l;
	size_t margin_r;
	size_t margin_v;
	size_t encoding;
} AssStyleEntry;

typedef struct {
	STBDS_ARRAY(AssStyleEntry) entries;
} AssStyles;
typedef struct {
	int todo;
} AssEvents;
typedef struct {
	int todo;
} AssFonts;
typedef struct {
	int todo;
} AssGraphics;

typedef struct {
	AssScriptInfo script_info;
	AssStyles styles;
	AssEvents events;
	AssFonts fonts;
	AssGraphics graphics;
	Utf8Data allocated_data;
} AssResult;

struct AssParseResultImpl {
	bool is_error;
	union {
		const char* error;
		AssResult ok;
	} data;
};

[[nodiscard]] static SizedPtr get_data_from_source(AssSource source) {
	switch(source.type) {
		case AssSourceTypeFile: {
			return read_entire_file(source.data.file);
		}
		case AssSourceTypeStr: {
			return source.data.str;
		}
		default: return ptr_error("unknown source type");
	}
}

typedef enum {
	FileTypeUnknown,
	FileTypeUtf8,
	FileTypeUtf16BE,
	FileTypeUtf16LE,
	FileTypeUtf32BE,
	FileTypeUtf32LE,
} FileType;

// see: https://en.wikipedia.org/wiki/Byte_order_mark
[[nodiscard]] FileType determine_file_type(SizedPtr data) {

	uint8_t* bom = (uint8_t*)data.data;

	if(data.len < 4) {
		return FileTypeUnknown;
	}

	if(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
		return FileTypeUtf8;
	} else if(bom[0] == 0xFF && bom[1] == 0xFE && bom[2] == 0x00 && bom[3] == 0x00) {
		return FileTypeUtf32LE;
	} else if(bom[0] == 0x00 && bom[1] == 0x00 && bom[2] == 0xFE && bom[3] == 0xFF) {
		return FileTypeUtf32BE;
	} else if(bom[0] == 0xFF && bom[1] == 0xFE) {
		return FileTypeUtf16LE;
	} else if(bom[0] == 0xFE && bom[1] == 0xFF) {
		return FileTypeUtf16BE;
	} else {
		return FileTypeUnknown;
	}
}

#define RETURN_ERROR(err_str) \
	do { \
		result->is_error = true; \
		result->data.error = err_str; \
		return result; \
	} while(false)

[[nodiscard]] static bool str_view_starts_with_ascii_or_eof(Utf8StrView str_view,
                                                            const char* ascii_str) {

	if(str_view_starts_with_ascii(str_view, ascii_str)) {
		return true;
	}

	return str_view_is_eof(str_view);
}

[[nodiscard]] static const char*
parse_format_line_for_styles(Utf8StrView* line_view, STBDS_ARRAY(AssStyleFormat) * format_result) {

	while(!(str_view_is_eof(*line_view))) {

		if(!str_view_skip_optional_whitespace(line_view)) {
			return "skip whitespace error";
		}

		ConstUtf8StrView key = {};
		if(!str_view_get_substring_by_delimiter(line_view, &key, char_delimiter, true, ",")) {
			if(!str_view_get_substring_until_eof(line_view, &key)) {

				return "eof before comma in styles section format line";
			}
		}

		if(key.length == 0) {
			return "implementation error";
		}

		AssStyleFormat format = 0;

		if(str_view_eq_ascii(key, "Name")) {
			format = AssStyleFormatName;
		} else if(str_view_eq_ascii(key, "Fontname")) {
			format = AssStyleFormatFontname;
		} else if(str_view_eq_ascii(key, "Fontsize")) {
			format = AssStyleFormatFontsize;
		} else if(str_view_eq_ascii(key, "PrimaryColour")) {
			format = AssStyleFormatPrimaryColour;
		} else if(str_view_eq_ascii(key, "SecondaryColour")) {
			format = AssStyleFormatSecondaryColour;
		} else if(str_view_eq_ascii(key, "OutlineColour")) {
			format = AssStyleFormatOutlineColour;
		} else if(str_view_eq_ascii(key, "BackColour")) {
			format = AssStyleFormatBackColour;
		} else if(str_view_eq_ascii(key, "Bold")) {
			format = AssStyleFormatBold;
		} else if(str_view_eq_ascii(key, "Italic")) {
			format = AssStyleFormatItalic;
		} else if(str_view_eq_ascii(key, "Underline")) {
			format = AssStyleFormatUnderline;
		} else if(str_view_eq_ascii(key, "StrikeOut")) {
			format = AssStyleFormatStrikeOut;
		} else if(str_view_eq_ascii(key, "ScaleX")) {
			format = AssStyleFormatScaleX;
		} else if(str_view_eq_ascii(key, "ScaleY")) {
			format = AssStyleFormatScaleY;
		} else if(str_view_eq_ascii(key, "Spacing")) {
			format = AssStyleFormatSpacing;
		} else if(str_view_eq_ascii(key, "Angle")) {
			format = AssStyleFormatAngle;
		} else if(str_view_eq_ascii(key, "BorderStyle")) {
			format = AssStyleFormatBorderStyle;
		} else if(str_view_eq_ascii(key, "Outline")) {
			format = AssStyleFormatOutline;
		} else if(str_view_eq_ascii(key, "Shadow")) {
			format = AssStyleFormatShadow;
		} else if(str_view_eq_ascii(key, "Alignment")) {
			format = AssStyleFormatAlignment;
		} else if(str_view_eq_ascii(key, "MarginL")) {
			format = AssStyleFormatMarginL;
		} else if(str_view_eq_ascii(key, "MarginR")) {
			format = AssStyleFormatMarginR;
		} else if(str_view_eq_ascii(key, "MarginV")) {
			format = AssStyleFormatMarginV;
		} else if(str_view_eq_ascii(key, "Encoding")) {
			format = AssStyleFormatEncoding;
		} else {
			fprintf(stderr, "Format key: %s\n", get_normalized_string(key.start, key.length));
			return "unrecognized format key in format line in styles section";
		}

		stbds_arrput(*format_result, format);
	}

	return NULL;
}

[[nodiscard]] size_t parse_str_as_unsigned_number(ConstUtf8StrView value, const char** error_ptr) {
	size_t result = 0;

	for(size_t i = 0; i < value.length; ++i) {
		int32_t current_codepoint = value.start[i];

		if(current_codepoint < (unsigned char)'0' || current_codepoint > (unsigned char)'9') {
			fprintf(stderr, "whole value: %s\n", get_normalized_string(value.start, value.length));
			*error_ptr = "error, not a valid decimal number";
			return 0;
		}
		result = (result * 10) + (current_codepoint - '0');
	}

	*error_ptr = NULL;
	return result;
}

[[nodiscard]] bool parse_str_as_bool(ConstUtf8StrView value, const char** error_ptr) {

	if(str_view_eq_ascii(value, "-1")) {
		*error_ptr = NULL;
		return true;
	}

	if(str_view_eq_ascii(value, "0")) {
		*error_ptr = NULL;
		return false;
	}

	*error_ptr = "error, not a valid bool";
	return false;
}

[[nodiscard]] AssColor parse_str_as_color(ConstUtf8StrView value, const char** error_ptr) {

	AssColor color = {};

	if(value.length != 10) {
		*error_ptr = "error, not a valid color, not correct length";
		return color;
	}

	Utf8StrView value_view = get_str_view_from_const_str_view(value);

	if(!str_view_expect_ascii(&value_view, "&H")) {
		*error_ptr = "error, not a valid color, invalid prefix";
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
				*error_ptr = "error, not a valid hex color number";
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
				*error_ptr = "error, not a valid color, implementation error";
				return color;
			}
		}
	}

	*error_ptr = NULL;
	return color;
}

[[nodiscard]] double parse_str_as_double(ConstUtf8StrView value, const char** error_ptr) {

	Utf8StrView value_view = get_str_view_from_const_str_view(value);

	double final_value = 1.0F;

	if(str_view_starts_with_ascii(value_view, "-")) {
		final_value = 1.0F;
		if(!str_view_expect_ascii(&value_view, "-")) {
			*error_ptr = "implementation error";
			return 0.0;
		}
	}

	ConstUtf8StrView prefix = {};
	if(!str_view_get_substring_by_delimiter(&value_view, &prefix, char_delimiter, true, ".")) {

		size_t num =
		    parse_str_as_unsigned_number(get_const_str_view_from_str_view(value_view), error_ptr);

		if(*error_ptr != NULL) {
			return 0.0;
		}

		*error_ptr = NULL;
		return (double)num;
	}

	size_t prefix_num = parse_str_as_unsigned_number(prefix, error_ptr);

	if(*error_ptr != NULL) {
		return 0.0;
	}

	final_value = final_value * (double)prefix_num;

	if(str_view_is_eof(value_view)) {
		*error_ptr = NULL;
		return final_value;
	}

	ConstUtf8StrView suffix = {};
	if(!str_view_get_substring_until_eof(&value_view, &suffix)) {
		*error_ptr = "implementation error";
		return 0.0;
	}

	size_t suffix_num = parse_str_as_unsigned_number(suffix, error_ptr);

	if(*error_ptr != NULL) {
		return 0.0;
	}

	size_t suffix_power_of_10 = suffix.length;

	final_value = final_value + (double)suffix_num / (pow(10.0, (double)suffix_power_of_10));

	*error_ptr = NULL;
	return final_value;
}

[[nodiscard]] BorderStyle parse_str_as_border_style(ConstUtf8StrView value,
                                                    const char** error_ptr) {
	size_t num = parse_str_as_unsigned_number(value, error_ptr);

	if(*error_ptr != NULL) {
		return BorderStyleOutline;
	}

	switch(num) {
		case 1: {
			*error_ptr = NULL;
			return BorderStyleOutline;
		}
		case 3: {
			*error_ptr = NULL;
			return BorderStyleOpaqueBox;
		}
		default: {
			*error_ptr = "invalid border style value";
			return BorderStyleOutline;
		}
	}
}

[[nodiscard]] AssAlignment parse_str_as_style_alignment(ConstUtf8StrView value,
                                                        const char** error_ptr) {
	size_t num = parse_str_as_unsigned_number(value, error_ptr);

	if(*error_ptr != NULL) {
		return AssAlignmentBL;
	}

	switch(num) {
		// bottom
		case 1: {
			*error_ptr = NULL;
			return AssAlignmentBL;
		}
		case 2: {
			*error_ptr = NULL;
			return AssAlignmentBC;
		}
		case 3: {
			*error_ptr = NULL;
			return AssAlignmentBR;
		}
			// middle
		case 4: {
			*error_ptr = NULL;
			return AssAlignmentML;
		}
		case 5: {
			*error_ptr = NULL;
			return AssAlignmentMC;
		}
		case 6: {
			*error_ptr = NULL;
			return AssAlignmentMR;
		}
			// top
		case 7: {
			*error_ptr = NULL;
			return AssAlignmentTL;
		}
		case 8: {
			*error_ptr = NULL;
			return AssAlignmentTC;
		}
		case 9: {
			*error_ptr = NULL;
			return AssAlignmentTR;
		}
		default: {
			*error_ptr = "invalid alignment value";
			return AssAlignmentBL;
		}
	}
}

[[nodiscard]] static const char* parse_style_line_for_styles(Utf8StrView* line_view,
                                                             const STBDS_ARRAY(AssStyleFormat)
                                                                 const format_spec,
                                                             AssStyles* styles_result) {

	size_t field_size = stbds_arrlenu(format_spec);

	AssStyleEntry entry = {};

	if(!str_view_skip_optional_whitespace(line_view)) {
		return "skip whitespace error";
	}

	size_t i = 0;
	for(; !str_view_is_eof(*line_view); ++i) {

		ConstUtf8StrView value = {};
		if(!str_view_get_substring_by_delimiter(line_view, &value, char_delimiter, true, ",")) {
			if(!str_view_get_substring_until_eof(line_view, &value)) {

				return "eof before comma in styles section style line";
			}
		}

		if(value.length == 0) {
			return "implementation error";
		}

		const char* error = NULL;

		if(i >= field_size) {
			return "error, too many fields in the style line, the format line specified less";
		}

		AssStyleFormat format = format_spec[i];

		switch(format) {
			case AssStyleFormatName: {
				entry.name = value;
				break;
			}
			case AssStyleFormatFontname: {
				entry.fontname = value;
				break;
			}
			case AssStyleFormatFontsize: {
				entry.fontsize = parse_str_as_unsigned_number(value, &error);
				break;
			}
			case AssStyleFormatPrimaryColour: {
				entry.primary_colour = parse_str_as_color(value, &error);
				break;
			}
			case AssStyleFormatSecondaryColour: {
				entry.secondary_colour = parse_str_as_color(value, &error);
				break;
			}
			case AssStyleFormatOutlineColour: {
				entry.outline_colour = parse_str_as_color(value, &error);
				break;
			}
			case AssStyleFormatBackColour: {
				entry.back_colour = parse_str_as_color(value, &error);
				break;
			}
			case AssStyleFormatBold: {
				entry.bold = parse_str_as_bool(value, &error);
				break;
			}
			case AssStyleFormatItalic: {
				entry.italic = parse_str_as_bool(value, &error);
				break;
			}
			case AssStyleFormatUnderline: {
				entry.underline = parse_str_as_bool(value, &error);
				break;
			}
			case AssStyleFormatStrikeOut: {
				entry.strike_out = parse_str_as_bool(value, &error);
				break;
			}
			case AssStyleFormatScaleX: {
				entry.scale_x = parse_str_as_unsigned_number(value, &error);
				break;
			}
			case AssStyleFormatScaleY: {
				entry.scale_y = parse_str_as_unsigned_number(value, &error);
				break;
			}
			case AssStyleFormatSpacing: {
				entry.spacing = parse_str_as_double(value, &error);
				break;
			}
			case AssStyleFormatAngle: {
				entry.angle = parse_str_as_double(value, &error);
				break;
			}
			case AssStyleFormatBorderStyle: {
				entry.border_style = parse_str_as_border_style(value, &error);
				break;
			}
			case AssStyleFormatOutline: {
				entry.outline = parse_str_as_double(value, &error);
				break;
			}
			case AssStyleFormatShadow: {
				entry.shadow = parse_str_as_double(value, &error);
				break;
			}
			case AssStyleFormatAlignment: {
				entry.alignment = parse_str_as_style_alignment(value, &error);
				break;
			}
			case AssStyleFormatMarginL: {
				entry.margin_l = parse_str_as_unsigned_number(value, &error);
				break;
			}
			case AssStyleFormatMarginR: {
				entry.margin_r = parse_str_as_unsigned_number(value, &error);
				break;
			}
			case AssStyleFormatMarginV: {
				entry.margin_v = parse_str_as_unsigned_number(value, &error);
				break;
			}
			case AssStyleFormatEncoding: {
				entry.encoding = parse_str_as_unsigned_number(value, &error);
				break;
			}
		}

		if(error != NULL) {
			fprintf(stderr, "While parsing value: %s\n",
			        get_normalized_string(value.start, value.length));
			return error;
		}
	}

	if(i != field_size) {
		return "error, too few fields in the style line, the format line specified more";
	}

	stbds_arrput(styles_result->entries, entry);

	return NULL;
}

[[nodiscard]] static const char* parse_styles(AssStyles* ass_styles, Utf8StrView* data_view) {

	AssStyles styles = { .entries = STBDS_ARRAY_EMPTY };

	STBDS_ARRAY(AssStyleFormat) style_format = STBDS_ARRAY_EMPTY;

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstUtf8StrView line = {};
		if(!str_view_get_substring_by_delimiter(data_view, &line, newline_delimiter, true, NULL)) {
			return "eof before newline in parse styles";
		}

		// parse line
		{

			Utf8StrView line_view = get_str_view_from_const_str_view(line);

			ConstUtf8StrView field = {};
			if(!str_view_get_substring_by_delimiter(&line_view, &field, char_delimiter, false,
			                                        ":")) {
				return "end of line before ':' in line parsing in styles section";
			}

			if(str_view_eq_ascii(field, "Format")) {

				if(stbds_arrlenu(style_format) != 0) {
					return "multiple format fields detected in the styles section, this is not "
					       "allowed";
				}

				const char* format_line_error =
				    parse_format_line_for_styles(&line_view, &style_format);

				if(format_line_error != NULL) {
					return format_line_error;
				}

			} else if(str_view_eq_ascii(field, "Style")) {

				if(stbds_arrlenu(style_format) == 0) {
					return "no format line occurred before the style line in the styles section, "
					       "this is an error";
				}

				const char* format_line_error =
				    parse_style_line_for_styles(&line_view, style_format, &styles);

				if(format_line_error != NULL) {
					return format_line_error;
				}

			} else {
				fprintf(stderr, "Field: %s\n", get_normalized_string(field.start, field.length));
				return "unexpected field in styles section";
			}
		}

		// end of line parse

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

	stbds_arrfree(style_format);
	*ass_styles = styles;
	return NULL;
	// end of script info
}

[[nodiscard]] static const char* skip_section(Utf8StrView* data_view) {

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstUtf8StrView line = {};
		if(!str_view_get_substring_by_delimiter(data_view, &line, newline_delimiter, true, NULL)) {
			return "eof before newline in skipping section";
		}

		// skip the line
		UNUSED(line);

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

	return NULL;
	// end of script info
}

[[nodiscard]] static const char*
get_section_by_name(ConstUtf8StrView section_name, AssResult* ass_result, Utf8StrView* data_view) {

	if(str_view_eq_ascii(section_name, "Aegisub Project Garbage")) {
		return skip_section(data_view);
	}

	if(str_view_eq_ascii(section_name, "V4+ Styles")) {
		return parse_styles(&(ass_result->styles), data_view);
	}

	if(str_view_eq_ascii(section_name, "V4 Styles")) {
		return "v4 styles are not supported";
	}

	if(str_view_eq_ascii(section_name, "Events")) {
		return skip_section(data_view);
	}

	if(str_view_eq_ascii(section_name, "Fonts")) {
		return skip_section(data_view);
	}

	if(str_view_eq_ascii(section_name, "Graphics")) {
		return skip_section(data_view);
	}

	fprintf(stderr, "Section name: %s\n",
	        get_normalized_string(section_name.start, section_name.length));
	return "unrecognized section name";
}

[[nodiscard]] AssParseResult* parse_ass(AssSource source) {

	AssParseResult* result = (AssParseResult*)malloc(sizeof(AssParseResult));

	if(!result) {
		return NULL;
	}

	SizedPtr data = get_data_from_source(source);

	if(is_ptr_error(data)) {
		RETURN_ERROR(ptr_get_error(data));
	}

	FileType file_type = determine_file_type(data);

	if(file_type == FileTypeUnknown) {
		free_sized_ptr(data);
		RETURN_ERROR("unrecognized file, no BOM present");
	}

	if(file_type != FileTypeUtf8) {
		free_sized_ptr(data);
		RETURN_ERROR("only UTF-8 encoded files supported atm");
	}

	Utf8DataResult utf8_result = get_utf8_string(data.data, data.len);

	free_sized_ptr(data);

	if(utf8_result.has_error) {
		RETURN_ERROR(utf8_result.data.error);
	}

	Utf8Data final_data = utf8_result.data.result;

	Utf8StrView data_view = str_view_from_data(final_data);

	// NOTE: the bom byte is always just one codepoint
	if(!str_view_advance(&data_view, 1)) {
		RETURN_ERROR("couldn't skip bom bytes");
	}

	// parse script info
	AssScriptInfo script_info = { .todo = 0 };

	if(!str_view_expect_ascii(&data_view, "[Script Info]")) {
		RETURN_ERROR("first line must be the script info section");
	}

	if(!str_view_expect_newline(&data_view)) {
		RETURN_ERROR("expected newline");
	}

	while(!str_view_starts_with_ascii(data_view, "[")) {

		ConstUtf8StrView line = {};
		if(!str_view_get_substring_by_delimiter(&data_view, &line, newline_delimiter, true, NULL)) {
			RETURN_ERROR("eof before newline in parse script info");
		}

		// TODO: parse script info lines
		UNUSED(line);

		if(str_view_is_eof(data_view)) {
			break;
		}
	}

	// end of script info

	AssResult ass_result = { .allocated_data = final_data, .script_info = script_info };

	while(true) {

		// classify section, and then parsing that section

		if(!str_view_expect_ascii(&data_view, "[")) {
			RETURN_ERROR("implementation error");
		}

		ConstUtf8StrView section_name = {};
		if(!str_view_get_substring_by_delimiter(&data_view, &section_name, char_delimiter, false,
		                                        "]")) {
			RETURN_ERROR("script section not terminated by ']'");
		}

		if(!str_view_expect_newline(&data_view)) {
			RETURN_ERROR("no newline after section name");
		}

		const char* section_parse_result =
		    get_section_by_name(section_name, &ass_result, &data_view);

		if(section_parse_result != NULL) {
			RETURN_ERROR(section_parse_result);
		}

		if(str_view_is_eof(data_view)) {
			break;
		}
	}

	result->is_error = false;
	result->data.ok = ass_result;
	return result;
}

[[nodiscard]] bool parse_result_is_error(AssParseResult* result) {
	if(!result) {
		return true;
	}

	return result->is_error;
}

[[nodiscard]] const char* parse_result_get_error(AssParseResult* result) {
	if(!result) {
		return "allocation error";
	}

	return result->data.error;
}

static void free_ass_result(AssResult data) {
	free_utf8_data(data.allocated_data);
	stbds_arrfree(data.styles.entries);
}

void free_parse_result(AssParseResult* result) {
	if(!result->is_error) {
		free_ass_result(result->data.ok);
	}

	free(result);
}
