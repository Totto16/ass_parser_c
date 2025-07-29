#include <ass_parser_lib.h>
#include <stb/ds.h>

#include "./log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum : uint8_t {
	UsageCommandAll,
	UsageCommandCheck,
} UsageCommand;

// note: this usage printing is from the simple http server, that I wrote, so it might be overkill
// in some sections

#define IDENT1 "  "
#define IDENT2 IDENT1 IDENT1
#define IDENT3 IDENT2 IDENT1

static void print_check_usage(bool is_subcommand) {
	if(is_subcommand) {
		printf("<file> [options]\n");
	} else {
		printf(IDENT1 "check <file> [options]\n");
	}

	printf(IDENT1 "file: the file to use, can be '-' for stdin (required)\n");
	printf(IDENT1 "options:\n");

	printf(IDENT2 "general options:\n");

	printf(IDENT3 "-v, --version: print the version and exit\n");
	printf(IDENT3 "-h, -?, --help: print the help message for commands or the entire program\n");

	printf(IDENT2 "common options:\n");

	printf(IDENT3 "-l, --loglevel <loglevel>: Set the log level for the application\n");

	printf(IDENT2 "common strictness options\n");

	printf(IDENT3 "-n, --non-strict: don't enable strict checking of the '.ass' file, this "
	              "disables all checks\n");
	printf(IDENT3 "-s, --strict: don't enable strict checking of the '.ass' file, this "
	              "enables all checks\n");

	printf(IDENT2 "single strictness options\n");

	printf(IDENT3 "--allow-missing-script-type-in-script-info [value]: set this specific option, "
	              "specifying no value is enabling it\n");
	printf(IDENT3 "--allow-duplicate-fields-in-script-info [value]: set this specific option, "
	              "specifying no value "
	              "is enabling it\n");

	printf(IDENT3 "--allow-additional-fields [value]: set this specific option, specifying no "
	              "value is enabling it\n");
	printf(IDENT3
	       "--allow-number-truncating [value]: set this specific option, specifying no value "
	       "is enabling it\n");

	printf(
	    IDENT3
	    "--allow-unrecognized-file-encoding [value]: set this specific option, specifying no value "
	    "is enabling it\n");

	printf(IDENT3
	       "--allow-validation-errors [value]: set this specific option, specifying no value "
	       "is enabling it\n");

	printf(IDENT2 "common validation options\n");

	printf(IDENT3 "-a, --validate-everything: validate everything\n");
	printf(IDENT3 "-A, --validate-nothing: validate nothing\n");

	printf(IDENT2 "single validation options\n");

	printf(IDENT3 "--validate-styles [value]: set this specific option, "
	              "specifying no value is enabling it\n");
	printf(IDENT3 "--validate-text [value]: set this specific option, "
	              "specifying no value is enabling it\n");

	printf(IDENT3 "--font-validate-preset <preset>: set the preset for font validation\n");
}

// prints the usage, if argc is not the right amount!
static void print_usage(const char* program_name, UsageCommand usage_command) {
	switch(usage_command) {
		case UsageCommandCheck: {
			printf("usage: %s check ", program_name);
			print_check_usage(true);
			break;
		}

		case UsageCommandAll:
		default: {
			printf("usage: %s <command>\n", program_name);
			printf("commands: check\n");
			print_check_usage(false);
			break;
		}
	}
}

[[nodiscard]] static bool is_help_string(const char* str) {
	if(strcmp(str, "--help") == 0) {
		return true;
	}

	if(strcmp(str, "-h") == 0) {
		return true;
	}

	if(strcmp(str, "-?") == 0) {
		return true;
	}

	return false;
}

[[nodiscard]] static bool is_version_string(const char* str) {
	if(strcmp(str, "--version") == 0) {
		return true;
	}

	if(strcmp(str, "-v") == 0) {
		return true;
	}

	return false;
}

[[nodiscard]] static bool get_optional_bool_value(bool default_value, int* processed_args, int argc,
                                                  char* argv[]) {

	if(*processed_args + 1 > argc) {
		// no more values to use
		return default_value;
	}

	char* value = argv[*processed_args + 1];

	size_t value_length = strlen(value);

	if(value_length < 1) {
		// an empty str value, not a bool
		return default_value;
	}

	if(value[0] == '-') {
		// starting a subcommand
		return default_value;
	}

	if(strcmp(value, "0") == 0) {
		return false;
	}

	if(strcmp(value, "1") == 0) {
		return true;
	}

	if(strcmp(value, "false") == 0) {
		return false;
	}

	if(strcmp(value, "true") == 0) {
		return true;
	}

	fprintf(stderr, "Invalid bool argument: %s\n", value);
	print_usage(argv[0], UsageCommandCheck);
	exit(EXIT_FAILURE);
}

[[nodiscard]] static int subcommand_check(const char* program_name, int argc, char* argv[]) {

	if(argc < 1) {
		fprintf(stderr, "missing <file>\n");
		print_usage(program_name, UsageCommandCheck);
		return EXIT_FAILURE;
	}

	if(is_help_string(argv[0])) {
		printf("'check' command help menu:\n");
		print_usage(program_name, UsageCommandCheck);
		return EXIT_SUCCESS;
	}

	AssSource source = {};

	const char* file = argv[0];

	const char* source_file = NULL;

	if(strcmp(file, "-") == 0) {
		source.type = AssSourceTypeStr;
		source.data.str = read_entire_stdin();
		source_file = NULL;
	} else {
		source.type = AssSourceTypeFile;
		source.data.file = file;
		source_file = file;
	}

	ParseSettings settings = { .strict_settings =
		                           (StrictSettings){ .script_info =
		                                                 (ScriptInfoStrictSettings){
		                                                     .allow_duplicate_fields = false,
		                                                     .allow_missing_script_type = false,
		                                                 },

		                                             .allow_additional_fields = false,
		                                             .allow_number_truncating = false,
		                                             .allow_unrecognized_file_encoding = false,
		                                             .allow_validation_errors = false },

		                       .validate_settings = (ValidateSettings){
		                           .font_settings = (FontSettings){ .preset = FontPresetStrict },
		                           .validate_text = true,
		                           .validate_styles = true } };

	LogLevel log_level =
#ifdef NDEBUG
	    LogLevelInfo
#else
	    LogLevelTrace
#endif
	    ;

	// the file
	int processed_args = 1;

	while(processed_args != argc) {

		const char* arg = argv[processed_args];

		if((strcmp(arg, "-n") == 0) || (strcmp(arg, "--non-strict") == 0)) {
			settings.strict_settings.script_info.allow_duplicate_fields = true;
			settings.strict_settings.script_info.allow_missing_script_type = true;
			settings.strict_settings.allow_additional_fields = true;
			settings.strict_settings.allow_number_truncating = true;
			settings.strict_settings.allow_unrecognized_file_encoding = true;
			settings.strict_settings.allow_validation_errors = true;

			processed_args++;
		} else if((strcmp(arg, "-s") == 0) || (strcmp(arg, "--strict") == 0)) {
			settings.strict_settings.script_info.allow_duplicate_fields = false;
			settings.strict_settings.script_info.allow_missing_script_type = false;
			settings.strict_settings.allow_additional_fields = false;
			settings.strict_settings.allow_number_truncating = false;
			settings.strict_settings.allow_unrecognized_file_encoding = false;
			settings.strict_settings.allow_validation_errors = false;

			processed_args++;
		} else if((strcmp(arg, "--allow-duplicate-fields-in-script-info") == 0)) {
			processed_args++;

			bool value = get_optional_bool_value(true, &processed_args, argc, argv);

			settings.strict_settings.script_info.allow_duplicate_fields = value;

		} else if((strcmp(arg, "--allow-missing-script-type-in-script-info") == 0)) {
			processed_args++;

			bool value = get_optional_bool_value(true, &processed_args, argc, argv);

			settings.strict_settings.script_info.allow_missing_script_type = value;

		} else if((strcmp(arg, "--allow-additional-fields") == 0)) {
			processed_args++;

			bool value = get_optional_bool_value(true, &processed_args, argc, argv);

			settings.strict_settings.allow_additional_fields = value;

		} else if((strcmp(arg, "--allow-number-truncating") == 0)) {
			processed_args++;

			bool value = get_optional_bool_value(true, &processed_args, argc, argv);

			settings.strict_settings.allow_number_truncating = value;

		} else if((strcmp(arg, "--allow-unrecognized-file-encoding") == 0)) {
			processed_args++;

			bool value = get_optional_bool_value(true, &processed_args, argc, argv);

			settings.strict_settings.allow_unrecognized_file_encoding = value;

		} else if((strcmp(arg, "--allow-validation-errors") == 0)) {
			processed_args++;

			bool value = get_optional_bool_value(true, &processed_args, argc, argv);

			settings.strict_settings.allow_validation_errors = value;

		} else if((strcmp(arg, "-a") == 0) || (strcmp(arg, "--validate-everything") == 0)) {
			settings.validate_settings.font_settings.preset = FontPresetStrictAll;
			settings.validate_settings.validate_text = true;
			settings.validate_settings.validate_styles = true;

			processed_args++;
		} else if((strcmp(arg, "-A") == 0) || (strcmp(arg, "--validate-nothing") == 0)) {
			settings.validate_settings.font_settings.preset = FontPresetDisabled;
			settings.validate_settings.validate_text = false;
			settings.validate_settings.validate_styles = false;

			processed_args++;
		} else if((strcmp(arg, "--font-validate-preset") == 0)) {

			if(processed_args + 2 > argc) {
				fprintf(stderr, "Not enough arguments for the 'font-validate-preset' option\n");
				print_usage(argv[0], UsageCommandCheck);
				return EXIT_FAILURE;
			}

			int parsed_preset = parse_font_preset(argv[processed_args + 1]);

			if(parsed_preset < 0) {
				fprintf(
				    stderr,
				    "Wrong option for the 'font-validate-preset' option, unrecognized preset: %s\n",
				    argv[processed_args + 1]);
				print_usage(argv[0], UsageCommandCheck);
				return EXIT_FAILURE;
			}

			settings.validate_settings.font_settings.preset = parsed_preset;

			processed_args += 2;

		} else if((strcmp(arg, "--validate-styles") == 0)) {
			processed_args++;

			bool value = get_optional_bool_value(true, &processed_args, argc, argv);

			settings.validate_settings.validate_styles = value;

		} else if((strcmp(arg, "--validate-text") == 0)) {
			processed_args++;

			bool value = get_optional_bool_value(true, &processed_args, argc, argv);

			settings.validate_settings.validate_text = value;

		} else if((strcmp(arg, "-l") == 0) || (strcmp(arg, "--loglevel") == 0)) {
			if(processed_args + 2 > argc) {
				fprintf(stderr, "Not enough arguments for the 'loglevel' option\n");
				print_usage(argv[0], UsageCommandCheck);
				return EXIT_FAILURE;
			}

			int parsed_level = parse_log_level(argv[processed_args + 1]);

			if(parsed_level < 0) {
				fprintf(stderr, "Wrong option for the 'loglevel' option, unrecognized level: %s\n",
				        argv[processed_args + 1]);
				print_usage(argv[0], UsageCommandCheck);
				return EXIT_FAILURE;
			}

			log_level = parsed_level;

			processed_args += 2;
		} else {
			fprintf(stderr, "Unrecognized option: %s\n", arg);
			print_usage(argv[0], UsageCommandCheck);
			return EXIT_FAILURE;
		}
	}

	initialize_logger(false);

	set_log_level(log_level);

	AssParseResult* result = parse_ass(source, settings);

	if(result == NULL) {
		LOG_MESSAGE_SIMPLE(LogLevelError, "Allocation error\n");
		return EXIT_FAILURE;
	}

	// log diagnostics

	Diagnostics diagnostics = get_diagnostics_from_result(result);

	size_t diagnostics_length = stbds_arrlenu(diagnostics.entries);

	for(size_t i = 0; i < diagnostics_length; ++i) {
		DiagnosticEntry entry = diagnostics.entries[i];

		MessageStruct message = get_message_from_entry_pretty(entry, source_file);

		switch(entry.severity) {
			case DiagnosticSeverityWarning: {
				LOG_MESSAGE(LogLevelWarn, "%s\n", (char*)message.message);
				break;
			}
			case DiagnosticSeverityError: {
				LOG_MESSAGE(LogLevelError, "%s\n", (char*)message.message);
				break;
			}
			default: {
				LOG_MESSAGE(LogLevelCritical, "Unknown severity: %s\n", (char*)message.message);
				break;
			}
		}

		free_message_struct(message);
	}

	if(parse_result_is_error(result)) {
		LOG_MESSAGE(LogLevelError, "Parse error: %lu diagnostics occurred\n", diagnostics_length);
		free_parse_result(result);
		return EXIT_FAILURE;
	}

	LOG_MESSAGE(LogLevelInfo, "File is valid %lu diagnostics occurred\n", diagnostics_length);
	free_parse_result(result);
	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {

	// checking if there are enough arguments
	if(argc < 2) {
		fprintf(stderr, "No command specified\n");
		print_usage(argv[0], UsageCommandAll);
		return EXIT_FAILURE;
	}

	const char* command = argv[1];

	if(strcmp(command, "check") == 0) {
		return subcommand_check(argv[0], argc - 2, argv + 2);
	}

	if(is_help_string(command)) {
		printf("General help menu:\n");
		print_usage(argv[0], UsageCommandAll);
		return EXIT_SUCCESS;
	}

	if(is_version_string(command)) {
		printf("%s\nCommit: %s\n", ass_parser_lib_version(), ass_parser_lib_commit_hash());
		return EXIT_SUCCESS;
	}

	fprintf(stderr, "Invalid command '%s'\n", command);
	print_usage(argv[0], UsageCommandAll);
	return EXIT_FAILURE;
}
