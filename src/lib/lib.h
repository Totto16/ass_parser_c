

#pragma once

#include "../helper/decl.h"

[[nodiscard]] PUBLIC("ass_parser_lib_version") ANNOTATION_CSTRING const
    char* ass_parser_lib_version(void);

[[nodiscard]] PUBLIC("ass_parser_lib_commit_hash") ANNOTATION_CSTRING const
    char* ass_parser_lib_commit_hash(void);
