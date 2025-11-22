
example_numbers = [0,1,2]

foreach num : example_numbers

    example_file = file('example_' + num.to_string() + '.c')

    example_exe = executable(
        'example_' + num.to_string(),
        example_file,
        dependencies: zvec_dep,
        override_options: {
            'warning_level': '3',
            'werror': true,
        },
    )

endforeach
