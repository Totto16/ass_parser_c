

#pragma once

#include "../../helper/decl.h"
#include <my_malloc/statistics.h>
#include <stddef.h>
#include <stdint.h>

#include "../../public/ass_parser_lib.h"

// expose allocator statistics

PUBLIC("allocator_get_statistics") AllocatorStatistics* allocator_get_statistics(void);

PUBLIC("allocator_statistics_get_free")
uint64_t allocator_statistics_get_free(AllocatorStatistics statistics);

PUBLIC("allocator_statistics_get_total")
uint64_t allocator_statistics_get_total(AllocatorStatistics statistics);

PUBLIC("allocator_statistics_get_used")
uint64_t allocator_statistics_get_used(AllocatorStatistics statistics);

PUBLIC("allocator_statistics_get_metadata")
uint64_t allocator_statistics_get_metadata(AllocatorStatistics statistics);

PUBLIC("source_from_string")
AssSource* source_from_string(const char* source, size_t len);

PUBLIC("default_parse_settings") ParseSettings* default_parse_settings(void);

typedef enum : uint8_t {
	//
	SettingsOption_Strict_Script_allow_duplicate_fields = 0,
	SettingsOption_Strict_Script_allow_missing_script_type,
	//
	SettingsOption_Strict_allow_additional_fields,
	SettingsOption_Strict_allow_number_truncating,
	SettingsOption_Strict_allow_unrecognized_file_encoding,
	SettingsOption_Strict_allow_validation_errors,
	//
	SettingsOption_Validate_Font_preset,
	//
	SettingsOption_Validate_validate_styles,
	SettingsOption_Validate_validate_text
} SettingsOption;

PUBLIC("set_settings_option")
void set_settings_option(ParseSettings* settings, SettingsOption option, int value);

PUBLIC("diagnostics_get_length")
size_t diagnostics_get_length(Diagnostics* diagnostics);

PUBLIC("diagnostics_get_at")
DiagnosticEntry* diagnostics_get_at(Diagnostics* diagnostics, size_t index);

PUBLIC("get_message_from_entry")
MessageStruct* get_message_from_entry_js(DiagnosticEntry* entry);

PUBLIC("diagnostic_get_file_pos")
FilePos* diagnostic_get_file_pos(DiagnosticEntry* entry);

PUBLIC("diagnostic_get_severity")
DiagnosticSeverity diagnostic_get_severity(DiagnosticEntry* entry);

PUBLIC("file_pos_get_line")
size_t file_pos_get_line(FilePos* pos);

PUBLIC("file_pos_get_column")
size_t file_pos_get_column(FilePos* pos);

PUBLIC("events_get_length")
size_t events_get_length(AssEvents* events);

PUBLIC("events_get_at")
AssEventEntry* events_get_at(AssEvents* events, size_t index);
