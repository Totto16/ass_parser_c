
#define ASS_PARSER_C_INTERNAL_USAGE

#include "./validate.h"
#include "../helper/macros.h"

#undef ASS_PARSER_C_INTERNAL_USAGE

#include <stb/ds.h>
#include <stdio.h>
#include <strings.h>

#include <fontconfig/fontconfig.h>

typedef struct {
	FcPattern* pattern;
	FcObjectSet* object_set;
	FcFontSet* font_list;
} FontResultOk;

typedef struct {
	bool error;
	union {
		FontResultOk ok;
	} data;
} FontResultObject;

static void free_font_result_ok(FontResultOk result) {
	FcPatternDestroy(result.pattern);
	FcObjectSetDestroy(result.object_set);
	FcFontSetDestroy(result.font_list);
}

static void free_font_result(FontResultObject result) {
	if(!result.error) {
		free_font_result_ok(result.data.ok);
	}
}

static FontResultObject fontconfig_find_font_by_family_name(const char* font_name) {
	// Build a pattern to search for
	FcPattern* pattern = FcPatternCreate();

	if(pattern == NULL) {
		return (FontResultObject){ .error = true };
	}

#define FREE_AT_END() \
	do { \
		FcPatternDestroy(pattern); \
	} while(false)

	if(!FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*)font_name)) {
		FREE_AT_END();

		return (FontResultObject){ .error = true };
	}

	FcObjectSet* object_set = FcObjectSetBuild(FC_FAMILY, FC_FILE, FC_STYLE, FC_SLANT, NULL);

	if(object_set == NULL) {
		FREE_AT_END();

		return (FontResultObject){ .error = true };
	}

#undef FREE_AT_END
#define FREE_AT_END() \
	do { \
		FcPatternDestroy(pattern); \
		FcObjectSetDestroy(object_set); \
	} while(false)

	FcConfig* config = NULL; // use the default one

	FcFontSet* font_list = FcFontList(config, pattern, object_set);

	if(!font_list) {
		FREE_AT_END();

		return (FontResultObject){ .error = true };
	}

	return (FontResultObject){ .error = false,
		                       .data = { .ok = (FontResultOk){ .pattern = pattern,
		                                                       .object_set = object_set,
		                                                       .font_list = font_list } } };
}

#undef FREE_AT_END

typedef enum : uint8_t {
	FontStyleTypeNormal,
	FontStyleTypeBold,
	FontStyleTypeItalic,
	FontStyleTypeBoldItalic,
} FontStyleType;

[[nodiscard]] const char* get_search_type_name(FontStyleType type) {
	switch(type) {
		case FontStyleTypeNormal: return "Normal";
		case FontStyleTypeBold: return "Bold";
		case FontStyleTypeItalic: return "Italic";
		case FontStyleTypeBoldItalic: return "Bold Italic";
		default: return "<unknown>";
	}
}

[[nodiscard]] FontStyleType get_style_type_for_font(AssStyleEntry entry) {

	// TODO: support  strike_out and  underline

	if(entry.bold && entry.italic) {
		return FontStyleTypeBoldItalic;
	}

	if(entry.bold) {
		return FontStyleTypeBold;
	}

	if(entry.italic) {
		return FontStyleTypeItalic;
	}

	return FontStyleTypeNormal;
}

typedef struct {
	size_t size;
	const char** values;
} StaticStringArray;

[[nodiscard]]
bool is_valid_name_for_type(FontStyleType type, FcChar8* name) {

	StaticStringArray array = { .size = 0, .values = NULL };

	switch(type) {
		case FontStyleTypeNormal: {
			const char* strings[] = { "Regular", "Normal", "Standard" };

			array.values = strings;
			array.size = sizeof(strings) / sizeof(*strings);
			break;
		}
		case FontStyleTypeBold: {
			const char* strings[] = { "Bold" };

			array.values = strings;
			array.size = sizeof(strings) / sizeof(*strings);
			break;
		}
		case FontStyleTypeItalic: {
			const char* strings[] = { "Italic" };

			array.values = strings;
			array.size = sizeof(strings) / sizeof(*strings);
			break;
		}
		case FontStyleTypeBoldItalic: {
			const char* strings[] = { "Bold Italic" };

			array.values = strings;
			array.size = sizeof(strings) / sizeof(*strings);
			break;
		}

		default: return false;
	}

	for(size_t i = 0; i < array.size; ++i) {
		const char* value = array.values[i];

		if(strcasecmp((char*)name, value) == 0) {
			return true;
		}
	}

	return false;
}

typedef enum : uint8_t {
	FontSearchResultTypeFound,
	FontSearchResultTypeNotFound,
	FontSearchResultTypeError
} FontSearchResultType;

typedef struct {
	FontSearchResultType type;
	union {
		MessageStruct error;
	} data;
} FontSearchResult;

[[nodiscard]] FontSearchResult find_type_for_fonts(const char* font_name, FcFontSet* font_list,
                                                   FontStyleType font_type) {
	for(int i = 0; i < font_list->nfont; ++i) {
		FcPattern* font = font_list->fonts[i];

		FcChar8* family = NULL;

		FcResult res = FcPatternGetString(font, FC_FAMILY, 0, &family);

		if(res != FcResultMatch) {

			return (FontSearchResult){ .type = FontSearchResultTypeError,
				                       .data = { .error = STATIC_MESSAGE_STRUCT(
				                                     "couldn't get font family") } };
		}

		if(strcasecmp((char*)family, font_name) != 0) {
			// not an exact match
			continue;
		}

		for(int i = 0;; ++i) {
			FcChar8* result = NULL;

			FcResult res = FcPatternGetString(font, FC_STYLE, i, &result);

			switch(res) {
				case FcResultMatch: {
					if(result == NULL) {

						return (FontSearchResult){
							.type = FontSearchResultTypeError,
							.data = { .error = STATIC_MESSAGE_STRUCT(
							              "result matched but string was null") }
						};
					}

					if(is_valid_name_for_type(font_type, result)) {
						return (FontSearchResult){ .type = FontSearchResultTypeFound };
					}
					break;
				}
				case FcResultNoId: {
					goto break_for_inner;
				}
				default: {
					char* result_buffer = NULL;
					FORMAT_STRING_DEFAULT(&result_buffer, "result for getting the style was: '%d'",
					                      res);

					return (FontSearchResult){ .type = FontSearchResultTypeError,
						                       .data = { .error = DYNAMIC_MESSAGE_STRUCT(
						                                     result_buffer) } };
				}
			}
		}
	break_for_inner:
	}

	return (FontSearchResult){ .type = FontSearchResultTypeNotFound };
}

static void validate_font(AssStyleEntry entry, bool allow_validation_errors,
                          Diagnostics* diagnostics) {

	char* font_name = get_normalized_string(entry.fontname);

	if(!font_name) {
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
		                    NO_POS());
		return;
	}

	FontResultObject result = fontconfig_find_font_by_family_name(font_name);

#define FREE_AT_END() \
	do { \
		free(font_name); \
		free_font_result(result); \
	} while(false)

	if(result.error) {
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("fontconfig error"),
		                    NO_POS());
		return;
	}

	FontResultOk ok_res = result.data.ok;

	if(ok_res.font_list->nfont == 0) {
		char* result_buffer = NULL;
		FORMAT_STRING_DEFAULT(&result_buffer, "no font for '%s' found", font_name);

		const DiagnosticSeverity severity_type =
		    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

		INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
		                         NO_POS(), severity_type);

		FREE_AT_END();
		return;
	}

	FontStyleType search_type = get_style_type_for_font(entry);

	FontSearchResult found_result = find_type_for_fonts(font_name, ok_res.font_list, search_type);

	if(found_result.type == FontSearchResultTypeError) {
		char* result_buffer = NULL;
		FORMAT_STRING_DEFAULT(&result_buffer,
		                      "an error occurred while trying to find font '%s': %s", font_name,
		                      found_result.data.error.message);

		const DiagnosticSeverity severity_type =
		    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

		INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
		                         NO_POS(), severity_type);

		free_message_struct(found_result.data.error);

		FREE_AT_END();
		return;
	}

	if(found_result.type == FontSearchResultTypeNotFound) {
		char* result_buffer = NULL;
		FORMAT_STRING_DEFAULT(&result_buffer, "variant '%s' for font '%s' not found",
		                      get_search_type_name(search_type), font_name);

		const DiagnosticSeverity severity_type =
		    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

		INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
		                         NO_POS(), severity_type);

		FREE_AT_END();
		return;
	}

	FREE_AT_END();
}

#undef FREE_AT_END

static void validate_fonts(AssResult ass_result, bool allow_validation_errors,
                           Diagnostics* diagnostics) {

	if(!FcInit()) {

		const DiagnosticSeverity severity_type =
		    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

		INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries,
		                         STATIC_MESSAGE_STRUCT("failed to load fontconfig"), NO_POS(),
		                         severity_type);
		return;
	}

	for(size_t i = 0; i < stbds_arrlenu(ass_result.styles.entries); ++i) {
		AssStyleEntry entry = ass_result.styles.entries[i];

		validate_font(entry, allow_validation_errors, diagnostics);
	}

	FcFini();
}

static void validate_style_angles(double angle, bool allow_validation_errors,
                                  Diagnostics* diagnostics) {

	if(angle < 0.0 || angle > 360.0) {
		const DiagnosticSeverity severity_type =
		    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

		char* result_buffer = NULL;
		FORMAT_STRING_DEFAULT(&result_buffer, "value for angle is out of range: %f", angle);

		INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
		                         NO_POS(), severity_type);
	}
}

static void validate_styles(AssResult ass_result, bool allow_validation_errors,
                            Diagnostics* diagnostics) {

	for(size_t i = 0; i < stbds_arrlenu(ass_result.styles.entries); ++i) {
		AssStyleEntry entry = ass_result.styles.entries[i];

		// TODO: validate windows encoding
		// validate_style_encoding(entry, diagnostics);

		validate_style_angles(entry.angle, allow_validation_errors, diagnostics);
	}

	// TODO: check if the style in a effect line is present
}

static void validate_text(AssResult ass_result, bool allow_validation_errors,
                          Diagnostics* diagnostics) {

	// TODO(Totto): check and parse text value, for invalid escape sequences, and invald
	// values inside {}, like eg {bogus}, or {\j} etc, or not closed {} blocks

	UNUSED(ass_result);
	UNUSED(allow_validation_errors);
	UNUSED(diagnostics);
}

void validate_ass_result(AssResult ass_result, ParseSettings settings, Diagnostics* diagnostics) {

	if(settings.validate_settings.validate_fonts) {
		validate_fonts(ass_result, settings.strict_settings.allow_validation_errors, diagnostics);
	}

	if(settings.validate_settings.validate_styles) {
		validate_styles(ass_result, settings.strict_settings.allow_validation_errors, diagnostics);
	}

	if(settings.validate_settings.validate_text) {
		validate_text(ass_result, settings.strict_settings.allow_validation_errors, diagnostics);
	}
}
