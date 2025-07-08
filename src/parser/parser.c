

#include "./parser.h"
#include "../helper/io.h"
#include "../helper/utf8_helper.h"
#include "../helper/utf8_string_view.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
	int todo;
} AssScriptInfo;
typedef struct {
	int todo;
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

[[nodiscard]] static const char* parse_styles(AssStyles* ass_styles, Utf8StrView* data_view) {

	AssStyles styles = {};

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		// TODO: parse style lines
		ConstUtf8StrView line = {};
		if(!str_view_get_substring_by_delimiter(data_view, &line, newline_delimiter, true, NULL)) {
			return "eof before newline";
		}

		(void)line;
	}

	*ass_styles = styles;
	return NULL;
	// end of script info
}

[[nodiscard]] static const char* skip_section(Utf8StrView* data_view) {

	while(!str_view_starts_with_ascii_or_eof(*data_view, "[")) {

		ConstUtf8StrView line = {};
		if(!str_view_get_substring_by_delimiter(data_view, &line, newline_delimiter, true, NULL)) {
			return "eof before newline";
		}

		(void)line;
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

	if(str_view_eq_ascii(section_name, "Events")) {
		return skip_section(data_view);
	}

	if(str_view_eq_ascii(section_name, "Fonts")) {
		return skip_section(data_view);
	}

	if(str_view_eq_ascii(section_name, "Graphics")) {
		return skip_section(data_view);
	}

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
	AssScriptInfo script_info = {};

	if(!str_view_expect_ascii(&data_view, "[Script Info]")) {
		RETURN_ERROR("first line must be the script info section");
	}

	if(!str_view_expect_newline(&data_view)) {
		RETURN_ERROR("expected newline");
	}

	while(!str_view_starts_with_ascii(data_view, "[")) {

		// TODO: parse script info lines
		ConstUtf8StrView line = {};
		if(!str_view_get_substring_by_delimiter(&data_view, &line, newline_delimiter, true, NULL)) {
			RETURN_ERROR("eof before newline");
		}

		(void)line;
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
