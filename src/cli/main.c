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

	if(strcmp(file, "-") == 0) {
		source.type = AssSourceTypeStr;
		source.data.str = read_entire_stdin();
	} else {
		source.type = AssSourceTypeFile;
		source.data.file = file;
	}

	ParseSettings settings = { .strict_settings =
		                           (StrictSettings){ .script_info =
		                                                 (ScriptInfoStrictSettings){
		                                                     .allow_duplicate_fields = false,
		                                                     .allow_missing_script_type = false,
		                                                 },

		                                             .allow_additional_fields = false,
		                                             .allow_number_truncating = false,
		                                             .allow_unrecognized_file_encoding = false } };

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

			processed_args++;
		} else if((strcmp(arg, "-s") == 0) || (strcmp(arg, "--strict") == 0)) {
			settings.strict_settings.script_info.allow_duplicate_fields = false;
			settings.strict_settings.script_info.allow_missing_script_type = false;
			settings.strict_settings.allow_additional_fields = false;
			settings.strict_settings.allow_number_truncating = false;
			settings.strict_settings.allow_unrecognized_file_encoding = false;

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

	initialize_logger();

	set_log_level(log_level);

	set_thread_name("main thread");

	AssParseResult* result = parse_ass(source, settings);

	if(result == NULL) {
		LOG_MESSAGE_SIMPLE(LogLevelError, "Allocation error\n");
		return EXIT_FAILURE;
	}

	{
		// log warnings

		Warnings warnings = get_warnings_from_result(result);

		for(size_t i = 0; i < stbds_arrlenu(warnings.entries); ++i) {
			WarningEntry entry = warnings.entries[i];

			ErrorStruct message = get_warnings_message_from_entry(entry);

			LOG_MESSAGE(LogLevelWarn, "%s\n", (char*)message.message);

			free_error_struct(message);
		}
	}

	if(parse_result_is_error(result)) {
		LOG_MESSAGE(LogLevelError, "Parse error: %s\n", parse_result_get_error(result));
		free_parse_result(result);
		return EXIT_FAILURE;
	}

	LOG_MESSAGE_SIMPLE(LogLevelInfo, "File is valid\n");
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
