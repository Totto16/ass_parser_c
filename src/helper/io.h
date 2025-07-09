

#pragma once

#include "./sized_ptr.h"

#include <stdio.h>

[[nodiscard]] SizedPtr read_entire_file(const char* file_name);

[[nodiscard]] SizedPtr read_entire_file_raw(FILE* file);
