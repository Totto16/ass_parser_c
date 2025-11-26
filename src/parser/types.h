#pragma once

#include "../helper/sized_ptr.h"
#include "../helper/string_view.h"
#include "./diagnostics.h"

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
	bool allow_validation_errors;
} StrictSettings;

typedef enum : uint8_t {
	FontPresetDisabled = 0,
	FontPresetStrictAll,
	FontPresetStrict,
	FontPresetModerate,
	FontPresetLenient
} FontPreset;

typedef struct {
	FontPreset preset;
} FontSettings;

typedef struct {
	FontSettings font_settings;
	bool validate_styles;
	bool validate_text;
} ValidateSettings;

typedef struct {
	StrictSettings strict_settings;
	ValidateSettings validate_settings;
} ParseSettings;

typedef enum : uint8_t {
	WrapStyleSmart = 0,
	WrapStyleEOL,
	WrapStyleNoWrap,
	WrapStyleSmartLow,
} WrapStyle;

#define ENUM_ANNOT_WrapStyle ENUM_ANNOT_C(WrapStyle, uint8_t)

typedef enum : uint8_t {
	ScriptTypeUnknown,
	ScriptTypeV4,
	ScriptTypeV4Plus,
} ScriptType;

#define ENUM_ANNOT_ScriptType ENUM_ANNOT_C(ScriptType, uint8_t)

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

typedef size_t EncodingType;

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
	EncodingType encoding;
} AssStyleEntry;

ZVEC_DEFINE_VEC_TYPE(AssStyleEntry)

typedef struct {
	ZVEC_TYPENAME(AssStyleEntry) entries;
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
	FinalStr todo;
} AssText;

typedef enum : uint8_t {
	SSACommandTypePause,
	SSACommandTypeWaitForTrigger,
} SSACommandType;

typedef struct {
	bool recognized_command;
	union {
		SSACommandType recognized;
		FinalStr unrecognized;
	} data;
} SSACommand;

typedef struct {
	bool ssa_command;
	union {
		SSACommand command;
		FinalStr string;
	} data;
} CommandEvent;

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
	union {
		AssText dialogue;
		AssText comment;
		FinalStr picture; // not only png is supported
		FinalStr sound;   // only wav is supported
		FinalStr movie;   // only avi supported
		CommandEvent command;
	} text_union;
} AssEventEntry;

ZVEC_DEFINE_VEC_TYPE(AssEventEntry)

typedef struct {
	ZVEC_TYPENAME(AssEventEntry) entries;
} AssEvents;

typedef struct {
	FinalStr name;
	bool bold;
	bool italic;
	EncodingType encoding;
} AssFontName;

typedef struct {
	AssFontName name;
	SizedPtr data;
} AssFontEntry;

ZVEC_DEFINE_VEC_TYPE(AssFontEntry)

typedef struct {
	ZVEC_TYPENAME(AssFontEntry) entries;
} AssFonts;

typedef struct {
	FinalStr name;
	SizedPtr data;
} AssGraphicEntry;

ZVEC_DEFINE_VEC_TYPE(AssGraphicEntry)

typedef struct {
	ZVEC_TYPENAME(AssGraphicEntry) entries;
} AssGraphics;

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

#define ENUM_ANNOT_FileType ENUM_ANNOT_C(FileType, uint8_t)

typedef struct {
	LineType line_type;
	FileType file_type;
} FileProps;

typedef struct {
	AssScriptInfo script_info;
	AssStyles styles;
	AssEvents events;
	AssFonts fonts;
	AssGraphics graphics;
	ExtraSections extra_sections;
	FileProps file_props;
} AssResult;

// TODO: windows support
//  todo fonts + graphics
//  todo: text parsing, not in validation!
