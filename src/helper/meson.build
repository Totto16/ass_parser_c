
lib_src_files += files(
    'assert.h',
    'assert.c',
    'io.c',
    'io.h',
    'macros.h',
    'message_struct.c',
    'message_struct.h',
    'sized_ptr.c',
    'sized_ptr.h',
    'string_view.c',
    'string_view.h',
    'utf_helper.c',
    'utf_helper.h',
)



install_headers(
    files('assert.h', 'io.h', 'message_struct.h', 'sized_ptr.h', 'string_view.h', 'utf_helper.h'),
    install_dir: install_include_dir / 'helper',
    preserve_path: true,
)
