import type { Annotated, Annotations, CTypeSimple, IsCString, IsFreeFn, IsNullable, Malloced, NoAnnot } from '../c/types'


export type I32 = CTypeSimple<"int32_t", number>
export type I64 = CTypeSimple<"int64_t", bigint>

export interface GeneratedExportedFunctions {
	free: (p_0: I32) => Annotated<void, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, IsFreeFn, NoAnnot<"nullable">>>
	free_message_struct: (p_0: I32) => Annotated<void, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, IsFreeFn, NoAnnot<"nullable">>>
	get_message: (p_0: I32) => Annotated<I32, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">, NoAnnot<"nullable">>>
	malloc: (p_0: I32) => I32
	is_empty_pos: (p_0: I32) => I32
	uu_encode: (p_0: I32, p_1: I32) => void
	uu_decode: (p_0: I32, p_1: I32) => void
	is_empty_message_struct: (p_0: I32) => I32
	get_script_type_name: (p_0: I32) => Annotated<I32, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">, NoAnnot<"nullable">>>
	get_file_type_name: (p_0: I32) => Annotated<I32, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">, NoAnnot<"nullable">>>
	parse_ass: (p_0: I32, p_1: I32) => Annotated<I32, Annotations<Malloced<"free_parse_result">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	validate_ass_result: (p_0: I32, p_1: I32, p_2: I32) => void
	parse_result_is_error: (p_0: I32) => I32
	get_diagnostics_from_result: (p_0: I32) => I32
	parse_result_get_value: (p_0: I32) => I32
	free_parse_result: (p_0: I32) => Annotated<void, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, IsFreeFn, NoAnnot<"nullable">>>
	validate_fonts_of_result: (p_0: I32, p_1: I32, p_2: I32, p_3: I32) => void
	parse_font_preset: (p_0: I32) => I32
	realloc: (p_0: I32, p_1: I32) => I32
	ass_parser_lib_version: () => Annotated<I32, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">, NoAnnot<"nullable">>>
	ass_parser_lib_commit_hash: () => Annotated<I32, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">, NoAnnot<"nullable">>>
	allocator_get_statistics: () => I32
	allocator_statistics_get_free: (p_0: I32) => I64
	allocator_statistics_get_total: (p_0: I32) => I64
	allocator_statistics_get_used: (p_0: I32) => I64
	allocator_statistics_get_metadata: (p_0: I32) => I64
	source_from_string: (p_0: I32, p_1: I32) => Annotated<I32, Annotations<Malloced<"free">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	default_parse_settings: () => Annotated<I32, Annotations<Malloced<"free">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	set_settings_option: (p_0: I32, p_1: I32, p_2: I32) => void
	diagnostics_get_length: (p_0: I32) => I32
	diagnostics_get_at: (p_0: I32, p_1: I32) => Annotated<I32, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	get_message_from_entry: (p_0: I32) => Annotated<I32, Annotations<Malloced<"free_message_struct">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	diagnostic_get_file_pos: (p_0: I32) => I32
	diagnostic_get_severity: (p_0: I32) => I32
	file_pos_get_line: (p_0: I32) => I32
	file_pos_get_column: (p_0: I32) => I32
	events_get_length: (p_0: I32) => I32
	events_get_at: (p_0: I32, p_1: I32) => Annotated<I32, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	styles_get_length: (p_0: I32) => I32
	styles_get_at: (p_0: I32, p_1: I32) => Annotated<I32, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, NoAnnot<"free_fn">, IsNullable>>
	get_script_info_from_ass_result: (p_0: I32) => I32
	get_styles_from_ass_result: (p_0: I32) => I32
	get_events_from_ass_result: (p_0: I32) => I32
	_initialize: () => void
}
