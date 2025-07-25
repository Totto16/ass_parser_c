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
    version: '0.0.3',
)

deps = []

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

lib_src_files = []
cli_files = []
inc_dirs = []
lib_inc_dirs = []

# so that the version of the lib can be different than the cli version
ass_parser_lib_version = '0.0.3'

ass_parser_lib_header_install_dir = 'ass_parser'

subdir('src')

ass_parser_lib = library(
    'ass_parser',
    lib_src_files,
    dependencies: deps,
    include_directories: inc_dirs,
    c_args: ['-DLIB_VERSION_STRING="' + ass_parser_lib_version + '"'],
    install: true,
    version: ass_parser_lib_version,
)

pkg = import('pkgconfig')

pkg.generate(
    ass_parser_lib,
    description: 'An ass parser library',
    name: 'ass_parser',
    filebase: 'ass_parser',
    subdirs: [
        ass_parser_lib_header_install_dir / 'public',
        ass_parser_lib_header_install_dir / 'thirdparty',
    ],
)

ass_parser_dep = declare_dependency(
    link_with: ass_parser_lib,
    include_directories: lib_inc_dirs,
    dependencies: deps,
    version: ass_parser_lib_version,
)

executable(
    'ass_parser',
    cli_files,
    dependencies: [ass_parser_dep],
    install: true,
    c_args: ['-DPROGRAM_NAME=' + meson.project_name()],
)
