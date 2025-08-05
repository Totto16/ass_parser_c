declare const _Ptr_SYM: unique symbol

declare const _Ptr_Key_SYM: unique symbol

export type Ptr<a> = { [_Ptr_SYM]: boolean } & { [_Ptr_Key_SYM]: a }

declare const _Bool_SYM: unique symbol

export type Bool = typeof _Bool_SYM

declare const _Char_SYM: unique symbol

export type Char = typeof _Char_SYM

declare const _Int_SYM: unique symbol

export type Int = typeof _Int_SYM

declare const _Void_SYM: unique symbol

export type Void = typeof _Void_SYM

declare const _SizeT_SYM: unique symbol

export type SizeT = typeof _SizeT_SYM

declare const _UInt64T_SYM: unique symbol

export type UInt64T = typeof _UInt64T_SYM

export type UInt64TJs = bigint

declare const _WasmStructRef_SYM: unique symbol

declare const _WasmStructRef_Key_SYM: unique symbol

export type WasmStructRef<a> = { [_WasmStructRef_SYM]: boolean } & {
	[_WasmStructRef_Key_SYM]: a
}

export type Mem = Uint8Array

export type MemBuf = ArrayBuffer

export interface Allocator {
	malloc: (amount: SizeT) => Ptr<Void>
	free: (ptr: Ptr<Void>) => Void
}

function get_pointer<a>(ptr_r: Ptr<a>): number {
	return ptr_r as unknown as number
}

export function ptr_cast<a, b>(ptr_r: Ptr<a>): Ptr<b> {
	return ptr_r as unknown as Ptr<b>
}

export function get_bool(value_r: Bool): boolean {
	// eslint-disable-next-line @typescript-eslint/no-unnecessary-type-conversion
	return !!(value_r as unknown as boolean)
}

export function get_int(int_r: Int): number {
	return int_r as unknown as number
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
): ArrayBufferView {
	const ptr = get_pointer(size_ptr.data_ptr)
	const len = get_size_t(size_ptr.len)

	return new DataView(buffer, ptr, len)
}

export function write_ptr_to_memory<A>(
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

export function allocate_js_utf8_string(
	buffer: MemBuf,
	allocator: Allocator,
	string: string,
	allocate_bom = false
): CStr {
	const data = new Uint8Array(buffer)

	const encoded = new TextEncoder().encode(string)

	const bom: number[] = [0xef, 0xbb, 0xbf]

	const bom_length = allocate_bom ? bom.length : 0

	const str_length: SizeT = to_size_t(bom_length + encoded.length + 1)

	const data_ptr: Ptr<Char> = ptr_cast<Void, Char>(
		allocator.malloc(str_length)
	)

	if (get_pointer(data_ptr) == 0) {
		throw new Error('allocation failed')
	}

	const ptr = get_pointer(data_ptr)

	const zeroByte: Uint8Array = new Uint8Array(1)

	zeroByte.set([0], 0)

	if (allocate_bom) {
		data.set(bom, ptr)
	}

	data.set(encoded, ptr + bom_length)
	data.set(zeroByte, ptr + bom_length + encoded.length)

	console.log(data.subarray(ptr, ptr + encoded.length + 1 + bom_length))

	return { data_ptr, len: str_length }
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
