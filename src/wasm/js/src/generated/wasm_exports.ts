import type { CType, CTypeSimple } from '../c/types'


export type I32 = CTypeSimple<"int32_t", number>
export type I64 = CTypeSimple<"int64_t", bigint>

export type Annotated<C extends CType, A> = C & {
	readonly __annotated: A
}

export interface Malloced<F extends keyof ExportedFunctions> {
	readonly __call: F
}

export class MallocedDisposable<
	C extends CType,
	Fn extends keyof ExportedFunctions,
> implements Disposable
{
	private val: C
	private fn: (f: C) => void

	constructor(val: Annotated<C, Malloced<Fn>>, fn: (f: C) => void) {
		this.val = val
		this.fn = fn
	}

	[Symbol.dispose](): void {
		this.fn(this.val)
	}
}

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
	allocator_get_statistics: () => I32
	allocator_statistics_get_free: (p_0: I32) => I64
	allocator_statistics_get_total: (p_0: I32) => I64
	allocator_statistics_get_used: (p_0: I32) => I64
	allocator_statistics_get_metadata: (p_0: I32) => I64
	source_from_string: (p_0: I32, p_1: I32) => Annotated<I32, Malloced<"free">>
	default_parse_settings: () => Annotated<I32, Malloced<"free">>
	set_settings_option: (p_0: I32, p_1: I32, p_2: I32) => void
	diagnostics_get_length: (p_0: I32) => I32
	diagnostics_get_at: (p_0: I32, p_1: I32) => I32
	get_message_from_entry: (p_0: I32) => Annotated<I32, Malloced<"free">>
	diagnostic_get_file_pos: (p_0: I32) => I32
	diagnostic_get_severity: (p_0: I32) => I32
	file_pos_get_line: (p_0: I32) => I32
	file_pos_get_column: (p_0: I32) => I32
	events_get_length: (p_0: I32) => I32
	events_get_at: (p_0: I32, p_1: I32) => I32
	_initialize: () => void
}
