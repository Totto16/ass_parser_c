#include "./helper.h"

#include <assert.h>
#include <stdlib.h>

#include <stb/ds.h>

AllocatorStatistics* allocator_get_statistics(void) {

	AllocatorStatistics* result = malloc(sizeof(AllocatorStatistics));

	if(result == NULL) {
		return NULL;
	}

	*result = my_malloc_get_statistics();

	return result;
}

uint64_t allocator_statistics_get_free(AllocatorStatistics statistics) {
	return statistics.free;
}

uint64_t allocator_statistics_get_total(AllocatorStatistics statistics) {
	return statistics.total;
}

uint64_t allocator_statistics_get_used(AllocatorStatistics statistics) {
	return statistics.used;
}

uint64_t allocator_statistics_get_metadata(AllocatorStatistics statistics) {
	return statistics.metadata;
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
			PANIC("UNRECAHBLE");
			break;
		}
	}
}

size_t diagnostics_get_length(Diagnostics* diagnostics) {
	return stbds_arrlenu(diagnostics->entries);
}

DiagnosticEntry* diagnostics_get_at(Diagnostics* diagnostics, size_t index) {
	if(index >= stbds_arrlenu(diagnostics->entries)) {
		return NULL;
	}

	return &(diagnostics->entries[index]);
}

MessageStruct* get_message_from_entry_js(DiagnosticEntry* entry) {
	MessageStruct* result = malloc(sizeof(MessageStruct));

	if(result == NULL) {
		return NULL;
	}

	*result = get_message_from_entry(*entry);

	return result;
}

FilePos* diagnostic_get_file_pos(DiagnosticEntry* entry) {
	return &(entry->position);
}

DiagnosticSeverity diagnostic_get_severity(DiagnosticEntry* entry) {
	return entry->severity;
}

size_t file_pos_get_line(FilePos* pos) {
	return pos->line;
}

size_t file_pos_get_column(FilePos* pos) {
	return pos->column;
}

size_t events_get_length(AssEvents* events) {
	return stbds_arrlenu(events->entries);
}

AssEventEntry* events_get_at(AssEvents* events, size_t index) {
	if(index >= stbds_arrlenu(events->entries)) {
		return NULL;
	}

	return &(events->entries[index]);
}
