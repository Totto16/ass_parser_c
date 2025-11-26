import type { Annotated, Annotations, CategoryWrapper, CEnum, CType, CTypeSimple, IsCString, IsFreeFn, IsNullable, Malloced, NoAnnot, Ptr, UInt8T } from '../c/types'


export type I32 = CTypeSimple<"int32_t", number>
export type I64 = CTypeSimple<"int64_t", bigint>


export type PtrWrapper<T extends CType> = CategoryWrapper<Ptr<T>, 'ptr'>
type JSEnumWrapper = number 
export type EnumWrapper<
	CName extends string,
	UnderlyingType extends CType,
> = CategoryWrapper<CEnum<JSEnumWrapper, CName, UnderlyingType>, 'enum'>



export interface GeneratedExportedFunctions {
	calloc: (p_0: I32, p_1: I32) => PtrWrapper<I32>
	realloc: (p_0: I32, p_1: I32) => PtrWrapper<I32>
	free: (p_0: I32) => Annotated<void, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, IsFreeFn, NoAnnot<"nullable">>>
	qsort: (p_0: I32, p_1: I32, p_2: I32, p_3: I32) => void
	bsearch: (p_0: I32, p_1: I32, p_2: I32, p_3: I32, p_4: I32) => PtrWrapper<I32>
	get_message: (p_0: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">, NoAnnot<"nullable">>>
	malloc: (p_0: I32) => PtrWrapper<I32>
	is_empty_pos: (p_0: I32) => I32
	is_empty_message_struct: (p_0: I32) => I32
	get_script_type_name: (p_0: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">, NoAnnot<"nullable">>>
	get_file_type_name: (p_0: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">, NoAnnot<"nullable">>>
	parse_ass: (p_0: I32, p_1: I32) => Annotated<PtrWrapper<I32>, Annotations<Malloced<"free_parse_result">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	validate_ass_result: (p_0: I32, p_1: I32, p_2: I32) => void
	parse_result_is_error: (p_0: I32) => I32
	get_diagnostics_from_result: (p_0: I32) => PtrWrapper<I32>
	parse_result_get_value: (p_0: I32) => PtrWrapper<I32>
	free_parse_result: (p_0: I32) => Annotated<void, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, IsFreeFn, NoAnnot<"nullable">>>
	validate_fonts_of_result: (p_0: I32, p_1: I32, p_2: I32, p_3: I32) => void
	parse_font_preset: (p_0: I32) => I32
	ass_parser_lib_version: () => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">, NoAnnot<"nullable">>>
	ass_parser_lib_commit_hash: () => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">, NoAnnot<"nullable">>>
	allocator_get_statistics: () => PtrWrapper<I32>
	allocator_statistics_get_free: (p_0: I32) => I64
	allocator_statistics_get_total: (p_0: I32) => I64
	allocator_statistics_get_used: (p_0: I32) => I64
	allocator_statistics_get_metadata: (p_0: I32) => I64
	source_from_string: (p_0: I32, p_1: I32) => Annotated<PtrWrapper<I32>, Annotations<Malloced<"free">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	default_parse_settings: () => Annotated<PtrWrapper<I32>, Annotations<Malloced<"free">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	set_settings_option: (p_0: I32, p_1: I32, p_2: I32) => void
	diagnostics_get_length: (p_0: I32) => I32
	diagnostics_get_at: (p_0: I32, p_1: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	get_message_from_entry: (p_0: I32) => Annotated<PtrWrapper<I32>, Annotations<Malloced<"free_message_struct">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	free_message_struct: (p_0: I32) => Annotated<void, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, IsFreeFn, NoAnnot<"nullable">>>
	diagnostic_get_file_pos: (p_0: I32) => PtrWrapper<I32>
	diagnostic_get_severity: (p_0: I32) => EnumWrapper<"DiagnosticSeverity", UInt8T>
	file_pos_get_line: (p_0: I32) => I32
	file_pos_get_column: (p_0: I32) => I32
	events_get_length: (p_0: I32) => I32
	events_get_at: (p_0: I32, p_1: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	styles_get_length: (p_0: I32) => I32
	styles_get_at: (p_0: I32, p_1: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	get_script_info_from_ass_result: (p_0: I32) => PtrWrapper<I32>
	get_styles_from_ass_result: (p_0: I32) => PtrWrapper<I32>
	get_events_from_ass_result: (p_0: I32) => PtrWrapper<I32>
	get_extra_sections_from_ass_result: (p_0: I32) => PtrWrapper<I32>
	get_file_props_from_ass_result: (p_0: I32) => PtrWrapper<I32>
	get_scaled_border_and_shadow_from_script_info: (p_0: I32) => I32
	get_script_type_from_script_info: (p_0: I32) => EnumWrapper<"ScriptType", UInt8T>
	get_wrap_style_from_script_info: (p_0: I32) => EnumWrapper<"WrapStyle", UInt8T>
	get_video_aspect_ratio_from_script_info: (p_0: I32) => I32
	get_video_zoom_from_script_info: (p_0: I32) => I32
	get_play_res_x_from_script_info: (p_0: I32) => I32
	get_play_res_y_from_script_info: (p_0: I32) => I32
	get_string_by_name_from_script_info: (p_0: I32, p_1: I32) => Annotated<PtrWrapper<I32>, Annotations<Malloced<"free">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	get_ass_color_from_ass_style: (p_0: I32, p_1: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	get_color_component_from_ass_color: (p_0: I32, p_1: I32) => I32
	get_file_type_from_file_props: (p_0: I32) => I32
	get_line_type_from_file_props: (p_0: I32) => I32
	get_entry_from_name_in_extra_sections: (p_0: I32, p_1: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	get_entry_from_name_in_extra_section_entry: (p_0: I32, p_1: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	extra_sections_hm_get_at: (p_0: I32, p_1: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	extra_sections_hm_get_length: (p_0: I32) => I32
	extra_section_entry_hm_get_at: (p_0: I32, p_1: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	extra_section_entry_hm_get_length: (p_0: I32) => I32
	extra_sections_hm_entry_get_key: (p_0: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">, NoAnnot<"nullable">>>
	extra_sections_hm_entry_get_value: (p_0: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	extra_section_entry_hm_entry_get_key: (p_0: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">, NoAnnot<"nullable">>>
	extra_section_entry_hm_entry_get_value: (p_0: I32) => Annotated<PtrWrapper<I32>, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	_initialize: () => void
}
export type GeneratedExportedFunctionKeys = ["calloc",
	"realloc",
	"free",
	"qsort",
	"bsearch",
	"get_message",
	"malloc",
	"is_empty_pos",
	"is_empty_message_struct",
	"get_script_type_name",
	"get_file_type_name",
	"parse_ass",
	"validate_ass_result",
	"parse_result_is_error",
	"get_diagnostics_from_result",
	"parse_result_get_value",
	"free_parse_result",
	"validate_fonts_of_result",
	"parse_font_preset",
	"ass_parser_lib_version",
	"ass_parser_lib_commit_hash",
	"allocator_get_statistics",
	"allocator_statistics_get_free",
	"allocator_statistics_get_total",
	"allocator_statistics_get_used",
	"allocator_statistics_get_metadata",
	"source_from_string",
	"default_parse_settings",
	"set_settings_option",
	"diagnostics_get_length",
	"diagnostics_get_at",
	"get_message_from_entry",
	"free_message_struct",
	"diagnostic_get_file_pos",
	"diagnostic_get_severity",
	"file_pos_get_line",
	"file_pos_get_column",
	"events_get_length",
	"events_get_at",
	"styles_get_length",
	"styles_get_at",
	"get_script_info_from_ass_result",
	"get_styles_from_ass_result",
	"get_events_from_ass_result",
	"get_extra_sections_from_ass_result",
	"get_file_props_from_ass_result",
	"get_scaled_border_and_shadow_from_script_info",
	"get_script_type_from_script_info",
	"get_wrap_style_from_script_info",
	"get_video_aspect_ratio_from_script_info",
	"get_video_zoom_from_script_info",
	"get_play_res_x_from_script_info",
	"get_play_res_y_from_script_info",
	"get_string_by_name_from_script_info",
	"get_ass_color_from_ass_style",
	"get_color_component_from_ass_color",
	"get_file_type_from_file_props",
	"get_line_type_from_file_props",
	"get_entry_from_name_in_extra_sections",
	"get_entry_from_name_in_extra_section_entry",
	"extra_sections_hm_get_at",
	"extra_sections_hm_get_length",
	"extra_section_entry_hm_get_at",
	"extra_section_entry_hm_get_length",
	"extra_sections_hm_entry_get_key",
	"extra_sections_hm_entry_get_value",
	"extra_section_entry_hm_entry_get_key",
	"extra_section_entry_hm_entry_get_value",
	"_initialize"
]
