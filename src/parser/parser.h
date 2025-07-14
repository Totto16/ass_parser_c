

#pragma once

#include "../helper/sized_ptr.h"
#include "../helper/string_view.h"
#include "./warnings.h"

#include <stdint.h>

typedef enum : uint8_t {
	AssSourceTypeFile,
	AssSourceTypeStr,
} AssSourceType;

typedef struct {
	AssSourceType type;
	union {
		const char* file;
		SizedPtr str;
	} data;
} AssSource;

typedef struct {
	bool allow_duplicate_fields;
	bool allow_missing_script_type;
} ScriptInfoStrictSettings;

typedef struct {
	ScriptInfoStrictSettings script_info;
	bool allow_additional_fields;
	bool allow_number_truncating;
	bool allow_unrecognized_file_encoding;
} StrictSettings;

typedef struct {
	StrictSettings strict_settings;
} ParseSettings;

typedef enum : uint8_t {
	WrapStyleSmart = 0,
	WrapStyleEOL,
	WrapStyleNoWrap,
	WrapStyleSmartLow,
} WrapStyle;

typedef enum : uint8_t {
	ScriptTypeUnknown,
	ScriptTypeV4,
	ScriptTypeV4Plus,
} ScriptType;

typedef struct {
	FinalStr title;
	FinalStr original_script;
	FinalStr original_translation;
	FinalStr original_editing;
	FinalStr original_timing;
	FinalStr synch_point;
	FinalStr script_updated_by;
	FinalStr update_details;
	ScriptType script_type;
	FinalStr collisions;
	size_t play_res_y;
	size_t play_res_x;
	FinalStr play_depth;
	FinalStr timer;
	WrapStyle wrap_style;
	// not documented, but present
	bool scaled_border_and_shadow;
	size_t video_aspect_ratio;
	size_t video_zoom;
	FinalStr ycbcr_matrix;
} AssScriptInfo;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} AssColor;

typedef enum : uint8_t {
	BorderStyleOutline = 1,
	BorderStyleOpaqueBox = 3,
} BorderStyle;

typedef enum : uint8_t {
	// bottom
	AssAlignmentBL = 1,
	AssAlignmentBC,
	AssAlignmentBR,
	// middle
	AssAlignmentML,
	AssAlignmentMC,
	AssAlignmentMR,
	// top
	AssAlignmentTL,
	AssAlignmentTC,
	AssAlignmentTR,
} AssAlignment;

typedef struct {
	FinalStr name;
	FinalStr fontname;
	size_t fontsize;
	AssColor primary_colour;
	AssColor secondary_colour;
	AssColor outline_colour;
	AssColor back_colour;
	bool bold;
	bool italic;
	bool underline;
	bool strike_out;
	size_t scale_x;
	size_t scale_y;
	double spacing;
	double angle;
	BorderStyle border_style;
	double outline;
	double shadow;
	AssAlignment alignment;
	size_t margin_l;
	size_t margin_r;
	size_t margin_v;
	size_t encoding;
} AssStyleEntry;

typedef struct {
	STBDS_ARRAY(AssStyleEntry) entries;
} AssStyles;

typedef enum : uint8_t {
	EventTypeDialogue,
	EventTypeComment,
	EventTypePicture,
	EventTypeSound,
	EventTypeMovie,
	EventTypeCommand
} EventType;

typedef struct {
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t hundred;
} AssTime;

typedef struct {
	bool is_default;
	union {
		size_t value;
	} data;
} MarginValue;

typedef struct {
	// marks different event_types
	EventType type;
	// original fields
	size_t layer;
	AssTime start;
	AssTime end;
	FinalStr style;
	FinalStr name;
	MarginValue margin_l;
	MarginValue margin_r;
	MarginValue margin_v;
	FinalStr effect;
	FinalStr text;
} AssEventEntry;

typedef struct {
	STBDS_ARRAY(AssEventEntry) entries;
} AssEvents;
/* typedef struct {
    int todo;
} AssFonts;
typedef struct {
    int todo;
} AssGraphics; */

STBDS_HASH_MAP_TYPE(char*, FinalStr, SectionFieldEntry);

typedef struct {
	STBDS_HASH_MAP(SectionFieldEntry) fields;
} ExtraSectionEntry;

STBDS_HASH_MAP_TYPE(char*, ExtraSectionEntry, ExtraSectionHashMapEntry);

typedef struct {
	STBDS_HASH_MAP(ExtraSectionHashMapEntry) entries;
} ExtraSections;

typedef enum : uint8_t {
	FileTypeUnknown,
	FileTypeUtf8,
	FileTypeUtf16BE,
	FileTypeUtf16LE,
	FileTypeUtf32BE,
	FileTypeUtf32LE,
} FileType;

typedef struct {
	LineType line_type;
	FileType file_type;
} FileProps;

typedef struct {
	AssScriptInfo script_info;
	AssStyles styles;
	AssEvents events;
	//	AssFonts fonts;
	//	AssGraphics graphics;
	ExtraSections extra_sections;
	FileProps file_props;
} AssResult;

typedef struct AssParseResultImpl AssParseResult;

[[nodiscard]] AssParseResult* parse_ass(AssSource source, ParseSettings settings);

[[nodiscard]] Warnings get_warnings_from_result(AssParseResult* result);

[[nodiscard]] bool parse_result_is_error(AssParseResult* result);

[[nodiscard]] char* parse_result_get_error(AssParseResult* result);

[[nodiscard]] AssResult parse_result_get_value(AssParseResult* result);

void free_parse_result(AssParseResult* result);

[[nodiscard]] const char* get_script_type_name(ScriptType script_type);

[[nodiscard]] const char* get_file_type_name(FileType file_type);
