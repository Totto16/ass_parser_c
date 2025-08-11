

#pragma once

#ifdef __WASM__

#define PUBLIC(arg) __attribute__((visibility("default"))) __attribute__((export_name(arg)))

// denotes an annotation, the value is malloced and the arguments are free, which means, call METHOD
// (e.g. free) to deallocated it
#define MALLOCED_RESULT(METHOD) __attribute__((annotate("malloced:" METHOD)))

#else

#define PUBLIC(arg) __attribute__((visibility("default")))
#define MALLOCED_RESULT(METHOD)

#endif
