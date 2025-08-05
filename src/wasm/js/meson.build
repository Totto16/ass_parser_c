

node = find_program('node')

custom_target(
    'ass_parser.wasm',
    command: [node, '@CURRENT_SOURCE_DIR@/scripts/meson_copy.js', '@INPUT0@', '@OUTPUT@'],
    build_by_default: true,
    output: ['ass_parser.wasm'],
    input: [wasm_file],
    depends: [wasm_file],
)

if have_wat_file

    custom_target(
        'ass_parser.wat',
        command: [
            node,
            '@CURRENT_SOURCE_DIR@/scripts/meson_copy.js',
            '@INPUT0@',
            '@OUTPUT@',
        ],
        build_by_default: true,
        output: ['ass_parser.wat'],
        input: [wat_file],
        depends: [wat_file],
    )

endif
