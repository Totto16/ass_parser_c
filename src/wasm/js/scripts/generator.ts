#!/usr/bin/env node

import { existsSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs'
import path from 'node:path'

import {
	BinaryReader,
	BinaryReaderState,
	ExternalKind,
	type IExportEntry,
	type IFunctionEntry,
	type ITypeEntry,
	type Type,
	type IImportEntry,
} from 'wasmparser'

import { TypeKind } from 'wasmparser/dist/cjs/WasmParser'

interface FunctionType {
	arguments: Type[]
	return?: Type | undefined
}

interface FunctionExport {
	name: string
	type: FunctionType
}

function isImportEntry(
	entry: IFunctionEntry | IImportEntry
): entry is IImportEntry {
	return (entry as { kind?: unknown }).kind !== undefined
}

// not typed that good in the ts types, so see:
// https://github.com/wasdk/wasmparser/blob/master/src/WasmParser.ts#L2046
// and https://github.com/wasdk/wasmparser/blob/master/src/WasmParser.ts#L1687
export interface IFunctionTypeEntry {
	form: TypeKind.func
	params: Type[]
	returns: Type[]
}

function getExports(data: NonSharedBuffer): Error | FunctionExport[] {
	const parser = new BinaryReader()
	parser.setData(data.buffer, 0, data.length)

	interface FunctionExportInternal {
		name: string
		index: number
	}

	const functionExports: FunctionExportInternal[] = []
	const typeEntries: ITypeEntry[] = []
	const functionList: (IFunctionEntry | IImportEntry)[] = []

	// eslint-disable-next-line @typescript-eslint/no-unnecessary-condition
	parse_loop: while (true) {
		if (!parser.read()) {
			return parser.error
		}

		switch (parser.state) {
			case BinaryReaderState.ERROR: {
				return parser.error
			}
			case BinaryReaderState.INITIAL: {
				break
			}
			case BinaryReaderState.BEGIN_WASM: {
				break
			}
			case BinaryReaderState.END_WASM: {
				break parse_loop
			}
			case BinaryReaderState.BEGIN_SECTION: {
				break
			}
			case BinaryReaderState.END_SECTION: {
				break
			}
			case BinaryReaderState.SKIPPING_SECTION: {
				break
			}
			case BinaryReaderState.READING_SECTION_RAW_DATA: {
				break
			}
			case BinaryReaderState.SECTION_RAW_DATA: {
				break
			}
			case BinaryReaderState.TYPE_SECTION_ENTRY: {
				const typeEntry = parser.result as ITypeEntry
				typeEntries.push(typeEntry)
				break
			}
			case BinaryReaderState.IMPORT_SECTION_ENTRY: {
				const importEntry = parser.result as IImportEntry

				if (importEntry.kind == ExternalKind.Function) {
					functionList.push(importEntry)
				}
				break
			}
			case BinaryReaderState.FUNCTION_SECTION_ENTRY: {
				const functionEntry = parser.result as IFunctionEntry
				functionList.push(functionEntry)
				break
			}
			case BinaryReaderState.TABLE_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.MEMORY_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.GLOBAL_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.EXPORT_SECTION_ENTRY: {
				const exportEntry = parser.result as IExportEntry

				if (exportEntry.kind === ExternalKind.Function) {
					functionExports.push({
						name: new TextDecoder().decode(exportEntry.field),
						index: exportEntry.index,
					})
				}
				break
			}
			case BinaryReaderState.DATA_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.NAME_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.ELEMENT_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.LINKING_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.START_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.TAG_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.BEGIN_INIT_EXPRESSION_BODY: {
				break
			}
			case BinaryReaderState.INIT_EXPRESSION_OPERATOR: {
				break
			}
			case BinaryReaderState.END_INIT_EXPRESSION_BODY: {
				break
			}
			case BinaryReaderState.BEGIN_FUNCTION_BODY: {
				break
			}
			case BinaryReaderState.READING_FUNCTION_HEADER: {
				break
			}
			case BinaryReaderState.CODE_OPERATOR: {
				break
			}
			case BinaryReaderState.END_FUNCTION_BODY: {
				break
			}
			case BinaryReaderState.SKIPPING_FUNCTION_BODY: {
				break
			}
			case BinaryReaderState.BEGIN_ELEMENT_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.ELEMENT_SECTION_ENTRY_BODY: {
				break
			}
			case BinaryReaderState.END_ELEMENT_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.BEGIN_DATA_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.DATA_SECTION_ENTRY_BODY: {
				break
			}
			case BinaryReaderState.END_DATA_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.BEGIN_GLOBAL_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.END_GLOBAL_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.RELOC_SECTION_HEADER: {
				break
			}
			case BinaryReaderState.RELOC_SECTION_ENTRY: {
				break
			}
			case BinaryReaderState.SOURCE_MAPPING_URL: {
				break
			}
			case BinaryReaderState.BEGIN_OFFSET_EXPRESSION_BODY: {
				break
			}
			case BinaryReaderState.OFFSET_EXPRESSION_OPERATOR: {
				break
			}
			case BinaryReaderState.END_OFFSET_EXPRESSION_BODY: {
				break
			}
			case BinaryReaderState.BEGIN_REC_GROUP: {
				break
			}
			case BinaryReaderState.END_REC_GROUP: {
				break
			}
			case BinaryReaderState.DATA_COUNT_SECTION_ENTRY: {
				break
			}
			default: {
				const _should_be_never: never = parser.state
				console.error(`Unknown parser state: `, parser.state)
				return new Error(
					`Unknown parser state: ${parser.state as string}`
				)
			}
		}
	}

	const results: FunctionExport[] = []

	for (const export_ of functionExports) {
		const functionEntry = functionList[export_.index]

		if (functionEntry === undefined) {
			return new Error(
				`function entry not found for function ${export_.name}`
			)
		}

		if (isImportEntry(functionEntry)) {
			return new Error(
				`index in function ${export_.name} refers to an imported function, but we only scan exported function, a function can't be exported and imported at the same time`
			)
		}
		const funcType = typeEntries[functionEntry.typeIndex]

		if (funcType === undefined) {
			return new Error(
				`function type not found for function ${export_.name}`
			)
		}

		// eslint-disable-next-line @typescript-eslint/no-unsafe-enum-comparison
		if (funcType.form != TypeKind.func) {
			return new Error(
				`resolved function type is not a correct type for function ${export_.name}`
			)
		}

		const funcTypeCorrect = funcType as IFunctionTypeEntry

		let returnType: Type | undefined = undefined

		if (funcTypeCorrect.returns.length == 0) {
			returnType = undefined
		} else if (funcTypeCorrect.returns.length == 1) {
			returnType = funcTypeCorrect.returns[0]
		} else {
			return new Error(
				`Multiple function return values is not yet supported!`
			)
		}

		const type: FunctionType = {
			arguments: funcTypeCorrect.params,
			return: returnType,
		}

		results.push({ name: export_.name, type })
	}

	return results
}

interface TSTypeRepr {
	typename: string
	underlying_type: string
	c_name: string
}

function wasmTypeToTSTypeRepr(type_: Type): TSTypeRepr {
	switch (type_.kind) {
		case TypeKind.i32:
			return {
				typename: 'I32',
				c_name: 'int32_t',
				underlying_type: 'number',
			}
		case TypeKind.i64:
			return {
				typename: 'I64',
				c_name: 'int64_t',
				underlying_type: 'bigint',
			}
		default:
			throw new Error(`Got unknown wasm type: ${type_.kind.toString()}`)
	}
}

function wasmTypeToTSTypeString(type_: Type): string {
	return wasmTypeToTSTypeRepr(type_).typename
}

interface TSFunctionParam {
	name: string
	value: string
}

function toTsType(export_: FunctionExport): string {
	const params: TSFunctionParam[] = export_.type.arguments.map(
		(arg, idx): TSFunctionParam => {
			const value = wasmTypeToTSTypeString(arg)

			return { name: `p_${idx.toString()}`, value }
		}
	)

	const functionParams: string = params
		.map(({ name, value }) => {
			return `${name}: ${value}`
		})
		.join(', ')

	let returnType = 'void'

	if (export_.type.return !== undefined) {
		returnType = wasmTypeToTSTypeString(export_.type.return)
	}

	return `${export_.name}: (${functionParams}) => ${returnType}`
}

function generateTypes(exports: FunctionExport[]): string[] {
	const neededTypes: Record<string, TSTypeRepr> = {}

	function addType(repr: TSTypeRepr): void {
		// eslint-disable-next-line @typescript-eslint/prefer-nullish-coalescing
		if (neededTypes[repr.typename] === undefined) {
			neededTypes[repr.typename] = repr
		}
	}

	for (const export_ of exports) {
		for (const p of export_.type.arguments) {
			addType(wasmTypeToTSTypeRepr(p))
		}

		if (export_.type.return !== undefined) {
			addType(wasmTypeToTSTypeRepr(export_.type.return))
		}
	}

	const result: string[] = []

	const generatedStructName = 'GeneratedCType'

	if (Object.entries(neededTypes).length > 0) {
		const dataToAdd = `interface ${generatedStructName}<Desc extends string, JSType> {
	readonly __marker: unique symbol
	readonly __type: JSType
	readonly __desc: Desc
}
	
export type GetJSTypeFrom${generatedStructName}<
	C extends ${generatedStructName}<string, unknown>,
> = C extends {
	readonly __type: infer JSType
}
	? JSType
	: never
`

		result.push(...dataToAdd.split('\n'))
	}

	for (const type of Object.values(neededTypes)) {
		const generatedType = `export type ${type.typename} = ${generatedStructName}<"${type.c_name}", ${type.underlying_type}>`

		result.push(generatedType)
	}

	return result
}

interface GenerateOptions {
	inputFile: string
	outputFile: string
}

function generateFiles(options: GenerateOptions): void {
	if (!existsSync(options.inputFile)) {
		throw new Error(`Input file doesn't exists: ${options.inputFile}`)
	}

	const data = readFileSync(options.inputFile)

	const exports = getExports(data)

	if (exports instanceof Error) {
		throw exports
	}

	const exportedFunctionTypes = exports.map((export_) => {
		return toTsType(export_)
	})

	const jsTypes = generateTypes(exports)

	const interface_ = `export interface ExportedFunctions {\n${exportedFunctionTypes.map((t) => `\t${t}`).join('\n')}\n}`

	const types = jsTypes.join('\n')

	const wholeType = `${types}\n\n${interface_}`

	const folder = path.dirname(options.outputFile)

	if (!existsSync(folder)) {
		mkdirSync(folder, { recursive: true })
	}

	writeFileSync(options.outputFile, wholeType)
}

function fileToAbsolute(file: string): string {
	if (path.isAbsolute(file)) {
		return file
	}

	return path.resolve(process.cwd(), file)
}

function main(): void {
	const options: GenerateOptions = {
		inputFile: '',
		outputFile: '',
	}

	// eslint-disable-next-line @typescript-eslint/prefer-for-of
	for (let i = 0; i < process.argv.length; ++i) {
		// eslint-disable-next-line @typescript-eslint/no-non-null-assertion
		const value = process.argv[i]!

		if (value.endsWith('node')) {
			continue
		}

		if (value.endsWith('.js')) {
			continue
		}

		if (value.endsWith('.ts')) {
			continue
		}

		if (value == '-o' || value == '--output') {
			if (i + 1 >= process.argv.length) {
				throw new Error(
					`Expected another argument for the output argument`
				)
			}

			// eslint-disable-next-line @typescript-eslint/no-non-null-assertion
			const output = fileToAbsolute(process.argv[i + 1]!)

			options.outputFile = output
			++i
			continue
		}

		if (value == '-i' || value == '--input') {
			if (i + 1 >= process.argv.length) {
				throw new Error(
					`Expected another argument for the input argument`
				)
			}

			// eslint-disable-next-line @typescript-eslint/no-non-null-assertion
			const input = fileToAbsolute(process.argv[i + 1]!)

			options.inputFile = input
			++i
			continue
		}

		if (value == '--ignore-after') {
			break
		}

		throw new Error(`Unrecognized argument: ${value}`)
	}

	if (options.inputFile == '') {
		throw new Error(`No input file given`)
	}

	if (options.outputFile == '') {
		throw new Error(`No output file given`)
	}

	generateFiles(options)
}

main()
