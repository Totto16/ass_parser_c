
#include "./diagnostics.h"

#include "../helper/macros.h"

#include <stb/ds.h>
#include <stdio.h>

void free_message_struct(MessageStruct msg) {
	if(msg.dynamic) {
		free(msg.message);
	}
}

static void free_diagnostic_entry(DiagnosticEntry entry) {

	switch(entry.type) {
		case DiagnosticTypeSimple: {
			free_message_struct(entry.data.simple);
			break;
		}
		case DiagnosticTypeUnexpectedField: {
			break;
		}
		case DiagnosticTypeDuplicateField: {
			break;
		}
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

			FORMAT_STRING_DEFAULT(&result_buffer, "unexpected field '%s' in '%s' section",
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

			FORMAT_STRING_DEFAULT(&result_buffer, "duplicate field '%s' in '%s' section",
			                      field_name, data.section);

			free(field_name);

			break;
		}
		default: {
			return STATIC_MESSAGE_STRUCT("unknown diagnostic type");
		}
	}

#define FILE_POS_FORMAT "%zu:%zu:"

#define FILE_FORMAT "%s:"

#define FORMAT_LINE_FMT_EXPAND_POS(pos) ((pos).line + 1), ((pos).column + 1)

	char* final_result = NULL;

	if(source_file == NULL) {
		final_result = strdup(result_buffer);
	} else {
		if(is_empty_pos(entry.position)) {
			FORMAT_STRING_DEFAULT(&final_result,
			                      FILE_FORMAT " "
			                                  "%s",
			                      source_file, result_buffer);
		} else {
			FORMAT_STRING_DEFAULT(&final_result,
			                      FILE_FORMAT FILE_POS_FORMAT " "
			                                                  "%s",
			                      source_file, FORMAT_LINE_FMT_EXPAND_POS(entry.position),
			                      result_buffer);
		}
	}

	free(result_buffer);

	return DYNAMIC_MESSAGE_STRUCT(final_result);
}
