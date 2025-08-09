#!/usr/bin/env node

import { decode, type DecoderOpts } from '@webassemblyjs/wasm-parser'
import { readFile } from 'node:fs/promises'

import { parse } from '@webassemblyjs/wast-parser'

async function main(): Promise<void> {
	const binary = await readFile(
		'./static/build/ass_parser.wasm'
	)

	const decoderOpts: DecoderOpts = {
		dump: false,
		ignoreCodeSection: false,
		ignoreDataSection: true,
		ignoreCustomNameSection: true,
	}
	//const ast = decode(binary, decoderOpts)
	//console.log(ast)

	const textFormat = await readFile(
		'./static/build/ass_parser.wat'
	)

	const ast = parse(textFormat.toString())
}

void main()
