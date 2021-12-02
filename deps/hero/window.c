#ifndef _HERO_WINDOW_H_
#include "window.h"
#endif

// ===========================================
//
//
// X11
//
//
// ===========================================
#if HERO_X11_ENABLE

//
// WARNING: xlib likes to define lots of identifiers without any prefix :(
// such as:
// - Window
// - Display
// - Bool
// - Visual
//
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

HeroKeyCode hero_evdev_scancode_to_scan_code_map[] = {
	HERO_KEY_CODE_NULL,
	HERO_KEY_CODE_ESCAPE,
	HERO_KEY_CODE_1,
	HERO_KEY_CODE_2,
	HERO_KEY_CODE_3,
	HERO_KEY_CODE_4,
	HERO_KEY_CODE_5,
	HERO_KEY_CODE_6,
	HERO_KEY_CODE_7,
	HERO_KEY_CODE_8,
	HERO_KEY_CODE_9,
	HERO_KEY_CODE_0,
	HERO_KEY_CODE_MINUS,
	HERO_KEY_CODE_EQUALS,
	HERO_KEY_CODE_BACKSPACE,
	HERO_KEY_CODE_TAB,
	HERO_KEY_CODE_Q,
	HERO_KEY_CODE_W,
	HERO_KEY_CODE_E,
	HERO_KEY_CODE_R,
	HERO_KEY_CODE_T,
	HERO_KEY_CODE_Y,
	HERO_KEY_CODE_U,
	HERO_KEY_CODE_I,
	HERO_KEY_CODE_O,
	HERO_KEY_CODE_P,
	HERO_KEY_CODE_LEFT_BRACKET,
	HERO_KEY_CODE_RIGHT_BRACKET,
	HERO_KEY_CODE_ENTER,
	HERO_KEY_CODE_LEFT_CTRL,
	HERO_KEY_CODE_A,
	HERO_KEY_CODE_S,
	HERO_KEY_CODE_D,
	HERO_KEY_CODE_F,
	HERO_KEY_CODE_G,
	HERO_KEY_CODE_H,
	HERO_KEY_CODE_J,
	HERO_KEY_CODE_K,
	HERO_KEY_CODE_L,
	HERO_KEY_CODE_SEMICOLON,
	HERO_KEY_CODE_APOSTROPHE,
	HERO_KEY_CODE_GHERO,
	HERO_KEY_CODE_LEFT_SHIFT,
	HERO_KEY_CODE_BACKSLASH,
	HERO_KEY_CODE_Z,
	HERO_KEY_CODE_X,
	HERO_KEY_CODE_C,
	HERO_KEY_CODE_V,
	HERO_KEY_CODE_B,
	HERO_KEY_CODE_N,
	HERO_KEY_CODE_M,
	HERO_KEY_CODE_COMMA,
	HERO_KEY_CODE_FULL_STOP,
	HERO_KEY_CODE_FORWARDSLASH,
	HERO_KEY_CODE_RIGHT_SHIFT,
	HERO_KEY_CODE_KP_MULTIPLY,
	HERO_KEY_CODE_LEFT_ALT,
	HERO_KEY_CODE_SPACE,
	HERO_KEY_CODE_CAPS_LOCK,
	HERO_KEY_CODE_F1,
	HERO_KEY_CODE_F2,
	HERO_KEY_CODE_F3,
	HERO_KEY_CODE_F4,
	HERO_KEY_CODE_F5,
	HERO_KEY_CODE_F6,
	HERO_KEY_CODE_F7,
	HERO_KEY_CODE_F8,
	HERO_KEY_CODE_F9,
	HERO_KEY_CODE_F10,
	HERO_KEY_CODE_NUM_LOCK_OR_CLEAR,
	HERO_KEY_CODE_SCROLL_LOCK,
	HERO_KEY_CODE_KP_7,
	HERO_KEY_CODE_KP_8,
	HERO_KEY_CODE_KP_9,
	HERO_KEY_CODE_KP_MINUS,
	HERO_KEY_CODE_KP_4,
	HERO_KEY_CODE_KP_5,
	HERO_KEY_CODE_KP_6,
	HERO_KEY_CODE_KP_PLUS,
	HERO_KEY_CODE_KP_1,
	HERO_KEY_CODE_KP_2,
	HERO_KEY_CODE_KP_3,
	HERO_KEY_CODE_KP_0,
	HERO_KEY_CODE_KP_FULL_STOP,
	[85] = HERO_KEY_CODE_LANG_5, // KEY_ZENKAKUHANKAKU
	HERO_KEY_CODE_NON_US_BACKSLASH, // KEY_102ND
	HERO_KEY_CODE_F11,
	HERO_KEY_CODE_F12,
	HERO_KEY_CODE_INTERNATIONAL_1, // KEY_RO
	HERO_KEY_CODE_LANG_3, // KEY_KATAKANA
	HERO_KEY_CODE_LANG_4, // KEY_HIRAGANA
	HERO_KEY_CODE_INTERNATIONAL_4, // KEY_HENKAN
	HERO_KEY_CODE_INTERNATIONAL_2, // KEY_KATAKANAHIRAGANA
	HERO_KEY_CODE_INTERNATIONAL_5, // KEY_MUHENKAN
	HERO_KEY_CODE_INTERNATIONAL_5, // KEY_KPJPCOMMA
	HERO_KEY_CODE_KP_ENTER,
	HERO_KEY_CODE_RIGHT_CTRL,
	HERO_KEY_CODE_KP_DIVIDE,
	HERO_KEY_CODE_SYS_REQ,
	HERO_KEY_CODE_RIGHT_ALT,
	HERO_KEY_CODE_NULL, // KEY_LINEFEED
	HERO_KEY_CODE_HOME,
	HERO_KEY_CODE_UP,
	HERO_KEY_CODE_PAGE_UP,
	HERO_KEY_CODE_LEFT,
	HERO_KEY_CODE_RIGHT,
	HERO_KEY_CODE_END,
	HERO_KEY_CODE_DOWN,
	HERO_KEY_CODE_PAGE_DOWN,
	HERO_KEY_CODE_INSERT,
	HERO_KEY_CODE_DELETE,
	HERO_KEY_CODE_NULL, // KEY_MACRO
	HERO_KEY_CODE_MUTE,
	HERO_KEY_CODE_VOLUME_DOWN,
	HERO_KEY_CODE_VOLUME_UP,
	HERO_KEY_CODE_POWER,
	HERO_KEY_CODE_KP_EQUALS,
	HERO_KEY_CODE_KP_PLUS_MINUS,
	HERO_KEY_CODE_PAUSE,
	[121] = HERO_KEY_CODE_KP_COMMA,
	HERO_KEY_CODE_LANG_1, // KEY_HANGUEL
	HERO_KEY_CODE_LANG_2, // KEY_HANJA
	HERO_KEY_CODE_INTERNATIONAL_3, // KEY_YEN
	HERO_KEY_CODE_LEFT_META,
	HERO_KEY_CODE_RIGHT_META,
	HERO_KEY_CODE_APPLICATION, // KEY_COMPOSE
	HERO_KEY_CODE_STOP,
	HERO_KEY_CODE_AGAIN,
	HERO_KEY_CODE_NULL, // KEY_PROPS
	HERO_KEY_CODE_UNDO,
	HERO_KEY_CODE_NULL, // KEY_FRONT
	HERO_KEY_CODE_COPY,
	HERO_KEY_CODE_NULL, // KEY_OPEN
	HERO_KEY_CODE_PASTE,
	HERO_KEY_CODE_FIND,
	HERO_KEY_CODE_CUT,
	HERO_KEY_CODE_HELP,
	HERO_KEY_CODE_MENU,
	HERO_KEY_CODE_CALCULATOR,
	HERO_KEY_CODE_NULL, // KEY_SETUP
	HERO_KEY_CODE_SLEEP,
	HERO_KEY_CODE_NULL, // KEY_WAKEUP
	HERO_KEY_CODE_NULL, // KEY_FILE
	HERO_KEY_CODE_NULL, // KEY_SENDFILE
	HERO_KEY_CODE_NULL, // KEY_DELETEFILE
	HERO_KEY_CODE_NULL, // KEY_XFER
	HERO_KEY_CODE_APP_1, // KEY_PROG1
	HERO_KEY_CODE_APP_2, // KEY_PROG2
	HERO_KEY_CODE_WWW,
	HERO_KEY_CODE_NULL, // KEY_MSDOS
	HERO_KEY_CODE_NULL, // KEY_COFFEE
	HERO_KEY_CODE_NULL, // KEY_DIRECTION
	HERO_KEY_CODE_NULL, // KEY_CYCLEWINDOWS
	HERO_KEY_CODE_MAIL,
	HERO_KEY_CODE_AC_BOOKMARKS,
	HERO_KEY_CODE_COMPUTER,
	HERO_KEY_CODE_AC_BACK,
	HERO_KEY_CODE_AC_FORWARD,
	HERO_KEY_CODE_NULL, // KEY_CLOSECD
	HERO_KEY_CODE_EJECT,
    HERO_KEY_CODE_NULL, // KEY_EJECTCLOSECD
	HERO_KEY_CODE_AUDIO_NEXT,
	HERO_KEY_CODE_AUDIO_PLAY,
	HERO_KEY_CODE_AUDIO_PREV,
	HERO_KEY_CODE_AUDIO_STOP,
	HERO_KEY_CODE_NULL, // KEY_RECORD
	HERO_KEY_CODE_AUDIO_REWIND,
	HERO_KEY_CODE_NULL, // KEY_PHONE
	HERO_KEY_CODE_NULL, // KEY_ISO
	HERO_KEY_CODE_NULL, // KEY_CONFIG
	HERO_KEY_CODE_AC_HOME,
	HERO_KEY_CODE_AC_REFRESH,
	HERO_KEY_CODE_NULL, // KEY_EXIT
	HERO_KEY_CODE_NULL, // KEY_MOVE
	HERO_KEY_CODE_NULL, // KEY_EDIT
	HERO_KEY_CODE_NULL, // KEY_SCROLLUP
	HERO_KEY_CODE_NULL, // KEY_SCROLLDOWN
	HERO_KEY_CODE_KP_LEFT_PAREN,
	HERO_KEY_CODE_KP_RIGHT_PAREN,
	HERO_KEY_CODE_NULL, // KEY_NEW
	HERO_KEY_CODE_NULL, // KEY_REDO
	HERO_KEY_CODE_F13,
	HERO_KEY_CODE_F14,
	HERO_KEY_CODE_F15,
	HERO_KEY_CODE_F16,
	HERO_KEY_CODE_F17,
	HERO_KEY_CODE_F18,
	HERO_KEY_CODE_F19,
	HERO_KEY_CODE_F20,
	HERO_KEY_CODE_F21,
	HERO_KEY_CODE_F22,
	HERO_KEY_CODE_F23,
	HERO_KEY_CODE_F24,
	[200] = HERO_KEY_CODE_NULL, // KEY_PLAYCD
	HERO_KEY_CODE_NULL, // KEY_PAUSECD
	HERO_KEY_CODE_NULL, // KEY_PROG3
	HERO_KEY_CODE_NULL, // KEY_PROG4
	[205] = HERO_KEY_CODE_NULL, // KEY_SUSPEND
	HERO_KEY_CODE_NULL, // KEY_CLOSE
	HERO_KEY_CODE_NULL, // KEY_PLAY
	HERO_KEY_CODE_AUDIO_FASTFORWARD,
	HERO_KEY_CODE_NULL, // KEY_BASSBOOST
	HERO_KEY_CODE_NULL, // KEY_PRINT
	HERO_KEY_CODE_NULL, // KEY_HP
	HERO_KEY_CODE_NULL, // KEY_CAMERA
	HERO_KEY_CODE_NULL, // KEY_SOUND
	HERO_KEY_CODE_NULL, // KEY_QUESTION
	HERO_KEY_CODE_NULL, // KEY_EMAIL
	HERO_KEY_CODE_NULL, // KEY_CHAT
	HERO_KEY_CODE_AC_SEARCH,
	HERO_KEY_CODE_NULL, // KEY_CONNECT
	HERO_KEY_CODE_NULL, // KEY_FINANCE
	HERO_KEY_CODE_NULL, // KEY_SPORT
	HERO_KEY_CODE_NULL, // KEY_SHOP
	HERO_KEY_CODE_ALT_ERASE,
	HERO_KEY_CODE_CANCEL,
	HERO_KEY_CODE_BRIGHTNESS_DOWN,
	HERO_KEY_CODE_BRIGHTNESS_UP,
	HERO_KEY_CODE_NULL, // KEY_MEDIA
	HERO_KEY_CODE_DISPLAY_SWITCH,
	HERO_KEY_CODE_KBD_ILLUM_TOGGLE,
	HERO_KEY_CODE_KBD_ILLUM_DOWN,
	HERO_KEY_CODE_KBD_ILLUM_UP,
	HERO_KEY_CODE_NULL, // KEY_SEND
	HERO_KEY_CODE_NULL, // KEY_REPLY
	HERO_KEY_CODE_NULL, // KEY_FORWARDMAIL
	HERO_KEY_CODE_NULL, // KEY_SAVE
	HERO_KEY_CODE_NULL, // KEY_DOCUMENTS
	HERO_KEY_CODE_NULL, // KEY_BATTERY
};

HeroKeyCode hero_x11_fn_keysym_to_key_code[] = {
	[0x08] = HERO_KEY_CODE_BACKSPACE,
	HERO_KEY_CODE_TAB,

	[0x0b] = HERO_KEY_CODE_CLEAR,
	[0x0d] = HERO_KEY_CODE_ENTER,
	[0x13] = HERO_KEY_CODE_PAUSE,
	HERO_KEY_CODE_SCROLL_LOCK,
	HERO_KEY_CODE_SYS_REQ,
	[0x1b] = HERO_KEY_CODE_ESCAPE,

	[0x50] = HERO_KEY_CODE_HOME,
	HERO_KEY_CODE_LEFT,
	HERO_KEY_CODE_UP,
	HERO_KEY_CODE_RIGHT,
	HERO_KEY_CODE_DOWN,
	HERO_KEY_CODE_PAGE_UP,
	HERO_KEY_CODE_PAGE_DOWN,
	HERO_KEY_CODE_END,

	[0x60] = HERO_KEY_CODE_SELECT,
	HERO_KEY_CODE_PRINT_SCREEN,
	HERO_KEY_CODE_EXECUTE,
	HERO_KEY_CODE_INSERT,

	[0x65] = HERO_KEY_CODE_UNDO,
	[0x67] = HERO_KEY_CODE_MENU,
	HERO_KEY_CODE_FIND,
	HERO_KEY_CODE_CANCEL,
	HERO_KEY_CODE_HELP,

	[0x7f] = HERO_KEY_CODE_NUM_LOCK_OR_CLEAR,
	[0x80] = HERO_KEY_CODE_KP_SPACE,
	[0x89] = HERO_KEY_CODE_KP_TAB,
	[0x8d] = HERO_KEY_CODE_KP_ENTER,
	[0xbd] = HERO_KEY_CODE_KP_EQUALS,

	[0xaa] = HERO_KEY_CODE_KP_MULTIPLY,
	HERO_KEY_CODE_KP_PLUS,

	[0xad] = HERO_KEY_CODE_KP_MINUS,
	HERO_KEY_CODE_KP_DECIMAL,
	HERO_KEY_CODE_KP_DIVIDE,

	[0xb0] = HERO_KEY_CODE_KP_0,
	HERO_KEY_CODE_KP_1,
	HERO_KEY_CODE_KP_2,
	HERO_KEY_CODE_KP_3,
	HERO_KEY_CODE_KP_4,
	HERO_KEY_CODE_KP_5,
	HERO_KEY_CODE_KP_6,
	HERO_KEY_CODE_KP_7,
	HERO_KEY_CODE_KP_8,
	HERO_KEY_CODE_KP_9,

	[0xbe] = HERO_KEY_CODE_F1,
	HERO_KEY_CODE_F2,
	HERO_KEY_CODE_F3,
	HERO_KEY_CODE_F4,
	HERO_KEY_CODE_F5,
	HERO_KEY_CODE_F6,
	HERO_KEY_CODE_F7,
	HERO_KEY_CODE_F8,
	HERO_KEY_CODE_F9,
	HERO_KEY_CODE_F10,
	HERO_KEY_CODE_F11,
	HERO_KEY_CODE_F12,
	HERO_KEY_CODE_F13,
	HERO_KEY_CODE_F14,
	HERO_KEY_CODE_F15,
	HERO_KEY_CODE_F16,
	HERO_KEY_CODE_F17,
	HERO_KEY_CODE_F18,
	HERO_KEY_CODE_F19,
	HERO_KEY_CODE_F20,
	HERO_KEY_CODE_F21,
	HERO_KEY_CODE_F22,
	HERO_KEY_CODE_F23,
	HERO_KEY_CODE_F24,

	[0xe1] = HERO_KEY_CODE_LEFT_SHIFT,
	HERO_KEY_CODE_RIGHT_SHIFT,
	HERO_KEY_CODE_LEFT_CTRL,
	HERO_KEY_CODE_RIGHT_CTRL,
	HERO_KEY_CODE_CAPS_LOCK,
	HERO_KEY_CODE_CAPS_LOCK,
	HERO_KEY_CODE_LEFT_META,
	HERO_KEY_CODE_RIGHT_META,
	HERO_KEY_CODE_LEFT_ALT,
	HERO_KEY_CODE_RIGHT_ALT,
	HERO_KEY_CODE_LEFT_META,
	HERO_KEY_CODE_RIGHT_META,

	[0xff] = HERO_KEY_CODE_DELETE,
};

HeroKeyCode hero_x11_fn_ex_keysym_to_key_code[] = {
	[0x03] = HERO_KEY_CODE_RIGHT_ALT,
	[0xff] = HERO_KEY_CODE_NULL,
};

HeroKeyCode hero_x11_latin_2_keysym_to_key_code[] = {
	[0x80] = HERO_KEY_CODE_EJECT,
};

HeroKeyCode hero_x11_latin_1_keysym_to_key_code[] = {
	[0x20] = HERO_KEY_CODE_SPACE,
	[0x27] = HERO_KEY_CODE_APOSTROPHE,
	[0x2c] = HERO_KEY_CODE_COMMA,
	HERO_KEY_CODE_MINUS,
	HERO_KEY_CODE_FULL_STOP,
	HERO_KEY_CODE_FORWARDSLASH,
	HERO_KEY_CODE_0,
	HERO_KEY_CODE_1,
	HERO_KEY_CODE_2,
	HERO_KEY_CODE_3,
	HERO_KEY_CODE_4,
	HERO_KEY_CODE_5,
	HERO_KEY_CODE_6,
	HERO_KEY_CODE_7,
	HERO_KEY_CODE_8,
	HERO_KEY_CODE_9,

	[0x3b] = HERO_KEY_CODE_SEMICOLON,
	[0x3d] = HERO_KEY_CODE_EQUALS,

	[0x5b] = HERO_KEY_CODE_LEFT_BRACKET,
	HERO_KEY_CODE_BACKSLASH,
	HERO_KEY_CODE_RIGHT_BRACKET,
	[0x60] = HERO_KEY_CODE_GHERO,
	HERO_KEY_CODE_A,
	HERO_KEY_CODE_B,
	HERO_KEY_CODE_C,
	HERO_KEY_CODE_D,
	HERO_KEY_CODE_E,
	HERO_KEY_CODE_F,
	HERO_KEY_CODE_G,
	HERO_KEY_CODE_H,
	HERO_KEY_CODE_I,
	HERO_KEY_CODE_J,
	HERO_KEY_CODE_K,
	HERO_KEY_CODE_L,
	HERO_KEY_CODE_M,
	HERO_KEY_CODE_N,
	HERO_KEY_CODE_O,
	HERO_KEY_CODE_P,
	HERO_KEY_CODE_Q,
	HERO_KEY_CODE_R,
	HERO_KEY_CODE_S,
	HERO_KEY_CODE_T,
	HERO_KEY_CODE_U,
	HERO_KEY_CODE_V,
	HERO_KEY_CODE_W,
	HERO_KEY_CODE_X,
	HERO_KEY_CODE_Y,
	HERO_KEY_CODE_Z,

	[0xff] = HERO_KEY_CODE_NULL,
};

HeroKeyCode hero_x11_keysym_to_key_code(KeySym sym) {
	if (sym >= 0xff00 && sym <= 0xffff) {
		return hero_x11_fn_keysym_to_key_code[sym - 0xff00];
	} else if (sym >= 0xfe00 && sym <= 0xfeff) {
		return hero_x11_fn_ex_keysym_to_key_code[sym - 0xfe00];
	} else if (sym >= 0x100 && sym <= 0x1ff) {
		return hero_x11_latin_2_keysym_to_key_code[sym - 0x100];
	} else if (sym <= 0xff) {
		return hero_x11_latin_1_keysym_to_key_code[sym];
	} else {
		return HERO_KEY_CODE_NULL;
	}
}

void hero_x11_update_keyboard_map(void) {
	XDisplay* connection = hero_window_sys.backend.x11.connection;
	XkbDescPtr xkb = hero_window_sys.backend.x11.xkb;
	hero_keyboard_map_reset_default();

	XkbStateRec state;
	XkbGetUpdatedMap(connection, XkbAllClientInfoMask, xkb);

    unsigned char group = 0;
	if (XkbGetState(connection, XkbUseCoreKbd, &state) == Success) {
		group = state.group;
	}

	int first_keycode = xkb->min_key_code;
	int last_keycode = xkb->max_key_code;
	// we dont map any more keys in evdev after 236
	if (last_keycode > 236) last_keycode = 236;

	//
	// does this look confusing AF?
	// documentation here: https://www.x.org/releases/current/doc/libX11/XKB/xkblib.html
	// evdev scancodes are 8 less than x11 keycodes
	XkbGetKeySyms(connection, first_keycode, last_keycode - first_keycode, xkb);
	XkbClientMapRec* cmap = xkb->map;
	for (int x11keycode = 8; x11keycode < last_keycode; x11keycode += 1) {
		XkbSymMapRec* sym_map = &cmap->key_sym_map[x11keycode];
		int sym_start_idx = sym_map->offset;
		// since we do not want any key modifiers, our shift is 0
		int shift = 0;
		int sym_idx = sym_start_idx + (sym_map->width * group) + shift;
		KeySym sym = cmap->syms[sym_idx];

		HeroScanCode scan_code = hero_evdev_scancode_to_scan_code_map[x11keycode - 8];
		HeroKeyCode key_code = hero_x11_keysym_to_key_code(sym);

#ifdef HERO_LOG_KEYMAP_KEYSYM
		printf("scan_code: %s -> key_sym: %s = 0x%lx\n", wal_key_code_string(scan_code), XKeysymToString(sym), sym);
#endif

		hero_window_sys.keyboard.scan_code_to_key_code[scan_code] = key_code;
		hero_window_sys.keyboard.key_code_to_scan_code[key_code] = scan_code;
	}

#ifdef HERO_LOG_KEYMAP
	for (int sc = 0; sc < HERO_KEY_CODE_COUNT; sc += 1) {
		printf("scan_code: %s -> key_code: %s\n", wal_key_code_string(sc), wal_key_code_string(hero_window_sys.keyboard.scan_code_to_key_code[sc]));
	}
#endif
}

void hero_x11_init_display_outputs(void) {
	XDisplay* connection = hero_window_sys.backend.x11.connection;

	XRRScreenResources* xsr = XRRGetScreenResources(connection, XDefaultRootWindow(connection));
	if (!xsr) return;

	RROutput primary_output = XRRGetOutputPrimary(connection, XDefaultRootWindow(connection));

	int display_outputs_count = xsr->noutput;
	HeroDisplayOutput* display_outputs = hero_alloc_array(HeroDisplayOutput, hero_system_alctor, HERO_ALLOC_TAG_WINDOW_DISPLAY_OUTPUTS, display_outputs_count);
	HERO_ASSERT(display_outputs, "failed to allocate memory for display outputs");
	HERO_ZERO_ELMT_MANY(display_outputs, display_outputs_count);

	//
	// loop over all of the display output ports on the machine
	//
	for (int i = 0; i < display_outputs_count; i += 1) {
		RROutput output = xsr->outputs[i];
		XRROutputInfo* opi = XRRGetOutputInfo(connection, xsr, output);

		HeroDisplayOutput* display_output_elmt = &display_outputs[i];
		if (output == primary_output) {
			display_output_elmt->flags |= HERO_DISPLAY_OUTPUT_FLAGS_IS_PRIMARY;
			hero_window_sys.primary_display_output_id.raw = i + 1;
		}

		//
		// if we have a crtc, then the display_output the port is connected to is enabled
		//
		RRMode mode = 0;
		Rotation rotation = 0;
		if (opi->crtc) {
			XRRCrtcInfo* xci = XRRGetCrtcInfo(connection, xsr, opi->crtc);
			mode = xci->mode;
			rotation = xci->rotation;
			display_output_elmt->x = xci->x;
			display_output_elmt->y = xci->y;
			display_output_elmt->width = xci->width;
			display_output_elmt->height = xci->height;
			display_output_elmt->flags |= HERO_DISPLAY_OUTPUT_FLAGS_IS_ENABLED;
			XRRFreeCrtcInfo(xci);
		}

		int video_modes_count = opi->nmode;
		HeroVideoMode* video_modes = NULL;
		HeroDisplayOutputId video_mode_id = {0};
		if (video_modes_count) {
			// the display_output is connected if it has video modes
			display_output_elmt->flags |= HERO_DISPLAY_OUTPUT_FLAGS_IS_CONNECTED;

			video_modes = hero_alloc_array(HeroVideoMode, hero_system_alctor, HERO_ALLOC_TAG(HERO_ALLOC_TAG_WINDOW_DISPLAY_OUTPUT_VIDEO_MODES, i), video_modes_count);
			HERO_ASSERT(video_modes, "failed to allocate memory for video modes");
			HERO_ZERO_ELMT_MANY(video_modes, video_modes_count);

			//
			// now extract out all of the video modes
			//
			for (int j = 0; j < video_modes_count; j += 1) {
				RRMode mode_id = opi->modes[j];
				if (mode == mode_id) {
					// found the mode our display output has
					video_mode_id.raw = j + 1;
				}

				//
				// locate the info for this video mode
				XRRModeInfo* xmi = NULL;
				for (int k = 0; k < xsr->nmode; k += 1) {
					xmi = &xsr->modes[k];
					if (xmi->id == mode_id) break;
				}

				HeroVideoMode* video_mode_elmt = &video_modes[j];
				video_mode_elmt->backend.x11.mode_id = mode_id;

#define XRANDR_ROTATION_LEFT    (1 << 1)
#define XRANDR_ROTATION_RIGHT   (1 << 3)
				if (rotation & (XRANDR_ROTATION_LEFT|XRANDR_ROTATION_RIGHT)) {
					video_mode_elmt->width = xmi->height;
					video_mode_elmt->height = xmi->width;
				} else {
					video_mode_elmt->width = xmi->width;
					video_mode_elmt->height = xmi->height;
				}
				strncpy(video_mode_elmt->name, xmi->name, HERO_MIN(xmi->nameLength + 1, HERO_VIDEO_MODE_NAME_CAP));

				video_mode_elmt->refresh_rate = (xmi->hTotal && xmi->vTotal)
					? (U16)round(((double)xmi->dotClock / (double)(xmi->hTotal * xmi->vTotal)))
					: 0;
			}
		}

		strncpy(display_output_elmt->name, opi->name, HERO_MIN(opi->nameLen + 1, HERO_DISPLAY_OUTPUT_NAME_CAP));
		display_output_elmt->video_mode_id = video_mode_id;
		display_output_elmt->video_modes = video_modes;
		display_output_elmt->video_modes_count = video_modes_count;
		display_output_elmt->backend.x11.crtc_id = opi->crtc;

		XRRFreeOutputInfo(opi);
	}

	hero_window_sys.display_outputs = display_outputs;
	hero_window_sys.display_outputs_count = display_outputs_count;

	XRRFreeScreenResources(xsr);
}

HeroResult hero_x11_init(void) {
	XInitThreads();

	XDisplay* connection = XOpenDisplay(NULL);
	HERO_ASSERT(connection, "failed to open X11 connection");

	// loads the XMODIFIERS environment variable to see what IME to use
	XSetLocaleModifiers("");
	XIM xim = XOpenIM(connection, 0, 0, 0);
    if(!xim){
        // fallback to internal input method
        XSetLocaleModifiers("@im=none");
        xim = XOpenIM(connection, 0, 0, 0);
    }

	int screen = XDefaultScreen(connection);
	Visual* visual = XDefaultVisual(connection, screen);

	hero_window_sys.backend.x11.connection = connection;
	hero_window_sys.backend.x11.xim = xim;
	hero_window_sys.backend.x11.wm_delete_window = XInternAtom(connection, "WM_DELETE_WINDOW", False);
	hero_window_sys.backend.x11.visual_id = visual->visualid;

	{
		//
		// XKB
		//
		XAutoRepeatOn(connection);
		int major = 1;
		int minor = 0;
		HERO_ASSERT(
			XkbQueryExtension(connection, NULL, NULL, NULL, &major, &minor),
			"X11 error: cannot find XKB extension\n"
		);
		hero_window_sys.backend.x11.xkb = XkbGetMap(connection, XkbAllClientInfoMask, XkbUseCoreKbd);

		// this will remove KeyRelease events for held keys.
		int repeat = 0;
		XkbSetDetectableAutoRepeat(connection, True, &repeat);

		int first_keycode = 0;
		int last_keycode = 0;
		XDisplayKeycodes(connection, &first_keycode, &last_keycode);
		hero_x11_update_keyboard_map();
	}

	{
		//
		// Xrandr
		//

		int major = 1;
		int minor = 0;
		if (!XRRQueryVersion(connection, &major, &minor)) {
			printf("X11 error: cannot find XRandR extension\n");
			exit(1);
		}
		XRRQueryExtension(connection, &hero_window_sys.backend.x11.xrandr_event_base, &hero_window_sys.backend.x11.xrandr_error_base);

		hero_x11_init_display_outputs();
	}

	return HERO_SUCCESS;
}

HeroResult hero_x11_window_open(HeroWindow* window, HeroWindowId window_id, const char* title) {
	XDisplay* connection = hero_window_sys.backend.x11.connection;

	//
	// create the window
	XWindow xwindow = 0;
	{
		int screen = XDefaultScreen(connection);
		Visual* visual = XDefaultVisual(connection, screen);
		int depth = XDefaultDepth(connection, screen);
		Colormap colormap = XDefaultColormap(connection, screen);

		XSetWindowAttributes swa = {0};
		swa.event_mask = StructureNotifyMask;
		swa.colormap = colormap;

		xwindow = XCreateWindow(connection, XRootWindow(connection, screen),
			window->x, window->y, window->width, window->height, 0, depth, InputOutput,
			visual, CWBackPixel|CWBorderPixel|CWEventMask, &swa);
		HERO_ASSERT(xwindow, "failed to create window");
	}

	XSizeHints* sizehints = XAllocSizeHints();
	sizehints->flags = PSize | USPosition;
	sizehints->x      = window->x;
	sizehints->y      = window->y;
	sizehints->width  = window->width;
	sizehints->height = window->height;
	XSetWMProperties(connection, xwindow, NULL, NULL, NULL, 0, sizehints, NULL, NULL);
	XFree(sizehints);

	//
	// create an input context used that is used by the whole window.
	// you are supposed to have one of these per text field, but this should do :D
	XIC xic = XCreateIC(hero_window_sys.backend.x11.xim,
		XNInputStyle,   XIMPreeditNothing | XIMStatusNothing,
		XNClientWindow, xwindow,
		XNFocusWindow,  xwindow,
		NULL);

	XSetICFocus(xic);

	window->backend.x11.xwindow = xwindow;
	window->backend.x11.xic = xic;

	//
	// update the title
	XStoreName(connection, xwindow, title);

	//
	// subscribe to the close window request event
	XSetWMProtocols(connection, window->backend.x11.xwindow, &hero_window_sys.backend.x11.wm_delete_window, 1);

	//
	// enable events to receive
	XSelectInput(connection, window->backend.x11.xwindow,
		EnterWindowMask
			| LeaveWindowMask
			| FocusChangeMask
			| StructureNotifyMask
			| KeyPressMask
			| KeyReleaseMask
			| PointerMotionMask
			| ButtonPressMask
			| ButtonReleaseMask
	);
	XRRSelectInput(connection, window->backend.x11.xwindow,
		RRCrtcChangeNotifyMask |
		RROutputChangeNotifyMask);

	//
	// show the window
	XMapWindow(connection, xwindow);

	//
	// flush all the commands to the X Server and wait until they have been processed
	XSync(connection, False);

	//
	// store in the hash table so we can map from an xwindow to our window id.
	HeroHashTableEntry(XWindow, HeroWindowId)* entry;
	HeroResult result = hero_hash_table(XWindow, HeroWindowId, find_or_insert)(
		&hero_window_sys.backend.x11.window_id_map, &window->backend.x11.xwindow, hero_system_alctor, HERO_ALLOC_TAG_WINDOW_X11_WINDOW_ID_MAP, &entry);
	if (result < 0) return result;
	entry->value = window_id;

	return HERO_SUCCESS;
}

HeroResult hero_x11_window_close(HeroWindow* window, HeroWindowId window_id) {
	XDisplay* connection = hero_window_sys.backend.x11.connection;

	//
	// destroy the input context
	XDestroyIC(window->backend.x11.xic);

	//
	// destroy the window
	XDestroyWindow(connection, window->backend.x11.xwindow);

	//
	// remove the xwindow from the hash table
	HeroResult result = hero_hash_table(XWindow, HeroWindowId, remove)(
		&hero_window_sys.backend.x11.window_id_map, &window->backend.x11.xwindow, hero_system_alctor, HERO_ALLOC_TAG_WINDOW_X11_WINDOW_ID_MAP);
	if (result < 0) return result;

	return HERO_SUCCESS;
}

HeroResult hero_x11_window_set_title(HeroWindow* window, const char* title) {
	XDisplay* connection = hero_window_sys.backend.x11.connection;
	XStoreName(connection, window->backend.x11.xwindow, title);
	XFlush(connection);
	return HERO_SUCCESS;
}

HeroResult hero_x11_window_id_map(XWindow xwindow, HeroWindowId* id_out) {
	HeroHashTableEntry(XWindow, HeroWindowId)* entry;
	HeroResult result = hero_hash_table(XWindow, HeroWindowId, find)(
		&hero_window_sys.backend.x11.window_id_map, &xwindow, &entry);
	if (result < 0) return result;
	*id_out = entry->value;
	return HERO_SUCCESS;
}

HeroResult hero_x11_event_process(XEvent* xe) {
	//
	// translate the X Window into our window identifier.
	HeroWindowId window_id;
	HeroResult result = hero_x11_window_id_map(xe->xany.window, &window_id);
	if (result < 0) return result;

	HeroWindow* window;
	result = hero_window_get(window_id, &window);
	HERO_RESULT_ASSERT(result);


	switch (xe->type) {
		case ClientMessage: {
			if ((Atom)xe->xclient.data.l[0] == hero_window_sys.backend.x11.wm_delete_window) {
				result = hero_event_queue_window_close_requested(window_id);
				if (result < 0) return result;
			}

			break;
		};
		case EnterNotify: {
			result = hero_event_queue_window_mouse_focus_changed(window_id);
			if (result < 0) return result;

			//
			// set x and y so rel_x and rel_y will be 0 when we push the mouse moved event.
			hero_window_sys.mouse.x = xe->xcrossing.x;
			hero_window_sys.mouse.y = xe->xcrossing.y;
			result = hero_event_queue_mouse_moved(window_id, HERO_MAX(xe->xcrossing.x, 0), HERO_MAX(xe->xcrossing.y, 0));
			if (result < 0) return result;
			break;
		};
		case LeaveNotify: {
			result = hero_event_queue_mouse_moved(window_id, HERO_MAX(xe->xcrossing.x, 0), HERO_MAX(xe->xcrossing.y, 0));
			if (result < 0) return result;

			// if the window is being grabbed, then ignore
			if (
				xe->xcrossing.mode == NotifyGrab ||
				xe->xcrossing.mode == NotifyUngrab ||
				xe->xcrossing.detail == NotifyInferior
			) {
				break;
			}

			result = hero_event_queue_window_mouse_focus_changed(((HeroWindowId){0}));
			if (result < 0) return result;
			break;
		};
		case FocusIn: {
			// if the window is being grabbed, then ignore
			if (xe->xfocus.mode == NotifyGrab || xe->xfocus.mode == NotifyUngrab) {
				break;
			}

			result = hero_event_queue_window_keyboard_focus_changed(window_id);
			if (result < 0) return result;
			break;
		};
		case FocusOut: {
			// if the window is being grabbed, then ignore
			if (xe->xfocus.mode == NotifyGrab || xe->xfocus.mode == NotifyUngrab) {
				break;
			}

			result = hero_event_queue_window_keyboard_focus_changed(((HeroWindowId){0}));
			if (result < 0) return result;
			break;
		};
		case ConfigureNotify: {
			//
			// see if the window has moved
			if (window->x != xe->xconfigure.x || window->y != xe->xconfigure.y) {
				result = hero_event_queue_window_moved(window, window_id, xe->xconfigure.x, xe->xconfigure.y);
				if (result < 0) return result;
			}

			//
			// see if the window has resized
			if (window->width != xe->xconfigure.width || window->height != xe->xconfigure.height) {
				result = hero_event_queue_window_resized(window, window_id, xe->xconfigure.width, xe->xconfigure.height);
				if (result < 0) return result;
			}
			break;
		};
		case DestroyNotify: {
			result = hero_event_queue_window_closed(window_id);
			if (result < 0) return result;
			break;
		};
		case KeyPress:
		case KeyRelease: {
			{
				// remove the control modifier, as it casues control codes to be returned
				xe->xkey.state &= ~ControlMask;

				char string[4] = {0};
				KeySym keysym = 0;
				U8 string_length = Xutf8LookupString(window->backend.x11.xic, &xe->xkey, string, sizeof(string), &keysym, NULL);

				// do not send any keys like ctrl, shift, function, arrow, escape, return, backspace.
				// instead, send regular key events.
				if (string_length && !(keysym >= 0xfd00 && keysym <= 0xffff)) {
					result = hero_event_queue_keyboard_key_input_utf32(window_id, string, string_length);
					if (result < 0) return result;
				}
			}

			// an X11 keycode is conceptually the same as our scan code.
			// they are both used to represent a physical key.
			// map evdev to our scan code
			unsigned int evdev_keycode = xe->xkey.keycode - 8;
			HeroKeyCode scan_code = hero_evdev_scancode_to_scan_code_map[evdev_keycode];
			result = hero_event_queue_keyboard_key_changed(window_id, xe->type == KeyPress, scan_code);
			if (result < 0) return result;
			break;
		};
		case MotionNotify: {
			result = hero_event_queue_mouse_moved(window_id, HERO_MAX(xe->xmotion.x, 0), HERO_MAX(xe->xmotion.y, 0));
			if (result < 0) return result;
			break;
		};
		case ButtonPress:
		case ButtonRelease: {
			if (xe->xbutton.button >= 4 && xe->xbutton.button <= 7) {
				if (xe->type == ButtonPress) {
					//
					// only process the scroll wheel on pressed.
					// in X11, the wheel is simulated as a button
					int32_t rel_x = 0;
					int32_t rel_y = 0;
					switch (xe->xbutton.button) {
						case 4: rel_y = 1; break;
						case 5: rel_y = -1; break;
						case 6: rel_x = 1; break;
						case 7: rel_x = -1; break;
						default: break;
					}
					result = hero_event_queue_mouse_wheel_moved(window_id, rel_x, rel_y);
					if (result < 0) return result;
				}
			} else {
				HeroMouseButtons btn = 0;
				switch (xe->xbutton.button) {
					case 1: btn = HERO_MOUSE_BUTTONS_LEFT; break;
					case 2: btn = HERO_MOUSE_BUTTONS_MIDDLE; break;
					case 3: btn = HERO_MOUSE_BUTTONS_RIGHT; break;
					case 8: btn = HERO_MOUSE_BUTTONS_BACK; break;
					case 9: btn = HERO_MOUSE_BUTTONS_FORWARD; break;
					default: break;
				}
				result = hero_event_queue_mouse_button_changed(window_id, xe->type == ButtonPress, btn);
				if (result < 0) return result;
			}
			break;
		};
		case MappingNotify: {
			if (xe->xmapping.request != MappingKeyboard) break;
			XRefreshKeyboardMapping(&xe->xmapping);
			hero_x11_update_keyboard_map();
			break;
		};
		case MapNotify: {
			break;
		};
		case UnmapNotify: {
			break;
		};
		case ReparentNotify: {
			break;
		};
		default:
			if (xe->type == hero_window_sys.backend.x11.xrandr_event_base + RRNotify) {
				XRRNotifyEvent* ne = (XRRNotifyEvent*)&xe;
				switch (ne->subtype) {
					case RRNotify_CrtcChange: {
						XRRCrtcChangeNotifyEvent* ce = (XRRCrtcChangeNotifyEvent*)ne;
						printf("crtc changed: crtc: %lu, mode: %lu, x: %d: y: %d: w: %d: h: %d\n", ce->crtc, ce->mode, ce->x, ce->y, ce->width, ce->height);
						break;
					};
					case RRNotify_OutputChange: {
						XRROutputChangeNotifyEvent* oe = (XRROutputChangeNotifyEvent*)ne;
						printf("output changed: output: %lu, crtc: %lu, mode: %lu\n", oe->output, oe->crtc, oe->mode);
						break;
					};
					default:
						printf("unhandled x11 xrandr event %d\n", ne->subtype);
						break;
				}
				break;
			} else {
				printf("unhandled x11 event %d\n", xe->type);
			}
			break;
	}

	return HERO_SUCCESS;
}

HeroResult hero_x11_events_poll(void) {
	XDisplay* connection = hero_window_sys.backend.x11.connection;
	XEvent xe;

	while (XPending(connection)) {
		XNextEvent(connection, &xe);

		HeroResult result = hero_x11_event_process(&xe);
		if (result < 0) return result;
	}

	return HERO_SUCCESS;
}

#endif // HERO_X11_ENABLE

// ===========================================
//
//
// Window System
//
//
// ===========================================

const char* hero_event_type_strings[HERO_EVENT_TYPE_COUNT] = {
	[HERO_EVENT_TYPE_WINDOW_MOVED] = "WINDOW_MOVED",
	[HERO_EVENT_TYPE_WINDOW_RESIZED] = "WINDOW_RESIZED",
	[HERO_EVENT_TYPE_WINDOW_CLOSE_REQUEST] = "WINDOW_CLOSE_REQUEST",
	[HERO_EVENT_TYPE_WINDOW_CLOSED] = "WINDOW_CLOSED",
	[HERO_EVENT_TYPE_KEYBOARD_ENTER] = "KEYBOARD_ENTER",
	[HERO_EVENT_TYPE_KEYBOARD_LEAVE] = "KEYBOARD_LEAVE",
	[HERO_EVENT_TYPE_KEYBOARD_KEY_UP] = "KEYBOARD_KEY_UP",
	[HERO_EVENT_TYPE_KEYBOARD_KEY_DOWN] = "KEYBOARD_KEY_DOWN",
	[HERO_EVENT_TYPE_KEYBOARD_KEY_INPUT_UTF32] = "KEYBOARD_KEY_INPUT_UTF32",
	[HERO_EVENT_TYPE_MOUSE_ENTER] = "MOUSE_ENTER",
	[HERO_EVENT_TYPE_MOUSE_LEAVE] = "MOUSE_LEAVE",
	[HERO_EVENT_TYPE_MOUSE_MOVED] = "MOUSE_MOVED",
	[HERO_EVENT_TYPE_MOUSE_BUTTON_UP] = "MOUSE_BUTTON_UP",
	[HERO_EVENT_TYPE_MOUSE_BUTTON_DOWN] = "MOUSE_BUTTON_DOWN",
	[HERO_EVENT_TYPE_MOUSE_WHEEL_MOVED] = "MOUSE_WHEEL_MOVED",
};

HeroWindowSys hero_window_sys;

HeroResult hero_window_sys_init(HeroWindowSysSetup* setup) {
	HeroResult result;

	hero_window_sys.alctor = setup->alctor;

	HeroIWindowSysVTable* vtable = &hero_window_sys.vtable;
	switch (setup->backend_type) {
#if HERO_X11_ENABLE
	case HERO_WINDOW_SYS_BACKEND_TYPE_X11:
		vtable->window_open_fn = hero_x11_window_open;
		vtable->window_close_fn = hero_x11_window_close;
		vtable->window_set_title_fn = hero_x11_window_set_title;
		vtable->events_poll_fn = hero_x11_events_poll;
		/*
		vtable->window_vulkan_create_surface_fn = hero_x11_window_vulkan_create_surface;
		vtable->vulkan_queue_has_presentation_support_fn = hero_x11_vulkan_queue_has_presentation_support;
		*/
		result = hero_x11_init();
		break;
#endif // HERO_X11_ENABLE
	}
	HERO_RESULT_ASSERT(result);
	hero_window_sys.backend_type = setup->backend_type;

	result = hero_object_pool(HeroWindow, init)(&hero_window_sys.windows_pool, setup->windows_cap, setup->alctor, HERO_ALLOC_TAG_WINDOW_WINDOWS_POOL);
	HERO_RESULT_ASSERT(result);

	return HERO_SUCCESS;
}

HeroResult hero_window_open(const char* title, U32 x, U32 y, U32 width, U32 height, HeroWindowId* id_out) {
	HERO_ASSERT(width && height, "a new window cannot have a width or height of zero");

	HeroWindow* window;
	HeroWindowId window_id;
	HeroResult result = hero_object_pool(HeroWindow, alloc)(&hero_window_sys.windows_pool, &window, &window_id);
	if (result < 0) {
		return result;
	}

	if (id_out) {
		*id_out = window_id;
	}

	window->x = x;
	window->y = y;
	window->width = width;
	window->height = height;

	result = hero_window_sys.vtable.window_open_fn(window, *id_out, title);
	if (result < 0) {
		return result;
	}

	return HERO_SUCCESS;
}

HeroResult hero_window_close(HeroWindowId id) {
	HeroWindow* window;
	HeroResult result = hero_object_pool(HeroWindow, get)(&hero_window_sys.windows_pool, id, &window);
	if (result < 0) return result;

	result = hero_window_sys.vtable.window_close_fn(window, id);
	if (result < 0) return result;

	result = hero_object_pool(HeroWindow, dealloc)(&hero_window_sys.windows_pool, id);
	if (result < 0) return result;

	return HERO_SUCCESS;
}

HeroResult hero_window_get(HeroWindowId id, HeroWindow** ptr_out) {
	return hero_object_pool(HeroWindow, get)(&hero_window_sys.windows_pool, id, ptr_out);
}

HeroResult hero_event_queue(HeroEvent** ptr_out) {
	return hero_stack(HeroEvent, push)(&hero_window_sys.event_queue, hero_system_alctor, HERO_ALLOC_TAG_WINDOW_EVENT_QUEUE, ptr_out);
}

HeroResult hero_events_poll(void) {
	hero_window_sys.event_queue.count = 0;
	return hero_window_sys.vtable.events_poll_fn();
}

HeroResult hero_events_iter_next(HeroEvent** ptr_out, U32* idx_mut) {
	U32 idx = *idx_mut;
	if (idx >= hero_window_sys.event_queue.count) {
		return HERO_SUCCESS_FINISHED;
	}

	*ptr_out = hero_stack(HeroEvent, get)(&hero_window_sys.event_queue, idx);
	*idx_mut += 1;
	return HERO_SUCCESS;
}

HeroResult hero_event_queue_window_keyboard_focus_changed(HeroWindowId window_id) {
	if (hero_window_sys.keyboard.focused_window_id.raw == window_id.raw) {
		return HERO_SUCCESS;
	}
	HeroEvent* e;

	//
	// if another window was focused, then queue an event for leaving
	if (hero_window_sys.keyboard.focused_window_id.raw) {
		HeroResult result = hero_event_queue(&e);
		if (result < 0) return result;
		e->any.type = HERO_EVENT_TYPE_KEYBOARD_LEAVE;
		e->any.window_id = hero_window_sys.keyboard.focused_window_id;
	}

	//
	// if we are focusing on another window of ours, then queue an event for entering
	hero_window_sys.keyboard.focused_window_id = window_id;
	if (window_id.raw) {
		HeroResult result = hero_event_queue(&e);
		if (result < 0) return result;
		e->any.type = HERO_EVENT_TYPE_KEYBOARD_ENTER;
		e->any.window_id = window_id;
	}

	return HERO_SUCCESS;
}

HeroResult hero_event_queue_keyboard_key_input_utf32(HeroWindowId window_id, const char* bytes, U8 size) {
	// if this is not the keyboard focused window_id, then don't track this key event
	if (hero_window_sys.keyboard.focused_window_id.raw != window_id.raw) {
		return HERO_SUCCESS;
	}

	HERO_DEBUG_ASSERT(size > 0, "size must be more than 0");
	HERO_DEBUG_ASSERT(
		size < sizeof(((HeroEvent*)0)->key_input_utf32.bytes),
		"input utf32 size of '%u' exceeds events event structure max length of '%lu'",
		size, sizeof(((HeroEvent*)0)->key_input_utf32.bytes));

	HeroEvent* e;
	HeroResult result = hero_event_queue(&e);
	if (result < 0) return result;
	e->any.type = HERO_EVENT_TYPE_KEYBOARD_KEY_INPUT_UTF32;
	e->any.window_id = window_id;
	memcpy(e->key_input_utf32.bytes, bytes, size);
	e->key_input_utf32.size = size;

	return HERO_SUCCESS;
}

HeroResult hero_event_queue_keyboard_key_changed(HeroWindowId window_id, bool is_pressed, HeroKeyCode scan_code) {
	// if this is not the keyboard focused window_id, then don't track this key event
	if (hero_window_sys.keyboard.focused_window_id.raw != window_id.raw) {
		return HERO_SUCCESS;
	}

	// the platform window system must only send out repeated presses and not any repeated releases.
	// in hero_x11_init we call XkbSetDetectableAutoRepeat to disable repeated KeyRelease events.
	// this allows us to simply check a repeat if the key is already pressed.
	bool is_repeat = is_pressed && hero_keyboard_scan_code_is_pressed(scan_code);

	//
	// get our key code from the scan code and see if it is a key modifier.
	HeroKeyCode key_code = hero_keyboard_scan_code_to_key_code(scan_code);
	HeroKeyMod key_mod = 0;
	switch (key_code) {
		case HERO_KEY_CODE_LEFT_CTRL:   key_mod = HERO_KEY_MOD_LEFT_CTRL; break;
		case HERO_KEY_CODE_RIGHT_CTRL:  key_mod = HERO_KEY_MOD_RIGHT_CTRL; break;
		case HERO_KEY_CODE_LEFT_SHIFT:  key_mod = HERO_KEY_MOD_LEFT_SHIFT; break;
		case HERO_KEY_CODE_RIGHT_SHIFT: key_mod = HERO_KEY_MOD_RIGHT_SHIFT; break;
		case HERO_KEY_CODE_LEFT_ALT:    key_mod = HERO_KEY_MOD_LEFT_ALT; break;
		case HERO_KEY_CODE_RIGHT_ALT:   key_mod = HERO_KEY_MOD_RIGHT_ALT; break;
		case HERO_KEY_CODE_LEFT_META:   key_mod = HERO_KEY_MOD_LEFT_META; break;
		case HERO_KEY_CODE_RIGHT_META:  key_mod = HERO_KEY_MOD_RIGHT_META; break;
		default: break;
	}

	//
	// if is_pressed -> mark the scan_code, key_code and key_mod as _pressed_ in the keyboard state
	// else ->  mark the scan_code, key_code and key_mod as _released_ in the keyboard state
	if (is_pressed) {
		hero_window_sys.keyboard.key_mod_is_pressed |= key_mod;
		hero_window_sys.keyboard.key_mod_has_been_pressed |= key_mod;
		hero_key_code_bitset_set(&hero_window_sys.keyboard.scan_code_is_pressed_bitset, scan_code);
		hero_key_code_bitset_set(&hero_window_sys.keyboard.scan_code_has_been_pressed_bitset, scan_code);
		hero_key_code_bitset_set(&hero_window_sys.keyboard.key_code_is_pressed_bitset, key_code);
		hero_key_code_bitset_set(&hero_window_sys.keyboard.key_code_has_been_pressed_bitset, key_code);
	} else {
		hero_window_sys.keyboard.key_mod_is_pressed &= ~key_mod;
		hero_window_sys.keyboard.key_mod_has_been_released |= key_mod;
		hero_key_code_bitset_unset(&hero_window_sys.keyboard.scan_code_is_pressed_bitset, scan_code);
		hero_key_code_bitset_set(&hero_window_sys.keyboard.scan_code_has_been_released_bitset, scan_code);
		hero_key_code_bitset_unset(&hero_window_sys.keyboard.key_code_is_pressed_bitset, key_code);
		hero_key_code_bitset_set(&hero_window_sys.keyboard.key_code_has_been_released_bitset, key_code);
	}

	HeroEvent* e;
	HeroResult result = hero_event_queue(&e);
	if (result < 0) return result;
	e->key.mod = hero_window_sys.keyboard.key_mod_is_pressed;
	e->any.type = is_pressed ? HERO_EVENT_TYPE_KEYBOARD_KEY_DOWN : HERO_EVENT_TYPE_KEYBOARD_KEY_UP;
	e->any.window_id = window_id;
	e->key.is_pressed = is_pressed;
	e->key.is_repeat = is_repeat;
	e->key.scan_code = scan_code;
	e->key.code = key_code;

	return HERO_SUCCESS;
}

HeroResult hero_event_queue_window_mouse_focus_changed(HeroWindowId window_id) {
	if (hero_window_sys.mouse.focused_window_id.raw == window_id.raw) {
		return HERO_SUCCESS;
	}
	HeroEvent* e;

	//
	// if another window was focused, then queue an event for leaving
	if (hero_window_sys.mouse.focused_window_id.raw) {
		HeroResult result = hero_event_queue(&e);
		if (result < 0) return result;
		e->any.type = HERO_EVENT_TYPE_MOUSE_LEAVE;
		e->any.window_id = hero_window_sys.mouse.focused_window_id;
	}

	//
	// if we are focusing on another window of ours, then queue an event for entering
	hero_window_sys.mouse.focused_window_id = window_id;
	if (window_id.raw) {
		HeroResult result = hero_event_queue(&e);
		if (result < 0) return result;
		e->any.type = HERO_EVENT_TYPE_MOUSE_ENTER;
		e->any.window_id = window_id;
	}

	return HERO_SUCCESS;
}

HeroResult hero_event_queue_mouse_moved(HeroWindowId window_id, U32 x, U32 y) {
	// if this is not the mouse focused window_id, then don't track this mouse event
	if (hero_window_sys.mouse.focused_window_id.raw != window_id.raw) {
		return HERO_SUCCESS;
	}

	//
	// queue the mouse moved event
	HeroEvent* e;
	HeroResult result = hero_event_queue(&e);
	if (result < 0) return result;
	e->any.type = HERO_EVENT_TYPE_MOUSE_MOVED;
	e->any.window_id = window_id;
	e->moved.x = x;
	e->moved.y = y;
	e->moved.rel_x = (S32)x - (S32)hero_window_sys.mouse.x;
	e->moved.rel_y = (S32)y - (S32)hero_window_sys.mouse.y;

	//
	// update the mouse state with the new location
	hero_window_sys.mouse.x = x;
	hero_window_sys.mouse.y = y;
	hero_window_sys.mouse.rel_x += e->moved.rel_x;
	hero_window_sys.mouse.rel_y += e->moved.rel_y;

	return HERO_SUCCESS;
}

HeroResult hero_event_queue_mouse_button_changed(HeroWindowId window_id, bool is_pressed, HeroMouseButtons button) {
	// if this is not the mouse focused window_id, then don't track this mouse event
	if (hero_window_sys.mouse.focused_window_id.raw != window_id.raw) {
		return HERO_SUCCESS;
	}

	//
	// queue the mouse button event
	HeroEvent* e;
	HeroResult result = hero_event_queue(&e);
	if (result < 0) return result;
	e->any.type = is_pressed ? HERO_EVENT_TYPE_MOUSE_BUTTON_DOWN : HERO_EVENT_TYPE_MOUSE_BUTTON_UP;
	e->any.window_id = window_id;
	e->mouse_button.is_pressed = is_pressed;
	e->mouse_button.button = button;

	//
	// update the mouse button state when changed between pressed or released
	if (is_pressed) {
		hero_window_sys.mouse.buttons_is_pressed |= button;
		hero_window_sys.mouse.buttons_has_been_pressed |= button;
	} else {
		hero_window_sys.mouse.buttons_is_pressed &= ~button;
		hero_window_sys.mouse.buttons_has_been_released |= button;
	}

	return HERO_SUCCESS;
}

HeroResult hero_event_queue_mouse_wheel_moved(HeroWindowId window_id, int32_t rel_x, int32_t rel_y) {
	// if this is not the mouse focused window_id, then don't track this mouse event
	if (hero_window_sys.mouse.focused_window_id.raw != window_id.raw) {
		return HERO_SUCCESS;
	}

	//
	// queue the mouse wheel moved event
	HeroEvent* e;
	HeroResult result = hero_event_queue(&e);
	if (result < 0) return result;
	e->any.type = HERO_EVENT_TYPE_MOUSE_WHEEL_MOVED;
	e->any.window_id = window_id;
	e->mouse_wheel_moved.rel_x = rel_x;
	e->mouse_wheel_moved.rel_y = rel_y;

	//
	// update the mouse wheel state
	hero_window_sys.mouse.rel_wheel_x += rel_x;
	hero_window_sys.mouse.rel_wheel_y += rel_y;

	return HERO_SUCCESS;
}

HeroResult hero_event_queue_window_moved(HeroWindow* window, HeroWindowId window_id, U32 x, U32 y) {
	HeroEvent* e;
	HeroResult result = hero_event_queue(&e);
	if (result < 0) return result;
	e->any.type = HERO_EVENT_TYPE_WINDOW_MOVED;
	e->any.window_id = window_id;
	e->moved.x = x;
	e->moved.y = y;
	e->moved.rel_x = (S32)x - (S32)window->x;
	e->moved.rel_y = (S32)y - (S32)window->y;

	window->x = x;
	window->y = y;

	return HERO_SUCCESS;
}

HeroResult hero_event_queue_window_resized(HeroWindow* window, HeroWindowId window_id, U32 width, U32 height) {
	HeroEvent* e;
	HeroResult result = hero_event_queue(&e);
	if (result < 0) return result;
	e->any.type = HERO_EVENT_TYPE_WINDOW_RESIZED;
	e->any.window_id = window_id;
	e->resized.width = width;
	e->resized.height = height;
	e->resized.rel_width = (S32)width - (S32)window->width;
	e->resized.rel_height = (S32)height - (S32)window->height;

	window->width = width;
	window->height = height;

	return HERO_SUCCESS;
}

HeroResult hero_event_queue_window_close_requested(HeroWindowId window_id) {
	HeroEvent* e;
	HeroResult result = hero_event_queue(&e);
	if (result < 0) return result;
	e->any.type = HERO_EVENT_TYPE_WINDOW_CLOSE_REQUEST;
	e->any.window_id = window_id;

	return HERO_SUCCESS;
}

HeroResult hero_event_queue_window_closed(HeroWindowId window_id) {
	HeroEvent* e;
	HeroResult result = hero_event_queue(&e);
	if (result < 0) return result;
	e->any.type = HERO_EVENT_TYPE_WINDOW_CLOSED;
	e->any.window_id = window_id;

	return HERO_SUCCESS;
}

void hero_keyboard_map_reset_default(void) {
	//
	// set the key codes to map directly to the scan codes
	for (int kc = 0; kc < HERO_KEY_CODE_COUNT; kc += 1) {
		hero_window_sys.keyboard.scan_code_to_key_code[kc] = kc;
		hero_window_sys.keyboard.key_code_to_scan_code[kc] = kc;
	}
}

bool hero_keyboard_scan_code_to_key_code(HeroScanCode scan_code) {
	return hero_window_sys.keyboard.scan_code_to_key_code[scan_code];
}

bool hero_keyboard_scan_code_is_pressed(HeroScanCode scan_code) {
	return hero_key_code_bitset_is_set(&hero_window_sys.keyboard.scan_code_is_pressed_bitset, scan_code);
}

bool hero_keyboard_scan_code_has_been_pressed(HeroScanCode scan_code) {
	return hero_key_code_bitset_is_set(&hero_window_sys.keyboard.scan_code_has_been_pressed_bitset, scan_code);
}

bool hero_keyboard_scan_code_has_been_released(HeroScanCode scan_code) {
	return hero_key_code_bitset_is_set(&hero_window_sys.keyboard.scan_code_has_been_released_bitset, scan_code);
}

bool hero_keyboard_key_code_to_scan_code(HeroKeyCode key_code) {
	return hero_window_sys.keyboard.key_code_to_scan_code[key_code];
}

bool hero_keyboard_key_code_is_pressed(HeroKeyCode key_code) {
	return hero_key_code_bitset_is_set(&hero_window_sys.keyboard.key_code_is_pressed_bitset, key_code);
}

bool hero_keyboard_key_code_has_been_pressed(HeroKeyCode key_code) {
	return hero_key_code_bitset_is_set(&hero_window_sys.keyboard.key_code_has_been_pressed_bitset, key_code);
}

bool hero_keyboard_key_code_has_been_released(HeroKeyCode key_code) {
	return hero_key_code_bitset_is_set(&hero_window_sys.keyboard.key_code_has_been_released_bitset, key_code);
}
