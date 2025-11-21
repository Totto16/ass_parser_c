#!/usr/bin/env node

import { existsSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs'
import path from 'node:path'

import { TypeKind } from 'wasmparser/dist/cjs/WasmParser'

import {
	BinaryReader,
	BinaryReaderState,
	ExternalKind,
	type IExportEntry,
	type IFunctionEntry,
	type ITypeEntry,
	type Type,
	type IImportEntry,
	type ISectionInformation,
	SectionCode,
} from 'wasmparser'

interface FunctionType {
	arguments: Type[]
	return?: Type | undefined
}

interface FunctionExport {
	name: string
	type: FunctionType
	annotations: Annotation[]
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

interface CustomSection {
	data: Uint8Array
}

interface Annotation {
	name: string
	params: string[]
}

function get_annotations(
	customSections: Record<string, CustomSection>
): Record<number, Annotation[]> {
	const annotations: Record<number, Annotation[]> = {}

	for (const [sectionName, section] of Object.entries(customSections)) {
		// TODO: parse dwarf
		// DW_TAG_subprogram (means function)
		// DW_AT_name (has the name)
		// DW_AT_type (points to the type)

		// e.g. DW_TAG_pointer_type

		if (sectionName.startsWith('llvm.func_attr.annotate.')) {
			if (section.data.byteLength % 4 != 0) {
				throw new Error(
					'annotation section has not uint32_t as values!'
				)
			}
			const data_view = new DataView(
				section.data.buffer,
				section.data.byteOffset,
				section.data.byteLength
			)

			const fullName = sectionName.replace('llvm.func_attr.annotate.', '')

			const parts = fullName.split(':')

			// eslint-disable-next-line @typescript-eslint/no-non-null-assertion
			const name = parts[0]!

			const params = parts.slice(1)

			const entry: Annotation = {
				name,
				params,
			}

			for (let i = 0; i < section.data.byteLength / 4; ++i) {
				const index = data_view.getUint32(i * 4, true)

				// eslint-disable-next-line @typescript-eslint/prefer-nullish-coalescing
				if (annotations[index] === undefined) {
					annotations[index] = []
				}
				annotations[index].push(entry)
			}
		}
	}

	return annotations
}

//Note: https://github.com/wasdk/wasmparser is old, and still uses NonSharedBuffer
// eslint-disable-next-line @typescript-eslint/no-deprecated
function getExports(data: NonSharedBuffer): Error | FunctionExport[] {
	const parser = new BinaryReader()
	parser.setData(data.buffer, 0, data.length)

	interface FunctionExportInternal {
		name: string
		index: number
	}

	type ISectionInformationFixed =
		| { id: SectionCode.Custom; name: Uint8Array }
		| { id: SectionCode; name: null }

	interface CustomSectionInfo {
		info: ISectionInformationFixed | null
		data: Uint8Array | null
	}

	const functionExports: FunctionExportInternal[] = []
	const typeEntries: ITypeEntry[] = []
	const functionList: (IFunctionEntry | IImportEntry)[] = []

	const currentSectionInfo: CustomSectionInfo = { data: null, info: null }

	const customSections: Record<string, CustomSection> = {}

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
				currentSectionInfo.info =
					parser.result as ISectionInformation as ISectionInformationFixed
				break
			}
			case BinaryReaderState.END_SECTION: {
				if (currentSectionInfo.info == null) {
					return new Error(
						'Parser error: section end before section begin?'
					)
				}

				if (currentSectionInfo.info.id == SectionCode.Custom) {
					const name = new TextDecoder().decode(
						currentSectionInfo.info.name
					)

					if (name != 'name') {
						if (currentSectionInfo.data == null) {
							return new Error(
								'Parser error: section end before section read: ' +
									name
							)
						}

						const customSection: CustomSection = {
							data: currentSectionInfo.data,
						}

						customSections[name] = customSection
					}
				}

				currentSectionInfo.data = null
				currentSectionInfo.info = null
				break
			}
			case BinaryReaderState.SKIPPING_SECTION: {
				break
			}
			case BinaryReaderState.READING_SECTION_RAW_DATA: {
				break
			}
			case BinaryReaderState.SECTION_RAW_DATA: {
				if (currentSectionInfo.info == null) {
					return new Error(
						'Parser error: section read before section begin?'
					)
				}

				currentSectionInfo.data = parser.result as Uint8Array | null
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

	const annotations: Record<number, Annotation[]> =
		get_annotations(customSections)

	for (const export_ of functionExports) {
		const functionEntry = functionList[export_.index]

		if (functionEntry === undefined) {
			return new Error(
				`function entry not found for function ${export_.name}`
			)
		}

		const funcAnnotations: Annotation[] =
			annotations[export_.index] === undefined
				? []
				: // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
					annotations[export_.index]!

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

		results.push({ name: export_.name, type, annotations: funcAnnotations })
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

function get_enum_wrapper_type(enum_category: EnumCategory): string {
	return `"${enum_category.c_name}", ${enum_category.underlying_type}`
}

function wasmTypeToTSTypeString(
	type_: Type,
	category: Category | null
): string {
	if (category === null) {
		return wasmTypeToTSTypeRepr(type_).typename
	}

	let wrapper = ''
	let inner_type = wasmTypeToTSTypeRepr(type_).typename

	switch (category.type) {
		case 'enum': {
			wrapper = 'EnumWrapper'
			inner_type = get_enum_wrapper_type(category)
			break
		}
		case 'literal': {
			wrapper = ''
			break
		}
		case 'pointer': {
			wrapper = 'PtrWrapper'
			break
		}
		case 'void': {
			throw new Error('void category not expected here')
		}
		default: {
			throw new Error(
				`Got unknown category type: ${category as unknown as string}`
			)
		}
	}

	if (wrapper === '') {
		return inner_type
	}

	return `${wrapper}<${inner_type}>`
}

interface TSFunctionParam {
	name: string
	value: string
}

interface Ok<V> {
	ok: true
	value: V
}
interface Err<E> {
	ok: false
	error: E
}

type Result<V, E> = Ok<V> | Err<E>

function isOk<V, E>(res: Result<V, E>): res is Ok<V> {
	return res.ok
}

function isErr<V, E>(res: Result<V, E>): res is Err<E> {
	return !isOk(res)
}

function makeErr<V, E>(error: E): Result<V, E> {
	return { ok: false, error }
}

function makeOk<V, E>(value: V): Result<V, E> {
	return { ok: true, value }
}

function getErr<E>(value: Err<E>): E {
	return value.error
}

function getOk<V>(value: Ok<V>): V {
	return value.value
}

type CategoryType = 'literal' | 'pointer' | 'void' | 'enum'

interface EnumCategory {
	type: 'enum'
	c_name: string
	underlying_type: string
}

interface NormalCategory {
	type: Exclude<CategoryType, 'enum'>
}

type Category = NormalCategory | EnumCategory

function parseCategoryType(inp: string): CategoryType | null {
	switch (inp.toLowerCase()) {
		case 'literal': {
			return 'literal'
		}
		case 'pointer': {
			return 'pointer'
		}
		case 'void': {
			return 'void'
		}
		case 'enum': {
			return 'enum'
		}
		default: {
			return null
		}
	}
}

function getUniqueItems<T>(array: T[]): T[] {
	return [...new Set(array)]
}

function paramsToString(params: ParamsRestriction): string {
	if (typeof params === 'number') {
		return params.toString()
	}

	const [start, end] = params

	if (end === null) {
		return `[${start.toString()}, ...]`
	}

	return `[${start.toString()}, ${end.toString()}]`
}

function isValidParamsRestriction(
	length: number,
	params: ParamsRestriction
): boolean {
	if (typeof params === 'number') {
		return length === params
	}

	const [start, end] = params

	if (length < start) {
		return false
	}

	if (end !== null && end > length) {
		return false
	}

	return true
}

function findOneAnnotation(
	annotation_setting: AnnotationSettingSkip,
	annotations: Annotation[]
): Result<string[], string> {
	let collected: string[][] = []

	for (const annotation of annotations) {
		if (annotation.name === annotation_setting.name) {
			if (
				!isValidParamsRestriction(
					annotation.params.length,
					annotation_setting.params
				)
			) {
				return makeErr(
					`annotation '${annotation_setting.name}' needs ${paramsToString(annotation_setting.params)} arguments, but ${annotation.params.length.toString()} given`
				)
			}
			// eslint-disable-next-line @typescript-eslint/no-non-null-assertion
			collected.push(annotation.params)
		}
	}

	if (collected.length === 0) {
		return makeErr(
			`no '${annotation_setting.name}' annotation found, that is required`
		)
	}

	collected = getUniqueItems(collected)

	if (collected.length !== 1) {
		return makeErr(
			`annotation '${annotation_setting.name}' to often specified for function: ${collected.join(', ')}`
		)
	}

	// eslint-disable-next-line @typescript-eslint/no-non-null-assertion
	const result = collected[0]!

	return makeOk(result)
}

function c_underlying_type_to_js_string(type_: string): Result<string, string> {
	switch (type_) {
		case 'uint8_t':
			return makeOk('UInt8T')

		default:
			return makeErr(`Got unknown C underlying type: ${type_}`)
	}
}

function parse_catgeory_enum(
	annotations: Annotation[]
): Result<EnumCategory, string> {
	const categoryExtensionResult = findOneAnnotation(
		categoryExtensionAnnotation,
		annotations
	)

	if (isErr(categoryExtensionResult)) {
		return categoryExtensionResult
	}

	const [enum_name, c_name, underlying_type_raw] = assertArrayLen(
		getOk(categoryExtensionResult),
		3
	) as [string, string, string]

	if (enum_name !== 'enum') {
		return makeErr(
			`first argument for '${categoryExtensionAnnotation.name}' annotation needs to be 'enum' for a enum 'category'`
		)
	}

	const underlying_typeResult =
		c_underlying_type_to_js_string(underlying_type_raw)

	if (isErr(underlying_typeResult)) {
		return underlying_typeResult
	}

	const underlying_type = getOk(underlying_typeResult)

	const cat: EnumCategory = {
		type: 'enum',
		c_name,
		underlying_type,
	}

	return makeOk(cat)
}

function assertArrayLen<A = unknown>(
	array: A[],
	params: ParamsRestriction
): A[] {
	if (!isValidParamsRestriction(array.length, params)) {
		throw new Array(
			`Array has not expected length of ${paramsToString(params)} but has length ${array.length}`
		)
	}

	return array
}

function getCategory(annotations: Annotation[]): Result<Category, string> {
	const categoryResult = findOneAnnotation(categoryAnnotation, annotations)

	if (isErr(categoryResult)) {
		return categoryResult
	}

	const category = assertArrayLen(getOk(categoryResult), 1)[0]!

	const parsedCategoryType = parseCategoryType(category)

	if (parsedCategoryType === null) {
		return makeErr(`failed to parse category '${category}'`)
	}

	let parsedCategory: Category

	if (parsedCategoryType !== 'enum') {
		parsedCategory = {
			type: parsedCategoryType,
		} as NormalCategory
	} else {
		const parsedCategoryEnum = parse_catgeory_enum(annotations)

		if (isErr(parsedCategoryEnum)) {
			return makeErr(
				`failed to parse 'enum' category: ${getErr(parsedCategoryEnum)}`
			)
		}

		parsedCategory = getOk(parsedCategoryEnum)
	}

	return makeOk(parsedCategory)
}

interface TypeError {
	name: string
	message: string
}

function toTsType(export_: FunctionExport): Result<string, TypeError> {
	const params: TSFunctionParam[] = export_.type.arguments.map(
		(arg, idx): TSFunctionParam => {
			const value = wasmTypeToTSTypeString(arg, null)

			return { name: `p_${idx.toString()}`, value }
		}
	)

	const functionParams: string = params
		.map(({ name, value }) => {
			return `${name}: ${value}`
		})
		.join(', ')

	let returnType = 'void'

	const category_result = getCategory(export_.annotations)

	if (isErr(category_result)) {
		return makeErr({
			name: export_.name,
			message: getErr(category_result),
		})
	}

	const category: Category = getOk(category_result)

	if (export_.type.return !== undefined) {
		returnType = wasmTypeToTSTypeString(export_.type.return, category)
	} else {
		if (category.type !== 'void') {
			return makeErr({
				name: export_.name,
				message: `expected void function ot have category void, but got: ${category.type}`,
			})
		}
	}

	const annotations: Annotations = getDefaultAnnotations()

	ann_loop: for (const annotation of export_.annotations) {
		for (const globalAnn of globalAnnotations) {
			if (annotation.name === globalAnn.name) {
				if (
					!isValidParamsRestriction(
						annotation.params.length,
						globalAnn.params
					)
				) {
					throw new Error(
						`The ${globalAnn.name} annotation needs ${paramsToString(globalAnn.params)} arguments, but ${annotation.params.length.toString()} given`
					)
				}

				if (isSkipAnnotation(globalAnn)) {
					continue ann_loop
				}

				const annotType: string =
					annotation.params.length == 0
						? globalAnn.typename
						: `${globalAnn.typename}<${annotation.params.map((p) => `"${p}"`).join(', ')}>`

				annotations[globalAnn.name] = annotType

				continue ann_loop
			}
		}

		return makeErr({
			name: export_.name,
			message: `Unrecognized annotation: ${annotation.name}`,
		})
	}

	if (!areDefaultAnnotations(annotations)) {
		const keysInOrder: (keyof Annotations)[] = [
			'malloced',
			'string',
			'is_free_fn',
			'nullable',
		]
		const annotValues: string[] = keysInOrder.map((key) => annotations[key])
		returnType = `Annotated<${returnType}, Annotations<${annotValues.join(', ')}>>`
	}

	return makeOk(`${export_.name}: (${functionParams}) => ${returnType}`)
}

interface Annotations {
	malloced: string
	string: string
	is_free_fn: string
	nullable: string
}

type ParamsRestriction = number | [number, number | null]

interface AnnotationSettingNormal {
	name: keyof Annotations
	params: ParamsRestriction
	typename: string
}

interface AnnotationSettingSkip {
	name: string
	params: ParamsRestriction
	skip: boolean
}

const categoryAnnotation: AnnotationSettingSkip = {
	name: 'category',
	params: 1,
	skip: true,
}
const categoryExtensionAnnotation: AnnotationSettingSkip = {
	name: 'category_extension',
	params: [3, null],
	skip: true,
}

// eslint-disable-next-line @typescript-eslint/array-type
const globalAnnotations: Array<
	AnnotationSettingNormal | AnnotationSettingSkip
> = [
	{
		name: 'malloced',
		params: 1,
		typename: 'Malloced',
	},
	{
		name: 'string',
		params: 0,
		typename: 'IsCString',
	},
	{
		name: 'is_free_fn',
		params: 0,
		typename: 'IsFreeFn',
	},
	{
		name: 'nullable',
		params: 0,
		typename: 'IsNullable',
	},
	categoryAnnotation,
	categoryExtensionAnnotation,
]

function isSkipAnnotation(
	annotation: AnnotationSettingNormal | AnnotationSettingSkip
): annotation is AnnotationSettingSkip {
	return (annotation as { skip?: boolean | undefined }).skip !== undefined
}

function getDefaultAnnotations(): Annotations {
	const annotations: Annotations = {
		malloced: 'NoAnnot<"malloced">',
		string: 'NoAnnot<"cstr">',
		is_free_fn: 'NoAnnot<"free_fn">',
		nullable: 'NoAnnot<"nullable">',
	}

	return annotations
}

function areDefaultAnnotations(annotations: Annotations): boolean {
	for (const key of Object.keys(annotations)) {
		const val = annotations[key as keyof Annotations]
		if (/NoAnnot<".*">/.exec(val) == null) {
			return false
		}
	}

	return true
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

	const generatedStructName = 'CTypeSimple'

	if (Object.entries(neededTypes).length > 0) {
		const dataToAdd = `import type { Annotated, Annotations, CEnum, CType, CTypeSimple, IsCString, IsFreeFn, IsNullable, Malloced, NoAnnot, Ptr, UInt8T } from '../c/types'

`

		result.push(...dataToAdd.split('\n'))
	}

	for (const type of Object.values(neededTypes)) {
		const generatedType = `export type ${type.typename} = ${generatedStructName}<"${type.c_name}", ${type.underlying_type}>`

		result.push(generatedType)
	}

	result.push(`

export type CategoryWrapper<T extends CType, Desc extends string> = {
	readonly __wrapper: '__generated_from_category'
	readonly __wrapper_type: Desc
} & T

export type PtrWrapper<T extends CType> = CategoryWrapper<Ptr<T>, 'ptr'>
type JSEnumWrapper = number 
export type EnumWrapper<
	CName extends string,
	UnderlyingType extends CType,
> = CategoryWrapper<CEnum<JSEnumWrapper, CName, UnderlyingType>, 'enum'>

`)

	return result
}

interface GenerateOptions {
	inputFile: string
	outputFile: string
}

function results_map_get_values<V, E>(
	arr: Result<V, E>[]
): [values: V[], errors: E[]] {
	const result: [values: V[], errors: E[]] = [[], []]

	for (const value of arr) {
		if (isErr(value)) {
			result[1].push(getErr(value))
		} else {
			result[0].push(getOk(value))
		}
	}

	return result
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

	const exportedFunctionResults = exports.map((export_) => {
		return toTsType(export_)
	})

	const [exportedFunctionTypes, errors] = results_map_get_values(
		exportedFunctionResults
	)

	if (errors.length !== 0) {
		for (const err of errors) {
			console.error(
				`An error occurred, while handling function ${err.name}: ${err.message}`
			)
		}
		throw new Error('Errors occurred, see above')
	}

	const jsTypes = generateTypes(exports)

	const interface_ = `export interface GeneratedExportedFunctions {\n${exportedFunctionTypes.map((t) => `\t${t}`).join('\n')}\n}`

	const types = jsTypes.join('\n')

	const wholeType = `${types}\n\n${interface_}\n`

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
