
lib_src_files += files('lib.c', 'lib.h')

git = find_program('git')

vcs_dep = vcs_tag(
    command: [git, 'describe', '--tags', '--always', '--abbrev=12'],
    input: 'git_version.h.in',
    output: 'git_version.h',
    replace_string: '@GIT_COMMIT_HASH@',
)

lib_src_files += vcs_dep


install_headers(
    files('lib.h'),
    install_dir: install_include_dir / 'lib',
    preserve_path: true,
)
