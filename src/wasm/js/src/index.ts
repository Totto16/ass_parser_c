import { WasmBinding } from './wasm'

import { uiStart } from './ui'

function process_file(instance: WasmBinding, file: File): void {
	console.log(instance.allocator_get_statistics())
	const result = instance.parse_ass(file, { todo: 0 })

	console.log(instance.allocator_get_statistics())
}

async function main(): Promise<void> {
	const instance = await WasmBinding.getInstance('/static/')

	uiStart((file) => {
		process_file(instance, file)
	})
}

void main()
