import type { Annotated, Annotations, CTypeSimple, IsCString, IsFreeFn, Malloced, NoAnnot } from '../c/types'


export type I32 = CTypeSimple<"int32_t", number>
export type I64 = CTypeSimple<"int64_t", bigint>

export interface GeneratedExportedFunctions {
	free: (p_0: I32) => Annotated<void, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, IsFreeFn>>
	free_message_struct: (p_0: I32) => Annotated<void, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, IsFreeFn>>
	get_message: (p_0: I32) => Annotated<I32, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">>>
	malloc: (p_0: I32) => I32
	uu_encode: (p_0: I32, p_1: I32) => void
	uu_decode: (p_0: I32, p_1: I32) => void
	is_empty_message_struct: (p_0: I32) => I32
	get_script_type_name: (p_0: I32) => Annotated<I32, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">>>
	get_file_type_name: (p_0: I32) => Annotated<I32, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">>>
	parse_ass: (p_0: I32, p_1: I32) => Annotated<I32, Annotations<Malloced<"free_parse_result">, NoAnnot<"cstr">, NoAnnot<"free_fn">>>
	validate_ass_result: (p_0: I32, p_1: I32, p_2: I32) => void
	parse_result_is_error: (p_0: I32) => I32
	get_diagnostics_from_result: (p_0: I32) => I32
	parse_result_get_value: (p_0: I32) => I32
	free_parse_result: (p_0: I32) => Annotated<void, Annotations<NoAnnot<"malloced">, NoAnnot<"cstr">, IsFreeFn>>
	validate_fonts_of_result: (p_0: I32, p_1: I32, p_2: I32, p_3: I32) => void
	parse_font_preset: (p_0: I32) => I32
	realloc: (p_0: I32, p_1: I32) => I32
	ass_parser_lib_version: () => Annotated<I32, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">>>
	ass_parser_lib_commit_hash: () => Annotated<I32, Annotations<NoAnnot<"malloced">, IsCString, NoAnnot<"free_fn">>>
	allocator_get_statistics: () => I32
	allocator_statistics_get_free: (p_0: I32) => I64
	allocator_statistics_get_total: (p_0: I32) => I64
	allocator_statistics_get_used: (p_0: I32) => I64
	allocator_statistics_get_metadata: (p_0: I32) => I64
	source_from_string: (p_0: I32, p_1: I32) => Annotated<I32, Annotations<Malloced<"free">, NoAnnot<"cstr">, NoAnnot<"free_fn">>>
	default_parse_settings: () => Annotated<I32, Annotations<Malloced<"free">, NoAnnot<"cstr">, NoAnnot<"free_fn">>>
	set_settings_option: (p_0: I32, p_1: I32, p_2: I32) => void
	diagnostics_get_length: (p_0: I32) => I32
	diagnostics_get_at: (p_0: I32, p_1: I32) => I32
	get_message_from_entry: (p_0: I32) => Annotated<I32, Annotations<Malloced<"free_message_struct">, NoAnnot<"cstr">, NoAnnot<"free_fn">>>
	diagnostic_get_file_pos: (p_0: I32) => I32
	diagnostic_get_severity: (p_0: I32) => I32
	file_pos_get_line: (p_0: I32) => I32
	file_pos_get_column: (p_0: I32) => I32
	events_get_length: (p_0: I32) => I32
	events_get_at: (p_0: I32, p_1: I32) => I32
	_initialize: () => void
}
