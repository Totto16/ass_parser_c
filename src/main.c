#include <helper/io.h>
#include <parser/parser.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum : uint8_t {
	UsageCommandAll,
	UsageCommandCheck,
} UsageCommand;

// note: this usage printing is from the simple http server, that I wrote, so it might be overkill
// in some sections

#define IDENT1 "\t"
#define IDENT2 IDENT1 IDENT1

static void print_check_usage(bool is_subcommand) {
	if(is_subcommand) {
		printf("<file> [options]\n");
	} else {
		printf(IDENT1 "check <file> [options]\n");
	}

	printf(IDENT1 "file: the file to use, can be '-' for stdin (required)\n");
	// no options yet
	printf(IDENT1 "options:\n");
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

[[nodiscard]] static int subcommand_check(const char* program_name, int argc, char* argv[]) {

	if(argc < 1) {
		fprintf(stderr, "missing <file>\n");
		print_usage(program_name, UsageCommandCheck);
		return EXIT_FAILURE;
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

	AssParseResult* result = parse_ass(source);

	if(parse_result_is_error(result)) {
		fprintf(stderr, "Parse error: %s\n", parse_result_get_error(result));
		free_parse_result(result);
		return EXIT_FAILURE;
	}

	printf("File is valid\n");
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

	fprintf(stderr, "Invalid command '%s'\n", command);
	print_usage(argv[0], UsageCommandAll);
	return EXIT_FAILURE;
}
