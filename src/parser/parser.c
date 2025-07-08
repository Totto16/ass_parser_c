

#include "./parser.h"
#include "./io.h"

#include <stdlib.h>

typedef struct {
	int todo;
} AssParsedState;

struct AssParseResultImpl {
	bool is_error;
	union {
		const char* error;
		AssParsedState ok;
	} data;
};

[[nodiscard]] static SizedPtr get_data_from_source(AssSource source) {
	switch(source.type) {
		case AssSourceTypeFile: {
			return read_entire_file(source.data.file);
		}
		case AssSourceTypeStr: {
			return source.data.str;
		}
		default: return ptr_error("unknown source type");
	}
}

[[nodiscard]] AssParseResult* parse_ass(AssSource source) {

	AssParseResult* result = (AssParseResult*)malloc(sizeof(AssParseResult));

	if(!result) {
		return NULL;
	}

	SizedPtr data = get_data_from_source(source);

	if(is_ptr_error(data)) {
		result->is_error = true;
		result->data.error = ptr_get_error(data);
		return result;
	}

	AssParsedState todo_data = { .todo = 0 };

	result->is_error = false;
	result->data.ok = todo_data;
	return result;
}

[[nodiscard]] bool parse_result_is_error(AssParseResult* result) {
	if(!result) {
		return true;
	}

	return result->is_error;
}

[[nodiscard]] const char* parse_result_get_error(AssParseResult* result) {
	if(!result) {
		return "allocation error";
	}

	return result->data.error;
}
