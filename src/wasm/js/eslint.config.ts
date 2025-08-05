import eslint from '@eslint/js'
import tseslint from 'typescript-eslint'
import globals from 'globals'

export default [
	{
		plugins: {
			tseslint: tseslint.plugin,
			eslint: eslint,
		},
	},
	{
		ignores: ['dist/**'],
	},
	eslint.configs.recommended,
	...tseslint.configs.strictTypeChecked,
	...tseslint.configs.stylisticTypeChecked,
	...tseslint.configs.recommendedTypeChecked,
	{
		languageOptions: {
			parserOptions: {
				project: './tsconfig.json',
				projectService: true,
				warnOnUnsupportedTypeScriptVersion: true,
			},
		},
	},
	{
		rules: {
			'@typescript-eslint/no-unused-vars': [
				'error',
				{
					args: 'all',
					argsIgnorePattern: '^_',
					caughtErrors: 'all',
					caughtErrorsIgnorePattern: '^_',
					destructuredArrayIgnorePattern: '^_',
					varsIgnorePattern: '^_',
					ignoreRestSiblings: true,
				},
			],
			'@typescript-eslint/no-non-null-assertion': ['error'],
			curly: ['error', 'all'],
			'@typescript-eslint/explicit-function-return-type': 'error',
			'@typescript-eslint/consistent-type-exports': 'error',
			'@typescript-eslint/consistent-type-imports': 'error',
		},
	},
	{
		files: ['src/*.ts', 'index.d.ts', 'webpack.config.ts'],
		languageOptions: {
			parserOptions: {
				project: './tsconfig.json',
			},
		},
	},
]
