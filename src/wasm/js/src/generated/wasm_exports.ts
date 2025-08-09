interface GeneratedCType<Desc extends string, JSType> {
	readonly __marker: unique symbol
	readonly __type: JSType
	readonly __desc: Desc
}
	
export type GetJSTypeFromGeneratedCType<
	C extends GeneratedCType<string, unknown>,
> = C extends {
	readonly __type: infer JSType
}
	? JSType
	: never

export type I32 = GeneratedCType<"int32_t", number>
export type I64 = GeneratedCType<"int64_t", bigint>

export interface ExportedFunctions {
	free: (p_0: I32) => void
	free_message_struct: (p_0: I32) => void
	get_message: (p_0: I32) => I32
	malloc: (p_0: I32) => I32
	uu_encode: (p_0: I32, p_1: I32) => void
	uu_decode: (p_0: I32, p_1: I32) => void
	is_empty_message_struct: (p_0: I32) => I32
	get_script_type_name: (p_0: I32) => I32
	get_file_type_name: (p_0: I32) => I32
	parse_ass: (p_0: I32, p_1: I32) => I32
	validate_ass_result: (p_0: I32, p_1: I32, p_2: I32) => void
	parse_result_is_error: (p_0: I32) => I32
	get_diagnostics_from_result: (p_0: I32) => I32
	parse_result_get_value: (p_0: I32) => I32
	free_parse_result: (p_0: I32) => void
	validate_fonts_of_result: (p_0: I32, p_1: I32, p_2: I32, p_3: I32) => void
	parse_font_preset: (p_0: I32) => I32
	realloc: (p_0: I32, p_1: I32) => I32
	ass_parser_lib_version: () => I32
	ass_parser_lib_commit_hash: () => I32
	allocator_statistics_get_free: (p_0: I32) => I64
	allocator_statistics_get_total: (p_0: I32) => I64
	allocator_statistics_get_used: (p_0: I32) => I64
	allocator_statistics_get_metadata: (p_0: I32) => I64
	source_from_string: (p_0: I32, p_1: I32) => I32
	default_parse_settings: () => I32
	set_settings_option: (p_0: I32, p_1: I32, p_2: I32) => void
	diagnostics_get_length: (p_0: I32) => I32
	diagnostics_get_at: (p_0: I32, p_1: I32) => I32
	get_message_from_entry: (p_0: I32) => I32
	file_pos_get_line: (p_0: I32) => I32
	file_pos_get_column: (p_0: I32) => I32
	_initialize: () => void
}