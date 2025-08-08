import {
	allocate_js_utf8_string,
	construct_ptr_error,
	cstr_by_ptr,
	FreeList,
	get_sized_ptr_from_memory,
	write_ptr_to_memory,
	write_size_t_to_memory,
	type Allocator,
	ptr_cast,
	make_string_from_array_buffer,
	get_c_enum_from_enum,
	get_c_bool,
	c_bool_to_int,
	nullptr,
	CArrayGeneric,
	type MemBuf,
	get_value,
	get_bool,
} from './c/functions'

import type { Equal, Expect, NotEqual } from 'type-testing'
import type {
	AreAllFunctionCFns,
	Bool,
	CEnum,
	Char,
	CStruct,
	CType,
	GetJSTypeFromCType,
	Int,
	IsCType,
	Ptr,
	SizeT,
	UInt64T,
	UInt8T,
	Void,
	WasmStructRef,
} from './c/types'

type AssParseResultC = CStruct<'AssParseResult'>

type AssSource = CStruct<'AssSource'>

type ParseSettingsC = CStruct<'ParseSettings'>

type SettingsOptionC = CStruct<'SettingsOption'>

type AllocatorStatistics = CStruct<'AllocatorStatistics'>

type DiagnosticsC = CStruct<'Diagnostics'>

type DiagnosticC = CStruct<'Diagnostic'>

type _expect_0 = Expect<
	CheckIsCArrayType<DiagnosticsC, DiagnosticC, 'diagnostics'>
>

type AssResultC = CStruct<'AssResult'>

type MessageStructC = CStruct<'MessageStruct'>

type FilePosC = CStruct<'FilePos'>

export interface ScriptInfoStrictSettings {
	allow_duplicate_fields: boolean
	allow_missing_script_type: boolean
}

export interface StrictSettings {
	script_info: ScriptInfoStrictSettings
	allow_additional_fields: boolean
	allow_number_truncating: boolean
	allow_unrecognized_file_encoding: boolean
	allow_validation_errors: boolean
}

export enum FontPreset {
	'disabled' = 0,
	'strict-all',
	'strict',
	'moderate',
	'lenient',
}

export interface FontSettings {
	preset: FontPreset
}

export interface ValidateSettings {
	font_settings: FontSettings
	validate_styles: boolean
	validate_text: boolean
}

export interface ParseSettings {
	strict_settings: StrictSettings
	validate_settings: ValidateSettings
}

export enum SettingsOption {
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
	SettingsOption_Validate_validate_text,
}

interface AllocatorStatisticsImpl<A> {
	total: A
	free: A
	used: A
	metadata: A
}

export type AllocatorStatisticsJS = AllocatorStatisticsImpl<
	GetJSTypeFromCType<UInt64T>
>

interface WASMExportsFn {
	parse_ass: (
		source: WasmStructRef<AssSource> | Ptr<AssSource>,
		settings: WasmStructRef<ParseSettingsC> | Ptr<ParseSettingsC>
	) => Ptr<AssParseResultC>
	//
	source_from_string: (source: Ptr<Char>, len: SizeT) => Ptr<AssSource>
	//
	default_parse_settings: () => Ptr<ParseSettingsC>
	set_settings_option: (
		ptr: Ptr<ParseSettingsC>,
		option: SettingsOptionC,
		value: Int
	) => void
	//
	allocator_get_statistics: () => WasmStructRef<AllocatorStatistics>
	allocator_statistics_get_free: (
		statistics: WasmStructRef<AllocatorStatistics>
	) => UInt64T
	allocator_statistics_get_total: (
		statistics: WasmStructRef<AllocatorStatistics>
	) => UInt64T
	allocator_statistics_get_used: (
		statistics: WasmStructRef<AllocatorStatistics>
	) => UInt64T
	allocator_statistics_get_metadata: (
		statistics: WasmStructRef<AllocatorStatistics>
	) => UInt64T
	//
	free_parse_result: (result: Ptr<AssParseResultC>) => void
	//
	get_diagnostics_from_result: (
		result: Ptr<AssParseResultC>
	) => Ptr<DiagnosticsC>
	//
	parse_result_is_error: (result: Ptr<AssParseResultC>) => Bool
	parse_result_get_value: (result: Ptr<AssParseResultC>) => Ptr<AssResultC>
	//
	_initialize: () => void
	//
	diagnostics_get_length: (diagnostics: Ptr<DiagnosticsC>) => SizeT
	diagnostics_get_at: (
		diagnostics: Ptr<DiagnosticsC>,
		index: SizeT
	) => Ptr<DiagnosticC>
	//
	get_message_from_entry: (element: Ptr<DiagnosticC>) => Ptr<MessageStructC>
	diagnostic_get_file_pos: (element: Ptr<DiagnosticC>) => Ptr<FilePosC>
	diagnostic_get_severity: (element: Ptr<DiagnosticC>) => DiagnosticSeverityC
	//
	file_pos_get_line: (pos: Ptr<FilePosC>) => SizeT
	file_pos_get_column: (pos: Ptr<FilePosC>) => SizeT
	//
	is_empty_message_struct: (message: Ptr<MessageStructC>) => Bool
	get_message: (message: Ptr<MessageStructC>) => Ptr<Char>
	free_message_struct: (message: Ptr<MessageStructC>) => void
	//
	//TODO: just to check if the type function work correctly, with multiple matches
	test_get_length: (diagnostics: Ptr<Char>) => SizeT
	test_get_at: (diagnostics: Ptr<Char>, index: SizeT) => Ptr<SizeT>
}

type _expect0 = Expect<AreAllFunctionCFns<WASMExportsFn>>

interface WASMExports extends WebAssembly.Exports, Allocator, WASMExportsFn {}

interface TypedWasmFn {
	platform_panic: (
		file_path_ptr: Ptr<Char>,
		line: Int,
		message_ptr: Ptr<Char>
	) => void
	platform_log_start: (error: Bool) => void
	platform_log_add: (message_ptr: Ptr<Char>) => void
	platform_log_end: () => void
	platform_string_conversion: (
		data_ptr: Ptr<Void>,
		len: SizeT,
		format_ptr: Ptr<Char>,
		out_data_ptr: Ptr<Ptr<Void>>,
		out_len_ptr: Ptr<SizeT>
	) => void
}

type _expect1 = Expect<AreAllFunctionCFns<TypedWasmFn>>
interface TypedWasmEnv extends WebAssembly.ModuleImports, TypedWasmFn {
	memory: WebAssembly.Memory
}

interface WASMInstance extends WebAssembly.Instance {
	readonly exports: WASMExports
}

interface WASM extends WebAssembly.WebAssemblyInstantiatedSource {
	instance: WASMInstance
}

type LogStateState = 'empty' | 'processing'

interface LogState {
	state: LogStateState
	buffer: string
	error: boolean
}

type DeepPartial<T> = T extends (...args: unknown[]) => unknown
	? T
	: T extends (infer U)[]
		? DeepPartial<U>[]
		: T extends object
			? { [P in keyof T]?: DeepPartial<T[P]> }
			: T

class WASMWrapper {
	private wasm: WASM
	private memory: WebAssembly.Memory

	public constructor(wasm: WASM, memory: WebAssembly.Memory) {
		this.wasm = wasm
		this.memory = memory
	}

	public initialize(): void {
		this.wasm.instance.exports._initialize()
	}

	private get_memory_buffer(): MemBuf {
		return this.memory.buffer
	}

	private get_allocator(): Allocator {
		return this.wasm.instance.exports
	}

	public get buffer(): MemBuf {
		return this.get_memory_buffer()
	}

	public get allocator(): Allocator {
		return this.get_allocator()
	}

	public get functions(): WASMExportsFn {
		return this.wasm.instance.exports
	}
}

export class WasmBinding {
	private wasm: WASMWrapper

	private log_prefix = '[ASS_PARSER] '
	private log_state: LogState = { state: 'empty', buffer: '', error: false }

	private constructor(wasm: WASM, memory: WebAssembly.Memory) {
		this.wasm = new WASMWrapper(wasm, memory)
	}

	private initialize(): void {
		this.wasm.initialize()
	}

	// void platform_panic(const char* file_path, int line, const char* message);
	private platform_panic(
		file_path_ptr: Ptr<Char>,
		line: Int,
		message_ptr: Ptr<Char>
	): void {
		const file_path = cstr_by_ptr(this.wasm.buffer, file_path_ptr)
		const message = cstr_by_ptr(this.wasm.buffer, message_ptr)
		console.error(
			`${this.log_prefix}${file_path}:${get_value<Int>(line).toString()}: ${message}`
		)
	}

	// void platform_log_start(bool error)
	private platform_log_start(error: Bool): void {
		if (this.log_state.state != 'empty') {
			console.error('Invalid log_start call!')
			console.error(`Buffer had: ${this.log_state.buffer}`)
		}

		this.log_state.state = 'processing'
		this.log_state.buffer = ''
		this.log_state.error = get_bool(error)
	}

	// void platform_log_add(const char* message);
	private platform_log_add(message_ptr: Ptr<Char>): void {
		if (this.log_state.state == 'empty') {
			console.error('Invalid log_add call!')
			console.error(`Buffer had: ${this.log_state.buffer}`)
		}

		const message = cstr_by_ptr(this.wasm.buffer, message_ptr)

		this.log_state.buffer += message
	}

	// void platform_log_end(void);
	private platform_log_end(): void {
		if (this.log_state.state == 'empty') {
			console.error('Invalid log_end call!')
			console.error(`Buffer had: ${this.log_state.buffer}`)
		}

		if (this.log_state.error) {
			console.error(this.log_prefix + this.log_state.buffer)
		} else {
			console.log(this.log_prefix + this.log_state.buffer)
		}

		this.log_state.state = 'empty'
		this.log_state.buffer = ''
		this.log_state.error = false
	}

	// void platform_string_conversion(void* data, size_t len, const char* format, void** out_data, size_t* out_len);
	private platform_string_conversion(
		data_ptr: Ptr<Void>,
		len: SizeT,
		format_ptr: Ptr<Char>,
		out_data_ptr: Ptr<Ptr<Void>>,
		out_len_ptr: Ptr<SizeT>
	): void {
		const format = cstr_by_ptr(this.wasm.buffer, format_ptr)

		try {
			let decoder: TextDecoder

			// see https://developer.mozilla.org/de/docs/Web/API/Encoding_API/Encodings
			switch (format) {
				case 'UTF-16BE': {
					decoder = new TextDecoder('utf-16be')
					break
				}
				case 'UTF-16LE': {
					decoder = new TextDecoder('utf-16le')
					break
				}
				case 'UTF-32BE': {
					throw new Error('utf-32 be encoding not yet supported')
				}
				case 'UTF-32LE': {
					throw new Error('utf-32 le encoding not yet supported')
				}
				default: {
					throw new Error(`unrecognized format: ${format}`)
				}
			}

			const data: Uint8Array = get_sized_ptr_from_memory(
				this.wasm.buffer,
				{
					data_ptr,
					len,
				}
			)

			const string = decoder.decode(data)

			const allocated_string = allocate_js_utf8_string(
				this.wasm.buffer,
				this.wasm.allocator,
				string
			)

			write_ptr_to_memory(
				this.wasm.buffer,
				out_data_ptr,
				ptr_cast<Char, Void>(allocated_string.data_ptr)
			)

			write_size_t_to_memory(
				this.wasm.buffer,
				out_len_ptr,
				allocated_string.len
			)
		} catch (err) {
			const error_ptr = construct_ptr_error(
				this.wasm.buffer,
				this.wasm.allocator,
				(err as Error).message
			)

			write_ptr_to_memory(
				this.wasm.buffer,
				out_data_ptr,
				error_ptr.data_ptr
			)

			write_size_t_to_memory(this.wasm.buffer, out_len_ptr, error_ptr.len)

			return
		}
	}

	public static async getInstance(prefix: string): Promise<WasmBinding> {
		// for this sizes see in the meson.build file and the link arguments
		const memory = new WebAssembly.Memory({ initial: 8, maximum: 128 })

		const env: TypedWasmEnv = {
			platform_panic: (
				file_path_ptr: Ptr<Char>,
				line: Int,
				message_ptr: Ptr<Char>
			) => {
				wasm.platform_panic.call(wasm, file_path_ptr, line, message_ptr)
			},
			platform_log_start: (error: Bool): void => {
				wasm.platform_log_start.call(wasm, error)
			},
			platform_log_add: (message_ptr: Ptr<Char>): void => {
				wasm.platform_log_add.call(wasm, message_ptr)
			},
			platform_log_end: (): void => {
				wasm.platform_log_end.call(wasm)
			},
			platform_string_conversion: (
				data_ptr: Ptr<Void>,
				len: SizeT,
				format_ptr: Ptr<Char>,
				out_data_ptr: Ptr<Ptr<Void>>,
				out_len_ptr: Ptr<SizeT>
			): void => {
				wasm.platform_string_conversion.call(
					wasm,
					data_ptr,
					len,
					format_ptr,
					out_data_ptr,
					out_len_ptr
				)
			},
			memory,
		}

		const result: WASM = (await WebAssembly.instantiateStreaming(
			fetch(
				`${prefix + (prefix.endsWith('/') ? '' : '/')}ass_parser.wasm`
			),
			{
				env,
			}
		)) as WASM

		const wasm = new WasmBinding(result, memory)

		wasm.initialize()

		return wasm
	}

	public allocator_get_statistics(): AllocatorStatisticsJS {
		const c_statictics: WasmStructRef<AllocatorStatistics> =
			this.wasm.functions.allocator_get_statistics()

		const result: AllocatorStatisticsImpl<UInt64T> = {
			free: this.wasm.functions.allocator_statistics_get_free(
				c_statictics
			),
			total: this.wasm.functions.allocator_statistics_get_total(
				c_statictics
			),
			used: this.wasm.functions.allocator_statistics_get_used(
				c_statictics
			),
			metadata:
				this.wasm.functions.allocator_statistics_get_metadata(
					c_statictics
				),
		}

		const js_result: AllocatorStatisticsJS = {
			free: get_value<UInt64T>(result.free),
			total: get_value<UInt64T>(result.total),
			used: get_value<UInt64T>(result.used),
			metadata: get_value<UInt64T>(result.metadata),
		}

		return js_result
	}

	private modify_parse_settings(
		ptr: Ptr<ParseSettingsC>,
		settings: DeepPartial<ParseSettings>
	): void {
		const SET_OPS: SettingsOption[] = [
			SettingsOption.SettingsOption_Strict_Script_allow_duplicate_fields,
			SettingsOption.SettingsOption_Strict_Script_allow_missing_script_type,
			SettingsOption.SettingsOption_Strict_allow_additional_fields,
			SettingsOption.SettingsOption_Strict_allow_number_truncating,
			SettingsOption.SettingsOption_Strict_allow_unrecognized_file_encoding,
			SettingsOption.SettingsOption_Strict_allow_validation_errors,
			SettingsOption.SettingsOption_Validate_Font_preset,
			SettingsOption.SettingsOption_Validate_validate_styles,
			SettingsOption.SettingsOption_Validate_validate_text,
		]

		type SetCommand = [option: SettingsOptionC, value: Int]

		function get_command_data(op: SettingsOption): SetCommand | undefined {
			const opV: SettingsOptionC = get_c_enum_from_enum<
				SettingsOption,
				SettingsOptionC
			>(op)

			switch (op) {
				case SettingsOption.SettingsOption_Strict_Script_allow_duplicate_fields: {
					if (
						settings.strict_settings?.script_info
							?.allow_duplicate_fields === undefined
					) {
						return undefined
					}

					return [
						opV,
						c_bool_to_int(
							get_c_bool(
								settings.strict_settings.script_info
									.allow_duplicate_fields
							)
						),
					]
				}
				case SettingsOption.SettingsOption_Strict_Script_allow_missing_script_type: {
					if (
						settings.strict_settings?.script_info
							?.allow_missing_script_type === undefined
					) {
						return undefined
					}

					return [
						opV,
						c_bool_to_int(
							get_c_bool(
								settings.strict_settings.script_info
									.allow_missing_script_type
							)
						),
					]
				}
				case SettingsOption.SettingsOption_Strict_allow_additional_fields: {
					if (
						settings.strict_settings?.allow_additional_fields ===
						undefined
					) {
						return undefined
					}

					return [
						opV,
						c_bool_to_int(
							get_c_bool(
								settings.strict_settings.allow_additional_fields
							)
						),
					]
				}
				case SettingsOption.SettingsOption_Strict_allow_number_truncating: {
					if (
						settings.strict_settings?.allow_number_truncating ===
						undefined
					) {
						return undefined
					}

					return [
						opV,
						c_bool_to_int(
							get_c_bool(
								settings.strict_settings.allow_number_truncating
							)
						),
					]
				}
				case SettingsOption.SettingsOption_Strict_allow_unrecognized_file_encoding: {
					if (
						settings.strict_settings
							?.allow_unrecognized_file_encoding === undefined
					) {
						return undefined
					}

					return [
						opV,
						c_bool_to_int(
							get_c_bool(
								settings.strict_settings
									.allow_unrecognized_file_encoding
							)
						),
					]
				}
				case SettingsOption.SettingsOption_Strict_allow_validation_errors: {
					if (
						settings.strict_settings?.allow_validation_errors ===
						undefined
					) {
						return undefined
					}

					return [
						opV,
						c_bool_to_int(
							get_c_bool(
								settings.strict_settings.allow_validation_errors
							)
						),
					]
				}
				case SettingsOption.SettingsOption_Validate_Font_preset: {
					if (
						settings.validate_settings?.font_settings?.preset ===
						undefined
					) {
						return undefined
					}

					return [
						opV,
						get_c_enum_from_enum<FontPreset, Int>(
							settings.validate_settings.font_settings.preset
						),
					]
				}
				case SettingsOption.SettingsOption_Validate_validate_styles: {
					if (
						settings.validate_settings?.validate_styles ===
						undefined
					) {
						return undefined
					}

					return [
						opV,
						c_bool_to_int(
							get_c_bool(
								settings.validate_settings.validate_styles
							)
						),
					]
				}
				case SettingsOption.SettingsOption_Validate_validate_text: {
					if (
						settings.validate_settings?.validate_text === undefined
					) {
						return undefined
					}

					return [
						opV,
						c_bool_to_int(
							get_c_bool(settings.validate_settings.validate_text)
						),
					]
				}
				default:
					throw new Error('Implementation error')
			}
		}

		const set_commands: SetCommand[] = SET_OPS.map((op) =>
			get_command_data(op)
		).filter((f) => f != undefined)

		for (const set_command of set_commands) {
			this.wasm.functions.set_settings_option(
				ptr,
				set_command[0],
				set_command[1]
			)
		}
	}

	public async parse_ass(
		source: string | File,
		settings: DeepPartial<ParseSettings>
	): Promise<AssParseResult> {
		let ass_source: Ptr<AssSource>

		if (typeof source === 'string') {
			const source_string = allocate_js_utf8_string(
				this.wasm.buffer,
				this.wasm.allocator,
				source,
				true
			)

			ass_source = this.wasm.functions.source_from_string(
				source_string.data_ptr,
				source_string.len
			)
		} else {
			const content = await source.arrayBuffer()

			const source_string = make_string_from_array_buffer(
				this.wasm.buffer,
				this.wasm.allocator,
				content
			)

			ass_source = this.wasm.functions.source_from_string(
				source_string.data_ptr,
				source_string.len
			)
		}

		const parse_settings: Ptr<ParseSettingsC> =
			this.wasm.functions.default_parse_settings()

		this.modify_parse_settings(parse_settings, settings)

		const result = this.wasm.functions.parse_ass(ass_source, parse_settings)

		const freelist = new FreeList()

		freelist.add(
			ptr_cast<AssSource, Void>(ass_source),
			this.wasm.allocator.free
		)

		freelist.add(
			ptr_cast<ParseSettingsC, Void>(parse_settings),
			this.wasm.allocator.free
		)

		const ass_result = new AssParseResult(this.wasm, result, freelist)

		return ass_result
	}
}

abstract class CDisposable implements Disposable {
	protected freelist: FreeList

	constructor(freelist: FreeList) {
		this.freelist = freelist
	}

	protected abstract set_freed(): void

	private is_free = false

	protected assert_not_freed(message = ''): void {
		if (this.is_free) {
			throw new Error(
				`Tried to operate on freed value:${message ? ` ${message}` : ''}`
			)
		}
	}

	public free(): void {
		if (!this.is_free) {
			this.freelist.free()

			this.is_free = true

			this.set_freed()
		}
	}

	[Symbol.dispose](): void {
		this.free()
	}
}

export class AssParseResult extends CDisposable {
	private wasm: WASMWrapper
	private result: Ptr<AssParseResultC>

	constructor(
		wasm: WASMWrapper,
		result: Ptr<AssParseResultC>,
		freelist: FreeList
	) {
		super(freelist)

		this.wasm = wasm
		this.result = result

		freelist.add(result, this.wasm.functions.free_parse_result)
	}

	private is_error_value: null | boolean = null

	public is_error(): boolean {
		this.assert_not_freed('result is a valid ptr')

		if (this.is_error_value !== null) {
			return this.is_error_value
		}

		this.is_error_value = get_bool(
			this.wasm.functions.parse_result_is_error(this.result)
		)

		return this.is_error_value
	}

	public diagnostics(): Diagnostics {
		this.assert_not_freed('result is a valid ptr')

		const c_diagnostics = this.wasm.functions.get_diagnostics_from_result(
			this.result
		)

		const diagnostics = new Diagnostics(this.wasm, c_diagnostics)

		return diagnostics
	}

	protected set_freed(): void {
		this.result = ptr_cast<Void, AssParseResultC>(nullptr())
	}
}

export interface FilePos {
	line: number
	column: number
}

export type DiagnosticSeverity = 'warning' | 'error'

enum DiagnosticSeverityCEnum {
	'warning' = 0,
	'error',
}

type DiagnosticSeverityC = CEnum<
	DiagnosticSeverityCEnum,
	'DiagnosticSeverity',
	UInt8T
>

export interface Diagnostic {
	message: string
	severity: DiagnosticSeverity
	position: FilePos
}

type ParamsOf<F> = F extends (...args: infer Args) => infer Rest
	? [Args, Rest]
	: never

type ValidLengthArrayFn<F> =
	ParamsOf<F> extends [args: infer Args, infer Rest]
		? Rest extends SizeT
			? Args extends [infer A]
				? A extends Ptr<infer C>
					? IsCType<C> extends true
						? [true, C]
						: [false, 'Ptr without CTpye as first argument']
					: [false, 'no ptr as first argument', { o: A }]
				: [false, 'args amount invalid']
			: [false, 'invalid return type']
		: [false, 'no fn']

type GetLengthFnsImpl<T> = {
	[K in keyof T]: K extends `${infer Start}_get_length`
		? [Start, ValidLengthArrayFn<T[K]>]
		: never
}[keyof T]

type ValidGetArrayFn<F> =
	ParamsOf<F> extends [args: infer Args, infer Rest]
		? Args extends [infer A, infer B]
			? A extends Ptr<infer C>
				? IsCType<C> extends true
					? B extends SizeT
						? Rest extends Ptr<infer D>
							? IsCType<D> extends true
								? [true, [C, D]]
								: [false, 'Ptr without CTpye as return type']
							: [false, 'no ptr as return type']
						: [false, 'invalid second argument type']
					: [false, 'Ptr without CTpye as first argument']
				: [false, 'no ptr as first argument']
			: [false, 'args amount invalid']
		: [false, 'no fn']

type GetIndexFnsImpl<T> = {
	[K in keyof T]: K extends `${infer Start}_get_at`
		? [Start, ValidGetArrayFn<T[K]>]
		: never
}[keyof T]

type ExtractSimpleValue<T> = T extends [infer First, rest: unknown]
	? First
	: never

type GetIndexFnsSimple<A> = ExtractSimpleValue<GetIndexFnsImpl<A>>

type GetLengthFnsSimple<A> = ExtractSimpleValue<GetLengthFnsImpl<A>>

type GetListTypeFromLength<A> = A extends [start: unknown, infer Res]
	? Res extends [true, infer Elem]
		? Elem
		: Res extends [false, ...errs: infer Errs]
			? [`length error (b)`, ...Errs]
			: 'length error: not a valid input - 2'
	: 'length error: not a valid input - 1'

type GetListTypeFromIdx<A> = A extends [start: unknown, infer Res]
	? Res extends [true, infer Elem]
		? Elem extends [infer Elem1, elem2: unknown]
			? Elem1
			: 'idx error: not a valid input - 3'
		: Res extends [false, ...errs: infer Errs]
			? [`idx error (b)`, ...Errs]
			: 'idx error: not a valid input - 2'
	: 'idx error: not a valid input - 1'

type Intersection<T, U> = T extends U ? T : never

type ValidateArrayFns<A, B> =
	Equal<GetListTypeFromLength<A>, GetListTypeFromIdx<B>> extends true
		? Intersection<ExtractSimpleValue<A>, ExtractSimpleValue<B>>
		: [never, GetListTypeFromLength<A>, GetListTypeFromIdx<B>]

type ArrayAccessFnsOfImpl<O> = ValidateArrayFns<
	GetLengthFnsImpl<O>,
	GetIndexFnsImpl<O>
>

type ArrayAccessFnsOf<O> = ArrayAccessFnsOfImpl<O>

type _LengthFns = GetLengthFnsSimple<WASMExportsFn>

type _IndexFns = GetIndexFnsSimple<WASMExportsFn>

type _expect2 = Expect<Equal<_LengthFns, _IndexFns>>

interface Test1Fns {
	abcs_get_length: (a: Ptr<Void>) => SizeT
	abcs_get_at: (a: Ptr<Char>, index: SizeT) => Ptr<SizeT>
}

type _expectcheck_0_0 = Expect<Equal<GetIndexFnsSimple<Test1Fns>, 'abcs'>>

type Test1FnsRes1 = ArrayAccessFnsOf<Test1Fns>

type _expectcheck_0_1 = Expect<NotEqual<IsValidCListImpl<Test1FnsRes1>, true>>

type _expectcheck_0_2 = Expect<Equal<Test1FnsRes1, [never, Void, Char]>>

type TestStruct = CStruct<'TestStructFormFns'>

interface Test2Fns {
	abcs_get_length: (a: Ptr<Void>) => SizeT
	abcs_get_at: (a: Ptr<Void>, index: SizeT) => Ptr<SizeT>

	abcd_get_length: (a: Ptr<TestStruct>) => SizeT
	abcd_get_at: (a: Ptr<TestStruct>, index: SizeT) => Ptr<SizeT>
}

type _expectcheck_1_0 = Expect<
	Equal<GetIndexFnsSimple<Test2Fns>, 'abcs' | 'abcd'>
>

type Test2FnsRes1 = ArrayAccessFnsOf<Test2Fns>

type _expectcheck_1_1 = Expect<Equal<IsValidCListImpl<Test2FnsRes1>, true>>

type _expectcheck_1_2 = Expect<Equal<Test2FnsRes1, 'abcs' | 'abcd'>>

type ExportedCListAccessFns = ArrayAccessFnsOf<WASMExportsFn>

type IsValidCListImpl<A> = A extends [never, ...args: unknown[]] ? false : true

type _expect3 = Expect<IsValidCListImpl<ExportedCListAccessFns>>

type CheckIsCArrayType<
	List extends CType,
	Element extends CType,
	T extends ExportedCListAccessFns,
> = TypesFromFromIndexFnImpl<T> extends [List, Element] ? true : false

type FindElementWhereForFn<A, Elem> = A extends [
	infer Elem1,
	...args: infer ResArgs,
]
	? Equal<Elem1, Elem> extends true
		? ResArgs
		: [never, 'error 2', Elem, Elem1]
	: [never, 'error 1', A]

type DEBUG_TYPE = false

type NeverError<MSG> = DEBUG_TYPE extends true ? [never, MSG] : never

type TypesFromFromLengthFnImpl<C extends ExportedCListAccessFns> =
	FindElementWhereForFn<GetLengthFnsImpl<WASMExportsFn>, C> extends infer Temp
		? Temp extends [never, ...args: unknown[]]
			? never
			: Temp extends [[true, infer Res]]
				? Res
				: NeverError<'error 2'>
		: NeverError<'error 1'>

type ListTypeFromFn<C extends ExportedCListAccessFns> =
	| TypesFromFromLengthFnImpl<C>
	| TypesFromFromIndexFnImpl<C>[0]

type _expect4 = Expect<
	Equal<
		TypesFromFromLengthFnImpl<'diagnostics'>,
		TypesFromFromIndexFnImpl<'diagnostics'>[0]
	>
>

type _expect4_1 = Expect<
	Equal<
		TypesFromFromLengthFnImpl<'test'>,
		TypesFromFromIndexFnImpl<'test'>[0]
	>
>

type TypesFromFromIndexFnImpl<C extends ExportedCListAccessFns> =
	FindElementWhereForFn<GetIndexFnsImpl<WASMExportsFn>, C> extends infer Temp
		? Temp extends [never, ...args: unknown[]]
			? never
			: Temp extends [[true, infer Res]]
				? Res
				: NeverError<'error 2'>
		: NeverError<'error 1'>

type ElementTypeFrom<C extends ExportedCListAccessFns> =
	TypesFromFromIndexFnImpl<C>[1]

export abstract class CArray<
	JsElement,
	CFuncLit extends ExportedCListAccessFns,
	ListType extends ListTypeFromFn<CFuncLit> = ListTypeFromFn<CFuncLit>,
	ElementType extends ElementTypeFrom<CFuncLit> = ElementTypeFrom<CFuncLit>,
> extends CArrayGeneric<JsElement, ListType, ElementType> {
	protected wasm: WASMWrapper
	private c_func_lit: CFuncLit

	constructor(
		wasm: WASMWrapper,
		underlying_type: Ptr<ListType>,
		c_func_lit: CFuncLit
	) {
		super(underlying_type)

		this.wasm = wasm
		this.c_func_lit = c_func_lit
	}

	protected override length_of_impl(underlying_type: Ptr<ListType>): SizeT {
		const fn: WASMExportsFn[`${CFuncLit}_get_length`] =
			this.wasm.functions[`${this.c_func_lit}_get_length`]

		return fn(underlying_type)
	}

	protected override element_get_at_impl(
		underlying_type: Ptr<ListType>,
		index: SizeT
	): Ptr<ElementType> {
		const fn: WASMExportsFn[`${CFuncLit}_get_at`] =
			this.wasm.functions[`${this.c_func_lit}_get_at`]

		return fn(underlying_type, index) as ElementType
	}
}

export class Diagnostics extends CArray<Diagnostic, 'diagnostics'> {
	constructor(wasm: WASMWrapper, diagnostics: Ptr<DiagnosticsC>) {
		super(wasm, diagnostics, 'diagnostics')
	}

	private get_file_pos_from_c(file_pos_c: Ptr<FilePosC>): FilePos {
		const line: number = get_value<SizeT>(
			this.wasm.functions.file_pos_get_line(file_pos_c)
		)
		const column: number = get_value<SizeT>(
			this.wasm.functions.file_pos_get_column(file_pos_c)
		)

		return { line, column }
	}

	private get_severity_from_c(sev: DiagnosticSeverityC): DiagnosticSeverity {
		const severity: DiagnosticSeverityCEnum = get_value(sev)

		switch (severity) {
			case DiagnosticSeverityCEnum.warning:
				return 'warning'
			case DiagnosticSeverityCEnum.error:
				return 'error'
			default:
				throw new Error('Implementation error')
		}
	}

	private get_string_from_message_struct(
		message_ptr: Ptr<MessageStructC>
	): string {
		const is_empty = get_bool(
			this.wasm.functions.is_empty_message_struct(message_ptr)
		)

		if (is_empty) {
			throw new Error('MessageStruct is empty')
		}

		const str_ptr = this.wasm.functions.get_message(message_ptr)

		const result = cstr_by_ptr(this.wasm.buffer, str_ptr)

		this.wasm.functions.free_message_struct(message_ptr)

		return result
	}

	protected override convert_element_from_c_to_js(
		element: Ptr<DiagnosticC>
	): Diagnostic {
		const message_ptr = this.wasm.functions.get_message_from_entry(element)

		const message = this.get_string_from_message_struct(message_ptr)

		const file_pos_ptr =
			this.wasm.functions.diagnostic_get_file_pos(element)

		const position: FilePos = this.get_file_pos_from_c(file_pos_ptr)

		const severity_c = this.wasm.functions.diagnostic_get_severity(element)

		const severity = this.get_severity_from_c(severity_c)

		return { message, position, severity }
	}
}
