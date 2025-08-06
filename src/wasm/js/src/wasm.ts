import {
	allocate_js_utf8_string,
	construct_ptr_error,
	cstr_by_ptr,
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
} from './c_helper'

declare const _AssParseResult_SYM: unique symbol

type AssParseResult = typeof _AssParseResult_SYM

declare const _AssSource_SYM: unique symbol

type AssSource = typeof _AssSource_SYM

declare const _ParseSettings_SYM: unique symbol

type ParseSettings = typeof _ParseSettings_SYM

declare const _AllocatorStatistics_SYM: unique symbol

type AllocatorStatistics = typeof _AllocatorStatistics_SYM

export interface ParseSettingsJS {
	todo: number
}

export interface AssParseResultJS {
	todo: number
}

interface AllocatorStatisticsImpl<A> {
	total: A
	free: A
	used: A
	metadata: A
}

export type AllocatorStatisticsJS = AllocatorStatisticsImpl<UInt64TJs>

interface WASMExports extends WebAssembly.Exports, Allocator {
	parse_ass: (
		source: WasmStructRef<AssSource> | Ptr<AssSource>,
		settings: WasmStructRef<ParseSettings> | Ptr<ParseSettings>
	) => Ptr<AssParseResult>
	source_from_string: (source: Ptr<Char>, len: SizeT) => Ptr<AssSource>
	default_parse_settings: () => Ptr<ParseSettings>
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
	free_parse_result: (result: Ptr<AssParseResult>) => void
	_initialize: () => void
}

interface TypedWasmEnv extends WebAssembly.ModuleImports {
	memory: WebAssembly.Memory
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

	public parse_ass(
		source: string,
		settings: ParseSettingsJS
	): AssParseResultJS {
		const buffer = this.get_memory_buffer()
		const allocator = this.get_allocator()

		//TODO: allocate bom
		const source_string = allocate_js_utf8_string(
			buffer,
			allocator,
			source,
			true
		)

		console.log(source_string)

		const ass_source: Ptr<AssSource> =
			this.wasm.instance.exports.source_from_string(
				source_string.data_ptr,
				source_string.len
			)

		console.log('ass_source', ass_source)
		const parse_settings: Ptr<ParseSettings> =
			this.wasm.instance.exports.default_parse_settings()

		const result = this.wasm.instance.exports.parse_ass(
			ass_source,
			parse_settings
		)

		//TODO
		console.log(result, settings)

		this.wasm.instance.exports.free_parse_result(result)

		return {
			todo: 0,
		}
	}
}
