
#include "./diagnostics.h"

#include "../helper/macros.h"

#include <stb/ds.h>
#include <stdio.h>

static void free_diagnostic_entry(DiagnosticEntry entry) {

	switch(entry.type) {
		case DiagnosticTypeSimple: {
			free_message_struct(entry.data.simple);
			break;
		}
		case DiagnosticTypeUnexpectedField:
		case DiagnosticTypeDuplicateField:
		default: {
			break;
		}
	}
}

void free_diagnostics(Diagnostics diagnostics) {

	for(size_t i = 0; i < stbds_arrlenu(diagnostics.entries); ++i) {
		DiagnosticEntry entry = diagnostics.entries[i];

		free_diagnostic_entry(entry);
	}
	stbds_arrfree(diagnostics.entries);
}

MessageStruct get_message_from_entry(DiagnosticEntry entry, const char* source_file) {

	char* result_buffer = NULL;

	switch(entry.type) {
		case DiagnosticTypeSimple: {

			result_buffer = strdup(entry.data.simple.message);
			break;
		}
		case DiagnosticTypeUnexpectedField: {

			UnexpectedFieldDiagnostic data = entry.data.unexpected_field;

			char* field_name = get_normalized_string(data.field);

			if(!field_name) {
				return STATIC_MESSAGE_STRUCT("<diagnostic message allocation error>");
			}

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		free(field_name); \
		return STATIC_MESSAGE_STRUCT(message); \
	} while(false)

			FORMAT_STRING_PROPAGATE_ERROR(&result_buffer, "unexpected field '%s' in '%s' section",
			                              field_name, data.section);

			free(field_name);

			break;
		}
		case DiagnosticTypeDuplicateField: {

			DuplicateFieldDiagnostic data = entry.data.duplicate_field;

			char* field_name = get_normalized_string(data.field);

			if(!field_name) {
				return STATIC_MESSAGE_STRUCT("<diagnostic message allocation error>");
			}

			FORMAT_STRING_PROPAGATE_ERROR(&result_buffer, "duplicate field '%s' in '%s' section",
			                              field_name, data.section);

			free(field_name);

			break;
		}
		default: {
			return STATIC_MESSAGE_STRUCT("unknown diagnostic type");
		}
	}

#undef PROPAGATE_ERROR_IMPL

#define FILE_POS_FORMAT "%zu:%zu:"

#define FILE_FORMAT "%s:"

#define FORMAT_LINE_FMT_EXPAND_POS(pos) ((pos).line + 1), ((pos).column + 1)

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		free(result_buffer); \
		return STATIC_MESSAGE_STRUCT(message); \
	} while(false)

	char* final_result = NULL;

	if(source_file == NULL) {
		final_result = strdup(result_buffer);
	} else {
		if(is_empty_pos(entry.position)) {
			FORMAT_STRING_PROPAGATE_ERROR(&final_result,
			                              FILE_FORMAT " "
			                                          "%s",
			                              source_file, result_buffer);
		} else {
			FORMAT_STRING_PROPAGATE_ERROR(&final_result,
			                              FILE_FORMAT FILE_POS_FORMAT " "
			                                                          "%s",
			                              source_file, FORMAT_LINE_FMT_EXPAND_POS(entry.position),
			                              result_buffer);
		}
	}

	free(result_buffer);

	return DYNAMIC_MESSAGE_STRUCT(final_result);
}

#undef PROPAGATE_ERROR_IMPL
