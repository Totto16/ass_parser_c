

#pragma once

#include "./sized_ptr.h"

[[nodiscard]] SizedPtr read_entire_file(const char* file_name);

[[nodiscard]] SizedPtr read_entire_stdin(void);
