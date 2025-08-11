

#pragma once

#ifdef __WASM__

#define PUBLIC(arg) __attribute__((visibility("default"))) __attribute__((export_name(arg)))

// denotes an annotation, the value is malloced and the arguments are free, which means, call METHOD
// (e.g. free) to deallocated it
#define ANNOTATION_MALLOCED_RESULT(METHOD) __attribute__((annotate("malloced:" METHOD)))
#define ANNOTATION_CSTRING __attribute__((annotate("string")))
#define ANNOTATION_FREE_FN __attribute__((annotate("is_free_fn")))

#else

#define PUBLIC(arg) __attribute__((visibility("default")))
#define ANNOTATION_MALLOCED_RESULT(METHOD)
#define ANNOTATION_CSTRING
#define ANNOTATION_FREE_FN
#endif
