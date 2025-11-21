

#pragma once

#ifdef __WASM__

#define PUBLIC(arg) __attribute__((visibility("default"))) __attribute__((export_name(arg)))

// denotes an annotation, the value is malloced and the arguments are free, which means, call METHOD
// (e.g. free) to deallocated it
// note every malloced value is nullable
#define ANNOTATION_MALLOCED_RESULT(METHOD) \
	__attribute__((annotate("malloced:" METHOD))) CATEGORY_POINTER
#define ANNOTATION_CSTRING __attribute__((annotate("string"))) CATEGORY_POINTER
#define ANNOTATION_FREE_FN __attribute__((annotate("is_free_fn"))) CATEGORY_VOID
#define ANNOTATION_NULLABLE __attribute__((annotate("nullable"))) CATEGORY_POINTER
#define CATEGORY_IMPL(CATEGORY) __attribute__((annotate("category:" CATEGORY)))

#else

#define PUBLIC(arg) __attribute__((visibility("default")))

#define ANNOTATION_MALLOCED_RESULT(METHOD)
#define ANNOTATION_CSTRING
#define ANNOTATION_FREE_FN
#define ANNOTATION_NULLABLE
#define CATEGORY_IMPL(CATEGORY)

#endif

#define CATEGORY_LITERAL CATEGORY_IMPL("LITERAL")
#define CATEGORY_POINTER CATEGORY_IMPL("POINTER")
#define CATEGORY_VOID CATEGORY_IMPL("VOID")
#define CATEGORY_ENUM CATEGORY_IMPL("ENUM")
