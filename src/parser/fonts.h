

#pragma once

#include "../helper/decl.h"
#include "../helper/sized_ptr.h"

// note: line endings are not inserted, that has to be done on the caller site, if necessary
[[nodiscard]] PUBLIC("uu_encode") CATEGORY_STRUCT SizedPtr uu_encode(SizedPtr input);

[[nodiscard]] PUBLIC("uu_decode") CATEGORY_STRUCT SizedPtr uu_decode(SizedPtr input);
