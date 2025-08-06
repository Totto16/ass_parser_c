import {
	allocate_js_utf8_string,
	construct_ptr_error,
	cstr_by_ptr,
	FreeList,
	get_int,
	get_sized_ptr_from_memory,
	write_ptr_to_memory,
	write_size_t_to_memory,
	type Allocator,
	type Char,
	type Int,
	type MemBuf,
	type Ptr,
	type SizeT,
	type Void,
	type WasmStructRef,
	type UInt64T,
	type UInt64TJs,
	get_uint64_t,
	type Bool,
	get_bool,
	ptr_cast,
	make_string_from_array_buffer,
	type AreAllFunctionCFns,
	get_c_enum_from_enum,
	get_c_bool,
	c_bool_to_int,
	nullptr,
} from './c_helper'

import type { Expect } from 'type-testing'

declare const _AssParseResult_SYM: unique symbol

type AssParseResultC = typeof _AssParseResult_SYM

declare const _AssSource_SYM: unique symbol

type AssSource = typeof _AssSource_SYM

declare const _ParseSettings_SYM: unique symbol

type ParseSettingsC = typeof _ParseSettings_SYM

declare const _SettingsOption_SYM: unique symbol

type SettingsOptionC = typeof _SettingsOption_SYM

declare const _AllocatorStatistics_SYM: unique symbol

type AllocatorStatistics = typeof _AllocatorStatistics_SYM

declare const _Diagnostics_SYM: unique symbol

type DiagnosticsC = typeof _Diagnostics_SYM

declare const _AssResult_SYM: unique symbol

type AssResultC = typeof _AssResult_SYM

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

export type AllocatorStatisticsJS = AllocatorStatisticsImpl<UInt64TJs>

interface WASMExportsFn {
	parse_ass: (
		source: WasmStructRef<AssSource> | Ptr<AssSource>,
		settings: WasmStructRef<ParseSettingsC> | Ptr<ParseSettingsC>
	) => Ptr<AssParseResultC>
	source_from_string: (source: Ptr<Char>, len: SizeT) => Ptr<AssSource>
	default_parse_settings: () => Ptr<ParseSettingsC>
	set_settings_option: (
		ptr: Ptr<ParseSettingsC>,
		option: SettingsOptionC,
		value: Int
	) => void
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
	free_parse_result: (result: Ptr<AssParseResultC>) => void
	get_diagnostics_from_result: (
		result: Ptr<AssParseResultC>
	) => Ptr<DiagnosticsC>
	parse_result_is_error: (result: Ptr<AssParseResultC>) => Bool
	parse_result_get_value: (result: Ptr<AssParseResultC>) => Ptr<AssResultC>
	_initialize: () => void
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

export class WasmBinding {
	private wasm: WASM
	private memory: WebAssembly.Memory

	private log_prefix = '[ASS_PARSER] '
	private log_state: LogState = { state: 'empty', buffer: '', error: false }

	private constructor(wasm: WASM, memory: WebAssembly.Memory) {
		this.wasm = wasm
		this.memory = memory
	}

	private initialize(): void {
		this.wasm.instance.exports._initialize()
	}

	private get_memory_buffer(): MemBuf {
		return this.memory.buffer
	}

	private get_allocator(): Allocator {
		return this.wasm.instance.exports
	}

	// void platform_panic(const char* file_path, int line, const char* message);
	private platform_panic(
		file_path_ptr: Ptr<Char>,
		line: Int,
		message_ptr: Ptr<Char>
	): void {
		const buffer = this.get_memory_buffer()
		const file_path = cstr_by_ptr(buffer, file_path_ptr)
		const message = cstr_by_ptr(buffer, message_ptr)
		console.error(
			`${this.log_prefix}${file_path}:${get_int(line).toString()}: ${message}`
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

		const buffer = this.get_memory_buffer()
		const message = cstr_by_ptr(buffer, message_ptr)

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
		const buffer = this.get_memory_buffer()
		const allocator = this.get_allocator()

		const format = cstr_by_ptr(buffer, format_ptr)

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

			const data: Uint8Array = get_sized_ptr_from_memory(buffer, {
				data_ptr,
				len,
			})

			const string = decoder.decode(data)

			const allocated_string = allocate_js_utf8_string(
				buffer,
				allocator,
				string
			)

			write_ptr_to_memory(
				buffer,
				out_data_ptr,
				ptr_cast<Char, Void>(allocated_string.data_ptr)
			)

			write_size_t_to_memory(buffer, out_len_ptr, allocated_string.len)
		} catch (err) {
			const error_ptr = construct_ptr_error(
				buffer,
				allocator,
				(err as Error).message
			)

			write_ptr_to_memory(buffer, out_data_ptr, error_ptr.data_ptr)

			write_size_t_to_memory(buffer, out_len_ptr, error_ptr.len)

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
			this.wasm.instance.exports.allocator_get_statistics()

		const result: AllocatorStatisticsImpl<UInt64T> = {
			free: this.wasm.instance.exports.allocator_statistics_get_free(
				c_statictics
			),
			total: this.wasm.instance.exports.allocator_statistics_get_total(
				c_statictics
			),
			used: this.wasm.instance.exports.allocator_statistics_get_used(
				c_statictics
			),
			metadata:
				this.wasm.instance.exports.allocator_statistics_get_metadata(
					c_statictics
				),
		}

		const js_result: AllocatorStatisticsJS = {
			free: get_uint64_t(result.free),
			total: get_uint64_t(result.total),
			used: get_uint64_t(result.used),
			metadata: get_uint64_t(result.metadata),
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
			this.wasm.instance.exports.set_settings_option(
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
		const buffer = this.get_memory_buffer()
		const allocator = this.get_allocator()

		let ass_source: Ptr<AssSource>

		if (typeof source === 'string') {
			const source_string = allocate_js_utf8_string(
				buffer,
				allocator,
				source,
				true
			)

			ass_source = this.wasm.instance.exports.source_from_string(
				source_string.data_ptr,
				source_string.len
			)
		} else {
			const content = await source.arrayBuffer()

			const source_string = make_string_from_array_buffer(
				buffer,
				allocator,
				content
			)

			ass_source = this.wasm.instance.exports.source_from_string(
				source_string.data_ptr,
				source_string.len
			)
		}

		const parse_settings: Ptr<ParseSettingsC> =
			this.wasm.instance.exports.default_parse_settings()

		this.modify_parse_settings(parse_settings, settings)

		const result = this.wasm.instance.exports.parse_ass(
			ass_source,
			parse_settings
		)

		const freelist = new FreeList()

		freelist.add(
			ptr_cast<AssSource, Void>(ass_source),
			this.wasm.instance.exports.free
		)

		freelist.add(
			ptr_cast<ParseSettingsC, Void>(parse_settings),
			this.wasm.instance.exports.free
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
	private wasm: WASM
	private result: Ptr<AssParseResultC>

	constructor(wasm: WASM, result: Ptr<AssParseResultC>, freelist: FreeList) {
		super(freelist)

		this.wasm = wasm
		this.result = result

		freelist.add(result, this.wasm.instance.exports.free_parse_result)
	}

	private is_error_value: null | boolean = null

	public is_error(): boolean {
		this.assert_not_freed('result is a valid ptr')

		if (this.is_error_value !== null) {
			return this.is_error_value
		}

		this.is_error_value = get_bool(
			this.wasm.instance.exports.parse_result_is_error(this.result)
		)

		return this.is_error_value
	}

	public diagnostics(): Diagnostics {
		this.assert_not_freed('result is a valid ptr')

		const c_diagnostics =
			this.wasm.instance.exports.get_diagnostics_from_result(this.result)

		const diagnostics = new Diagnostics(this.wasm, c_diagnostics)

		return diagnostics
	}

	protected set_freed(): void {
		this.result = ptr_cast<Void, AssParseResultC>(nullptr())
	}
}

export class Diagnostics extends CDisposable {
	private wasm: WASM
	private diagnostics: Ptr<DiagnosticsC>

	constructor(wasm: WASM, diagnostics: Ptr<DiagnosticsC>) {
		super(new FreeList())
		this.wasm = wasm
		this.diagnostics = diagnostics
	}

	protected set_freed(): void {
		this.diagnostics = ptr_cast<Void, DiagnosticsC>(nullptr())
	}
}
