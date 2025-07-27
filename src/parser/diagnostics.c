
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

#define LINE_FORMAT "%zu:%zu"

#define FORMAT_LINE_FMT_EXPAND(pos) pos.line, pos.column

MessageStruct get_message_from_entry(DiagnosticEntry entry) {

	switch(entry.type) {
		case DiagnosticTypeSimple: {

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer, LINE_FORMAT ": %s",
			                      FORMAT_LINE_FMT_EXPAND(entry.position),
			                      entry.data.simple.message);

			return DYNAMIC_MESSAGE_STRUCT(result_buffer);
		}
		case DiagnosticTypeUnexpectedField: {

			UnexpectedFieldDiagnostic data = entry.data.unexpected_field;

			char* field_name = get_normalized_string(data.field);

			if(!field_name) {
				return STATIC_MESSAGE_STRUCT("<diagnostic message allocation error>");
			}

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer,
			                      LINE_FORMAT ": unexpected field '%s' in '%s' section",
			                      FORMAT_LINE_FMT_EXPAND(entry.position), field_name, data.section);

			free(field_name);

			return DYNAMIC_MESSAGE_STRUCT(result_buffer);
		}
		case DiagnosticTypeDuplicateField: {

			DuplicateFieldDiagnostic data = entry.data.duplicate_field;

			char* field_name = get_normalized_string(data.field);

			if(!field_name) {
				return STATIC_MESSAGE_STRUCT("<diagnostic message allocation error>");
			}

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer,
			                      LINE_FORMAT "duplicate field '%s' in '%s' section",
			                      FORMAT_LINE_FMT_EXPAND(entry.position), field_name, data.section);

			free(field_name);

			return DYNAMIC_MESSAGE_STRUCT(result_buffer);
		}
		default: {
			return STATIC_MESSAGE_STRUCT("unknown diagnostic type");
			break;
		}
	}
}
