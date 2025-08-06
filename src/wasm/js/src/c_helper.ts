import type { Expect } from 'type-testing'

declare const _Nested_Key_SYM: unique symbol

interface NestedCType {
	_Nested_Key_SYM: true
}

type CTypeSimple = symbol

type CType = CTypeSimple | NestedCType

type IsCType<A> = A extends CType ? true : false

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

declare const _Ptr_SYM: unique symbol

declare const _Ptr_Key_SYM: unique symbol

export type Ptr<a extends CType> = { [_Ptr_SYM]: boolean } & {
	[_Ptr_Key_SYM]: a
} & NestedCType

type _expect0 = Expect<IsCType<Ptr<CTypeSimple>>>

type _expect0_1 = Expect<IsCType<Ptr<Ptr<CTypeSimple>>>>

declare const _Bool_SYM: unique symbol

export type Bool = typeof _Bool_SYM

type _expect1 = Expect<IsCType<Bool>>

declare const _Char_SYM: unique symbol

export type Char = typeof _Char_SYM

type _expect2 = Expect<IsCType<Char>>

declare const _Int_SYM: unique symbol

export type Int = typeof _Int_SYM

type _expect3 = Expect<IsCType<Int>>

declare const _Void_SYM: unique symbol

export type Void = typeof _Void_SYM

type _expect4 = Expect<IsCType<Void>>

declare const _SizeT_SYM: unique symbol

export type SizeT = typeof _SizeT_SYM

type _expect5 = Expect<IsCType<SizeT>>

declare const _UInt64T_SYM: unique symbol

export type UInt64T = typeof _UInt64T_SYM

type _expect6 = Expect<IsCType<UInt64T>>

export type UInt64TJs = bigint

declare const _WasmStructRef_SYM: unique symbol

declare const _WasmStructRef_Key_SYM: unique symbol

export type WasmStructRef<a extends CType> = {
	[_WasmStructRef_SYM]: boolean
} & {
	[_WasmStructRef_Key_SYM]: a
} & NestedCType

type _expect7 = Expect<IsCType<WasmStructRef<CTypeSimple>>>

export type Mem = Uint8Array

export type MemBuf = ArrayBuffer

export interface Allocator {
	malloc: (amount: SizeT) => Ptr<Void>
	free: (ptr: Ptr<Void>) => Void
}

function get_pointer<a extends CType>(ptr_r: Ptr<a>): number {
	return ptr_r as unknown as number
}

export function ptr_cast<a extends CType, b extends CType>(
	ptr_r: Ptr<a>
): Ptr<b> {
	return ptr_r as unknown as Ptr<b>
}

export function get_bool(value_r: Bool): boolean {
	// eslint-disable-next-line @typescript-eslint/no-unnecessary-type-conversion
	return !!(value_r as unknown as boolean)
}

export function get_int(int_r: Int): number {
	return int_r as unknown as number
}

// eslint-disable-next-line @typescript-eslint/no-unnecessary-type-parameters
export function get_c_enum_from_enum<E extends number, C extends CType>(
	enum_v: E
): C {
	return enum_v as unknown as C
}

export function c_bool_to_int(inp: Bool): Int {
	return inp as unknown as Int
}

export function get_c_bool(inp: boolean): Bool {
	// eslint-disable-next-line @typescript-eslint/no-unnecessary-type-conversion, no-extra-boolean-cast
	const val: number = !!inp ? 1 : 0
	return val as unknown as Bool
}

export function get_size_t(size_t_r: SizeT): number {
	return size_t_r as unknown as number
}

export function get_uint64_t(val_r: UInt64T): UInt64TJs {
	return val_r as unknown as UInt64TJs
}

function to_size_t(num: number): SizeT {
	return num as unknown as SizeT
}

function cstrlen(mem: Mem, ptr_r: Ptr<Char>): number {
	let ptr = get_pointer(ptr_r)
	let len = 0
	while (mem[ptr] != 0) {
		len++
		ptr++
	}
	return len
}

export function cstr_by_ptr(mem_buffer: MemBuf, ptr_r: Ptr<Char>): string {
	const mem = new Uint8Array(mem_buffer)

	const ptr = get_pointer(ptr_r)

	const len = cstrlen(mem, ptr_r)
	const bytes = new Uint8Array(mem_buffer, ptr, len)
	return new TextDecoder().decode(bytes)
}

export interface SizedPtr {
	data_ptr: Ptr<Void>
	len: SizeT
}

export interface CStr {
	data_ptr: Ptr<Char>
	len: SizeT
}

export function get_sized_ptr_from_memory(
	buffer: MemBuf,
	size_ptr: SizedPtr
): Uint8Array {
	const ptr = get_pointer(size_ptr.data_ptr)
	const len = get_size_t(size_ptr.len)

	return new Uint8Array(buffer, ptr, len)
}

export function write_ptr_to_memory<A extends CType>(
	buffer: MemBuf,
	ptr: Ptr<Ptr<A>>,
	value: Ptr<A>
): void {
	const data = new DataView(buffer)

	data.setUint32(get_pointer(ptr), get_pointer(value))
}
export function write_size_t_to_memory(
	buffer: MemBuf,
	ptr: Ptr<SizeT>,
	value: SizeT
): void {
	const data = new DataView(buffer)

	data.setUint32(get_pointer(ptr), get_size_t(value))
}

function sized_ptr_to_cstr(ptr: SizedPtr): CStr {
	return { data_ptr: ptr_cast<Void, Char>(ptr.data_ptr), len: ptr.len }
}

function copy_js_array_to_wasm_memory(
	buffer: MemBuf,
	allocator: Allocator,
	array: Uint8Array
): SizedPtr {
	const data = new Uint8Array(buffer)

	const str_length: SizeT = to_size_t(array.length)

	const data_ptr: Ptr<Void> = allocator.malloc(str_length)

	const ptr = get_pointer(data_ptr)

	if (ptr == 0) {
		throw new Error('allocation failed')
	}

	data.set(array, ptr)

	return { data_ptr, len: str_length }
}

export function allocate_js_utf8_string(
	buffer: MemBuf,
	allocator: Allocator,
	string: string,
	allocate_bom = false
): CStr {
	const encoded = new TextEncoder().encode(string + '\0')

	let final_encoded = encoded

	if (allocate_bom) {
		const bom: number[] = [0xef, 0xbb, 0xbf]

		final_encoded = new Uint8Array(encoded.byteLength + 1)

		final_encoded.set(bom, 0)
		final_encoded.set(encoded, bom.length)
	}

	return sized_ptr_to_cstr(
		copy_js_array_to_wasm_memory(buffer, allocator, final_encoded)
	)
}

export function make_string_from_array_buffer(
	buffer: MemBuf,
	allocator: Allocator,
	content: ArrayBuffer
): CStr {
	const encoded = new Uint8Array(content.byteLength + 1)

	encoded.set(new Uint8Array(content), 0)
	encoded.set([0], content.byteLength)

	return sized_ptr_to_cstr(
		copy_js_array_to_wasm_memory(buffer, allocator, encoded)
	)
}

export function construct_ptr_error(
	buffer: MemBuf,
	allocator: Allocator,
	message: string
): SizedPtr {
	const str_data = allocate_js_utf8_string(buffer, allocator, message)

	return {
		data_ptr: ptr_cast<Char, Void>(str_data.data_ptr),
		len: to_size_t(0),
	}
}

export type FreeFn<A extends CType> = (arg: Ptr<A>) => void

export type FreeData<A extends CType> = [value: Ptr<A>, free_fn: FreeFn<A>]

export class FreeList {
	private list: FreeData<Void>[]

	constructor() {
		this.list = []
	}

	public add<B extends CType>(ptr: Ptr<B>, free_fn: FreeFn<B>): void {
		this.list.push([ptr_cast<B, Void>(ptr), free_fn as FreeFn<Void>])
	}

	public free(): void {
		for (const value of this.list) {
			value[1](value[0])
		}
	}
}
