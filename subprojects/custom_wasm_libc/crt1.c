
// from: https://github.com/WebAssembly/wasi-libc/blob/main/libc-bottom-half/crt/crt1-reactor.c

// see:
// https://stackoverflow.com/questions/72568387/why-is-an-objects-constructor-being-called-in-every-exported-wasm-function

extern void __wasm_call_ctors(void);

__attribute__((export_name("_initialize"))) void _initialize(void) {

	static volatile int initialized = 0;
	if(initialized != 0) {
		__builtin_trap();
	}
	initialized = 1;

	// The linker synthesizes this to call constructors.
	__wasm_call_ctors();
}
