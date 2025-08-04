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

is_wasm = false

if meson.is_cross_build()
    if host_machine.system() == 'wasm'
        is_wasm = true
    else
        error('Not supported cross build system: ' + host_machine.system())
    endif
endif


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

internal_c_args = []

internal_c_args += cc.get_supported_arguments(
    '-Wassign-enum',
    '-Wenum-conversion',
    '-Wenum-switch',
    '-fstrict-enums',
    '-Wenum-identifier',
    '-Wenum-compare',
    '-Wenum-enum-conversion',
    '-Wenum-float-conversion',
    '-Wenum-too-large',
    '-Wenum-compare-switch',
    '-Wenum-compare-conditional',
)

iconv_dep = cc.find_library(
    'iconv',
    required: false,
)
if iconv_dep.found()
    internal_c_args += '-DASS_PARSER_HAVE_ICONV'
    deps += iconv_dep
elif cc.has_function('iconv_open')
    message('iconv already provided by libc')
    internal_c_args += '-DASS_PARSER_HAVE_ICONV'
else
    iconv_dep = dependency('iconv', required: not is_wasm)
    if iconv_dep.found()
        internal_c_args += '-DASS_PARSER_HAVE_ICONV'
        deps += iconv_dep
    endif
endif

if not is_wasm

    deps += dependency(
        'fontconfig',
        allow_fallback: true,
    )
    internal_c_args += '-DASS_PARSER_HAVE_FONTCONFIG'

    deps += dependency(
        'freetype2',
        allow_fallback: true,
    )
    internal_c_args += '-DASS_PARSER_HAVE_FREETYPE'
endif


lib_src_files = []
cli_files = []
inc_dirs = []
lib_inc_dirs = []

ass_parser_lib_version = meson.project_version()

ass_parser_lib_header_install_dir = 'ass_parser'

subdir('src')


if is_wasm

    executable(
        'ass_parser',
        lib_src_files,
        dependencies: deps,
        include_directories: inc_dirs,
        install: true,
        c_args: [
            '-DASS_PARSER_VERSION_STRING="' + ass_parser_lib_version + '"',
            internal_c_args,
        ],
        link_args: [
            '--export-all',
            '--no-entry',
            '--allow-undefined-file=' + meson.project_source_root() / 'cross/wasm-external-symbols.txt',
            '--import-memory',
        ],
        name_suffix: 'wasm',
    )

else

    ass_parser_lib = library(
        'ass_parser',
        lib_src_files,
        dependencies: deps,
        include_directories: inc_dirs,
        c_args: [
            '-DASS_PARSER_VERSION_STRING="' + ass_parser_lib_version + '"',
            internal_c_args,
        ],
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

    meson.override_dependency('ass_parser', ass_parser_dep)

    executable(
        'ass_parser',
        cli_files,
        dependencies: [ass_parser_dep],
        install: true,
        c_args: ['-DPROGRAM_NAME=' + meson.project_name()],
    )


endif
