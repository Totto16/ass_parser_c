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
} AssColorTODO;

typedef struct {
	AssColorTODO base;
	uint8_t a;
} AssColorWithAlpha;

typedef enum : uint8_t {
	BorderStyleOutline = 1,
	BorderStyleOpaqueBox = 3,
} BorderStyle;

typedef size_t EncodingType;

typedef enum : uint8_t {
	// bottom, sub
	AssAlignmentNumpadBL = 1,
	AssAlignmentNumpadBC = 2,
	AssAlignmentNumpadBR = 3,
	// middle, mid
	AssAlignmentNumpadML = 4,
	AssAlignmentNumpadMC = 5,
	AssAlignmentNumpadMR = 6,
	// top
	AssAlignmentNumpadTL = 7,
	AssAlignmentNumpadTC = 8,
	AssAlignmentNumpadTR = 9,
} AssAlignmentNumpad;

typedef enum : uint8_t {
	// bottom
	AssAlignmentOldBL = 1,
	AssAlignmentOldBC = 2,
	AssAlignmentOldBR = 3,
	// middle
	AssAlignmentOldML = 8 + 1,
	AssAlignmentOldMC = 8 + 2,
	AssAlignmentOldMR = 8 + 3,
	// top
	AssAlignmentOldTL = 4 + 1,
	AssAlignmentOldTC = 4 + 2,
	AssAlignmentOldTR = 4 + 3,
} AssAlignmentOld;

typedef struct {
	FinalStr name;
	FinalStr fontname;
	size_t fontsize;
	AssColorWithAlpha primary_colour;
	AssColorWithAlpha secondary_colour;
	AssColorWithAlpha outline_colour;
	AssColorWithAlpha back_colour;
	bool bold;
	bool italic;
	bool underline;
	bool strike_out;
	size_t scale_x;
	size_t scale_y;
	double spacing; // TODO: spacing can be size_t ?? use same types for overrides and style!
	double angle;
	BorderStyle border_style;
	double outline;
	double shadow;
	AssAlignmentNumpad alignment;
	size_t margin_l;
	size_t margin_r;
	size_t margin_v;
	EncodingType encoding;
} AssStyleEntry;

TVEC_DEFINE_VEC_TYPE(AssStyleEntry)

typedef struct {
	TVEC_TYPENAME(AssStyleEntry) entries;
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

typedef enum : unsigned char {
	AssTextSpecialCharOpenCurlyBrace = '{',      // "\{" or non closed "{"
	AssTextSpecialCharClosedCurlyBrace = '}',    // "\}" or non opened "}"
	AssTextSpecialCharNewlineSuggestion = '\n',  // "\n"
	AssTextSpecialCharNewlineForced = ('\n') + 1 // "\N"
} AssTextSpecialChar;

typedef enum : bool {
	_2DChoiceX = false,
	_2DChoiceY = true,
} _2DChoice;

typedef enum : uint8_t {
	_3DChoiceX = 0,
	_3DChoiceY,
	_3DChoiceZ,
} _3DChoice;

typedef struct {
	_2DChoice _2dchoice; // x or y
	double scale;
} FontSizeScale;

typedef struct {
	_3DChoice _3dchoice; // x, y or z
	double angle;
} RotationAngle;

typedef enum : uint8_t {
	NumberColorType1 = 1,
	NumberColorType2 = 2,
	NumberColorType3 = 3,
	NumberColorType4 = 4
} NumberColorType;

typedef struct {
	NumberColorType color_type;
	AssColorTODO color;
} AssColorOverride;

typedef enum : uint8_t {
	AlphaTypeAll = 0,
	AlphaType1 = 1,
	AlphaType2 = 2,
	AlphaType3 = 3,
	AlphaType4 = 4
} AlphaType;

typedef struct {
	AlphaType alpha_type; // \alpha or 1-4, \alpha sets all 4!
	uint8_t alpha;
} AssAlphaOverride;

typedef enum : uint8_t {
	KaraokeStyleOverrideTypeNormal,             // \k
	KaraokeStyleOverrideTypeFillUp,             // \K or \kf
	KaraokeStyleOverrideTypeOutlineHighlighting // \ko
} KaraokeStyleOverrideType;

typedef struct {
	KaraokeStyleOverrideType type;
	size_t duration; // in 10ms intervals alias hundreds
} KaraokeStyleOverride;

typedef struct {
	bool has_style; // can eb empty \r, so restore / resets to line style
	FinalStr style;
} RestoreStyle;

struct ASSOverrideFunctionImpl;
typedef struct ASSOverrideFunctionImpl ASSOverrideFunction;

typedef enum : uint8_t {
	StyleOverrideEntryTypeBold = 0,
	StyleOverrideEntryTypeItalic,
	StyleOverrideEntryTypeUnderline,
	StyleOverrideEntryTypeStrikeout,
	StyleOverrideEntryTypeBorder,
	StyleOverrideEntryTypeShadow,
	StyleOverrideEntryTypeBlurEdges,
	StyleOverrideEntryTypeFontName,
	StyleOverrideEntryTypeFontSize,
	StyleOverrideEntryTypeFontSizeScale,
	StyleOverrideEntryTypeFontSpacing,
	StyleOverrideEntryTypeRotationAngle,
	StyleOverrideEntryTypeCharset,
	StyleOverrideEntryTypeColor,
	StyleOverrideEntryTypeAlpha,
	StyleOverrideEntryTypeAlignment,
	StyleOverrideEntryTypeAlignmentNumpad,
	StyleOverrideEntryTypeKaraoke,
	StyleOverrideEntryTypeWrapStyle,
	StyleOverrideEntryTypeRestore,
	StyleOverrideEntryTypeFunction
} StyleOverrideEntryType;

typedef struct {
	StyleOverrideEntryType type;
	union {
		size_t bold;
		bool italic;
		bool underline;
		bool strikeout;
		size_t border;
		size_t shadow;
		bool blur_edges;
		FinalStr font_name;
		size_t font_size;
		FontSizeScale font_size_scale;
		size_t font_spacing; // fsp
		RotationAngle rotation_angle;
		size_t charset;
		AssColorOverride color;
		AssAlphaOverride alpha;
		AssAlignmentOld alignment;
		AssAlignmentNumpad alignment_numpad;
		KaraokeStyleOverride karaoke;
		WrapStyle wrap_style;
		RestoreStyle restore;
		ASSOverrideFunction* function;
	} data;
} StyleOverrideEntry;

TVEC_DEFINE_VEC_TYPE(StyleOverrideEntry)

typedef struct {
	TVEC_TYPENAME(StyleOverrideEntry) overrides;
} StyleOverride;

//TODO support also template parsing!

typedef enum : uint8_t {
	ASSOverrideFunctionTypeT = 0,
	ASSOverrideFunctionTypeMove,
	ASSOverrideFunctionTypePos,
	ASSOverrideFunctionTypeOrg,
	ASSOverrideFunctionTypeFade,
	ASSOverrideFunctionTypeFad,
	ASSOverrideFunctionTypeClip,
}ASSOverrideFunctionType;

//TODO: also check aegisub functions and manual!

struct ASSOverrideFunctionImpl{
	ASSOverrideFunctionType type;
	// TODO: \t() can have a vec of StyleOverrideEntry as argument
};

typedef enum : uint8_t {
	AssTextEntryTypeText,
	AssTextEntryTypeStyleOverride,
	AssTextEntryTypeSpecialChar
} AssTextEntryType;

typedef struct {
	AssTextEntryType type;
	union {
		FinalStr text;
		StyleOverride style_override;
		AssTextSpecialChar special_char;
	} data;
} AssTextEntry;

TVEC_DEFINE_VEC_TYPE(AssTextEntry)

typedef struct {
	FinalStr original;
	TVEC_TYPENAME(AssTextEntry) parsed;
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

TVEC_DEFINE_VEC_TYPE(AssEventEntry)

typedef struct {
	TVEC_TYPENAME(AssEventEntry) entries;
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

TVEC_DEFINE_VEC_TYPE(AssFontEntry)

typedef struct {
	TVEC_TYPENAME(AssFontEntry) entries;
} AssFonts;

typedef struct {
	FinalStr name;
	SizedPtr data;
} AssGraphicEntry;

TVEC_DEFINE_VEC_TYPE(AssGraphicEntry)

typedef struct {
	TVEC_TYPENAME(AssGraphicEntry) entries;
} AssGraphics;

#define CHAR_PTR_KEYNAME CString

TMAP_DEFINE_MAP_TYPE(char*, CHAR_PTR_KEYNAME, FinalStr, SectionFieldEntry)

typedef TMAP_TYPENAME_MAP(SectionFieldEntry) ExtraSectionEntry;
typedef TMAP_TYPENAME_ENTRY(SectionFieldEntry) SectionFieldEntry;

TMAP_DEFINE_MAP_TYPE(char*, CHAR_PTR_KEYNAME, ExtraSectionEntry, ExtraSectionHashMapEntry)

typedef TMAP_TYPENAME_MAP(ExtraSectionHashMapEntry) ExtraSections;
typedef TMAP_TYPENAME_ENTRY(ExtraSectionHashMapEntry) ExtraSectionHashMapEntry;

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
