#include "./helper.h"

#include <stdlib.h>

uint64_t allocator_statistics_get_free(AllocatorStatistics statistics) {
	return statistics.free;
}

uint64_t allocator_statistics_get_total(AllocatorStatistics statistics) {
	return statistics.total;
}

uint64_t allocator_statistics_get_used(AllocatorStatistics statistics) {
	return statistics.used;
}

uint64_t allocator_statistics_get_metadata(AllocatorStatistics statistics) {
	return statistics.metadata;
}

AssSource* source_from_string(const char* const source, size_t len) {

	AssSource* result = (AssSource*)malloc(sizeof(AssSource));

	if(result == NULL) {
		return NULL;
	}

	result->type = AssSourceTypeStr;
	result->data.str = (SizedPtr){ .data = (void*)source, .len = len };

	return result;
}

ParseSettings* default_parse_settings(void) {

	ParseSettings* result = (ParseSettings*)malloc(sizeof(ParseSettings));

	if(result == NULL) {
		return NULL;
	}

	result->strict_settings = (StrictSettings){ .script_info =
		                                            (ScriptInfoStrictSettings){
		                                                .allow_duplicate_fields = false,
		                                                .allow_missing_script_type = false,
		                                            },

		                                        .allow_additional_fields = false,
		                                        .allow_number_truncating = false,
		                                        .allow_unrecognized_file_encoding = false,
		                                        .allow_validation_errors = false };

	result->validate_settings =
	    (ValidateSettings){ .font_settings = (FontSettings){ .preset = FontPresetStrict },
		                    .validate_text = true,
		                    .validate_styles = true };

	return result;
}
