

#pragma once

#ifdef __WASM__

#define PUBLIC(arg) __attribute__((visibility("default"))) __attribute__((export_name(arg)))

#define ANNOTATE_SEPERATOR ":"

// denotes an annotation, the value is malloced and the arguments are free, which means, call METHOD
// (e.g. free) to deallocated it
// note every malloced value is nullable
#define ANNOTATION_MALLOCED_RESULT(METHOD) \
	__attribute__((annotate("malloced" ANNOTATE_SEPERATOR METHOD))) CATEGORY_POINTER
#define ANNOTATION_CSTRING __attribute__((annotate("string"))) CATEGORY_POINTER
#define ANNOTATION_FREE_FN __attribute__((annotate("is_free_fn"))) CATEGORY_VOID
#define ANNOTATION_NULLABLE __attribute__((annotate("nullable"))) CATEGORY_POINTER
#define CATEGORY_IMPL(CATEGORY) __attribute__((annotate("category" ANNOTATE_SEPERATOR CATEGORY)))

#define MACRO_STRINGIFY(a) MACRO_STRINGIFY_IMPL(a)
#define MACRO_STRINGIFY_IMPL(a) #a

#define ENUM_ANNOT_C(Name, UnderlyingType) \
	MACRO_STRINGIFY(Name) ANNOTATE_SEPERATOR MACRO_STRINGIFY(UnderlyingType)

#define CATEGORY_ENUM(ENUM_TYPE) \
	CATEGORY_IMPL("ENUM") \
	__attribute__(( \
	    annotate("category_extension" ANNOTATE_SEPERATOR "enum" ANNOTATE_SEPERATOR ENUM_TYPE)))

#else

#define PUBLIC(arg) __attribute__((visibility("default")))

#define ANNOTATION_MALLOCED_RESULT(METHOD)
#define ANNOTATION_CSTRING
#define ANNOTATION_FREE_FN
#define ANNOTATION_NULLABLE
#define CATEGORY_IMPL(CATEGORY)

#define ENUM_ANNOT_C(Name, UnderlyingType)

#define CATEGORY_ENUM(ENUM_TYPE)

#endif

#define CATEGORY_LITERAL CATEGORY_IMPL("LITERAL")
#define CATEGORY_POINTER CATEGORY_IMPL("POINTER")
#define CATEGORY_VOID CATEGORY_IMPL("VOID")
