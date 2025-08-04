

#pragma once

// js needs to provide these things, when using the resulting wasm file

__attribute__((import_module("env"), import_name("platform_panic")))
__attribute__((noreturn)) extern void
platform_panic(const char* file_path, int line, const char* message);

__attribute__((import_module("env"), import_name("platform_log"))) extern void
platform_log(const char* message);

__attribute__((import_module("env"), import_name("platform_error"))) extern void
platform_error(const char* message);
