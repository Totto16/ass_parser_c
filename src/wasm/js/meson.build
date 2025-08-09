

node = find_program('node')

wasm_file_for_js = custom_target(
    'ass_parser.wasm',
    command: [node, '@CURRENT_SOURCE_DIR@/scripts/meson_copy.js', '@INPUT@', '@OUTPUT@'],
    build_by_default: true,
    output: ['ass_parser.wasm'],
    input: [wasm_file],
    depends: [wasm_file],
)

if have_wat_file

    custom_target(
        'ass_parser.wat',
        command: [node, '@CURRENT_SOURCE_DIR@/scripts/meson_copy.js', '@INPUT@', '@OUTPUT@'],
        build_by_default: true,
        output: ['ass_parser.wat'],
        input: [wat_file],
        depends: [wat_file],
    )

endif

pnpm = find_program(
    'pnpm',
    required: false,
)

if pnpm.found()

    custom_target(
        'wasm_exports.ts',
        command: [
            pnpm,
            '--dir',
            '@CURRENT_SOURCE_DIR@',
            'run',
            'generate',
            '--ignore-after',
            '@CURRENT_SOURCE_DIR@/scripts/generator.ts',
            '-i',
            '@INPUT@',
            '-o',
            '@OUTPUT@',
        ],
        build_by_default: true,
        output: ['wasm_exports.ts'],
        input: [wasm_file],
        depends: [wasm_file, wasm_file_for_js],
    )

endif
