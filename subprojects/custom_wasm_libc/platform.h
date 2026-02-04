

#pragma once

#include "./stddef.h"

// js needs to provide these things, when using the resulting wasm file

__attribute__((import_module("env"), import_name("platform_panic")))
__attribute__((noreturn)) extern void
platform_panic(const char* file_path, int line, const char* message);

__attribute__((import_module("env"), import_name("platform_log_start"))) extern void
platform_log_start(bool error);

__attribute__((import_module("env"), import_name("platform_log_add"))) extern void
platform_log_add(const char* message);

__attribute__((import_module("env"), import_name("platform_log_end"))) extern void
platform_log_end(void);

__attribute__((import_module("env"), import_name("platform_exit")))
__attribute__((noreturn)) extern void
platform_exit(int status);

__attribute__((import_module("env"), import_name("platform_string_conversion"))) extern void
platform_string_conversion(void* data, size_t len, const char* format, void** out_data,
                           size_t* out_len);

// helper functions
void platform_log_single(bool error, const char* message);
