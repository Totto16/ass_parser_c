#!/usr/bin/env node

import { readFileSync } from 'node:fs'


function main(): void {
	const binary = readFileSync('./static/build/ass_parser.wasm')

	const wasmModule = new WebAssembly.Module(binary)

	// Get export names and types
	const exports = WebAssembly.Module.exports(wasmModule)

	const customSections = WebAssembly.Module.customSections(wasmModule, 'name')

	console.log(
		exports,
		customSections.map((a) => new TextDecoder().decode(a))
	)
}

main()

