
#define ASS_PARSER_C_INTERNAL_USAGE

#include "./parser.h"
#ifndef __WASM__
#include "../helper/io.h"
#endif

#include "../helper/macros.h"
#include "../helper/utf_helper.h"
#include "./fonts.h"
#include "./helper.h"
#include "./validate.h"

#undef ASS_PARSER_C_INTERNAL_USAGE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <zmap/zmap.h>
#include <zvec/zvec.h>

[[nodiscard]] const char* get_script_type_name(ScriptType script_type) {
	switch(script_type) {
		case ScriptTypeUnknown: return "Unknown";
		case ScriptTypeV4: return "V4";
		case ScriptTypeV4Plus: return "V4+";
		default: return "<unknown>";
	}
}

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
	// rfc needs us to ignore unknown values, we do that here
	AssStyleFormatUnknownField
} AssStyleFormat;

[[nodiscard]] static const char* get_name_for_style_format(AssStyleFormat format) {
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
		case AssStyleFormatUnknownField: return "UnknownField";
		default: return "<unknown>";
	}
}

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
	// rfc needs us to ignore unknown values, we do that here
	AssEventFormatUnknownField
} AssEventFormat;

[[nodiscard]] static const char* get_name_for_event_format(AssEventFormat format) {
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
		case AssEventFormatUnknownField: return "UnknownField";
		default: return "<unknown>";
	}
}

struct AssParseResultImpl {
	bool is_error;
	union {
		AssResult ok;
	} data;
	Diagnostics diagnostics;
	Codepoints allocated_codepoints;
};

[[nodiscard]] static SizedPtr get_data_from_source(AssSource source) {
	switch(source.type) {
		case AssSourceTypeFile: {
#ifndef __WASM__
			return read_entire_file(source.data.file);
#else
			return ptr_error("file not suported in wasm builds");
#endif
		}
		case AssSourceTypeStr: {
			return source.data.str;
		}
		default: return ptr_error("unknown source type");
	}
}

[[nodiscard]] const char* get_file_type_name(FileType file_type) {
	switch(file_type) {
		case FileTypeUnknown: return "Unknown";
		case FileTypeUtf8: return "UTF-8";
		case FileTypeUtf16BE: return "UTF-16 BE";
		case FileTypeUtf16LE: return "UTF-16 LE";
		case FileTypeUtf32BE: return "UTF-32 BE";
		case FileTypeUtf32LE: return "UTF-32 LE";
		default: return "<unknown>";
	}
}

// see: https://en.wikipedia.org/wiki/Byte_order_mark
[[nodiscard]] static FileType determine_file_type(SizedPtr data) {

	uint8_t* bom = (uint8_t*)data.data;

	if(data.len < 4) {
		return FileTypeUnknown;
	}

	if(bom[0] == 0xEF && // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	   bom[1] == 0xBB && // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	   bom[2] == 0xBF) { // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
		return FileTypeUtf8;
	}
	if(bom[0] == 0xFF && // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	   bom[1] == 0xFE && // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	   bom[2] == 0x00 && // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	   bom[3] == 0x00) { // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
		return FileTypeUtf32LE;
	}
	if(bom[0] == 0x00 && bom[1] == 0x00 &&
	   bom[2] == 0xFE && // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	   bom[3] == 0xFF) { // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
		return FileTypeUtf32BE;
	}
	if(bom[0] == 0xFF && // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	   bom[1] == 0xFE) { // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
		return FileTypeUtf16LE;
	}
	if(bom[0] == 0xFE && // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	   bom[1] == 0xFF) { // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
		return FileTypeUtf16BE;
	}

	return FileTypeUnknown;
}

[[nodiscard]] static bool str_view_starts_with_ascii_or_eof(StrView str_view,
                                                            const char* ascii_str) {

	if(str_view_starts_with_ascii(str_view, ascii_str)) {
		return true;
	}

	return str_view_is_eof(str_view);
}

#if defined(__clang__)
#define ENUM_EXTENSIBILITY_CLOSED __attribute__((enum_extensibility(closed)))
#else
#define ENUM_EXTENSIBILITY_CLOSED
#endif

typedef enum ENUM_EXTENSIBILITY_CLOSED : bool {
	ErrorTypeFatal = false,
	ErrorTypeNone = true,
} ErrorType;

ZVEC_DEFINE_AND_IMPLEMENT_VEC_TYPE(AssStyleFormat)

[[nodiscard]] static ErrorType
parse_format_line_for_styles(const ConstStrView line, ZVEC_TYPENAME(AssStyleFormat) * format_result,
                             Diagnostics* diagnostics) {

	StrView line_view = get_str_view_from_const_str_view(line);

	while(!(str_view_is_eof(line_view))) {

		if(!str_view_skip_optional_whitespace(&line_view)) {
			INSERT_SIMPLE_ERROR(diagnostics->entries,
			                    STATIC_MESSAGE_STRUCT("skip whitespace error"),
			                    line_view.position.file_pos);
			return ErrorTypeFatal;
		}

		ConstStrView key = {};
		if(!str_view_get_substring_by_char_delimiter(&line_view, &key, ',', true)) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("implementation error"),
			                    line_view.position.file_pos);
			return ErrorTypeFatal;
		}

		if(key.length == 0) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("implementation error"),
			                    line_view.position.file_pos);
			return ErrorTypeFatal;
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
				INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
				                    line_view.position.file_pos);
				return ErrorTypeFatal;
			}

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		free(key_name); \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), \
		                    line_view.position.file_pos); \
		return ErrorTypeFatal; \
	} while(false)
			char* result_buffer = NULL;
			FORMAT_STRING_PROPAGATE_ERROR(
			    &result_buffer, "unrecognized format key '%s' in format line in styles section",
			    key_name);

			free(key_name);

			INSERT_SIMPLE_WARNING(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
			                      line_view.position.file_pos);

			format = AssStyleFormatUnknownField;
		}

		ZVEC_PUSH_AND_ASSERT(AssStyleFormat, format_result, format);
	}

	return ErrorTypeNone;
}

#undef PROPAGATE_ERROR_IMPL

/*** general note:
    @see keep-going
    means that, even if we encountered an error, at the end this is all invalid, so
    the invalid data in the field is ok, but we can keep going parsing and get more
    errors */

[[nodiscard]] static ErrorType parse_style_line_for_styles(
    const ConstStrView line, const ZVEC_TYPENAME(AssStyleFormat) format_spec,
    AssStyles* styles_result, ParseSettings settings, Diagnostics* diagnostics) {

	StrView line_view = get_str_view_from_const_str_view(line);

	size_t field_size = ZVEC_LENGTH(format_spec);

	AssStyleEntry entry = {};

	if(!str_view_skip_optional_whitespace(&line_view)) {
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("skip whitespace error"),
		                    line_view.position.file_pos);
		return ErrorTypeFatal;
	}

	size_t i = 0; // NOLINT(readability-identifier-length)
	for(; !str_view_is_eof(line_view); ++i) {

		ConstStrView value = {};
		if(!str_view_get_substring_by_char_delimiter(&line_view, &value, ',', true)) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("implementation error"),
			                    line_view.position.file_pos);
			return ErrorTypeFatal;
		}

		if(i >= field_size) {

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), \
		                    line_view.position.file_pos); \
		return ErrorTypeFatal; \
	} while(false)

			char* result_buffer = NULL;
			FORMAT_STRING_PROPAGATE_ERROR(
			    &result_buffer,
			    "error, too many fields in the style line, the format line "
			    "specified %lu, but we are already at %lu",
			    field_size, (i + 1));

			INSERT_SIMPLE_ERROR(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
			                    line_view.position.file_pos);
			return ErrorTypeFatal;
		}

#undef PROPAGATE_ERROR_IMPL

		MessageStruct error = EMPTY_MESSAGE_STRUCT();

		AssStyleFormat format = ZVEC_AT(AssStyleFormat, format_spec, i);

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
				    value, &error, settings.strict_settings.allow_number_truncating, diagnostics);
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
				entry.scale_x = parse_str_as_unsigned_number(value, &error, diagnostics);
				break;
			}
			case AssStyleFormatScaleY: {
				entry.scale_y = parse_str_as_unsigned_number(value, &error, diagnostics);
				break;
			}
			case AssStyleFormatSpacing: {
				entry.spacing = parse_str_as_double(value, &error, diagnostics);
				break;
			}
			case AssStyleFormatAngle: {
				entry.angle = parse_str_as_double(value, &error, diagnostics);
				break;
			}
			case AssStyleFormatBorderStyle: {
				entry.border_style = parse_str_as_border_style(value, &error, diagnostics);
				break;
			}
			case AssStyleFormatOutline: {
				entry.outline = parse_str_as_double(value, &error, diagnostics);
				break;
			}
			case AssStyleFormatShadow: {
				entry.shadow = parse_str_as_double(value, &error, diagnostics);
				break;
			}
			case AssStyleFormatAlignment: {
				entry.alignment = parse_str_as_style_alignment(value, &error, diagnostics);
				break;
			}
			case AssStyleFormatMarginL: {
				entry.margin_l = parse_str_as_unsigned_number(value, &error, diagnostics);
				break;
			}
			case AssStyleFormatMarginR: {
				entry.margin_r = parse_str_as_unsigned_number(value, &error, diagnostics);
				break;
			}
			case AssStyleFormatMarginV: {
				entry.margin_v = parse_str_as_unsigned_number(value, &error, diagnostics);
				break;
			}
			case AssStyleFormatEncoding: {
				entry.encoding = parse_str_as_unsigned_number(value, &error, diagnostics);
				break;
			}
			case AssStyleFormatUnknownField: {
				INSERT_SIMPLE_WARNING(diagnostics->entries,
				                      STATIC_MESSAGE_STRUCT("ignoring unknown style field entry"),
				                      value.file_pos);
				goto skip_entry_style;
			}
			default: {
				UNREACHABLE();
			}
		}

		if(!is_empty_message_struct(error)) {

			char* value_name = get_normalized_string(value);

			if(!value_name) {
				INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
				                    line_view.position.file_pos);
				return ErrorTypeFatal;
			}

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		free(value_name); \
		free_message_struct(error); \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), \
		                    line_view.position.file_pos); \
		return ErrorTypeFatal; \
	} while(false)

			char* result_buffer = NULL;
			FORMAT_STRING_PROPAGATE_ERROR(
			    &result_buffer, "While parsing field '%s' with value '%s': %s",
			    get_name_for_style_format(format), value_name, get_message(error));

#undef PROPAGATE_ERROR_IMPL

			free(value_name);
			free_message_struct(error);

			INSERT_SIMPLE_ERROR(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
			                    line_view.position.file_pos);

			/*** @see keep-going */
			continue;
		}

	skip_entry_style:
	}

	if(i != field_size) {

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), \
		                    line_view.position.file_pos); \
		return ErrorTypeFatal; \
	} while(false)

		char* result_buffer = NULL;
		FORMAT_STRING_PROPAGATE_ERROR(&result_buffer,
		                              "error, too few fields in the style line, the format line "
		                              "specified %lu, but we only have %lu",
		                              field_size, i);

		INSERT_SIMPLE_ERROR(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
		                    line_view.position.file_pos);
		return ErrorTypeFatal;
	}

	ZVEC_PUSH_SLOT_AND_ASSERT(AssStyleEntry, &(styles_result->entries), entry);

	return ErrorTypeNone;
}

#undef PROPAGATE_ERROR_IMPL

[[nodiscard]] static ErrorType parse_styles(AssStyles* ass_styles, StrView* data_view,
                                            ParseSettings settings, LineType line_type,
                                            Diagnostics* diagnostics) {

	AssStyles styles = { .entries = ZVEC_EMPTY(AssStyleEntry) };

	ZVEC_TYPENAME(AssStyleFormat) style_format = ZVEC_EMPTY(AssStyleFormat);

#define FREE_AT_END() \
	do { \
		ZVEC_FREE(AssStyleEntry, &styles.entries); \
		ZVEC_FREE(AssStyleFormat, &style_format); \
	} while(false)

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstStrView line = {};
		if(!str_view_get_substring_until_eol(data_view, &line, line_type, true)) {
			FREE_AT_END();

			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("implementation error"),
			                    data_view->position.file_pos);
			return ErrorTypeFatal;
		}

		// parse line
		{

			if(line.length == 0) {
				continue;
			}

			StrView line_view = get_str_view_from_const_str_view(line);

			ConstStrView field = {};
			if(!str_view_get_substring_by_char_delimiter(&line_view, &field, ':', false)) {
				FREE_AT_END();

				INSERT_SIMPLE_ERROR(diagnostics->entries,
				                    STATIC_MESSAGE_STRUCT(
				                        "end of line before ':' in line parsing in styles section"),
				                    data_view->position.file_pos);
				return ErrorTypeFatal;
			}

			if(str_view_eq_ascii(field, "Format")) {

				if(ZVEC_LENGTH(style_format) != 0) {
					INSERT_SIMPLE_ERROR(
					    diagnostics->entries,
					    STATIC_MESSAGE_STRUCT(
					        "multiple format fields detected in the styles section, this is not "
					        "allowed"),
					    data_view->position.file_pos);

					/*** @see keep-going */
					continue;
				}

				ErrorType format_line_error = parse_format_line_for_styles(
				    get_const_str_view_from_str_view(line_view), &style_format, diagnostics);

				if(format_line_error != ErrorTypeNone) {
					/*** @see keep-going */
					continue;
				}

			} else if(str_view_eq_ascii(field, "Style")) {

				if(ZVEC_LENGTH(style_format) == 0) {
					FREE_AT_END();

					INSERT_SIMPLE_ERROR(
					    diagnostics->entries,
					    STATIC_MESSAGE_STRUCT(
					        "no format line occurred before the style line in the styles section, "
					        "this is an error"),
					    line_view.position.file_pos);
					return ErrorTypeFatal;
				}

				ErrorType style_parse_error =
				    parse_style_line_for_styles(get_const_str_view_from_str_view(line_view),
				                                style_format, &styles, settings, diagnostics);

				if(style_parse_error != ErrorTypeNone) {
					/*** @see keep-going */
					continue;
				}

			} else {
				const DiagnosticSeverity severity_type =
				    settings.strict_settings.allow_additional_fields ? DiagnosticSeverityWarning
				                                                     : DiagnosticSeverityError;

				UnexpectedFieldDiagnostic unexpected_field = { .field = field,
					                                           .section = "styles" };

				DiagnosticEntry diagnostic = {
					.inner = (InnerDiagnostic){ .type = DiagnosticTypeUnexpectedField,
					                            .data = { .unexpected_field = unexpected_field } },
					.severity = severity_type,
					.position = field.file_pos
				};

				ZVEC_PUSH_SLOT_AND_ASSERT(DiagnosticEntry, &(diagnostics->entries), diagnostic);

				/*** @see keep-going */
				continue;
			}
		}

		// end of line parse

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

	ZVEC_FREE(AssStyleFormat, &style_format);

	*ass_styles = styles;

	return ErrorTypeNone;
	// end of script info
}

#undef FREE_AT_END

// global default values, so that they are valid all the time

static FinalStr
    g_default_ass_title = { // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
	    (int32_t[]){ '<', 'u', 'n', 't', 'i', 't', 'l', 'e', 'd', '>' },
	    10, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	    .file_pos = { .line = EMPTY_POS_VAL, .column = EMPTY_POS_VAL }
    };

static FinalStr
    g_default_ass_script_name = { // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
	    (int32_t[]){ '<', 'u', 'n', 'k', 'n', 'o', 'w', 'n', '>' },
	    9, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	    .file_pos = { .line = EMPTY_POS_VAL, .column = EMPTY_POS_VAL }
    };

ZVEC_DEFINE_AND_IMPLEMENT_VEC_TYPE(FinalStr)

#define FREE_AT_END() \
	do { \
		ZVEC_FREE(FinalStr, &field_names); \
	} while(false)

[[nodiscard]] static ErrorType parse_script_info(AssScriptInfo* script_info_result,
                                                 StrView* data_view, ParseSettings settings,
                                                 LineType line_type, Diagnostics* diagnostics) {

	AssScriptInfo script_info = { .script_type = ScriptTypeUnknown,
		                          .title = { .start = NULL, .length = 0 },
		                          .original_script = { .start = NULL, .length = 0 } };

	// note: we already advanced to the next line, so this needs to be one line before
	FilePos script_info_section_pos = { .line = data_view->position.file_pos.line - 1,
		                                data_view->position.file_pos.column };

	ZVEC_TYPENAME(FinalStr) field_names = ZVEC_EMPTY(FinalStr);

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstStrView line = {};
		if(!str_view_get_substring_until_eol(data_view, &line, line_type, true)) {
			FREE_AT_END();
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("implementation error"),
			                    data_view->position.file_pos);

			return ErrorTypeFatal;
		}

		// parse line
		{

			if(line.length == 0) {
				continue;
			}

			StrView line_view = get_str_view_from_const_str_view(line);

			if(str_view_starts_with_ascii(line_view, ";")) {
				continue;
			}

			ConstStrView field = {};
			if(!str_view_get_substring_by_char_delimiter(&line_view, &field, ':', false)) {

				INSERT_SIMPLE_ERROR(
				    diagnostics->entries,
				    STATIC_MESSAGE_STRUCT(
				        "end of line before ':' in line parsing in script info section"),
				    data_view->position.file_pos);

				/*** @see keep-going */
				continue;
			}

			// check for duplicate fields
			bool found_field = false;
			for(size_t i = 0; i < ZVEC_LENGTH(field_names); ++i) {
				FinalStr field_str = ZVEC_AT(FinalStr, field_names, i);

				if(str_view_eq_str_view(field_str, field)) {
					found_field = true;

					const DiagnosticSeverity severity_type =
					    settings.strict_settings.script_info.allow_duplicate_fields
					        ? DiagnosticSeverityWarning
					        : DiagnosticSeverityError;

					DuplicateFieldDiagnostic duplicate_field = { .field = field,
						                                         .section = "script info" };

					DiagnosticEntry diagnostic = {
						.inner =
						    (InnerDiagnostic){ .type = DiagnosticTypeDuplicateField,
						                       .data = { .duplicate_field = duplicate_field } },
						.severity = severity_type,
						.position = field.file_pos
					};

					ZVEC_PUSH_SLOT_AND_ASSERT(DiagnosticEntry, &(diagnostics->entries), diagnostic);

					/*** @see keep-going */
					break;
				}
			}

			if(!found_field) {
				// note: as this is not intended, we can choose if the last or the first value is
				// the final value
				ZVEC_PUSH_SLOT_AND_ASSERT(FinalStr, &field_names, field);
			}

			if(!str_view_skip_optional_whitespace(&line_view)) {
				FREE_AT_END();

				INSERT_SIMPLE_ERROR(diagnostics->entries,
				                    STATIC_MESSAGE_STRUCT("skip whitespace error"),
				                    line_view.position.file_pos);
				return ErrorTypeFatal;
			}

			ConstStrView value = get_const_str_view_from_str_view(line_view);

			MessageStruct error = EMPTY_MESSAGE_STRUCT();

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
			} else if(str_view_eq_ascii(field, "Script Updated By")) {
				script_info.script_updated_by = value;
			} else if(str_view_eq_ascii(field, "Update Details")) {
				script_info.update_details = value;
				// TODO: this is only allowed in non strict mode
			} else if(str_view_eq_ascii(field, "ScriptType") ||
			          str_view_eq_ascii(field, "Script Type")) {
				script_info.script_type = parse_str_as_script_type(value, &error);
			} else if(str_view_eq_ascii(field, "Collisions")) {
				script_info.collisions = value;
			} else if(str_view_eq_ascii(field, "PlayResY")) {
				script_info.play_res_y = parse_str_as_unsigned_number(value, &error, diagnostics);
				;
			} else if(str_view_eq_ascii(field, "PlayResX")) {
				script_info.play_res_x = parse_str_as_unsigned_number(value, &error, diagnostics);
				;
			} else if(str_view_eq_ascii(field, "PlayDepth")) {
				script_info.play_depth = value;
			} else if(str_view_eq_ascii(field, "Timer")) {
				script_info.timer = value;
			} else if(str_view_eq_ascii(field, "WrapStyle")) {
				script_info.wrap_style = parse_str_as_wrap_style(value, &error, diagnostics);
			} else if(str_view_eq_ascii(field, "ScaledBorderAndShadow")) {
				script_info.scaled_border_and_shadow = parse_str_as_str_bool(value, &error);
			} else if(str_view_eq_ascii(field, "Video Aspect Ratio")) {
				script_info.video_aspect_ratio =
				    parse_str_as_unsigned_number(value, &error, diagnostics);
			} else if(str_view_eq_ascii(field, "Video Zoom")) {
				script_info.video_zoom = parse_str_as_unsigned_number(value, &error, diagnostics);
			} else if(str_view_eq_ascii(field, "YCbCr Matrix")) {
				script_info.ycbcr_matrix = value;
			} else {

				const DiagnosticSeverity severity_type =
				    settings.strict_settings.allow_additional_fields ? DiagnosticSeverityWarning
				                                                     : DiagnosticSeverityError;

				UnexpectedFieldDiagnostic unexpected_field = { .field = field,
					                                           .section = "script info" };

				DiagnosticEntry diagnostic = {
					.inner = (InnerDiagnostic){ .type = DiagnosticTypeUnexpectedField,
					                            .data = { .unexpected_field = unexpected_field } },
					.severity = severity_type,
					.position = field.file_pos
				};

				ZVEC_PUSH_SLOT_AND_ASSERT(DiagnosticEntry, &(diagnostics->entries), diagnostic);

				/*** @see keep-going */
				continue;
			}

			if(!is_empty_message_struct(error)) {

				char* field_name = get_normalized_string(field);

				if(!field_name) {
					FREE_AT_END();
					INSERT_SIMPLE_ERROR(diagnostics->entries,
					                    STATIC_MESSAGE_STRUCT("allocation error"),
					                    line_view.position.file_pos);
					return ErrorTypeFatal;
				}

				char* value_name = get_normalized_string(value);

				if(!value_name) {
					FREE_AT_END();
					INSERT_SIMPLE_ERROR(diagnostics->entries,
					                    STATIC_MESSAGE_STRUCT("allocation error"),
					                    line_view.position.file_pos);
					return ErrorTypeFatal;
				}

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		FREE_AT_END(); \
		free_message_struct(error); \
		free(field_name); \
		free(value_name); \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), \
		                    line_view.position.file_pos); \
		return ErrorTypeFatal; \
	} while(false)

				char* result_buffer = NULL;
				FORMAT_STRING_PROPAGATE_ERROR(&result_buffer,
				                              "While parsing field '%s' with value '%s': %s",
				                              field_name, value_name, get_message(error));

#undef PROPAGATE_ERROR_IMPL

				free_message_struct(error);
				free(field_name);
				free(value_name);

				INSERT_SIMPLE_ERROR(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
				                    line_view.position.file_pos);

				/*** @see keep-going */
				continue;
			}
		}

		// end of line parse

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

	ZVEC_FREE(FinalStr, &field_names);
#undef FREE_AT_END

	// check script info
	{

		if(script_info.script_type == ScriptTypeUnknown) {
			const char* error = "missing script type in script info section";

			const DiagnosticSeverity severity_type =
			    settings.strict_settings.script_info.allow_missing_script_type
			        ? DiagnosticSeverityWarning
			        : DiagnosticSeverityError;

			INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries, STATIC_MESSAGE_STRUCT(error),
			                         script_info_section_pos, severity_type);

		} else if(script_info.script_type != ScriptTypeV4Plus) {

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), \
		                    script_info_section_pos); \
		return ErrorTypeFatal; \
	} while(false)

			char* result_buffer = NULL;
			FORMAT_STRING_PROPAGATE_ERROR(&result_buffer,
			                              "only scrypt type v4+ is supported but got: %s",
			                              get_script_type_name(script_info.script_type));

#undef PROPAGATE_ERROR_IMPL

			INSERT_SIMPLE_ERROR(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
			                    script_info_section_pos);
		}

		if(script_info.title.start == NULL) {

			script_info.title = g_default_ass_title;
		}

		if(script_info.original_script.start == NULL) {
			script_info.original_script = g_default_ass_script_name;
		}
	}

	*script_info_result = script_info;

	return ErrorTypeNone;
	// end of script info
}

typedef struct {
	ZVEC_TYPENAME(FinalStr) entries;
} TempDataArray;

typedef struct {
	FinalStr name_raw;
	TempDataArray data_raw;
} TempEncodingEntry;

#define EMPTY_TEMP_ENCODE_ENTRY() \
	((TempEncodingEntry){ .name_raw = \
	                          (FinalStr){ .start = 0, .length = 0, .file_pos = EMPTY_POS() }, \
	                      .data_raw = (TempDataArray){ .entries = ZVEC_EMPTY(FinalStr) } })

#define IS_EMPTY_ENCODE_ENTRY_DATA(entry) (ZVEC_LENGTH((entry).data_raw.entries) == 0)

#define IS_EMPTY_ENCODE_NAME(entry) ((entry).name_raw.length == 0)

#define IS_EMPTY_ENCODE_ENTRY(entry) \
	((IS_EMPTY_ENCODE_ENTRY_DATA(entry)) && (IS_EMPTY_ENCODE_NAME(entry)))

#define POS_FROM_ENCODE_ENTRY(entry) \
	((!IS_EMPTY_ENCODE_NAME(entry)) \
	     ? ((entry).name_raw.file_pos) \
	     : ((IS_EMPTY_ENCODE_ENTRY_DATA(entry)) \
	            ? (EMPTY_POS()) \
	            : ((ZVEC_AT(FinalStr, ((entry).data_raw.entries), 0)).file_pos)))

[[nodiscard]] static MessageStruct parse_font_name_attributes(FinalStr attributes,
                                                              AssFontName* name_result,
                                                              Diagnostics* diagnostics) {

	StrView attributes_view = get_str_view_from_const_str_view(attributes);

	name_result->bold = false;
	name_result->italic = false;

	if(str_view_starts_with_ascii(attributes_view, "B")) {
		name_result->bold = true;

		if(!str_view_advance(&attributes_view, 1)) {
			return STATIC_MESSAGE_STRUCT("implementation error");
		}
	}

	if(str_view_starts_with_ascii(attributes_view, "I")) {
		name_result->italic = true;

		if(!str_view_advance(&attributes_view, 1)) {
			return STATIC_MESSAGE_STRUCT("implementation error");
		}
	}

	MessageStruct error = EMPTY_MESSAGE_STRUCT();

	ConstStrView encoding_value = get_const_str_view_from_str_view(attributes_view);

	size_t encoding = parse_str_as_unsigned_number(encoding_value, &error, diagnostics);

	if(!is_empty_message_struct(error)) {

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		free_message_struct(error); \
		return STATIC_MESSAGE_STRUCT(message); \
	} while(false)

		char* result_buffer = NULL;
		FORMAT_STRING_PROPAGATE_ERROR(&result_buffer, "couldn't parse font encoding: %s",
		                              get_message(error));

#undef PROPAGATE_ERROR_IMPL

		free_message_struct(error);

		return DYNAMIC_MESSAGE_STRUCT(result_buffer);
	}

	name_result->encoding = encoding;

	return EMPTY_MESSAGE_STRUCT();
}

[[nodiscard]] static MessageStruct parse_font_name(FinalStr name_raw, AssFontName* result_name,
                                                   Diagnostics* diagnostics) {

	// NOTE: <name of file> is the file name that SSA will use when saving the font file. It is:
	//  the name of the original truetype font,
	//  plus an underscore,
	//  plus an optional “B” if Bold,
	//  plus an optional “I” if Italic,
	//  plus a number specifying the font encoding (character set),
	//  plus “.ttf”

	AssFontName name = { .italic = false, .bold = false };

	StrView name_view = get_str_view_from_const_str_view(name_raw);

#define TTF_EXT ".ttf"

	if(!str_view_ends_with_ascii(name_view, TTF_EXT)) {
		return STATIC_MESSAGE_STRUCT("Only ttf fonts supported, got unknown extension");
	}

	if(!str_view_advance_from_end(&name_view, (sizeof(TTF_EXT) - 1))) {
		return STATIC_MESSAGE_STRUCT("implementation error");
	}

	ConstStrView name_without_ext = get_const_str_view_from_str_view(name_view);

	size_t underscore_idx = 0;
	bool underscore_found = false;

	for(size_t i = 0; i < name_without_ext.length; ++i) {
		int32_t value = name_without_ext.start[name_without_ext.length - 1 - i];

		if(value == ((unsigned char)'_')) {
			underscore_idx = i;
			underscore_found = true;
			break;
		}
	}

	if(!underscore_found) {
		return STATIC_MESSAGE_STRUCT("font name requires an underscore, none found");
	}

	size_t final_name_length = name_without_ext.length - underscore_idx - 1;

	FinalStr final_name = { .start = name_without_ext.start,
		                    .length = final_name_length,
		                    .file_pos = name_without_ext.file_pos };

	if(str_view_eq_ascii(final_name, "")) {
		return STATIC_MESSAGE_STRUCT("empty font name not permitted");
	}

	name.name = final_name;

	FinalStr attributes = { .start = name_without_ext.start + final_name_length + 1,
		                    .length = underscore_idx,
		                    .file_pos = { .line = name_without_ext.file_pos.line,
		                                  .column = name_without_ext.file_pos.column +
		                                            final_name_length + 1 } };

	MessageStruct attributes_result = parse_font_name_attributes(attributes, &name, diagnostics);

	if(!is_empty_message_struct(attributes_result)) {
		return attributes_result;
	}

	*result_name = name;
	return EMPTY_MESSAGE_STRUCT();
}

ZVEC_DEFINE_VEC_TYPE(char)

[[nodiscard]] static MessageStruct parse_uu_encoded_data(TempDataArray data_raw,
                                                         SizedPtr* result_data) {

	ZVEC_TYPENAME(char) final_data = ZVEC_EMPTY(char);

	for(size_t i = 0; i < ZVEC_LENGTH(data_raw.entries); ++i) {
		FinalStr entry = ZVEC_AT(FinalStr, (data_raw.entries), i);

		char* entry_normalized = get_normalized_string(entry);

		if(!entry_normalized) {
			ZVEC_FREE(char, &final_data);
			return STATIC_MESSAGE_STRUCT("allocation error");
		}

		size_t normalized_length = strlen(entry_normalized);

		size_t current_arr_size = ZVEC_LENGTH(final_data);

		ASSERT(ZVEC_RESERVE(char, &final_data, current_arr_size + normalized_length) ==
		           ZvecResultOk,
		       "OOM");

		for(size_t j = 0; j < normalized_length; ++j) {
			ZVEC_PUSH_AND_ASSERT(char, &final_data, entry_normalized[j]);
		}

		free(entry_normalized);
	}

	SizedPtr input = { .data = ZVEC_DATA(char, &final_data), .len = ZVEC_LENGTH(final_data) };

	SizedPtr decode_result = uu_decode(input);

	ZVEC_FREE(char, &final_data);

	if(is_ptr_error(decode_result)) {
#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		return STATIC_MESSAGE_STRUCT(message); \
	} while(false)

		char* result_buffer = NULL;
		FORMAT_STRING_PROPAGATE_ERROR(&result_buffer,
		                              "encoded data parse error: failed to decode uu encoding: %s",
		                              ptr_get_error(decode_result));

#undef PROPAGATE_ERROR_IMPL

		return DYNAMIC_MESSAGE_STRUCT(result_buffer);
	}

	*result_data = decode_result;

	return EMPTY_MESSAGE_STRUCT();
}

[[nodiscard]] static MessageStruct process_font(TempEncodingEntry entry_data,
                                                AssFontEntry* out_entry, Diagnostics* diagnostics) {

	if(IS_EMPTY_ENCODE_NAME(entry_data)) {
		return STATIC_MESSAGE_STRUCT("Couldn't parse font, no font name before data specified");
	}

	if(IS_EMPTY_ENCODE_ENTRY_DATA(entry_data)) {
		return STATIC_MESSAGE_STRUCT("Couldn't parse font, no font data specified");
	}

	MessageStruct font_name_result =
	    parse_font_name(entry_data.name_raw, &(out_entry->name), diagnostics);

	if(!is_empty_message_struct(font_name_result)) {
		return font_name_result;
	}

	MessageStruct font_data_result = parse_uu_encoded_data(entry_data.data_raw, &(out_entry->data));

	if(!is_empty_message_struct(font_data_result)) {
		return font_data_result;
	}

	return EMPTY_MESSAGE_STRUCT();
}

[[nodiscard]] static bool are_all_uu_encodings_fast(ConstStrView str_view) {

	for(size_t i = 0; i < str_view.length; ++i) {

		int32_t val = str_view.start[i];

		// uu encodings start at 33 (so space is under it e.g.)
		if(val < 33) {
			return false;
		}

		// an uu encoding has max 6 bytes (0x3F max value) + 33
		if(val > (33 + 0x3F)) {
			return false;
		}
	}

	return true;
}

[[nodiscard]] static ErrorType parse_fonts(AssFonts* ass_fonts, StrView* data_view,
                                           LineType line_type, Diagnostics* diagnostics) {

	AssFonts fonts = { .entries = ZVEC_EMPTY(AssFontEntry) };

	TempEncodingEntry temp_entry = EMPTY_TEMP_ENCODE_ENTRY();

#define PROCESS_FONT(entry, pos) \
	do { \
		AssFontEntry result_font = {}; \
		MessageStruct font_process_result = process_font(entry, &result_font, diagnostics); \
		if(!is_empty_message_struct(font_process_result)) { \
			INSERT_SIMPLE_ERROR(diagnostics->entries, font_process_result, (pos)); \
		} else { \
			ZVEC_PUSH_SLOT_AND_ASSERT(AssFontEntry, &(fonts.entries), result_font); \
		} \
	} while(false)

#define FREE_FONT_ENTRY(entry) \
	do { \
		ZVEC_FREE(FinalStr, &((entry).data_raw.entries)); \
	} while(false)

#define FREE_AT_END() \
	do { \
		FREE_FONT_ENTRY(temp_entry); \
		ZVEC_FREE(AssFontEntry, &fonts.entries); \
	} while(false)

	// Note: we collect data, as the font data is over multiple
	// lines, than after we got enough data, which has many
	// possible ways of occuring, we create a final font entry and
	// add it to the fonts!

	while(true) {

		// NOTE: we can't just search for a line that starts with "[", since that may occur in the
		// uu encoding, so we try to eliminate that by checking for valid section starts, that means
		// lines in this regex:
		// "^\[.*\]$", this is still not enough, we also check, that the characters in between []
		// are all valid uu encoding, if this is not the case, we have a section (this is necessary,
		// as uu encodings may have both charcaters [ and ] inside it, and even if most lines need
		// to be 80 codepoints long, the last line isn't that long)
		if(str_view_starts_with_ascii(*data_view, "[")) {

			StrView str_view_copy = { .start = data_view->start,
				                      .length = data_view->length,
				                      .position = data_view->position };

			ConstStrView peek_line = {};
			if(!str_view_get_substring_until_eol(&str_view_copy, &peek_line, line_type, true)) {
				INSERT_SIMPLE_ERROR(diagnostics->entries,
				                    STATIC_MESSAGE_STRUCT("implementation error"),
				                    data_view->position.file_pos);

				FREE_AT_END();
				return ErrorTypeFatal;
			}

			StrView peek_line_view = get_str_view_from_const_str_view(peek_line);

			if(str_view_ends_with_ascii(peek_line_view, "]")) {

				ConstStrView all_data = { .start = peek_line.start + 1,
					                      .length = peek_line.length - 2,
					                      .file_pos = EMPTY_POS() };

				if(!are_all_uu_encodings_fast(all_data)) {
					goto got_new_section_font;
				}
			}
		}

		ConstStrView line = {};
		if(!str_view_get_substring_until_eol(data_view, &line, line_type, true)) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("implementation error"),
			                    data_view->position.file_pos);

			FREE_AT_END();
			return ErrorTypeFatal;
		}

		{

			if(line.length == 0) {
				// if we already have a font, process that, otherwise skip this line
				if(!IS_EMPTY_ENCODE_ENTRY(temp_entry)) {
					PROCESS_FONT(temp_entry, POS_FROM_ENCODE_ENTRY(temp_entry));
					FREE_FONT_ENTRY(temp_entry);
					temp_entry = EMPTY_TEMP_ENCODE_ENTRY();
				}
				continue;
			}

			StrView line_view = get_str_view_from_const_str_view(line);

			if(str_view_expect_ascii(&line_view, "fontname:")) {

				if(!str_view_skip_optional_whitespace(&line_view)) {
					INSERT_SIMPLE_ERROR(diagnostics->entries,
					                    STATIC_MESSAGE_STRUCT("skip whitespace error"),
					                    line_view.position.file_pos);

					FREE_AT_END();
					return ErrorTypeFatal;
				}

				ConstStrView fontname_raw = get_const_str_view_from_str_view(line_view);

				// if we already have a font, process that first,
				// before creating a new font
				if(!IS_EMPTY_ENCODE_ENTRY(temp_entry)) {
					PROCESS_FONT(temp_entry, POS_FROM_ENCODE_ENTRY(temp_entry));
					FREE_FONT_ENTRY(temp_entry);
					temp_entry = EMPTY_TEMP_ENCODE_ENTRY();
				}

				temp_entry.name_raw = fontname_raw;
			} else {

				// add to the raw data!

				ZVEC_PUSH_SLOT_AND_ASSERT(FinalStr, &(temp_entry.data_raw.entries), line);
			}
		}

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

got_new_section_font:

	// if we just run out of lines and we didn't process the
	// current font, do that
	if(!IS_EMPTY_ENCODE_ENTRY(temp_entry)) {
		PROCESS_FONT(temp_entry, POS_FROM_ENCODE_ENTRY(temp_entry));
		FREE_FONT_ENTRY(temp_entry);
		temp_entry = EMPTY_TEMP_ENCODE_ENTRY();
	}

	*ass_fonts = fonts;

	return ErrorTypeNone;
}

#undef FREE_FONT_ENTRY
#undef FREE_AT_END

[[nodiscard]] static MessageStruct process_graphic(TempEncodingEntry entry_data,
                                                   AssGraphicEntry* out_entry) {

	if(IS_EMPTY_ENCODE_NAME(entry_data)) {
		return STATIC_MESSAGE_STRUCT("Couldn't parse graphic, no file name before data specified");
	}

	if(IS_EMPTY_ENCODE_ENTRY_DATA(entry_data)) {
		return STATIC_MESSAGE_STRUCT("Couldn't parse graphic, no graphic data specified");
	}

	out_entry->name = entry_data.name_raw;

	MessageStruct graphic_data_result =
	    parse_uu_encoded_data(entry_data.data_raw, &(out_entry->data));

	if(!is_empty_message_struct(graphic_data_result)) {
		return graphic_data_result;
	}

	return EMPTY_MESSAGE_STRUCT();
}

[[nodiscard]] static ErrorType parse_graphics(AssGraphics* ass_graphics, StrView* data_view,
                                              LineType line_type, Diagnostics* diagnostics) {

	AssGraphics graphics = { .entries = ZVEC_EMPTY(AssGraphicEntry) };

	TempEncodingEntry temp_entry = EMPTY_TEMP_ENCODE_ENTRY();

#define PROCESS_GRAPHIC(entry, pos) \
	do { \
		AssGraphicEntry result_graphic = {}; \
		MessageStruct graphic_process_result = process_graphic(entry, &result_graphic); \
		if(!is_empty_message_struct(graphic_process_result)) { \
			INSERT_SIMPLE_ERROR(diagnostics->entries, graphic_process_result, (pos)); \
		} else { \
			ZVEC_PUSH_SLOT_AND_ASSERT(AssGraphicEntry, &(graphics.entries), result_graphic); \
		} \
	} while(false)

#define FREE_GRAPHIC_ENTRY(entry) \
	do { \
		ZVEC_FREE(FinalStr, &((entry).data_raw.entries)); \
	} while(false)

#define FREE_AT_END() \
	do { \
		FREE_GRAPHIC_ENTRY(temp_entry); \
		ZVEC_FREE(AssGraphicEntry, &(graphics.entries)); \
	} while(false)

	// Note: we collect data, as the graphics data is over multiple
	// lines, than after we got enough data, which has many
	// possible ways of occuring, we create a final graphic entry and
	// add it to the graphics!

	while(true) {

		// NOTE: we can't just search for a line that starts with "[", since that may occur in the
		// uu encoding, so we try to eliminate that by checking for valid section starts, that means
		// lines in this regex:
		// "^\[.*\]$", this is still not enough, we also check, that the characters in between []
		// are all valid uu encoding, if this is not the case, we have a section (this is necessary,
		// as uu encodings may have both charcaters [ and ] inside it, and even if most lines need
		// to be 80 codepoints long, the last line isn't that long)
		if(str_view_starts_with_ascii(*data_view, "[")) {

			StrView str_view_copy = { .start = data_view->start,
				                      .length = data_view->length,
				                      .position = data_view->position };

			ConstStrView peek_line = {};
			if(!str_view_get_substring_until_eol(&str_view_copy, &peek_line, line_type, true)) {
				INSERT_SIMPLE_ERROR(diagnostics->entries,
				                    STATIC_MESSAGE_STRUCT("implementation error"),
				                    data_view->position.file_pos);

				FREE_AT_END();
				return ErrorTypeFatal;
			}

			StrView peek_line_view = get_str_view_from_const_str_view(peek_line);

			if(str_view_ends_with_ascii(peek_line_view, "]")) {

				ConstStrView all_data = { .start = peek_line.start + 1,
					                      .length = peek_line.length - 2,
					                      .file_pos = EMPTY_POS() };

				if(!are_all_uu_encodings_fast(all_data)) {
					goto got_new_section_graphic;
				}
			}
		}

		ConstStrView line = {};
		if(!str_view_get_substring_until_eol(data_view, &line, line_type, true)) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("implementation error"),
			                    data_view->position.file_pos);

			FREE_AT_END();
			return ErrorTypeFatal;
		}

		{

			if(line.length == 0) {
				// if we already have a graphic, process that, otherwise skip this line
				if(!IS_EMPTY_ENCODE_ENTRY(temp_entry)) {
					PROCESS_GRAPHIC(temp_entry, POS_FROM_ENCODE_ENTRY(temp_entry));
					FREE_GRAPHIC_ENTRY(temp_entry);
					temp_entry = EMPTY_TEMP_ENCODE_ENTRY();
				}
				continue;
			}

			StrView line_view = get_str_view_from_const_str_view(line);

			if(str_view_expect_ascii(&line_view, "filename:")) {

				if(!str_view_skip_optional_whitespace(&line_view)) {
					INSERT_SIMPLE_ERROR(diagnostics->entries,
					                    STATIC_MESSAGE_STRUCT("skip whitespace error"),
					                    line_view.position.file_pos);

					FREE_AT_END();
					return ErrorTypeFatal;
				}

				ConstStrView filename_raw = get_const_str_view_from_str_view(line_view);

				// if we already have a graphic, process that first,
				// before creating a new graphic
				if(!IS_EMPTY_ENCODE_ENTRY(temp_entry)) {
					PROCESS_GRAPHIC(temp_entry, POS_FROM_ENCODE_ENTRY(temp_entry));
					FREE_GRAPHIC_ENTRY(temp_entry);
					temp_entry = EMPTY_TEMP_ENCODE_ENTRY();
				}

				temp_entry.name_raw = filename_raw;
			} else {

				// add to the raw data!

				ZVEC_PUSH_SLOT_AND_ASSERT(FinalStr, &(temp_entry.data_raw.entries), line);
			}
		}

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

got_new_section_graphic:

	// if we just run out of lines and we didn't process the
	// current graphic, do that
	if(!IS_EMPTY_ENCODE_ENTRY(temp_entry)) {
		PROCESS_GRAPHIC(temp_entry, POS_FROM_ENCODE_ENTRY(temp_entry));
		FREE_GRAPHIC_ENTRY(temp_entry);
		temp_entry = EMPTY_TEMP_ENCODE_ENTRY();
	}

	*ass_graphics = graphics;

	return ErrorTypeNone;
}

#undef FREE_GRAPHIC_ENTRY
#undef FREE_AT_END

[[nodiscard]] static ErrorType extra_section(ConstStrView section_name, StrView* data_view,
                                             ExtraSections* extra_sections, LineType line_type,
                                             Diagnostics* diagnostics) {

	char* section_name_str = get_normalized_string(section_name);

	if(section_name_str == NULL) {
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
		                    data_view->position.file_pos);
		return ErrorTypeFatal;
	}

	ExtraSectionEntry extra_section_entry = ZMAP_INIT_WITH_DEFAULTS(SectionFieldEntry, CString);

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstStrView line = {};
		if(!str_view_get_substring_until_eol(data_view, &line, line_type, true)) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("implementation error"),
			                    data_view->position.file_pos);
			return ErrorTypeFatal;
		}

		{

			if(line.length == 0) {
				continue;
			}

			StrView line_view = get_str_view_from_const_str_view(line);

			ConstStrView field = {};
			if(!str_view_get_substring_by_char_delimiter(&line_view, &field, ':', false)) {

				INSERT_SIMPLE_ERROR(diagnostics->entries,
				                    STATIC_MESSAGE_STRUCT("end of line before ':' in line parsing "
				                                          "in extra section"),
				                    data_view->position.file_pos);

				/*** @see keep-going */
				continue;
			}

			if(!str_view_skip_optional_whitespace(&line_view)) {
				INSERT_SIMPLE_ERROR(diagnostics->entries,
				                    STATIC_MESSAGE_STRUCT("skip whitespace error"),
				                    line_view.position.file_pos);
				return ErrorTypeFatal;
			}

			ConstStrView field_entry_value = {};

			if(!str_view_get_substring_until_eof(&line_view, &field_entry_value, false,
			                                     NO_LINE_TYPE)) {
				INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("eof error"),
				                    line_view.position.file_pos);
				return ErrorTypeFatal;
			}

			const char* field_entry_key = get_normalized_string(field);

			{
				// NOTE: don't care for overwrites
				ZVEC_SHOULD_USE_INSERT_SLOT(field_entry_value);
				FinalStr* slot = ZMAP_INSERT_SLOT(SectionFieldEntry, &extra_section_entry,
				                                  field_entry_key, true);
				ASSERT(slot != NULL, "OOM");
				*slot = field_entry_value;
			}
		}

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

	{
		// NOTE: don't care for overwrites
		ZVEC_SHOULD_USE_INSERT_SLOT(extra_section_entry);
		ExtraSectionEntry* slot =
		    ZMAP_INSERT_SLOT(ExtraSectionHashMapEntry, extra_sections, section_name_str, true);
		ASSERT(slot != NULL, "OOM");
		*slot = extra_section_entry;
	}

	return ErrorTypeNone;
}

ZVEC_DEFINE_AND_IMPLEMENT_VEC_TYPE(AssEventFormat)

[[nodiscard]] static ErrorType
parse_format_line_for_events(const ConstStrView line, ZVEC_TYPENAME(AssEventFormat) * format_result,
                             Diagnostics* diagnostics) {

	StrView line_view = get_str_view_from_const_str_view(line);

	while(!(str_view_is_eof(line_view))) {

		if(!str_view_skip_optional_whitespace(&line_view)) {
			INSERT_SIMPLE_ERROR(diagnostics->entries,
			                    STATIC_MESSAGE_STRUCT("skip whitespace error"),
			                    line_view.position.file_pos);
			return ErrorTypeFatal;
		}

		ConstStrView key = {};
		if(!str_view_get_substring_by_char_delimiter(&line_view, &key, ',', true)) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("implementation error"),
			                    line_view.position.file_pos);
			return ErrorTypeFatal;
		}

		if(key.length == 0) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("implementation error"),
			                    line_view.position.file_pos);
			return ErrorTypeFatal;
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
				INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
				                    line_view.position.file_pos);
				return ErrorTypeFatal;
			}

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		free(key_name); \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), \
		                    line_view.position.file_pos); \
		return ErrorTypeFatal; \
	} while(false)

			char* result_buffer = NULL;
			FORMAT_STRING_PROPAGATE_ERROR(&result_buffer,
			                              "unrecognized format key '%s' in format line in "
			                              "events section",
			                              key_name);

#undef PROPAGATE_ERROR_IMPL

			free(key_name);

			INSERT_SIMPLE_WARNING(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
			                      line_view.position.file_pos);

			format = AssEventFormatUnknownField;
		}

		ZVEC_PUSH_AND_ASSERT(AssEventFormat, format_result, format);
	}

	return ErrorTypeNone;
}

[[nodiscard]] static ErrorType parse_event_line_for_events(EventType type, const ConstStrView line,
                                                           const ZVEC_TYPENAME(AssEventFormat)
                                                               format_spec,
                                                           AssEvents* events_result,
                                                           Diagnostics* diagnostics) {

	size_t field_size = ZVEC_LENGTH(format_spec);

	AssEventEntry entry = { .type = type };

	StrView line_view = get_str_view_from_const_str_view(line);

	if(!str_view_skip_optional_whitespace(&line_view)) {
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("skip whitespace error"),
		                    line_view.position.file_pos);
		return ErrorTypeFatal;
	}

	size_t i = 0; // NOLINT(readability-identifier-length)

	for(bool are_at_end = false; !are_at_end; ++i) {

		if(i >= field_size) {

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), \
		                    line_view.position.file_pos); \
		return ErrorTypeFatal; \
	} while(false)

			char* result_buffer = NULL;
			FORMAT_STRING_PROPAGATE_ERROR(&result_buffer,
			                              "error, too many fields in the event line, the "
			                              "format line "
			                              "specified %lu, but we are already at %lu",
			                              field_size, (i + 1));

#undef PROPAGATE_ERROR_IMPL

			INSERT_SIMPLE_ERROR(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
			                    line_view.position.file_pos);
			return ErrorTypeFatal;
		}

		AssEventFormat format = ZVEC_AT(AssEventFormat, format_spec, i);

		// special handling fot the text field, as it may contain
		// ","

		ConstStrView value = {};

		if(format == AssEventFormatText) {
			if(i != field_size - 1) {

				INSERT_SIMPLE_ERROR(diagnostics->entries,
				                    STATIC_MESSAGE_STRUCT("'Text' field of event lines may only "
				                                          "occur at the last position!"),
				                    line_view.position.file_pos);
				return ErrorTypeFatal;
			}

			if(!str_view_get_substring_until_eof(&line_view, &value, false, NO_LINE_TYPE)) {
				INSERT_SIMPLE_ERROR(diagnostics->entries,
				                    STATIC_MESSAGE_STRUCT("implementation error"),
				                    line_view.position.file_pos);
				return ErrorTypeFatal;
			}
			are_at_end = true;

		} else {
			if(!str_view_get_substring_by_char_delimiter(&line_view, &value, ',', true)) {
				INSERT_SIMPLE_ERROR(diagnostics->entries,
				                    STATIC_MESSAGE_STRUCT("implementation error"),
				                    line_view.position.file_pos);
				return ErrorTypeFatal;

				are_at_end = true;
			}
		}

		MessageStruct error = EMPTY_MESSAGE_STRUCT();

		switch(format) {
			case AssEventFormatLayer: {
				entry.layer = parse_str_as_unsigned_number(value, &error, diagnostics);
				break;
			}
			case AssEventFormatStart: {
				entry.start = parse_str_as_time(value, &error, diagnostics);
				break;
			}
			case AssEventFormatEnd: {
				entry.end = parse_str_as_time(value, &error, diagnostics);
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
				entry.margin_l = parse_str_as_margin_value(value, &error, diagnostics);
				break;
			}
			case AssEventFormatMarginR: {
				entry.margin_r = parse_str_as_margin_value(value, &error, diagnostics);
				break;
			}
			case AssEventFormatMarginV: {
				entry.margin_v = parse_str_as_margin_value(value, &error, diagnostics);
				break;
			}
			case AssEventFormatEffect: {
				entry.effect = value;
				break;
			}
			case AssEventFormatText: {
				switch(type) {
					case EventTypeDialogue: {

						// TODO
						AssText todo = { .todo = value };

						entry.text_union.dialogue = todo;
						break;
					}
					case EventTypeComment: {
						// TODO
						AssText todo = { .todo = value };

						entry.text_union.comment = todo;
						break;
					}
					case EventTypePicture: {
						entry.text_union.picture = value;
						break;
					}
					case EventTypeSound: {
						entry.text_union.sound = value;
						break;
					}
					case EventTypeMovie: {
						entry.text_union.movie = value;
						break;
					}
					case EventTypeCommand: {

						// TODO
						CommandEvent todo = { .ssa_command = false, .data = { .string = value } };

						entry.text_union.command = todo;
						break;
					}
					default: {
						UNREACHABLE();
					}
				}

				break;
			}
			case AssEventFormatUnknownField: {
				INSERT_SIMPLE_WARNING(diagnostics->entries,
				                      STATIC_MESSAGE_STRUCT("ignoring unknown event field entry"),
				                      value.file_pos);
				goto skip_entry_event;
			}
			default: {
				UNREACHABLE();
			}
		}

		if(!is_empty_message_struct(error)) {

			char* value_name = get_normalized_string(value);

			if(!value_name) {
				INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
				                    line_view.position.file_pos);
				return ErrorTypeFatal;
			}

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		free_message_struct(error); \
		free(value_name); \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), \
		                    line_view.position.file_pos); \
		return ErrorTypeFatal; \
	} while(false)

			char* result_buffer = NULL;
			FORMAT_STRING_PROPAGATE_ERROR(
			    &result_buffer, "While parsing field '%s' with value '%s': %s",
			    get_name_for_event_format(format), value_name, get_message(error));

#undef PROPAGATE_ERROR_IMPL

			free_message_struct(error);
			free(value_name);

			INSERT_SIMPLE_ERROR(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
			                    line_view.position.file_pos);

			/*** @see keep-going */
			continue;
		}
	skip_entry_event:
	}

	if(i != field_size) {

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), \
		                    line_view.position.file_pos); \
		return ErrorTypeFatal; \
	} while(false)

		char* result_buffer = NULL;
		FORMAT_STRING_PROPAGATE_ERROR(&result_buffer,
		                              "error, too few fields in the event line, the format "
		                              "line "
		                              "specified %lu, but we only have %lu",
		                              field_size, i);

#undef PROPAGATE_ERROR_IMPL

		INSERT_SIMPLE_ERROR(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
		                    line_view.position.file_pos);
		return ErrorTypeFatal;
	}

	ZVEC_PUSH_SLOT_AND_ASSERT(AssEventEntry, &(events_result->entries), entry);

	return ErrorTypeNone;
}

[[nodiscard]] static ErrorType parse_events(AssEvents* ass_events, StrView* data_view,
                                            ParseSettings settings, LineType line_type,
                                            Diagnostics* diagnostics) {

	AssEvents events = { .entries = ZVEC_EMPTY(AssEventEntry) };

	ZVEC_TYPENAME(AssEventFormat) event_format = ZVEC_EMPTY(AssEventFormat);

#define FREE_AT_END() \
	do { \
		ZVEC_FREE(AssEventEntry, &(events.entries)); \
		ZVEC_FREE(AssEventFormat, &event_format); \
	} while(false)

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstStrView line = {};
		if(!str_view_get_substring_until_eol(data_view, &line, line_type, true)) {
			FREE_AT_END();

			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("implementation error"),
			                    data_view->position.file_pos);
			return ErrorTypeFatal;
		}

		// parse line
		{

			if(line.length == 0) {
				continue;
			}

			StrView line_view = get_str_view_from_const_str_view(line);

			ConstStrView field = {};
			if(!str_view_get_substring_by_char_delimiter(&line_view, &field, ':', false)) {
				FREE_AT_END();

				INSERT_SIMPLE_ERROR(diagnostics->entries,
				                    STATIC_MESSAGE_STRUCT("end of line before ':' in line parsing "
				                                          "in events section"),
				                    data_view->position.file_pos);
				return ErrorTypeFatal;
			}

			if(str_view_eq_ascii(field, "Format")) {

				if(ZVEC_LENGTH(event_format) != 0) {
					INSERT_SIMPLE_ERROR(diagnostics->entries,
					                    STATIC_MESSAGE_STRUCT("multiple format fields detected in "
					                                          "the events section, this is not "
					                                          "allowed"),
					                    data_view->position.file_pos);

					/*** @see keep-going */
					continue;
				}

				ErrorType format_line_error = parse_format_line_for_events(
				    get_const_str_view_from_str_view(line_view), &event_format, diagnostics);

				if(format_line_error != ErrorTypeNone) {
					/*** @see keep-going */
					continue;
				}

			} else if(str_view_eq_ascii(field, "Dialogue")) {

				if(ZVEC_LENGTH(event_format) == 0) {
					FREE_AT_END();

					INSERT_SIMPLE_ERROR(diagnostics->entries,
					                    STATIC_MESSAGE_STRUCT("no format line occurred before the "
					                                          "style line in the events section, "
					                                          "this is an error"),
					                    line_view.position.file_pos);
					return ErrorTypeFatal;
				}

				ErrorType event_parse_error = parse_event_line_for_events(
				    EventTypeDialogue, get_const_str_view_from_str_view(line_view), event_format,
				    &events, diagnostics);

				if(event_parse_error != ErrorTypeNone) {
					/*** @see keep-going */
					continue;
				}

			} else if(str_view_eq_ascii(field, "Comment")) {

				if(ZVEC_LENGTH(event_format) == 0) {
					FREE_AT_END();

					INSERT_SIMPLE_ERROR(diagnostics->entries,
					                    STATIC_MESSAGE_STRUCT("no format line occurred before the "
					                                          "style line in the events section, "
					                                          "this is an error"),
					                    line_view.position.file_pos);
					return ErrorTypeFatal;
				}

				ErrorType event_parse_error = parse_event_line_for_events(
				    EventTypeComment, get_const_str_view_from_str_view(line_view), event_format,
				    &events, diagnostics);

				if(event_parse_error != ErrorTypeNone) {
					/*** @see keep-going */
					continue;
				}

			} else if(str_view_eq_ascii(field, "Picture")) {

				if(ZVEC_LENGTH(event_format) == 0) {
					FREE_AT_END();

					INSERT_SIMPLE_ERROR(diagnostics->entries,
					                    STATIC_MESSAGE_STRUCT("no format line occurred before the "
					                                          "style line in the events section, "
					                                          "this is an error"),
					                    line_view.position.file_pos);
					return ErrorTypeFatal;
				}

				ErrorType event_parse_error = parse_event_line_for_events(
				    EventTypePicture, get_const_str_view_from_str_view(line_view), event_format,
				    &events, diagnostics);

				if(event_parse_error != ErrorTypeNone) {
					/*** @see keep-going */
					continue;
				}

			} else if(str_view_eq_ascii(field, "Sound")) {

				if(ZVEC_LENGTH(event_format) == 0) {
					FREE_AT_END();

					INSERT_SIMPLE_ERROR(diagnostics->entries,
					                    STATIC_MESSAGE_STRUCT("no format line occurred before the "
					                                          "style line in the events section, "
					                                          "this is an error"),
					                    line_view.position.file_pos);
					return ErrorTypeFatal;
				}

				ErrorType event_parse_error = parse_event_line_for_events(
				    EventTypeSound, get_const_str_view_from_str_view(line_view), event_format,
				    &events, diagnostics);

				if(event_parse_error != ErrorTypeNone) {
					/*** @see keep-going */
					continue;
				}

			} else if(str_view_eq_ascii(field, "Movie")) {

				if(ZVEC_LENGTH(event_format) == 0) {
					FREE_AT_END();

					INSERT_SIMPLE_ERROR(diagnostics->entries,
					                    STATIC_MESSAGE_STRUCT("no format line occurred before the "
					                                          "style line in the events section, "
					                                          "this is an error"),
					                    line_view.position.file_pos);
					return ErrorTypeFatal;
				}

				ErrorType event_parse_error = parse_event_line_for_events(
				    EventTypeMovie, get_const_str_view_from_str_view(line_view), event_format,
				    &events, diagnostics);

				if(event_parse_error != ErrorTypeNone) {
					/*** @see keep-going */
					continue;
				}

			} else if(str_view_eq_ascii(field, "Command")) {

				if(ZVEC_LENGTH(event_format) == 0) {
					FREE_AT_END();

					INSERT_SIMPLE_ERROR(diagnostics->entries,
					                    STATIC_MESSAGE_STRUCT("no format line occurred before the "
					                                          "style line in the events section, "
					                                          "this is an error"),
					                    line_view.position.file_pos);
					return ErrorTypeFatal;
				}

				ErrorType event_parse_error = parse_event_line_for_events(
				    EventTypeCommand, get_const_str_view_from_str_view(line_view), event_format,
				    &events, diagnostics);

				if(event_parse_error != ErrorTypeNone) {
					/*** @see keep-going */
					continue;
				}

			} else {

				const DiagnosticSeverity severity_type =
				    settings.strict_settings.allow_additional_fields ? DiagnosticSeverityWarning
				                                                     : DiagnosticSeverityError;

				UnexpectedFieldDiagnostic unexpected_field = { .field = field,
					                                           .section = "events" };

				DiagnosticEntry diagnostic = {
					.inner = (InnerDiagnostic){ .type = DiagnosticTypeUnexpectedField,
					                            .data = { .unexpected_field = unexpected_field } },
					.severity = severity_type,
					.position = field.file_pos
				};

				ZVEC_PUSH_SLOT_AND_ASSERT(DiagnosticEntry, &(diagnostics->entries), diagnostic);

				/*** @see keep-going */
				continue;
			}
		}

		// end of line parse

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

	ZVEC_FREE(AssEventFormat, &(event_format));

	*ass_events = events;

	return ErrorTypeNone;
	// end of script info
}

#undef FREE_AT_END

[[nodiscard]] static ErrorType get_section_by_name(ConstStrView section_name, AssResult* ass_result,
                                                   StrView* data_view, ParseSettings settings,
                                                   LineType line_type, Diagnostics* diagnostics) {

	if(str_view_eq_ascii(section_name, "V4+ Styles")) {
		return parse_styles(&(ass_result->styles), data_view, settings, line_type, diagnostics);
	}

	if(str_view_eq_ascii(section_name, "V4 Styles")) {

		INSERT_SIMPLE_ERROR(diagnostics->entries,
		                    STATIC_MESSAGE_STRUCT("v4 styles are not supported"),
		                    data_view->position.file_pos);
		return ErrorTypeFatal;
	}

	if(str_view_eq_ascii(section_name, "Events")) {
		return parse_events(&(ass_result->events), data_view, settings, line_type, diagnostics);
	}

	if(str_view_eq_ascii(section_name, "Fonts")) {
		return parse_fonts(&(ass_result->fonts), data_view, line_type, diagnostics);
	}

	if(str_view_eq_ascii(section_name, "Graphics")) {
		return parse_graphics(&(ass_result->graphics), data_view, line_type, diagnostics);
	}

	return extra_section(section_name, data_view, &(ass_result->extra_sections), line_type,
	                     diagnostics);

	return ErrorTypeNone;
}

static void free_extra_section_entry(ExtraSectionEntry entry) {
	size_t hm_total_length = ZMAP_CAPACITY(entry);

	for(size_t i = 0; i < hm_total_length; ++i) {
		ZMAP_TYPENAME_BUCKET(SectionFieldEntry) hm_bucket = entry.buckets[i];

		if(hm_bucket.state == ZMAP_OCCUPIED) {
			free(hm_bucket.key);
		}
	}

	ZMAP_FREE(SectionFieldEntry, &entry);
}

static void free_extra_sections(ExtraSections sections) {

	size_t hm_total_length = ZMAP_CAPACITY(sections);

	for(size_t i = 0; i < hm_total_length; ++i) {
		ZMAP_TYPENAME_BUCKET(ExtraSectionHashMapEntry) hm_bucket = sections.buckets[i];

		if(hm_bucket.state == ZMAP_OCCUPIED) {
			free_extra_section_entry(hm_bucket.value);
			free(hm_bucket.key);
		}
	}

	ZMAP_FREE(ExtraSectionHashMapEntry, &sections);
}

static void free_fonts(AssFonts fonts) {
	for(size_t i = 0; i < ZVEC_LENGTH(fonts.entries); ++i) {
		AssFontEntry entry = ZVEC_AT(AssFontEntry, (fonts.entries), i);
		free_sized_ptr(entry.data);
	}
	ZVEC_FREE(AssFontEntry, &(fonts.entries));
}

static void free_graphics(AssGraphics graphics) {
	for(size_t i = 0; i < ZVEC_LENGTH(graphics.entries); ++i) {
		AssGraphicEntry entry = ZVEC_AT(AssGraphicEntry, (graphics.entries), i);
		free_sized_ptr(entry.data);
	}
	ZVEC_FREE(AssGraphicEntry, &(graphics.entries));
}

static void free_ass_result(AssResult data) {
	ZVEC_FREE(AssStyleEntry, &(data.styles.entries));
	ZVEC_FREE(AssEventEntry, &(data.events.entries));

	free_extra_sections(data.extra_sections);
	free_fonts(data.fonts);
	free_graphics(data.graphics);
}

#define FREE_AT_END() \
	do { \
	} while(false)

#define RETURN_ERROR_NO_MESSAGE() \
	do { \
		FREE_AT_END(); \
		result->is_error = true; \
		return result; \
	} while(false)

#define RETURN_ERROR_IMPL(err, pos) \
	do { \
		INSERT_SIMPLE_ERROR(result->diagnostics.entries, err, pos); \
		RETURN_ERROR_NO_MESSAGE(); \
	} while(false)

#define RETURN_ERROR(err) RETURN_ERROR_IMPL(err, data_view.position.file_pos)

#define RETURN_ERROR_AT_START(err) RETURN_ERROR_IMPL(err, EMPTY_POS())

[[nodiscard]] AssParseResult* parse_ass(AssSource source, ParseSettings settings) {

	AssParseResult* result = (AssParseResult*)malloc(sizeof(AssParseResult));

	if(!result) {
		return NULL;
	}

	result->diagnostics = (Diagnostics){ .entries = ZVEC_EMPTY(DiagnosticEntry) };
	result->allocated_codepoints =
	    (Codepoints){ .data = (CodePointsData){ .data_const = NULL, .data_readable = NULL },
		              .size = 0 };

	SizedPtr source_data = get_data_from_source(source);

	if(is_ptr_error(source_data)) {
		RETURN_ERROR_AT_START(STATIC_MESSAGE_STRUCT(ptr_get_error(source_data)));
	}

	FileType file_type = determine_file_type(source_data);

	size_t bom_size = 0;
	CodepointsResult codepoints_result = { .has_error = true,
		                                   .data = { .error = "implementation error" } };

	switch(file_type) {
		case FileTypeUnknown: {
			const char* error = "unrecognized file type, no BOM present";

			if(!settings.strict_settings.allow_unrecognized_file_encoding) {
				RETURN_ERROR_AT_START(STATIC_MESSAGE_STRUCT(error));
			}

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		RETURN_ERROR_AT_START(STATIC_MESSAGE_STRUCT(message)); \
	} while(false)

			char* result_buffer = NULL;
			FORMAT_STRING_PROPAGATE_ERROR(&result_buffer,
			                              "%s, assuming UTF-8 (ascii also works with that)", error);

			INSERT_SIMPLE_WARNING(result->diagnostics.entries,
			                      DYNAMIC_MESSAGE_STRUCT(result_buffer), EMPTY_POS());

#undef PROPAGATE_ERROR_IMPL

			bom_size = 0;
			codepoints_result = get_codepoints_from_utf8(source_data);
			break;
		}
		case FileTypeUtf8: {
			bom_size = 1;
			codepoints_result = get_codepoints_from_utf8(source_data);
			break;
		}
		case FileTypeUtf16BE: {
			bom_size = 1;
			codepoints_result = get_codepoints_from_utf16(source_data, true);
			break;
		}
		case FileTypeUtf16LE: {
			bom_size = 1;
			codepoints_result = get_codepoints_from_utf16(source_data, false);
			break;
		}
		case FileTypeUtf32BE: {
			bom_size = 1;
			codepoints_result = get_codepoints_from_utf32(source_data, true);
			break;
		}
		case FileTypeUtf32LE: {
			bom_size = 1;
			codepoints_result = get_codepoints_from_utf32(source_data, false);
			break;
		}

		default: {
			free_sized_ptr(source_data);

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		RETURN_ERROR_AT_START(STATIC_MESSAGE_STRUCT(message)); \
	} while(false)

			char* result_buffer = NULL;
			FORMAT_STRING_PROPAGATE_ERROR(&result_buffer,
			                              "only UTF-8 encoded files supported atm, but "
			                              "got: %s",
			                              get_file_type_name(file_type));

#undef PROPAGATE_ERROR_IMPL

			RETURN_ERROR_AT_START(DYNAMIC_MESSAGE_STRUCT(result_buffer));
		}
	}

	free_sized_ptr(source_data);

	if(codepoints_result.has_error) {
		RETURN_ERROR_AT_START(STATIC_MESSAGE_STRUCT(codepoints_result.data.error));
	}

	Codepoints final_data = codepoints_result.data.result;

	if(final_data.data.data_const == NULL && final_data.size == 0) {
		RETURN_ERROR_AT_START(
		    STATIC_MESSAGE_STRUCT("file conversion resulted in invalid UTF-8 string"));
	}

	result->allocated_codepoints = final_data;

	StrView data_view = str_view_from_data(final_data);

	if(bom_size > 0) {
		// NOTE: the bom byte is always just one codepoint
		if(!str_view_advance(&data_view, bom_size)) {
			RETURN_ERROR_AT_START(STATIC_MESSAGE_STRUCT("couldn't skip bom bytes"));
		}
	}

	// get line type
	MessageStruct line_type_error = EMPTY_MESSAGE_STRUCT();

	ConstStrView line_type_view = get_const_str_view_from_str_view(data_view);

	LineType line_type = get_line_type(line_type_view, &line_type_error);

	if(!is_empty_message_struct(line_type_error)) {
		RETURN_ERROR_AT_START(line_type_error);
	}

	// parse script info

	if(!str_view_expect_ascii(&data_view, "[Script Info]")) {
		RETURN_ERROR(STATIC_MESSAGE_STRUCT("first line must be the script info section"));
	}

	if(!str_view_expect_newline(&data_view, line_type)) {
		RETURN_ERROR(STATIC_MESSAGE_STRUCT("expected newline"));
	}

	AssResult ass_result = { .extra_sections = { 0 },
		                     .file_props =
		                         (FileProps){ .file_type = file_type, .line_type = line_type },
		                     .events = (AssEvents){ .entries = ZVEC_EMPTY(AssEventEntry) },
		                     .fonts = (AssFonts){ .entries = ZVEC_EMPTY(AssFontEntry) },
		                     .styles = (AssStyles){ .entries = ZVEC_EMPTY(AssStyleEntry) },
		                     .graphics = (AssGraphics){ .entries = ZVEC_EMPTY(AssGraphicEntry) } };

	ass_result.extra_sections = ZMAP_INIT_WITH_DEFAULTS(ExtraSectionHashMapEntry, CString);

#undef FREE_AT_END
#define FREE_AT_END() \
	do { \
		free_ass_result(ass_result); \
	} while(false)

	ErrorType script_info_parse_result = parse_script_info(
	    &(ass_result.script_info), &data_view, settings, line_type, &(result->diagnostics));

	if(script_info_parse_result != ErrorTypeNone) {
		RETURN_ERROR_NO_MESSAGE();
	}

	while(true) {

		// classify section, and then parsing that section

		if(!str_view_expect_ascii(&data_view, "[")) {
			RETURN_ERROR(STATIC_MESSAGE_STRUCT("implementation error"));
		}

		ConstStrView section_name = {};
		if(!str_view_get_substring_by_char_delimiter(&data_view, &section_name, ']', false)) {
			RETURN_ERROR(STATIC_MESSAGE_STRUCT("script section not terminated by ']'"));
		}

		if(!str_view_expect_newline(&data_view, line_type)) {
			RETURN_ERROR(STATIC_MESSAGE_STRUCT("no newline after section name"));
		}

		ErrorType section_parse_result = get_section_by_name(
		    section_name, &ass_result, &data_view, settings, line_type, &(result->diagnostics));

		if(section_parse_result != ErrorTypeNone) {
			RETURN_ERROR_NO_MESSAGE();
		}

		if(str_view_is_eof(data_view)) {
			break;
		}
	}

	validate_ass_result(ass_result, settings, &(result->diagnostics));

	// if we have one error diagnostic, we consider this an error,
	// so we can collect errors until now, and only now report a
	// fatal error
	for(size_t i = 0; i < ZVEC_LENGTH(result->diagnostics.entries); ++i) {
		DiagnosticEntry entry = ZVEC_AT(DiagnosticEntry, (result->diagnostics.entries), i);

		if(entry.severity == DiagnosticSeverityError) {
			RETURN_ERROR_NO_MESSAGE();
		}
	}

	result->is_error = false;
	result->data.ok = ass_result;
	return result;
}

#undef FREE_AT_END
#undef RETURN_ERROR_IMPL
#undef RETURN_ERROR_AT_START
#undef RETURN_ERROR
#undef RETURN_ERROR_NO_MESSAGE

[[nodiscard]] Diagnostics get_diagnostics_from_result(AssParseResult* result) {
	return result->diagnostics;
}

[[nodiscard]] bool parse_result_is_error(AssParseResult* result) {
	if(!result) {
		return true;
	}

	return result->is_error;
}

[[nodiscard]] AssResult parse_result_get_value(AssParseResult* result) {
	return result->data.ok;
}

#ifdef __WASM__

Diagnostics* get_diagnostics_from_result_js(AssParseResult* result) {
	return &(result->diagnostics);
}

AssResult* parse_result_get_value_js(AssParseResult* result) {
	return &(result->data.ok);
}

#endif

void free_parse_result(AssParseResult* result) {
	if(!result->is_error) {
		free_ass_result(result->data.ok);
	}

	free_diagnostics(result->diagnostics);
	free_codepoints(result->allocated_codepoints);

	free(result);
}
