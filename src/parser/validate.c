
#include "./validate.h"

// TODO: remove
#include "../helper/macros.h"

static void validate_fonts(AssResult ass_result, bool allow_validation_errors,
                           Diagnostics* diagnostics) {

	UNUSED(ass_result);
	UNUSED(allow_validation_errors);
	UNUSED(diagnostics);

	// TODO
}

static void validate_styles(AssResult ass_result, bool allow_validation_errors,
                            Diagnostics* diagnostics) {

	UNUSED(ass_result);
	UNUSED(allow_validation_errors);
	UNUSED(diagnostics);

	// TODO
}

static void validate_text(AssResult ass_result, bool allow_validation_errors,
                          Diagnostics* diagnostics) {

	UNUSED(ass_result);
	UNUSED(allow_validation_errors);
	UNUSED(diagnostics);

	// TODO(Totto): check and parse text value, for invalid escape sequences, and invald
	// values inside {}, like eg {bogus}, or {\j} etc, or not closed {} blocks

	// TODO
}

void validate_ass_result(AssResult ass_result, ParseSettings settings, Diagnostics* diagnostics) {

	if(settings.validate_settings.validate_fonts) {
		validate_fonts(ass_result, settings.strict_settings.allow_validation_errors, diagnostics);
	}

	if(settings.validate_settings.validate_styles) {
		validate_styles(ass_result, settings.strict_settings.allow_validation_errors, diagnostics);
	}

	if(settings.validate_settings.validate_text) {
		validate_text(ass_result, settings.strict_settings.allow_validation_errors, diagnostics);
	}
}
