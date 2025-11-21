

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

#define MACRO_STR_N_1(a) MACRO_STRINGIFY(a)
#define MACRO_STR_N_2(a, b) MACRO_STRINGIFY(a) ANNOTATE_SEPERATOR MACRO_STRINGIFY(b)
#define MACRO_STR_N_3(a, b, c) \
	MACRO_STRINGIFY(a) ANNOTATE_SEPERATOR MACRO_STRINGIFY(b) \
	ANNOTATE_SEPERATOR MACRO_STRINGIFY(c)

#define NARGS(...) NARGS_(__VA_ARGS__, 3, 2, 1)
#define NARGS_(_1, _2, _3, N, ...) N

#define MACRO_STR_N_IMPL(n, ...) MACRO_STR_N_##n(__VA_ARGS__)
#define MACRO_STR_N(n, ...) MACRO_STR_N_IMPL(n, __VA_ARGS__)

#define MACRO_STRINGIFY_MANY(...) MACRO_STR_N(NARGS(__VA_ARGS__), __VA_ARGS__)

#define JS_ANNOTATION_ENUM(ENUM_TYPE) \
	__attribute__(( \
	    annotate("category_extension" ANNOTATE_SEPERATOR "enum" ANNOTATE_SEPERATOR ENUM_TYPE)))
#define ENUM_JS_ANNOT(Name, UnderlyingType, ...) \
	Name ANNOTATE_SEPERATOR MACRO_STRINGIFY(UnderlyingType) \
	ANNOTATE_SEPERATOR MACRO_STRINGIFY_MANY(__VA_ARGS__)

#else

#define PUBLIC(arg) __attribute__((visibility("default")))

#define ANNOTATION_MALLOCED_RESULT(METHOD)
#define ANNOTATION_CSTRING
#define ANNOTATION_FREE_FN
#define ANNOTATION_NULLABLE
#define CATEGORY_IMPL(CATEGORY)

#define JS_ANNOTATION_ENUM(ENUM_TYPE)
#define ENUM_JS_ANNOT(Name, UnderlyingType, ...)

#endif

#define CATEGORY_LITERAL CATEGORY_IMPL("LITERAL")
#define CATEGORY_POINTER CATEGORY_IMPL("POINTER")
#define CATEGORY_VOID CATEGORY_IMPL("VOID")
#define CATEGORY_ENUM CATEGORY_IMPL("ENUM")
