#include "./helper.h"

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

#include <stdio.h>

AssSource source_from_string(const char* source, size_t len) {

	fprintf(stderr, "herE s: %p, size: %lu, %c", (void*)(&source), len, *((char*)402288));

	return (AssSource){ .type = AssSourceTypeStr,
		                .data = { .str = { .data = (void*)source, .len = len } } };
}

ParseSettings default_parse_settings(void) {

	ParseSettings settings = { .strict_settings =
		                           (StrictSettings){ .script_info =
		                                                 (ScriptInfoStrictSettings){
		                                                     .allow_duplicate_fields = false,
		                                                     .allow_missing_script_type = false,
		                                                 },

		                                             .allow_additional_fields = false,
		                                             .allow_number_truncating = false,
		                                             .allow_unrecognized_file_encoding = false,
		                                             .allow_validation_errors = false },

		                       .validate_settings = (ValidateSettings){
		                           .font_settings = (FontSettings){ .preset = FontPresetStrict },
		                           .validate_text = true,
		                           .validate_styles = true } };

	return settings;
}
