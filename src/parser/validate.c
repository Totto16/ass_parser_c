
#define _GNU_SOURCE // NOLINT(readability-identifier-naming,bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)
#include <string.h>
#undef _GNU_SOURCE

#define ASS_PARSER_C_INTERNAL_USAGE

#include "../helper/macros.h"
#include "./validate.h"

#undef ASS_PARSER_C_INTERNAL_USAGE

#include <zvec/zvec.h>

#include <zmap/zmap.h>

#include <stdio.h>

#ifdef ASS_PARSER_HAVE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

typedef enum : uint8_t {
	MatchResolutionExact,
	MatchResolutionIncludes,
	MatchResolutionLikeIs
} MatchResolution;

[[nodiscard]] static bool matches_font(const char* font_name, const char* received_name,
                                       MatchResolution resolution) {

	switch(resolution) {
		case MatchResolutionExact: {
			return strcasecmp(received_name, font_name) == 0;
		}
		case MatchResolutionIncludes: {
			const char* pos1 = strcasestr(received_name, font_name);

			if(pos1 != NULL) {
				return true;
			}

			const char* pos2 = strcasestr(font_name, received_name);

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

#ifdef ASS_PARSER_HAVE_FONTCONFIG

typedef struct {
	FcPattern* pattern;
	FcObjectSet* object_set;
	FcFontSet* font_list;
} FontConfigFontResultOk;

typedef struct {
	bool error;
	union {
		FontConfigFontResultOk ok;
		MessageStruct error;
	} data;
} FontConfigFontResultObject;

static void free_fontconfig_font_result_ok(FontConfigFontResultOk result) {
	FcPatternDestroy(result.pattern);
	FcObjectSetDestroy(result.object_set);
	FcFontSetDestroy(result.font_list);
}

static void free_fontconfig_font_result(FontConfigFontResultObject result) {
	if(!result.error) {
		free_fontconfig_font_result_ok(result.data.ok);
	} else {
		free_message_struct(result.data.error);
	}
}

static FontConfigFontResultObject fontconfig_find_fonts_by_family_name(const char* font_name) {
	// Build a pattern to search for
	FcPattern* pattern = FcPatternCreate();

	if(pattern == NULL) {
		return (FontConfigFontResultObject){
			.error = true,
			.data = { .error = STATIC_MESSAGE_STRUCT("fontconfig: error in pattern creation") }
		};
	}

#define FREE_AT_END() \
	do { \
		FcPatternDestroy(pattern); \
	} while(false)

	if(!FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*)font_name)) {
		FREE_AT_END();

		return (FontConfigFontResultObject){
			.error = true,
			.data = { .error = STATIC_MESSAGE_STRUCT("fontconfig: error in pattern setup") }
		};
	}

	FcObjectSet* object_set = FcObjectSetBuild(FC_FAMILY, FC_FILE, FC_STYLE, FC_SLANT, NULL);

	if(object_set == NULL) {
		FREE_AT_END();

		return (FontConfigFontResultObject){ .error = true,
			                                 .data = { .error = STATIC_MESSAGE_STRUCT(
			                                               "fontconfig: error in set build") } };
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

		return (FontConfigFontResultObject){
			.error = true,
			.data = { .error = STATIC_MESSAGE_STRUCT("fontconfig: error in list retrieval") }
		};
	}

	return (FontConfigFontResultObject){ .error = false,
		                                 .data = { .ok = (FontConfigFontResultOk){
		                                               .pattern = pattern,
		                                               .object_set = object_set,
		                                               .font_list = font_list } } };
}

#undef FREE_AT_END

#endif

ZVEC_DEFINE_AND_IMPLEMENT_VEC_TYPE(AssFontName)

typedef ZVEC_TYPENAME(AssFontName) AssFontNames;

static AssFontNames*
embedded_fonts_find_fonts_by_family_name(AssFonts ass_fonts, const char* font_name,
                                         DetailedFontValidateSettings settings) {

	AssFontNames* names = malloc(sizeof(AssFontNames));
	*names = ZVEC_EMPTY(AssFontName);

#define FREE_AT_END() \
	do { \
		ZVEC_FREE(AssFontName, names); \
	} while(false)

	for(size_t i = 0; i < ZVEC_LENGTH(ass_fonts.entries); ++i) {
		AssFontEntry entry = ZVEC_AT(AssFontEntry, (ass_fonts.entries), i);

		char* received_name = get_normalized_string(entry.name.name);

		if(!received_name) {
			FREE_AT_END();
			return NULL;
		}

		if(matches_font(font_name, received_name, settings.font_match_res)) {
			ZVEC_PUSH_SLOT_AND_ASSERT(AssFontName, names, entry.name);
		}

		free(received_name);
	}

	return names;
}

#undef FREE_AT_END

typedef enum : uint8_t {
	FontHandleTypeFontConfig,
	FontHandleTypeEmbedded,
} FontHandleType;

typedef struct {
	size_t index_in_list;
} FontHandleDataFontConfig;

typedef struct {
	AssFontName font_name;
} FontHandleDataEmbedded;

typedef struct {
	FontHandleType type;
	union {
		FontHandleDataFontConfig font_config;
		FontHandleDataEmbedded embedded;
	} data;
} FontHandle;

typedef struct {
#ifdef ASS_PARSER_HAVE_FONTCONFIG
	FontConfigFontResultOk fontconfig;
#endif
	AssFontNames embedded;
} FontConfigRefs;

ZVEC_DEFINE_AND_IMPLEMENT_VEC_TYPE(FontHandle)

typedef ZVEC_TYPENAME(FontHandle) FontHandles;

typedef struct {
	FontHandles handles;
	FontConfigRefs refs;
} FontResultOk;

typedef struct {
	bool error;
	union {
		FontResultOk ok;
		MessageStruct error;
	} data;
} FontResultObject;

static void free_font_result_ok(FontResultOk result) {
#ifdef ASS_PARSER_HAVE_FONTCONFIG
	free_fontconfig_font_result_ok(result.refs.fontconfig);
#endif
	ZVEC_FREE(AssFontName, &(result.refs.embedded));
	ZVEC_FREE(FontHandle, &(result.handles));
}

static void free_font_result(FontResultObject result) {
	if(!result.error) {
		free_font_result_ok(result.data.ok);
	} else {
		free_message_struct(result.data.error);
	}
}

static FontResultObject find_fonts_by_family_name(AssFonts ass_fonts, const char* font_name,
                                                  DetailedFontValidateSettings settings) {

#ifdef ASS_PARSER_HAVE_FONTCONFIG

	FontConfigFontResultObject font_config_result = fontconfig_find_fonts_by_family_name(font_name);

	if(font_config_result.error) {
		return (FontResultObject){ .error = true,
			                       .data = { .error = font_config_result.data.error } };
	}

#endif

	AssFontNames* embedded_result_temp =
	    embedded_fonts_find_fonts_by_family_name(ass_fonts, font_name, settings);

	if(embedded_result_temp == NULL) {
#ifdef ASS_PARSER_HAVE_FONTCONFIG
		free_fontconfig_font_result(font_config_result);
#endif
		return (FontResultObject){ .error = true,
			                       .data = { .error = STATIC_MESSAGE_STRUCT(
			                                     "embedded fonts, error in retrieval") } };
	}

	AssFontNames embedded_result = *embedded_result_temp;
	free(embedded_result_temp);

	FontHandles handles = ZVEC_EMPTY(FontHandle);

#ifdef ASS_PARSER_HAVE_FONTCONFIG
	FontConfigFontResultOk font_config_ok = font_config_result.data.ok;

	for(int i = 0; i < font_config_ok.font_list->nfont; ++i) {

		FontHandle handle = { .type = FontHandleTypeFontConfig,
			                  .data = { .font_config = { .index_in_list = i } } };

		ZVEC_PUSH_SLOT_AND_ASSERT(FontHandle, &handles, handle);
	}

#endif

	for(size_t i = 0; i < ZVEC_LENGTH(embedded_result); ++i) {
		FontHandle handle = {
			.type = FontHandleTypeEmbedded,
			.data = { .embedded = { .font_name = ZVEC_AT(AssFontName, embedded_result, i) } }
		};

		ZVEC_PUSH_SLOT_AND_ASSERT(FontHandle, &handles, handle);
	}

	FontResultOk ok_result = { .handles = handles,
		                       .refs = {
#ifdef ASS_PARSER_HAVE_FONTCONFIG
		                           .fontconfig = font_config_ok,
#endif
		                           .embedded = embedded_result } };

	FontResultObject result = { .error = false, .data = { .ok = ok_result } };

	return result;
}

// note: strike_out and underline are not specifically supported by fonts, but they work for all
// fonts (at least I did understand it that way, also see specs for embedded font, who don#t even
// care about those two flags)
typedef enum : uint8_t {
	FontStyleTypeNormal,
	FontStyleTypeBold,
	FontStyleTypeItalic,
	FontStyleTypeBoldItalic,
} FontStyleType;

[[nodiscard]] static const char* get_search_type_name(FontStyleType type) {
	switch(type) {
		case FontStyleTypeNormal: return "Normal";
		case FontStyleTypeBold: return "Bold";
		case FontStyleTypeItalic: return "Italic";
		case FontStyleTypeBoldItalic: return "Bold Italic";
		default: return "<unknown>";
	}
}

[[nodiscard]] static FontStyleType get_style_type_impl(bool bold, bool italic) {

	if(bold && italic) {
		return FontStyleTypeBoldItalic;
	}

	if(bold) {
		return FontStyleTypeBold;
	}

	if(italic) {
		return FontStyleTypeItalic;
	}

	return FontStyleTypeNormal;
}

[[nodiscard]] static FontStyleType get_style_type_for_font(AssStyleEntry entry) {
	return get_style_type_impl(entry.bold, entry.italic);
}

[[nodiscard]] static FontStyleType get_style_type_for_ass_font_name(AssFontName name) {

	return get_style_type_impl(name.bold, name.italic);
}

typedef struct {
	size_t size;
	const char** values;
} StaticStringArray;

[[nodiscard]] __attribute__((__unused__)) static bool is_valid_name_for_type(FontStyleType type,
                                                                             char* name) {

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

		if(strcasecmp(name, value) == 0) {
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

static void free_font_search_result(FontSearchResult result) {
	if(result.type == FontSearchResultTypeError) {
		free_message_struct(result.data.error);
	}
}

#ifdef ASS_PARSER_HAVE_FONTCONFIG

[[nodiscard]] static FontSearchResult
fontconfig_find_type_for_fonts(const char* font_name, FcPattern* font, FontStyleType font_type,
                               DetailedFontValidateSettings settings) {

	FcChar8* family = NULL;

	FcResult res = FcPatternGetString(font, FC_FAMILY, 0, &family);

	if(res != FcResultMatch) {

		return (FontSearchResult){ .type = FontSearchResultTypeError,
			                       .data = { .error = STATIC_MESSAGE_STRUCT(
			                                     "couldn't get font family") } };
	}

	if(!matches_font(font_name, (char*)family, settings.font_match_res)) {
		return (FontSearchResult){ .type = FontSearchResultTypeNotFound };
	}

	for(int i = 0;; ++i) {
		FcChar8* result = NULL;

		FcResult res = FcPatternGetString(font, FC_STYLE, i, &result);

		switch(res) {
			case FcResultMatch: {
				if(result == NULL) {

					return (
					    FontSearchResult){ .type = FontSearchResultTypeError,
						                   .data = { .error = STATIC_MESSAGE_STRUCT(
						                                 "result matched but string was null") } };
				}

				if(is_valid_name_for_type(font_type, (char*)result)) {
					return (FontSearchResult){ .type = FontSearchResultTypeFound };
				}
				break;
			}
			case FcResultNoId: {
				return (FontSearchResult){ .type = FontSearchResultTypeNotFound };
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

				return (
				    FontSearchResult){ .type = FontSearchResultTypeError,
					                   .data = { .error = DYNAMIC_MESSAGE_STRUCT(result_buffer) } };
			}
		}
	}

	return (FontSearchResult){ .type = FontSearchResultTypeNotFound };
}

#endif

[[nodiscard]] static FontSearchResult
embedded_find_type_for_fonts(const char* font_name, AssFontName name, FontStyleType font_type,
                             DetailedFontValidateSettings settings) {

	char* received_name = get_normalized_string(name.name);

	if(!received_name) {
		return (
		    FontSearchResult){ .type = FontSearchResultTypeError,
			                   .data = { .error = STATIC_MESSAGE_STRUCT("alllocation error") } };
	}

	if(!matches_font(font_name, received_name, settings.font_match_res)) {
		free(received_name);
		return (FontSearchResult){ .type = FontSearchResultTypeNotFound };
	}

	free(received_name);

	FontStyleType received_type = get_style_type_for_ass_font_name(name);

	if(font_type == received_type) {
		return (FontSearchResult){ .type = FontSearchResultTypeFound };
	}

	return (FontSearchResult){ .type = FontSearchResultTypeNotFound };
}

[[nodiscard]] FontSearchResult find_type_for_fonts(const char* font_name, FontResultOk fonts_result,
                                                   FontStyleType font_type,
                                                   DetailedFontValidateSettings settings,
                                                   bool strict_errors) {
	for(size_t i = 0; i < ZVEC_LENGTH(fonts_result.handles); ++i) {

		FontHandle handle = ZVEC_AT(FontHandle, (fonts_result.handles), i);

		FontSearchResult res = { .type = FontSearchResultTypeNotFound };

		switch(handle.type) {
			case FontHandleTypeFontConfig: {
#ifdef ASS_PARSER_HAVE_FONTCONFIG
				size_t index = handle.data.font_config.index_in_list;

				FcPattern* font = fonts_result.refs.fontconfig.font_list->fonts[index];

				res = fontconfig_find_type_for_fonts(font_name, font, font_type, settings);
#else
				res = (FontSearchResult){ .type = FontSearchResultTypeError,
					                      .data = { .error = STATIC_MESSAGE_STRUCT(
					                                    "fontconfig not supported") } };
#endif
				break;
			}
			case FontHandleTypeEmbedded: {
				AssFontName name = handle.data.embedded.font_name;

				res = embedded_find_type_for_fonts(font_name, name, font_type, settings);

				break;
			}
			default: {
				UNREACHABLE();
			}
		}

		switch(res.type) {
			case FontSearchResultTypeFound: {
				return (FontSearchResult){ .type = FontSearchResultTypeFound };
			}
			case FontSearchResultTypeNotFound: {
				// just search further
				break;
			}
			case FontSearchResultTypeError: {
				// if we are strict about errors, report it, otherwise discard it and go on

				if(strict_errors) {
					return res;
				}

				free_font_search_result(res);
				break;
			}
			default: {
				UNREACHABLE();
			}
		}
	}

	return (FontSearchResult){ .type = FontSearchResultTypeNotFound };
}

#undef PROPAGATE_ERROR_IMPL

static void validate_font(AssFonts ass_fonts, const char* style_name, const char* font_name,
                          FontStyleType search_type, FilePos file_pos, bool allow_validation_errors,
                          Diagnostics* diagnostics, DetailedFontValidateSettings settings) {

	bool strict_errors = !allow_validation_errors;

	FontResultObject result = find_fonts_by_family_name(ass_fonts, font_name, settings);

#define FREE_AT_END() \
	do { \
		free_font_result(result); \
	} while(false)

	if(result.error) {
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("font retrieval error"),
		                    EMPTY_POS());

		FREE_AT_END();
		return;
	}

	FontResultOk ok_res = result.data.ok;

	if(ZVEC_LENGTH(ok_res.handles) == 0) {

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), file_pos); \
		FREE_AT_END(); \
		return; \
	} while(false)

		char* result_buffer = NULL;
		FORMAT_STRING_PROPAGATE_ERROR(&result_buffer, "style '%s': no font for '%s' found",
		                              style_name, font_name);

		const DiagnosticSeverity severity_type =
		    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

		INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
		                         file_pos, severity_type);

		FREE_AT_END();
		return;
	}

	FontSearchResult found_result =
	    find_type_for_fonts(font_name, ok_res, search_type, settings, strict_errors);

#undef FREE_AT_END
#define FREE_AT_END() \
	do { \
		free_font_result(result); \
		free_font_search_result(found_result); \
	} while(false)

	if(found_result.type == FontSearchResultTypeError) {
		char* result_buffer = NULL; // NOLINT(clang-analyzer-unix.Malloc)
		FORMAT_STRING_PROPAGATE_ERROR(&result_buffer,
		                              "an error occurred while trying to find font '%s': %s",
		                              font_name, get_message(found_result.data.error));

		const DiagnosticSeverity severity_type =
		    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

		INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries, DYNAMIC_MESSAGE_STRUCT(result_buffer),
		                         file_pos, severity_type);

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

ZMAP_DEFINE_AND_IMPLEMENT_MAP_TYPE(char*, CHAR_PTR_KEYNAME, FinalStr, StyleToFontHMEntry)

typedef ZMAP_TYPENAME_MAP(StyleToFontHMEntry) StyleToFontHM;

static void free_style_to_font_hm(StyleToFontHM* style_to_font_hm) {

	size_t hm_total_length = ZMAP_CAPACITY(*style_to_font_hm);

	for(size_t i = 0; i < hm_total_length; ++i) {
		ZMAP_TYPENAME_BUCKET(StyleToFontHMEntry) hm_bucket = style_to_font_hm->buckets[i];

		if(hm_bucket.state == ZMAP_OCCUPIED) {
			free(hm_bucket.key);
		}
	}

	ZMAP_FREE(StyleToFontHMEntry, style_to_font_hm);
}

#if defined(__clang__) && !defined(__WASM__)
typedef struct {
} MonoState;

#define MONOSTATE MonoState
#define MONOSTATE_VALUE ((MonoState){})

#else

#define MONOSTATE bool
#define MONOSTATE_VALUE true

#endif

ZMAP_DEFINE_AND_IMPLEMENT_MAP_TYPE(char*, CHAR_PTR_KEYNAME, MONOSTATE, UsedFontHMEntry)

typedef ZMAP_TYPENAME_MAP(UsedFontHMEntry) UsedFontsHM;

static void free_used_fonts_hm(UsedFontsHM* used_fonts_hm) {

	size_t hm_total_length = ZMAP_CAPACITY(*used_fonts_hm);

	for(size_t i = 0; i < hm_total_length; ++i) {
		ZMAP_TYPENAME_BUCKET(UsedFontHMEntry) hm_bucket = used_fonts_hm->buckets[i];

		if(hm_bucket.state == ZMAP_OCCUPIED) {
			free(hm_bucket.key);
		}
	}

	ZMAP_FREE(UsedFontHMEntry, used_fonts_hm);
}

[[nodiscard]] static UsedFontsHM* get_used_fonts(AssResult ass_result, bool allow_validation_errors,
                                                 Diagnostics* diagnostics) {

#define FREE_AT_END() \
	do { \
		free_style_to_font_hm(&hm_style_to_font); \
	} while(false)

	StyleToFontHM hm_style_to_font = ZMAP_INIT(StyleToFontHMEntry);

	for(size_t i = 0; i < ZVEC_LENGTH(ass_result.styles.entries); ++i) {
		AssStyleEntry entry = ZVEC_AT(AssStyleEntry, (ass_result.styles.entries), i);

		char* style_name = get_normalized_string(entry.name);

		if(!style_name) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
			                    EMPTY_POS());

			FREE_AT_END();
			return NULL;
		}

		FinalStr* font_hm_entry = NULL;

		{
			ZMAP_ASSERT_SHOULD_USE_INSERT_SLOT(entry.fontname);

			// NOTE: overwrite check is required here
			FinalStr* slot =
			    ZMAP_INSERT_SLOT(StyleToFontHMEntry, &hm_style_to_font, style_name, false);

			ASSERT(slot != NULL, "OOM");
			font_hm_entry = slot;
		}

		if(font_hm_entry == Z_WOULD_OVERWRITE) {

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), \
		                    entry.name.file_pos); \
		FREE_AT_END(); \
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

		// this is a ZMAP_INSERT, but in a faster way, as we already need to check for a duplicate
		// entry above
		*font_hm_entry = entry.fontname;
	}

#undef FREE_AT_END
#define FREE_AT_END() \
	do { \
		free_used_fonts_hm(used_fonts); \
		free_style_to_font_hm(&hm_style_to_font); \
	} while(false)

	UsedFontsHM* used_fonts = malloc(sizeof(UsedFontsHM));

	*used_fonts = ZMAP_INIT(UsedFontHMEntry);

	for(size_t i = 0; i < ZVEC_LENGTH(ass_result.events.entries); ++i) {
		AssEventEntry entry = ZVEC_AT(AssEventEntry, (ass_result.events.entries), i);

		if(entry.type != EventTypeComment && entry.type != EventTypeDialogue) {
			continue;
		}

		char* style_name = get_normalized_string(entry.style);

		if(!style_name) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
			                    EMPTY_POS());

			FREE_AT_END();
			return NULL;
		}

		const FinalStr* font_value = ZMAP_GET(StyleToFontHMEntry, &hm_style_to_font, style_name);

		if(font_value == NULL) {

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT(message), \
		                    entry.name.file_pos); \
		free(style_name); \
		FREE_AT_END(); \
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

		char* font_name = get_normalized_string(*font_value);

		if(!font_name) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
			                    EMPTY_POS());
			FREE_AT_END();
			return NULL;
		}

		// TODO: make used_fonts a zvec and use bsearch to acomplish the same thing!

		// TODO: to make the free correct, see the TODO in the zmap type!

		{

			ZMAP_ASSERT_SHOULD_USE_INSERT(MONOSTATE_VALUE);

			// insert font, if not already in the hm (that is used like a set)
			ASSERT(ZMAP_INSERT(UsedFontHMEntry, used_fonts, font_name, MONOSTATE_VALUE, true) ==
			           ZmapInsertResultOk,
			       "OOM");
		}
	}

	// this hm is not needed anymore
	free_style_to_font_hm(&hm_style_to_font);

	return used_fonts;
}

#undef FREE_AT_END

static void validate_fonts_impl(AssResult ass_result, bool allow_validation_errors,
                                Diagnostics* diagnostics, DetailedFontValidateSettings settings) {

#ifdef ASS_PARSER_HAVE_FONTCONFIG
	if(!FcInit()) {

		const DiagnosticSeverity severity_type =
		    allow_validation_errors ? DiagnosticSeverityWarning : DiagnosticSeverityError;

		INSERT_SIMPLE_DIAGNOSTIC(diagnostics->entries,
		                         STATIC_MESSAGE_STRUCT("failed to load fontconfig"), EMPTY_POS(),
		                         severity_type);
		return;
	}
#endif

	UsedFontsHM* used_fonts = get_used_fonts(ass_result, allow_validation_errors, diagnostics);

#ifdef ASS_PARSER_HAVE_FONTCONFIG
#define FREE_AT_END() \
	do { \
		free_used_fonts_hm(used_fonts); \
		FcFini(); \
	} while(false)
#else
#define FREE_AT_END() \
	do { \
		free_used_fonts_hm(used_fonts); \
	} while(false)
#endif

	if(used_fonts == NULL) {
		// an error was already reported
		FREE_AT_END();
		return;
	}

	for(size_t i = 0; i < ZVEC_LENGTH(ass_result.styles.entries); ++i) {
		AssStyleEntry entry = ZVEC_AT(AssStyleEntry, (ass_result.styles.entries), i);

		char* font_name = get_normalized_string(entry.fontname);

		if(!font_name) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
			                    EMPTY_POS());

			FREE_AT_END();
			return;
		}

		char* style_name = get_normalized_string(entry.name);

		if(!style_name) {
			INSERT_SIMPLE_ERROR(diagnostics->entries, STATIC_MESSAGE_STRUCT("allocation error"),
			                    EMPTY_POS());

			FREE_AT_END();
			free(font_name);
			return;
		}

		if(settings.check_only_used_fonts) {
			const MONOSTATE* font_is_present = ZMAP_GET(UsedFontHMEntry, used_fonts, font_name);

			if(font_is_present == NULL) {

				free(font_name);
				free(style_name);
				continue;
			}
		}

		FontStyleType search_type = get_style_type_for_font(entry);

		validate_font(ass_result.fonts, style_name, font_name, search_type, entry.fontname.file_pos,
		              allow_validation_errors, diagnostics, settings);

		free(font_name);
		free(style_name);
	}

	FREE_AT_END();
}

#undef FREE_AT_END

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

	for(size_t i = 0; i < ZVEC_LENGTH(ass_result.styles.entries); ++i) {
		AssStyleEntry entry = ZVEC_AT(AssStyleEntry, (ass_result.styles.entries), i);

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

void validate_fonts_of_result(AssResult ass_result, bool allow_validation_errors, FontPreset preset,
                              Diagnostics* diagnostics) {

	DetailedFontValidateSettings font_settings = get_detailed_font_settings_from_presset(preset);

	validate_fonts_impl(ass_result, allow_validation_errors, diagnostics, font_settings);
}

void validate_ass_result(AssResult ass_result, ParseSettings settings, Diagnostics* diagnostics) {

	DetailedFontValidateSettings font_settings =
	    get_detailed_font_settings_from_presset(settings.validate_settings.font_settings.preset);

	if(font_settings.enabled) {
		validate_fonts_impl(ass_result, settings.strict_settings.allow_validation_errors,
		                    diagnostics, font_settings);
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

// TODO: validate also other events, check if files are existing (e.g. in graphics embeeded
// section or on disk and check some properties e.g. is valid image (with std_image, is
// executable fro script and  is valid video (maybe just by checking start bytes  + extension)))
