import type { Expect, NotEqual } from 'type-testing'

interface CTypeSimple<Desc extends string, JSType = unknown> {
	readonly __marker: unique symbol
	readonly __type: JSType
	readonly __desc: Desc
}

interface CTypeNested<
	Desc extends string,
	NestedType extends CType,
	JSType = unknown,
> extends CTypeSimple<Desc, JSType> {
	readonly __nested: NestedType
}

export type CType<Desc extends string = string, JSType = unknown> =
	| CTypeSimple<Desc, JSType>
	| CTypeNested<Desc, CType, JSType>

export type IsCType<C> =
	C extends CTypeSimple<infer _A>
		? true
		: C extends CTypeNested<infer B, infer _C>
			? IsCType<B>
			: false

export type GetJSTypeFromCType<C extends CType> = C extends {
	readonly __type: infer JSType
}
	? JSType
	: never

// eslint-disable-next-line @typescript-eslint/no-invalid-void-type
type IRetCType<T> = T extends void ? true : IsCType<T>

type IsCTypeArg<T> = IsCType<T>

type AreAllCTypes<A extends readonly unknown[]> = A extends [
	infer Head,
	...infer Tail,
]
	? IsCTypeArg<Head> extends true
		? AreAllCTypes<Tail>
		: false
	: true

type IsCFunc<C> = C extends (...args: infer Args) => infer Ret
	? AreAllCTypes<Args> extends true
		? IRetCType<Ret>
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

export type Ptr<C extends CType> = CTypeNested<'ptr', C> & {
	__type: PtrType
}

type _expect0_1 = Expect<NotEqual<Ptr<Char>, Ptr<Int>>>

type _expect0_2 = Expect<NotEqual<Ptr<Char>, Ptr<Void>>>

type _expect0_3 = Expect<NotEqual<Ptr<Char>, Ptr<UInt64T>>>

type _expect0_4 = Expect<NotEqual<Ptr<Void>, Ptr<SizeT>>>

type _expect0 = Expect<IsCType<Ptr<CTypeSimple<string>>>>

type _expect0__1 = Expect<IsCType<Ptr<Ptr<CTypeSimple<string>>>>>

type _expect0__2 = Expect<IsCType<Ptr<Ptr<Int>>>>

export type Bool = CTypeSimple<'bool', boolean>

type _expect1 = Expect<IsCType<Bool>>

export type Char = CTypeSimple<'char', number>

type _expect2_1 = Expect<NotEqual<Bool, Char>>

type _expect2 = Expect<IsCType<Char>>

export type Int = CTypeSimple<'int', number>

type _expect3_1 = Expect<NotEqual<Char, Int>>

type _expect3 = Expect<IsCType<Int>>

export type Void = CTypeSimple<'void', PtrType>

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

export type CStruct<Desc extends string> = CTypeNested<'struct', Ptr<Void>> & {
	readonly __struct_name: Desc
}

interface CTypeSimple<Desc extends string, JSType = unknown> {
	readonly __marker: unique symbol
	readonly __type: JSType
	readonly __desc: Desc
}

export type WasmStructRef<
	C extends CStruct<D>,
	D extends string = string,
> = CTypeNested<'struct_ref', C> & {
	__type: PtrType
}

type ExampleStruct1 = CStruct<'Test1'>

type ExampleStruct2 = CStruct<'Test2'>

type _expect7 = Expect<IsCType<WasmStructRef<ExampleStruct1>>>

type _expect7_1 = Expect<
	NotEqual<WasmStructRef<ExampleStruct1>, WasmStructRef<ExampleStruct2>>
>
