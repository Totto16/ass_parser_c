import type { GeneratedExportedFunctions } from 'src/generated/wasm_exports'
import type { Equal, Expect, NotEqual } from 'type-testing'

// eslint-disable-next-line @typescript-eslint/no-invalid-void-type
type ValidJSTypes = number | bigint | void

type AnnotationType = 'malloced' | 'cstr' | 'free_fn'

export interface AnnotationBase<AT extends AnnotationType> {
	readonly __annotation: unique symbol
	readonly __type: AT
}

export interface Annotation<VAL, AT extends AnnotationType>
	extends AnnotationBase<AT> {
	readonly value: VAL
}

export type NoAnnot<AT extends AnnotationType = AnnotationType> = Annotation<
	false,
	AT
>

export interface Annotations<
	MAL extends AnnotationBase<'malloced'>,
	CSTR extends AnnotationBase<'cstr'>,
	FREEFN extends AnnotationBase<'free_fn'>,
> {
	readonly __malloced: MAL
	readonly __cstring: CSTR
	readonly __free_fn: FREEFN
}

interface CTypeSimpleImpl<Desc extends string, JSType extends ValidJSTypes> {
	readonly __marker: unique symbol
	readonly __type: JSType
	readonly __desc: Desc
}

type DefaultAnnotations = Annotations<
	NoAnnot<'malloced'>,
	NoAnnot<'cstr'>,
	NoAnnot<'free_fn'>
>

export type CTypeSimple<
	Desc extends string,
	JSType extends ValidJSTypes,
> = CTypeSimpleImpl<Desc, JSType> & DefaultAnnotations

interface CTypeNestedImpl<
	Desc extends string,
	NestedType extends CTypeNoAnnotations,
	JSType extends ValidJSTypes,
> extends CTypeSimpleImpl<Desc, JSType> {
	readonly __nested: NestedType
}

type RawType<C extends CType> =
	C extends CTypeNestedImpl<infer Desc, infer CType, infer JSType>
		? CTypeNestedImpl<Desc, CType, JSType>
		: C extends CTypeSimpleImpl<infer Desc, infer JSType>
			? CTypeSimpleImpl<Desc, JSType>
			: [never, 'error']

type CTypeNested<
	Desc extends string,
	NestedType extends CType,
	JSType extends ValidJSTypes,
> = CTypeNestedImpl<Desc, RawType<NestedType>, JSType> & DefaultAnnotations

export type CTypeNoAnnotations<
	Desc extends string = string,
	JSType extends ValidJSTypes = ValidJSTypes,
> =
	| CTypeNestedImpl<Desc, CTypeNoAnnotations, JSType>
	| CTypeSimpleImpl<Desc, JSType>

export type CType<
	Desc extends string = string,
	JSType extends ValidJSTypes = ValidJSTypes,
> = CTypeNoAnnotations<Desc, JSType> &
	Annotations<
		AnnotationBase<'malloced'>,
		AnnotationBase<'cstr'>,
		AnnotationBase<'free_fn'>
	>

export type IsCType<C> =
	C extends CTypeSimpleImpl<infer _A, infer _B>
		? true
		: C extends CTypeNestedImpl<infer B, infer _C, infer _D>
			? IsCType<B>
			: C extends CType
				? IsCType<RawType<C>>
				: false

type AnnotNotAssignableTo = (() => Annotated<
	Ptr<Void>,
	Annotations<Malloced<'free'>, NoAnnot<'cstr'>, NoAnnot<'free_fn'>>
>) extends () => infer V
	? V extends Ptr<Void>
		? ['error', V, Ptr<Void>, Equal<V, Ptr<Void>>]
		: []
	: never

type _expect_annot_0 = Expect<Equal<[], AnnotNotAssignableTo>>

export type GetJSTypeFromCType<C extends CType> = C extends {
	readonly __type: infer JSType
}
	? JSType
	: never

export type GetJSTypeFromCTypeEnumSpecialCase<C extends CType> =
	C extends CEnum<infer _A, infer _B, infer D>
		? GetJSTypeFromCType<D>
		: GetJSTypeFromCType<C>

type UnionToIntersection<U> = (
	U extends unknown ? (k: U) => void : never
) extends (k: infer I) => void
	? I
	: never

type LastOf<T> =
	UnionToIntersection<
		T extends unknown ? () => T : never
	> extends () => infer R
		? R
		: never

type Push<T extends unknown[], V> = [...T, V]

export type UnionToTuple<T, L = LastOf<T>> = [T] extends [never]
	? []
	: Push<UnionToTuple<Exclude<T, L>>, L>

type IsCTypeArr<T> = IsCType<T> extends true ? [true] : [false, T]

type AreAllCTypes<A extends readonly unknown[]> = A extends [
	infer Head,
	...infer Tail,
]
	? IsCTypeArr<Head> extends infer Res
		? Res extends [true]
			? AreAllCTypes<Tail>
			: [false, Res]
		: [false, 'impl error 1', 'AreAllCTypes']
	: [true]

// eslint-disable-next-line @typescript-eslint/no-invalid-void-type
type IRetCType<T> = T extends void ? [true] : IsCTypeArr<T>

type IsCFunc<Args extends unknown[], Ret> =
	AreAllCTypes<Args> extends infer Res
		? Res extends [true]
			? IRetCType<Ret>
			: [false, Res]
		: [false, 'impl error 1', 'IsCFunc']

type AreAllFunctionCFnImpl<T> = {
	[K in keyof T]: T[K] extends (...args: infer Args) => infer Ret
		? IsCFunc<Args, Ret> extends infer Res
			? Res extends [true]
				? never
				: [K, Res]
			: never
		: never
}[keyof T]

export type AreAllFunctionCFns<T> = UnionToTuple<AreAllFunctionCFnImpl<T>>

type _32BitNum = number

type _64BitNum = bigint

type PtrType = _32BitNum

export type Ptr<C extends CType> = CTypeNested<'ptr', C, PtrType>

type _expect0_0 = Expect<
	Equal<
		Annotated<
			Ptr<Int>,
			Annotations<
				NoAnnot<'malloced'>,
				NoAnnot<'cstr'>,
				NoAnnot<'free_fn'>
			>
		>,
		Ptr<Int>
	>
>

type _expect0_01 = Expect<
	Equal<
		Annotated<
			Int,
			Annotations<
				NoAnnot<'malloced'>,
				NoAnnot<'cstr'>,
				NoAnnot<'free_fn'>
			>
		>,
		Int
	>
>

type _expect0_02 = Expect<
	NotEqual<
		Annotated<
			Ptr<Int>,
			Annotations<Malloced<'free'>, NoAnnot<'cstr'>, NoAnnot<'free_fn'>>
		>,
		Ptr<Int>
	>
>

type _expect0_1 = Expect<NotEqual<Ptr<Char>, Ptr<Int>>>

type _expect0_2 = Expect<NotEqual<Ptr<Char>, Ptr<Void>>>

type _expect0_3 = Expect<NotEqual<Ptr<Char>, Ptr<UInt64T>>>

type _expect0_4 = Expect<NotEqual<Ptr<Void>, Ptr<SizeT>>>

type _expect0 = Expect<IsCType<Ptr<CTypeSimple<string, ValidJSTypes>>>>

type _expect0__1 = Expect<IsCType<Ptr<Ptr<CTypeSimple<string, ValidJSTypes>>>>>

type _expect0__2 = Expect<IsCType<Ptr<Ptr<Int>>>>

export type Bool = CTypeSimple<'bool', number>

type _expect1 = Expect<IsCType<Bool>>

export type Char = CTypeSimple<'char', number>

type _expect2_1 = Expect<NotEqual<Bool, Char>>

type _expect2 = Expect<IsCType<Char>>

export type UInt8T = CTypeSimple<'uint8_t', number>

type _expect2__1 = Expect<NotEqual<UInt8T, Char>>

type _expect2_ = Expect<IsCType<UInt8T>>

export type Int = CTypeSimple<'int', number>

type _expect3_1 = Expect<NotEqual<Char, Int>>

type _expect3 = Expect<IsCType<Int>>

export type Void = CTypeSimple<'void', void>

type _expect4_1 = Expect<NotEqual<Int, Void>>

type _expect4_2 = Expect<NotEqual<Char, Void>>

type _expect4 = Expect<IsCType<Void>>

export type SizeT = CTypeSimple<'size_t', number>

type _expect5 = Expect<IsCType<SizeT>>

type _expect5_1 = Expect<NotEqual<SizeT, Void>>

type _expect5_2 = Expect<NotEqual<SizeT, Int>>

export type UInt64T = CTypeSimple<'uint64_t', _64BitNum>

type _expect6 = Expect<IsCType<UInt64T>>

type _expect6_1 = Expect<NotEqual<SizeT, UInt64T>>

type _expect6_2 = Expect<NotEqual<Void, UInt64T>>

type _expect6_3 = Expect<NotEqual<UInt64T, Int>>

export type CStruct<Desc extends string> = CTypeNested<
	'struct',
	Ptr<Void>,
	number
> & {
	readonly __struct_name: Desc
}

type EnumType<C extends CType, D extends string> = CTypeNested<
	'enum_type',
	C,
	never
> & {
	readonly __underlying_type: C
	readonly __name: D
}

export type GetEnumType<T> =
	T extends CEnum<infer _A, infer _B, infer C> ? C : never

export type CEnum<
	C extends ValidJSTypes,
	D extends string,
	UnderlyingCType extends CType,
> = CTypeNestedImpl<'enum', EnumType<UnderlyingCType, D>, C> &
	DefaultAnnotations

enum TestEnum {
	'test1' = 0,
	'test2',
}

type _expect8 = Expect<IsCType<CEnum<TestEnum, 'TestEnum', UInt8T>>>

type _expect8_1 = Expect<
	NotEqual<
		CEnum<TestEnum, 'Testenum', UInt8T>,
		CEnum<TestEnum, 'Testenum', Int>
	>
>

enum TestEnum2 {
	'test1' = 0,
	'test2',
}

type _expect8_2 = Expect<
	NotEqual<
		CEnum<TestEnum, 'Testenum', UInt8T>,
		CEnum<TestEnum2, 'Testenum2', UInt8T>
	>
>

type _expect8_3 = Expect<
	Equal<CEnum<TestEnum, 'TestEnum', UInt8T>['__desc'], 'enum'>
>

type _expect8_4 = Expect<
	Equal<CEnum<TestEnum, 'TestEnum', UInt8T>['__type'], TestEnum>
>

type _expect8_5 = Expect<
	Equal<
		CEnum<TestEnum, 'TestEnum', UInt8T>['__nested'],
		EnumType<UInt8T, 'TestEnum'>
	>
>

type _expect8_6 = Expect<
	Equal<GetJSTypeFromCType<CEnum<TestEnum, 'TestEnum', UInt8T>>, TestEnum>
>

type _expect8_7 = Expect<
	Equal<
		GetJSTypeFromCTypeEnumSpecialCase<CEnum<TestEnum, 'TestEnum', UInt8T>>,
		GetJSTypeFromCType<UInt8T>
	>
>

export type Annotated<
	// eslint-disable-next-line @typescript-eslint/no-invalid-void-type
	C extends CType | void,
	A extends Annotations<
		AnnotationBase<'malloced'>,
		AnnotationBase<'cstr'>,
		AnnotationBase<'free_fn'>
	>,
	// eslint-disable-next-line @typescript-eslint/no-invalid-void-type
> = (C extends CType ? RawType<C> : C extends void ? Void : never) & A

export type GetAnnotations<T> =
	T extends Annotated<infer _F, infer A>
		? A extends Annotations<infer A, infer B, infer C>
			? Annotations<A, B, C>
			: A
		: DefaultAnnotations

type _expect_annot_get_0 = Expect<
	Equal<DefaultAnnotations, GetAnnotations<Int>>
>

type _expect_annot_get_1 = Expect<
	Equal<GetAnnotations<void>, GetAnnotations<Int>>
>

type RemoveAnnotations<C> = C extends CType
	? RawType<C> & DefaultAnnotations
	: never

export function remove_annotations<A extends CType>(
	a: A
): RemoveAnnotations<A> {
	return a as unknown as RemoveAnnotations<A>
}

type GetFreeFns<T> = {
	[K in keyof T]: T[K] extends (...args: infer _Args) => infer Ret
		? Ret extends Annotations<infer _A, infer _B, IsFreeFn>
			? K
			: never
		: never
}[keyof T]

export type FreeFns = GetFreeFns<GeneratedExportedFunctions>

type _expected_free_fns = Expect<
	Equal<FreeFns, 'free' | 'free_parse_result' | 'free_message_struct'>
>

export interface Malloced<F extends FreeFns> extends Annotation<F, 'malloced'> {
	readonly __call_type: GeneratedExportedFunctions[F]
}

export type IsCString = Annotation<'is_c_string', 'cstr'>

export type IsFreeFn = Annotation<'is_free_fn', 'free_fn'>

export class MallocedDisposable<
	Fn extends FreeFns,
	C extends CType &
		Annotations<
			Malloced<Fn>,
			AnnotationBase<'cstr'>,
			AnnotationBase<'free_fn'>
		>,
> implements Disposable
{
	private val: RawType<C>
	private fn: (f: RawType<C>) => void

	constructor(val: C, fn: (f: RawType<C>) => void) {
		this.val = remove_annotations(val)
		this.fn = fn
	}

	private is_free = false

	public free(): void {
		if (!this.is_free) {
			this.fn(this.val)

			this.is_free = true
		}
	}
	[Symbol.dispose](): void {
		this.free()
	}
}
