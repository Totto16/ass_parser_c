

#pragma once

// js needs to provide these things, when using the resulting wasm file

__attribute__((import_module("env"), import_name("platform_panic"))) extern void
platform_panic(const char* file_path, int line, const char* message);
