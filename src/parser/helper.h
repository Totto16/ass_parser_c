

#pragma once

#include "./parser.h"

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

[[nodiscard]] double parse_str_as_double(ConstStrView value, ErrorStruct* error_ptr,
                                         Warnings* warnings);

[[nodiscard]] size_t parse_str_as_unsigned_number(ConstStrView value, ErrorStruct* error_ptr,
                                                  Warnings* warnings);

[[nodiscard]] bool parse_str_as_bool(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] bool parse_str_as_str_bool(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] AssColor parse_str_as_color(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] BorderStyle parse_str_as_border_style(ConstStrView value, ErrorStruct* error_ptr,
                                                    Warnings* warnings);

[[nodiscard]] AssAlignment parse_str_as_style_alignment(ConstStrView value, ErrorStruct* error_ptr,
                                                        Warnings* warnings);

[[nodiscard]] size_t parse_str_as_unsigned_number_with_option(ConstStrView value,
                                                              ErrorStruct* error_ptr,
                                                              bool allow_number_truncating,
                                                              Warnings* warnings);

[[nodiscard]] MarginValue parse_str_as_margin_value(ConstStrView value, ErrorStruct* error_ptr,
                                                    Warnings* warnings);

[[nodiscard]] AssTime parse_str_as_time(ConstStrView value, ErrorStruct* error_ptr,
                                        Warnings* warnings);

[[nodiscard]] ScriptType parse_str_as_script_type(ConstStrView value, ErrorStruct* error_ptr);

[[nodiscard]] WrapStyle parse_str_as_wrap_style(ConstStrView value, ErrorStruct* error_ptr,
                                                Warnings* warnings);
