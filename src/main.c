#include <parser/parser.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {

	// TODO: parse cli args
	if(argc < 2) {
		fprintf(stderr, "Not enough arguments\n");
		return EXIT_FAILURE;
	}

	AssSource source = { .type = AssSourceTypeFile, .data = { .file = argv[1] } };

	AssParseResult* result = parse_ass(source);

	if(parse_result_is_error(result)) {
		fprintf(stderr, "Parse error: %s\n", parse_result_get_error(result));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
