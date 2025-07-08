

#include "./parser.h"
#include "./io.h"

#include <stdint.h>
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

typedef enum {
	FileTypeUnknown,
	FileTypeUtf8,
	FileTypeUtf16BE,
	FileTypeUtf16LE,
	FileTypeUtf32BE,
	FileTypeUtf32LE,
} FileType;

// see: https://en.wikipedia.org/wiki/Byte_order_mark
[[nodiscard]] FileType determine_file_type(SizedPtr data) {

	uint8_t* bom = (uint8_t*)data.data;

	if(data.len < 4) {
		return FileTypeUnknown;
	}

	if(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
		return FileTypeUtf8;
	} else if(bom[0] == 0xFF && bom[1] == 0xFE && bom[2] == 0x00 && bom[3] == 0x00) {
		return FileTypeUtf32LE;
	} else if(bom[0] == 0x00 && bom[1] == 0x00 && bom[2] == 0xFE && bom[3] == 0xFF) {
		return FileTypeUtf32BE;
	} else if(bom[0] == 0xFF && bom[1] == 0xFE) {
		return FileTypeUtf16LE;
	} else if(bom[0] == 0xFE && bom[1] == 0xFF) {
		return FileTypeUtf16BE;
	} else {
		return FileTypeUnknown;
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

	FileType file_type = determine_file_type(data);

	if(file_type == FileTypeUnknown) {
		result->is_error = true;
		result->data.error = "unrecognized file, no BOM present";
		free_sized_ptr(data);
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
