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
// viv_state.h - the shared context layer (R70 split, transition state).
// Types, macros and extern declarations shared by the viv domain modules
// and the viv.c core. Definitions live in viv.c / the domain .c files.
// See docs/architecture/viv-split-spec.md 2.3 for the transition plan.
#ifndef VIV_STATE_H
#define VIV_STATE_H

#include "viv.h"

// ---- shared macros ----
#define _VIV_WM_REPLY							(WM_USER+1)
#define _VIV_WM_RETRY_RANDOM_EVERYTHING_SEARCH	(WM_USER+2)

#define _VIV_ASSOCIATION_BMP				0x00000001
#define _VIV_ASSOCIATION_GIF				0x00000002
#define _VIV_ASSOCIATION_ICO				0x00000004
#define _VIV_ASSOCIATION_JPEG				0x00000008
#define _VIV_ASSOCIATION_JPG				0x00000010
#define _VIV_ASSOCIATION_PNG				0x00000020
#define _VIV_ASSOCIATION_TIF				0x00000040
#define _VIV_ASSOCIATION_TIFF				0x00000080
#define _VIV_ASSOCIATION_WEBP				0x00000100

#define _VIV_ZOOM_MAX 1024 // one ladder entry per 1% multiplicative zoom step. long enough that the 16x size cap is reachable even for photos much larger than the window.
#define _VIV_ZOOM_STEPS_PER_NOTCH 10 // zoom steps per wheel notch / zoom button click (~10.5%)
#define _VIV_ZOOM_SHRINK_STEPS 278 // the below-fit zoom-out range: 1.01^-278 is about one sixteenth of the best fit, mirroring the 16x native cap above it. the field report: pinch-out could never zoom below the best fit, so a fill-window upscale locked small images at a 200% minimum.

#define BCM_SETSHIELD	0x0000160C

#ifdef VERSION_X64
	#define VERSION_TARGET_MACHINE "(x64)"
#else
	#ifdef VERSION_ARM
		#define VERSION_TARGET_MACHINE "(ARM)"
	#else
		#ifdef VERSION_ARM64
			#define VERSION_TARGET_MACHINE "(ARM64)"
		#else
			#ifdef VERSION_X86
				#define VERSION_TARGET_MACHINE "(x86)"
			#else
				#error unknown target machine.
			#endif
		#endif
	#endif
#endif

#define _VIV_DEFAULT_SHUFFLE_ALLOCATED		(65536 / sizeof(_viv_playlist_t *))

#define VIV_YEAR_STRING2(x)	#x
#define VIV_YEAR_STRING(x)	VIV_YEAR_STRING2(x)	

#define _VIV_STRETCH_BLT_STITCH_SIZE		512

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED 0x031A
#endif

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

#ifndef TTM_SETTIPBKCOLOR
#define TTM_SETTIPBKCOLOR (WM_USER+19)
#endif

#ifndef TTM_SETTIPTEXTCOLOR
#define TTM_SETTIPTEXTCOLOR (WM_USER+20)
#endif

#ifndef TVM_SETBKCOLOR
#define TVM_SETBKCOLOR (TV_FIRST+29)
#endif

#ifndef TVM_SETTEXTCOLOR
#define TVM_SETTEXTCOLOR (TV_FIRST+30)
#endif

#define _VIV_HIDE_CURSOR_DELAY		2000
#define _VIV_RECENT_SAVE_DELAY		2000 // the deferred recent-files mru save: coalesces rapid opens so the ui thread never writes the ini mid-burst (the exit and endsession paths fold the pending write in).

#define _VIV_STATUS_PART_MAX 8

// R70: the five table counts are literals here, not sizeof measurements:
// msvc c resolves sizeof on an unsized extern to zero (warning c4034, and
// error c2229 when the zero sizes a struct member), so a header macro can
// never measure a table defined in another translation unit. each defining
// unit pins its literal to the real table with a c_assert - a table edit
// that forgets the count fails the build.
#define _VIV_ANIMATION_RATE_MAX	21
#define _VIV_ANIMATION_RATE_ONE	10

#define _VIV_SLIDESHOW_RATE_PRESET_COUNT	17

#define _VIV_OPTIONS_PAGE_COUNT	3

#define _VIV_COMMAND_COUNT	154

#define _VIV_ASSOCIATION_COUNT	11

#define _VIV_DIALOG_FONT_PROP L"VIV_DFONT"

#define _VIV_DARK_OWNERDRAW_PROP L"VIV_DARK_OD"

#define _VIV_DARK_BUTTON_THEME_PROP L"VIV_DARK_BT"

// ---- shared types ----
enum
{
	_VIV_COPYDATA_COMMAND_LINE,
	_VIV_COPYDATA_OPEN_EVERYTHING_SEARCH,
	_VIV_COPYDATA_ADD_EVERYTHING_SEARCH,
	_VIV_COPYDATA_RANDOM_EVERYTHING_SEARCH,
};

enum
{
	_VIV_DOING_NOTHING,
	_VIV_DOING_SCROLL,
	_VIV_DOING_MSCROLL,
	_VIV_DOING_1TO1SCROLL,
};

enum
{
	_VIV_REPLY_LOAD_IMAGE_COMPLETE = 0,
	_VIV_REPLY_LOAD_IMAGE_FAILED,
	_VIV_REPLY_LOAD_IMAGE_FIRST_FRAME,
	_VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME,
};

enum
{
	_VIV_MENU_ROOT=0,
	_VIV_MENU_FILE,
	_VIV_MENU_EDIT,
	_VIV_MENU_VIEW,
	_VIV_MENU_VIEW_PRESET,
	_VIV_MENU_VIEW_WINDOW_SIZE,
	_VIV_MENU_VIEW_LAYOUT,
	_VIV_MENU_VIEW_ZOOM,
	_VIV_MENU_VIEW_ONTOP,
	_VIV_MENU_VIEW_BACKDROP,
	_VIV_MENU_SLIDESHOW,
	_VIV_MENU_SLIDESHOW_RATE,
	_VIV_MENU_ANIMATION,
	_VIV_MENU_NAVIGATE,
	_VIV_MENU_NAVIGATE_SORT,
	_VIV_MENU_NAVIGATE_PLAYLIST,
	_VIV_MENU_HELP,
	_VIV_MENU_FILE_RECENT,
	_VIV_MENU_COUNT,
};

/*
typedef struct _viv_fd_s
{
	unsigned __int64 id; // unique playlist id.
	unsigned __int64 date_modified; 
	unsigned __int64 size;
	unsigned __int64 date_created;
	
	// filename follows.
	// utf8_t filename[...];
	
}_viv_fd_t;
*/
// a reply from the image load thread
typedef struct _viv_reply_s
{
	struct _viv_reply_s *next;
	DWORD type;
	DWORD size;
	// data follows
	
}_viv_reply_t;

// a mipmap level
typedef struct _viv_mip_s
{
//	int wide;
//	int high;
	HBITMAP hbitmap;
	struct _viv_mip_s *mipmap;
	
}_viv_mipmap_t;

// a frame in the image.
// there might be more than one.
typedef struct _viv_frame_s
{
	HBITMAP hbitmap;
	
	// next mipmap level.
	// NULL if not computed.
	_viv_mipmap_t *mipmap;
	
	// frame delay in milliseconds.
	DWORD delay;
	
}_viv_frame_t;

typedef struct _viv_reply_load_image_first_frame_s
{
	UINT wide;
	UINT high;
	UINT frame_count;
	_viv_frame_t frame;
	BYTE is_low_res; // 1 = progressive preview frame (embedded thumbnail), the full frame follows
	
}_viv_reply_load_image_first_frame_t;

typedef struct _viv_playlist_s
{
	struct _viv_playlist_s *next;
	struct _viv_playlist_s *prev;
	WIN32_FIND_DATA fd;
	
}_viv_playlist_t;

typedef struct _viv_command_s
{
	localization_id_t localization_id;
	WORD flags;
	BYTE menu_id;
	WORD command_id;
}_viv_command_t;

// the command table extern: the key-list type and every domain that walks
// the menus index the table, so the extern prints with the type it serves.
extern _viv_command_t _viv_commands[];

typedef struct _viv_nav_item_s
{
	WIN32_FIND_DATA fd;
	struct _viv_nav_item_s *next;
	
}_viv_nav_item_t;

typedef struct _viv_webp_s
{
	DWORD wide;
	DWORD high;
	HDC screen_hdc;
	HDC mem_hdc;
	int has_alpha;
	int frame_index;
	DWORD frame_count;
	DWORD last_delay;
	int orientation;
	
}_viv_webp_t;

typedef struct _viv_name_mapping_s
{
	UINT count;
	SHNAMEMAPPING *mappings;
}_viv_name_mapping_t;

typedef struct _viv_key_list_s
{
	config_key_t *start[_VIV_COMMAND_COUNT];
	config_key_t *last[_VIV_COMMAND_COUNT];
	
}_viv_key_list_t;

// ---- core (viv.c) functions exported to the domains ----
void _viv_exit(void);
void _viv_kill(void);
int __cdecl main(int argc,char **argv);
int _viv_icompare_w(const wchar_t *a,const wchar_t *b);
CLIPFORMAT _viv_get_CF_PREFERREDDROPEFFECT(void);
// the R76 wndproc domain split: the WM_COPYDATA handler forwards the
// command line here, the core init consumes it.
void _viv_process_command_line(wchar_t *cl);

// ---- shared state (definitions remain in viv.c) ----
extern HMODULE _viv_stobject_hmodule;
extern _viv_playlist_t *_viv_playlist_start;
extern int _viv_playlist_count;
extern _viv_playlist_t **_viv_playlist_shuffle_indexes;
extern int _viv_playlist_shuffle_allocated;
extern HWND _viv_hwnd;
extern HWND _viv_status_hwnd;
extern float _viv_animation_rates[];
extern int _viv_animation_rate_pos;
extern BYTE _viv_animation_play;
extern BYTE _viv_1to1;
extern BYTE _viv_have_old_zoom;
extern WIN32_FIND_DATA *_viv_current_fd;
extern WIN32_FIND_DATA *_viv_preload_fd;
extern int _viv_view_x;
extern int _viv_view_y;
extern double _viv_view_ix;
extern double _viv_view_iy;
extern int _viv_zoom_pos;
extern float _viv_zoom_scales[_VIV_ZOOM_MAX];
extern BYTE _viv_image_is_low_res;
extern int _viv_image_wide;
extern int _viv_image_high;
extern int _viv_frame_count;
extern int _viv_frame_loaded_count;
extern int _viv_frame_position;
extern BYTE _viv_frame_looped;
extern BYTE _viv_is_slideshow_timeup;
extern _viv_frame_t *_viv_frames;
extern VIV_UINT64 _viv_timer_tick;
extern BYTE _viv_is_animation_timer;
extern VIV_UINT64 _viv_animation_timer_tick_start;
extern BYTE _viv_doing;
extern int _viv_doing_x;
extern int _viv_doing_y;
extern BYTE _viv_fullscreen_is_maxed;
extern BYTE _viv_is_fullscreen;
extern BYTE _viv_is_slideshow;
extern BYTE _viv_is_hide_cursor_timer;
extern int _viv_mousemove_x;
extern int _viv_mousemove_y;
extern BYTE _viv_in_popup_menu;
extern HMENU _viv_hmenu;
extern int _viv_dst_pos_x;
extern int _viv_dst_pos_y;
extern CRITICAL_SECTION _viv_cs;
extern HANDLE _viv_load_image_thread;
extern BYTE _viv_load_is_preload;
extern wchar_t *_viv_load_image_filename;
extern WIN32_FIND_DATA *_viv_load_image_next_fd;
extern BYTE _viv_load_image_next_is_preload;
extern volatile int _viv_load_image_terminate;
extern _viv_reply_t *_viv_reply_start;
extern _viv_reply_t *_viv_reply_last;
extern wchar_t *_viv_status_temp_text;
extern HFONT _viv_about_hfont;
extern wchar_t *_viv_last_open_file;
extern wchar_t *_viv_last_open_folder;
extern _viv_nav_item_t **_viv_nav_items;
extern _viv_nav_item_t *__viv_nav_item_start;
extern int _viv_nav_item_count;
extern wchar_t *_viv_random;
extern DWORD _viv_random_tot_results;
extern BYTE _viv_is_animation_timer_event;
extern BYTE _viv_preload_state;
extern int _viv_preload_image_wide;
extern int _viv_preload_image_high;
extern int _viv_preload_frame_count;
extern int _viv_preload_frame_loaded_count;
extern _viv_frame_t *_viv_preload_frames;
extern BYTE _viv_last_is_prev;
extern BYTE _viv_should_activate_preload_on_load;
extern int _viv_load_render_wide;
extern int _viv_load_render_high;
extern WIN32_FIND_DATA *_viv_last_fd;
extern WIN32_FIND_DATA *_viv_frame_fd;
extern WIN32_FIND_DATA *_viv_load_fd;
extern int _viv_last_frame_count;
extern _viv_frame_t *_viv_last_frames;
extern BYTE _viv_load_image_allow_draw;
extern BYTE _viv_file_not_found;
extern BYTE _viv_load_failed;
extern BYTE _viv_is_tracking_mouse;
extern BYTE _viv_is_mouseover;
extern BYTE _viv_prevent_on_deactivate;
extern int _viv_load_frame_count;
extern int _viv_src_pixel_x;
extern int _viv_src_pixel_y;
extern BYTE _viv_src_pixel_r;
extern BYTE _viv_src_pixel_g;
extern BYTE _viv_src_pixel_b;
// the R70 missing-export fixups: three core states the domain modules read
// or write directly (the one-shot split left them static in the core; CI
// caught the undeclared identifiers in chrome and dialogs).
extern wchar_t _viv_status_part_text[_VIV_STATUS_PART_MAX][STRING_SIZE];
extern BYTE _viv_is_cursor_shown;
extern int _viv_options_page_ids[];

extern _viv_key_list_t *_viv_key_list;
extern const char *_viv_association_extensions[];
extern const localization_id_t _viv_association_description_localization_id_array[];
extern const char *_viv_association_icon_locations[];
extern int _viv_recent_save_dirty;
extern HDC _viv_paint_hdc;
// created lazily in the wndproc WM_PAINT handler, released in _viv_kill.
extern HBRUSH _viv_background_hbrush;
extern HBRUSH _viv_dialog_dark_hbrush;
extern HBRUSH _viv_dark_chrome_hbrushes[4];
extern HBRUSH _viv_about_light_hbrushes[2];
extern HBRUSH _viv_backdrop_solid_hbrush;
extern COLORREF _viv_backdrop_solid_color;
extern HBRUSH _viv_backdrop_checker_hbrush;
extern HBITMAP _viv_backdrop_checker_hbitmap;
extern int _viv_backdrop_checker_cell;

#endif
