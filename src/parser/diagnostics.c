
#include "./diagnostics.h"

#include "../helper/macros.h"

#include <stb/ds.h>
#include <stdio.h>

void free_message_struct(MessageStruct msg) {
	if(msg.dynamic) {
		free(msg.message);
	}
}

[[nodiscard]] MessageStruct duplicate_message_struct(MessageStruct msg) {
	if(msg.dynamic) {
		return ((MessageStruct){ .message = strdup(msg.message), .dynamic = true });
	}
	return msg;
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

MessageStruct get_message_from_entry(DiagnosticEntry entry) {

	switch(entry.type) {
		case DiagnosticTypeSimple: {
			return duplicate_message_struct(entry.data.simple);
		}
		case DiagnosticTypeUnexpectedField: {

			UnexpectedFieldDiagnostic data = entry.data.unexpected_field;

			char* field_name = get_normalized_string(data.field);

			if(!field_name) {
				return STATIC_MESSAGE_STRUCT("<diagnostic message allocation error>");
			}

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer, "unexpected field '%s' in '%s' section",
			                      field_name, data.section);

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
			FORMAT_STRING_DEFAULT(&result_buffer, "duplicate field '%s' in '%s' section",
			                      field_name, data.section);

			free(field_name);

			return DYNAMIC_MESSAGE_STRUCT(result_buffer);
		}
		default: {
			return STATIC_MESSAGE_STRUCT("unknown diagnostic type");
			break;
		}
	}
}
