
#include "./diagnostics.h"

#include "../helper/macros.h"

#include <stb/ds.h>
#include <stdio.h>

static void free_inner_diagnostic(InnerDiagnostic inner) {
	switch(inner.type) {
		case DiagnosticTypeSimple: {
			free_message_struct(inner.data.simple);
			break;
		}
		case DiagnosticTypeUnexpectedField:
		case DiagnosticTypeDuplicateField:
		default: {
			break;
		}
	}
}

static void free_diagnostic_entry(DiagnosticEntry entry) {

	free_inner_diagnostic(entry.inner);
}

void free_diagnostics(Diagnostics diagnostics) {

	for(size_t i = 0; i < stbds_arrlenu(diagnostics.entries); ++i) {
		DiagnosticEntry entry = diagnostics.entries[i];

		free_diagnostic_entry(entry);
	}
	stbds_arrfree(diagnostics.entries);
}

[[nodiscard]] static MessageStruct get_message_from_inner_entry(InnerDiagnostic inner) {

	switch(inner.type) {
		case DiagnosticTypeSimple: {
			return STATIC_MESSAGE_STRUCT(inner.data.simple.message);
		}
		case DiagnosticTypeUnexpectedField: {

			UnexpectedFieldDiagnostic data = inner.data.unexpected_field;

			char* field_name = get_normalized_string(data.field);

			if(!field_name) {
				return STATIC_MESSAGE_STRUCT("<diagnostic message allocation error>");
			}

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		free(field_name); \
		return STATIC_MESSAGE_STRUCT(message); \
	} while(false)

			char* result_buffer = NULL;
			FORMAT_STRING_PROPAGATE_ERROR(&result_buffer, "unexpected field '%s' in '%s' section",
			                              field_name, data.section);

			free(field_name);

			return DYNAMIC_MESSAGE_STRUCT(result_buffer);
		}
		case DiagnosticTypeDuplicateField: {

			DuplicateFieldDiagnostic data = inner.data.duplicate_field;

			char* field_name = get_normalized_string(data.field);

			if(!field_name) {
				return STATIC_MESSAGE_STRUCT("<diagnostic message allocation error>");
			}

			char* result_buffer = NULL;
			FORMAT_STRING_PROPAGATE_ERROR(&result_buffer, "duplicate field '%s' in '%s' section",
			                              field_name, data.section);

			free(field_name);

			return DYNAMIC_MESSAGE_STRUCT(result_buffer);
		}
		default: {
			return STATIC_MESSAGE_STRUCT("unknown diagnostic type");
		}
	}
}

#undef PROPAGATE_ERROR_IMPL

[[nodiscard]] MessageStruct get_message_from_entry(DiagnosticEntry entry) {
	return get_message_from_inner_entry(entry.inner);
}

MessageStruct get_message_from_entry_pretty(DiagnosticEntry entry, const char* source_file) {

	MessageStruct inner_message = get_message_from_inner_entry(entry.inner);

#define FILE_POS_FORMAT "%zu:%zu:"

#define FILE_FORMAT "%s:"

#define FORMAT_LINE_FMT_EXPAND_POS(pos) ((pos).line + 1), ((pos).column + 1)

#define PROPAGATE_ERROR_IMPL(message) \
	do { \
		free_message_struct(inner_message); \
		return STATIC_MESSAGE_STRUCT(message); \
	} while(false)

	if(source_file == NULL) {

		MessageStruct final_result = EMPTY_MESSAGE_STRUCT();

		if(inner_message.dynamic) {
			// do a move, no need to copy
			final_result = inner_message;
			inner_message = EMPTY_MESSAGE_STRUCT();
		} else {
			// this is not freed as it's static!
			final_result = inner_message;
		}

		free_message_struct(inner_message);
		return final_result;
	}

	if(is_empty_pos(entry.position)) {

		char* final_result = NULL;
		FORMAT_STRING_PROPAGATE_ERROR(&final_result,
		                              FILE_FORMAT " "
		                                          "%s",
		                              source_file, inner_message.message);

		free_message_struct(inner_message);
		return DYNAMIC_MESSAGE_STRUCT(final_result);
	}

	char* final_result = NULL;
	FORMAT_STRING_PROPAGATE_ERROR(&final_result,
	                              FILE_FORMAT FILE_POS_FORMAT " "
	                                                          "%s",
	                              source_file, FORMAT_LINE_FMT_EXPAND_POS(entry.position),
	                              inner_message.message);

	free_message_struct(inner_message);
	return DYNAMIC_MESSAGE_STRUCT(final_result);
}

#undef PROPAGATE_ERROR_IMPL
