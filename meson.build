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
    version: '0.0.1',
)

deps = []
compile_flags = [
    '-DPROGRAM_NAME=' + meson.project_name(),
    '-DVERSION_STRING=' + meson.project_version(),
]

deps += dependency(
    'libutf8proc',
    allow_fallback: true,
    default_options: {
        'tests': 'disabled',
    },
)

cc = meson.get_compiler('c')

deps += cc.find_library(
    'm',
    required: false,
)

deps += dependency('threads')


iconv_dep = cc.find_library(
    'iconv',
    required: false,
)
if iconv_dep.found()
    deps += iconv_dep
elif cc.has_function('iconv_open')
    message('iconv already provided by libc')
else
    deps += dependency('iconv')
endif

src_files = []
cli_files = []
inc_dirs = []

subdir('src')

ass_parser_lib = library(
    'ass_parser',
    src_files,
    dependencies: deps,
    include_directories: inc_dirs,
    c_args: compile_flags,
    install: true,
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
    cli_files,
    dependencies: [ass_parser_dep],
    install: true,
)
