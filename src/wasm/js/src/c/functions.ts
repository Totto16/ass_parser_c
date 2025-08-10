import type { Equal } from 'type-testing'
import type {
	Bool,
	CEnum,
	Char,
	CType,
	GetJSTypeFromCType,
	Int,
	Ptr,
	SizeT,
	Void,
} from './types'

export type Mem = Uint8Array

export type MemBuf = ArrayBuffer

export interface Allocator {
	malloc: (amount: SizeT) => Ptr<Void>
	free: (ptr: Ptr<Void>) => Void
}

export function get_value<C extends CType>(value_r: C): GetJSTypeFromCType<C> {
	return value_r as unknown as GetJSTypeFromCType<C>
}

export function get_c_value<A, C extends CType>(
	val: A
): Equal<GetJSTypeFromCType<C>, A> extends true ? C : never {
	return val as unknown as Equal<GetJSTypeFromCType<C>, A> extends true
		? C
		: never
}

export function nullptr(): Ptr<Void> {
	return get_c_value<number, Ptr<Void>>(0)
}

export function ptr_cast<a extends CType, b extends CType>(
	ptr_r: Ptr<a>
): Ptr<b> {
	return ptr_r as unknown as Ptr<b>
}

export function get_c_enum_from_enum<
	// eslint-disable-next-line @typescript-eslint/no-unnecessary-type-parameters
	E extends number,
	// eslint-disable-next-line @typescript-eslint/no-unnecessary-type-parameters
	C extends CEnum<number, string, CType>,
>(enum_v: E): C {
	return enum_v as unknown as C
}

export function c_bool_to_int(inp: Bool): Int {
	return inp as unknown as Int
}

export function get_bool(value_r: Bool): boolean {
	return !!(value_r as unknown as GetJSTypeFromCType<Bool>)
}

export function get_c_bool(inp: boolean): Bool {
	const val: number = inp ? 1 : 0
	return val as unknown as Bool
}

function cstrlen(mem: Mem, ptr_r: Ptr<Char>): number {
	let ptr = get_value<Ptr<Char>>(ptr_r)
	let len = 0
	while (mem[ptr] != 0) {
		len++
		ptr++
	}
	return len
}

export function cstr_by_ptr(mem_buffer: MemBuf, ptr_r: Ptr<Char>): string {
	const mem = new Uint8Array(mem_buffer)

	const ptr = get_value<Ptr<Char>>(ptr_r)

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
	const ptr = get_value<Ptr<Void>>(size_ptr.data_ptr)
	const len = get_value<SizeT>(size_ptr.len)

	return new Uint8Array(buffer, ptr, len)
}

export function write_ptr_to_memory<A extends CType>(
	buffer: MemBuf,
	ptr: Ptr<Ptr<A>>,
	value: Ptr<A>
): void {
	const data = new DataView(buffer)

	data.setUint32(get_value<Ptr<Ptr<A>>>(ptr), get_value<Ptr<A>>(value))
}
export function write_size_t_to_memory(
	buffer: MemBuf,
	ptr: Ptr<SizeT>,
	value: SizeT
): void {
	const data = new DataView(buffer)

	data.setUint32(get_value<Ptr<SizeT>>(ptr), get_value<SizeT>(value))
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

	const str_length: SizeT = get_c_value<number, SizeT>(array.length)

	const data_ptr: Ptr<Void> = allocator.malloc(str_length)

	const ptr = get_value<Ptr<Void>>(data_ptr)

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
		len: get_c_value<number, SizeT>(0),
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

export class IndexOutOfBoundError extends RangeError {
	index: number
	length: number

	constructor(index: number, length: number) {
		super(
			`Index ${index.toString()} is out of bounds for array of length ${length.toString()}.`
		)
		this.name = 'IndexOutOfBoundError'
		this.index = index
		this.length = length

		Error.captureStackTrace(this, IndexOutOfBoundError)
	}
}

export abstract class CArrayGeneric<
	JsElement,
	ListType extends CType,
	ElementType extends CType,
> implements Iterable<JsElement>
{
	private underlying_type: Ptr<ListType>;

	[index: number]: JsElement

	constructor(underlying_type: Ptr<ListType>) {
		this.underlying_type = underlying_type

		return new Proxy(this, {
			get: (target, prop): unknown => {
				if (typeof prop === 'string') {
					const index = Number.parseInt(prop)
					if (!Number.isNaN(index)) {
						return target.element_at_impl(index)
					}
				}

				return (target as Record<string | symbol, unknown>)[prop]
			},
			has: (target, prop: string | symbol | number): boolean => {
				if (typeof prop === 'string') {
					const index = Number.parseInt(prop)
					if (!Number.isNaN(index)) {
						return index >= 0 && index < target.length
					}
				}

				return prop in target
			},
		})
	}

	private length_value: number | null = null

	protected abstract length_of_impl(underlying_type: Ptr<ListType>): SizeT

	public get length(): number {
		if (this.length_value !== null) {
			return this.length_value
		}

		this.length_value = get_value<SizeT>(
			this.length_of_impl(this.underlying_type)
		)

		this.cached_array = new Array<null>(this.length_value).fill(null)

		return this.length_value
	}

	private cached_array: (JsElement | null)[] = []

	protected abstract convert_element_from_c_to_js(
		element: Ptr<ElementType>
	): JsElement

	protected abstract element_get_at_impl(
		underlying_type: Ptr<ListType>,
		index: SizeT
	): Ptr<ElementType>

	private element_at_impl(index: number): JsElement {
		if (index < 0) {
			throw new IndexOutOfBoundError(index, this.length)
		}

		if (index >= this.length) {
			throw new IndexOutOfBoundError(index, this.length)
		}

		const cached_entry = this.cached_array[index]

		if (cached_entry === undefined) {
			throw new Error('Implementation error')
		}

		if (cached_entry === null) {
			const element: JsElement = this.convert_element_from_c_to_js(
				this.element_get_at_impl(
					this.underlying_type,
					get_c_value<number, SizeT>(index)
				)
			)

			this.cached_array[index] = element

			return element
		}

		return cached_entry
	}

	public at(index: number): JsElement | undefined {
		return Array.prototype.at.call<this, [number], JsElement>(this, index)
	}

	public *[Symbol.iterator](): Generator<JsElement, void, void> {
		for (let i = 0; i < this.length; i++) {
			yield this.element_at_impl(i)
		}
	}
}
