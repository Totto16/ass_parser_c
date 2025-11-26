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
	get_c_value,
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
	type GetWrapper,
	type NoWrapper,
	type WrapperWith,
	type NullWrapper,
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

type AssColorC = CStruct<'AssColor'>

type FilePropsC = CStruct<'FileProps'>

type ExtraSectionsC = CStruct<'ExtraSections'>

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
		source: Ptr<Char>, // not a cstr, as this may contain 0 bytes
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
		CStr,
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
	get_extra_sections_from_ass_result: (
		ass_result: Ptr<AssResultC>
	) => Ptr<ExtraSectionsC>
	get_file_props_from_ass_result: (
		ass_result: Ptr<AssResultC>
	) => Ptr<FilePropsC>
	//
	get_scaled_border_and_shadow_from_script_info: (
		script_info: Ptr<ScriptInfoC>
	) => Bool
	get_script_type_from_script_info: (
		script_info: Ptr<ScriptInfoC>
	) => ScriptTypeC
	get_wrap_style_from_script_info: (
		script_info: Ptr<ScriptInfoC>
	) => WrapStyleC
	get_video_aspect_ratio_from_script_info: (
		script_info: Ptr<ScriptInfoC>
	) => SizeT
	get_video_zoom_from_script_info: (script_info: Ptr<ScriptInfoC>) => SizeT
	get_play_res_x_from_script_info: (script_info: Ptr<ScriptInfoC>) => SizeT
	get_play_res_y_from_script_info: (script_info: Ptr<ScriptInfoC>) => SizeT
	get_string_by_name_from_script_info: (
		script_info: Ptr<ScriptInfoC>,
		name: CStr
	) => Annotated<
		CStr,
		Annotations<Malloced<'free'>, IsCString, NoAnnot<'free_fn'>, IsNullable>
	>
	get_color_component_from_ass_color: (
		ass_color: Ptr<AssColorC>,
		index: UInt8T
	) => UInt8T
	get_ass_color_from_ass_style: (
		ass_style: Ptr<AssStyleC>,
		index: UInt8T
	) => Ptr<AssColorC>
	get_file_type_from_file_props: (file_props: Ptr<FilePropsC>) => FileTypeC
	get_line_type_from_file_props: (file_props: Ptr<FilePropsC>) => LineTypeC
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

interface ExportEntryImpl<Key, Args, Ret, AN, Wrapper> {
	readonly key: Key
	readonly args: Args
	readonly ret: Ret
	readonly annotation: AN
	readonly wrapper: Wrapper
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
						GetAnnotations<Ret>,
						GetWrapper<Ret>
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
						GetAnnotations<Ret>,
						GetWrapper<Ret>
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
interface FindSuccessVal<Args, Ret, AN, Wrapper> {
	readonly args: Args
	readonly ret: Ret
	readonly annotation: AN
	readonly wrapper: Wrapper
}

type FindByType<ExportedFns extends unknown[], Type> = ExportedFns extends []
	? [never, 'error', 'end of list reached']
	: ExportedFns extends [infer First, ...infer Rest]
		? First extends ExportEntryImpl<
				infer Type2,
				infer Args,
				infer Ret,
				infer ANOT,
				infer Wrapper
			>
			? Equal<Type, Type2> extends true
				? FindSuccessVal<Args, Ret, ANOT, Wrapper>
				: FindByType<Rest, Type>
			: [
					never,
					'error',
					'exported fns is not a list of valid types',
					First,
				]
		: [never, 'error', 'error 1']

type _TestFindFn1 = [
	ExportEntryImpl<'test1', 0, 0, [], { e: '' }>,
	ExportEntryImpl<'test3', 2, 2, [], { e: '' }>,
]

type _Expected_test_fn_1_0 = Expect<
	Equal<
		FindByType<_TestFindFn1, 'test1'>,
		FindSuccessVal<0, 0, [], { e: '' }>
	>
>

type _Expected_test_fn_1_1 = Expect<
	Equal<
		FindByType<_TestFindFn1, 'test2'>,
		[never, 'error', 'end of list reached']
	>
>

type _Expected_test_fn_1_2 = Expect<
	Equal<
		FindByType<_TestFindFn1, 'test3'>,
		FindSuccessVal<2, 2, [], { e: '' }>
	>
>

type AreSameJsType<C1 extends CType, C2 extends CType> =
	Equal<GetJSTypeFromCType<C1>, GetJSTypeFromCType<C2>> extends true
		? true
		: [
				never,
				'error in js type',
				GetJSTypeFromCType<C1>,
				GetJSTypeFromCType<C2>,
			]

// ptr<void> to ptr<i32> is the same for the size, but even if ptr<ptr<void>> == ptr<void> in size, we don't allow this, only allow the same nesting
type AreBothPointerRec<C1 extends CType, C2 extends CType> =
	AreBothPointer<C1, C2> extends true ? true : true

type AreBothPointer<C1 extends CType, C2 extends CType> =
	C1 extends Ptr<infer A1>
		? C2 extends Ptr<infer A2>
			? AreBothPointerRec<A1, A2>
			: [never, 'error 1']
		: [never, 'error 2']

interface EnumType<A, B> {
	readonly name: A
	readonly underlying_type: B
}

type AreSameEnumType<A, B> =
	A extends EnumType<infer Name1, infer UType1>
		? B extends EnumType<infer Name2, infer UType2>
			? Equal<Name1, Name2> extends true
				? Equal<UType1, UType2> extends true
					? true
					: [never, 'error 1']
				: [never, 'error 2']
			: [never, 'error 3']
		: [never, 'error 4']

type AreBothEnums<C1 extends CType, C2 extends CType> =
	C1 extends CEnum<infer _1, infer Name1, infer UType1>
		? C2 extends CEnum<infer _2, infer Name2, infer UType2>
			? AreSameEnumType<EnumType<Name1, UType1>, EnumType<Name2, UType2>>
			: [never, 'error 1']
		: [never, 'error 2']

type IsEqualWrapperType<C1, C2> = C1 extends CType
	? C2 extends CType
		? AreBothPointer<C1, C2> extends true
			? true
			: AreBothEnums<C1, C2> extends true
				? true
				: Equal<C1, C2> extends true
					? true
					: [never, 'first argument is not equal to second argument']
		: [never, 'second type is nota ctype', C2]
	: [never, 'first type is nota ctype', C1]

type WrapEqual<W1, W2> =
	W1 extends NoWrapper<infer NoArg1>
		? W2 extends NoWrapper<infer NoArg2>
			? AreSameJsType<NoArg1, NoArg2> extends true
				? true
				: [
						never,
						'not same js type',
						AreSameJsType<NoArg1, NoArg2>,
						NoArg1,
						NoArg2,
					]
			: W2 extends WrapperWith<infer Desc, infer Type>
				? Desc extends 'ptr'
					? AreBothPointer<NoArg1, Type>
					: Desc extends 'enum'
						? AreBothEnums<NoArg1, Type>
						: [never, 'invalid Desc', Desc]
				: [
						never,
						'second argument is nota a wrapper and not not a wrapper ?!?!?, impl error',
					]
		: W1 extends WrapperWith<infer Desc1, infer Type1>
			? W2 extends WrapperWith<infer Desc2, infer Type2>
				? Equal<Desc1, Desc2> extends true
					? IsEqualWrapperType<Type1, Type2>
					: [never, 'Desc do not match', Desc1, Desc2]
				: [
						never,
						'first argument is a wrapper, but second is not one',
						W1,
						W2,
					]
			: W1 extends NullWrapper
				? W2 extends NullWrapper
					? true
					: [
							never,
							'first argument is null wrapper, but second is not one',
							W1,
							W2,
						]
				: [never, 'complete mismatch', W1, W2]

type FindMatchingFns<MyFn, ExportedFns extends unknown[]> =
	MyFn extends ExportEntryImpl<
		infer Type1,
		infer Args1,
		infer Ret1,
		infer AN1,
		infer Wrap1
	>
		? FindByType<ExportedFns, Type1> extends infer TypeRes
			? TypeRes extends FindSuccessVal<
					infer Args2,
					infer Ret2,
					infer AN2,
					infer Wrap2
				>
				? Equal<Args1, Args2> extends true
					? Equal<Ret1, Ret2> extends true
						? Equal<AN1, AN2> extends true
							? WrapEqual<Wrap1, Wrap2> extends true
								? FindSuccess
								: [
										'error',
										'wrapper mismatch',
										Type1,
										Wrap1,
										Wrap2,
										WrapEqual<Wrap1, Wrap2>,
									]
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
		const message_raw = this.wasm.functions.get_message_from_entry(element)

		const message_not_null = assert_not_null(message_raw)

		using message_ptr = new MallocedDisposable(message_not_null, (val) => {
			if (is_not_null(val)) {
				this.wasm.functions.free_message_struct(val)
			}
		})

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

export type BorderStyle = 'Outline' | 'OpaqueBox'

enum BorderStyleCEnum {
	'Outline' = 1,
	'OpaqueBox' = 3,
}

type BorderStyleC = CEnum<BorderStyleCEnum, 'BorderStyle', UInt8T>

export type AssAlignment =
	| 'BL'
	| 'BC'
	| 'BR'
	| 'ML'
	| 'MC'
	| 'MR'
	| 'TL'
	| 'TC'
	| 'TR'

enum AssAlignmentCEnum {
	'BL' = 1,
	'BC',
	'BR',
	'ML',
	'MC',
	'MR',
	'TL',
	'TC',
	'TR',
}

type AssAlignmentC = CEnum<AssAlignmentCEnum, 'AssAlignment', UInt8T>

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
	play_res_y: number
	play_res_x: number
	play_depth: string
	timer: string
	wrap_style: WrapStyle
	// not documented, but present
	scaled_border_and_shadow: boolean
	video_aspect_ratio: number
	video_zoom: number
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
		super()

		this.#wasm = wasm
		this.#ass_result = ass_result
	}

	public script_info_ref(): ScriptInfoRef {
		const c_script_info =
			this.#wasm.functions.get_script_info_from_ass_result(
				this.#ass_result
			)

		const script_info = new ScriptInfoRef(this.#wasm, c_script_info)

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

	public extra_sections_ref(): ExtraSectionsRef {
		const c_extra_sections =
			this.#wasm.functions.get_extra_sections_from_ass_result(
				this.#ass_result
			)

		const extra_sections = new ExtraSectionsRef(
			this.#wasm,
			c_extra_sections
		)

		return extra_sections
	}

	public file_props_ref(): FilePropsRef {
		const c_file_props =
			this.#wasm.functions.get_file_props_from_ass_result(
				this.#ass_result
			)

		const file_props = new FilePropsRef(this.#wasm, c_file_props)

		return file_props
	}

	public override unref(): AssResult {
		return {
			script_info: this.script_info_ref().unref(),
			styles: this.styles_ref().unref(),
			events: this.events_ref().unref(),
			extra_sections: this.extra_sections_ref().unref(),
			file_props: this.file_props_ref().unref(),
		}
	}
}

type GetStringKeys<T> = {
	[K in keyof T as T[K] extends string
		? string extends T[K]
			? K
			: never
		: never]: T[K]
}

type _expect_string_keys_0 = Expect<
	Equal<GetStringKeys<{ hello: string }>, { hello: string }>
>

type _expect_string_keys_1 = Expect<
	Equal<GetStringKeys<{ hello: string; not: number }>, { hello: string }>
>

type _expect_string_keys_2 = Expect<
	Equal<
		GetStringKeys<{
			hello: string
			not: number
			second: string
			third: bigint
		}>,
		{ hello: string; second: string }
	>
>

// eslint-disable-next-line @typescript-eslint/no-empty-object-type
type _expect_string_keys_3 = Expect<Equal<GetStringKeys<{ hello: symbol }>, {}>>

type _expect_string_keys_4 = Expect<
	Equal<
		GetStringKeys<{
			hello: 'hello' | 'or' | 'not'
			second: string
		}>,
		{ second: string }
	>
>

type ScriptInfoStrings = GetStringKeys<AssScriptInfo>

export class ScriptInfoRef extends Unref<AssScriptInfo> {
	#wasm: WASMWrapper
	#script_info: Ptr<ScriptInfoC>

	constructor(wasm: WASMWrapper, script_info: Ptr<ScriptInfoC>) {
		super()

		this.#wasm = wasm
		this.#script_info = script_info
	}

	private get_scaled_border_and_shadow(): boolean {
		const scaled_border_and_shadow = get_bool(
			this.#wasm.functions.get_scaled_border_and_shadow_from_script_info(
				this.#script_info
			)
		)

		return scaled_border_and_shadow
	}

	private get_video_aspect_ratio(): number {
		const video_aspect_ratio = get_value<SizeT>(
			this.#wasm.functions.get_video_aspect_ratio_from_script_info(
				this.#script_info
			)
		)

		return video_aspect_ratio
	}

	private get_video_zoom(): number {
		const video_zoom = get_value<SizeT>(
			this.#wasm.functions.get_video_zoom_from_script_info(
				this.#script_info
			)
		)

		return video_zoom
	}

	private get_play_res_y(): number {
		const play_res_y = get_value<SizeT>(
			this.#wasm.functions.get_play_res_y_from_script_info(
				this.#script_info
			)
		)

		return play_res_y
	}

	private get_play_res_x(): number {
		const play_res_x = get_value<SizeT>(
			this.#wasm.functions.get_play_res_x_from_script_info(
				this.#script_info
			)
		)

		return play_res_x
	}

	private get_strings(): ScriptInfoStrings {
		const result: Partial<ScriptInfoStrings> = {}

		const string_names: (keyof ScriptInfoStrings)[] = [
			'title',
			'original_script',
			'original_translation',
			'original_editing',
			'original_timing',
			'synch_point',
			'script_updated_by',
			'update_details',
			'collisions',
			'play_depth',
			'timer',
			'ycbcr_matrix',
		]
		for (const string_name of string_names) {
			const c_name_sized = allocate_js_utf8_string(
				this.#wasm.buffer,
				this.#wasm.allocator,
				string_name,
				false
			)
			const c_value_raw =
				this.#wasm.functions.get_string_by_name_from_script_info(
					this.#script_info,
					c_name_sized.data_ptr as unknown as CStr
				)

			const c_value_not_null = assert_not_null(c_value_raw)

			using c_value = get_malloced_disposable<
				'free',
				typeof c_value_not_null
			>(c_value_not_null, this.#wasm, 'free')

			const result_value = cstr_by_ptr(this.#wasm.buffer, c_value.value)

			result[string_name] = result_value
		}

		return result as ScriptInfoStrings
	}

	private get_script_type_from_c(script_type: ScriptTypeC): ScriptType {
		const script_type_js: ScriptTypeCEnum = get_enum_value<
			ScriptTypeC,
			ScriptTypeCEnum
		>(script_type)

		switch (script_type_js) {
			case ScriptTypeCEnum.Unknown:
				return 'Unknown'
			case ScriptTypeCEnum.V4:
				return 'V4'
			case ScriptTypeCEnum.V4Plus:
				return 'V4Plus'
			default:
				throw new Error('Implementation error')
		}
	}

	private get_script_type(): ScriptType {
		const script_type_c =
			this.#wasm.functions.get_script_type_from_script_info(
				this.#script_info
			)

		return this.get_script_type_from_c(script_type_c)
	}

	private get_wrap_style_from_c(wrap_style: WrapStyleC): WrapStyle {
		const wrap_style_js: WrapStyleCEnum = get_enum_value<
			WrapStyleC,
			WrapStyleCEnum
		>(wrap_style)

		switch (wrap_style_js) {
			case WrapStyleCEnum.EOL:
				return 'EOL'
			case WrapStyleCEnum.NoWrap:
				return 'NoWrap'
			case WrapStyleCEnum.Smart:
				return 'Smart'
			case WrapStyleCEnum.SmartLow:
				return 'SmartLow'
			default:
				throw new Error('Implementation error')
		}
	}

	private get_wrap_style(): WrapStyle {
		const wrap_style_c =
			this.#wasm.functions.get_wrap_style_from_script_info(
				this.#script_info
			)

		return this.get_wrap_style_from_c(wrap_style_c)
	}

	public override unref(): AssScriptInfo {
		const script_type: ScriptType = this.get_script_type()

		const wrap_style: WrapStyle = this.get_wrap_style()

		const scaled_border_and_shadow: boolean =
			this.get_scaled_border_and_shadow()

		const video_aspect_ratio: number = this.get_video_aspect_ratio()
		const video_zoom: number = this.get_video_zoom()

		const play_res_y: number = this.get_play_res_y()
		const play_res_x: number = this.get_play_res_x()

		const {
			title,
			original_script,
			original_translation,
			original_editing,
			original_timing,
			synch_point,
			script_updated_by,
			update_details,
			collisions,
			play_depth,
			timer,
			ycbcr_matrix,
		} = this.get_strings()

		return {
			title,
			original_script,
			original_translation,
			original_editing,
			original_timing,
			synch_point,
			script_updated_by,
			update_details,
			script_type,
			collisions,
			play_res_y,
			play_res_x,
			play_depth,
			timer,
			wrap_style,
			scaled_border_and_shadow,
			video_aspect_ratio,
			video_zoom,
			ycbcr_matrix,
		}
	}
}

export class ExtraSectionsRef extends Unref<ExtraSections> {
	#wasm: WASMWrapper
	#extra_sections: Ptr<ExtraSectionsC>

	constructor(wasm: WASMWrapper, extra_sections: Ptr<ExtraSectionsC>) {
		super()

		this.#wasm = wasm
		this.#extra_sections = extra_sections
	}

	public get_section_by_name(name: string): ExtraSectionRef | undefined {
		//TODO
	}

	public get_all_names(): string[] {
		//
	}

	public override unref(): ExtraSections {
		const names = this.get_all_names()

		const result: ExtraSections = {}

		for (const name of names) {
			const section_ref = this.get_section_by_name(name)
			if (section_ref === undefined) {
				throw new Error(
					`Expected name '${name}' in extra sections to be present, but it is not!`
				)
			}

			result[name] = section_ref.unref()
		}

		return result
	}
}

export class FilePropsRef extends Unref<FileProps> {
	#wasm: WASMWrapper
	#file_props: Ptr<FilePropsC>

	constructor(wasm: WASMWrapper, file_props: Ptr<FilePropsC>) {
		super()

		this.#wasm = wasm
		this.#file_props = file_props
	}

	private get_line_type_from_c(line_type: LineTypeC): LineType {
		const line_type_js: LineTypeCEnum = get_enum_value<
			LineTypeC,
			LineTypeCEnum
		>(line_type)

		switch (line_type_js) {
			case LineTypeCEnum.Cr:
				return 'Cr'
			case LineTypeCEnum.CrLf:
				return 'CrLf'
			case LineTypeCEnum.Lf:
				return 'Lf'
			default:
				throw new Error('Implementation error')
		}
	}

	private get_line_type(): LineType {
		const line_type_c = this.#wasm.functions.get_line_type_from_file_props(
			this.#file_props
		)

		return this.get_line_type_from_c(line_type_c)
	}

	private get_file_type_from_c(file_type: FileTypeC): FileType {
		const file_type_js: FileTypeCEnum = get_enum_value<
			FileTypeC,
			FileTypeCEnum
		>(file_type)

		switch (file_type_js) {
			case FileTypeCEnum.Unknown:
				return 'Unknown'
			case FileTypeCEnum['UTF-8']:
				return 'UTF-8'
			case FileTypeCEnum['UTF-16LE']:
				return 'UTF-16LE'
			case FileTypeCEnum['UTF-16BE']:
				return 'UTF-16BE'
			case FileTypeCEnum['UTF-32LE']:
				return 'UTF-32LE'
			case FileTypeCEnum['UTF-32BE']:
				return 'UTF-32BE'
			default:
				throw new Error('Implementation error')
		}
	}

	private get_file_type(): FileType {
		const file_type_c = this.#wasm.functions.get_file_type_from_file_props(
			this.#file_props
		)

		return this.get_file_type_from_c(file_type_c)
	}

	public override unref(): FileProps {
		const line_type = this.get_line_type()

		const file_type = this.get_file_type()

		return { file_type, line_type }
	}
}

export type EncodingType = number

export class AssColorRef extends Unref<AssColor> {
	#wasm: WASMWrapper
	#ass_color: Ptr<AssColorC>

	constructor(wasm: WASMWrapper, ass_color: Ptr<AssColorC>) {
		super()

		this.#wasm = wasm
		this.#ass_color = ass_color
	}

	public override unref(): AssColor {
		const values: [number, number, number, number] = (
			[0, 1, 2, 3] as [number, number, number, number]
		).map((index) => {
			const c_value =
				this.#wasm.functions.get_color_component_from_ass_color(
					this.#ass_color,
					get_c_value<number, UInt8T>(index)
				)

			return get_value<UInt8T>(c_value)
		}) as [number, number, number, number]

		return new AssColor(...values)
	}
}

export class AssColor {
	public r: number
	public g: number
	public b: number
	public a: number

	constructor(r: number, g: number, b: number, a: number) {
		this.r = r
		this.g = g
		this.b = b
		this.a = a
	}

	public toWebColor(): string {
		function display(n: number): string {
			return n.toString(16).padStart(2, '0')
		}

		return `#${display(this.r)}${display(this.g)}${display(this.b)}${display(this.a)}`
	}

	public toString(): string {
		return this.toWebColor()
	}
}

export interface AssStyle {
	name: string
	fontname: string
	fontsize: number
	primary_colour: AssColor
	secondary_colour: AssColor
	outline_colour: AssColor
	back_colour: AssColor
	bold: boolean
	italic: boolean
	underline: boolean
	strike_out: boolean
	scale_x: number
	scale_y: number
	spacing: number
	angle: number
	border_style: BorderStyle
	outline: number
	shadow: number
	alignment: AssAlignment
	margin_l: number
	margin_r: number
	margin_v: number
	encoding: EncodingType
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

export class AssTime {
	public hour: number
	public min: number
	public sec: number
	public hundred: number

	constructor(hour: number, min: number, sec: number, hundred: number) {
		this.hour = hour
		this.min = min
		this.sec = sec
		this.hundred = hundred
	}

	//TODO: implement some helper methods
}

export interface MarginValueDefault {
	default: true
}

export interface MarginValueWithValue {
	default: false
	value: number
}

export type MarginValue = MarginValueDefault | MarginValueWithValue

export type EventType =
	| 'Dialogue'
	| 'Comment'
	| 'Picture'
	| 'Sound'
	| 'Movie'
	| 'Command'

export interface AssEventBase {
	layer: number
	start: AssTime
	end: AssTime
	style: string
	name: string
	margin_l: MarginValue
	margin_r: MarginValue
	margin_v: MarginValue
	effect: string
}

export interface AssEventGeneric<Type extends EventType, Text>
	extends AssEventBase {
	type: Type
	text: Text
}

export interface AssText {
	todo: string
}

export type AssEventDialogue = AssEventGeneric<'Dialogue', AssText>

export type AssEventComment = AssEventGeneric<'Comment', AssText>

export type AssEventPicture = AssEventGeneric<'Picture', string>

export type AssEventSound = AssEventGeneric<'Sound', string>

export type AssEventMovie = AssEventGeneric<'Movie', string>

type SSACommand = 'TODO'

export interface CommandEventSSA {
	is_ssa: true
	command: SSACommand
}

export interface CommandEventString {
	is_ssa: false
	string: string
}

export type CommandEvent = CommandEventSSA | CommandEventString

export type AssEventCommand = AssEventGeneric<'Command', CommandEvent>

export type AssEvent =
	| AssEventDialogue
	| AssEventComment
	| AssEventPicture
	| AssEventSound
	| AssEventMovie
	| AssEventCommand

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
