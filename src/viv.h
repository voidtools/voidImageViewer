//
// Copyright 2025 voidtools / David Carpenter
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// VoidImageViewer

// include guard: upstream relied on each translation unit including this
// exactly once; the R70 split layers viv_state.h and the domain headers
// on top of it, so a plain guard is now required (double inclusion
// redefines the command-id enum).
#ifndef VIV_H
#define VIV_H

#ifdef __cplusplus
extern "C" {
#endif

// compiler options
#pragma warning(disable : 4311) // type cast void * to unsigned int
#pragma warning(disable : 4312) // type cast unsigned int to void *
#pragma warning(disable : 4244) // warning C4244: 'argument' : conversion from 'LONG_PTR' to 'LONG', possible loss of data
#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable : 4313) // 'debug_printf' : '%x' in format string conflicts with argument 2 of type 'line_t *'
#pragma warning(disable : 4996) // deprecation
#pragma warning(disable : 4701) // potentially uninitialized local variable 'last_stretch_mode' used

// REQUIRES IE 5.01
#define _WIN32_IE 0x0501

// WINNT required for some header definitions.
// minimum is really 0x0400
#define _WIN32_WINNT 0x0501

#define WIN32_LEAN_AND_MEAN

#define COBJMACROS // c object interface please
#define CINTERFACE // c interface only

#define VIV_UINT64_MAX	0xFFFFFFFFFFFFFFFFULL
#define VIV_DWORD_MAX	0xffffffff

// single image pixel budget: a corrupted or hostile header can
// claim dimensions that decode to gigabytes of rgba (a 428 kb png
// can ask for 20000 x 20000). the loaders refuse the file before
// any allocation happens so a huge canvas fails like any other
// unloadable file instead of dying inside the allocator.
//
// the ceiling is pointer-width dependent (1.0.04): 32-bit builds
// keep the 100 mp ceiling (a 100 mp rgba frame is already 400 mb
// against a 2 gb address space once gdi copies, mipmaps and
// rotation buffers pile up); 64-bit builds (x64 / arm64) can
// afford 400 mp so real panoramic stitches and large flatbed
// scans (15000 x 9000 and up) load normally instead of failing
// like a corrupt file. both ceilings still bound a hostile canvas
// before the decoder runs.
#if defined(_WIN64)
#define VIV_MAX_IMAGE_PIXELS	400000000
// animated webp keeps the static ceiling on 64-bit builds: the address
// space absorbs the decode canvas, the anim decoder working frame and
// the display bitmap.
#define VIV_MAX_ANIMATION_PIXELS	400000000
#else
#define VIV_MAX_IMAGE_PIXELS	100000000
// 32-bit: an animated canvas pays for the decode canvas, the anim
// decoder working frame and the display bitmap (roughly 12 bytes per
// pixel) inside a 2 gb address space, so the animation ceiling is a
// quarter of the static one (25 mp ~= 300 mb of buffers).
#define VIV_MAX_ANIMATION_PIXELS	25000000
#endif

typedef unsigned char utf8_t;

// __int64 is msvc only: mingw and zig cc spell it long long.
#ifndef _MSC_VER
#define __int64 long long
#endif

typedef unsigned __int64 VIV_UINT64;

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
//#include <gdiplus.h>
#include "../res/resource.h"
//#include <stdio.h>
//#include <math.h>
//#include <shlobj.h>
//#include <istream>
#include <commdlg.h> // OPENFILENAME
#include <shellapi.h> // ShellExecute
#include <uxtheme.h>
//#include <process.h> // _beginthreadex
#include <shlobj.h> // DROPFILES

enum
{
	VIV_ID_EDIT_CUT = 1000,
	VIV_ID_EDIT_COPY,
	VIV_ID_EDIT_PASTE,
	VIV_ID_FILE_DELETE,
	VIV_ID_FILE_EDIT,
	VIV_ID_FILE_PREVIEW,
	VIV_ID_FILE_PRINT,
	VIV_ID_FILE_SET_DESKTOP_WALLPAPER,
	VIV_ID_FILE_CLOSE,
	VIV_ID_FILE_OPEN_FILE_LOCATION,
	VIV_ID_FILE_PROPERTIES,
	VIV_ID_EDIT_COPY_TO,
	VIV_ID_EDIT_MOVE_TO,
	
	VIV_ID_VIEW_MENU,
	VIV_ID_VIEW_STATUS,
	VIV_ID_VIEW_CONTROLS,
	VIV_ID_VIEW_PRESET_1,
	VIV_ID_VIEW_PRESET_2,
	VIV_ID_VIEW_PRESET_3,
	VIV_ID_VIEW_ALLOW_SHRINKING,
	VIV_ID_VIEW_KEEP_ASPECT_RATIO,
	VIV_ID_VIEW_FILL_WINDOW,
	VIV_ID_VIEW_1TO1,
	VIV_ID_VIEW_BESTFIT,
	VIV_ID_VIEW_FULLSCREEN,
	VIV_ID_VIEW_SLIDESHOW,
	VIV_ID_VIEW_ONTOP_ALWAYS,
	VIV_ID_VIEW_ONTOP_WHILE_PLAYING_OR_ANIMATING,
	VIV_ID_VIEW_ONTOP_NEVER,
	VIV_ID_VIEW_OPTIONS,

	VIV_ID_VIEW_WINDOW_SIZE_50,
	VIV_ID_VIEW_WINDOW_SIZE_100,
	VIV_ID_VIEW_WINDOW_SIZE_200,
	VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT,

	VIV_ID_VIEW_ZOOM_IN,
	VIV_ID_VIEW_ZOOM_OUT,
	VIV_ID_VIEW_ZOOM_RESET,

	VIV_ID_SLIDESHOW_PAUSE,
	VIV_ID_SLIDESHOW_PLAY_ONLY,
	VIV_ID_SLIDESHOW_PAUSE_ONLY,
	VIV_ID_SLIDESHOW_STOP,
	VIV_ID_SLIDESHOW_RATE_DEC,
	VIV_ID_SLIDESHOW_RATE_INC,
	VIV_ID_SLIDESHOW_RATE_250,
	VIV_ID_SLIDESHOW_RATE_500,
	VIV_ID_SLIDESHOW_RATE_1000,
	VIV_ID_SLIDESHOW_RATE_2000,
	VIV_ID_SLIDESHOW_RATE_3000,
	VIV_ID_SLIDESHOW_RATE_4000,
	VIV_ID_SLIDESHOW_RATE_5000,
	VIV_ID_SLIDESHOW_RATE_6000,
	VIV_ID_SLIDESHOW_RATE_7000,
	VIV_ID_SLIDESHOW_RATE_8000,
	VIV_ID_SLIDESHOW_RATE_9000,
	VIV_ID_SLIDESHOW_RATE_10000,
	VIV_ID_SLIDESHOW_RATE_20000,
	VIV_ID_SLIDESHOW_RATE_30000,
	VIV_ID_SLIDESHOW_RATE_40000,
	VIV_ID_SLIDESHOW_RATE_50000,
	VIV_ID_SLIDESHOW_RATE_60000,
	VIV_ID_SLIDESHOW_RATE_CUSTOM,

	VIV_ID_ANIMATION_PLAY_PAUSE,
	VIV_ID_ANIMATION_JUMP_FORWARD_MEDIUM,
	VIV_ID_ANIMATION_JUMP_BACKWARD_MEDIUM,
	VIV_ID_ANIMATION_JUMP_FORWARD_SHORT,
	VIV_ID_ANIMATION_JUMP_BACKWARD_SHORT,
	VIV_ID_ANIMATION_JUMP_FORWARD_LONG,
	VIV_ID_ANIMATION_JUMP_BACKWARD_LONG,
	VIV_ID_ANIMATION_FRAME_HOME,
	VIV_ID_ANIMATION_FRAME_END,
	VIV_ID_ANIMATION_FRAME_STEP,
	VIV_ID_ANIMATION_FRAME_PREV,
	VIV_ID_ANIMATION_RATE_DEC,
	VIV_ID_ANIMATION_RATE_INC,
	VIV_ID_ANIMATION_RATE_RESET,

	VIV_ID_NAV_PREV,
	VIV_ID_NAV_NEXT,
	VIV_ID_NAV_HOME,
	VIV_ID_NAV_END,
	VIV_ID_NAV_SORT_NAME,
	VIV_ID_NAV_SORT_SIZE,
	VIV_ID_NAV_SORT_DATE_MODIFIED,
	VIV_ID_NAV_SORT_DATE_CREATED,
	VIV_ID_NAV_SORT_FULL_PATH,
	VIV_ID_NAV_SORT_ASCENDING,
	VIV_ID_NAV_SORT_DESCENDING,
	
	VIV_ID_HELP_HELP,
	VIV_ID_HELP_COMMAND_LINE_OPTIONS,
	VIV_ID_HELP_WEBSITE,
	VIV_ID_HELP_DONATE,
	VIV_ID_HELP_ABOUT,
	
	VIV_ID_FILE_EXIT,
	VIV_ID_FILE_SAVE_AS,
	
	VIV_ID_SLIDESHOW_TIMER,
	VIV_ID_HIDE_CURSOR_TIMER,
	VIV_ID_STATUS_TEMP_TEXT_TIMER,
	VIV_ID_ANIMATION_TIMER,
	VIV_ID_DARK_RECHECK_TIMER,
	VIV_ID_DARK_DIALOG_ASSERT_TIMER,
	VIV_ID_RECENT_SAVE_TIMER,

	VIV_ID_STATUS,
	VIV_ID_REBAR,
	VIV_ID_TOOLBAR,
	VIV_ID_MENUBAR,
	
	VIV_ID_OPTIONS_GENERAL,
	VIV_ID_OPTIONS_VIEW,
	VIV_ID_OPTIONS_CONTROLS,

	VIV_ID_FILE_OPEN_FILE,
	VIV_ID_FILE_OPEN_FOLDER,
	VIV_ID_FILE_ADD_FILE,
	VIV_ID_FILE_ADD_FOLDER,
	VIV_ID_FILE_RENAME,
	
	VIV_ID_NAV_JUMPTO,
	VIV_ID_NAV_SHUFFLE,
	VIV_ID_FILE_OPEN_EVERYTHING_SEARCH,
	VIV_ID_FILE_ADD_EVERYTHING_SEARCH,
	VIV_ID_EDIT_ROTATE_90,
	VIV_ID_EDIT_ROTATE_270,
	VIV_ID_FILE_DELETE_RECYCLE,
	VIV_ID_FILE_DELETE_PERMANENTLY,

	VIV_ID_EDIT_COPY_FILENAME,
	VIV_ID_EDIT_COPY_IMAGE,
	VIV_ID_VIEW_REFRESH,
	VIV_ID_VIEW_CAPTION,
	VIV_ID_VIEW_THICKFRAME,

	// touch / zoom controls (appended so existing ids are not shifted)
	VIV_ID_VIEW_ZOOM_CONTROLS,

	// backdrop shown under transparent pixels (appended, ids not shifted)
	VIV_ID_VIEW_BACKDROP_FOLLOW,
	VIV_ID_VIEW_BACKDROP_BLACK,
	VIV_ID_VIEW_BACKDROP_WHITE,
	VIV_ID_VIEW_BACKDROP_CUSTOM,
	VIV_ID_VIEW_BACKDROP_CHECKERBOARD,
	VIV_ID_ZOOMUI,

	// zoom overlay auto hide (appended so existing ids are not shifted)
	VIV_ID_VIEW_ZOOM_AUTO_HIDE,

	// recent files mru submenu items and the clear command
	// (appended so existing ids are not shifted)
	VIV_ID_FILE_RECENT_CLEAR,
	VIV_ID_FILE_RECENT_0,
	VIV_ID_FILE_RECENT_1,
	VIV_ID_FILE_RECENT_2,
	VIV_ID_FILE_RECENT_3,
	VIV_ID_FILE_RECENT_4,
	VIV_ID_FILE_RECENT_5,
	VIV_ID_FILE_RECENT_6,
	VIV_ID_FILE_RECENT_7,
	VIV_ID_FILE_RECENT_8,
	VIV_ID_FILE_RECENT_9,

	// view menu windowed background color picker
	// (appended so existing ids are not shifted)
	VIV_ID_VIEW_WINDOWED_BACKGROUND_COLOR,
};

#include "version.h"
#include "debug.h"
#include "mem.h"
#include "localization.h"
#include "os.h"
#include "wchar.h"
#include "string.h"
#include "utf8.h"
#include "ini.h"
#include "config.h"
#include "viv_theme.h"
#include "viv_msgbox.h"
#include "zoomui.h"
#include "viv_toolbar.h"
#include "viv_settings.h"
#include "glyphs.h"
#include "webp.h"
#include "small_pool.h"
#include "safe_size.h"
#include "everything_ipc.h"

int viv_get_command_count(void);
int viv_menu_name_to_ini_name(utf8_t *buf,int command_index);
void viv_key_add(int command_index,DWORD keyflags);
void viv_key_clear_all(int command_index);
config_key_t *viv_key_get_start(int command_index);

#ifdef __cplusplus
}
#endif

#endif // VIV_H
