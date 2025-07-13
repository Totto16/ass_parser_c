

#pragma once

#include "../helper/string_view.h"

#define STBDS_ONLY_MACROS
#include <stb/ds.h>
#undef STBDS_ONLY_MACROS

#include <stdint.h>

typedef ConstStrView FinalStr;

typedef struct {
	char* message;
	bool dynamic;
} ErrorStruct;

#define STATIC_ERROR(error) ((ErrorStruct){ .message = (char*)(error), .dynamic = false })

#define DYNAMIC_ERROR(error) ((ErrorStruct){ .message = (error), .dynamic = true })

#define NO_ERROR() STATIC_ERROR(NULL)

void free_error_struct(ErrorStruct error);

typedef enum : uint8_t {
	WarningTypeSimple,
	WarningTypeUnexpectedField,
	WarningTypeDuplicateField,
} WarningType;

typedef struct {
	const char* setion;
	FinalStr field;
} UnexpectedFieldWarning;

typedef struct {
	const char* setion;
	FinalStr field;
} DuplicateFieldWarning;

typedef struct {
	WarningType type;
	union {
		char* simple;
		UnexpectedFieldWarning unexpected_field;
		DuplicateFieldWarning duplicate_field;
	} data;
} WarningEntry;

typedef struct {
	STBDS_ARRAY(WarningEntry) entries;
} Warnings;

void free_warnings(Warnings warnings);

[[nodiscard]] ErrorStruct get_warnings_message_from_entry(WarningEntry entry);
