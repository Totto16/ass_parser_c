project(
    'zvec',
    'c',
    license: 'MIT',
    meson_version: '>=1.3.0',
    version: '0.0.1',
)


inc_dirs = []

inc_dirs += include_directories('.')

zvec_dep = declare_dependency(
    include_directories: inc_dirs,
    version: meson.project_version(),
)

meson.override_dependency('zvec', zvec_dep)


if get_option('examples')
    subdir('examples')
endif
