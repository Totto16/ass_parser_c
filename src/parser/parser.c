
#define ASS_PARSER_C_INTERNAL_USAGE

#include "./parser.h"
#include "../helper/io.h"
#include "../helper/macros.h"
#include "../helper/utf_helper.h"
#include "./helper.h"

#undef ASS_PARSER_C_INTERNAL_USAGE

#include <stb/ds.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

struct AssParseResultImpl {
	bool is_error;
	union {
		ErrorStruct error;
		AssResult ok;
	} data;
	Warnings warnings;
	Codepoints allocated_codepoints;
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

[[nodiscard]] static bool str_view_starts_with_ascii_or_eof(StrView str_view,
                                                            const char* ascii_str) {

	if(str_view_starts_with_ascii(str_view, ascii_str)) {
		return true;
	}

	return str_view_is_eof(str_view);
}

[[nodiscard]] static ErrorStruct
parse_format_line_for_styles(StrView* line_view, STBDS_ARRAY(AssStyleFormat) * format_result) {

	while(!(str_view_is_eof(*line_view))) {

		if(!str_view_skip_optional_whitespace(line_view)) {
			return STATIC_ERROR("skip whitespace error");
		}

		ConstStrView key = {};
		if(!str_view_get_substring_by_char_delimiter(line_view, &key, ',', true)) {
			return STATIC_ERROR("implementation error");
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

[[nodiscard]] static ErrorStruct
parse_style_line_for_styles(StrView* line_view, const STBDS_ARRAY(AssStyleFormat) const format_spec,
                            AssStyles* styles_result, ParseSettings settings, Warnings* warnings) {

	size_t field_size = stbds_arrlenu(format_spec);

	AssStyleEntry entry = {};

	if(!str_view_skip_optional_whitespace(line_view)) {
		return STATIC_ERROR("skip whitespace error");
	}

	size_t i = 0;
	for(; !str_view_is_eof(*line_view); ++i) {

		ConstStrView value = {};
		if(!str_view_get_substring_by_char_delimiter(line_view, &value, ',', true)) {
			return STATIC_ERROR("implementation error");
		}

		if(i >= field_size) {

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer,
			                      "error, too many fields in the style line, the format line "
			                      "specified %lu, but we are already at %lu",
			                      field_size, (i + 1));

			return DYNAMIC_ERROR(result_buffer);
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
				    value, &error, settings.strict_settings.allow_number_truncating, warnings);
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
				entry.scale_x = parse_str_as_unsigned_number(value, &error, warnings);
				break;
			}
			case AssStyleFormatScaleY: {
				entry.scale_y = parse_str_as_unsigned_number(value, &error, warnings);
				break;
			}
			case AssStyleFormatSpacing: {
				entry.spacing = parse_str_as_double(value, &error, warnings);
				break;
			}
			case AssStyleFormatAngle: {
				entry.angle = parse_str_as_double(value, &error, warnings);
				break;
			}
			case AssStyleFormatBorderStyle: {
				entry.border_style = parse_str_as_border_style(value, &error, warnings);
				break;
			}
			case AssStyleFormatOutline: {
				entry.outline = parse_str_as_double(value, &error, warnings);
				break;
			}
			case AssStyleFormatShadow: {
				entry.shadow = parse_str_as_double(value, &error, warnings);
				break;
			}
			case AssStyleFormatAlignment: {
				entry.alignment = parse_str_as_style_alignment(value, &error, warnings);
				break;
			}
			case AssStyleFormatMarginL: {
				entry.margin_l = parse_str_as_unsigned_number(value, &error, warnings);
				break;
			}
			case AssStyleFormatMarginR: {
				entry.margin_r = parse_str_as_unsigned_number(value, &error, warnings);
				break;
			}
			case AssStyleFormatMarginV: {
				entry.margin_v = parse_str_as_unsigned_number(value, &error, warnings);
				break;
			}
			case AssStyleFormatEncoding: {
				entry.encoding = parse_str_as_unsigned_number(value, &error, warnings);
				break;
			}
			default: {
				UNREACHABLE();
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

		char* result_buffer = NULL;
		FORMAT_STRING_DEFAULT(&result_buffer,
		                      "error, too few fields in the style line, the format line "
		                      "specified %lu, but we only have %lu",
		                      field_size, i);

		return DYNAMIC_ERROR(result_buffer);
	}

	stbds_arrput(styles_result->entries, entry);

	return NO_ERROR();
}

[[nodiscard]] static ErrorStruct parse_styles(AssStyles* ass_styles, StrView* data_view,
                                              ParseSettings settings, LineType line_type,
                                              Warnings* warnings) {

	AssStyles styles = { .entries = STBDS_ARRAY_EMPTY };

	STBDS_ARRAY(AssStyleFormat) style_format = STBDS_ARRAY_EMPTY;

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstStrView line = {};
		if(!str_view_get_substring_until_eol(data_view, &line, line_type, true)) {
			return STATIC_ERROR("implementation error");
		}

		// parse line
		{

			if(line.length == 0) {
				continue;
			}

			StrView line_view = get_str_view_from_const_str_view(line);

			ConstStrView field = {};
			if(!str_view_get_substring_by_char_delimiter(&line_view, &field, ':', false)) {
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

				ErrorStruct style_parse_error = parse_style_line_for_styles(
				    &line_view, style_format, &styles, settings, warnings);

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

					UnexpectedFieldWarning unexpected_field = { .field = field,
						                                        .section = "styles" };

					WarningEntry warning = { .type = WarningTypeUnexpectedField,
						                     .data = { .unexpected_field = unexpected_field } };

					stbds_arrput(warnings->entries, warning);

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

// global default values, so that they are valid all the time

static FinalStr
    g_default_ass_title = { // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
	    (int32_t[]){ '<', 'u', 'n', 't', 'i', 't', 'l', 'e', 'd', '>' }, 10
    };

static FinalStr
    g_default_ass_script_name = { // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
	    (int32_t[]){ '<', 'u', 'n', 'k', 'n', 'o', 'w', 'n', '>' }, 9
    };

#define FREE_AT_END() \
	do { \
		stbds_arrfree(field_names); \
	} while(false)

[[nodiscard]] static ErrorStruct parse_script_info(AssScriptInfo* script_info_result,
                                                   StrView* data_view, ParseSettings settings,
                                                   LineType line_type, Warnings* warnings) {

	AssScriptInfo script_info = { .script_type = ScriptTypeUnknown,
		                          .title = { .start = NULL, .length = 0 },
		                          .original_script = { .start = NULL, .length = 0 } };

	STBDS_ARRAY(FinalStr) field_names = STBDS_ARRAY_EMPTY;

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstStrView line = {};
		if(!str_view_get_substring_until_eol(data_view, &line, line_type, true)) {
			FREE_AT_END();
			return STATIC_ERROR("implementation error");
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
				FREE_AT_END();
				return STATIC_ERROR(
				    "end of line before ':' in line parsing in script info section");
			}

			// check for duplicate fields
			bool found_field = false;
			for(size_t i = 0; i < stbds_arrlenu(field_names); ++i) {
				FinalStr field_str = field_names[i];

				if(str_view_eq_str_view(field_str, field)) {
					found_field = true;

					char* field_name = get_normalized_string(field);

					if(!field_name) {
						FREE_AT_END();
						return STATIC_ERROR("allocation error");
					}

					char* result_buffer = NULL;
					FORMAT_STRING_DEFAULT(
					    &result_buffer, "duplicate field in script info section: '%s'", field_name);

					if(settings.strict_settings.script_info.allow_duplicate_fields) {

						DuplicateFieldWarning duplicate_field = { .field = field,
							                                      .section = "script info" };

						WarningEntry warning = { .type = WarningTypeDuplicateField,
							                     .data = { .duplicate_field = duplicate_field } };

						stbds_arrput(warnings->entries, warning);

						free(result_buffer);
						free(field_name);
						break;
					}

					free(field_name);
					FREE_AT_END();
					return DYNAMIC_ERROR(result_buffer);
				}
			}

			if(!found_field) {
				stbds_arrput(field_names, field);
			}

			if(!str_view_skip_optional_whitespace(&line_view)) {
				FREE_AT_END();
				return STATIC_ERROR("skip whitespace error");
			}

			ConstStrView value = get_const_str_view_from_str_view(line_view);

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
				script_info.play_res_y = parse_str_as_unsigned_number(value, &error, warnings);
				;
			} else if(str_view_eq_ascii(field, "PlayResX")) {
				script_info.play_res_x = parse_str_as_unsigned_number(value, &error, warnings);
				;
			} else if(str_view_eq_ascii(field, "PlayDepth")) {
				script_info.play_depth = value;
			} else if(str_view_eq_ascii(field, "Timer")) {
				script_info.timer = value;
			} else if(str_view_eq_ascii(field, "WrapStyle")) {
				script_info.wrap_style = parse_str_as_wrap_style(value, &error, warnings);
			} else if(str_view_eq_ascii(field, "ScaledBorderAndShadow")) {
				script_info.scaled_border_and_shadow = parse_str_as_str_bool(value, &error);
			} else if(str_view_eq_ascii(field, "Video Aspect Ratio")) {
				script_info.video_aspect_ratio =
				    parse_str_as_unsigned_number(value, &error, warnings);
			} else if(str_view_eq_ascii(field, "Video Zoom")) {
				script_info.video_zoom = parse_str_as_unsigned_number(value, &error, warnings);
			} else if(str_view_eq_ascii(field, "YCbCr Matrix")) {
				script_info.ycbcr_matrix = value;
			} else {

				char* field_name = get_normalized_string(field);

				if(!field_name) {
					FREE_AT_END();
					return STATIC_ERROR("allocation error");
				}

				char* result_buffer = NULL;
				FORMAT_STRING_DEFAULT(&result_buffer,
				                      "unexpected field in script info section: '%s'", field_name);

				if(settings.strict_settings.allow_additional_fields) {

					UnexpectedFieldWarning unexpected_field = { .field = field,
						                                        .section = "script info" };

					WarningEntry warning = { .type = WarningTypeUnexpectedField,
						                     .data = { .unexpected_field = unexpected_field } };

					stbds_arrput(warnings->entries, warning);

					free(result_buffer);
					free(field_name);
					continue;
				}

				free(field_name);
				FREE_AT_END();
				return DYNAMIC_ERROR(result_buffer);
			}

			if(error.message != NULL) {

				char* field_name = get_normalized_string(field);

				if(!field_name) {
					FREE_AT_END();
					return STATIC_ERROR("allocation error");
				}

				char* value_name = get_normalized_string(value);

				if(!value_name) {
					FREE_AT_END();
					return STATIC_ERROR("allocation error");
				}

				char* result_buffer = NULL;
				FORMAT_STRING_DEFAULT(&result_buffer,
				                      "While parsing field '%s' with value '%s': %s", field_name,
				                      value_name, error.message);

				free_error_struct(error);
				free(field_name);
				free(value_name);
				FREE_AT_END();
				return DYNAMIC_ERROR(result_buffer);
			}
		}

		// end of line parse

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

	stbds_arrfree(field_names);
#undef FREE_AT_END

	// check script info
	{

		if(script_info.script_type == ScriptTypeUnknown) {
			const char* error = "missing script type in script info section";

			if(settings.strict_settings.script_info.allow_missing_script_type) {
				WarningEntry warning = { .type = WarningTypeSimple,
					                     .data = { .simple = strdup(error) } };

				stbds_arrput(warnings->entries, warning); // NOLINT(clang-analyzer-unix.Malloc)
			} else {
				return STATIC_ERROR(error);
			}
		} else if(script_info.script_type != ScriptTypeV4Plus) {

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer, "only scrypt type v4+ is supported but got: %s",
			                      get_script_type_name(script_info.script_type));

			return DYNAMIC_ERROR(result_buffer);
		}

		if(script_info.title.start == NULL) {

			script_info.title = g_default_ass_title;
		}

		if(script_info.original_script.start == NULL) {
			script_info.original_script = g_default_ass_script_name;
		}
	}

	*script_info_result = script_info;
	return NO_ERROR();
	// end of script info
}

[[nodiscard]] static ErrorStruct skip_section(StrView* data_view, LineType line_type) {

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstStrView line = {};
		if(!str_view_get_substring_until_eol(data_view, &line, line_type, true)) {
			return STATIC_ERROR("implementation error");
		}

		// skip the line
		UNUSED(line);

		if(str_view_is_eof(*data_view)) {
			break;
		}
	}

	return NO_ERROR();
}

[[nodiscard]] static ErrorStruct extra_section(ConstStrView section_name, StrView* data_view,
                                               ExtraSections* extra_sections, LineType line_type) {

	char* section_name_str = get_normalized_string(section_name);

	if(section_name_str == NULL) {
		return STATIC_ERROR("alloc error");
	}

	ExtraSectionHashMapEntry extra_section = { .key = section_name_str,
		                                       .value = { .fields = STBDS_HASH_MAP_EMPTY } };

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstStrView line = {};
		if(!str_view_get_substring_until_eol(data_view, &line, line_type, true)) {
			return STATIC_ERROR("implementation error");
		}

		{

			if(line.length == 0) {
				continue;
			}

			SectionFieldEntry field_entry = {};

			StrView line_view = get_str_view_from_const_str_view(line);

			ConstStrView field = {};
			if(!str_view_get_substring_by_char_delimiter(&line_view, &field, ':', false)) {
				return STATIC_ERROR("end of line before ':' in line parsing in extra section");
			}

			if(!str_view_skip_optional_whitespace(&line_view)) {
				return STATIC_ERROR("skip whitespace error");
			}

			ConstStrView key = {};

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
parse_format_line_for_events(StrView* line_view, STBDS_ARRAY(AssEventFormat) * format_result) {

	while(!(str_view_is_eof(*line_view))) {

		if(!str_view_skip_optional_whitespace(line_view)) {
			return STATIC_ERROR("skip whitespace error");
		}

		ConstStrView key = {};
		if(!str_view_get_substring_by_char_delimiter(line_view, &key, ',', true)) {
			return STATIC_ERROR("implementation error");
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

[[nodiscard]] static ErrorStruct parse_event_line_for_events(EventType type, StrView* line_view,
                                                             const STBDS_ARRAY(AssEventFormat)
                                                                 const format_spec,
                                                             AssEvents* events_result,
                                                             Warnings* warnings) {

	size_t field_size = stbds_arrlenu(format_spec);

	AssEventEntry entry = { .type = type };

	if(!str_view_skip_optional_whitespace(line_view)) {
		return STATIC_ERROR("skip whitespace error");
	}

	size_t i = 0;

	for(bool are_at_end = false; !are_at_end; ++i) {

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

		ConstStrView value = {};

		if(format == AssEventFormatText) {
			if(i != field_size - 1) {
				return STATIC_ERROR(
				    "'Text' field of event lines may only occur at the last position!");
			}

			if(!str_view_get_substring_until_eof(line_view, &value)) {
				return STATIC_ERROR("eof before comma in events section event line");
			}
			are_at_end = true;

			// TODO(Totto): check and parse text value, for invalid escape sequences, and invald
			// values inside {}, like eg {bogus}, or {\j} etc, or not closed {} blocks

		} else {
			if(!str_view_get_substring_by_char_delimiter(line_view, &value, ',', true)) {
				return STATIC_ERROR("implementation error");

				are_at_end = true;
			}
		}

		ErrorStruct error = NO_ERROR();

		switch(format) {
			case AssEventFormatLayer: {
				entry.layer = parse_str_as_unsigned_number(value, &error, warnings);
				break;
			}
			case AssEventFormatStart: {
				entry.start = parse_str_as_time(value, &error, warnings);
				break;
			}
			case AssEventFormatEnd: {
				entry.end = parse_str_as_time(value, &error, warnings);
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
				entry.margin_l = parse_str_as_margin_value(value, &error, warnings);
				break;
			}
			case AssEventFormatMarginR: {
				entry.margin_r = parse_str_as_margin_value(value, &error, warnings);
				break;
			}
			case AssEventFormatMarginV: {
				entry.margin_v = parse_str_as_margin_value(value, &error, warnings);
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
				UNREACHABLE();
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
		char* result_buffer = NULL;
		FORMAT_STRING_DEFAULT(&result_buffer,
		                      "error, too few fields in the event line, the format line "
		                      "specified %lu, but we only have %lu",
		                      field_size, i);

		return DYNAMIC_ERROR(result_buffer);
	}

	stbds_arrput(events_result->entries, entry);

	return NO_ERROR();
}

[[nodiscard]] static ErrorStruct parse_events(AssEvents* ass_events, StrView* data_view,
                                              ParseSettings settings, LineType line_type,
                                              Warnings* warnings) {

	AssEvents events = { .entries = STBDS_ARRAY_EMPTY };

	STBDS_ARRAY(AssEventFormat) event_format = STBDS_ARRAY_EMPTY;

#define FREE_AT_END() \
	do { \
		stbds_arrfree(events.entries); \
		stbds_arrfree(event_format); \
	} while(false)

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstStrView line = {};
		if(!str_view_get_substring_until_eol(data_view, &line, line_type, true)) {
			FREE_AT_END();
			return STATIC_ERROR("implementation error");
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
				return STATIC_ERROR("end of line before ':' in line parsing in events section");
			}

			if(str_view_eq_ascii(field, "Format")) {

				if(stbds_arrlenu(event_format) != 0) {
					FREE_AT_END();
					return STATIC_ERROR(
					    "multiple format fields detected in the events section, this is not "
					    "allowed");
				}

				ErrorStruct format_line_error =
				    parse_format_line_for_events(&line_view, &event_format);

				if(format_line_error.message != NULL) {
					FREE_AT_END();
					return format_line_error;
				}

			} else if(str_view_eq_ascii(field, "Dialogue")) {

				if(stbds_arrlenu(event_format) == 0) {
					FREE_AT_END();
					return STATIC_ERROR(
					    "no format line occurred before the style line in the events section, "
					    "this is an error");
				}

				ErrorStruct event_parse_error = parse_event_line_for_events(
				    EventTypeDialogue, &line_view, event_format, &events, warnings);

				if(event_parse_error.message != NULL) {
					FREE_AT_END();
					return event_parse_error;
				}

			} else if(str_view_eq_ascii(field, "Comment")) {

				if(stbds_arrlenu(event_format) == 0) {
					FREE_AT_END();
					return STATIC_ERROR(
					    "no format line occurred before the style line in the events section, "
					    "this is an error");
				}

				ErrorStruct event_parse_error = parse_event_line_for_events(
				    EventTypeComment, &line_view, event_format, &events, warnings);

				if(event_parse_error.message != NULL) {
					FREE_AT_END();
					return event_parse_error;
				}

			} else if(str_view_eq_ascii(field, "Picture")) {

				if(stbds_arrlenu(event_format) == 0) {
					FREE_AT_END();
					return STATIC_ERROR(
					    "no format line occurred before the style line in the events section, "
					    "this is an error");
				}

				ErrorStruct event_parse_error = parse_event_line_for_events(
				    EventTypePicture, &line_view, event_format, &events, warnings);

				if(event_parse_error.message != NULL) {
					FREE_AT_END();
					return event_parse_error;
				}

			} else if(str_view_eq_ascii(field, "Sound")) {

				if(stbds_arrlenu(event_format) == 0) {
					FREE_AT_END();
					return STATIC_ERROR(
					    "no format line occurred before the style line in the events section, "
					    "this is an error");
				}

				ErrorStruct event_parse_error = parse_event_line_for_events(
				    EventTypeSound, &line_view, event_format, &events, warnings);

				if(event_parse_error.message != NULL) {
					FREE_AT_END();
					return event_parse_error;
				}

			} else if(str_view_eq_ascii(field, "Movie")) {

				if(stbds_arrlenu(event_format) == 0) {
					FREE_AT_END();
					return STATIC_ERROR(
					    "no format line occurred before the style line in the events section, "
					    "this is an error");
				}

				ErrorStruct event_parse_error = parse_event_line_for_events(
				    EventTypeMovie, &line_view, event_format, &events, warnings);

				if(event_parse_error.message != NULL) {
					FREE_AT_END();
					return event_parse_error;
				}

			} else if(str_view_eq_ascii(field, "Command")) {

				if(stbds_arrlenu(event_format) == 0) {
					FREE_AT_END();
					return STATIC_ERROR(
					    "no format line occurred before the style line in the events section, "
					    "this is an error");
				}

				ErrorStruct event_parse_error = parse_event_line_for_events(
				    EventTypeCommand, &line_view, event_format, &events, warnings);

				if(event_parse_error.message != NULL) {
					FREE_AT_END();
					return event_parse_error;
				}

			} else {

				char* field_name = get_normalized_string(field);

				if(!field_name) {
					FREE_AT_END();
					return STATIC_ERROR("allocation error");
				}

				char* result_buffer = NULL;
				FORMAT_STRING_DEFAULT(&result_buffer, "unexpected field in events section: '%s'",
				                      field_name);

				if(settings.strict_settings.allow_additional_fields) {

					UnexpectedFieldWarning unexpected_field = { .field = field,
						                                        .section = "events" };

					WarningEntry warning = { .type = WarningTypeUnexpectedField,
						                     .data = { .unexpected_field = unexpected_field } };

					stbds_arrput(warnings->entries, warning);

					free(result_buffer);
					free(field_name);
					continue;
				}

				free(field_name);
				FREE_AT_END();
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

#undef FREE_AT_END

[[nodiscard]] static ErrorStruct get_section_by_name(ConstStrView section_name,
                                                     AssResult* ass_result, StrView* data_view,
                                                     ParseSettings settings, LineType line_type,
                                                     Warnings* warnings) {

	if(str_view_eq_ascii(section_name, "V4+ Styles")) {
		return parse_styles(&(ass_result->styles), data_view, settings, line_type, warnings);
	}

	if(str_view_eq_ascii(section_name, "V4 Styles")) {
		return STATIC_ERROR("v4 styles are not supported");
	}

	if(str_view_eq_ascii(section_name, "Events")) {
		return parse_events(&(ass_result->events), data_view, settings, line_type, warnings);
	}

	if(str_view_eq_ascii(section_name, "Fonts")) {
		return skip_section(data_view, line_type);
	}

	if(str_view_eq_ascii(section_name, "Graphics")) {
		return skip_section(data_view, line_type);
	}

	return extra_section(section_name, data_view, &(ass_result->extra_sections), line_type);

	return NO_ERROR();
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
	stbds_arrfree(data.styles.entries);
	stbds_arrfree(data.events.entries);

	free_extra_sections(data.extra_sections);
}

#define FREE_AT_END() \
	do { \
	} while(false)

#define RETURN_ERROR(err) \
	do { \
		FREE_AT_END(); \
		result->is_error = true; \
		result->data.error = err; \
		return result; \
	} while(false)

[[nodiscard]] AssParseResult* parse_ass(AssSource source, ParseSettings settings) {

	AssParseResult* result = (AssParseResult*)malloc(sizeof(AssParseResult));

	if(!result) {
		return NULL;
	}

	result->warnings = (Warnings){ .entries = STBDS_ARRAY_EMPTY };
	result->allocated_codepoints = (Codepoints){ .data = NULL, .size = 0 };

	SizedPtr data = get_data_from_source(source);

	if(is_ptr_error(data)) {
		RETURN_ERROR(STATIC_ERROR(ptr_get_error(data)));
	}

	FileType file_type = determine_file_type(data);

	size_t bom_size = 0;
	CodepointsResult codepoints_result = { .has_error = true,
		                                   .data = { .error = "implementation error" } };

	switch(file_type) {
		case FileTypeUnknown: {
			const char* error = "unrecognized file type, no BOM present";

			if(!settings.strict_settings.allow_unrecognized_file_encoding) {
				RETURN_ERROR(STATIC_ERROR(error));
			}

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer, "%s, assuming UTF-8 (ascii also works with that)",
			                      error);

			WarningEntry warning = { .type = WarningTypeSimple,
				                     .data = { .simple = result_buffer } };

			stbds_arrput(result->warnings.entries, warning); // NOLINT(clang-analyzer-unix.Malloc)
			bom_size = 0;
			codepoints_result = get_codepoints_from_utf8(data);
			break;
		}
		case FileTypeUtf8: {
			bom_size = 1;
			codepoints_result = get_codepoints_from_utf8(data);
			break;
		}
		case FileTypeUtf16BE: {
			bom_size = 1;
			codepoints_result = get_codepoints_from_utf16(data, true);
			break;
		}
		case FileTypeUtf16LE: {
			bom_size = 1;
			codepoints_result = get_codepoints_from_utf16(data, false);
			break;
		}
		case FileTypeUtf32BE: {
			bom_size = 1;
			codepoints_result = get_codepoints_from_utf32(data, true);
			break;
		}
		case FileTypeUtf32LE: {
			bom_size = 1;
			codepoints_result = get_codepoints_from_utf32(data, false);
			break;
		}

		default: {
			free_sized_ptr(data);

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer,
			                      "only UTF-8 encoded files supported atm, but got: %s",
			                      get_file_type_name(file_type));

			RETURN_ERROR(DYNAMIC_ERROR(result_buffer));
		}
	}

	free_sized_ptr(data);

	if(codepoints_result.has_error) {
		RETURN_ERROR(STATIC_ERROR(codepoints_result.data.error));
	}

	Codepoints final_data = codepoints_result.data.result;

	if(final_data.data == NULL && final_data.size == 0) {
		RETURN_ERROR(STATIC_ERROR("file conversion resulted in empty UTF-8 string"));
	}

	result->allocated_codepoints = final_data;

	StrView data_view = str_view_from_data(final_data);

	if(bom_size > 0) {
		// NOTE: the bom byte is always just one codepoint
		if(!str_view_advance(&data_view, bom_size)) {
			RETURN_ERROR(STATIC_ERROR("couldn't skip bom bytes"));
		}
	}

	// get line type
	char* line_type_error = NULL;

	ConstStrView line_type_view = get_const_str_view_from_str_view(data_view);

	LineType line_type = get_line_type(line_type_view, &line_type_error);

	if(line_type_error != NULL) {
		RETURN_ERROR(DYNAMIC_ERROR(line_type_error));
	}

	// parse script info

	if(!str_view_expect_ascii(&data_view, "[Script Info]")) {
		RETURN_ERROR(STATIC_ERROR("first line must be the script info section"));
	}

	if(!str_view_expect_newline(&data_view, line_type)) {
		RETURN_ERROR(STATIC_ERROR("expected newline"));
	}

	AssResult ass_result = { .extra_sections = (ExtraSections){ .entries = STBDS_HASH_MAP_EMPTY },
		                     .file_props = { .file_type = file_type, .line_type = line_type } };

#undef FREE_AT_END
#define FREE_AT_END() \
	do { \
		free_ass_result(ass_result); \
	} while(false)

	ErrorStruct script_info_parse_result = parse_script_info(
	    &(ass_result.script_info), &data_view, settings, line_type, &(result->warnings));

	if(script_info_parse_result.message != NULL) {
		RETURN_ERROR(script_info_parse_result);
	}

	while(true) {

		// classify section, and then parsing that section

		if(!str_view_expect_ascii(&data_view, "[")) {
			RETURN_ERROR(STATIC_ERROR("implementation error"));
		}

		ConstStrView section_name = {};
		if(!str_view_get_substring_by_char_delimiter(&data_view, &section_name, ']', false)) {
			RETURN_ERROR(STATIC_ERROR("script section not terminated by ']'"));
		}

		if(!str_view_expect_newline(&data_view, line_type)) {
			RETURN_ERROR(STATIC_ERROR("no newline after section name"));
		}

		ErrorStruct section_parse_result = get_section_by_name(
		    section_name, &ass_result, &data_view, settings, line_type, &(result->warnings));

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

#undef FREE_AT_END

[[nodiscard]] Warnings get_warnings_from_result(AssParseResult* result) {
	return result->warnings;
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

[[nodiscard]] AssResult parse_result_get_value(AssParseResult* result) {
	return result->data.ok;
}

void free_parse_result(AssParseResult* result) {
	if(!result->is_error) {
		free_ass_result(result->data.ok);
	} else {
		free_error_struct(result->data.error);
	}

	free_warnings(result->warnings);
	free_codepoints(result->allocated_codepoints);

	free(result);
}
