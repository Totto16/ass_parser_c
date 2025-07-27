

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
} MessageStruct;

#define STATIC_MESSAGE_STRUCT(error) \
	((MessageStruct){ .message = (char*)(error), .dynamic = false })

#define DYNAMIC_MESSAGE_STRUCT(error) ((MessageStruct){ .message = (error), .dynamic = true })

#define EMPTY_MESSAGE_STRUCT() STATIC_MESSAGE_STRUCT(NULL)

void free_message_struct(MessageStruct msg);

[[nodiscard]] MessageStruct duplicate_message_struct(MessageStruct msg);

typedef enum : uint8_t {
	DiagnosticTypeSimple,
	DiagnosticTypeUnexpectedField,
	DiagnosticTypeDuplicateField,
} DiagnosticType;

typedef enum : uint8_t {
	DiagnosticSeverityWarning,
	DiagnosticSeverityError,
} DiagnosticSeverity;

typedef struct {
	const char* section;
	FinalStr field;
} UnexpectedFieldDiagnostic;

typedef struct {
	const char* section;
	FinalStr field;
} DuplicateFieldDiagnostic;

typedef struct {
	DiagnosticType type;
	union {
		MessageStruct simple;
		UnexpectedFieldDiagnostic unexpected_field;
		DuplicateFieldDiagnostic duplicate_field;
	} data;
	DiagnosticSeverity severity;
} DiagnosticEntry;

typedef struct {
	STBDS_ARRAY(DiagnosticEntry) entries;
} Diagnostics;

void free_diagnostics(Diagnostics diagnostics);

[[nodiscard]] MessageStruct get_message_from_entry(DiagnosticEntry entry);
