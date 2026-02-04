import eslint from '@eslint/js'
import tseslint from 'typescript-eslint'

export default [
	{
		plugins: {
			tseslint: tseslint.plugin,
			eslint: eslint,
		},
	},
	{
		ignores: ['dist/**', 'scripts/meson_copy.js'],
	},
	eslint.configs.recommended,
	...tseslint.configs.strictTypeChecked,
	...tseslint.configs.stylisticTypeChecked,
	...tseslint.configs.recommendedTypeChecked,
	{
		linterOptions: {
			reportUnusedDisableDirectives: 'error',
		},
		languageOptions: {
			parserOptions: {
				project: './tsconfig.json',
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
	{
		files: ['scripts/generator.ts'],
		languageOptions: {
			parserOptions: {
				project: './tsconfig.script.json',
			},
		},
		rules: {
			camelcase: [
				'error',
				{
					properties: 'always',
					ignoreDestructuring: true,
					ignoreImports: true,
					ignoreGlobals: true,
					allow: ['_should_be_never', 'is_free_fn'],
				},
			],
		},
	},
]
