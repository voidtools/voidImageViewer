
// Copyright (C) 2017 David Carpenter
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
// 
// See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// VoidImageViewer

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

typedef unsigned char utf8_t;
typedef unsigned __int64 QWORD;

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
#include <Shellapi.h> // ShellExecute
#include <uxtheme.h>
#include <process.h> // _beginthreadex
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

	VIV_ID_VIEW_PANSCAN_INCREASE_SIZE,
	VIV_ID_VIEW_PANSCAN_DECREASE_SIZE,
	VIV_ID_VIEW_PANSCAN_INCREASE_WIDTH,
	VIV_ID_VIEW_PANSCAN_DECREASE_WIDTH,
	VIV_ID_VIEW_PANSCAN_INCREASE_HEIGHT,
	VIV_ID_VIEW_PANSCAN_DECREASE_HEIGHT,
	VIV_ID_VIEW_PANSCAN_MOVE_UP,
	VIV_ID_VIEW_PANSCAN_MOVE_DOWN,
	VIV_ID_VIEW_PANSCAN_MOVE_LEFT,
	VIV_ID_VIEW_PANSCAN_MOVE_RIGHT,
	VIV_ID_VIEW_PANSCAN_MOVE_UP_LEFT,
	VIV_ID_VIEW_PANSCAN_MOVE_UP_RIGHT,
	VIV_ID_VIEW_PANSCAN_MOVE_DOWN_LEFT,
	VIV_ID_VIEW_PANSCAN_MOVE_DOWN_RIGHT,
	VIV_ID_VIEW_PANSCAN_MOVE_CENTER,
	VIV_ID_VIEW_PANSCAN_RESET,

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
	VIV_ID_NAV_SORT_FULL_PATH_AND_FILENAME,
	VIV_ID_NAV_SORT_ASCENDING,
	VIV_ID_NAV_SORT_DESCENDING,
	
	VIV_ID_HELP_HELP,
	VIV_ID_HELP_COMMAND_LINE_OPTIONS,
	VIV_ID_HELP_WEBSITE,
	VIV_ID_HELP_DONATE,
	VIV_ID_HELP_ABOUT,
	
	VIV_ID_FILE_EXIT,
	
	VIV_ID_SLIDESHOW_TIMER,
	VIV_ID_HIDE_CURSOR_TIMER,
	VIV_ID_STATUS_TEMP_TEXT_TIMER,
	VIV_ID_ANIMATION_TIMER,

	VIV_ID_STATUS,
	VIV_ID_REBAR,
	VIV_ID_TOOLBAR,
	
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
};

#include "version.h"
#include "debug.h"
#include "mem.h"
#include "os.h"
#include "wchar.h"
#include "string.h"
#include "utf8.h"
#include "ini.h"
#include "config.h"
#include "webp.h"

#include "everything_ipc.h"

int viv_get_command_count(void);
int viv_menu_name_to_ini_name(utf8_t *buf,int command_index);
void viv_key_add(int command_index,DWORD keyflags);
void viv_key_clear_all(int command_index);
config_key_t *viv_key_get_start(int command_index);

#ifdef __cplusplus
}
#endif
