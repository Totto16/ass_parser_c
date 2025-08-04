

#pragma once

#ifdef __WASM__

#define PUBLIC(arg) __attribute__((visibility("default"))) __attribute__((export_name(arg)))

#else

#define PUBLIC(arg) __attribute__((visibility("default")))

#endif
