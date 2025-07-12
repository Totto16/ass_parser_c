

#include "./lib.h"
#include "./git_version.h"

[[nodiscard]] const char* ass_parser_lib_version(void) {

	return LIB_VERSION_STRING;
}

[[nodiscard]] const char* ass_parser_lib_commit_hash(void) {
	return GIT_COMMIT_HASH;
}
