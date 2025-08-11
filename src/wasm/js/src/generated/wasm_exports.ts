import type { Annotated, Annotations, CTypeSimple, IsCString, IsFreeFn, Malloced, NoAnnot, Void } from '../c/types'


export type I32 = CTypeSimple<"int32_t", number>
export type I64 = CTypeSimple<"int64_t", bigint>

export interface ExportedFunctions {
	free: (p_0: I32) => Annotated<Void, Annotations<NoAnnot, NoAnnot, IsFreeFn>>
	free_message_struct: (p_0: I32) => Void
	get_message: (p_0: I32) => Annotated<I32, Annotations<NoAnnot, IsCString, NoAnnot>>
	malloc: (p_0: I32) => I32
	uu_encode: (p_0: I32, p_1: I32) => Void
	uu_decode: (p_0: I32, p_1: I32) => Void
	is_empty_message_struct: (p_0: I32) => I32
	get_script_type_name: (p_0: I32) => Annotated<I32, Annotations<NoAnnot, IsCString, NoAnnot>>
	get_file_type_name: (p_0: I32) => Annotated<I32, Annotations<NoAnnot, IsCString, NoAnnot>>
	parse_ass: (p_0: I32, p_1: I32) => I32
	validate_ass_result: (p_0: I32, p_1: I32, p_2: I32) => Void
	parse_result_is_error: (p_0: I32) => I32
	get_diagnostics_from_result: (p_0: I32) => I32
	parse_result_get_value: (p_0: I32) => I32
	free_parse_result: (p_0: I32) => Annotated<Void, Annotations<NoAnnot, NoAnnot, IsFreeFn>>
	validate_fonts_of_result: (p_0: I32, p_1: I32, p_2: I32, p_3: I32) => Void
	parse_font_preset: (p_0: I32) => I32
	realloc: (p_0: I32, p_1: I32) => I32
	ass_parser_lib_version: () => Annotated<I32, Annotations<NoAnnot, IsCString, NoAnnot>>
	ass_parser_lib_commit_hash: () => Annotated<I32, Annotations<NoAnnot, IsCString, NoAnnot>>
	allocator_get_statistics: () => Annotated<I32, Annotations<Malloced<"free">, NoAnnot, NoAnnot>>
	allocator_statistics_get_free: (p_0: I32) => I64
	allocator_statistics_get_total: (p_0: I32) => I64
	allocator_statistics_get_used: (p_0: I32) => I64
	allocator_statistics_get_metadata: (p_0: I32) => I64
	source_from_string: (p_0: I32, p_1: I32) => Annotated<I32, Annotations<Malloced<"free">, NoAnnot, NoAnnot>>
	default_parse_settings: () => Annotated<I32, Annotations<Malloced<"free">, NoAnnot, NoAnnot>>
	set_settings_option: (p_0: I32, p_1: I32, p_2: I32) => Void
	diagnostics_get_length: (p_0: I32) => I32
	diagnostics_get_at: (p_0: I32, p_1: I32) => I32
	get_message_from_entry: (p_0: I32) => Annotated<I32, Annotations<Malloced<"free">, NoAnnot, NoAnnot>>
	diagnostic_get_file_pos: (p_0: I32) => I32
	diagnostic_get_severity: (p_0: I32) => I32
	file_pos_get_line: (p_0: I32) => I32
	file_pos_get_column: (p_0: I32) => I32
	events_get_length: (p_0: I32) => I32
	events_get_at: (p_0: I32, p_1: I32) => I32
	_initialize: () => Void
}
