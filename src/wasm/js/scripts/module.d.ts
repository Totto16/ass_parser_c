declare module '@webassemblyjs/wasm-parser' {
	interface DecoderOpts {
		dump: boolean
		ignoreCodeSection: boolean
		ignoreDataSection: boolean
		ignoreCustomNameSection: boolean
	}

	declare const decode: (
		buf: ArrayBuffer | Uint8Array,
		opts: Partial<DecoderOpts>
	) => unknown
}

declare module '@webassemblyjs/wast-parser' {
	declare const parse: (source: string) => unknown
}
