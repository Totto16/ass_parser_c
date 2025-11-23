

#pragma once

#include "./platform.h"
#include "./stdbool.h"

void custom_assert(const char* file, int line, bool cond, const char* message);

#define ASSERT(cond, message) custom_assert(__FILE__, __LINE__, cond, message)

#define PANIC(message) platform_panic(__FILE__, __LINE__, message)

#define assert(x) ASSERT(x, #x)
