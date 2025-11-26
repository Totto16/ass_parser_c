

#pragma once

#include "../../helper/decl.h"
#include <my_malloc/statistics.h>
#include <stddef.h>
#include <stdint.h>

#include "../../public/ass_parser_lib.h"

PUBLIC("allocator_get_statistics")
CATEGORY_POINTER AllocatorStatistics* allocator_get_statistics(void);

PUBLIC("allocator_statistics_get_free")
CATEGORY_LITERAL uint64_t allocator_statistics_get_free(AllocatorStatistics* statistics);

PUBLIC("allocator_statistics_get_total")
CATEGORY_LITERAL uint64_t allocator_statistics_get_total(AllocatorStatistics* statistics);

PUBLIC("allocator_statistics_get_used")
CATEGORY_LITERAL uint64_t allocator_statistics_get_used(AllocatorStatistics* statistics);

PUBLIC("allocator_statistics_get_metadata")
CATEGORY_LITERAL uint64_t allocator_statistics_get_metadata(AllocatorStatistics* statistics);

PUBLIC("source_from_string")
ANNOTATION_MALLOCED_RESULT("free")
ANNOTATION_NULLABLE AssSource* source_from_string(const char* source, size_t len);

PUBLIC("default_parse_settings")
ANNOTATION_MALLOCED_RESULT("free")
ANNOTATION_NULLABLE ParseSettings* default_parse_settings(void);

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
CATEGORY_VOID void set_settings_option(ParseSettings* settings, SettingsOption option, int value);

PUBLIC("diagnostics_get_length")
CATEGORY_LITERAL size_t diagnostics_get_length(const Diagnostics* diagnostics);

PUBLIC("diagnostics_get_at")
ANNOTATION_NULLABLE const DiagnosticEntry* diagnostics_get_at(const Diagnostics* diagnostics,
                                                              size_t index);

PUBLIC("get_message_from_entry")
ANNOTATION_MALLOCED_RESULT("free_message_struct")
ANNOTATION_NULLABLE MessageStruct* get_message_from_entry_js(const DiagnosticEntry* entry);

PUBLIC("free_message_struct")
ANNOTATION_FREE_FN void free_message_struct_ptr_js(MessageStruct* msg);

PUBLIC("diagnostic_get_file_pos")
CATEGORY_POINTER const FilePos* diagnostic_get_file_pos(const DiagnosticEntry* entry);

PUBLIC("diagnostic_get_severity")
CATEGORY_ENUM(ENUM_ANNOT_DiagnosticSeverity)
DiagnosticSeverity diagnostic_get_severity(const DiagnosticEntry* entry);

PUBLIC("file_pos_get_line") CATEGORY_LITERAL size_t file_pos_get_line(const FilePos* pos);

PUBLIC("file_pos_get_column") CATEGORY_LITERAL size_t file_pos_get_column(const FilePos* pos);

PUBLIC("events_get_length") CATEGORY_LITERAL size_t events_get_length(const AssEvents* events);

PUBLIC("events_get_at")
ANNOTATION_NULLABLE const AssEventEntry* events_get_at(const AssEvents* events, size_t index);

PUBLIC("styles_get_length") CATEGORY_LITERAL size_t styles_get_length(const AssStyles* styles);

PUBLIC("styles_get_at")
ANNOTATION_NULLABLE const AssStyleEntry* styles_get_at(const AssStyles* styles, size_t index);

PUBLIC("get_script_info_from_ass_result")
CATEGORY_POINTER const AssScriptInfo* get_script_info_from_ass_result(const AssResult* ass_result);

PUBLIC("get_styles_from_ass_result")
CATEGORY_POINTER const AssStyles* get_styles_from_ass_result(const AssResult* ass_result);

PUBLIC("get_events_from_ass_result")
CATEGORY_POINTER const AssEvents* get_events_from_ass_result(const AssResult* ass_result);

PUBLIC("get_extra_sections_from_ass_result")
CATEGORY_POINTER const ExtraSections*
get_extra_sections_from_ass_result(const AssResult* ass_result);

PUBLIC("get_file_props_from_ass_result")
CATEGORY_POINTER const FileProps* get_file_props_from_ass_result(const AssResult* ass_result);

PUBLIC("get_scaled_border_and_shadow_from_script_info")
CATEGORY_LITERAL bool
get_scaled_border_and_shadow_from_script_info(const AssScriptInfo* script_info);

PUBLIC("get_script_type_from_script_info")
CATEGORY_ENUM(ENUM_ANNOT_ScriptType)
ScriptType get_script_type_from_script_info(const AssScriptInfo* script_info);

PUBLIC("get_wrap_style_from_script_info")
CATEGORY_ENUM(ENUM_ANNOT_WrapStyle)
WrapStyle get_wrap_style_from_script_info(const AssScriptInfo* script_info);

PUBLIC("get_video_aspect_ratio_from_script_info")
CATEGORY_LITERAL size_t get_video_aspect_ratio_from_script_info(const AssScriptInfo* script_info);

PUBLIC("get_video_zoom_from_script_info")
CATEGORY_LITERAL size_t get_video_zoom_from_script_info(const AssScriptInfo* script_info);

PUBLIC("get_play_res_x_from_script_info")
CATEGORY_LITERAL size_t get_play_res_x_from_script_info(const AssScriptInfo* script_info);

PUBLIC("get_play_res_y_from_script_info")
CATEGORY_LITERAL size_t get_play_res_y_from_script_info(const AssScriptInfo* script_info);

PUBLIC("get_string_by_name_from_script_info")
ANNOTATION_MALLOCED_RESULT("free")
ANNOTATION_NULLABLE ANNOTATION_CSTRING
char* get_string_by_name_from_script_info(const AssScriptInfo* script_info, const char* name);

PUBLIC("get_ass_color_from_ass_style")
ANNOTATION_NULLABLE const AssColor* get_ass_color_from_ass_style(const AssStyleEntry* ass_style,
                                                                 uint8_t index);

PUBLIC("get_color_component_from_ass_color")
CATEGORY_LITERAL uint8_t get_color_component_from_ass_color(const AssColor* ass_color,
                                                            uint8_t index);

PUBLIC("get_file_type_from_file_props")
CATEGORY_ENUM(ENUM_ANNOT_FileType) FileType get_file_type_from_file_props(const FileProps* file_props);

PUBLIC("get_line_type_from_file_props")
CATEGORY_ENUM(ENUM_ANNOT_LineType) LineType get_line_type_from_file_props(const FileProps* file_props);

PUBLIC("get_entry_from_name_in_extra_sections")
ANNOTATION_NULLABLE ExtraSectionEntry*
get_entry_from_name_in_extra_sections(ExtraSections* extra_sections, char* name);

PUBLIC("get_entry_from_name_in_extra_section_entry")
ANNOTATION_NULLABLE ANNOTATION_CSTRING char*
get_entry_from_name_in_extra_section_entry(ExtraSectionEntry* extra_section_entry, char* name);

PUBLIC("extra_sections_hm_get_at")
ANNOTATION_NULLABLE const ExtraSectionHashMapEntry*
extra_sections_hm_get_at(const ExtraSections* extra_sections_hm, size_t index);

PUBLIC("extra_sections_hm_get_length")
CATEGORY_LITERAL size_t extra_sections_hm_get_length(ExtraSections* extra_sections_hm);

PUBLIC("extra_section_entry_hm_get_at")
ANNOTATION_NULLABLE const SectionFieldEntry*
extra_section_entry_hm_get_at(const ExtraSectionEntry* extra_section_entry_hm, size_t index);

PUBLIC("extra_section_entry_hm_get_length")
CATEGORY_LITERAL size_t
extra_section_entry_hm_get_length(ExtraSectionEntry* extra_section_entry_hm);

PUBLIC("extra_sections_hm_entry_get_key")
ANNOTATION_CSTRING
char* extra_sections_hm_entry_get_key(const ExtraSectionHashMapEntry* extra_sections_hm_entry);

PUBLIC("extra_sections_hm_entry_get_value")
ANNOTATION_NULLABLE const ExtraSectionEntry*
extra_sections_hm_entry_get_value(const ExtraSectionHashMapEntry* extra_sections_hm_entry);

PUBLIC("extra_section_entry_hm_entry_get_key")
ANNOTATION_CSTRING char*
extra_section_entry_hm_entry_get_key(const SectionFieldEntry* extra_section_entry_hm_entry);

PUBLIC("extra_section_entry_hm_entry_get_value")
ANNOTATION_NULLABLE char*
extra_section_entry_hm_entry_get_value(const SectionFieldEntry* extra_section_entry_hm_entry);
