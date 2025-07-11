
#include "./warnings.h"

#include "../helper/macros.h"

#include <stdio.h>

void free_error_struct(ErrorStruct error) {
	if(error.dynamic) {
		free(error.message);
	}
}

static void free_warning_entry(WarningEntry entry) {

	switch(entry.type) {
		case WarningTypeSimple: {
			free(entry.data.simple);
			break;
		}
		case WarningTypeUnexpectedField: {
			break;
		}
		case WarningTypeDuplicateField: {
			break;
		}
		default: {
			break;
		}
	}
}

void free_warnings(Warnings warnings) {

	for(size_t i = 0; i < stbds_arrlenu(warnings.entries); ++i) {
		WarningEntry entry = warnings.entries[i];

		free_warning_entry(entry);
	}
	stbds_arrfree(warnings.entries);
}

ErrorStruct get_warnings_message_from_entry(WarningEntry entry) {

	switch(entry.type) {
		case WarningTypeSimple: {
			return STATIC_ERROR(entry.data.simple);
		}
		case WarningTypeUnexpectedField: {

			UnexpectedFieldWarning data = entry.data.unexpected_field;

			char* field_name = get_normalized_string(data.field);

			if(!field_name) {
				return STATIC_ERROR("<warning message allocation error>");
			}

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer, "unexpected field '%s' in '%s' section",
			                      data.setion, field_name);

			return DYNAMIC_ERROR(result_buffer);
		}
		case WarningTypeDuplicateField: {

			DuplicateFieldWarning data = entry.data.duplicate_field;

			char* field_name = get_normalized_string(data.field);

			if(!field_name) {
				return STATIC_ERROR("<warning message allocation error>");
			}

			char* result_buffer = NULL;
			FORMAT_STRING_DEFAULT(&result_buffer, "duplicate field '%s' in '%s' section",
			                      data.setion, field_name);

			return DYNAMIC_ERROR(result_buffer);
		}
		default: {
			return STATIC_ERROR("unknown warning type");
			break;
		}
	}
}
