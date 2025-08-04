"use strict"

const AssParseResult_SYM = Symbol("AssParseResult")

type AssParseResult = typeof AssParseResult_SYM

const AssSource_SYM = Symbol("AssSource")

type AssSource = typeof AssSource_SYM

const ParseSettings_SYM = Symbol("ParseSettings")

type ParseSettings = typeof ParseSettings_SYM

interface WASMExports extends WebAssembly.Exports {
	memory: WebAssembly.Memory
	parse_ass: (
		source: AssSource,
		settings: ParseSettings
	) => Ptr<AssParseResult>
}

interface WASMInstance extends WebAssembly.Instance {
	readonly exports: WASMExports
}

interface WASM extends WebAssembly.WebAssemblyInstantiatedSource {
	instance: WASMInstance
}

let wasm: null | WASM = null

type Ptr<a> = number

type Mem = Uint8Array

type MemBuf = ArrayBuffer

type Char = number

type Int = number

function cstrlen(mem: Mem, ptr: Ptr<Char>): number {
	let len = 0
	while (mem[ptr] != 0) {
		len++
		ptr++
	}
	return len
}

function cstr_by_ptr(mem_buffer: MemBuf, ptr: Ptr<Char>): string {
	const mem = new Uint8Array(mem_buffer)
	const len = cstrlen(mem, ptr)
	const bytes = new Uint8Array(mem_buffer, ptr, len)
	return new TextDecoder().decode(bytes)
}

function get_memory_buffer(): MemBuf {
	if (!wasm) {
		throw new Error("Wasm is not initialized")
	}

	return wasm.instance.exports.memory.buffer
}

// void platform_panic(const char* file_path, int line, const char* message);
function platform_panic(
	file_path_ptr: Ptr<Char>,
	line: Int,
	message_ptr: Ptr<Char>
): void {
	const buffer = get_memory_buffer()
	const file_path = cstr_by_ptr(buffer, file_path_ptr)
	const message = cstr_by_ptr(buffer, message_ptr)
	console.error(file_path + ":" + line + ": " + message)
	// TODO: WASM platform_panic() does not halt the game
}
// void platform_log(const char* message);
function platform_log(message_ptr: Ptr<Char>): void {
	const buffer = get_memory_buffer()
	const message = cstr_by_ptr(buffer, message_ptr)
	console.log(message)
}

// void platform_error(const char* message);
function platform_error(message_ptr: Ptr<Char>): void {
	const buffer = get_memory_buffer()
	const message = cstr_by_ptr(buffer, message_ptr)
	console.error(message)
}

// void platform_string_conversion(void* data, size_t len, const char* format, void** out_data,               size_t* out_len);
function platform_string_conversion() {
	//TODO
}

export async function startWasm() {
	const numPages = 4
	const memory = new WebAssembly.Memory({ initial: numPages })

	const result: WASM = (await WebAssembly.instantiateStreaming(
		fetch("./static/ass_parser.wasm"),
		{
			env: {
				platform_panic,
				platform_log,
				platform_error,
				platform_string_conversion,
				memory,
			},
		}
	)) as WASM

	wasm = result
}

export function parse_ass(source: string): { todo: number } {
	if (!wasm) {
		throw new Error("Wasm is not initialized")
	}

	const ass_source: AssSource =
		wasm.instance.exports.source_from_string(source)

	const result = wasm.instance.exports.parse_ass(ass_source, settings)
}
