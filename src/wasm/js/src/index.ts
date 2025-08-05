import { WasmBinding } from './wasm'

async function main(): Promise<void> {
	const instance = await WasmBinding.getInstance('/static/')

	console.log(instance.allocator_get_statistics())

	console.log(instance.parse_ass('', { todo: 0 }))

	console.log(instance.allocator_get_statistics())
}

void main().catch((e) => {
	console.error('Error in main function', e)
})
