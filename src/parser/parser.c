

#include "./parser.h"
#include "../helper/io.h"
#include "../helper/log.h"
#include "../helper/macros.h"
#include "../helper/utf8_helper.h"
#include "../helper/utf8_string_view.h"

#include <math.h>
#include <stb/ds.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef ConstUtf8StrView FinalStr;

typedef enum : uint8_t {
	ScriptTypeV4,
	ScriptTypeV4Plus,
} ScriptType;

typedef enum : uint8_t {
	WrapStyleSmart = 0,
	WrapStyleEOL,
	WrapStyleNoWrap,
	WrapStyleSmartLow,
} WrapStyle;

typedef struct {
	FinalStr title;
	FinalStr original_script;
	FinalStr original_translation;
	FinalStr original_editing;
	FinalStr original_timing;
	FinalStr synch_point;
	FinalStr script_updated_by;
	FinalStr update_details;
	ScriptType script_type;
	FinalStr collisions;
	size_t play_res_y;
	size_t play_res_x;
	FinalStr play_depth;
	FinalStr timer;
	WrapStyle wrap_style;
	// not documented, but present
	bool scaled_border_and_shadow;
	size_t video_aspect_ratio;
	size_t video_zoom;
	FinalStr ycbcr_matrix;
} AssScriptInfo;

typedef enum : uint8_t {
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

[[nodiscard]] const char* get_name_for_style_format(AssStyleFormat format) {
	switch(format) {
		case AssStyleFormatName: return "Name";
		case AssStyleFormatFontname: return "Fontname";
		case AssStyleFormatFontsize: return "Fontsize";
		case AssStyleFormatPrimaryColour: return "PrimaryColour";
		case AssStyleFormatSecondaryColour: return "SecondaryColour";
		case AssStyleFormatOutlineColour: return "OutlineColour";
		case AssStyleFormatBackColour: return "BackColour";
		case AssStyleFormatBold: return "Bold";
		case AssStyleFormatItalic: return "Italic";
		case AssStyleFormatUnderline: return "Underline";
		case AssStyleFormatStrikeOut: return "StrikeOut";
		case AssStyleFormatScaleX: return "ScaleX";
		case AssStyleFormatScaleY: return "ScaleY";
		case AssStyleFormatSpacing: return "Spacing";
		case AssStyleFormatAngle: return "Angle";
		case AssStyleFormatBorderStyle: return "BorderStyle";
		case AssStyleFormatOutline: return "Outline";
		case AssStyleFormatShadow: return "Shadow";
		case AssStyleFormatAlignment: return "Alignment";
		case AssStyleFormatMarginL: return "MarginL";
		case AssStyleFormatMarginR: return "MarginR";
		case AssStyleFormatMarginV: return "MarginV";
		case AssStyleFormatEncoding: return "Encoding";
		default: return "<unknown>";
	}
}

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} AssColor;

typedef enum : uint8_t {
	BorderStyleOutline = 1,
	BorderStyleOpaqueBox = 3,
} BorderStyle;

typedef enum : uint8_t {
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

typedef enum : uint8_t {
	AssEventFormatLayer,
	AssEventFormatStart,
	AssEventFormatEnd,
	AssEventFormatStyle,
	AssEventFormatName,
	AssEventFormatMarginL,
	AssEventFormatMarginR,
	AssEventFormatMarginV,
	AssEventFormatEffect,
	AssEventFormatText,
} AssEventFormat;

[[nodiscard]] const char* get_name_for_event_format(AssEventFormat format) {
	switch(format) {
		case AssEventFormatLayer: return "Layer";
		case AssEventFormatStart: return "Start";
		case AssEventFormatEnd: return "End";
		case AssEventFormatStyle: return "Style";
		case AssEventFormatName: return "Name";
		case AssEventFormatMarginL: return "MarginL";
		case AssEventFormatMarginR: return "MarginR";
		case AssEventFormatMarginV: return "MarginV";
		case AssEventFormatEffect: return "Effect";
		case AssEventFormatText: return "Text";
		default: return "<unknown>";
	}
}

typedef enum : uint8_t {
	EventTypeDialogue,
	EventTypeComment,
	EventTypePicture,
	EventTypeSound,
	EventTypeMovie,
	EventTypeCommand
} EventType;

typedef struct {
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t hundred;
} AssTime;

typedef struct {
	bool is_default;
	union {
		size_t value;
	} data;
} MarginValue;

typedef struct {
	// marks different event_types
	EventType type;
	// original fields
	size_t layer;
	AssTime start;
	AssTime end;
	FinalStr style;
	FinalStr name;
	MarginValue margin_l;
	MarginValue margin_r;
	MarginValue margin_v;
	FinalStr effect;
	FinalStr text;
} AssEventEntry;

typedef struct {
	STBDS_ARRAY(AssEventEntry) entries;
} AssEvents;
typedef struct {
	int todo;
} AssFonts;
typedef struct {
	int todo;
} AssGraphics;

STBDS_HASH_MAP_TYPE(char*, FinalStr, SectionFieldEntry);

typedef struct {
	STBDS_HASH_MAP(SectionFieldEntry) fields;
} ExtraSectionEntry;

STBDS_HASH_MAP_TYPE(char*, ExtraSectionEntry, ExtraSectionHashMapEntry);

typedef struct {
	STBDS_HASH_MAP(ExtraSectionHashMapEntry) entries;
} ExtraSections;

typedef struct {
	AssScriptInfo script_info;
	AssStyles styles;
	AssEvents events;
	AssFonts fonts;
	AssGraphics graphics;
	Utf8Data allocated_data;
	ExtraSections extra_sections;
} AssResult;

typedef struct {
	char* message;
	bool dynamic;
} ErrorStruct;

#define STATIC_ERROR(error) ((ErrorStruct){ .message = (char*)(error), .dynamic = false })

#define DYNAMIC_ERROR(error) ((ErrorStruct){ .message = (error), .dynamic = true })

#define NO_ERROR() STATIC_ERROR(NULL)

static void free_error_struct(ErrorStruct error) {
	if(error.dynamic) {
		free(error.message);
	}
}

struct AssParseResultImpl {
	bool is_error;
	union {
		ErrorStruct error;
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

typedef enum : uint8_t {
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

#define RETURN_ERROR(err) \
	do { \
		result->is_error = true; \
		result->data.error = err; \
		return result; \
	} while(false)

[[nodiscard]] static bool str_view_starts_with_ascii_or_eof(Utf8StrView str_view,
                                                            const char* ascii_str) {

	if(str_view_starts_with_ascii(str_view, ascii_str)) {
		return true;
	}

	return str_view_is_eof(str_view);
}

[[nodiscard]] static ErrorStruct
parse_format_line_for_styles(Utf8StrView* line_view, STBDS_ARRAY(AssStyleFormat) * format_result) {

	while(!(str_view_is_eof(*line_view))) {

		if(!str_view_skip_optional_whitespace(line_view)) {
			return STATIC_ERROR("skip whitespace error");
		}

		ConstUtf8StrView key = {};
		if(!str_view_get_substring_by_delimiter(line_view, &key, char_delimiter, false, ",")) {
			if(!str_view_get_substring_until_eof(line_view, &key)) {

				return STATIC_ERROR("eof before comma in styles section format line");
			}
		}

		if(key.length == 0) {
			return STATIC_ERROR("implementation error");
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

			char* key_name = get_normalized_string(key);

			if(!key_name) {
				return STATIC_ERROR("allocation error");
			}

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer,
			                      "unrecognized format key %s in format line in styles section",
			                      key_name);

			free(key_name);

			return DYNAMIC_ERROR(result_buffer);
		}

		stbds_arrput(*format_result, format);
	}

	return NO_ERROR();
}

// TODO: tuck this into a header file and seperate translation units
[[nodiscard]] double parse_str_as_double(ConstUtf8StrView value, ErrorStruct* error_ptr);

[[nodiscard]] size_t parse_str_as_unsigned_number_with_option(ConstUtf8StrView value,
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

[[nodiscard]] size_t parse_str_as_unsigned_number(ConstUtf8StrView value, ErrorStruct* error_ptr) {
	return parse_str_as_unsigned_number_with_option(value, error_ptr, false);
}

[[nodiscard]] bool parse_str_as_bool(ConstUtf8StrView value, ErrorStruct* error_ptr) {

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

[[nodiscard]] bool parse_str_as_str_bool(ConstUtf8StrView value, ErrorStruct* error_ptr) {

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

[[nodiscard]] AssColor parse_str_as_color(ConstUtf8StrView value, ErrorStruct* error_ptr) {

	AssColor color = {};

	if(value.length != 10) {
		*error_ptr = STATIC_ERROR("error, not a valid color, not correct length");
		return color;
	}

	Utf8StrView value_view = get_str_view_from_const_str_view(value);

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

[[nodiscard]] double parse_str_as_double(ConstUtf8StrView value, ErrorStruct* error_ptr) {

	Utf8StrView value_view = get_str_view_from_const_str_view(value);

	double final_value = 1.0F;

	if(str_view_starts_with_ascii(value_view, "-")) {
		final_value = 1.0F;
		if(!str_view_expect_ascii(&value_view, "-")) {
			*error_ptr = STATIC_ERROR("implementation error");
			return 0.0;
		}
	}

	ConstUtf8StrView prefix = {};
	if(!str_view_get_substring_by_delimiter(&value_view, &prefix, char_delimiter, false, ".")) {

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

	ConstUtf8StrView suffix = {};
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

[[nodiscard]] BorderStyle parse_str_as_border_style(ConstUtf8StrView value,
                                                    ErrorStruct* error_ptr) {
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

[[nodiscard]] AssAlignment parse_str_as_style_alignment(ConstUtf8StrView value,
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

[[nodiscard]] static ErrorStruct parse_style_line_for_styles(Utf8StrView* line_view,
                                                             const STBDS_ARRAY(AssStyleFormat)
                                                                 const format_spec,
                                                             AssStyles* styles_result,
                                                             ParseSettings settings) {

	size_t field_size = stbds_arrlenu(format_spec);

	AssStyleEntry entry = {};

	if(!str_view_skip_optional_whitespace(line_view)) {
		return STATIC_ERROR("skip whitespace error");
	}

	size_t i = 0;
	for(; !str_view_is_eof(*line_view); ++i) {

		ConstUtf8StrView value = {};
		if(!str_view_get_substring_by_delimiter(line_view, &value, char_delimiter, false, ",")) {
			if(!str_view_get_substring_until_eof(line_view, &value)) {

				return STATIC_ERROR("eof before comma in styles section style line");
			}
		}

		if(i >= field_size) {
			return STATIC_ERROR(
			    "error, too many fields in the style line, the format line specified less");
		}

		ErrorStruct error = NO_ERROR();

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
				entry.fontsize = parse_str_as_unsigned_number_with_option(
				    value, &error, settings.strict_settings.allow_number_truncating);
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
			default: {
				error = STATIC_ERROR("unreachable");
			}
		}

		if(error.message != NULL) {

			char* value_name = get_normalized_string(value);

			if(!value_name) {
				return STATIC_ERROR("allocation error");
			}

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer, "While parsing field '%s' with value '%s': %s",
			                      get_name_for_style_format(format), value_name, error.message);

			free(value_name);
			free_error_struct(error);
			return DYNAMIC_ERROR(result_buffer);
		}
	}

	if(i != field_size) {
		return STATIC_ERROR(
		    "error, too few fields in the style line, the format line specified more");
	}

	stbds_arrput(styles_result->entries, entry);

	return NO_ERROR();
}

[[nodiscard]] static ErrorStruct parse_styles(AssStyles* ass_styles, Utf8StrView* data_view,
                                              ParseSettings settings) {

	AssStyles styles = { .entries = STBDS_ARRAY_EMPTY };

	STBDS_ARRAY(AssStyleFormat) style_format = STBDS_ARRAY_EMPTY;

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstUtf8StrView line = {};
		if(!str_view_get_substring_by_delimiter(data_view, &line, newline_delimiter, true, NULL)) {
			return STATIC_ERROR("eof before newline in parse styles");
		}

		// parse line
		{

			Utf8StrView line_view = get_str_view_from_const_str_view(line);

			ConstUtf8StrView field = {};
			if(!str_view_get_substring_by_delimiter(&line_view, &field, char_delimiter, false,
			                                        ":")) {
				return STATIC_ERROR("end of line before ':' in line parsing in styles section");
			}

			if(str_view_eq_ascii(field, "Format")) {

				if(stbds_arrlenu(style_format) != 0) {
					return STATIC_ERROR(
					    "multiple format fields detected in the styles section, this is not "
					    "allowed");
				}

				ErrorStruct format_line_error =
				    parse_format_line_for_styles(&line_view, &style_format);

				if(format_line_error.message != NULL) {
					return format_line_error;
				}

			} else if(str_view_eq_ascii(field, "Style")) {

				if(stbds_arrlenu(style_format) == 0) {
					return STATIC_ERROR(
					    "no format line occurred before the style line in the styles section, "
					    "this is an error");
				}

				ErrorStruct style_parse_error =
				    parse_style_line_for_styles(&line_view, style_format, &styles, settings);

				if(style_parse_error.message != NULL) {
					return style_parse_error;
				}

			} else {

				char* field_name = get_normalized_string(field);

				if(!field_name) {
					return STATIC_ERROR("allocation error");
				}

				char* result_buffer = NULL;
				FORMAT_STRING_DEFAULT(&result_buffer, "unexpected field in styles section: '%s'",
				                      field_name);

				if(settings.strict_settings.allow_additional_fields) {
					LOG_MESSAGE(LogLevelWarn, "%s\n", result_buffer);

					free(result_buffer);
					free(field_name);
					continue;
				}

				free(field_name);
				return DYNAMIC_ERROR(result_buffer);
			}
		}

		// end of line parse

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

	stbds_arrfree(style_format);
	*ass_styles = styles;
	return NO_ERROR();
	// end of script info
}

[[nodiscard]] ScriptType parse_str_as_script_type(ConstUtf8StrView value, ErrorStruct* error_ptr) {

	if(str_view_eq_ascii(value, "V4.00") || str_view_eq_ascii(value, "v4.00")) {
		*error_ptr = NO_ERROR();
		return ScriptTypeV4;
	} else if(str_view_eq_ascii(value, "V4.00+") || str_view_eq_ascii(value, "v4.00+")) {
		*error_ptr = NO_ERROR();
		return ScriptTypeV4Plus;
	} else {
		*error_ptr = STATIC_ERROR("invalid script type value");
		return ScriptTypeV4;
	}
}

[[nodiscard]] WrapStyle parse_str_as_wrap_style(ConstUtf8StrView value, ErrorStruct* error_ptr) {
	size_t num = parse_str_as_unsigned_number(value, error_ptr);

	if(error_ptr->message != NULL) {
		return WrapStyleSmart;
	}

	switch(num) {
		case 0: {
			*error_ptr = NO_ERROR();
			return WrapStyleSmart;
		}
		case 1: {
			*error_ptr = NO_ERROR();
			return WrapStyleEOL;
		}
		case 2: {
			*error_ptr = NO_ERROR();
			return WrapStyleNoWrap;
		}
		case 3: {
			*error_ptr = NO_ERROR();
			return WrapStyleSmartLow;
		}
		default: {
			*error_ptr = STATIC_ERROR("invalid wrap style value");
			return WrapStyleSmart;
		}
	}
}

[[nodiscard]] static ErrorStruct parse_script_info(AssScriptInfo* script_info_result,
                                                   Utf8StrView* data_view, ParseSettings settings) {

	AssScriptInfo script_info = {};

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstUtf8StrView line = {};
		if(!str_view_get_substring_by_delimiter(data_view, &line, newline_delimiter, true, NULL)) {
			return STATIC_ERROR("eof before newline in parse script style");
		}

		// parse line
		{

			Utf8StrView line_view = get_str_view_from_const_str_view(line);

			if(str_view_starts_with_ascii(line_view, ";")) {
				continue;
			}

			ConstUtf8StrView field = {};
			if(!str_view_get_substring_by_delimiter(&line_view, &field, char_delimiter, false,
			                                        ":")) {
				return STATIC_ERROR(
				    "end of line before ':' in line parsing in script info section");
			}

			if(!str_view_skip_optional_whitespace(&line_view)) {
				return STATIC_ERROR("skip whitespace error");
			}

			ConstUtf8StrView value = get_const_str_view_from_str_view(line_view);

			ErrorStruct error = NO_ERROR();

			if(str_view_eq_ascii(field, "Title")) {
				script_info.title = value;
			} else if(str_view_eq_ascii(field, "Original Script")) {
				script_info.original_script = value;
			} else if(str_view_eq_ascii(field, "Original Translation")) {
				script_info.original_translation = value;
			} else if(str_view_eq_ascii(field, "Original Editing")) {
				script_info.original_editing = value;
			} else if(str_view_eq_ascii(field, "Original Timing")) {
				script_info.original_timing = value;
			} else if(str_view_eq_ascii(field, "Synch Point")) {
				script_info.synch_point = value;
			} else if(str_view_eq_ascii(field, "Update Details")) {
				script_info.update_details = value;
			} else if(str_view_eq_ascii(field, "ScriptType") ||
			          str_view_eq_ascii(field, "Script Type")) {
				script_info.script_type = parse_str_as_script_type(value, &error);
			} else if(str_view_eq_ascii(field, "Collisions")) {
				script_info.collisions = value;
			} else if(str_view_eq_ascii(field, "PlayResY")) {
				script_info.play_res_y = parse_str_as_unsigned_number(value, &error);
				;
			} else if(str_view_eq_ascii(field, "PlayResX")) {
				script_info.play_res_x = parse_str_as_unsigned_number(value, &error);
				;
			} else if(str_view_eq_ascii(field, "PlayDepth")) {
				script_info.play_depth = value;
			} else if(str_view_eq_ascii(field, "Timer")) {
				script_info.timer = value;
			} else if(str_view_eq_ascii(field, "WrapStyle")) {
				script_info.wrap_style = parse_str_as_wrap_style(value, &error);
			} else if(str_view_eq_ascii(field, "ScaledBorderAndShadow")) {
				script_info.scaled_border_and_shadow = parse_str_as_str_bool(value, &error);
			} else if(str_view_eq_ascii(field, "Video Aspect Ratio")) {
				script_info.video_aspect_ratio = parse_str_as_unsigned_number(value, &error);
			} else if(str_view_eq_ascii(field, "Video Zoom")) {
				script_info.video_zoom = parse_str_as_unsigned_number(value, &error);
			} else if(str_view_eq_ascii(field, "YCbCr Matrix")) {
				script_info.ycbcr_matrix = value;
			} else {

				char* field_name = get_normalized_string(field);

				if(!field_name) {
					return STATIC_ERROR("allocation error");
				}

				char* result_buffer = NULL;
				FORMAT_STRING_DEFAULT(&result_buffer,
				                      "unexpected field in script info section: '%s'", field_name);

				if(settings.strict_settings.allow_additional_fields) {
					LOG_MESSAGE(LogLevelWarn, "%s\n", result_buffer);

					free(result_buffer);
					free(field_name);
					continue;
				}

				free(field_name);
				return DYNAMIC_ERROR(result_buffer);
			}

			if(error.message != NULL) {

				char* field_name = get_normalized_string(field);

				if(!field_name) {
					return STATIC_ERROR("allocation error");
				}

				char* value_name = get_normalized_string(value);

				if(!value_name) {
					return STATIC_ERROR("allocation error");
				}

				char* result_buffer = NULL;
				FORMAT_STRING_DEFAULT(&result_buffer,
				                      "While parsing field '%s' with value '%s': %s", field_name,
				                      value_name, error.message);

				free_error_struct(error);
				free(field_name);
				free(value_name);
				return DYNAMIC_ERROR(result_buffer);
			}
		}

		// end of line parse

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

	// check script info
	{

		if(script_info.script_type != ScriptTypeV4Plus) {
			return STATIC_ERROR("only scrypt type v4+ is supported");
		}

		// TODO: initialize title and original script
		// <untitled>
		// <unknown>
	}

	*script_info_result = script_info;
	return NO_ERROR();
	// end of script info
}

[[nodiscard]] static ErrorStruct skip_section(Utf8StrView* data_view) {

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstUtf8StrView line = {};
		if(!str_view_get_substring_by_delimiter(data_view, &line, newline_delimiter, true, NULL)) {
			return STATIC_ERROR("eof before newline in skipping section");
		}

		// skip the line
		UNUSED(line);

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

	return NO_ERROR();
}

[[nodiscard]] static ErrorStruct extra_section(ConstUtf8StrView section_name,
                                               Utf8StrView* data_view,
                                               ExtraSections* extra_sections) {

	char* section_name_str = get_normalized_string(section_name);

	if(section_name_str == NULL) {
		return STATIC_ERROR("alloc error");
	}

	ExtraSectionHashMapEntry extra_section = { .key = section_name_str,
		                                       .value = { .fields = STBDS_HASH_MAP_EMPTY } };

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstUtf8StrView line = {};
		if(!str_view_get_substring_by_delimiter(data_view, &line, newline_delimiter, true, NULL)) {
			return STATIC_ERROR("eof before newline in skipping section");
		}

		{

			SectionFieldEntry field_entry = {};

			Utf8StrView line_view = get_str_view_from_const_str_view(line);

			ConstUtf8StrView field = {};
			if(!str_view_get_substring_by_delimiter(&line_view, &field, char_delimiter, false,
			                                        ":")) {
				return STATIC_ERROR("end of line before ':' in line parsing in extra section");
			}

			if(!str_view_skip_optional_whitespace(&line_view)) {
				return STATIC_ERROR("skip whitespace error");
			}

			ConstUtf8StrView key = {};

			if(!str_view_get_substring_until_eof(&line_view, &key)) {
				return STATIC_ERROR("eof error");
			}

			field_entry.key = get_normalized_string(field);
			field_entry.value = key;

			stbds_shputs(extra_section.value.fields, field_entry);
		}

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

	stbds_shputs(extra_sections->entries, extra_section);

	return NO_ERROR();
}

[[nodiscard]] static ErrorStruct
parse_format_line_for_events(Utf8StrView* line_view, STBDS_ARRAY(AssEventFormat) * format_result) {

	while(!(str_view_is_eof(*line_view))) {

		if(!str_view_skip_optional_whitespace(line_view)) {
			return STATIC_ERROR("skip whitespace error");
		}

		ConstUtf8StrView key = {};
		if(!str_view_get_substring_by_delimiter(line_view, &key, char_delimiter, false, ",")) {
			if(!str_view_get_substring_until_eof(line_view, &key)) {

				return STATIC_ERROR("eof before comma in events section format line");
			}
		}

		if(key.length == 0) {
			return STATIC_ERROR("implementation error");
		}

		AssEventFormat format = 0;

		if(str_view_eq_ascii(key, "Layer")) {
			format = AssEventFormatLayer;
		} else if(str_view_eq_ascii(key, "Start")) {
			format = AssEventFormatStart;
		} else if(str_view_eq_ascii(key, "End")) {
			format = AssEventFormatEnd;
		} else if(str_view_eq_ascii(key, "Style")) {
			format = AssEventFormatStyle;
		} else if(str_view_eq_ascii(key, "Name")) {
			format = AssEventFormatName;
		} else if(str_view_eq_ascii(key, "MarginL")) {
			format = AssEventFormatMarginL;
		} else if(str_view_eq_ascii(key, "MarginR")) {
			format = AssEventFormatMarginR;
		} else if(str_view_eq_ascii(key, "MarginV")) {
			format = AssEventFormatMarginV;
		} else if(str_view_eq_ascii(key, "Effect")) {
			format = AssEventFormatEffect;
		} else if(str_view_eq_ascii(key, "Text")) {
			format = AssEventFormatText;
		} else {

			char* key_name = get_normalized_string(key);

			if(!key_name) {
				return STATIC_ERROR("allocation error");
			}

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer,
			                      "unrecognized format key %s in format line in events section",
			                      key_name);

			free(key_name);
			return DYNAMIC_ERROR(result_buffer);
		}

		stbds_arrput(*format_result, format);
	}

	return NO_ERROR();
}

[[nodiscard]] MarginValue parse_str_as_margin_value(ConstUtf8StrView value,
                                                    ErrorStruct* error_ptr) {
	MarginValue result = { .is_default = true };

	// spec: 4-figure Margin override. The values are in pixels. All zeroes means the default
	// margins defined by the style are used.

	// TODO: 0000 is used in older ass files, but is "0" also the default?
	if(str_view_eq_ascii(value, "0000")) {
		result.is_default = true;
		*error_ptr = NO_ERROR();

		return result;
	}

	size_t num = parse_str_as_unsigned_number(value, error_ptr);
	if(error_ptr->message != NULL) {
		return result;
	}

	result.is_default = false;
	result.data.value = num;
	*error_ptr = NO_ERROR();

	return result;
}

[[nodiscard]] AssTime parse_str_as_time(ConstUtf8StrView value, ErrorStruct* error_ptr) {

	// spec: in 0:00:00:00 format ie. Hrs:Mins:Secs:hundredths. Note that there is a single digit
	// for the hours!

	AssTime time = {};

	if(value.length != 10) {
		*error_ptr = STATIC_ERROR("error, not a valid time, not correct length");
		return time;
	}

	Utf8StrView value_view = get_str_view_from_const_str_view(value);

	{
		// hour

		ConstUtf8StrView hour_str = {};
		if(!str_view_get_substring_by_amount(&value_view, &hour_str, 1)) {
			*error_ptr = STATIC_ERROR("error, couldn't get hour value");
			return time;
		}

		size_t num = parse_str_as_unsigned_number(hour_str, error_ptr);

		if(error_ptr->message != NULL) {
			return time;
		}

		time.hour = (uint8_t)num;

		if(!str_view_expect_ascii(&value_view, ":")) {
			*error_ptr = STATIC_ERROR("error, not a valid time, missing ':'");
			return time;
		}
	}

	{
		// min

		ConstUtf8StrView min_str = {};
		if(!str_view_get_substring_by_amount(&value_view, &min_str, 2)) {
			*error_ptr = STATIC_ERROR("error, couldn't get min value");
			return time;
		}

		size_t num = parse_str_as_unsigned_number(min_str, error_ptr);

		if(error_ptr->message != NULL) {
			return time;
		}

		time.min = (uint8_t)num;

		if(!str_view_expect_ascii(&value_view, ":")) {
			*error_ptr = STATIC_ERROR("error, not a valid time, missing ':'");
			return time;
		}
	}

	{
		// sec

		ConstUtf8StrView sec_str = {};
		if(!str_view_get_substring_by_amount(&value_view, &sec_str, 2)) {
			*error_ptr = STATIC_ERROR("error, couldn't get sec value");
			return time;
		}

		size_t num = parse_str_as_unsigned_number(sec_str, error_ptr);

		if(error_ptr->message != NULL) {
			return time;
		}

		time.sec = (uint8_t)num;

		if(!str_view_expect_ascii(&value_view, ":")) {
			// note: specs defines ":" but in the wild we mostly get "."
			if(!str_view_expect_ascii(&value_view, ".")) {
				*error_ptr = STATIC_ERROR("error, not a valid time, missing ':' or '.' after secs");
				return time;
			}
		}
	}

	{
		// hundred

		ConstUtf8StrView hundred_str = {};
		if(!str_view_get_substring_by_amount(&value_view, &hundred_str, 2)) {
			*error_ptr = STATIC_ERROR("error, couldn't get hundred value");
			return time;
		}

		size_t num = parse_str_as_unsigned_number(hundred_str, error_ptr);

		if(error_ptr->message != NULL) {
			return time;
		}

		time.hundred = (uint8_t)num;

		if(!str_view_is_eof(value_view)) {
			*error_ptr = STATIC_ERROR("error, not a valid time, more data then expected");
			return time;
		}
	}

	*error_ptr = NO_ERROR();
	return time;
}

[[nodiscard]] static ErrorStruct parse_event_line_for_events(EventType type, Utf8StrView* line_view,
                                                             const STBDS_ARRAY(AssEventFormat)
                                                                 const format_spec,
                                                             AssEvents* events_result) {

	size_t field_size = stbds_arrlenu(format_spec);

	AssEventEntry entry = { .type = type };

	if(!str_view_skip_optional_whitespace(line_view)) {
		return STATIC_ERROR("skip whitespace error");
	}

	size_t i = 0;
	for(; !str_view_is_eof(*line_view); ++i) {

		if(i >= field_size) {

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer,
			                      "error, too many fields in the event line, the format line "
			                      "specified %lu, but we are already at %lu",
			                      field_size, (i + 1));

			return DYNAMIC_ERROR(result_buffer);
		}

		AssEventFormat format = format_spec[i];

		// special handling fot the text field, as it may contain ","

		ConstUtf8StrView value = {};

		if(format == AssEventFormatText) {
			if(i != field_size - 1) {
				return STATIC_ERROR(
				    "'Text' field of event lines may only occur at the last position!");
			}

			if(!str_view_get_substring_until_eof(line_view, &value)) {
				return STATIC_ERROR("eof before comma in events section event line");
			}

		} else {
			if(!str_view_get_substring_by_delimiter(line_view, &value, char_delimiter, false,
			                                        ",")) {
				if(!str_view_get_substring_until_eof(line_view, &value)) {
					return STATIC_ERROR("eof before comma in events section event line");
				}
			}
		}

		ErrorStruct error = NO_ERROR();

		switch(format) {
			case AssEventFormatLayer: {
				entry.layer = parse_str_as_unsigned_number(value, &error);
				break;
			}
			case AssEventFormatStart: {
				entry.start = parse_str_as_time(value, &error);
				break;
			}
			case AssEventFormatEnd: {
				entry.end = parse_str_as_time(value, &error);
				break;
			}
			case AssEventFormatStyle: {
				entry.style = value;
				break;
			}
			case AssEventFormatName: {
				entry.name = value;
				break;
			}
			case AssEventFormatMarginL: {
				entry.margin_l = parse_str_as_margin_value(value, &error);
				break;
			}
			case AssEventFormatMarginR: {
				entry.margin_r = parse_str_as_margin_value(value, &error);
				break;
			}
			case AssEventFormatMarginV: {
				entry.margin_v = parse_str_as_margin_value(value, &error);
				break;
			}
			case AssEventFormatEffect: {
				entry.effect = value;
				break;
			}
			case AssEventFormatText: {
				entry.text = value;
				break;
			}
			default: {
				error = STATIC_ERROR("unreachable");
			}
		}

		if(error.message != NULL) {

			char* value_name = get_normalized_string(value);

			if(!value_name) {
				return STATIC_ERROR("allocation error");
			}

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer, "While parsing field '%s' with value '%s': %s",
			                      get_name_for_event_format(format), value_name, error.message);

			free_error_struct(error);
			free(value_name);
			return DYNAMIC_ERROR(result_buffer);
		}
	}

	if(i != field_size) {
		return STATIC_ERROR(
		    "error, too few fields in the event line, the format line specified more");
	}

	stbds_arrput(events_result->entries, entry);

	return NO_ERROR();
}

[[nodiscard]] static ErrorStruct parse_events(AssEvents* ass_events, Utf8StrView* data_view,
                                              ParseSettings settings) {

	AssEvents events = { .entries = STBDS_ARRAY_EMPTY };

	STBDS_ARRAY(AssEventFormat) event_format = STBDS_ARRAY_EMPTY;

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstUtf8StrView line = {};
		if(!str_view_get_substring_by_delimiter(data_view, &line, newline_delimiter, true, NULL)) {
			return STATIC_ERROR("eof before newline in parse events");
		}

		// parse line
		{

			Utf8StrView line_view = get_str_view_from_const_str_view(line);

			ConstUtf8StrView field = {};
			if(!str_view_get_substring_by_delimiter(&line_view, &field, char_delimiter, false,
			                                        ":")) {
				return STATIC_ERROR("end of line before ':' in line parsing in events section");
			}

			if(str_view_eq_ascii(field, "Format")) {

				if(stbds_arrlenu(event_format) != 0) {
					return STATIC_ERROR(
					    "multiple format fields detected in the events section, this is not "
					    "allowed");
				}

				ErrorStruct format_line_error =
				    parse_format_line_for_events(&line_view, &event_format);

				if(format_line_error.message != NULL) {
					return format_line_error;
				}

			} else if(str_view_eq_ascii(field, "Dialogue")) {

				if(stbds_arrlenu(event_format) == 0) {
					return STATIC_ERROR(
					    "no format line occurred before the style line in the events section, "
					    "this is an error");
				}

				ErrorStruct event_parse_error = parse_event_line_for_events(
				    EventTypeDialogue, &line_view, event_format, &events);

				if(event_parse_error.message != NULL) {
					return event_parse_error;
				}

			} else if(str_view_eq_ascii(field, "Comment")) {

				if(stbds_arrlenu(event_format) == 0) {
					return STATIC_ERROR(
					    "no format line occurred before the style line in the events section, "
					    "this is an error");
				}

				ErrorStruct event_parse_error = parse_event_line_for_events(
				    EventTypeComment, &line_view, event_format, &events);

				if(event_parse_error.message != NULL) {
					return event_parse_error;
				}

			} else if(str_view_eq_ascii(field, "Picture")) {

				if(stbds_arrlenu(event_format) == 0) {
					return STATIC_ERROR(
					    "no format line occurred before the style line in the events section, "
					    "this is an error");
				}

				ErrorStruct event_parse_error = parse_event_line_for_events(
				    EventTypePicture, &line_view, event_format, &events);

				if(event_parse_error.message != NULL) {
					return event_parse_error;
				}

			} else if(str_view_eq_ascii(field, "Sound")) {

				if(stbds_arrlenu(event_format) == 0) {
					return STATIC_ERROR(
					    "no format line occurred before the style line in the events section, "
					    "this is an error");
				}

				ErrorStruct event_parse_error =
				    parse_event_line_for_events(EventTypeSound, &line_view, event_format, &events);

				if(event_parse_error.message != NULL) {
					return event_parse_error;
				}

			} else if(str_view_eq_ascii(field, "Movie")) {

				if(stbds_arrlenu(event_format) == 0) {
					return STATIC_ERROR(
					    "no format line occurred before the style line in the events section, "
					    "this is an error");
				}

				ErrorStruct event_parse_error =
				    parse_event_line_for_events(EventTypeMovie, &line_view, event_format, &events);

				if(event_parse_error.message != NULL) {
					return event_parse_error;
				}

			} else if(str_view_eq_ascii(field, "Command")) {

				if(stbds_arrlenu(event_format) == 0) {
					return STATIC_ERROR(
					    "no format line occurred before the style line in the events section, "
					    "this is an error");
				}

				ErrorStruct event_parse_error = parse_event_line_for_events(
				    EventTypeCommand, &line_view, event_format, &events);

				if(event_parse_error.message != NULL) {
					return event_parse_error;
				}

			} else {

				char* field_name = get_normalized_string(field);

				if(!field_name) {
					return STATIC_ERROR("allocation error");
				}

				char* result_buffer = NULL;
				FORMAT_STRING_DEFAULT(&result_buffer, "unexpected field in events section: '%s'",
				                      field_name);

				if(settings.strict_settings.allow_additional_fields) {
					LOG_MESSAGE(LogLevelWarn, "%s\n", result_buffer);

					free(result_buffer);
					free(field_name);
					continue;
				}

				free(field_name);
				return DYNAMIC_ERROR(result_buffer);
			}
		}

		// end of line parse

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

	stbds_arrfree(event_format);
	*ass_events = events;
	return NO_ERROR();
	// end of script info
}

[[nodiscard]] static ErrorStruct get_section_by_name(ConstUtf8StrView section_name,
                                                     AssResult* ass_result, Utf8StrView* data_view,
                                                     ParseSettings settings) {

	if(str_view_eq_ascii(section_name, "V4+ Styles")) {
		return parse_styles(&(ass_result->styles), data_view, settings);
	}

	if(str_view_eq_ascii(section_name, "V4 Styles")) {
		return STATIC_ERROR("v4 styles are not supported");
	}

	if(str_view_eq_ascii(section_name, "Events")) {
		return parse_events(&(ass_result->events), data_view, settings);
	}

	if(str_view_eq_ascii(section_name, "Fonts")) {
		return skip_section(data_view);
	}

	if(str_view_eq_ascii(section_name, "Graphics")) {
		return skip_section(data_view);
	}

	return extra_section(section_name, data_view, &(ass_result->extra_sections));

	return NO_ERROR();
}

[[nodiscard]] AssParseResult* parse_ass(AssSource source, ParseSettings settings) {

	AssParseResult* result = (AssParseResult*)malloc(sizeof(AssParseResult));

	if(!result) {
		return NULL;
	}

	SizedPtr data = get_data_from_source(source);

	if(is_ptr_error(data)) {
		RETURN_ERROR(STATIC_ERROR(ptr_get_error(data)));
	}

	FileType file_type = determine_file_type(data);

	size_t bom_size = 0;

	switch(file_type) {
		case FileTypeUnknown: {
			const char* error = "unrecognized file, no BOM present";

			if(!settings.strict_settings.allow_non_utf8) {
				RETURN_ERROR(STATIC_ERROR(error));
			}

			LOG_MESSAGE(LogLevelWarn, "%s, assuming UTF-8 (ascii also works)\n", error);
			bom_size = 0;
			break;
		}
		case FileTypeUtf8: {
			bom_size = 1;
			break;
		}
		default: {
			free_sized_ptr(data);
			RETURN_ERROR(STATIC_ERROR("only UTF-8 encoded files supported atm"));
		}
	}

	Utf8DataResult utf8_result = get_utf8_string(data.data, data.len);

	free_sized_ptr(data);

	if(utf8_result.has_error) {
		RETURN_ERROR(STATIC_ERROR(utf8_result.data.error));
	}

	Utf8Data final_data = utf8_result.data.result;

	Utf8StrView data_view = str_view_from_data(final_data);

	if(bom_size > 0) {
		// NOTE: the bom byte is always just one codepoint
		if(!str_view_advance(&data_view, bom_size)) {
			RETURN_ERROR(STATIC_ERROR("couldn't skip bom bytes"));
		}
	}

	// parse script info

	if(!str_view_expect_ascii(&data_view, "[Script Info]")) {
		RETURN_ERROR(STATIC_ERROR("first line must be the script info section"));
	}

	if(!str_view_expect_newline(&data_view)) {
		RETURN_ERROR(STATIC_ERROR("expected newline"));
	}

	AssResult ass_result = { .allocated_data = final_data,
		                     .extra_sections = (ExtraSections){ .entries = STBDS_HASH_MAP_EMPTY } };

	ErrorStruct script_info_parse_result =
	    parse_script_info(&(ass_result.script_info), &data_view, settings);

	if(script_info_parse_result.message != NULL) {
		RETURN_ERROR(script_info_parse_result);
	}

	while(true) {

		// classify section, and then parsing that section

		if(!str_view_expect_ascii(&data_view, "[")) {
			RETURN_ERROR(STATIC_ERROR("implementation error"));
		}

		ConstUtf8StrView section_name = {};
		if(!str_view_get_substring_by_delimiter(&data_view, &section_name, char_delimiter, false,
		                                        "]")) {
			RETURN_ERROR(STATIC_ERROR("script section not terminated by ']'"));
		}

		if(!str_view_expect_newline(&data_view)) {
			RETURN_ERROR(STATIC_ERROR("no newline after section name"));
		}

		ErrorStruct section_parse_result =
		    get_section_by_name(section_name, &ass_result, &data_view, settings);

		if(section_parse_result.message != NULL) {
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

[[nodiscard]] char* parse_result_get_error(AssParseResult* result) {
	if(!result) {
		return "allocation error";
	}

	return result->data.error.message;
}

static void free_extra_section_entry(ExtraSectionEntry entry) {
	size_t hm_length = stbds_shlenu(entry.fields);

	for(size_t i = 0; i < hm_length; ++i) {
		SectionFieldEntry hm_entry = entry.fields[i];

		free(hm_entry.key);
	}

	stbds_shfree(entry.fields);
}

static void free_extra_sections(ExtraSections sections) {

	size_t hm_length = stbds_shlenu(sections.entries);

	for(size_t i = 0; i < hm_length; ++i) {
		ExtraSectionHashMapEntry entry = sections.entries[i];

		free_extra_section_entry(entry.value);
		free(entry.key);
	}

	stbds_shfree(sections.entries);
}

static void free_ass_result(AssResult data) {
	free_utf8_data(data.allocated_data);
	stbds_arrfree(data.styles.entries);
	stbds_arrfree(data.events.entries);

	free_extra_sections(data.extra_sections);
}

void free_parse_result(AssParseResult* result) {
	if(!result->is_error) {
		free_ass_result(result->data.ok);
	} else {
		free_error_struct(result->data.error);
	}

	free(result);
}
