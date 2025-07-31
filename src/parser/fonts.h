

#pragma once

#include "../helper/sized_ptr.h"

// note: line endings are not inserted, that has to be done on the caller site, if necessary
[[nodiscard]] SizedPtr uu_encode(SizedPtr input);

[[nodiscard]] SizedPtr uu_decode(SizedPtr input);
