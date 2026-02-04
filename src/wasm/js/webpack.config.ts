import path from 'path'

import type * as webpack from 'webpack'
import 'webpack-dev-server'

import ESLintPlugin from 'eslint-webpack-plugin'
import HtmlWebpackPlugin from 'html-webpack-plugin'
import CssMinimizerPlugin from 'css-minimizer-webpack-plugin'
import MiniCssExtractPlugin from 'mini-css-extract-plugin'
import CopyWebpackPlugin from 'copy-webpack-plugin'

interface EnvObject {
	WEBPACK_BUNDLE: boolean
	WEBPACK_BUILD: boolean
}

type Mode = 'development' | 'production' | 'none'

interface Arguments {
	mode: Mode
	env: EnvObject
}

function retrieveConfig(
	_env: EnvObject,
	argv: Arguments
): webpack.Configuration {
	const ENTRY_PATH = path.resolve(__dirname, 'src/index')
	const DIST_PATH = path.resolve(__dirname, 'dist')

	console.info(`Using ${argv.mode} environment`)

	const production = argv.mode === 'production'

	const config: webpack.Configuration = {
		entry: {
			main: ENTRY_PATH,
		},
		output: {
			path: DIST_PATH,
			filename: '[name].[contenthash].js',
			clean: true,
		},
		module: {
			rules: [
				{
					test: /spec\.ts$/,
					loader: 'ignore-loader',
					exclude: /node_modules/,
				},

				{
					test: /\.ts$/,
					use: [
						{
							loader: 'ts-loader',
							options: {
								configFile: path.resolve(
									__dirname,
									'tsconfig.json'
								),
							},
						},
					],
					exclude: /node_modules/,
				},
				{
					test: /\.js$/,
					use: [],
					exclude: /node_modules/,
				},
				{
					test: /\.css$/,
					use: [MiniCssExtractPlugin.loader, 'css-loader'],
				},
				{ test: /\.hbs$/, loader: 'handlebars-loader' },
			],
		},
		resolve: {
			extensions: ['.ts', '.js'],
		},
		plugins: [
			new HtmlWebpackPlugin({
				filename: 'index.html',
				template: path.resolve(__dirname, 'static/index.html'),
			}),
			new MiniCssExtractPlugin({
				filename: '[name].css',
				chunkFilename: '[id].css',
			}),
			new ESLintPlugin({
				cwd: __dirname,
				configType: 'flat',
				failOnError: true,
				failOnWarning: false,
				extensions: ['ts', 'js'],
			}),
			new CopyWebpackPlugin({
				patterns: [
					{
						from: path.resolve(__dirname, 'static/build'),
						to: path.resolve(__dirname, DIST_PATH, 'static'),
						noErrorOnMissing: true, // Optional: ignore missing source folder
					},
				],
			}),
		],
		devtool: production ? false : 'inline-source-map',
		devServer: {
			static: DIST_PATH,
			hot: true,
		},
		optimization: {
			minimizer: [
				`...`,
				new CssMinimizerPlugin({
					minimizerOptions: {
						preset: [
							'default',
							{
								discardComments: { removeAll: true },
							},
						],
					},
				}),
			],
			splitChunks: {
				chunks: 'all',
			},
		},
	}

	return config
}

export default retrieveConfig
