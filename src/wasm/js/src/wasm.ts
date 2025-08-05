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
	memory: WebAssembly.Memory
	parse_ass: (
		source: WasmStructRef<AssSource>,
		settings: WasmStructRef<ParseSettings>
	) => Ptr<AssParseResult>
	source_from_string: (source: string) => WasmStructRef<AssSource>
	settings_from_js: (
		settings: ParseSettingsJS
	) => WasmStructRef<ParseSettings>
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
}

interface WASMInstance extends WebAssembly.Instance {
	readonly exports: WASMExports
}

interface WASM extends WebAssembly.WebAssemblyInstantiatedSource {
	instance: WASMInstance
}

export class WasmBinding {
	private wasm: WASM

	private log_prefix = '[ASS_PARSER] '

	private constructor(wasm: WASM) {
		this.wasm = wasm
	}

	private get_memory_buffer(): MemBuf {
		return this.wasm.instance.exports.memory.buffer
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
			`${this.log_prefix}${file_path}:${get_int(line).toString()}:${message}`
		)
		// TODO: WASM platform_panic() does not halt the game
	}
	// void platform_log(const char* message);
	private platform_log(message_ptr: Ptr<Char>): void {
		const buffer = this.get_memory_buffer()
		const message = cstr_by_ptr(buffer, message_ptr)
		console.log(this.log_prefix + message)
	}

	// void platform_error(const char* message);
	private platform_error(message_ptr: Ptr<Char>): void {
		const buffer = this.get_memory_buffer()
		const message = cstr_by_ptr(buffer, message_ptr)
		console.error(this.log_prefix + message)
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

			const data: ArrayBufferView = get_sized_ptr_from_memory(buffer, {
				data_ptr,
				len,
			})

			const string = decoder.decode(data)

			const allocated_string = allocate_js_utf8_string(
				buffer,
				allocator,
				string
			)

			write_ptr_to_memory(buffer, out_data_ptr, allocated_string.data_ptr)

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
		const numPages = 4
		const memory = new WebAssembly.Memory({ initial: numPages })

		const result: WASM = (await WebAssembly.instantiateStreaming(
			fetch(
				`${prefix + (prefix.endsWith('/') ? '' : '/')}ass_parser.wasm`
			),
			{
				env: {
					platform_panic: () => {
						wasm.platform_panic.bind(wasm)
					},
					platform_log: () => {
						wasm.platform_log.bind(wasm)
					},
					platform_error: () => {
						wasm.platform_error.bind(wasm)
					},
					platform_string_conversion: () => {
						wasm.platform_string_conversion.bind(wasm)
					},
					memory,
				},
			}
		)) as WASM

		const wasm = new WasmBinding(result)

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
		const ass_source: WasmStructRef<AssSource> =
			this.wasm.instance.exports.source_from_string(source)

		const parse_settings: WasmStructRef<ParseSettings> =
			this.wasm.instance.exports.settings_from_js(settings)

		const result = this.wasm.instance.exports.parse_ass(
			ass_source,
			parse_settings
		)

		//TODO
		console.log(result)

		return {
			todo: 0,
		}
	}
}
