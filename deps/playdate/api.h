#ifndef PLAYDATE_H
#define PLAYDATE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// -- pd_api_display.h ---------------------------------------------------------

struct playdate_display
{
	int (*getWidth)(void);
	int (*getHeight)(void);
	
	void (*setRefreshRate)(float rate);

	void (*setInverted)(int flag);
	void (*setScale)(unsigned int s);
	void (*setMosaic)(unsigned int x, unsigned int y);
	void (*setFlipped)(int x, int y);
	void (*setOffset)(int x, int y);
};

// -- pd_api_file.h ------------------------------------------------------------

#if TARGET_EXTENSION

typedef void SDFile;

typedef enum
{
	kFileRead		= (1<<0),
	kFileReadData	= (1<<1),
	kFileWrite		= (1<<2),
	kFileAppend		= (2<<2)
} FileOptions;

typedef struct
{
	int isdir;
	unsigned int size;
	int m_year;
	int m_month;
	int m_day;
	int m_hour;
	int m_minute;
	int m_second;
} FileStat;

#endif // TARGET_EXTENSION

#if !defined(SEEK_SET)
#  define SEEK_SET        0       /* Seek from beginning of file.  */
#  define SEEK_CUR        1       /* Seek from current position.  */
#  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#endif // defined(SEEK_SET)

struct playdate_file
{
	const char* (*geterr)(void);

	int		(*listfiles)(const char* path, void (*callback)(const char* path, void* userdata), void* userdata, int showhidden);
	int		(*stat)(const char* path, FileStat* stat);
	int		(*mkdir)(const char* path);
	int		(*unlink)(const char* name, int recursive);
	int		(*rename)(const char* from, const char* to);
	
	SDFile*	(*open)(const char* name, FileOptions mode);
	int		(*close)(SDFile* file);
	int		(*read)(SDFile* file, void* buf, unsigned int len);
	int		(*write)(SDFile* file, const void* buf, unsigned int len);
	int		(*flush)(SDFile* file);
	int		(*tell)(SDFile* file);
	int		(*seek)(SDFile* file, int pos, int whence);
};

// -- pd_api_gfx.h -------------------------------------------------------------

#if TARGET_EXTENSION

typedef struct
{
	int left;
	int right; // not inclusive
	int top;
	int bottom; // not inclusive
} LCDRect;

static inline LCDRect LCDMakeRect(int x, int y, int width, int height)
{
	// XXX - assumes width and height are positive
	LCDRect r = { .left = x, .right = x + width, .top = y, .bottom = y + height };
	return r;
}

static inline LCDRect LCDRect_translate(LCDRect r, int dx, int dy)
{
	LCDRect rect = { .left = r.left + dx, .right = r.right + dx, .top = r.top + dy, .bottom = r.bottom + dy };
	return rect;
}

#define LCD_COLUMNS	400
#define LCD_ROWS	240
#define LCD_ROWSIZE 52
#define LCD_SCREEN_RECT LCDMakeRect(0,0,LCD_COLUMNS,LCD_ROWS)

typedef enum
{
	kDrawModeCopy,
	kDrawModeWhiteTransparent,
	kDrawModeBlackTransparent,
	kDrawModeFillWhite,
	kDrawModeFillBlack,
	kDrawModeXOR,
	kDrawModeNXOR,
	kDrawModeInverted
} LCDBitmapDrawMode;

typedef enum
{
	kBitmapUnflipped,
	kBitmapFlippedX,
	kBitmapFlippedY,
	kBitmapFlippedXY
} LCDBitmapFlip;

typedef enum
{
	kColorBlack,
	kColorWhite,
	kColorClear,
	kColorXOR
} LCDSolidColor;

typedef enum
{
	kLineCapStyleButt,
	kLineCapStyleSquare,
	kLineCapStyleRound
} LCDLineCapStyle;

typedef enum
{
	kLCDFontLanguageEnglish,
	kLCDFontLanguageJapanese,
	kLCDFontLanguageUnknown,
} LCDFontLanguage;

typedef enum
{
	kASCIIEncoding,
	kUTF8Encoding,
	k16BitLEEncoding
} PDStringEncoding;

typedef uint8_t LCDPattern[16]; // 8x8 pattern: 8 rows image data, 8 rows mask
typedef uintptr_t LCDColor; // LCDSolidColor or pointer to LCDPattern

#define LCDOpaquePattern(r0,r1,r2,r3,r4,r5,r6,r7) {(r0),(r1),(r2),(r3),(r4),(r5),(r6),(r7),0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}

typedef enum
{
	kPolygonFillNonZero,
	kPolygonFillEvenOdd
} LCDPolygonFillRule;

#endif // TARGET_EXTENSION

typedef struct LCDBitmap LCDBitmap;
typedef struct LCDBitmapTable LCDBitmapTable;
typedef struct LCDFont LCDFont;
typedef struct LCDFontData LCDFontData;
typedef struct LCDFontPage LCDFontPage;
typedef struct LCDFontGlyph LCDFontGlyph;
typedef struct LCDVideoPlayer LCDVideoPlayer;

struct playdate_video
{
	LCDVideoPlayer* (*loadVideo)(const char* path);
	void (*freePlayer)(LCDVideoPlayer* p);
	int (*setContext)(LCDVideoPlayer* p, LCDBitmap* context);
	void (*useScreenContext)(LCDVideoPlayer* p);
	int (*renderFrame)(LCDVideoPlayer* p, int n);
	const char* (*getError)(LCDVideoPlayer* p);
	void (*getInfo)(LCDVideoPlayer* p, int* outWidth, int* outHeight, float* outFrameRate, int* outFrameCount, int* outCurrentFrame);
	LCDBitmap* (*getContext)(LCDVideoPlayer *p);
};

struct playdate_graphics
{
	const struct playdate_video* video;

	// Drawing Functions
	void (*clear)(LCDColor color);
	void (*setBackgroundColor)(LCDSolidColor color);
	void (*setStencil)(LCDBitmap* stencil); // deprecated in favor of setStencilImage, which adds a "tile" flag
	LCDBitmapDrawMode (*setDrawMode)(LCDBitmapDrawMode mode);
	void (*setDrawOffset)(int dx, int dy);
	void (*setClipRect)(int x, int y, int width, int height);
	void (*clearClipRect)(void);
	void (*setLineCapStyle)(LCDLineCapStyle endCapStyle);
	void (*setFont)(LCDFont* font);
	void (*setTextTracking)(int tracking);
	void (*pushContext)(LCDBitmap* target);
	void (*popContext)(void);

	void (*drawBitmap)(LCDBitmap* bitmap, int x, int y, LCDBitmapFlip flip);
	void (*tileBitmap)(LCDBitmap* bitmap, int x, int y, int width, int height, LCDBitmapFlip flip);
	void (*drawLine)(int x1, int y1, int x2, int y2, int width, LCDColor color);
	void (*fillTriangle)(int x1, int y1, int x2, int y2, int x3, int y3, LCDColor color);
	void (*drawRect)(int x, int y, int width, int height, LCDColor color);
	void (*fillRect)(int x, int y, int width, int height, LCDColor color);
	void (*drawEllipse)(int x, int y, int width, int height, int lineWidth, float startAngle, float endAngle, LCDColor color); // stroked inside the rect
	void (*fillEllipse)(int x, int y, int width, int height, float startAngle, float endAngle, LCDColor color);
	void (*drawScaledBitmap)(LCDBitmap* bitmap, int x, int y, float xscale, float yscale);
	int  (*drawText)(const void* text, size_t len, PDStringEncoding encoding, int x, int y);

	// LCDBitmap
	LCDBitmap* (*newBitmap)(int width, int height, LCDColor bgcolor);
	void (*freeBitmap)(LCDBitmap*);
	LCDBitmap* (*loadBitmap)(const char* path, const char** outerr);
	LCDBitmap* (*copyBitmap)(LCDBitmap* bitmap);
	void (*loadIntoBitmap)(const char* path, LCDBitmap* bitmap, const char** outerr);
	void (*getBitmapData)(LCDBitmap* bitmap, int* width, int* height, int* rowbytes, uint8_t** mask, uint8_t** data);
	void (*clearBitmap)(LCDBitmap* bitmap, LCDColor bgcolor);
	LCDBitmap* (*rotatedBitmap)(LCDBitmap* bitmap, float rotation, float xscale, float yscale, int* allocedSize);

	// LCDBitmapTable
	LCDBitmapTable* (*newBitmapTable)(int count, int width, int height);
	void (*freeBitmapTable)(LCDBitmapTable* table);
	LCDBitmapTable* (*loadBitmapTable)(const char* path, const char** outerr);
	void (*loadIntoBitmapTable)(const char* path, LCDBitmapTable* table, const char** outerr);
	LCDBitmap* (*getTableBitmap)(LCDBitmapTable* table, int idx);

	// LCDFont
	LCDFont* (*loadFont)(const char* path, const char** outErr);
	LCDFontPage* (*getFontPage)(LCDFont* font, uint32_t c);
	LCDFontGlyph* (*getPageGlyph)(LCDFontPage* page, uint32_t c, LCDBitmap** bitmap, int* advance);
	int (*getGlyphKerning)(LCDFontGlyph* glyph, uint32_t glyphcode, uint32_t nextcode);
	int (*getTextWidth)(LCDFont* font, const void* text, size_t len, PDStringEncoding encoding, int tracking);

	// raw framebuffer access
	uint8_t* (*getFrame)(void); // row stride = LCD_ROWSIZE
	uint8_t* (*getDisplayFrame)(void); // row stride = LCD_ROWSIZE
	LCDBitmap* (*getDebugBitmap)(void); // valid in simulator only, function is NULL on device
	LCDBitmap* (*copyFrameBufferBitmap)(void);
	void (*markUpdatedRows)(int start, int end);
	void (*display)(void);

	// misc util.
	void (*setColorToPattern)(LCDColor* color, LCDBitmap* bitmap, int x, int y);
	int (*checkMaskCollision)(LCDBitmap* bitmap1, int x1, int y1, LCDBitmapFlip flip1, LCDBitmap* bitmap2, int x2, int y2, LCDBitmapFlip flip2, LCDRect rect);

	// 1.1
	void (*setScreenClipRect)(int x, int y, int width, int height);

	// 1.1.1
	void (*fillPolygon)(int nPoints, int* coords, LCDColor color, LCDPolygonFillRule fillrule);
	uint8_t (*getFontHeight)(LCDFont* font);

	// 1.7
	LCDBitmap* (*getDisplayBufferBitmap)(void);
	void (*drawRotatedBitmap)(LCDBitmap* bitmap, int x, int y, float rotation, float centerx, float centery, float xscale, float yscale);
	void (*setTextLeading)(int lineHeightAdustment);

	// 1.8
	int (*setBitmapMask)(LCDBitmap* bitmap, LCDBitmap* mask);
	LCDBitmap* (*getBitmapMask)(LCDBitmap* bitmap);

	// 1.10
	void (*setStencilImage)(LCDBitmap* stencil, int tile);

	// 1.12
	LCDFont* (*makeFontFromData)(LCDFontData* data, int wide);

	// 2.1
	int (*getTextTracking)(void);

	// 2.5
	void (*setPixel)(int x, int y, LCDColor c);
	LCDSolidColor (*getBitmapPixel)(LCDBitmap* bitmap, int x, int y);
	void (*getBitmapTableInfo)(LCDBitmapTable* table, int* count, int* width);
};

// -- pd_api_json.h ------------------------------------------------------------

#if TARGET_EXTENSION

typedef enum
{
	kJSONNull,
	kJSONTrue,
	kJSONFalse,
	kJSONInteger,
	kJSONFloat,
	kJSONString,
	kJSONArray,
	kJSONTable
} json_value_type;

typedef struct
{
	char type;
	
	union
	{
		int intval;
		float floatval;
		char* stringval;
		void* arrayval;
		void* tableval;
	} data;
} json_value;

static inline int json_intValue(json_value value) {
	switch (value.type) {
		case kJSONInteger: return value.data.intval;
		case kJSONFloat:   return (int)value.data.floatval;
		case kJSONString:  return (int)strtol(value.data.stringval, NULL, 10);
		case kJSONTrue:    return 1;
		default:           return 0;
	}
}

static inline float json_floatValue(json_value value) {
	switch (value.type) {
		case kJSONInteger: return (float)value.data.intval;
		case kJSONFloat:   return value.data.floatval;
		case kJSONString:  return 0; //strtof(value.data.stringval, NULL); XXX - required _strbk
		case kJSONTrue:    return 1.0;
		default:           return 0.0;
	}
}

static inline int json_boolValue(json_value value) {
	return value.type == kJSONString ? strcmp(value.data.stringval,"") != 0 : json_intValue(value);
}

static inline char* json_stringValue(json_value value) {
	return value.type == kJSONString ? value.data.stringval : NULL;
}


// decoder

typedef struct json_decoder json_decoder;

struct json_decoder
{
	void (*decodeError)(json_decoder* decoder, const char* error, int linenum);
	
	// the following functions are each optional
	void (*willDecodeSublist)(json_decoder* decoder, const char* name, json_value_type type);
	int (*shouldDecodeTableValueForKey)(json_decoder* decoder, const char* key);
	void (*didDecodeTableValue)(json_decoder* decoder, const char* key, json_value value);
	int (*shouldDecodeArrayValueAtIndex)(json_decoder* decoder, int pos);
	void (*didDecodeArrayValue)(json_decoder* decoder, int pos, json_value value); // if pos==0, this was a bare value at the root of the file
	void* (*didDecodeSublist)(json_decoder* decoder, const char* name, json_value_type type);
	
	void* userdata;
	int returnString; // when set, the decoder skips parsing and returns the current subtree as a string
	const char* path; // updated during parsing, reflects current position in tree
};

// convenience functions for setting up a table-only or array-only decoder

static inline void json_setTableDecode(json_decoder* decoder,
									   void (*willDecodeSublist)(json_decoder* decoder, const char* name, json_value_type type),
									   void (*didDecodeTableValue)(json_decoder* decoder, const char* key, json_value value),
									   void* (*didDecodeSublist)(json_decoder* decoder, const char* name, json_value_type type))
{
	decoder->didDecodeTableValue = didDecodeTableValue;
	decoder->didDecodeArrayValue = NULL;
	decoder->willDecodeSublist = willDecodeSublist;
	decoder->didDecodeSublist = didDecodeSublist;
}

static inline void json_setArrayDecode(json_decoder* decoder,
									   void (*willDecodeSublist)(json_decoder* decoder, const char* name, json_value_type type),
									   void (*didDecodeArrayValue)(json_decoder* decoder, int pos, json_value value),
									   void* (*didDecodeSublist)(json_decoder* decoder, const char* name, json_value_type type))
{
	decoder->didDecodeTableValue = NULL;
	decoder->didDecodeArrayValue = didDecodeArrayValue;
	decoder->willDecodeSublist = willDecodeSublist;
	decoder->didDecodeSublist = didDecodeSublist;
}

// fill buffer, return bytes written or -1 on end of data
typedef int (json_readFunc)(void* userdata, uint8_t* buf, int bufsize);

typedef struct
{
    json_readFunc* read;
	void* userdata; // passed back to the read function above
} json_reader;


// encoder

typedef void (json_writeFunc)(void* userdata, const char* str, int len);

typedef struct json_encoder
{
    json_writeFunc* writeStringFunc;
	void* userdata;
	
	int pretty : 1;
	int startedTable : 1;
	int startedArray : 1;
	int depth : 29;
	
	void (*startArray)(struct json_encoder* encoder);
	void (*addArrayMember)(struct json_encoder* encoder);
	void (*endArray)(struct json_encoder* encoder);
	void (*startTable)(struct json_encoder* encoder);
	void (*addTableMember)(struct json_encoder* encoder, const char* name, int len);
	void (*endTable)(struct json_encoder* encoder);
	void (*writeNull)(struct json_encoder* encoder);
	void (*writeFalse)(struct json_encoder* encoder);
	void (*writeTrue)(struct json_encoder* encoder);
	void (*writeInt)(struct json_encoder* encoder, int num);
	void (*writeDouble)(struct json_encoder* encoder, double num);
	void (*writeString)(struct json_encoder* encoder, const char* str, int len);
	
} json_encoder;

#endif // TARGET_EXTENSION

struct playdate_json
{
	void (*initEncoder)(json_encoder* encoder, json_writeFunc* write, void* userdata, int pretty);

	int (*decode)(struct json_decoder* functions, json_reader reader, json_value* outval);
	int (*decodeString)(struct json_decoder* functions, const char* jsonString, json_value* outval);
};

// -- pd_api_lua.h -------------------------------------------------------------

#if TARGET_EXTENSION

typedef void* lua_State;
typedef int (*lua_CFunction)(lua_State* L);

typedef struct LuaUDObject LuaUDObject;
typedef struct LCDSprite LCDSprite;

typedef enum { kInt, kFloat, kStr } l_valtype;

#endif // TARGET_EXTENSION

typedef struct
{
	const char *name;
	lua_CFunction func;
} lua_reg;

enum LuaType
{
	kTypeNil,
	kTypeBool,
	kTypeInt,
	kTypeFloat,
	kTypeString,
	kTypeTable,
	kTypeFunction,
	kTypeThread,
	kTypeObject
};

typedef struct
{
	const char *name;
	l_valtype type;
	union
	{
		unsigned int intval;
		float floatval;
		const char* strval;
	} v;
} lua_val;

struct playdate_lua
{
	// these two return 1 on success, else 0 with an error message in outErr
	int (*addFunction)(lua_CFunction f, const char* name, const char** outErr);
	int (*registerClass)(const char* name, const lua_reg* reg, const lua_val* vals, int isstatic, const char** outErr);

	void (*pushFunction)(lua_CFunction f);
	int (*indexMetatable)(void);

	void (*stop)(void);
	void (*start)(void);
	
	// stack operations
	int (*getArgCount)(void);
	enum LuaType (*getArgType)(int pos, const char** outClass);

	int (*argIsNil)(int pos);
	int (*getArgBool)(int pos);
	int (*getArgInt)(int pos);
	float (*getArgFloat)(int pos);
	const char* (*getArgString)(int pos);
	const char* (*getArgBytes)(int pos, size_t* outlen);
	void* (*getArgObject)(int pos, char* type, LuaUDObject** outud);
	
	LCDBitmap* (*getBitmap)(int pos);
	LCDSprite* (*getSprite)(int pos);

	// for returning values back to Lua
	void (*pushNil)(void);
	void (*pushBool)(int val);
	void (*pushInt)(int val);
	void (*pushFloat)(float val);
	void (*pushString)(const char* str);
	void (*pushBytes)(const char* str, size_t len);
	void (*pushBitmap)(LCDBitmap* bitmap);
	void (*pushSprite)(LCDSprite* sprite);
	
	LuaUDObject* (*pushObject)(void* obj, char* type, int nValues);
	LuaUDObject* (*retainObject)(LuaUDObject* obj);
	void (*releaseObject)(LuaUDObject* obj);
	
	void (*setUserValue)(LuaUDObject* obj, unsigned int slot); // sets item on top of stack and pops it
	int (*getUserValue)(LuaUDObject* obj, unsigned int slot); // pushes item at slot to top of stack, returns stack position

	// calling lua from C has some overhead. use sparingly!
	void (*callFunction_deprecated)(const char* name, int nargs);
	int (*callFunction)(const char* name, int nargs, const char** outerr);
};

// -- pd_api_scoreboards.h -----------------------------------------------------

typedef struct {
	uint32_t rank;
	uint32_t value;
	char *player;
} PDScore;

typedef struct {
	char *boardID;
	unsigned int count;
	uint32_t lastUpdated;
	int playerIncluded;
	unsigned int limit;
	PDScore *scores;
} PDScoresList;

typedef struct {
	char *boardID;
	char *name;
} PDBoard;

typedef struct {
	unsigned int count;
	uint32_t lastUpdated;
	PDBoard *boards;
} PDBoardsList;

typedef void (*AddScoreCallback)(PDScore *score, const char* errorMessage);
typedef void (*PersonalBestCallback)(PDScore *score, const char *errorMessage);
typedef void (*BoardsListCallback)(PDBoardsList *boards, const char *errorMessage);
typedef void (*ScoresCallback)(PDScoresList *scores, const char *errorMessage);

struct playdate_scoreboards
{
	int (*addScore)(const char *boardId, uint32_t value, AddScoreCallback callback);
	int (*getPersonalBest)(const char *boardId, PersonalBestCallback callback);
	void (*freeScore)(PDScore *score);

	int (*getScoreboards)(BoardsListCallback callback);
	void (*freeBoardsList)(PDBoardsList *boardsList);

	int (*getScores)(const char *boardId, ScoresCallback callback);
	void (*freeScoresList)(PDScoresList *scoresList);
};

// -- pd_api_sound.h -----------------------------------------------------------

#if TARGET_EXTENSION

#define AUDIO_FRAMES_PER_CYCLE 512

typedef enum
{
	kSound8bitMono = 0,
	kSound8bitStereo = 1,
	kSound16bitMono = 2,
	kSound16bitStereo = 3,
	kSoundADPCMMono = 4,
	kSoundADPCMStereo = 5
} SoundFormat;

#define SoundFormatIsStereo(f) ((f)&1)
#define SoundFormatIs16bit(f) ((f)>=kSound16bitMono)

static inline uint32_t SoundFormat_bytesPerFrame(SoundFormat fmt)
{
	return (SoundFormatIsStereo(fmt) ? 2 : 1) * (SoundFormatIs16bit(fmt) ? 2 : 1);
}

#endif // TARGET_EXTENSION

typedef float MIDINote;
#define NOTE_C4 60

#include <math.h>
static inline float pd_noteToFrequency(MIDINote n) { return 440 * powf(2.0, (n-69)/12.0f); }
static inline MIDINote pd_frequencyToNote(float f) { return 12*log2f(f) - 36.376316562f; }

// SOUND SOURCES

typedef struct SoundSource SoundSource;
typedef void sndCallbackProc(SoundSource* c, void* userdata);

// SoundSource is the parent class for FilePlayer, SamplePlayer, PDSynth, and DelayLineTap. You can safely cast those objects to a SoundSource* and use these functions:

struct playdate_sound_source
{
	void (*setVolume)(SoundSource* c, float lvol, float rvol);
	void (*getVolume)(SoundSource* c, float* outl, float* outr);
	int (*isPlaying)(SoundSource* c);
	void (*setFinishCallback)(SoundSource* c, sndCallbackProc callback, void* userdata);
};

typedef struct FilePlayer FilePlayer; // extends SoundSource

struct playdate_sound_fileplayer
{
	FilePlayer* (*newPlayer)(void);
	void (*freePlayer)(FilePlayer* player);
	int (*loadIntoPlayer)(FilePlayer* player, const char* path);
	void (*setBufferLength)(FilePlayer* player, float bufferLen);
	int (*play)(FilePlayer* player, int repeat);
	int (*isPlaying)(FilePlayer* player);
	void (*pause)(FilePlayer* player);
	void (*stop)(FilePlayer* player);
	void (*setVolume)(FilePlayer* player, float left, float right);
	void (*getVolume)(FilePlayer* player, float* left, float* right);
	float (*getLength)(FilePlayer* player);
	void (*setOffset)(FilePlayer* player, float offset);
	void (*setRate)(FilePlayer* player, float rate);
	void (*setLoopRange)(FilePlayer* player, float start, float end);
	int (*didUnderrun)(FilePlayer* player);
	void (*setFinishCallback)(FilePlayer* player, sndCallbackProc callback, void* userdata);
	void (*setLoopCallback)(FilePlayer* player, sndCallbackProc callback, void* userdata);
	float (*getOffset)(FilePlayer* player);
	float (*getRate)(FilePlayer* player);
	void (*setStopOnUnderrun)(FilePlayer* player, int flag);
	void (*fadeVolume)(FilePlayer* player, float left, float right, int32_t len, sndCallbackProc finishCallback, void* userdata);
	void (*setMP3StreamSource)(FilePlayer* player, int (*dataSource)(uint8_t* data, int bytes, void* userdata), void* userdata, float bufferLen);
};


typedef struct AudioSample AudioSample;
typedef struct SamplePlayer SamplePlayer;

struct playdate_sound_sample
{
	AudioSample* (*newSampleBuffer)(int byteCount);
	int (*loadIntoSample)(AudioSample* sample, const char* path);
	AudioSample* (*load)(const char* path);
	AudioSample* (*newSampleFromData)(uint8_t* data, SoundFormat format, uint32_t sampleRate, int byteCount, int shouldFreeData);
	void (*getData)(AudioSample* sample, uint8_t** data, SoundFormat* format, uint32_t* sampleRate, uint32_t* bytelength);
	void (*freeSample)(AudioSample* sample);
	float (*getLength)(AudioSample* sample);
	
	// 2.4
	int (*decompress)(AudioSample* sample);
};

struct playdate_sound_sampleplayer // SamplePlayer extends SoundSource
{
	SamplePlayer* (*newPlayer)(void);
	void (*freePlayer)(SamplePlayer* player);
	void (*setSample)(SamplePlayer* player, AudioSample* sample);
	int (*play)(SamplePlayer* player, int repeat, float rate);
	int (*isPlaying)(SamplePlayer* player);
	void (*stop)(SamplePlayer* player);
	void (*setVolume)(SamplePlayer* player, float left, float right);
	void (*getVolume)(SamplePlayer* player, float* left, float* right);
	float (*getLength)(SamplePlayer* player);
	void (*setOffset)(SamplePlayer* player, float offset);
	void (*setRate)(SamplePlayer* player, float rate);
	void (*setPlayRange)(SamplePlayer* player, int start, int end);
	void (*setFinishCallback)(SamplePlayer* player, sndCallbackProc callback, void* userdata);
	void (*setLoopCallback)(SamplePlayer* player, sndCallbackProc callback, void* userdata);
	float (*getOffset)(SamplePlayer* player);
	float (*getRate)(SamplePlayer* player);
	void (*setPaused)(SamplePlayer* player, int flag);
};


// SIGNALS

// a PDSynthSignalValue represents a signal that can be used as an input to a modulator.
// its PDSynthSignal subclass is used for "active" signals that change their values automatically. PDSynthLFO and PDSynthEnvelope are subclasses of PDSynthSignal.

typedef struct PDSynthSignalValue PDSynthSignalValue;
typedef struct PDSynthSignal PDSynthSignal;

typedef float (*signalStepFunc)(void* userdata, int* ioframes, float* ifval);
typedef void (*signalNoteOnFunc)(void* userdata, MIDINote note, float vel, float len); // len = -1 for indefinite
typedef void (*signalNoteOffFunc)(void* userdata, int stop, int offset); // stop = 0 for note release, = 1 when note stops playing
typedef void (*signalDeallocFunc)(void* userdata);

struct playdate_sound_signal
{
	PDSynthSignal* (*newSignal)(signalStepFunc step, signalNoteOnFunc noteOn, signalNoteOffFunc noteOff, signalDeallocFunc dealloc, void* userdata);
	void (*freeSignal)(PDSynthSignal* signal);
	float (*getValue)(PDSynthSignal* signal);
	void (*setValueScale)(PDSynthSignal* signal, float scale);
	void (*setValueOffset)(PDSynthSignal* signal, float offset);
};

#if TARGET_EXTENSION
typedef enum
{
	kLFOTypeSquare,
	kLFOTypeTriangle,
	kLFOTypeSine,
	kLFOTypeSampleAndHold,
	kLFOTypeSawtoothUp,
	kLFOTypeSawtoothDown,
	kLFOTypeArpeggiator,
	kLFOTypeFunction
} LFOType;
#endif // TARGET_EXTENSION

typedef struct PDSynthLFO PDSynthLFO; // inherits from SynthSignal

struct playdate_sound_lfo
{
	PDSynthLFO* (*newLFO)(LFOType type);
	void (*freeLFO)(PDSynthLFO* lfo);
	
	void (*setType)(PDSynthLFO* lfo, LFOType type);
	void (*setRate)(PDSynthLFO* lfo, float rate);
	void (*setPhase)(PDSynthLFO* lfo, float phase);
	void (*setCenter)(PDSynthLFO* lfo, float center);
	void (*setDepth)(PDSynthLFO* lfo, float depth);
	void (*setArpeggiation)(PDSynthLFO* lfo, int nSteps, float* steps);
	void (*setFunction)(PDSynthLFO* lfo, float (*lfoFunc)(PDSynthLFO* lfo, void* userdata), void* userdata, int interpolate);
	void (*setDelay)(PDSynthLFO* lfo, float holdoff, float ramptime);
	void (*setRetrigger)(PDSynthLFO* lfo, int flag);
	
	float (*getValue)(PDSynthLFO* lfo);
	
	// 1.10
	void (*setGlobal)(PDSynthLFO* lfo, int global);
	
	// 2.2
	void (*setStartPhase)(PDSynthLFO* lfo, float phase);
};


typedef struct PDSynthEnvelope PDSynthEnvelope; // inherits from SynthSignal

struct playdate_sound_envelope
{
	PDSynthEnvelope* (*newEnvelope)(float attack, float decay, float sustain, float release);
	void (*freeEnvelope)(PDSynthEnvelope* env);
	
	void (*setAttack)(PDSynthEnvelope* env, float attack);
	void (*setDecay)(PDSynthEnvelope* env, float decay);
	void (*setSustain)(PDSynthEnvelope* env, float sustain);
	void (*setRelease)(PDSynthEnvelope* env, float release);
	
	void (*setLegato)(PDSynthEnvelope* env, int flag);
	void (*setRetrigger)(PDSynthEnvelope* lfo, int flag);

	float (*getValue)(PDSynthEnvelope* env);
	
	// 1.13
	void (*setCurvature)(PDSynthEnvelope* env, float amount);
	void (*setVelocitySensitivity)(PDSynthEnvelope* env, float velsens);
	void (*setRateScaling)(PDSynthEnvelope* env, float scaling, MIDINote start, MIDINote end);
};


// SYNTHS

#if TARGET_EXTENSION
typedef enum
{
	kWaveformSquare,
	kWaveformTriangle,
	kWaveformSine,
	kWaveformNoise,
	kWaveformSawtooth,
	kWaveformPOPhase,
	kWaveformPODigital,
	kWaveformPOVosim
} SoundWaveform;
#endif // TARGET_EXTENSION

// generator render callback
// samples are in Q8.24 format. left is either the left channel or the single mono channel,
// right is non-NULL only if the stereo flag was set in the setGenerator() call.
// nsamples is at most 256 but may be shorter
// rate is Q0.32 per-frame phase step, drate is per-frame rate step (i.e., do rate += drate every frame)
// return value is the number of sample frames rendered
typedef int (*synthRenderFunc)(void* userdata, int32_t* left, int32_t* right, int nsamples, uint32_t rate, int32_t drate);

// generator event callbacks
typedef void (*synthNoteOnFunc)(void* userdata, MIDINote note, float velocity, float len); // len == -1 if indefinite
typedef void (*synthReleaseFunc)(void* userdata, int stop);
typedef int (*synthSetParameterFunc)(void* userdata, int parameter, float value);
typedef void (*synthDeallocFunc)(void* userdata);
typedef void* (*synthCopyUserdata)(void* userdata);
typedef struct PDSynth PDSynth;

struct playdate_sound_synth // PDSynth extends SoundSource
{
	PDSynth* (*newSynth)(void);
	void (*freeSynth)(PDSynth* synth);
	
	void (*setWaveform)(PDSynth* synth, SoundWaveform wave);
	void (*setGenerator_deprecated)(PDSynth* synth, int stereo, synthRenderFunc render, synthNoteOnFunc noteOn, synthReleaseFunc release, synthSetParameterFunc setparam, synthDeallocFunc dealloc, void* userdata);
	void (*setSample)(PDSynth* synth, AudioSample* sample, uint32_t sustainStart, uint32_t sustainEnd);

	void (*setAttackTime)(PDSynth* synth, float attack);
	void (*setDecayTime)(PDSynth* synth, float decay);
	void (*setSustainLevel)(PDSynth* synth, float sustain);
	void (*setReleaseTime)(PDSynth* synth, float release);
	
	void (*setTranspose)(PDSynth* synth, float halfSteps);
	
	void (*setFrequencyModulator)(PDSynth* synth, PDSynthSignalValue* mod);
	PDSynthSignalValue* (*getFrequencyModulator)(PDSynth* synth);
	void (*setAmplitudeModulator)(PDSynth* synth, PDSynthSignalValue* mod);
	PDSynthSignalValue* (*getAmplitudeModulator)(PDSynth* synth);
	
	int (*getParameterCount)(PDSynth* synth);
	int (*setParameter)(PDSynth* synth, int parameter, float value);
	void (*setParameterModulator)(PDSynth* synth, int parameter, PDSynthSignalValue* mod);
	PDSynthSignalValue* (*getParameterModulator)(PDSynth* synth, int parameter);
	
	void (*playNote)(PDSynth* synth, float freq, float vel, float len, uint32_t when); // len == -1 for indefinite
	void (*playMIDINote)(PDSynth* synth, MIDINote note, float vel, float len, uint32_t when); // len == -1 for indefinite
	void (*noteOff)(PDSynth* synth, uint32_t when); // move to release part of envelope
	void (*stop)(PDSynth* synth); // stop immediately
	
	void (*setVolume)(PDSynth* synth, float left, float right);
	void (*getVolume)(PDSynth* synth, float* left, float* right);
	
	int (*isPlaying)(PDSynth* synth);
	
	// 1.13
	PDSynthEnvelope* (*getEnvelope)(PDSynth* synth); // synth keeps ownership--don't free this!
	
	// 2.2
	int (*setWavetable)(PDSynth* synth, AudioSample* sample, int log2size, int columns, int rows);
	
	// 2.4
	void (*setGenerator)(PDSynth* synth, int stereo, synthRenderFunc render, synthNoteOnFunc noteOn, synthReleaseFunc release, synthSetParameterFunc setparam, synthDeallocFunc dealloc, synthCopyUserdata copyUserdata, void* userdata);
	PDSynth* (*copy)(PDSynth* synth);
};


// SEQUENCES

// a ControlSignal is a PDSynthSignal with values specified on a timeline

typedef struct ControlSignal ControlSignal;

struct playdate_control_signal
{
	ControlSignal* (*newSignal)(void);
	void (*freeSignal)(ControlSignal* signal);
	void (*clearEvents)(ControlSignal* control);
	void (*addEvent)(ControlSignal* control, int step, float value, int interpolate);
	void (*removeEvent)(ControlSignal* control, int step);
	int (*getMIDIControllerNumber)(ControlSignal* control);
};


// a PDSynthInstrument is a bank of voices for playing a sequence track

typedef struct PDSynthInstrument PDSynthInstrument;

struct playdate_sound_instrument
{
	PDSynthInstrument* (*newInstrument)(void);
	void (*freeInstrument)(PDSynthInstrument* inst);
	int (*addVoice)(PDSynthInstrument* inst, PDSynth* synth, MIDINote rangeStart, MIDINote rangeEnd, float transpose);
	PDSynth* (*playNote)(PDSynthInstrument* inst, float frequency, float vel, float len, uint32_t when);
	PDSynth* (*playMIDINote)(PDSynthInstrument* inst, MIDINote note, float vel, float len, uint32_t when);
	void (*setPitchBend)(PDSynthInstrument* inst, float bend);
	void (*setPitchBendRange)(PDSynthInstrument* inst, float halfSteps);
	void (*setTranspose)(PDSynthInstrument* inst, float halfSteps);
	void (*noteOff)(PDSynthInstrument* inst, MIDINote note, uint32_t when);
	void (*allNotesOff)(PDSynthInstrument* inst, uint32_t when);
	void (*setVolume)(PDSynthInstrument* inst, float left, float right);
	void (*getVolume)(PDSynthInstrument* inst, float* left, float* right);
	int (*activeVoiceCount)(PDSynthInstrument* inst);
};


// a SequenceTrack contains notes for an instrument to play

typedef struct SequenceTrack SequenceTrack;

struct playdate_sound_track
{
	SequenceTrack* (*newTrack)(void);
	void (*freeTrack)(SequenceTrack* track);
	
	void (*setInstrument)(SequenceTrack* track, PDSynthInstrument* inst);
	PDSynthInstrument* (*getInstrument)(SequenceTrack* track);
	
	void (*addNoteEvent)(SequenceTrack* track, uint32_t step, uint32_t len, MIDINote note, float velocity);
	void (*removeNoteEvent)(SequenceTrack* track, uint32_t step, MIDINote note);
	void (*clearNotes)(SequenceTrack* track);
	
	int (*getControlSignalCount)(SequenceTrack* track);
	ControlSignal* (*getControlSignal)(SequenceTrack* track, int idx);
	void (*clearControlEvents)(SequenceTrack* track);
	
	int (*getPolyphony)(SequenceTrack* track);
	int (*activeVoiceCount)(SequenceTrack* track);
	
	void (*setMuted)(SequenceTrack* track, int mute);
	
	// 1.1
	uint32_t (*getLength)(SequenceTrack* track); // in steps, includes full last note
	int (*getIndexForStep)(SequenceTrack* track, uint32_t step);
	int (*getNoteAtIndex)(SequenceTrack* track, int index, uint32_t* outStep, uint32_t* outLen, MIDINote* outNote, float* outVelocity);
	
	// 1.10
	ControlSignal* (*getSignalForController)(SequenceTrack* track, int controller, int create);
};

// and a SoundSequence is a collection of tracks, along with control info like tempo and loops

typedef struct SoundSequence SoundSequence;
typedef void (*SequenceFinishedCallback)(SoundSequence* seq, void* userdata);

struct playdate_sound_sequence
{
	SoundSequence* (*newSequence)(void);
	void (*freeSequence)(SoundSequence* sequence);
	
	int (*loadMIDIFile)(SoundSequence* seq, const char* path);
	uint32_t (*getTime)(SoundSequence* seq);
	void (*setTime)(SoundSequence* seq, uint32_t time);
	void (*setLoops)(SoundSequence* seq, int loopstart, int loopend, int loops);
	int (*getTempo_deprecated)(SoundSequence* seq);
	void (*setTempo)(SoundSequence* seq, float stepsPerSecond);
	int (*getTrackCount)(SoundSequence* seq);
	SequenceTrack* (*addTrack)(SoundSequence* seq);
	SequenceTrack* (*getTrackAtIndex)(SoundSequence* seq, unsigned int track);
	void (*setTrackAtIndex)(SoundSequence* seq, SequenceTrack* track, unsigned int idx);
	void (*allNotesOff)(SoundSequence* seq);
	
	// 1.1
	int (*isPlaying)(SoundSequence* seq);
	uint32_t (*getLength)(SoundSequence* seq); // in steps, includes full last note
	void (*play)(SoundSequence* seq, SequenceFinishedCallback finishCallback, void* userdata);
	void (*stop)(SoundSequence* seq);
	int (*getCurrentStep)(SoundSequence* seq, int* timeOffset);
	void (*setCurrentStep)(SoundSequence* seq, int step, int timeOffset, int playNotes);
	
	// 2.5
	float (*getTempo)(SoundSequence* seq);
};


// EFFECTS

// A SoundEffect processes the output of a channel's SoundSources

typedef struct TwoPoleFilter TwoPoleFilter;

#if TARGET_EXTENSION
typedef enum
{
	kFilterTypeLowPass,
	kFilterTypeHighPass,
	kFilterTypeBandPass,
	kFilterTypeNotch,
	kFilterTypePEQ,
	kFilterTypeLowShelf,
	kFilterTypeHighShelf
} TwoPoleFilterType;
#endif // TARGET_EXTENSION

struct playdate_sound_effect_twopolefilter
{
	TwoPoleFilter* (*newFilter)(void);
	void (*freeFilter)(TwoPoleFilter* filter);
	void (*setType)(TwoPoleFilter* filter, TwoPoleFilterType type);
	void (*setFrequency)(TwoPoleFilter* filter, float frequency);
	void (*setFrequencyModulator)(TwoPoleFilter* filter, PDSynthSignalValue* signal);
	PDSynthSignalValue* (*getFrequencyModulator)(TwoPoleFilter* filter);
	void (*setGain)(TwoPoleFilter* filter, float gain);
	void (*setResonance)(TwoPoleFilter* filter, float resonance);
	void (*setResonanceModulator)(TwoPoleFilter* filter, PDSynthSignalValue* signal);
	PDSynthSignalValue* (*getResonanceModulator)(TwoPoleFilter* filter);
};

typedef struct OnePoleFilter OnePoleFilter;

struct playdate_sound_effect_onepolefilter
{
	OnePoleFilter* (*newFilter)(void);
	void (*freeFilter)(OnePoleFilter* filter);
	void (*setParameter)(OnePoleFilter* filter, float parameter);
	void (*setParameterModulator)(OnePoleFilter* filter, PDSynthSignalValue* signal);
	PDSynthSignalValue* (*getParameterModulator)(OnePoleFilter* filter);
};

typedef struct BitCrusher BitCrusher;

struct playdate_sound_effect_bitcrusher
{
	BitCrusher* (*newBitCrusher)(void);
	void (*freeBitCrusher)(BitCrusher* filter);
	void (*setAmount)(BitCrusher* filter, float amount);
	void (*setAmountModulator)(BitCrusher* filter, PDSynthSignalValue* signal);
	PDSynthSignalValue* (*getAmountModulator)(BitCrusher* filter);
	void (*setUndersampling)(BitCrusher* filter, float undersampling);
	void (*setUndersampleModulator)(BitCrusher* filter, PDSynthSignalValue* signal);
	PDSynthSignalValue* (*getUndersampleModulator)(BitCrusher* filter);
};

typedef struct RingModulator RingModulator;

struct playdate_sound_effect_ringmodulator
{
	RingModulator* (*newRingmod)(void);
	void (*freeRingmod)(RingModulator* filter);
	void (*setFrequency)(RingModulator* filter, float frequency);
	void (*setFrequencyModulator)(RingModulator* filter, PDSynthSignalValue* signal);
	PDSynthSignalValue* (*getFrequencyModulator)(RingModulator* filter);
};

typedef struct DelayLine DelayLine;
typedef struct DelayLineTap DelayLineTap;

struct playdate_sound_effect_delayline
{
	DelayLine* (*newDelayLine)(int length, int stereo);
	void (*freeDelayLine)(DelayLine* filter);
	void (*setLength)(DelayLine* d, int frames);
	void (*setFeedback)(DelayLine* d, float fb);
	DelayLineTap* (*addTap)(DelayLine* d, int delay);
	
	// note that DelayLineTap is a SoundSource, not a SoundEffect
	void (*freeTap)(DelayLineTap* tap);
	void (*setTapDelay)(DelayLineTap* t, int frames);
	void (*setTapDelayModulator)(DelayLineTap* t, PDSynthSignalValue* mod);
	PDSynthSignalValue* (*getTapDelayModulator)(DelayLineTap* t);
	void (*setTapChannelsFlipped)(DelayLineTap* t, int flip);
};

typedef struct Overdrive Overdrive;

struct playdate_sound_effect_overdrive
{
	Overdrive* (*newOverdrive)(void);
	void (*freeOverdrive)(Overdrive* filter);
	void (*setGain)(Overdrive* o, float gain);
	void (*setLimit)(Overdrive* o, float limit);
	void (*setLimitModulator)(Overdrive* o, PDSynthSignalValue* mod);
	PDSynthSignalValue* (*getLimitModulator)(Overdrive* o);
	void (*setOffset)(Overdrive* o, float offset);
	void (*setOffsetModulator)(Overdrive* o, PDSynthSignalValue* mod);
	PDSynthSignalValue* (*getOffsetModulator)(Overdrive* o);
};


typedef struct SoundEffect SoundEffect;
typedef int effectProc(SoundEffect* e, int32_t* left, int32_t* right, int nsamples, int bufactive); // samples are in signed q8.24 format

struct playdate_sound_effect
{
	SoundEffect* (*newEffect)(effectProc* proc, void* userdata);
	void (*freeEffect)(SoundEffect* effect);
	
	void (*setMix)(SoundEffect* effect, float level);
	void (*setMixModulator)(SoundEffect* effect, PDSynthSignalValue* signal);
	PDSynthSignalValue* (*getMixModulator)(SoundEffect* effect);

	void (*setUserdata)(SoundEffect* effect, void* userdata);
	void* (*getUserdata)(SoundEffect* effect);

	const struct playdate_sound_effect_twopolefilter* twopolefilter;
	const struct playdate_sound_effect_onepolefilter* onepolefilter;
	const struct playdate_sound_effect_bitcrusher* bitcrusher;
	const struct playdate_sound_effect_ringmodulator* ringmodulator;
	const struct playdate_sound_effect_delayline* delayline;
	const struct playdate_sound_effect_overdrive* overdrive;
};


// A SoundChannel contains SoundSources and SoundEffects

typedef struct SoundChannel SoundChannel;

typedef int AudioSourceFunction(void* context, int16_t* left, int16_t* right, int len); // len is # of samples in each buffer, function should return 1 if it produced output

struct playdate_sound_channel
{
	SoundChannel* (*newChannel)(void);
	void (*freeChannel)(SoundChannel* channel);
	int (*addSource)(SoundChannel* channel, SoundSource* source);
	int (*removeSource)(SoundChannel* channel, SoundSource* source);
	SoundSource* (*addCallbackSource)(SoundChannel* channel, AudioSourceFunction* callback, void* context, int stereo);
	void (*addEffect)(SoundChannel* channel, SoundEffect* effect);
	void (*removeEffect)(SoundChannel* channel, SoundEffect* effect);
	void (*setVolume)(SoundChannel* channel, float volume);
	float (*getVolume)(SoundChannel* channel);
	void (*setVolumeModulator)(SoundChannel* channel, PDSynthSignalValue* mod);
	PDSynthSignalValue* (*getVolumeModulator)(SoundChannel* channel);
	void (*setPan)(SoundChannel* channel, float pan);
	void (*setPanModulator)(SoundChannel* channel, PDSynthSignalValue* mod);
	PDSynthSignalValue* (*getPanModulator)(SoundChannel* channel);
	PDSynthSignalValue* (*getDryLevelSignal)(SoundChannel* channel);
	PDSynthSignalValue* (*getWetLevelSignal)(SoundChannel* channel);
};

typedef int RecordCallback(void *context, int16_t* buffer, int length); // data is mono

enum MicSource {
	kMicInputAutodetect = 0,
	kMicInputInternal = 1,
	kMicInputHeadset = 2
};

struct playdate_sound
{
	const struct playdate_sound_channel* channel;
	const struct playdate_sound_fileplayer* fileplayer;
	const struct playdate_sound_sample* sample;
	const struct playdate_sound_sampleplayer* sampleplayer;
	const struct playdate_sound_synth* synth;
	const struct playdate_sound_sequence* sequence;
	const struct playdate_sound_effect* effect;
	const struct playdate_sound_lfo* lfo;
	const struct playdate_sound_envelope* envelope;
	const struct playdate_sound_source* source;
	const struct playdate_control_signal* controlsignal;
	const struct playdate_sound_track* track;
	const struct playdate_sound_instrument* instrument;

	uint32_t (*getCurrentTime)(void);
	SoundSource* (*addSource)(AudioSourceFunction* callback, void* context, int stereo);

	SoundChannel* (*getDefaultChannel)(void);
	
	int (*addChannel)(SoundChannel* channel);
	int (*removeChannel)(SoundChannel* channel);
	
	int (*setMicCallback)(RecordCallback* callback, void* context, enum MicSource source);
	void (*getHeadphoneState)(int* headphone, int* headsetmic, void (*changeCallback)(int headphone, int mic));
	void (*setOutputsActive)(int headphone, int speaker);
	
	// 1.5
	int (*removeSource)(SoundSource* source);
	
	// 1.12
	const struct playdate_sound_signal* signal;
	
	// 2.2
	const char* (*getError)(void);
};

// -- pd_api_sprite.h ----------------------------------------------------------

#if TARGET_EXTENSION

typedef enum {kCollisionTypeSlide, kCollisionTypeFreeze, kCollisionTypeOverlap, kCollisionTypeBounce} SpriteCollisionResponseType;

typedef struct
{
	float x;
	float y;
	float width;
	float height;

} PDRect;

static inline PDRect PDRectMake(float x, float y, float width, float height)
{
	PDRect r = { .x = x, .y = y, .width = width, .height = height };
	return r;
}

#endif // TARGET_EXTENSION

typedef struct {
	float x;
	float y;
} CollisionPoint;

typedef struct {
	int x;
	int y;
} CollisionVector;

typedef struct SpriteCollisionInfo SpriteCollisionInfo;
struct SpriteCollisionInfo
{
	LCDSprite *sprite;		// The sprite being moved
	LCDSprite *other;		// The sprite colliding with the sprite being moved
	SpriteCollisionResponseType responseType;	// The result of collisionResponse
	uint8_t overlaps;		// True if the sprite was overlapping other when the collision started. False if it didn’t overlap but tunneled through other.
	float ti;				// A number between 0 and 1 indicating how far along the movement to the goal the collision occurred
	CollisionPoint move;	// The difference between the original coordinates and the actual ones when the collision happened
	CollisionVector normal;	// The collision normal; usually -1, 0, or 1 in x and y. Use this value to determine things like if your character is touching the ground.
	CollisionPoint touch;	// The coordinates where the sprite started touching other
	PDRect spriteRect;		// The rectangle the sprite occupied when the touch happened
	PDRect otherRect; 		// The rectangle the sprite being collided with occupied when the touch happened
};

typedef struct SpriteQueryInfo SpriteQueryInfo;
struct SpriteQueryInfo
{
	LCDSprite *sprite;			// The sprite being intersected by the segment
								// ti1 and ti2 are numbers between 0 and 1 which indicate how far from the starting point of the line segment the collision happened
	float ti1;					// entry point
	float ti2;					// exit point
	CollisionPoint entryPoint;	// The coordinates of the first intersection between sprite and the line segment
	CollisionPoint exitPoint;	// The coordinates of the second intersection between sprite and the line segment
};

typedef struct LCDSprite LCDSprite;
typedef struct CWCollisionInfo CWCollisionInfo;
typedef struct CWItemInfo CWItemInfo;
typedef void LCDSpriteDrawFunction(LCDSprite* sprite, PDRect bounds, PDRect drawrect);
typedef void LCDSpriteUpdateFunction(LCDSprite* sprite);
typedef SpriteCollisionResponseType LCDSpriteCollisionFilterProc(LCDSprite* sprite, LCDSprite* other);

struct playdate_sprite
{
	void (*setAlwaysRedraw)(int flag);
	void (*addDirtyRect)(LCDRect dirtyRect);
	void (*drawSprites)(void);
	void (*updateAndDrawSprites)(void);

	LCDSprite* (*newSprite)(void);
	void (*freeSprite)(LCDSprite *sprite);
	LCDSprite* (*copy)(LCDSprite *sprite);

	void (*addSprite)(LCDSprite *sprite);
	void (*removeSprite)(LCDSprite *sprite);
	void (*removeSprites)(LCDSprite **sprites, int count);
	void (*removeAllSprites)(void);
	int (*getSpriteCount)(void);

	void (*setBounds)(LCDSprite *sprite, PDRect bounds);
	PDRect (*getBounds)(LCDSprite *sprite);
	void (*moveTo)(LCDSprite *sprite, float x, float y);
	void (*moveBy)(LCDSprite *sprite, float dx, float dy);

	void (*setImage)(LCDSprite *sprite, LCDBitmap *image, LCDBitmapFlip flip);
	LCDBitmap* (*getImage)(LCDSprite *sprite);
	void (*setSize)(LCDSprite *s, float width, float height);
	void (*setZIndex)(LCDSprite *sprite, int16_t zIndex);
	int16_t (*getZIndex)(LCDSprite *sprite);

	void (*setDrawMode)(LCDSprite *sprite, LCDBitmapDrawMode mode);
	void (*setImageFlip)(LCDSprite *sprite, LCDBitmapFlip flip);
	LCDBitmapFlip (*getImageFlip)(LCDSprite *sprite);
	void (*setStencil)(LCDSprite *sprite, LCDBitmap* stencil); // deprecated in favor of setStencilImage()

	void (*setClipRect)(LCDSprite *sprite, LCDRect clipRect);
	void (*clearClipRect)(LCDSprite *sprite);
	void (*setClipRectsInRange)(LCDRect clipRect, int startZ, int endZ);
	void (*clearClipRectsInRange)(int startZ, int endZ);

	void (*setUpdatesEnabled)(LCDSprite *sprite, int flag);
	int (*updatesEnabled)(LCDSprite *sprite);
	void (*setCollisionsEnabled)(LCDSprite *sprite, int flag);
	int (*collisionsEnabled)(LCDSprite *sprite);
	void (*setVisible)(LCDSprite *sprite, int flag);
	int (*isVisible)(LCDSprite *sprite);
	void (*setOpaque)(LCDSprite *sprite, int flag);
	void (*markDirty)(LCDSprite *sprite);

	void (*setTag)(LCDSprite *sprite, uint8_t tag);
	uint8_t (*getTag)(LCDSprite *sprite);

	void (*setIgnoresDrawOffset)(LCDSprite *sprite, int flag);

	void (*setUpdateFunction)(LCDSprite *sprite, LCDSpriteUpdateFunction *func);
	void (*setDrawFunction)(LCDSprite *sprite, LCDSpriteDrawFunction *func);

	void (*getPosition)(LCDSprite *sprite, float *x, float *y);

	// Collisions
	void (*resetCollisionWorld)(void);

	void (*setCollideRect)(LCDSprite *sprite, PDRect collideRect);
	PDRect (*getCollideRect)(LCDSprite *sprite);
	void (*clearCollideRect)(LCDSprite *sprite);

	// caller is responsible for freeing the returned array for all collision methods
	void (*setCollisionResponseFunction)(LCDSprite *sprite, LCDSpriteCollisionFilterProc *func);
	SpriteCollisionInfo* (*checkCollisions)(LCDSprite *sprite, float goalX, float goalY, float *actualX, float *actualY, int *len);			// access results using SpriteCollisionInfo *info = &results[i];
	SpriteCollisionInfo* (*moveWithCollisions)(LCDSprite *sprite, float goalX, float goalY, float *actualX, float *actualY, int *len);
	LCDSprite** (*querySpritesAtPoint)(float x, float y, int *len);
	LCDSprite** (*querySpritesInRect)(float x, float y, float width, float height, int *len);
	LCDSprite** (*querySpritesAlongLine)(float x1, float y1, float x2, float y2, int *len);
	SpriteQueryInfo* (*querySpriteInfoAlongLine)(float x1, float y1, float x2, float y2, int *len);											// access results using SpriteQueryInfo *info = &results[i];
	LCDSprite** (*overlappingSprites)(LCDSprite *sprite, int *len);
	LCDSprite** (*allOverlappingSprites)(int *len);

	// added in 1.7
	void (*setStencilPattern)(LCDSprite* sprite, uint8_t pattern[8]);
	void (*clearStencil)(LCDSprite* sprite);

	void (*setUserdata)(LCDSprite* sprite, void* userdata);
	void* (*getUserdata)(LCDSprite* sprite);

	// added in 1.10
	void (*setStencilImage)(LCDSprite *sprite, LCDBitmap* stencil, int tile);
	
	// 2.1
	void (*setCenter)(LCDSprite* s, float x, float y);
	void (*getCenter)(LCDSprite* s, float* x, float* y);
};

// -- pd_api_sys.h -------------------------------------------------------------

#include <stdarg.h>

#if TARGET_EXTENSION

typedef enum
{
	kButtonLeft		= (1<<0),
	kButtonRight	= (1<<1),
	kButtonUp		= (1<<2),
	kButtonDown		= (1<<3),
	kButtonB		= (1<<4),
	kButtonA		= (1<<5)
} PDButtons;

typedef enum
{
	kPDLanguageEnglish,
	kPDLanguageJapanese,
	kPDLanguageUnknown,
} PDLanguage;

#endif // TARGET_EXTENSION

struct PDDateTime
{
	uint16_t year;
	uint8_t month; // 1-12
	uint8_t day; // 1-31
	uint8_t weekday; // 1=monday-7=sunday
	uint8_t hour; // 0-23
	uint8_t minute;
	uint8_t second;
};

typedef struct PDMenuItem PDMenuItem;

typedef enum
{
	kNone = 0,
	kAccelerometer	= (1<<0),
	// ...
	kAllPeripherals = 0xffff
} PDPeripherals;

typedef int PDCallbackFunction(void* userdata); // return 0 when done
typedef void PDMenuItemCallbackFunction(void* userdata);
typedef int PDButtonCallbackFunction(PDButtons button, int down, uint32_t when, void* userdata);

struct playdate_sys
{
	void* (*realloc)(void* ptr, size_t size); // ptr = NULL -> malloc, size = 0 -> free
	int (*formatString)(char **ret, const char *fmt, ...);
	void (*logToConsole)(const char* fmt, ...);
	void (*error)(const char* fmt, ...);
	PDLanguage (*getLanguage)(void);
	unsigned int (*getCurrentTimeMilliseconds)(void);
	unsigned int (*getSecondsSinceEpoch)(unsigned int *milliseconds);
	void (*drawFPS)(int x, int y);

	void (*setUpdateCallback)(PDCallbackFunction* update, void* userdata);
	void (*getButtonState)(PDButtons* current, PDButtons* pushed, PDButtons* released);
	void (*setPeripheralsEnabled)(PDPeripherals mask);
	void (*getAccelerometer)(float* outx, float* outy, float* outz);

	float (*getCrankChange)(void);
	float (*getCrankAngle)(void);
	int (*isCrankDocked)(void);
	int (*setCrankSoundsDisabled)(int flag); // returns previous setting

	int (*getFlipped)(void);
	void (*setAutoLockDisabled)(int disable);

	void (*setMenuImage)(LCDBitmap* bitmap, int xOffset);
	PDMenuItem* (*addMenuItem)(const char *title, PDMenuItemCallbackFunction* callback, void* userdata);
	PDMenuItem* (*addCheckmarkMenuItem)(const char *title, int value, PDMenuItemCallbackFunction* callback, void* userdata);
	PDMenuItem* (*addOptionsMenuItem)(const char *title, const char** optionTitles, int optionsCount, PDMenuItemCallbackFunction* f, void* userdata);
	void (*removeAllMenuItems)(void);
	void (*removeMenuItem)(PDMenuItem *menuItem);
	int (*getMenuItemValue)(PDMenuItem *menuItem);
	void (*setMenuItemValue)(PDMenuItem *menuItem, int value);
	const char* (*getMenuItemTitle)(PDMenuItem *menuItem);
	void (*setMenuItemTitle)(PDMenuItem *menuItem, const char *title);
	void* (*getMenuItemUserdata)(PDMenuItem *menuItem);
	void (*setMenuItemUserdata)(PDMenuItem *menuItem, void *ud);
	
	int (*getReduceFlashing)(void);
	
	// 1.1
	float (*getElapsedTime)(void);
	void (*resetElapsedTime)(void);

	// 1.4
	float (*getBatteryPercentage)(void);
	float (*getBatteryVoltage)(void);
	
	// 1.13
	int32_t (*getTimezoneOffset)(void);
	int (*shouldDisplay24HourTime)(void);
	void (*convertEpochToDateTime)(uint32_t epoch, struct PDDateTime* datetime);
	uint32_t (*convertDateTimeToEpoch)(struct PDDateTime* datetime);

	// 2.0
	void (*clearICache)(void);
	
	// 2.4
	void (*setButtonCallback)(PDButtonCallbackFunction* cb, void* buttonud, int queuesize);
	void (*setSerialMessageCallback)(void (*callback)(const char* data));
	int (*vaFormatString)(char **outstr, const char *fmt, va_list args);
	int (*parseString)(const char *str, const char *format, ...);
};

// -- pd_api.h -----------------------------------------------------------------

#include <stdint.h>
#include <stdlib.h>

typedef struct PlaydateAPI PlaydateAPI;

struct PlaydateAPI
{
	const struct playdate_sys* system;
	const struct playdate_file* file;
	const struct playdate_graphics* graphics;
	const struct playdate_sprite* sprite;
	const struct playdate_display* display;
	const struct playdate_sound* sound;
	const struct playdate_lua* lua;
	const struct playdate_json* json;
	const struct playdate_scoreboards* scoreboards;
};

#if TARGET_EXTENSION
typedef enum
{
	kEventInit,
	kEventInitLua,
	kEventLock,
	kEventUnlock,
	kEventPause,
	kEventResume,
	kEventTerminate,
	kEventKeyPressed, // arg is keycode
	kEventKeyReleased,
	kEventLowPower
} PDSystemEvent;
#endif

// main entry point defined in game code, called for the above events

#ifdef _WINDLL
__declspec(dllexport)
#endif // _WINDLL
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg);

// -- setup.c ------------------------------------------------------------------

#ifdef PLAYDATE_SETUP

static void* (*pdrealloc)(void* ptr, size_t size);

PlaydateAPI* playdate;

typedef int (*playdate_event_handler)(void);

extern int playdate_init(void);  
extern int playdate_update(void);

playdate_event_handler lock;
playdate_event_handler unlock;
playdate_event_handler pause;
playdate_event_handler resume;
playdate_event_handler terminate;
playdate_event_handler low_power;
playdate_event_handler key_pressed;
playdate_event_handler key_released;

static inline __attribute__((always_inline)) int shim(void* userdata) {
	return playdate_update();
}

int eventHandlerShim(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg) {
	if (event == kEventInit) {
		playdate = pd;
		pdrealloc = playdate->system->realloc;
		playdate->system->setUpdateCallback(shim, NULL);
		return playdate_init();
	}
	if (event == kEventUnlock) if (unlock) return unlock();
	if (event == kEventLock) if (lock) return lock();
	if (event == kEventPause) if (pause) return pause();
	if (event == kEventResume) if (resume) return resume();
	if (event == kEventTerminate) if (terminate) return terminate();
	if (event == kEventLowPower) if (low_power) return low_power();
	if (event == kEventKeyPressed) if (key_pressed) return key_pressed();
	if (event == kEventKeyReleased) if (key_released) return key_released();
	return 0;
}

#if TARGET_PLAYDATE
void* _malloc_r(struct _reent* _REENT, size_t nbytes) { return pdrealloc(NULL, nbytes); }
void* _realloc_r(struct _reent* _REENT, void* ptr, size_t nbytes) { return pdrealloc(ptr, nbytes); }
void  _free_r(struct _reent* _REENT, void* ptr) { if ( ptr != NULL ) pdrealloc(ptr, 0); }
#else
void* malloc(size_t nbytes) { return pdrealloc(NULL, nbytes); }
void* realloc(void* ptr, size_t nbytes) { return pdrealloc(ptr, nbytes); }
void  free(void* ptr ) { if ( ptr != NULL ) pdrealloc(ptr, 0); }
#endif // TARGET_PLAYDATE

#endif // PLAYDATE_SETUP
#endif // PLAYDATE_H
