#ifndef _HERO_WINDOW_H_
#define _HERO_WINDOW_H_

// ===========================================
//
//
// General
//
//
// ===========================================

typedef struct HeroWindow HeroWindow;

enum {
	HERO_ALLOC_TAG_WINDOW_WINDOWS_POOL = HERO_ERROR_WINDOW_START,
	HERO_ALLOC_TAG_WINDOW_EVENT_QUEUE,
	HERO_ALLOC_TAG_WINDOW_DISPLAY_OUTPUT_VIDEO_MODES,
	HERO_ALLOC_TAG_WINDOW_DISPLAY_OUTPUTS,
	HERO_ALLOC_TAG_WINDOW_X11_WINDOW_ID_MAP,
};

// ===========================================
//
//
// Input
//
//
// ===========================================

typedef U8 HeroMouseButtons;
enum {
	HERO_MOUSE_BUTTONS_LEFT = 0x1,
	HERO_MOUSE_BUTTONS_MIDDLE = 0x2,
	HERO_MOUSE_BUTTONS_RIGHT = 0x4,
	HERO_MOUSE_BUTTONS_FORWARD = 0x8,
	HERO_MOUSE_BUTTONS_BACK = 0x10,
};

typedef U16 HeroKeyMod;
enum {
	HERO_KEY_MOD_LEFT_SHIFT = 0x0001,
	HERO_KEY_MOD_RIGHT_SHIFT = 0x0002,
	HERO_KEY_MOD_SHIFT = 0x0003,
	HERO_KEY_MOD_LEFT_CTRL = 0x0040,
	HERO_KEY_MOD_RIGHT_CTRL = 0x0080,
	HERO_KEY_MOD_CTRL = 0x00C0,
	HERO_KEY_MOD_LEFT_ALT = 0x0100,
	HERO_KEY_MOD_RIGHT_ALT = 0x0200,
	HERO_KEY_MOD_ALT = 0x300,
	HERO_KEY_MOD_LEFT_META = 0x0400,
	HERO_KEY_MOD_RIGHT_META = 0x0800,
	HERO_KEY_MOD_META = 0xC00,
	HERO_KEY_MOD_CAPS_LOCK = 0x1000,
	HERO_KEY_MOD_NUM_LOCK = 0x2000,
};

//
// ScanCode:
// platform & keyboard layout independant enumeration to represents a physical key.
// this does not use the keyboard mapping, is used to reason a physical location on the keyboard.
// use the HeroKeyCode enumeration values.
typedef U16 HeroScanCode;

//
// KeyCode:
// platform independant enumeration to represents the virtual key.
// this respects the keyboard mapping.
// the values in this enumeration are based on the USB HID usage tables standard (section 0x07):
// https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf
//
typedef U16 HeroKeyCode;
enum {
	HERO_KEY_CODE_NULL = 0,

	HERO_KEY_CODE_A = 4,
	HERO_KEY_CODE_B = 5,
	HERO_KEY_CODE_C = 6,
	HERO_KEY_CODE_D = 7,
	HERO_KEY_CODE_E = 8,
	HERO_KEY_CODE_F = 9,
	HERO_KEY_CODE_G = 10,
	HERO_KEY_CODE_H = 11,
	HERO_KEY_CODE_I = 12,
	HERO_KEY_CODE_J = 13,
	HERO_KEY_CODE_K = 14,
	HERO_KEY_CODE_L = 15,
	HERO_KEY_CODE_M = 16,
	HERO_KEY_CODE_N = 17,
	HERO_KEY_CODE_O = 18,
	HERO_KEY_CODE_P = 19,
	HERO_KEY_CODE_Q = 20,
	HERO_KEY_CODE_R = 21,
	HERO_KEY_CODE_S = 22,
	HERO_KEY_CODE_T = 23,
	HERO_KEY_CODE_U = 24,
	HERO_KEY_CODE_V = 25,
	HERO_KEY_CODE_W = 26,
	HERO_KEY_CODE_X = 27,
	HERO_KEY_CODE_Y = 28,
	HERO_KEY_CODE_Z = 29,

	HERO_KEY_CODE_1 = 30,
	HERO_KEY_CODE_2 = 31,
	HERO_KEY_CODE_3 = 32,
	HERO_KEY_CODE_4 = 33,
	HERO_KEY_CODE_5 = 34,
	HERO_KEY_CODE_6 = 35,
	HERO_KEY_CODE_7 = 36,
	HERO_KEY_CODE_8 = 37,
	HERO_KEY_CODE_9 = 38,
	HERO_KEY_CODE_0 = 39,

	HERO_KEY_CODE_ENTER = 40,
	HERO_KEY_CODE_ESCAPE = 41,
	HERO_KEY_CODE_BACKSPACE = 42,
	HERO_KEY_CODE_TAB = 43,
	HERO_KEY_CODE_SPACE = 44,

	HERO_KEY_CODE_MINUS = 45,
	HERO_KEY_CODE_EQUALS = 46,
	HERO_KEY_CODE_LEFT_BRACKET = 47,
	HERO_KEY_CODE_RIGHT_BRACKET = 48,
	HERO_KEY_CODE_BACKSLASH = 49,
	HERO_KEY_CODE_NON_US_HASH = 50,
	HERO_KEY_CODE_SEMICOLON = 51,
	HERO_KEY_CODE_APOSTROPHE = 52,
	HERO_KEY_CODE_GHERO = 53,
	HERO_KEY_CODE_COMMA = 54,
	HERO_KEY_CODE_FULL_STOP = 55,
	HERO_KEY_CODE_FORWARDSLASH = 56,
	HERO_KEY_CODE_CAPS_LOCK = 57,

	HERO_KEY_CODE_F1 = 58,
	HERO_KEY_CODE_F2 = 59,
	HERO_KEY_CODE_F3 = 60,
	HERO_KEY_CODE_F4 = 61,
	HERO_KEY_CODE_F5 = 62,
	HERO_KEY_CODE_F6 = 63,
	HERO_KEY_CODE_F7 = 64,
	HERO_KEY_CODE_F8 = 65,
	HERO_KEY_CODE_F9 = 66,
	HERO_KEY_CODE_F10 = 67,
	HERO_KEY_CODE_F11 = 68,
	HERO_KEY_CODE_F12 = 69,

	HERO_KEY_CODE_PRINT_SCREEN = 70,
	HERO_KEY_CODE_SCROLL_LOCK = 71,
	HERO_KEY_CODE_PAUSE = 72,
	HERO_KEY_CODE_INSERT = 73,
	HERO_KEY_CODE_HOME = 74,
	HERO_KEY_CODE_PAGE_UP = 75,
	HERO_KEY_CODE_DELETE = 76,
	HERO_KEY_CODE_END = 77,
	HERO_KEY_CODE_PAGE_DOWN = 78,
	HERO_KEY_CODE_RIGHT = 79,
	HERO_KEY_CODE_LEFT = 80,
	HERO_KEY_CODE_DOWN = 81,
	HERO_KEY_CODE_UP = 82,

	HERO_KEY_CODE_NUM_LOCK_OR_CLEAR = 83,
	HERO_KEY_CODE_KP_DIVIDE = 84,
	HERO_KEY_CODE_KP_MULTIPLY = 85,
	HERO_KEY_CODE_KP_MINUS = 86,
	HERO_KEY_CODE_KP_PLUS = 87,
	HERO_KEY_CODE_KP_ENTER = 88,
	HERO_KEY_CODE_KP_1 = 89,
	HERO_KEY_CODE_KP_2 = 90,
	HERO_KEY_CODE_KP_3 = 91,
	HERO_KEY_CODE_KP_4 = 92,
	HERO_KEY_CODE_KP_5 = 93,
	HERO_KEY_CODE_KP_6 = 94,
	HERO_KEY_CODE_KP_7 = 95,
	HERO_KEY_CODE_KP_8 = 96,
	HERO_KEY_CODE_KP_9 = 97,
	HERO_KEY_CODE_KP_0 = 98,
	HERO_KEY_CODE_KP_FULL_STOP = 99,

	HERO_KEY_CODE_NON_US_BACKSLASH = 100,
	HERO_KEY_CODE_APPLICATION = 101,
	HERO_KEY_CODE_POWER = 102,
	HERO_KEY_CODE_KP_EQUALS = 103,
	HERO_KEY_CODE_F13 = 104,
	HERO_KEY_CODE_F14 = 105,
	HERO_KEY_CODE_F15 = 106,
	HERO_KEY_CODE_F16 = 107,
	HERO_KEY_CODE_F17 = 108,
	HERO_KEY_CODE_F18 = 109,
	HERO_KEY_CODE_F19 = 110,
	HERO_KEY_CODE_F20 = 111,
	HERO_KEY_CODE_F21 = 112,
	HERO_KEY_CODE_F22 = 113,
	HERO_KEY_CODE_F23 = 114,
	HERO_KEY_CODE_F24 = 115,
	HERO_KEY_CODE_EXECUTE = 116,
	HERO_KEY_CODE_HELP = 117,
	HERO_KEY_CODE_MENU = 118,
	HERO_KEY_CODE_SELECT = 119,
	HERO_KEY_CODE_STOP = 120,
	HERO_KEY_CODE_AGAIN = 121,
	HERO_KEY_CODE_UNDO = 122,
	HERO_KEY_CODE_CUT = 123,
	HERO_KEY_CODE_COPY = 124,
	HERO_KEY_CODE_PASTE = 125,
	HERO_KEY_CODE_FIND = 126,
	HERO_KEY_CODE_MUTE = 127,
	HERO_KEY_CODE_VOLUME_UP = 128,
	HERO_KEY_CODE_VOLUME_DOWN = 129,

	HERO_KEY_CODE_KP_COMMA = 133,
	HERO_KEY_CODE_KP_EQUALSAS400 = 134,

	HERO_KEY_CODE_INTERNATIONAL_1 = 135,
	HERO_KEY_CODE_INTERNATIONAL_2 = 136,
	HERO_KEY_CODE_INTERNATIONAL_3 = 137, // YEN
	HERO_KEY_CODE_INTERNATIONAL_4 = 138,
	HERO_KEY_CODE_INTERNATIONAL_5 = 139,
	HERO_KEY_CODE_INTERNATIONAL_6 = 140,
	HERO_KEY_CODE_INTERNATIONAL_7 = 141,
	HERO_KEY_CODE_INTERNATIONAL_8 = 142,
	HERO_KEY_CODE_INTERNATIONAL_9 = 143,
	HERO_KEY_CODE_LANG_1 = 144, // HANGUL/ENGLISH TOGGLE
	HERO_KEY_CODE_LANG_2 = 145, // HANJA CONVERSION
	HERO_KEY_CODE_LANG_3 = 146, // KATAKANA
	HERO_KEY_CODE_LANG_4 = 147, // HIRAGANA
	HERO_KEY_CODE_LANG_5 = 148, // ZENKAKU/HANKAKU
	HERO_KEY_CODE_LANG_6 = 149, // RESERVED
	HERO_KEY_CODE_LANG_7 = 150, // RESERVED
	HERO_KEY_CODE_LANG_8 = 151, // RESERVED
	HERO_KEY_CODE_LANG_9 = 152, // RESERVED

	HERO_KEY_CODE_ALT_ERASE = 153,
	HERO_KEY_CODE_SYS_REQ = 154,
	HERO_KEY_CODE_CANCEL = 155,
	HERO_KEY_CODE_CLEAR = 156,
	HERO_KEY_CODE_PRIOR = 157,
	HERO_KEY_CODE_ENTER_2 = 158,
	HERO_KEY_CODE_SEPARATOR = 159,
	HERO_KEY_CODE_OUT = 160,
	HERO_KEY_CODE_OPER = 161,
	HERO_KEY_CODE_CLEARAGAIN = 162,
	HERO_KEY_CODE_CRSEL = 163,
	HERO_KEY_CODE_EXSEL = 164,

	HERO_KEY_CODE_KP_00 = 176,
	HERO_KEY_CODE_KP_000 = 177,
	HERO_KEY_CODE_THOUSANDSSEPARATOR = 178,
	HERO_KEY_CODE_DECIMALSEPARATOR = 179,
	HERO_KEY_CODE_CURRENCYUNIT = 180,
	HERO_KEY_CODE_CURRENCYSUBUNIT = 181,
	HERO_KEY_CODE_KP_LEFT_PAREN = 182,
	HERO_KEY_CODE_KP_RIGHT_PAREN = 183,
	HERO_KEY_CODE_KP_LEFT_BRACE = 184,
	HERO_KEY_CODE_KP_RIGHT_BRACE = 185,
	HERO_KEY_CODE_KP_TAB = 186,
	HERO_KEY_CODE_KP_BACKSPACE = 187,
	HERO_KEY_CODE_KP_A = 188,
	HERO_KEY_CODE_KP_B = 189,
	HERO_KEY_CODE_KP_C = 190,
	HERO_KEY_CODE_KP_D = 191,
	HERO_KEY_CODE_KP_E = 192,
	HERO_KEY_CODE_KP_F = 193,
	HERO_KEY_CODE_KP_XOR = 194,
	HERO_KEY_CODE_KP_POWER = 195,
	HERO_KEY_CODE_KP_PERCENT = 196,
	HERO_KEY_CODE_KP_LESS = 197,
	HERO_KEY_CODE_KP_GREATER = 198,
	HERO_KEY_CODE_KP_AMPERSAND = 199,
	HERO_KEY_CODE_KP_DBLAMPERSAND = 200,
	HERO_KEY_CODE_KP_VERTICALBAR = 201,
	HERO_KEY_CODE_KP_DBLVERTICALBAR = 202,
	HERO_KEY_CODE_KP_COLON = 203,
	HERO_KEY_CODE_KP_HASH = 204,
	HERO_KEY_CODE_KP_SPACE = 205,
	HERO_KEY_CODE_KP_AT = 206,
	HERO_KEY_CODE_KP_EXCLAM = 207,
	HERO_KEY_CODE_KP_MEMSTORE = 208,
	HERO_KEY_CODE_KP_MEMRECALL = 209,
	HERO_KEY_CODE_KP_MEMCLEAR = 210,
	HERO_KEY_CODE_KP_MEMADD = 211,
	HERO_KEY_CODE_KP_MEMSUBTRACT = 212,
	HERO_KEY_CODE_KP_MEMMULTIPLY = 213,
	HERO_KEY_CODE_KP_MEMDIVIDE = 214,
	HERO_KEY_CODE_KP_PLUS_MINUS = 215,
	HERO_KEY_CODE_KP_CLEAR = 216,
	HERO_KEY_CODE_KP_CLEARENTRY = 217,
	HERO_KEY_CODE_KP_BINARY = 218,
	HERO_KEY_CODE_KP_OCTAL = 219,
	HERO_KEY_CODE_KP_DECIMAL = 220,
	HERO_KEY_CODE_KP_HEXADECIMAL = 221,

	HERO_KEY_CODE_LEFT_CTRL = 224,
	HERO_KEY_CODE_LEFT_SHIFT = 225,
	HERO_KEY_CODE_LEFT_ALT = 226, // ALT, OPTION
	HERO_KEY_CODE_LEFT_META = 227, // WINDOWS, COMMAND (APPLE), META
	HERO_KEY_CODE_RIGHT_CTRL = 228,
	HERO_KEY_CODE_RIGHT_SHIFT = 229,
	HERO_KEY_CODE_RIGHT_ALT = 230, // ALT GR, OPTION
	HERO_KEY_CODE_RIGHT_META = 231, // WINDOWS, COMMAND (APPLE), META

	//
	// Usage page 0x0C
	// These values are mapped from usage page 0x0C (USB consumer page).
	//
	HERO_KEY_CODE_AUDIO_NEXT = 258,
	HERO_KEY_CODE_AUDIO_PREV = 259,
	HERO_KEY_CODE_AUDIO_STOP = 260,
	HERO_KEY_CODE_AUDIO_PLAY = 261,
	HERO_KEY_CODE_AUDIO_MUTE = 262,
	HERO_KEY_CODE_MEDIASELECT = 263,
	HERO_KEY_CODE_WWW = 264,
	HERO_KEY_CODE_MAIL = 265,
	HERO_KEY_CODE_CALCULATOR = 266,
	HERO_KEY_CODE_COMPUTER = 267,
	HERO_KEY_CODE_AC_SEARCH = 268,
	HERO_KEY_CODE_AC_HOME = 269,
	HERO_KEY_CODE_AC_BACK = 270,
	HERO_KEY_CODE_AC_FORWARD = 271,
	HERO_KEY_CODE_AC_STOP = 272,
	HERO_KEY_CODE_AC_REFRESH = 273,
	HERO_KEY_CODE_AC_BOOKMARKS = 274,

	HERO_KEY_CODE_BRIGHTNESS_DOWN = 275,
	HERO_KEY_CODE_BRIGHTNESS_UP = 276,
	HERO_KEY_CODE_DISPLAY_SWITCH = 277,
	HERO_KEY_CODE_KBD_ILLUM_TOGGLE = 278,
	HERO_KEY_CODE_KBD_ILLUM_DOWN = 279,
	HERO_KEY_CODE_KBD_ILLUM_UP = 280,
	HERO_KEY_CODE_EJECT = 281,
	HERO_KEY_CODE_SLEEP = 282,

	HERO_KEY_CODE_APP_1 = 283,
	HERO_KEY_CODE_APP_2 = 284,

	HERO_KEY_CODE_AUDIO_REWIND = 285,
	HERO_KEY_CODE_AUDIO_FASTFORWARD = 286,

	HERO_KEY_CODE_COUNT = 512
};

extern const char* HeroKeyCode_strings[HERO_KEY_CODE_COUNT];

HERO_TYPEDEF_OBJECT_ID(HeroWindowId);
HERO_TYPEDEF_OBJECT_ID(HeroDisplayOutputId);

// ===========================================
//
//
// X11
//
//
// ===========================================
#if HERO_X11_ENABLE

//
// To avoid X11 header files polluting our namespace, we are doing to define our own versions with an 'X' prefix.
//
typedef Uptr  XWindow;
typedef void  XDisplay;
typedef Uptr  XAtom;
typedef Uptr  XRRMode;
typedef Uptr  XRRCrtc;
typedef Uptr  XVisualID;
typedef void* XXkbDescPtr;
typedef void* XXIM;
typedef void* XXIC;

#define HERO_HASH_TABLE_KEY_TYPE XWindow
#define HERO_HASH_TABLE_VALUE_TYPE HeroWindowId
#include "hash_table_gen.inl"

typedef struct HeroX11 HeroX11;
struct HeroX11 {
	XDisplay* connection;
	XXkbDescPtr xkb;
	XXIM xim;
	XVisualID visual_id;
	XAtom wm_delete_window;
	int xrandr_event_base;
	int xrandr_error_base;
	HeroHashTable(XWindow, HeroWindowId) window_id_map;
};

HeroResult hero_x11_init(void);
HeroResult hero_x11_window_open(HeroWindow* window, HeroWindowId window_id, const char* title);
HeroResult hero_x11_window_close(HeroWindow* window, HeroWindowId window_id);
HeroResult hero_x11_window_set_title(HeroWindow* window, const char* title);
HeroResult hero_x11_events_poll(void);

#endif // HERO_X11_ENABLE

// ===========================================
//
//
// Window System
//
//
// ===========================================

#define HERO_DISPLAY_OUTPUT_NAME_CAP 256
#define HERO_VIDEO_MODE_NAME_CAP 256

typedef struct HeroVideoMode HeroVideoMode;
struct HeroVideoMode {
	U32 width;
	U32 height;
	U16 refresh_rate;
	union {
		struct {
			XRRMode mode_id;
		} x11;
	} backend;
	U16 name_len;
	char name[HERO_VIDEO_MODE_NAME_CAP];
};

typedef U8 HeroDisplayOutputFlags;
enum {
	HERO_DISPLAY_OUTPUT_FLAGS_IS_CONNECTED = 0x1,
	HERO_DISPLAY_OUTPUT_FLAGS_IS_ENABLED = 0x2,
	HERO_DISPLAY_OUTPUT_FLAGS_IS_PRIMARY = 0x4,
};

typedef struct HeroDisplayOutput HeroDisplayOutput;
struct HeroDisplayOutput {
	HeroVideoMode* video_modes;
	HeroDisplayOutputId video_mode_id;
	U16 video_modes_count;
	U32 x;
	U32 y;
	U32 width;
	U32 height;
	HeroDisplayOutputFlags flags;
	union {
		struct {
			XRRCrtc crtc_id;
		} x11;
	} backend;
	U16 name_len;
	char name[HERO_DISPLAY_OUTPUT_NAME_CAP];
};

typedef U32 HeroWindowFlags;
enum {
	HERO_WINDOW_FLAGS_IS_UPDATING = 0x1,
	HERO_WINDOW_FLAGS_IS_RENDERING = 0x2,
};

struct HeroWindow {
	HeroObjectHeader header;
	HeroWindowFlags flags;
	S32 x;
	S32 y;
	U32 width;
	U32 height;
	union {
#if HERO_X11_ENABLE
		struct {
			XWindow xwindow;
			XXIC xic;
		} x11;
#endif
	} backend;
};

typedef struct HeroKeyCodeBitset HeroKeyCodeBitset;
struct HeroKeyCodeBitset {
	U64 bits[8];
};

static inline bool hero_key_code_bitset_is_set(HeroKeyCodeBitset* bitset, HeroKeyCode key_code) {
	U64 bit = (U64)1 << (key_code % 64);
	return (bitset->bits[key_code / 64] & bit) == bit;
}

static inline void hero_key_code_bitset_set(HeroKeyCodeBitset* bitset, HeroKeyCode key_code) {
	U64 bit = (U64)1 << (key_code % 64);
	bitset->bits[key_code / 64] |= bit;
}

static inline void hero_key_code_bitset_unset(HeroKeyCodeBitset* bitset, HeroKeyCode key_code) {
	bitset->bits[key_code / 64] &= ~((U64)1 << (key_code % 64));
}

typedef struct HeroKeyboard HeroKeyboard;
struct HeroKeyboard {
	//
	// tables to convert between scan code and key codes depending on the current keyboard mapping.
	HeroKeyCode scan_code_to_key_code[HERO_KEY_CODE_COUNT];
	HeroScanCode key_code_to_scan_code[HERO_KEY_CODE_COUNT];

	//
	// the state of all key modifiers
	HeroKeyMod key_mod_is_pressed;
	HeroKeyMod key_mod_has_been_pressed;
	HeroKeyMod key_mod_has_been_released;

	//
	// the state of all keys using a scan code to check
	HeroKeyCodeBitset scan_code_is_pressed_bitset;
	HeroKeyCodeBitset scan_code_has_been_pressed_bitset;
	HeroKeyCodeBitset scan_code_has_been_released_bitset;

	//
	// because multiple scancodes can translate to a single keycode.
	// we cant translate a keycode to a scancode and expect it to account for all scancodes.
	// so for keycodes, we need separate tables to check the states of the keys using a key code to check
	HeroKeyCodeBitset key_code_is_pressed_bitset;
	HeroKeyCodeBitset key_code_has_been_pressed_bitset;
	HeroKeyCodeBitset key_code_has_been_released_bitset;

	HeroWindowId focused_window_id;
};

//
// mouse data that is captured on a per frame basis
typedef struct HeroMouse HeroMouse;
struct HeroMouse {
	U32 x;
	U32 y;
	S32 rel_x;
	S32 rel_y;
	S32 rel_wheel_x;
	S32 rel_wheel_y;

	//
	// the state of all mouse buttons
	HeroMouseButtons buttons_is_pressed;
	HeroMouseButtons buttons_has_been_pressed;
	HeroMouseButtons buttons_has_been_released;

	HeroWindowId focused_window_id;
};

typedef HeroResult (*HeroIWindowSysWindowOpenFn)(HeroWindow* window, HeroWindowId window_id, const char* title);
typedef HeroResult (*HeroIWindowSysWindowCloseFn)(HeroWindow* window, HeroWindowId window_id);
typedef HeroResult (*HeroIWindowSysWindowSetTitleFn)(HeroWindow* window, const char* title);
typedef HeroResult (*HeroIWindowSysEventsPollFn)(void);


typedef struct HeroIWindowSysVTable HeroIWindowSysVTable;
struct HeroIWindowSysVTable {
	HeroIWindowSysWindowOpenFn window_open_fn;
	HeroIWindowSysWindowCloseFn window_close_fn;
	HeroIWindowSysWindowSetTitleFn window_set_title_fn;
	HeroIWindowSysEventsPollFn events_poll_fn;
};

typedef U8 HeroWindowSysBackendType;
enum {
#if HERO_X11_ENABLE
	HERO_WINDOW_SYS_BACKEND_TYPE_X11,
#endif
};

typedef U8 HeroEventType;
enum {
	HERO_EVENT_TYPE_WINDOW_MOVED,             // HeroEvent.moved
	HERO_EVENT_TYPE_WINDOW_RESIZED,           // HeroEvent.resized
	HERO_EVENT_TYPE_WINDOW_CLOSE_REQUEST,     // HeroEvent.any
	HERO_EVENT_TYPE_WINDOW_CLOSED,            // HeroEvent.any
	HERO_EVENT_TYPE_KEYBOARD_ENTER,           // HeroEvent.any
	HERO_EVENT_TYPE_KEYBOARD_LEAVE,           // HeroEvent.any
	HERO_EVENT_TYPE_KEYBOARD_KEY_UP,          // HeroEvent.key
	HERO_EVENT_TYPE_KEYBOARD_KEY_DOWN,        // HeroEvent.key
	HERO_EVENT_TYPE_KEYBOARD_KEY_INPUT_UTF32, // HeroEvent.key_utf32
	HERO_EVENT_TYPE_MOUSE_ENTER,              // HeroEvent.any
	HERO_EVENT_TYPE_MOUSE_LEAVE,              // HeroEvent.any
	HERO_EVENT_TYPE_MOUSE_MOVED,              // HeroEvent.moved
	HERO_EVENT_TYPE_MOUSE_BUTTON_UP,          // HeroEvent.mouse_button
	HERO_EVENT_TYPE_MOUSE_BUTTON_DOWN,        // HeroEvent.mouse_button
	HERO_EVENT_TYPE_MOUSE_WHEEL_MOVED,        // HeroEvent.mouse_wheel_moved
	HERO_EVENT_TYPE_COUNT,
};

extern const char* hero_event_type_strings[HERO_EVENT_TYPE_COUNT];

typedef union HeroEvent HeroEvent;
union HeroEvent {
	struct {
		HeroEventType type;
		HeroWindowId window_id;
	} any;
	struct {
		HeroEventType type;
		HeroWindowId window_id;
		U32 x;
		U32 y;
		S32 rel_x;
		S32 rel_y;
	} moved;
	struct {
		HeroEventType type;
		HeroWindowId window_id;
		U32 width;
		U32 height;
		S32 rel_width;
		S32 rel_height;
	} resized;
	struct {
		HeroEventType type;
		HeroWindowId window_id;
		HeroKeyCode code;
		HeroKeyCode scan_code;
		HeroKeyMod mod;
		U8 is_pressed;
		U8 is_repeat;
	} key;
	struct {
		HeroEventType type;
		HeroWindowId window_id;
		char bytes[4];
		U8 size;
	} key_input_utf32;
	struct {
		HeroEventType type;
		HeroWindowId window_id;
		U8 is_pressed;
		HeroMouseButtons button;
	} mouse_button;
	struct {
		HeroEventType type;
		HeroWindowId window_id;
		S32 rel_x;
		S32 rel_y;
	} mouse_wheel_moved;
};

#define HERO_STACK_ELMT_TYPE HeroEvent
#include "stack_gen.inl"

#define HERO_OBJECT_ID_TYPE HeroWindowId
#define HERO_OBJECT_TYPE HeroWindow
#include "object_pool_gen.inl"

typedef struct HeroWindowSys HeroWindowSys;
struct HeroWindowSys {
	union {
#if HERO_X11_ENABLE
		HeroX11 x11;
#endif
	} backend;

	HeroWindowSysBackendType backend_type;
	HeroIWindowSysVTable vtable;

	HeroStack(HeroEvent) event_queue;

	HeroKeyboard keyboard;
	HeroMouse mouse;

	HeroDisplayOutput* display_outputs;
	U32 display_outputs_count;

	HeroDisplayOutputId primary_display_output_id;

	HeroObjectPool(HeroWindow) windows_pool;

	HeroIAlctor alctor;
};

extern HeroWindowSys hero_window_sys;

typedef struct HeroWindowSysSetup HeroWindowSysSetup;
struct HeroWindowSysSetup {
	HeroWindowSysBackendType backend_type;
	Uptr windows_cap;
	HeroIAlctor alctor;
};

HeroResult hero_window_sys_init(HeroWindowSysSetup* setup);

HeroResult hero_window_open(const char* title, U32 x, U32 y, U32 width, U32 height, HeroWindowId* id_out);
HeroResult hero_window_close(HeroWindowId id);
HeroResult hero_window_get(HeroWindowId id, HeroWindow** ptr_out);

HeroResult hero_event_queue(HeroEvent** ptr_out);
HeroResult hero_events_poll(void);
HeroResult hero_events_iter_next(HeroEvent** ptr_out, U32* idx_mut);

HeroResult hero_event_queue_window_keyboard_focus_changed(HeroWindowId window_id);
HeroResult hero_event_queue_keyboard_key_input_utf32(HeroWindowId window_id, const char* bytes, U8 size);
HeroResult hero_event_queue_keyboard_key_changed(HeroWindowId window_id, bool is_pressed, HeroKeyCode scan_code);
HeroResult hero_event_queue_window_mouse_focus_changed(HeroWindowId window_id);
HeroResult hero_event_queue_mouse_moved(HeroWindowId window_id, U32 x, U32 y);
HeroResult hero_event_queue_mouse_button_changed(HeroWindowId window_id, bool is_pressed, HeroMouseButtons button);
HeroResult hero_event_queue_mouse_wheel_moved(HeroWindowId window_id, S32 rel_x, S32 rel_y);
HeroResult hero_event_queue_window_moved(HeroWindow* window, HeroWindowId window_id, U32 x, U32 y);
HeroResult hero_event_queue_window_resized(HeroWindow* window, HeroWindowId window_id, U32 width, U32 height);
HeroResult hero_event_queue_window_close_requested(HeroWindowId window_id);
HeroResult hero_event_queue_window_closed(HeroWindowId window_id);
void hero_keyboard_map_reset_default(void);

bool hero_keyboard_scan_code_to_key_code(HeroScanCode scan_code);
bool hero_keyboard_scan_code_is_pressed(HeroScanCode scan_code);
bool hero_keyboard_scan_code_has_been_pressed(HeroScanCode scan_code);
bool hero_keyboard_scan_code_has_been_released(HeroScanCode scan_code);
bool hero_keyboard_key_code_to_scan_code(HeroKeyCode key_code);
bool hero_keyboard_key_code_is_pressed(HeroKeyCode key_code);
bool hero_keyboard_key_code_has_been_pressed(HeroKeyCode key_code);
bool hero_keyboard_key_code_has_been_released(HeroKeyCode key_code);

#endif // _HERO_WINDOW_H_

