import './styles/ui.css'

export type FileCB = (file: File) => void

export function uiStart(onFileCb: FileCB): void {
	const fileUploadButton = document.getElementById('file-upload-btn')

	if (!fileUploadButton) {
		throw new Error('UI error')
	}

	const fileUploadInput = document.getElementById(
		'file-upload-input'
	) as HTMLInputElement | null

	if (!fileUploadInput) {
		throw new Error('UI error')
	}

	fileUploadButton.addEventListener('click', () => {
		fileUploadInput.click()
	})

	fileUploadInput.addEventListener('change', () => {
		readFileURL(fileUploadInput, onFileCb)
	})
}

function readFileURL(input: HTMLInputElement, onFileCb: FileCB): void {
	if (input.files?.[0]) {
		onFileCb(input.files[0])
	} else {
		console.error('file removed')
	}
}
