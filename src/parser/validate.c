
#define _GNU_SOURCE // NOLINT(readability-identifier-naming,bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)
#include <string.h>
#undef _GNU_SOURCE

#define ASS_PARSER_C_INTERNAL_USAGE

#include "../helper/macros.h"
#include "./validate.h"

#undef ASS_PARSER_C_INTERNAL_USAGE

#include <stb/ds.h>
#include <stdio.h>

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

static FontResultObject fontconfig_find_fonts_by_family_name(const char* font_name) {
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
			static const char* s_strings[] = { "Regular", "Normal", "Standard" };

			array.values = s_strings;
			array.size = sizeof(s_strings) / sizeof(*s_strings);
			break;
		}
		case FontStyleTypeBold: {
			static const char* s_strings[] = { "Bold" };

			array.values = s_strings;
			array.size = sizeof(s_strings) / sizeof(*s_strings);
			break;
		}
		case FontStyleTypeItalic: {
			static const char* s_strings[] = { "Italic" };

			array.values = s_strings;
			array.size = sizeof(s_strings) / sizeof(*s_strings);
			break;
		}
		case FontStyleTypeBoldItalic: {
			static const char* s_strings[] = { "Bold Italic" };

			array.values = s_strings;
			array.size = sizeof(s_strings) / sizeof(*s_strings);
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
	MatchResolutionExact,
	MatchResolutionIncludes,
	MatchResolutionLikeIs
} MatchResolution;

[[nodiscard]] bool matches_font(const char* family, const char* font_name,
                                MatchResolution resolution) {

	switch(resolution) {
		case MatchResolutionExact: {
			return strcasecmp(family, font_name) == 0;
		}
		case MatchResolutionIncludes: {
			const char* pos1 = strcasestr(family, font_name);

			if(pos1 != NULL) {
				return true;
			}

			const char* pos2 = strcasestr(font_name, family);

			return pos2 != NULL;
		}
		case MatchResolutionLikeIs:
		default: {
			return true;
		}
	}
}

typedef struct {
	MatchResolution font_match_res;
	bool check_only_used_fonts;
	bool enabled;
} DetailedFontValidateSettings;

[[nodiscard]] static DetailedFontValidateSettings
get_detailed_font_settings_from_presset(FontPreset preset) {

	switch(preset) {
		case FontPresetDisabled: {
			return (DetailedFontValidateSettings){ .enabled = false,
				                                   .check_only_used_fonts = true,
				                                   .font_match_res = MatchResolutionExact };
		}
		case FontPresetStrictAll: {
			return (DetailedFontValidateSettings){ .enabled = true,
				                                   .check_only_used_fonts = false,
				                                   .font_match_res = MatchResolutionExact };
		}
		case FontPresetStrict: {
			return (DetailedFontValidateSettings){ .enabled = true,
				                                   .check_only_used_fonts = true,
				                                   .font_match_res = MatchResolutionExact };
		}
		case FontPresetModerate: {
			return (DetailedFontValidateSettings){ .enabled = true,
				                                   .check_only_used_fonts = true,
				                                   .font_match_res = MatchResolutionIncludes };
		}
		case FontPresetLenient: {
			return (DetailedFontValidateSettings){ .enabled = true,
				                                   .check_only_used_fonts = true,
				                                   .font_match_res = MatchResolutionLikeIs };
		}
		default: UNREACHABLE();
	}
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
                                                   FontStyleType font_type,
                                                   DetailedFontValidateSettings settings) {
	for(int i = 0; i < font_list->nfont; ++i) {
		FcPattern* font = font_list->fonts[i];

		FcChar8* family = NULL;

		FcResult res = FcPatternGetString(font, FC_FAMILY, 0, &family);

		if(res != FcResultMatch) {

			return (FontSearchResult){ .type = FontSearchResultTypeError,
				                       .data = { .error = STATIC_MESSAGE_STRUCT(
				                                     "couldn't get font family") } };
		}

		if(!matches_font((char*)family, font_name, settings.font_match_res)) {
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

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		return (FontSearchResult){ .type = FontSearchResultTypeError, \
			                       .data = { .error = STATIC_MESSAGE_STRUCT(message) } }; \
	} while(false)

					char* result_buffer = NULL;
					FORMAT_STRING_PROPAGATE_ERROR(&result_buffer,
					                              "result for getting the style was: '%d'", res);

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

#undef PROPAGATE_ERROR_IMPL

static void validate_font(const char* font_name, FontStyleType search_type, FilePos file_pos,
                          bool allow_validation_errors, Diagnostics* diagnostics,
                          DetailedFontValidateSettings settings) {

	FontResultObject result = fontconfig_find_fonts_by_family_name(font_name);

#define FREE_AT_END() \
	do { \
		free_font_result(result); \
	} while(false)

	if(result.error) {
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("fontconfig error"),
		                    EMPTY_POS());
		return;
	}

	FontResultOk ok_res = result.data.ok;

	if(ok_res.font_list->nfont == 0) {

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), file_pos); \
		FREE_AT_END(); \
		return; \
	} while(false)

		char* result_buffer = NULL;
		FORMAT_STRING_PROPAGATE_ERROR(&result_buffer, "no font for '%s' found", font_name);

		const DiagnosticSeverity severity_type =
		    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

		INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
		                         file_pos, severity_type);

		FREE_AT_END();
		return;
	}

	FontSearchResult found_result =
	    find_type_for_fonts(font_name, ok_res.font_list, search_type, settings);

	if(found_result.type == FontSearchResultTypeError) {
		char* result_buffer = NULL;
		FORMAT_STRING_PROPAGATE_ERROR(&result_buffer,
		                              "an error occurred while trying to find font '%s': %s",
		                              font_name, found_result.data.error.message);

		const DiagnosticSeverity severity_type =
		    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

		INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
		                         file_pos, severity_type);

		free_message_struct(found_result.data.error);

		FREE_AT_END();
		return;
	}

	if(found_result.type == FontSearchResultTypeNotFound) {
		char* result_buffer = NULL;
		FORMAT_STRING_PROPAGATE_ERROR(&result_buffer, "variant '%s' for font '%s' not found",
		                              get_search_type_name(search_type), font_name);

		const DiagnosticSeverity severity_type =
		    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

		INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
		                         file_pos, severity_type);

		FREE_AT_END();
		return;
	}

	FREE_AT_END();
}

#undef PROPAGATE_ERROR_IMPL
#undef FREE_AT_END

STBDS_HASH_MAP_TYPE(char*, FinalStr, StyleToFontHMEntry);

typedef STBDS_HASH_MAP(StyleToFontHMEntry) StyleToFontHM;

static void free_style_to_font_hm(StyleToFontHM* style_to_font_hm) {

	size_t hm_length = stbds_shlenu(*style_to_font_hm);

	for(size_t i = 0; i < hm_length; ++i) {
		StyleToFontHMEntry entry = (*style_to_font_hm)[i];
		free(entry.key);
	}

	stbds_shfree(*style_to_font_hm);
}

#if defined(__clang__)
typedef struct {
} MonoState;

#define MONOSTATE MonoState
#define MONOSTATE_VALUE ((MonoState){})

#else

#define MONOSTATE bool
#define MONOSTATE_VALUE true

#endif

STBDS_HASH_MAP_TYPE(char*, MONOSTATE, UsedFontHMEntry);

typedef STBDS_HASH_MAP(UsedFontHMEntry) UsedFontsHM;

static void free_used_fonts_hm(UsedFontsHM* used_fonts_hm) {

	size_t hm_length = stbds_shlenu(*used_fonts_hm);

	for(size_t i = 0; i < hm_length; ++i) {
		UsedFontHMEntry entry = (*used_fonts_hm)[i];
		free(entry.key);
	}

	stbds_shfree(*used_fonts_hm);
}

[[nodiscard]] static UsedFontsHM get_used_fonts(AssResult ass_result, bool allow_validation_errors,
                                                Diagnostics* diagnostics) {

	StyleToFontHM hm_style_to_font = STBDS_HASH_MAP_EMPTY;

	for(size_t i = 0; i < stbds_arrlenu(ass_result.styles.entries); ++i) {
		AssStyleEntry entry = ass_result.styles.entries[i];

		char* style_name = get_normalized_string(entry.name);

		if(!style_name) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
			                    EMPTY_POS());
			return NULL;
		}

		int index = stbds_shgeti(hm_style_to_font, style_name);

		if(index >= 0) {

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), \
		                    entry.name.file_pos); \
		free(style_name); \
		return NULL; \
	} while(false)

			char* result_buffer = NULL;
			FORMAT_STRING_PROPAGATE_ERROR(&result_buffer, "style with the name '%s' already exists",
			                              style_name);

#undef PROPAGATE_ERROR_IMPL

			const DiagnosticSeverity severity_type =
			    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

			INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
			                         entry.name.file_pos, severity_type);

			free(style_name);
			continue;
		}

		StyleToFontHMEntry hm_entry = { .key = style_name, .value = entry.fontname };

		stbds_shputs(hm_style_to_font, hm_entry);
	}

	UsedFontsHM used_fonts = STBDS_HASH_MAP_EMPTY;

	for(size_t i = 0; i < stbds_arrlenu(ass_result.events.entries); ++i) {
		AssEventEntry entry = ass_result.events.entries[i];

		if(entry.type != EventTypeComment && entry.type != EventTypeDialogue) {
			continue;
		}

		char* style_name = get_normalized_string(entry.style);

		if(!style_name) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
			                    EMPTY_POS());
			return NULL;
		}

		int index = stbds_shgeti(hm_style_to_font, style_name);

		if(index < 0) {

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), \
		                    entry.name.file_pos); \
		free(style_name); \
		return NULL; \
	} while(false)

			char* result_buffer = NULL;
			FORMAT_STRING_PROPAGATE_ERROR(&result_buffer, "style '%s' for event line not found",
			                              style_name);

#undef PROPAGATE_ERROR_IMPL

			const DiagnosticSeverity severity_type =
			    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

			INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
			                         entry.style.file_pos, severity_type);

			free(style_name);
			continue;
		}

		free(style_name);

		StyleToFontHMEntry style_to_font_entry = hm_style_to_font[index];

		char* font_name = get_normalized_string(style_to_font_entry.value);

		if(!font_name) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
			                    EMPTY_POS());
			return NULL;
		}

		// insert font, if not already in the hm (that is used like a set)
		int font_index = stbds_shgeti(used_fonts, font_name);

		if(font_index < 0) {
			UsedFontHMEntry used_font_entry = { .key = font_name, .value = MONOSTATE_VALUE };

			stbds_shputs(used_fonts, used_font_entry);
		} else {
			free(font_name);
		}
	}

	// this hm is not needed anymore
	free_style_to_font_hm(&hm_style_to_font);

	if(used_fonts == STBDS_HASH_MAP_EMPTY) {
		UsedFontHMEntry default_value = { .key = NULL, .value = MONOSTATE_VALUE };
		// forces the length to be 0, but the pointer to not be null!
		stbds_shdefaults(used_fonts, default_value);
	}

	return used_fonts;
}

static void validate_fonts(AssResult ass_result, bool allow_validation_errors,
                           Diagnostics* diagnostics, DetailedFontValidateSettings settings) {

	if(!FcInit()) {

		const DiagnosticSeverity severity_type =
		    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

		INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries,
		                         STATIC_MESSAGE_STRUCT("failed to load fontconfig"), EMPTY_POS(),
		                         severity_type);
		return;
	}

	UsedFontsHM used_fonts = get_used_fonts(ass_result, allow_validation_errors, diagnostics);

	if(used_fonts == NULL) {
		// an error was already reported
		return;
	}

	for(size_t i = 0; i < stbds_arrlenu(ass_result.styles.entries); ++i) {
		AssStyleEntry entry = ass_result.styles.entries[i];

		char* font_name = get_normalized_string(entry.fontname);

		if(!font_name) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
			                    EMPTY_POS());

			free_used_fonts_hm(&used_fonts);
			return;
		}

		if(settings.check_only_used_fonts) {
			int font_index = stbds_shgeti(used_fonts, font_name);

			if(font_index < 0) {

				free(font_name);
				continue;
			}
		}

		FontStyleType search_type = get_style_type_for_font(entry);

		validate_font(font_name, search_type, entry.fontname.file_pos, allow_validation_errors,
		              diagnostics, settings);
		free(font_name);
	}

	free_used_fonts_hm(&used_fonts);

	FcFini();
}

#define MAX_ANGLE_DEGREE_DOUBLE 360.0

static void validate_style_angle(double angle, FilePos file_pos, bool allow_validation_errors,
                                 Diagnostics* diagnostics) {

	if(angle < 0.0 || angle > MAX_ANGLE_DEGREE_DOUBLE) {

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), file_pos); \
		return; \
	} while(false)

		char* result_buffer = NULL;
		FORMAT_STRING_PROPAGATE_ERROR(&result_buffer, "value for angle is out of range: %f", angle);

#undef PROPAGATE_ERROR_IMPL

		const DiagnosticSeverity severity_type =
		    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

		INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
		                         file_pos, severity_type);
	}
}

static void validate_styles(AssResult ass_result, bool allow_validation_errors,
                            Diagnostics* diagnostics) {

	for(size_t i = 0; i < stbds_arrlenu(ass_result.styles.entries); ++i) {
		AssStyleEntry entry = ass_result.styles.entries[i];

		// TODO: validate windows encoding
		// validate_style_encoding(entry, diagnostics);

		validate_style_angle(entry.angle, entry.name.file_pos, allow_validation_errors,
		                     diagnostics);
	}
}

static void validate_text(AssResult ass_result, bool allow_validation_errors,
                          Diagnostics* diagnostics) {

	// TODO: check and parse text value, for invalid escape sequences, and invald
	// values inside {}, like eg {bogus}, or {\j} etc, or not closed {} blocks

	UNUSED(ass_result);
	UNUSED(allow_validation_errors);
	UNUSED(diagnostics);
}

void validate_ass_result(AssResult ass_result, ParseSettings settings, Diagnostics* diagnostics) {

	DetailedFontValidateSettings font_settings =
	    get_detailed_font_settings_from_presset(settings.validate_settings.font_settings.preset);

	if(font_settings.enabled) {
		validate_fonts(ass_result, settings.strict_settings.allow_validation_errors, diagnostics,
		               font_settings);
	}

	if(settings.validate_settings.validate_styles) {
		validate_styles(ass_result, settings.strict_settings.allow_validation_errors, diagnostics);
	}

	if(settings.validate_settings.validate_text) {
		validate_text(ass_result, settings.strict_settings.allow_validation_errors, diagnostics);
	}
}

[[nodiscard]] int parse_font_preset(const char* preset) {

	if(strcmp(preset, "disabled") == 0) {
		return FontPresetDisabled;
	}
	if(strcmp(preset, "strict-all") == 0) {
		return FontPresetStrictAll;
	}
	if(strcmp(preset, "strict") == 0) {
		return FontPresetStrict;
	}
	if(strcmp(preset, "moderate") == 0) {
		return FontPresetModerate;
	}
	if(strcmp(preset, "lenient") == 0) {
		return FontPresetLenient;
	}

	return -1;
}
