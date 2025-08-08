

#pragma once

#include "../helper/message_struct.h"
#include "../helper/string_view.h"

#define STBDS_ONLY_MACROS
#include <stb/ds.h>
#undef STBDS_ONLY_MACROS

#include <stdint.h>

typedef ConstStrView FinalStr;

typedef enum : uint8_t {
	DiagnosticTypeSimple,
	DiagnosticTypeUnexpectedField,
	DiagnosticTypeDuplicateField,
} DiagnosticType;

typedef enum : uint8_t {
	DiagnosticSeverityWarning = 0,
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
} InnerDiagnostic;

typedef struct {
	InnerDiagnostic inner;
	DiagnosticSeverity severity;
	FilePos position;
} DiagnosticEntry;

typedef struct {
	STBDS_ARRAY(DiagnosticEntry) entries;
} Diagnostics;

#ifdef ASS_PARSER_C_INTERNAL_USAGE

// for internal use only

#define INSERT_SIMPLE_DIAGNOSTIC(entries, message, pos, severity_type) \
	do { \
		DiagnosticEntry diagnostic = { .inner = \
			                               (InnerDiagnostic){ .type = DiagnosticTypeSimple, \
			                                                  .data = { .simple = (message) } }, \
			                           .severity = (severity_type), \
			                           .position = (pos) }; \
		stbds_arrput(entries, diagnostic); /*NOLINT(clang-analyzer-unix.Malloc)*/ \
	} while(false)

#define INSERT_SIMPLE_WARNING(entries, message, pos) \
	INSERT_SIMPLE_DIAGNOSTIC(entries, message, pos, DiagnosticSeverityWarning)

#define INSERT_SIMPLE_ERROR(entries, message, pos) \
	INSERT_SIMPLE_DIAGNOSTIC(entries, message, pos, DiagnosticSeverityError)

#endif

void free_diagnostics(Diagnostics diagnostics);

[[nodiscard]] MessageStruct get_message_from_entry(DiagnosticEntry entry);

[[nodiscard]] MessageStruct get_message_from_entry_pretty(DiagnosticEntry entry,
                                                          const char* source_file);
