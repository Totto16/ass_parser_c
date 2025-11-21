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
	uint8_t_to_int,
	enum_get_underlying_c_type,
	type WASMExportsFnFromLibC,
	type CStr,
	assert_not_null,
	is_not_null,
	get_ptr_value,
	get_enum_value,
	Unref,
} from './c/functions'

import type { Equal, Expect, NotEqual } from 'type-testing'
import {
	type NoAnnot,
	type Annotated,
	type Annotations,
	type AreAllFunctionCFns,
	type Bool,
	type CEnum,
	type Char,
	type CStruct,
	type CType,
	type GetAnnotations,
	type GetJSTypeFromCType,
	type GetJSTypeFromCTypeEnumSpecialCase,
	type Int,
	type IsCString,
	type IsCType,
	type Malloced,
	type Ptr,
	type SizeT,
	type UInt64T,
	type UInt8T,
	type IsFreeFn,
	MallocedDisposable,
	type UnionToTuple,
	type Void,
	type FreeFns,
	type AnnotationBase,
	type MallocedAnnotationWrapper,
	type IsNullable,
	type NullChecked,
} from './c/types'
import type { GeneratedExportedFunctions } from './generated/wasm_exports'

type AssParseResultC = CStruct<'AssParseResult'>

type AssSource = CStruct<'AssSource'>

type ParseSettingsC = CStruct<'ParseSettings'>

type SettingsOptionC = CEnum<SettingsOptionEnum, 'SettingsOption', UInt8T>

type AllocatorStatistics = CStruct<'AllocatorStatistics'>

type DiagnosticsC = CStruct<'Diagnostics'>

type DiagnosticC = CStruct<'Diagnostic'>

type _expect_0 = Expect<
	CheckIsCArrayType<DiagnosticsC, DiagnosticC, 'diagnostics'>
>

type AssEventsC = CStruct<'AssEvents'>

type AssEventC = CStruct<'AssEvent'>

type _expect_1 = Expect<CheckIsCArrayType<AssEventsC, AssEventC, 'events'>>

type AssStylesC = CStruct<'AssStyles'>

type AssStyleC = CStruct<'AssStyle'>

type _expect_2 = Expect<CheckIsCArrayType<AssStylesC, AssStyleC, 'styles'>>

type ScriptInfoC = CStruct<'ScriptInfo'>

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

type FontPresetC = CEnum<FontPreset, 'FontPreset', UInt8T>

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

export enum SettingsOptionEnum {
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

interface WASMExportsFnWithoutLibC {
	parse_ass: (
		source: Ptr<AssSource>,
		settings: Ptr<ParseSettingsC>
	) => Annotated<
		Ptr<AssParseResultC>,
		Annotations<
			Malloced<'free_parse_result'>,
			NoAnnot<'cstr'>,
			NoAnnot<'free_fn'>,
			IsNullable
		>
	>
	//
	source_from_string: (
		source: Ptr<Char>,
		len: SizeT
	) => Annotated<
		Ptr<AssSource>,
		Annotations<
			Malloced<'free'>,
			NoAnnot<'cstr'>,
			NoAnnot<'free_fn'>,
			IsNullable
		>
	>
	//
	default_parse_settings: () => Annotated<
		Ptr<ParseSettingsC>,
		Annotations<
			Malloced<'free'>,
			NoAnnot<'cstr'>,
			NoAnnot<'free_fn'>,
			IsNullable
		>
	>
	set_settings_option: (
		ptr: Ptr<ParseSettingsC>,
		option: SettingsOptionC,
		value: Int
	) => void
	//
	allocator_get_statistics: () => Ptr<AllocatorStatistics> // statically allocated

	allocator_statistics_get_free: (
		statistics: Ptr<AllocatorStatistics>
	) => UInt64T
	allocator_statistics_get_total: (
		statistics: Ptr<AllocatorStatistics>
	) => UInt64T
	allocator_statistics_get_used: (
		statistics: Ptr<AllocatorStatistics>
	) => UInt64T
	allocator_statistics_get_metadata: (
		statistics: Ptr<AllocatorStatistics>
	) => UInt64T
	//
	free_parse_result: (
		result: Annotated<
			Ptr<AssParseResultC>,
			Annotations<
				Malloced<'free_parse_result'>,
				NoAnnot<'cstr'>,
				NoAnnot<'free_fn'>,
				NoAnnot<'nullable'>
			>
		>
	) => Annotated<
		void,
		Annotations<
			NoAnnot<'malloced'>,
			NoAnnot<'cstr'>,
			IsFreeFn,
			NoAnnot<'nullable'>
		>
	>
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
	) => Annotated<
		Ptr<DiagnosticC>,
		Annotations<
			NoAnnot<'malloced'>,
			NoAnnot<'cstr'>,
			NoAnnot<'free_fn'>,
			IsNullable
		>
	>

	//
	get_message_from_entry: (
		element: Ptr<DiagnosticC>
	) => Annotated<
		Ptr<MessageStructC>,
		Annotations<
			Malloced<'free_message_struct'>,
			NoAnnot<'cstr'>,
			NoAnnot<'free_fn'>,
			IsNullable
		>
	>
	diagnostic_get_file_pos: (element: Ptr<DiagnosticC>) => Ptr<FilePosC>
	diagnostic_get_severity: (element: Ptr<DiagnosticC>) => DiagnosticSeverityC
	//
	file_pos_get_line: (pos: Ptr<FilePosC>) => SizeT
	file_pos_get_column: (pos: Ptr<FilePosC>) => SizeT
	is_empty_pos: (pos: Ptr<FilePosC>) => Bool
	//
	is_empty_message_struct: (message: Ptr<MessageStructC>) => Bool
	get_message: (
		message: Ptr<MessageStructC>
	) => Annotated<
		Ptr<Char>,
		Annotations<
			NoAnnot<'malloced'>,
			IsCString,
			NoAnnot<'free_fn'>,
			NoAnnot<'nullable'>
		>
	>
	free_message_struct: (
		message: Annotated<
			Ptr<MessageStructC>,
			Annotations<
				Malloced<'free_message_struct'>,
				NoAnnot<'cstr'>,
				NoAnnot<'free_fn'>,
				NoAnnot<'nullable'>
			>
		>
	) => Annotated<
		void,
		Annotations<
			NoAnnot<'malloced'>,
			NoAnnot<'cstr'>,
			IsFreeFn,
			NoAnnot<'nullable'>
		>
	>
	//
	events_get_length: (events: Ptr<AssEventsC>) => SizeT
	events_get_at: (
		events: Ptr<AssEventsC>,
		index: SizeT
	) => Annotated<
		Ptr<AssEventC>,
		Annotations<
			NoAnnot<'malloced'>,
			NoAnnot<'cstr'>,
			NoAnnot<'free_fn'>,
			IsNullable
		>
	>
	//
	styles_get_length: (events: Ptr<AssStylesC>) => SizeT
	styles_get_at: (
		events: Ptr<AssStylesC>,
		index: SizeT
	) => Annotated<
		Ptr<AssStyleC>,
		Annotations<
			NoAnnot<'malloced'>,
			NoAnnot<'cstr'>,
			NoAnnot<'free_fn'>,
			IsNullable
		>
	>
	//
	get_script_info_from_ass_result: (
		ass_result: Ptr<AssResultC>
	) => Ptr<ScriptInfoC>
	get_styles_from_ass_result: (ass_result: Ptr<AssResultC>) => Ptr<AssStylesC>
	get_events_from_ass_result: (ass_result: Ptr<AssResultC>) => Ptr<AssEventsC>
	//
}

type _NotCFuncsExportedCFunctions = AreAllFunctionCFns<ExportedCFunctions>

type _expect0 = Expect<Equal<_NotCFuncsExportedCFunctions, []>>

type _NotCFuncsGeneratedExportedFunctions =
	AreAllFunctionCFns<GeneratedExportedFunctions>

type _expect1_1 = Expect<Equal<_NotCFuncsGeneratedExportedFunctions, []>>

type GetJSTypeFromCTypeArr<A extends CType[]> = A extends []
	? []
	: A extends [infer First extends CType, ...infer Rest extends CType[]]
		? [
				GetJSTypeFromCTypeEnumSpecialCase<First>,
				...GetJSTypeFromCTypeArr<Rest>,
			]
		: never

interface ExportEntryImpl<Key, Args, Ret, AN> {
	readonly key: Key
	readonly args: Args
	readonly ret: Ret
	readonly annotation: AN
}

type GetExportFnsInUniformFormatManual<T> = {
	[K in keyof T]: T[K] extends (...args: infer Args) => infer Ret
		? Args extends CType[]
			? // eslint-disable-next-line @typescript-eslint/no-invalid-void-type
				Ret extends CType | void
				? ExportEntryImpl<
						K,
						GetJSTypeFromCTypeArr<Args>,
						Ret extends CType
							? GetJSTypeFromCTypeEnumSpecialCase<Ret>
							: // eslint-disable-next-line @typescript-eslint/no-invalid-void-type
								void,
						GetAnnotations<Ret>
					>
				: [K, 'error', 'ret non ctype']
			: [K, 'error', 'args not all ctypes']
		: never
}[keyof T]

type GetJSTypeFromGeneratedCTypeArr<A extends CType[]> = A extends []
	? []
	: A extends [infer First extends CType, ...infer Rest extends CType[]]
		? [
				GetJSTypeFromCTypeEnumSpecialCase<First>,
				...GetJSTypeFromGeneratedCTypeArr<Rest>,
			]
		: never

type GetExportFnsInUniformFormatGenerated<T> = {
	[K in keyof T]: T[K] extends (...args: infer Args) => infer Ret
		? Args extends CType[]
			? // eslint-disable-next-line @typescript-eslint/no-invalid-void-type
				Ret extends CType | void
				? ExportEntryImpl<
						K,
						GetJSTypeFromGeneratedCTypeArr<Args>,
						Ret extends CType
							? GetJSTypeFromCTypeEnumSpecialCase<Ret>
							: // eslint-disable-next-line @typescript-eslint/no-invalid-void-type
								void,
						GetAnnotations<Ret>
					>
				: [K, 'error', 'ret non generated ctype']
			: [K, 'error', 'args not all generated ctypes']
		: never
}[keyof T]

type _DeclaredExportFns = GetExportFnsInUniformFormatManual<ExportedCFunctions>

type _GeneratedExportFns =
	GetExportFnsInUniformFormatGenerated<GeneratedExportedFunctions>

interface FindSuccess {
	success: true
}
interface FindSuccessVal<Args, Ret, AN> {
	readonly args: Args
	readonly ret: Ret
	readonly annotation: AN
}

type FindByType<ExportedFns extends unknown[], Type> = ExportedFns extends []
	? [never, 'error', 'end of list reached']
	: ExportedFns extends [infer First, ...infer Rest]
		? First extends ExportEntryImpl<
				infer Type2,
				infer Args,
				infer Ret,
				infer ANOT
			>
			? Equal<Type, Type2> extends true
				? FindSuccessVal<Args, Ret, ANOT>
				: FindByType<Rest, Type>
			: [
					never,
					'error',
					'exported fns is not a list of valid types',
					First,
				]
		: [never, 'error', 'error 1']

type _TestFindFn1 = [
	ExportEntryImpl<'test1', 0, 0, []>,
	ExportEntryImpl<'test3', 2, 2, []>,
]

type _Expected_test_fn_1_0 = Expect<
	Equal<FindByType<_TestFindFn1, 'test1'>, FindSuccessVal<0, 0, []>>
>

type _Expected_test_fn_1_1 = Expect<
	Equal<
		FindByType<_TestFindFn1, 'test2'>,
		[never, 'error', 'end of list reached']
	>
>

type _Expected_test_fn_1_2 = Expect<
	Equal<FindByType<_TestFindFn1, 'test3'>, FindSuccessVal<2, 2, []>>
>

type FindMatchingFns<MyFn, ExportedFns extends unknown[]> =
	MyFn extends ExportEntryImpl<
		infer Type1,
		infer Args1,
		infer Ret1,
		infer AN1
	>
		? FindByType<ExportedFns, Type1> extends infer TypeRes
			? TypeRes extends FindSuccessVal<infer Args2, infer Ret2, infer AN2>
				? Equal<Args1, Args2> extends true
					? Equal<Ret1, Ret2> extends true
						? Equal<AN1, AN2> extends true
							? FindSuccess
							: ['error', 'annotation mismatch', Type1, AN1, AN2]
						: ['error', 'ret mismatch', Type1, Ret1, Ret2]
					: ['error', 'args mismatch', Type1, Args1, Args2]
				: ['error', 'no such function', Type1, TypeRes]
			: ['error', 'impl error 2']
		: ['error', 'impl error 1', 'FindMatchingFns', MyFn]

type ExportedFunctionMismatchImpl<
	MyFns extends unknown[],
	ExportedFns extends unknown[],
> = MyFns extends []
	? []
	: MyFns extends [infer First, ...infer Rest extends unknown[]]
		? FindMatchingFns<First, ExportedFns> extends infer Val
			? Val extends FindSuccess
				? ExportedFunctionMismatchImpl<Rest, ExportedFns>
				: [Val, ...ExportedFunctionMismatchImpl<Rest, ExportedFns>]
			: [never, 'error 1']
		: [never, 'error 2']

type _DeclaredExportFnsA = UnionToTuple<_DeclaredExportFns>
type _GeneratedExportFnsA = UnionToTuple<_GeneratedExportFns>

type _ExportedFunctionMismatch = ExportedFunctionMismatchImpl<
	_DeclaredExportFnsA,
	_GeneratedExportFnsA
>

type _ExportedFunctionMismatchL = _ExportedFunctionMismatch['length']

type _expect_0_0 = Expect<Equal<_ExportedFunctionMismatchL, 0>>

type _expect_0_1 = Expect<Equal<_ExportedFunctionMismatch, []>>

interface WASMExports
	extends WebAssembly.Exports,
		WASMExportsFnFromLibC,
		WASMExportsFnWithoutLibC {}

type ExportedCFunctions = Allocator & WASMExportsFnWithoutLibC

interface TypedWasmFn {
	platform_panic: (file_path_ptr: CStr, line: Int, message_ptr: CStr) => void
	platform_log_start: (error: Bool) => void
	platform_log_add: (message_ptr: CStr) => void
	platform_log_end: () => void
	platform_string_conversion: (
		data_ptr: Ptr<Void>,
		len: SizeT,
		format_ptr: CStr,
		out_data_ptr: Ptr<Ptr<Void>>,
		out_len_ptr: Ptr<SizeT>
	) => void
}

type _NotCFuncsTypesWasmFn = AreAllFunctionCFns<TypedWasmFn>

type _expect1 = Expect<Equal<_NotCFuncsTypesWasmFn, []>>

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

type GetFreeFnFromFn<Fn extends FreeFns> = Fn extends 'free'
	? Allocator['free']
	: Fn extends keyof WASMExportsFnWithoutLibC
		? WASMExportsFnWithoutLibC[Fn]
		: never

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

	public get functions(): WASMExportsFnWithoutLibC {
		return this.wasm.instance.exports
	}

	public get_free_fn<Fn extends FreeFns>(fn: Fn): GetFreeFnFromFn<Fn> {
		function is_free(f: FreeFns): f is 'free' {
			return f == 'free'
		}

		if (is_free(fn)) {
			return this.allocator.free as GetFreeFnFromFn<Fn>
		}
		return this.functions[
			fn as keyof WASMExportsFnWithoutLibC
		] as unknown as GetFreeFnFromFn<Fn>
	}
}

function get_malloced_disposable<
	Fn extends FreeFns,
	C extends CType &
		Annotations<
			Malloced<Fn>,
			AnnotationBase<'cstr'>,
			AnnotationBase<'free_fn'>,
			NoAnnot<'nullable'>
		>,
>(value: C, wasm: WASMWrapper, fn: Fn): MallocedDisposable<Fn, C> {
	return new MallocedDisposable<Fn, C>(value, (to_free: C): void => {
		type Param<Fn1 extends FreeFns> = Fn1 extends 'free'
			? Parameters<Allocator['free']>[0]
			: Fn1 extends keyof WASMExportsFnWithoutLibC
				? Parameters<WASMExportsFnWithoutLibC[Fn1]>[0]
				: never
		;(wasm.get_free_fn<Fn>(fn) as (p: Param<Fn>) => void)(
			to_free as unknown as Param<Fn>
		)
	})
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
		file_path_ptr: CStr,
		line: Int,
		message_ptr: CStr
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
	private platform_log_add(message_ptr: CStr): void {
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
		format_ptr: CStr,
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
				file_path_ptr: CStr,
				line: Int,
				message_ptr: CStr
			): void => {
				wasm.platform_panic.call(wasm, file_path_ptr, line, message_ptr)
			},
			platform_log_start: (error: Bool): void => {
				wasm.platform_log_start.call(wasm, error)
			},
			platform_log_add: (message_ptr: CStr): void => {
				wasm.platform_log_add.call(wasm, message_ptr)
			},
			platform_log_end: (): void => {
				wasm.platform_log_end.call(wasm)
			},
			platform_string_conversion: (
				data_ptr: Ptr<Void>,
				len: SizeT,
				format_ptr: CStr,
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
		const c_statictics: Ptr<AllocatorStatistics> =
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
		const SET_OPS: SettingsOptionEnum[] = [
			SettingsOptionEnum.SettingsOption_Strict_Script_allow_duplicate_fields,
			SettingsOptionEnum.SettingsOption_Strict_Script_allow_missing_script_type,
			SettingsOptionEnum.SettingsOption_Strict_allow_additional_fields,
			SettingsOptionEnum.SettingsOption_Strict_allow_number_truncating,
			SettingsOptionEnum.SettingsOption_Strict_allow_unrecognized_file_encoding,
			SettingsOptionEnum.SettingsOption_Strict_allow_validation_errors,
			SettingsOptionEnum.SettingsOption_Validate_Font_preset,
			SettingsOptionEnum.SettingsOption_Validate_validate_styles,
			SettingsOptionEnum.SettingsOption_Validate_validate_text,
		]

		type SetCommand = [option: SettingsOptionC, value: Int]

		function get_command_data(
			op: SettingsOptionEnum
		): SetCommand | undefined {
			const opV: SettingsOptionC = get_c_enum_from_enum<
				SettingsOptionEnum,
				SettingsOptionC
			>(op)

			switch (op) {
				case SettingsOptionEnum.SettingsOption_Strict_Script_allow_duplicate_fields: {
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
				case SettingsOptionEnum.SettingsOption_Strict_Script_allow_missing_script_type: {
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
				case SettingsOptionEnum.SettingsOption_Strict_allow_additional_fields: {
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
				case SettingsOptionEnum.SettingsOption_Strict_allow_number_truncating: {
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
				case SettingsOptionEnum.SettingsOption_Strict_allow_unrecognized_file_encoding: {
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
				case SettingsOptionEnum.SettingsOption_Strict_allow_validation_errors: {
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
				case SettingsOptionEnum.SettingsOption_Validate_Font_preset: {
					if (
						settings.validate_settings?.font_settings?.preset ===
						undefined
					) {
						return undefined
					}

					return [
						opV,
						uint8_t_to_int(
							enum_get_underlying_c_type<FontPreset, FontPresetC>(
								settings.validate_settings.font_settings.preset
							)
						),
					]
				}
				case SettingsOptionEnum.SettingsOption_Validate_validate_styles: {
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
				case SettingsOptionEnum.SettingsOption_Validate_validate_text: {
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
		let ass_source_raw: Annotated<
			Ptr<AssSource>,
			Annotations<
				Malloced<'free'>,
				NoAnnot<'cstr'>,
				NoAnnot<'free_fn'>,
				IsNullable
			>
		>

		if (typeof source === 'string') {
			const source_string = allocate_js_utf8_string(
				this.wasm.buffer,
				this.wasm.allocator,
				source,
				true
			)

			ass_source_raw = this.wasm.functions.source_from_string(
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

			ass_source_raw = this.wasm.functions.source_from_string(
				source_string.data_ptr,
				source_string.len
			)
		}

		if (!is_not_null(ass_source_raw)) {
			throw new Error('failed to allocate source')
		}

		using ass_source = get_malloced_disposable<
			'free',
			NullChecked<
				ReturnType<typeof this.wasm.functions.source_from_string>
			>
		>(ass_source_raw, this.wasm, 'free')

		const parse_settings_raw = this.wasm.functions.default_parse_settings()

		if (!is_not_null(parse_settings_raw)) {
			throw new Error('failed to allocate parse settings')
		}

		using parse_settings = get_malloced_disposable<
			'free',
			NullChecked<
				ReturnType<typeof this.wasm.functions.default_parse_settings>
			>
		>(parse_settings_raw, this.wasm, 'free')

		this.modify_parse_settings(parse_settings.value, settings)

		const raw_result: Annotated<
			Ptr<AssParseResultC>,
			Annotations<
				Malloced<'free_parse_result'>,
				NoAnnot<'cstr'>,
				NoAnnot<'free_fn'>,
				IsNullable
			>
		> = this.wasm.functions.parse_ass(
			ass_source.value,
			parse_settings.value
		)

		const raw_result_non_null = assert_not_null(raw_result)

		using result = get_malloced_disposable<
			'free_parse_result',
			NullChecked<ReturnType<typeof this.wasm.functions.parse_ass>>
		>(raw_result_non_null, this.wasm, 'free_parse_result')

		const freelist = new FreeList()

		freelist.add(ass_source)

		freelist.add(parse_settings)

		const ass_result = new AssParseResult(
			this.wasm,
			result.release_into_self_managed(),
			freelist
		)

		return ass_result
	}
}

abstract class CDisposable implements Disposable {
	#freelist: FreeList

	protected get freelist(): FreeList {
		return this.#freelist
	}

	constructor(freelist: FreeList) {
		this.#freelist = freelist
	}

	protected abstract set_freed(): void

	#is_free = false

	protected assert_not_freed(message = ''): void {
		if (this.#is_free) {
			throw new Error(
				`Tried to operate on freed value:${message ? ` ${message}` : ''}`
			)
		}
	}

	public free(): void {
		if (!this.#is_free) {
			this.#freelist.free()

			this.#is_free = true

			this.set_freed()
		}
	}

	[Symbol.dispose](): void {
		this.free()
	}
}

export class AssParseResult extends CDisposable {
	#wasm: WASMWrapper
	#result: MallocedAnnotationWrapper<
		'free_parse_result',
		Annotated<
			Ptr<AssParseResultC>,
			Annotations<
				Malloced<'free_parse_result'>,
				NoAnnot<'cstr'>,
				NoAnnot<'free_fn'>,
				NoAnnot<'nullable'>
			>
		>
	>

	constructor(
		wasm: WASMWrapper,
		result: MallocedAnnotationWrapper<
			'free_parse_result',
			Annotated<
				Ptr<AssParseResultC>,
				Annotations<
					Malloced<'free_parse_result'>,
					NoAnnot<'cstr'>,
					NoAnnot<'free_fn'>,
					NoAnnot<'nullable'>
				>
			>
		>,
		freelist: FreeList
	) {
		super(freelist)

		this.#wasm = wasm
		this.#result = result

		freelist.add_already_disposed(result)
	}

	#is_error_value: null | boolean = null

	public is_error(): boolean {
		this.assert_not_freed('result is a valid ptr')

		if (this.#is_error_value !== null) {
			return this.#is_error_value
		}

		this.#is_error_value = get_bool(
			this.#wasm.functions.parse_result_is_error(this.#result.value)
		)

		return this.#is_error_value
	}

	public diagnostics_ref(): DiagnosticsRef {
		this.assert_not_freed('result is a valid ptr')

		const c_diagnostics = this.#wasm.functions.get_diagnostics_from_result(
			this.#result.value
		)

		const diagnostics_ref = new DiagnosticsRef(this.#wasm, c_diagnostics)

		return diagnostics_ref
	}

	public result_ref(): AssResultRef {
		this.assert_not_freed('result is a valid ptr')

		if (this.is_error()) {
			throw new Error(`Tried to get result value on error result`)
		}

		const c_ass_result = this.#wasm.functions.parse_result_get_value(
			this.#result.value
		)

		const ass_result = new AssResultRef(this.#wasm, c_ass_result)

		return ass_result
	}

	protected set_freed(): void {
		this.#result.value = ptr_cast<Void, AssParseResultC>(nullptr())
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
	position: FilePos | undefined
}

type ParamsOf<F> = F extends (...args: infer Args) => infer Rest
	? [Args, Rest]
	: never

type ValidLengthArrayFn<F> =
	ParamsOf<F> extends [args: infer Args, infer Rest]
		? Rest extends SizeT
			? Args extends [infer FirstArg]
				? FirstArg extends Ptr<infer C>
					? IsCType<C> extends true
						? [true, C]
						: [false, 'Ptr without CType as first argument']
					: [false, 'no ptr as first argument', { o: FirstArg }]
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
		? Args extends [infer FirstArg, infer B]
			? FirstArg extends Ptr<infer C>
				? IsCType<C> extends true
					? B extends SizeT
						? Rest extends Annotated<
								Ptr<infer D>,
								Annotations<
									NoAnnot<'malloced'>,
									NoAnnot<'cstr'>,
									NoAnnot<'free_fn'>,
									IsNullable
								>
							>
							? IsCType<D> extends true
								? [true, [C, D]]
								: [false, 'Ptr without CType as return type']
							: Rest extends Ptr<infer C>
								? [
										false,
										'return type is not nullable, but a pointer',
										{ o: C },
									]
								: [false, 'no ptr as return type', { o: Rest }]
						: [false, 'invalid second argument type']
					: [false, 'Ptr without CType as first argument']
				: [false, 'no ptr as first argument', { o: FirstArg }]
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

type _LengthFns = GetLengthFnsSimple<ExportedCFunctions>

type _IndexFns = GetIndexFnsSimple<ExportedCFunctions>

type _expect2 = Expect<Equal<_LengthFns, _IndexFns>>

interface Test1Fns {
	abcs_get_length: (a: Ptr<Void>) => SizeT
	abcs_get_at: (
		a: Ptr<Char>,
		index: SizeT
	) => Annotated<
		Ptr<SizeT>,
		Annotations<
			NoAnnot<'malloced'>,
			NoAnnot<'cstr'>,
			NoAnnot<'free_fn'>,
			IsNullable
		>
	>
}

type _expectcheck_0_0 = Expect<Equal<GetIndexFnsSimple<Test1Fns>, 'abcs'>>

type Test1FnsRes1 = ArrayAccessFnsOf<Test1Fns>

type _expectcheck_0_1 = Expect<NotEqual<IsValidCListImpl<Test1FnsRes1>, true>>

type _expectcheck_0_2 = Expect<Equal<Test1FnsRes1, [never, Void, Char]>>

type TestStruct = CStruct<'TestStructFormFns'>

interface Test2Fns {
	abcs_get_length: (a: Ptr<Void>) => SizeT
	abcs_get_at: (
		a: Ptr<Void>,
		index: SizeT
	) => Annotated<
		Ptr<SizeT>,
		Annotations<
			NoAnnot<'malloced'>,
			NoAnnot<'cstr'>,
			NoAnnot<'free_fn'>,
			IsNullable
		>
	>

	abcd_get_length: (a: Ptr<TestStruct>) => SizeT
	abcd_get_at: (
		a: Ptr<TestStruct>,
		index: SizeT
	) => Annotated<
		Ptr<SizeT>,
		Annotations<
			NoAnnot<'malloced'>,
			NoAnnot<'cstr'>,
			NoAnnot<'free_fn'>,
			IsNullable
		>
	>
}

type _expectcheck_1_0 = Expect<
	Equal<GetIndexFnsSimple<Test2Fns>, 'abcs' | 'abcd'>
>

type Test2FnsRes1 = ArrayAccessFnsOf<Test2Fns>

type _expectcheck_1_1 = Expect<Equal<IsValidCListImpl<Test2FnsRes1>, true>>

type _expectcheck_1_2 = Expect<Equal<Test2FnsRes1, 'abcs' | 'abcd'>>

type ExportedCListAccessFns = ArrayAccessFnsOf<WASMExportsFnWithoutLibC>

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
	FindElementWhereForFn<
		GetLengthFnsImpl<ExportedCFunctions>,
		C
	> extends infer Temp
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
		TypesFromFromLengthFnImpl<'events'>,
		TypesFromFromIndexFnImpl<'events'>[0]
	>
>

type TypesFromFromIndexFnImpl<C extends ExportedCListAccessFns> =
	FindElementWhereForFn<
		GetIndexFnsImpl<ExportedCFunctions>,
		C
	> extends infer Temp
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
		const fn: WASMExportsFnWithoutLibC[`${CFuncLit}_get_length`] =
			this.wasm.functions[`${this.c_func_lit}_get_length`]

		return fn(
			underlying_type as Parameters<
				WASMExportsFnWithoutLibC[`${CFuncLit}_get_length`]
			>[0]
		)
	}

	protected override element_get_at_impl(
		underlying_type: Ptr<ListType>,
		index: SizeT
	): Ptr<ElementType> {
		const fn: WASMExportsFnWithoutLibC[`${CFuncLit}_get_at`] =
			this.wasm.functions[`${this.c_func_lit}_get_at`]

		return fn(
			underlying_type as Parameters<
				WASMExportsFnWithoutLibC[`${CFuncLit}_get_at`]
			>[0],
			index
		) as ElementType
	}

	public override unref(): JsElement[] {
		const result: JsElement[] = []

		for (let i = 0; i < this.length; ++i) {
			result.push(this.get_element_at(i))
		}

		return result
	}
}

export class DiagnosticsRef extends CArray<Diagnostic, 'diagnostics'> {
	constructor(wasm: WASMWrapper, diagnostics: Ptr<DiagnosticsC>) {
		super(wasm, diagnostics, 'diagnostics')
	}

	private get_file_pos_from_c(
		file_pos_c: Ptr<FilePosC>
	): FilePos | undefined {
		const is_empty_pos = get_bool(
			this.wasm.functions.is_empty_pos(file_pos_c)
		)

		if (is_empty_pos) {
			return undefined
		}

		const line: number = get_value<SizeT>(
			this.wasm.functions.file_pos_get_line(file_pos_c)
		)
		const column: number = get_value<SizeT>(
			this.wasm.functions.file_pos_get_column(file_pos_c)
		)

		return { line, column }
	}

	private get_severity_from_c(sev: DiagnosticSeverityC): DiagnosticSeverity {
		const severity: DiagnosticSeverityCEnum = get_enum_value<
			DiagnosticSeverityC,
			DiagnosticSeverityCEnum
		>(sev)

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

		return result
	}

	protected override convert_element_from_c_to_js(
		element: Ptr<DiagnosticC>
	): Diagnostic {
		using message_ptr = new MallocedDisposable(
			this.wasm.functions.get_message_from_entry(element),
			(val) => {
				if (is_not_null(val)) {
					this.wasm.functions.free_message_struct(val)
				}
			}
		)

		const message = this.get_string_from_message_struct(message_ptr.value)

		const file_pos_ptr =
			this.wasm.functions.diagnostic_get_file_pos(element)

		const position: FilePos | undefined =
			this.get_file_pos_from_c(file_pos_ptr)

		const severity_c = this.wasm.functions.diagnostic_get_severity(element)

		const severity = this.get_severity_from_c(severity_c)

		return { message, position, severity }
	}
}

export type LineType = 'CrLf' | 'Lf' | 'Cr'

enum LineTypeCEnum {
	'CrLf' = 0,
	'Lf',
	'Cr',
}

type LineTypeC = CEnum<LineTypeCEnum, 'LineType', UInt8T>

export type FileType =
	| 'Unknown'
	| 'UTF-8'
	| 'UTF-16BE'
	| 'UTF-16LE'
	| 'UTF-32BE'
	| 'UTF-32LE'

enum FileTypeCEnum {
	'Unknown' = 0,
	'UTF-8',
	'UTF-16BE',
	'UTF-16LE',
	'UTF-32BE',
	'UTF-32LE',
}

type FileTypeC = CEnum<FileTypeCEnum, 'FileType', UInt8T>

export interface FileProps {
	line_type: LineType
	file_type: FileType
}

export type ScriptType = 'Unknown' | 'V4' | 'V4Plus'

enum ScriptTypeCEnum {
	'Unknown' = 0,
	'V4',
	'V4Plus',
}

type ScriptTypeC = CEnum<ScriptTypeCEnum, 'ScriptType', UInt8T>

export type WrapStyle = 'Smart' | 'EOL' | 'NoWrap' | 'SmartLow'

enum WrapStyleCEnum {
	'Smart' = 0,
	'EOL',
	'NoWrap',
	'SmartLow',
}

type WrapStyleC = CEnum<WrapStyleCEnum, 'WrapStyle', UInt8T>

export interface AssScriptInfo {
	title: string
	original_script: string
	original_translation: string
	original_editing: string
	original_timing: string
	synch_point: string
	script_updated_by: string
	update_details: string
	script_type: ScriptType
	collisions: string
	play_res_y: SizeT
	play_res_x: SizeT
	play_depth: string
	timer: string
	wrap_style: WrapStyle
	// not documented, but present
	scaled_border_and_shadow: boolean
	video_aspect_ratio: SizeT
	video_zoom: SizeT
	ycbcr_matrix: string
}

export type ExtraSectionEntry = Record<string, string>

export type ExtraSections = Record<string, ExtraSectionEntry>

export interface AssResult {
	script_info: AssScriptInfo
	styles: AssStyle[]
	events: AssEvent[]
	//fonts: AssFonts
	//graphics: AssGraphics
	extra_sections: ExtraSections
	file_props: FileProps
}

export class AssResultRef extends Unref<AssResult> {
	#wasm: WASMWrapper
	#ass_result: Ptr<AssResultC>

	constructor(wasm: WASMWrapper, ass_result: Ptr<AssResultC>) {
		this.#wasm = wasm
		this.#ass_result = ass_result
	}

	public script_info(): ScriptInfo {
		const c_script_info =
			this.#wasm.functions.get_script_info_from_ass_result(
				this.#ass_result
			)

		const script_info = new ScriptInfo(this.#wasm, c_script_info)

		return script_info
	}

	public styles_ref(): StylesRef {
		const c_styles = this.#wasm.functions.get_styles_from_ass_result(
			this.#ass_result
		)

		const styles = new StylesRef(this.#wasm, c_styles)

		return styles
	}

	public events_ref(): EventsRef {
		const c_events = this.#wasm.functions.get_events_from_ass_result(
			this.#ass_result
		)

		const events = new EventsRef(this.#wasm, c_events)

		return events
	}

	public override unref(): AssResult {
		return {
			script_info: 'TODO',
			styles: this.styles_ref().unref(),
			events: this.events_ref().unref(),
			extra_sections: 'TODO',
			file_props: 'TODO',
		}
	}
}

export class ScriptInfo {
	#wasm: WASMWrapper
	#script_info: Ptr<ScriptInfoC>

	constructor(wasm: WASMWrapper, script_info: Ptr<ScriptInfoC>) {
		this.#wasm = wasm
		this.#script_info = script_info
	}

	public todo(): void {
		console.log(this.#wasm, this.#script_info)
	}
}

export interface AssStyle {
	todo: number
}

export class StylesRef extends CArray<AssStyle, 'styles'> {
	constructor(wasm: WASMWrapper, style: Ptr<AssStylesC>) {
		super(wasm, style, 'styles')
	}

	protected override convert_element_from_c_to_js(
		element: Ptr<AssStyleC>
	): AssStyle {
		return { todo: get_ptr_value<AssStyleC>(element) }
	}
}

export interface AssEvent {
	todo: number
}

export class EventsRef extends CArray<AssEvent, 'events'> {
	constructor(wasm: WASMWrapper, event: Ptr<AssEventsC>) {
		super(wasm, event, 'events')
	}

	protected override convert_element_from_c_to_js(
		element: Ptr<AssEventC>
	): AssEvent {
		return { todo: get_ptr_value<AssEventC>(element) }
	}
}
