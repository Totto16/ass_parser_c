project(
    'ass parser',
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
    version: '3.4.0',
)

deps = []
compile_flags = []

src_files = []
inc_dirs = []

subdir('src')

ass_parser_lib = library(
    'ass_parser',
    src_files,
    dependencies: deps,
    include_directories: inc_dirs,
    c_args: compile_flags,
)

ass_parser_dep = declare_dependency(
    link_with: ass_parser_lib,
    include_directories: inc_dirs,
    compile_args: compile_flags,
    dependencies: deps,
    version: meson.project_version(),
)

executable(
    'ass_parser',
    files('src/main.c'),
    dependencies: [ass_parser_dep],
)
