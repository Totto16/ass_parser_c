import { FontPreset, WasmBinding } from './wasm'

import { uiStart } from './ui'

async function process_file(instance: WasmBinding, file: File): Promise<void> {
	console.log('process_file', file)

	console.log(instance.allocator_get_statistics())
	using result = await instance.parse_ass(file, {
		strict_settings: {
			allow_unrecognized_file_encoding: true,
		},
		validate_settings: {
			font_settings: {
				preset: FontPreset.disabled,
			},
		},
	})

	console.log('result', result)

	console.log('is result error: ', result.is_error())

	const diagnostics = result.diagnostics()

	console.log('diagnostics length', diagnostics.length)
	for (const diagnostic of diagnostics) {
		console.log('diagnostic', diagnostic)
	}

	//TODO: display diagnostics and result / error in case
	if (result.is_error()) {
		console.error('The result is an error')
	} else {
		const assResult = result.result()
		console.log(assResult)
	}
}

async function main(): Promise<void> {
	const instance = await WasmBinding.getInstance('/static/')

	uiStart((file) => {
		void process_file(instance, file).then(() => {
			console.log('at end')
			console.log(instance.allocator_get_statistics())
		})
	})
}

void main()
