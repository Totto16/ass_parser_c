import type { ExportedFunctions } from 'src/generated/wasm_exports'
import type { Equal, Expect, NotEqual } from 'type-testing'

// eslint-disable-next-line @typescript-eslint/no-invalid-void-type
type ValidJSTypes = number | bigint | void

export interface AnnotationBase {
	readonly __annotation: unique symbol
}

export interface Annotation<VAL> extends AnnotationBase {
	readonly value: VAL
}

export type NoAnnot = Annotation<false>

export interface Annotations<
	MAL extends AnnotationBase,
	CSTR extends AnnotationBase,
	FREEFN extends AnnotationBase,
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

export type CTypeSimple<
	Desc extends string,
	JSType extends ValidJSTypes,
> = CTypeSimpleImpl<Desc, JSType> & Annotations<NoAnnot, NoAnnot, NoAnnot>

interface CTypeNestedImpl<
	Desc extends string,
	NestedType extends CType,
	JSType extends ValidJSTypes,
> extends CTypeSimpleImpl<Desc, JSType> {
	readonly __nested: NestedType
}

type CTypeNested<
	Desc extends string,
	NestedType extends CType,
	JSType extends ValidJSTypes,
> = CTypeNestedImpl<Desc, NestedType, JSType> & {
	readonly __nested: NestedType
} & Annotations<NoAnnot, NoAnnot, NoAnnot>

export type CType<
	Desc extends string = string,
	JSType extends ValidJSTypes = ValidJSTypes,
> = (CTypeSimpleImpl<Desc, JSType> | CTypeNestedImpl<Desc, CType, JSType>) &
	Annotations<AnnotationBase, AnnotationBase, AnnotationBase>

export type IsCType<C> =
	C extends CTypeSimple<infer _A, infer _B>
		? true
		: C extends CTypeNested<infer B, infer _C, infer _D>
			? IsCType<B>
			: false

export type GetJSTypeFromCType<C extends CType> = C extends {
	readonly __type: infer JSType
}
	? JSType
	: never

export type GetJSTypeFromCTypeEnumSpecialCase<C extends CType> =
	C extends CEnum<infer _A, infer _B, infer D>
		? GetJSTypeFromCType<D>
		: GetJSTypeFromCType<C>

type AreAllCTypes<A extends readonly unknown[]> = A extends [
	infer Head,
	...infer Tail,
]
	? IsCType<Head> extends true
		? AreAllCTypes<Tail>
		: false
	: true

type IsCFunc<C> = C extends (...args: infer Args) => infer Ret
	? AreAllCTypes<Args> extends true
		? IsCType<Ret>
		: false
	: false

type AreAllFunctionCFnImpl<O, T extends keyof O> = [T] extends [never]
	? true
	: IsCFunc<O[T]> extends true
		? true
		: false

export type AreAllFunctionCFns<O> = AreAllFunctionCFnImpl<O, keyof O>

type _32BitNum = number

type _64BitNum = bigint

type PtrType = _32BitNum

export type Ptr<C extends CType> = CTypeNested<'ptr', C, PtrType>

type _expect0_0 = Expect<
	Equal<Annotated<Ptr<Int>, Annotations<NoAnnot, NoAnnot, NoAnnot>>, Ptr<Int>>
>

type _expect0_01 = Expect<
	NotEqual<
		Annotated<Ptr<Int>, Annotations<Malloced<'free'>, NoAnnot, NoAnnot>>,
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
> = CTypeNested<'enum', EnumType<UnderlyingCType, D>, C>

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
	C extends CType,
	A extends Annotations<AnnotationBase, AnnotationBase, AnnotationBase>,
> = C & A

export type GetAnnotations<T> =
	T extends Annotated<infer _F, infer A>
		? A
		: Annotations<NoAnnot, NoAnnot, NoAnnot>

export interface Malloced<F extends keyof ExportedFunctions>
	extends Annotation<F> {
	readonly __call_type: ExportedFunctions[F]
}

export type IsCString = Annotation<'is_c_string'>

export type IsFreeFn = Annotation<'is_free_fn'>

export class MallocedDisposable<
	C extends CType,
	Fn extends keyof ExportedFunctions,
	AN extends Annotations<Malloced<Fn>, AnnotationBase, AnnotationBase>,
> implements Disposable
{
	private val: C
	private fn: (f: C) => void

	constructor(val: Annotated<C, AN>, fn: (f: C) => void) {
		this.val = val
		this.fn = fn
	}

	[Symbol.dispose](): void {
		this.fn(this.val)
	}
}
