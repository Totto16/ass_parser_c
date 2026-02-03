#include "./helper.h"

#include <assert.h>
#include <stdlib.h>

#include <tmap.h>

// this is a hack, so that I don't need malloc the result, since only ptrs can be returned to wasm
static AllocatorStatistics g_global_stats = {};

AllocatorStatistics* allocator_get_statistics(void) {

	g_global_stats = my_malloc_get_statistics();

	return &g_global_stats;
}

uint64_t allocator_statistics_get_free(AllocatorStatistics* statistics) {
	return statistics->free;
}

uint64_t allocator_statistics_get_total(AllocatorStatistics* statistics) {
	return statistics->total;
}

uint64_t allocator_statistics_get_used(AllocatorStatistics* statistics) {
	return statistics->used;
}

uint64_t allocator_statistics_get_metadata(AllocatorStatistics* statistics) {
	return statistics->metadata;
}

AssSource* source_from_string(const char* const source, size_t len) {

	AssSource* result = (AssSource*)malloc(sizeof(AssSource));

	if(result == NULL) {
		return NULL;
	}

	result->type = AssSourceTypeStr;
	result->data.str = (SizedPtr){ .data = (void*)source, .len = len };

	return result;
}

ParseSettings* default_parse_settings(void) {

	ParseSettings* result = (ParseSettings*)malloc(sizeof(ParseSettings));

	if(result == NULL) {
		return NULL;
	}

	result->strict_settings = (StrictSettings){ .script_info =
		                                            (ScriptInfoStrictSettings){
		                                                .allow_duplicate_fields = false,
		                                                .allow_missing_script_type = false,
		                                            },

		                                        .allow_additional_fields = false,
		                                        .allow_number_truncating = false,
		                                        .allow_unrecognized_file_encoding = false,
		                                        .allow_validation_errors = false };

	result->validate_settings =
	    (ValidateSettings){ .font_settings = (FontSettings){ .preset = FontPresetStrict },
		                    .validate_text = true,
		                    .validate_styles = true };

	return result;
}

#define BOOL_VAL(val) ((bool)(val) ? true : false)

void set_settings_option(ParseSettings* settings, SettingsOption option, int value) {

	switch(option) {
		case SettingsOption_Strict_Script_allow_duplicate_fields: {
			settings->strict_settings.script_info.allow_duplicate_fields = BOOL_VAL(value);
			break;
		}
		case SettingsOption_Strict_Script_allow_missing_script_type: {
			settings->strict_settings.script_info.allow_missing_script_type = BOOL_VAL(value);
			break;
		}
		case SettingsOption_Strict_allow_additional_fields: {
			settings->strict_settings.allow_additional_fields = BOOL_VAL(value);
			break;
		}
		case SettingsOption_Strict_allow_number_truncating: {
			settings->strict_settings.allow_number_truncating = BOOL_VAL(value);
			break;
		}
		case SettingsOption_Strict_allow_unrecognized_file_encoding: {
			settings->strict_settings.allow_unrecognized_file_encoding = BOOL_VAL(value);
			break;
		}
		case SettingsOption_Strict_allow_validation_errors: {
			settings->strict_settings.allow_validation_errors = BOOL_VAL(value);
			break;
		}
		case SettingsOption_Validate_Font_preset: {
			settings->validate_settings.font_settings.preset = (FontPreset)value;
			break;
		}
		case SettingsOption_Validate_validate_styles: {
			settings->validate_settings.validate_styles = BOOL_VAL(value);
			break;
		}
		case SettingsOption_Validate_validate_text: {
			settings->validate_settings.validate_text = BOOL_VAL(value);
			break;
		}
		default: {
			PANIC("UNREACHABLE");
			break;
		}
	}
}

size_t diagnostics_get_length(const Diagnostics* diagnostics) {
	return TVEC_LENGTH(diagnostics->entries);
}

const DiagnosticEntry* diagnostics_get_at(const Diagnostics* const diagnostics, size_t index) {
	if(index >= TVEC_LENGTH(diagnostics->entries)) {
		return NULL;
	}

	return TVEC_GET_AT(DiagnosticEntry, &(diagnostics->entries), index);
}

MessageStruct* get_message_from_entry_js(const DiagnosticEntry* entry) {
	MessageStruct* result = malloc(sizeof(MessageStruct));

	if(result == NULL) {
		return NULL;
	}

	*result = get_message_from_entry(*entry);

	return result;
}

void free_message_struct_ptr_js(MessageStruct* msg) {
	free_message_struct(*msg);
	free(msg);
}

const FilePos* diagnostic_get_file_pos(const DiagnosticEntry* entry) {
	return &(entry->position);
}

DiagnosticSeverity diagnostic_get_severity(const DiagnosticEntry* entry) {
	return entry->severity;
}

size_t file_pos_get_line(const FilePos* pos) {
	return pos->line;
}

size_t file_pos_get_column(const FilePos* pos) {
	return pos->column;
}

size_t events_get_length(const AssEvents* events) {
	return TVEC_LENGTH(events->entries);
}

const AssEventEntry* events_get_at(const AssEvents* const events, size_t index) {
	if(index >= TVEC_LENGTH(events->entries)) {
		return NULL;
	}

	return TVEC_GET_AT(AssEventEntry, &(events->entries), index);
}

size_t styles_get_length(const AssStyles* styles) {
	return TVEC_LENGTH(styles->entries);
}

const AssStyleEntry* styles_get_at(const AssStyles* const styles, size_t index) {
	if(index >= TVEC_LENGTH(styles->entries)) {
		return NULL;
	}

	return TVEC_GET_AT(AssStyleEntry, &(styles->entries), index);
}

const AssScriptInfo* get_script_info_from_ass_result(const AssResult* ass_result) {
	return &ass_result->script_info;
}

const AssStyles* get_styles_from_ass_result(const AssResult* ass_result) {
	return &ass_result->styles;
}

const AssEvents* get_events_from_ass_result(const AssResult* ass_result) {
	return &ass_result->events;
}

const ExtraSections* get_extra_sections_from_ass_result(const AssResult* ass_result) {
	return &ass_result->extra_sections;
}

const FileProps* get_file_props_from_ass_result(const AssResult* ass_result) {
	return &ass_result->file_props;
}

bool get_scaled_border_and_shadow_from_script_info(const AssScriptInfo* script_info) {
	return script_info->scaled_border_and_shadow;
}

ScriptType get_script_type_from_script_info(const AssScriptInfo* script_info) {
	return script_info->script_type;
}

WrapStyle get_wrap_style_from_script_info(const AssScriptInfo* script_info) {
	return script_info->wrap_style;
}

size_t get_video_aspect_ratio_from_script_info(const AssScriptInfo* script_info) {
	return script_info->video_aspect_ratio;
}

size_t get_video_zoom_from_script_info(const AssScriptInfo* script_info) {
	return script_info->video_zoom;
}

size_t get_play_res_x_from_script_info(const AssScriptInfo* script_info) {
	return script_info->play_res_x;
}

size_t get_play_res_y_from_script_info(const AssScriptInfo* script_info) {
	return script_info->play_res_y;
}

#define RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(value) \
	do { \
		if((value).start == NULL) { \
			char* res = (char*)malloc(1); \
			res[0] = '\0'; \
			return res; \
		} \
		return get_normalized_string(value); \
	} while(false)

char* get_string_by_name_from_script_info(const AssScriptInfo* script_info, const char* name) {

	if(strcmp(name, "title") == 0) {
		RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(script_info->title);
	}

	if(strcmp(name, "original_script") == 0) {
		RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(script_info->original_script);
	}

	if(strcmp(name, "original_translation") == 0) {
		RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(script_info->original_translation);
	}

	if(strcmp(name, "original_editing") == 0) {
		RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(script_info->original_editing);
	}

	if(strcmp(name, "original_timing") == 0) {
		RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(script_info->original_timing);
	}

	if(strcmp(name, "synch_point") == 0) {
		RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(script_info->synch_point);
	}

	if(strcmp(name, "script_updated_by") == 0) {
		RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(script_info->script_updated_by);
	}

	if(strcmp(name, "update_details") == 0) {
		RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(script_info->update_details);
	}

	if(strcmp(name, "collisions") == 0) {
		RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(script_info->collisions);
	}

	if(strcmp(name, "play_depth") == 0) {
		RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(script_info->play_depth);
	}

	if(strcmp(name, "timer") == 0) {
		RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(script_info->timer);
	}

	if(strcmp(name, "ycbcr_matrix") == 0) {
		RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(script_info->ycbcr_matrix);
	}

	return NULL;
}

ANNOTATION_NULLABLE const AssColor* get_ass_color_from_ass_style(const AssStyleEntry* ass_style,
                                                                 uint8_t index) {
	switch(index) {
		case 0: return &(ass_style->primary_colour);
		case 1: return &(ass_style->secondary_colour);
		case 2: return &(ass_style->outline_colour);
		case 3: return &(ass_style->back_colour);
		default: {
			return NULL;
		}
	}
}

uint8_t get_color_component_from_ass_color(const AssColor* ass_color, uint8_t index) {
	switch(index) {
		case 0: return ass_color->r;
		case 1: return ass_color->g;
		case 2: return ass_color->b;
		case 3: return ass_color->a;
		default: {
			PANIC("UNREACHABLE");
			return 0;
		}
	}
}

FileType get_file_type_from_file_props(const FileProps* file_props) {
	return file_props->file_type;
}

LineType get_line_type_from_file_props(const FileProps* file_props) {
	return file_props->line_type;
}

const ExtraSectionEntry*
get_entry_from_name_in_extra_sections(const ExtraSections* const extra_sections, char* name) {
	if(extra_sections == NULL) {
		return NULL;
	}

	const ExtraSectionEntry* value = TMAP_GET(ExtraSectionHashMapEntry, extra_sections, name);

	return value;
}

char* get_entry_from_name_in_extra_section_entry(const ExtraSectionEntry* const extra_section_entry,
                                                 char* name) {
	if(extra_section_entry == NULL) {
		return NULL;
	}

	const FinalStr* value = TMAP_GET(SectionFieldEntry, extra_section_entry, name);

	if(value == NULL) {
		return NULL;
	}

	RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(*value);
}

const ExtraSectionHashMapEntry* extra_sections_hm_get_at(const ExtraSections* extra_sections_hm,
                                                         size_t index) {
	size_t hm_total_length = TMAP_CAPACITY(*extra_sections_hm);
	if(index >= hm_total_length) {
		return NULL;
	}

	const ExtraSectionHashMapEntry* hm_entry =
	    TMAP_GET_ENTRY_AT(ExtraSectionHashMapEntry, extra_sections_hm, index);

	return hm_entry;
}

size_t extra_sections_hm_get_length(ExtraSections* extra_sections_hm) {
	size_t hm_total_length = TMAP_CAPACITY(*extra_sections_hm);
	return hm_total_length;
}

const SectionFieldEntry*
extra_section_entry_hm_get_at(const ExtraSectionEntry* extra_section_entry_hm, size_t index) {
	size_t hm_total_length = TMAP_CAPACITY(*extra_section_entry_hm);
	if(index >= hm_total_length) {
		return NULL;
	}

	const SectionFieldEntry* hm_entry =
	    TMAP_GET_ENTRY_AT(SectionFieldEntry, extra_section_entry_hm, index);

	return hm_entry;
}

size_t extra_section_entry_hm_get_length(ExtraSectionEntry* extra_section_entry_hm) {
	size_t hm_total_length = TMAP_CAPACITY(*extra_section_entry_hm);
	return hm_total_length;
}

char* extra_sections_hm_entry_get_key(const ExtraSectionHashMapEntry* extra_sections_hm_entry) {
	return extra_sections_hm_entry->key;
}

const ExtraSectionEntry*
extra_sections_hm_entry_get_value(const ExtraSectionHashMapEntry* extra_sections_hm_entry) {
	return &(extra_sections_hm_entry->value);
}

char* extra_section_entry_hm_entry_get_key(const SectionFieldEntry* extra_section_entry_hm_entry) {
	return extra_section_entry_hm_entry->key;
}

char* extra_section_entry_hm_entry_get_value(
    const SectionFieldEntry* extra_section_entry_hm_entry) {
	FinalStr value = extra_section_entry_hm_entry->value;

	RETURN_FINALSTR_TO_WASM_COMPATIBLE_STR(value);
}
