project(
    'ass custom_wasm_libc',
    'c',
    license: 'MIT',
    meson_version: '>=1.3.0',
    default_options: {
        'buildtype': 'release',
        'optimization': '3',
        'c_std': ['c2y', 'c23', 'c2x'],
        'warning_level': '3',
        'werror': 'true',
        'b_ndebug': 'if-release',
    },
    version: '0.0.0',
)

custom_wasm_libc_src_files = files(
    'thirdparty/walloc.c',
    'assert.h',
    'limits.h',
    'platform.h',
    'stdbool.h',
    'stddef.h',
    'stdint.h',
    'stdlib.h',
    'string.h',
)

wasm_inc_dirs = include_directories('.')

wasm_libc_version = meson.project_version()

custom_wasm_libc_lib = library(
    'custom_wasm_libc',
    custom_wasm_libc_src_files,
    include_directories: wasm_inc_dirs,
    version: wasm_libc_version,
)


custom_wasm_libc_dep = declare_dependency(
    link_with: custom_wasm_libc_lib,
    include_directories: wasm_inc_dirs,
    version: wasm_libc_version,
)

meson.override_dependency('custom_wasm_libc', custom_wasm_libc_dep)
meson.override_dependency('c_stdlib', custom_wasm_libc_dep)
