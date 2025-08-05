const Ptr_SYM = Symbol("Ptr")

const Ptr_Key_SYM = Symbol("Ptr_Key")

export type Ptr<a> = { [Ptr_SYM]: boolean } & { [Ptr_Key_SYM]: a }

const Char_SYM = Symbol("Char")

export type Char = typeof Char_SYM

const Int_SYM = Symbol("Int")

export type Int = typeof Int_SYM

const Void_SYM = Symbol("Void")

export type Void = typeof Void_SYM

const SizeT_SYM = Symbol("SizeT")

export type SizeT = typeof SizeT_SYM

const UInt64T_SYM = Symbol("UInt64T")

export type UInt64T = typeof UInt64T_SYM

export type UInt64TJs = BigInt

const WasmStructRef_SYM = Symbol("WasmStructRef")

const WasmStructRef_Key_SYM = Symbol("WasmStructRef_Key")

export type WasmStructRef<a> = { [WasmStructRef_SYM]: boolean } & {
	[WasmStructRef_Key_SYM]: a
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

	let ptr = get_pointer(ptr_r)

	const len = cstrlen(mem, ptr_r)
	const bytes = new Uint8Array(mem_buffer, ptr, len)
	return new TextDecoder().decode(bytes)
}

export interface SizedPtr {
	data_ptr: Ptr<Void>
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

export function construct_ptr_error(
	buffer: MemBuf,
	allocator: Allocator,
	message: string
): SizedPtr {
	const data = new Uint8Array(buffer)

	const encoded = new TextEncoder().encode(message)

	const data_ptr: Ptr<Void> = allocator.malloc(to_size_t(encoded.length))

	if (get_pointer(data_ptr) == 0) {
		return construct_ptr_error(buffer, allocator, "allocation failed")
	}

	const ptr = get_pointer(data_ptr)

	data.set(encoded, ptr)

	return { data_ptr, len: to_size_t(0) }
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
	string: string
): SizedPtr {
	const data = new Uint8Array(buffer)

	const encoded = new TextEncoder().encode(string)

	const str_length: SizeT = to_size_t(encoded.length)

	const data_ptr: Ptr<Void> = allocator.malloc(str_length)

	if (get_pointer(data_ptr) == 0) {
		return construct_ptr_error(buffer, allocator, "allocation failed")
	}

	const ptr = get_pointer(data_ptr)

	data.set(encoded, ptr)

	return { data_ptr, len: str_length }
}
