import { WasmBinding } from './wasm'

import { uiStart } from './ui'

async function process_file(instance: WasmBinding, file: File): Promise<void> {
	console.log('process_file', file)

	console.log(instance.allocator_get_statistics())
	using result = await instance.parse_ass(file, {
		strict_settings: {
			allow_unrecognized_file_encoding: true,
		},
	})

	//TODO
	console.log('result', result)

	console.log('is result error: ', result.is_error())

	const diagnostics = result.diagnostics()

	for (const diagnostic of diagnostics) {
		console.log('diagnostic', diagnostic)
	}

	console.log(diagnostics.length)
	if (diagnostics.length > 0) {
		console.log('diagnostics 0', diagnostics[0])
	}

	console.log(instance.allocator_get_statistics())
}

async function main(): Promise<void> {
	const instance = await WasmBinding.getInstance('/static/')

	uiStart((file) => {
		void process_file(instance, file)
	})
}

void main()
