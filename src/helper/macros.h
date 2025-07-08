#pragma once

#define UNUSED(v) ((void)(v))

// cool trick from here:
// https://stackoverflow.com/questions/777261/avoiding-unused-variables-warnings-when-using-assert-in-a-release-build
#ifdef NDEBUG
#define assert(x) /* NOLINT(readability-identifier-naming) */ \
	do { \
		UNUSED((x)); \
	} while(false)
#else

#include <assert.h>

#endif
