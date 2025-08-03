

#pragma once

#include "./stdbool.h"

// js needs to provide these things, when using the resulting wasm file


__attribute__((import_module("env"), import_name("platform_assert")))
extern void platform_assert(bool value, const char* message);
