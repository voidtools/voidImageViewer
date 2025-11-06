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
// OpenGL renderer
// abandoned due to stitching texture concerns

#if 0

#define VIV_SHRINK_BLIT_MODE_COLORONCOLOR	0
#define VIV_SHRINK_BLIT_MODE_HALFTONE		1
#define VIV_MAG_FILTER_COLORONCOLOR			0
#define VIV_MAG_FILTER_HALFTONE				1

#define VIV_WM_TIMER						(WM_USER)
#define VIV_WM_REPLY						(WM_USER+1)

// compiler options
#pragma warning(disable : 4311) // type cast void * to unsigned int
#pragma warning(disable : 4312) // type cast unsigned int to void *
#pragma warning(disable : 4244) // warning C4244: 'argument' : conversion from 'LONG_PTR' to 'LONG', possible loss of data
#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable : 4313) // 'printf' : '%x' in format string conflicts with argument 2 of type 'line_t *'
#pragma warning(disable : 4996) // deprecation
#pragma warning(disable : 4701) // potentially uninitialized local variable 'last_stretch_mode' used
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4101) // unreferenced local variable

// REQUIRES IE 5.01
#define _WIN32_IE 0x0501

// WINNT required for some header definitions.
// minimum is really 0x0400
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <gdiplus.h>
#include "../resource.h"
#include <stdio.h>
#include <math.h>
#include <shlobj.h>
#include <istream>
#include <uxtheme.h>
#include "../directx/d3d9.h"
#include <process.h>
#include <gl/gl.h>

enum
{
	VIV_RENDERER_GDI = 0,
	VIV_RENDERER_OPENGL,
};

enum
{
	VIV_REPLY_LOAD_IMAGE_COMPLETE = 0,
	VIV_REPLY_LOAD_IMAGE_FIRST_FRAME,
	VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME,
};

enum
{
	VIV_NAV_SORT_NAME=0,
	VIV_NAV_SORT_SIZE,
	VIV_NAV_SORT_DATE_MODIFIED,
	VIV_NAV_SORT_DATE_CREATED,
};

enum
{
	VIV_ID_FILE_CUT = 1000,
	VIV_ID_FILE_COPY,
	VIV_ID_FILE_DELETE,
	VIV_ID_FILE_EDIT,
	VIV_ID_FILE_PREVIEW,
	VIV_ID_FILE_PRINT,
	VIV_ID_FILE_SET_DESKTOP_WALLPAPER,
	VIV_ID_FILE_OPEN_FILE_LOCATION,
	VIV_ID_FILE_PROPERTIES,
	
	VIV_ID_VIEW_MENU,
	VIV_ID_VIEW_STATUS,
	VIV_ID_VIEW_PRESET_1,
	VIV_ID_VIEW_PRESET_2,
	VIV_ID_VIEW_PRESET_3,
	VIV_ID_VIEW_ALLOW_SHRINKING,
	VIV_ID_VIEW_KEEP_ASPECT_RATIO,
	VIV_ID_VIEW_FILL_WINDOW,
	VIV_ID_VIEW_1TO1,
	VIV_ID_VIEW_FULLSCREEN,
	VIV_ID_VIEW_SLIDE_SHOW,
	VIV_ID_VIEW_ONTOP_ALWAYS,
	VIV_ID_VIEW_ONTOP_WHILE_PLAYING_OR_ANIMATING,
	VIV_ID_VIEW_ONTOP_NEVER,
	VIV_ID_VIEW_OPTIONS,

	VIV_ID_VIEW_WINDOW_SIZE_50,
	VIV_ID_VIEW_WINDOW_SIZE_100,
	VIV_ID_VIEW_WINDOW_SIZE_200,
	VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT,

	VIV_ID_SLIDESHOW_PAUSE,
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
	VIV_ID_NAV_SORT_ASCENDING,
	VIV_ID_NAV_SORT_DESCENDING,
	
	VIV_ID_FILE_EXIT,
	
	VIV_ID_SLIDE_SHOW_TIMER,
	VIV_ID_HIDE_CURSOR_TIMER,
	VIV_ID_UPDATE_FRAME_TIMER,

	VIV_ID_STATUS,
};

#define VIV_ID_NAV_PLAYLIST_START	10000
#define VIV_ID_NAV_PLAYLIST_END		20000 // exclusive.

#define VIV_HIDE_CURSOR_DELAY	2000

#define OAIF_HIDE_REGISTRATION (0x00000020)
#define OAIF_EXEC (0x00000004)

using namespace Gdiplus;

typedef unsigned char utf8_t;

typedef struct viv_reply_s
{
	struct viv_reply_s *next;
	DWORD type;
	DWORD size;
	// data follows
	
}viv_reply_t;

typedef struct viv_reply_load_image_first_frame_s
{
	DWORD wide;
	DWORD high;
	DWORD frame_count;
	HBITMAP hbitmap;
	DWORD *frame_delays;
	
}viv_reply_load_image_first_frame_t;

typedef struct viv_fileinfo_s
{
	unsigned __int64 size;
	unsigned __int64 date_modified;
	unsigned __int64 date_created;
	
}viv_fileinfo_t;

typedef struct viv_nav_item_s
{
	WIN32_FIND_DATA fd;
	struct viv_nav_item_s *next;
	
}viv_nav_item_t;

typedef struct viv_dragdrop_s
{
	struct viv_dragdrop_s *next;
	struct viv_dragdrop_s *prev;
	viv_fileinfo_t fileinfo;
	wchar_t *filename;
	
}viv_dragdrop_t;

typedef struct tagOPENASINFO 
{
	LPCWSTR pcszFile;
	LPCWSTR pcszClass;
	int	oaifInFlags;
	
}OPENASINFO;

typedef struct viv_opengl_proc_s
{
	FARPROC *proc;
	const char *name;
	
}viv_opengl_proc_t;

static void os_zero_memory(void *data,int size);
static void os_copy_memory(void *d,void *s,int size);
static int string_length(const char *text);
static int wstring_length(const wchar_t *text);
static void *mem_alloc(int size);
static void mem_free(void *ptr);
static void viv_update_title(void);
static void viv_on_size(void);
static LRESULT CALLBACK viv_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
static void viv_command(int command);
static void viv_load_settings(void);
static int viv_is_ws(wchar_t c);
static wchar_t *viv_skip_ws(wchar_t *p);
static wchar_t *viv_get_word(wchar_t *p,wchar_t *buf);
static void viv_process_command_line(wchar_t *cl);
static int viv_save_settings_by_location(const wchar_t *path,int is_root);
static void viv_save_settings(int appdata);
static int viv_init(void);
static void viv_kill(void);
static void os_make_rect_completely_visible(RECT *prect);
static void os_MonitorRectFromWindowRect(const RECT *window_rect,RECT *monitor_rect);
static void viv_exit(void);
static wchar_t *string_alloc(const wchar_t *s);
static void viv_next(int prev);
static void viv_home(int end);
static int viv_is_valid_filename(wchar_t *s);
static int viv_is_msg(MSG *msg);
static void viv_view_set(int view_x,int view_y);
static void viv_slide_show(void);
static void viv_toggle_fullscreen(void);
static void viv_get_render_size(int *prw,int *prh);
static void viv_set_custom_rate(void);
static void viv_set_rate(int rate);
static void viv_delete(void);
static void viv_copy(int cut);
static int viv_is_key_state(int control,int shift,int alt);
static CLIPFORMAT _viv_get_CF_PREFERREDDROPEFFECT(void);
static void viv_pause(void);
static void viv_write_int(HANDLE h,const utf8_t *key,int value);
static void viv_write_utf8(HANDLE h,const unsigned char *s);
static int _viv_compare(WIN32_FIND_DATA *a,WIN32_FIND_DATA *b);
static void viv_open(WIN32_FIND_DATA *fd);
static void viv_open_from_full_filename(const wchar_t *filename);
static void viv_increase_rate(int dec);
static void viv_file_preview(void);
static void viv_file_print(void);
static void viv_file_set_desktop_wallpaper(void);
static void viv_file_edit(void);
static void viv_open_file_location(void);
static void viv_properties(void);
static void viv_doing_cancel(void);
static const char *_viv_get_copydata_string(const char *p,const char *e,wchar_t *buf,int max);
static void os_remove_all_menu_items(HMENU hmenu);
static void viv_nav_item_free_all(void);
static int __cdecl viv_nav_playlist_compare(const void *a,const void *b);
static void viv_blank(void);
static void viv_options(void);
static INT_PTR CALLBACK viv_options_proc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK viv_options_general_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK viv_options_view_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void viv_install_association(const wchar_t *association,const wchar_t *description,const wchar_t *icon_location);
static void viv_uninstall_association(const wchar_t *association);
static int viv_is_association(const wchar_t *association);
static int viv_get_registry_string(HKEY hkey,const wchar_t *value,wchar_t *wbuf,int size_in_wchars);
static int viv_set_registry_string(HKEY hkey,const wchar_t *value,const wchar_t *wbuf);
static void wstring_copy(wchar_t *d,int size,const wchar_t *s);
static void wstring_cat(wchar_t *buf,int size,const wchar_t *s);
static int wstring_compare(const wchar_t *s1,const wchar_t *s2);
static void viv_check_menus(HMENU hmenu);
static void viv_increase_animation_rate(int dec);
static void viv_reset_animation_rate(void);
static void viv_timer_start(void);
static void viv_show_cursor(void);
static void viv_animation_pause(void);
static void viv_frame_step(void);
static void viv_frame_prev(void);
static void viv_view_1to1(void);
static void viv_dragdrop_clearall(void);
static viv_dragdrop_t *viv_dragdrop_add(const wchar_t *filename,viv_fileinfo_t *fileinfo);
static void viv_dragdrop_add_path(const wchar_t *filename);
static void viv_dragdrop_add_filename(const wchar_t *filename);
static int wstring_to_int(wchar_t *s);
static void viv_fileinfo_copy(viv_fileinfo_t *dst,viv_fileinfo_t *src);
static void viv_fileinfo_copy_fd(viv_fileinfo_t *fileinfo,WIN32_FIND_DATA *fd);
static void viv_fd_copy_fileinfo(WIN32_FIND_DATA *fd,viv_fileinfo_t *fileinfo);
static void viv_dragdrop_delete(const wchar_t *filename);
static ITEMIDLIST *viv_ILCreateFromPath(const wchar_t *path);
static void viv_get_path_part(wchar_t *buf,const wchar_t *s);
static void viv_timer_stop(void);
static void NTAPI viv_timer_callback(PVOID,BOOLEAN);
static int viv_is_window_maximized(HWND hwnd);
static wchar_t *wstring_format_number(wchar_t *buf,int number);
static int string_length(const utf8_t *text);
static int viv_load_settings_by_location(const wchar_t *path,int is_root);
static int wstring_get_appdata_path(wchar_t *buf);
static void viv_read_int(const utf8_t *p,const utf8_t *key,int *data);
static int utf8_to_int(const utf8_t *str);
static INT_PTR CALLBACK viv_custom_rate_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void viv_update_frame(void);
static void viv_update_ontop(void);
static void dst_pos_set(int x,int y);
static void dst_zoom_set(float x,float y);
static void viv_frame_skip(int size);
static unsigned __stdcall viv_load_image_proc(void *param);
static viv_reply_t *viv_reply_add(DWORD type,DWORD size,void *data);
static void viv_reply_clear_all(void);
static void viv_reply_free(viv_reply_t *e);
static int viv_opengl_init(void);
static void viv_opengl_destroy(void);
static void viv_status_show(int show);
static void viv_status_update(void);
static void viv_status_set(int part,const wchar_t *text);
static int viv_get_status_high(void);

static int viv_keep_centered = 1;
static viv_dragdrop_t *viv_dragdrop_start = 0;
static viv_dragdrop_t *viv_dragdrop_last = 0;
static HWND viv_hwnd = 0;
static HWND viv_status_hwnd = 0;
static HANDLE viv_mutex = 0;
static HMODULE viv_shell32_hmodule = 0;
static HMODULE viv_UxTheme_hmodule = 0;
static HMODULE viv_stobject_hmodule = 0;
static HMODULE viv_opengl32_hmodule = 0;
//static HMODULE viv_hmodule = 0;
//m_hD3D9 = LoadLibrary(L"d3d9.dll");
static HGLRC viv_hglrc = 0;
static int viv_renderer = VIV_RENDERER_GDI;
static int viv_x = 0;
static int viv_y = 0;
static int viv_wide = 640;
static int viv_high = 480;
static int viv_ontop = 0;
static int viv_slide_show_rate = 5000;
static int viv_slide_show_custom_rate = 3;
static int viv_slide_show_custom_rate_type = 1;
static int viv_animation_rate = 0;
static int viv_animation_play = 1;
static int viv_allow_shrinking = 1;
static int viv_shrink_blit_mode = VIV_SHRINK_BLIT_MODE_HALFTONE; 
static int viv_mag_filter = VIV_MAG_FILTER_COLORONCOLOR; 
static int viv_nav_sort = VIV_NAV_SORT_DATE_MODIFIED;
static int viv_nav_sort_ascending = 0;
static int viv_keep_aspect_ratio = 1;
static int viv_fill_window = 0;
static int viv_fullscreen_fill_window = 1;
static int viv_1to1 = 0;
static int viv_have_old_zoom = 0;
static int viv_old_zoom_pos = 0;
static int viv_multiple_instances = 0;
static int viv_appdata = 0;
WIN32_FIND_DATA *viv_current_fd;
static int viv_view_x = 0;
static int viv_view_y = 0;
static double viv_view_ix = 0.0;
static double viv_view_iy = 0.0;
static int viv_zoom_pos = 0;
static ULONG_PTR viv_gdiplusToken;
static int viv_image_wide = 0;
static int viv_image_high = 0;
static int viv_frame_count = 0;
static int viv_frame_loaded_count = 0;
static DWORD *viv_frame_delays = 0;
static int viv_frame_position = 0;
static int viv_frame_looped = 0;
static HBITMAP *viv_frame_hbitmap = 0;
static DWORD viv_timer_tick = 0;
static HANDLE viv_timer_handle = 0;
static int viv_doing = 0;
static int viv_doing_x;
static int viv_doing_y;
static int viv_mdoing_x;
static int viv_mdoing_y;
static int viv_fullscreen_is_maxed = 0;
static int viv_is_fullscreen = 0;
static RECT viv_fullscreen_rect;
static int viv_is_show_menu = 1;
static int viv_is_show_status = 1;
static int viv_is_show_caption = 1;
static int viv_is_show_thickframe = 1;
static int viv_is_slideshow = 0;
static int viv_is_hide_cursor_timer = 0;
static CLIPFORMAT _viv_CF_PREFERREDDROPEFFECT = 0;
static int viv_predefined_rates[] = {250,500,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000,20000,30000,40000,50000,60000};
#define VIV_PREDEFINED_RATE_COUNT (sizeof(viv_predefined_rates) / sizeof(int))
static HMENU viv_nav_playlist_menu;
static viv_nav_item_t **viv_nav_items = 0;
static int viv_nav_item_count = 0;
static int viv_cursor_shown = 1;
static int viv_hide_cursor_x = -1;
static int viv_hide_cursor_y = -1;
static int viv_in_popup_menu = 0;
static HRESULT (WINAPI *viv_SHOpenFolderAndSelectItems)(LPCITEMIDLIST pidlFolder,UINT cidl,LPCITEMIDLIST *apidl,DWORD dwFlags) = 0;
static int viv_loop_animations_once = 1;
static int viv_frame_changed = 0;
static int viv_is_update_frame_timer = 0; 
static HRESULT (__stdcall *viv_EnableThemeDialogTexture)(HWND hwnd, DWORD dwFlags) = 0;
static HMENU viv_main_menu = 0;
static int viv_dst_pos_x = 500;
static int viv_dst_pos_y = 500;
static float viv_dst_zoom_x = 1.0f;
static float viv_dst_zoom_y = 1.0f;
CRITICAL_SECTION viv_cs;
HANDLE viv_load_image_thread = 0;
static wchar_t *viv_load_image_filename = 0;
static WIN32_FIND_DATA *viv_load_image_next_fd = 0;
static volatile int viv_load_image_terminate = 0;
static viv_reply_t *viv_reply_start = 0;
static viv_reply_t *viv_reply_last = 0;
static HGLRC (WINAPI *viv_wglCreateContext)(HDC) = 0;
static BOOL (WINAPI *viv_wglDeleteContext)(HGLRC) = 0;
static GLenum (APIENTRY *viv_glGetError)(void) = 0;
static BOOL (WINAPI *viv_wglMakeCurrent)(HDC, HGLRC) = 0;
static void (APIENTRY *viv_glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) = 0;
static void (APIENTRY *viv_glClear)(GLbitfield mask) = 0;
static void (APIENTRY *viv_glGenTextures)(GLsizei n, GLuint *textures) = 0;
static void (APIENTRY *viv_glBindTexture)(GLenum target, GLuint texture) = 0;
static void (APIENTRY *viv_glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) = 0;
static void (APIENTRY *viv_glEnable)(GLenum cap) = 0;
static void (APIENTRY *viv_glViewport)(GLint x, GLint y, GLsizei width, GLsizei height) = 0;
static void (APIENTRY *viv_glMatrixMode)(GLenum mode) = 0;
static void (APIENTRY *viv_glLoadIdentity)(void) = 0;
static void (APIENTRY *viv_glOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) = 0;
static void (APIENTRY *viv_glBegin)(GLenum mode) = 0;
static void (APIENTRY *viv_glEnd)(void) = 0;
static void (APIENTRY *viv_glTexCoord2f)(GLfloat s, GLfloat t) = 0;
static void (APIENTRY *viv_glVertex2f)(GLfloat x, GLfloat y) = 0;
static void (APIENTRY *viv_glTexParameteri)(GLenum target, GLenum pname, GLint param);
viv_opengl_proc_t viv_opengl_procs[] = 
{
	{(FARPROC *)&viv_wglCreateContext,"wglCreateContext"},
	{(FARPROC *)&viv_wglDeleteContext,"wglDeleteContext"},
	{(FARPROC *)&viv_glGetError,"glGetError"},
	{(FARPROC *)&viv_wglMakeCurrent,"wglMakeCurrent"},
	{(FARPROC *)&viv_glClearColor,"glClearColor"},
	{(FARPROC *)&viv_glClear,"glClear"},
	{(FARPROC *)&viv_glGenTextures,"glGenTextures"},
	{(FARPROC *)&viv_glBindTexture,"glBindTexture"},
	{(FARPROC *)&viv_glTexImage2D,"glTexImage2D"},
	{(FARPROC *)&viv_glEnable,"glEnable"},
	{(FARPROC *)&viv_glViewport,"glViewport"},
	{(FARPROC *)&viv_glMatrixMode,"glMatrixMode"},
	{(FARPROC *)&viv_glLoadIdentity,"glLoadIdentity"},
	{(FARPROC *)&viv_glOrtho,"glOrtho"},
	{(FARPROC *)&viv_glBegin,"glBegin"},
	{(FARPROC *)&viv_glEnd,"glEnd"},
	{(FARPROC *)&viv_glTexCoord2f,"glTexCoord2f"},
	{(FARPROC *)&viv_glVertex2f,"glVertex2f"},
	{(FARPROC *)&viv_glTexParameteri,"glTexParameteri"},
	{0,0},
};

#define VIV_ZOOM_MAX 16

static int wstring_length(const wchar_t *text)
{
	register const wchar_t *p;
	
	p = text;
	
	while(*p)
	{
		p++;
	}
	
	return (int)(p - text);
}

static int string_length(const utf8_t *text)
{
	register const utf8_t *p;
	
	p = text;
	
	while(*p)
	{
		p++;
	}
	
	return (int)(p - text);
}

static void *mem_alloc(int size)
{
	return HeapAlloc(GetProcessHeap(),0,size);
}

static void mem_free(void *ptr)
{
	HeapFree(GetProcessHeap(),0,ptr);
}

static void os_zero_memory(void *data,int size)
{
	ZeroMemory(data,size);
}

static void os_copy_memory(void *d,void *s,int size)
{
	CopyMemory(d,s,size);
}

static void viv_update_title(void)
{
	wchar_t *d;
	const wchar_t *s;
	wchar_t window_title[MAX_PATH+MAX_PATH];
	const wchar_t *filespec;

	d = window_title;
	filespec = PathFindFileName(viv_current_fd->cFileName);
	
	if (*filespec)
	{
		s = filespec;

		while(*s)
		{
			*d++ = *s++;
		}
/*
		if ((viv_image_wide) && (viv_image_high))
		{
			wchar_t res[64];
			
			wsprintf(res,L" - %d x %d",viv_image_wide,viv_image_high);
			s = res;

			while(*s)
			{
				*d++ = *s++;
			}
		}
		*/
		
		{
			wchar_t res[64];
			
			wsprintf(res,L" - ");
			s = res;

			while(*s)
			{
				*d++ = *s++;
			}
			
		}
	}
		
	s = L"void Image Viewer";
	while(*s)
	{
		*d++ = *s++;
	}
	
	*d = 0;
	
	SetWindowTextW(viv_hwnd,window_title);
}

static void viv_clear(void)
{
	if (viv_frame_delays)
	{
		mem_free(viv_frame_delays);
		
		viv_frame_delays = 0;
	}
	
	if (viv_frame_hbitmap)
	{
		int i;
		
		for(i=0;i<viv_frame_loaded_count;i++)		
		{
			if (viv_frame_hbitmap[i])
			{
				DeleteObject(viv_frame_hbitmap[i]);
			}
		}
		
		mem_free(viv_frame_hbitmap);
		
		viv_frame_hbitmap = 0;
	}
	
	viv_timer_stop();	

	viv_frame_position = 0;
	viv_frame_looped = 0;
	viv_frame_count = 0;
	viv_zoom_pos = 0;
	viv_view_x = 0;
	viv_view_y = 0;
	viv_view_ix = 0.0;
	viv_view_iy = 0.0;
	viv_1to1 = 0;
	viv_have_old_zoom = 0;
	viv_image_wide = 0;
	viv_image_high = 0;
	viv_animation_play = 1;
}

static void viv_open_from_full_filename(const wchar_t *filename)
{
	WIN32_FIND_DATA fd;
	HANDLE h;

	printf("%S\n",filename);
	
	h = FindFirstFile(filename,&fd);
	if (h != INVALID_HANDLE_VALUE)
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// add subfolders and subsubfolders...
			viv_dragdrop_add_filename(filename);

			viv_home(0);
		}
		else
		{
			wchar_t path[MAX_PATH];
			
			_wfullpath(path,filename,MAX_PATH);
			
			PathRemoveFileSpec(path);

			printf("set cwd %S\n",path);
			
			if (SetCurrentDirectory(path))
			{
				printf("set cwd OK\n");
			}
			else
			{
				printf("set cwd FAILED %d\n",GetLastError());
			}
			
			viv_open(&fd);
		}
		
		FindClose(h);
	}
}

IStream *SHCreateMemStream(const BYTE *pInit,UINT cbInit);

static void viv_open(WIN32_FIND_DATA *fd)
{
	if (viv_load_image_thread)
	{
		// already loading a different image.
		// add it to the queue and cancel this one.
		viv_load_image_terminate = 1;
		
		if (viv_load_image_next_fd)
		{
			mem_free(viv_load_image_next_fd);
		}
		
		viv_load_image_next_fd = (WIN32_FIND_DATA *)mem_alloc(sizeof(WIN32_FIND_DATA));
		os_copy_memory(viv_load_image_next_fd,fd,sizeof(WIN32_FIND_DATA));
	}
	else
	{
		unsigned int thread_id;
		
		viv_load_image_terminate = 0;
		
		viv_load_image_filename = string_alloc(fd->cFileName);
		
		viv_load_image_thread = (HANDLE)_beginthreadex(NULL,0,viv_load_image_proc,0,0,&thread_id);

		viv_status_update();
	}
	
	os_copy_memory(viv_current_fd,fd,sizeof(WIN32_FIND_DATA));
	
	viv_update_title();
}

static void viv_on_size(void)
{
	if (!IsIconic(viv_hwnd))
	{
		if (!IsMaximized(viv_hwnd))
		{
			if (!viv_is_fullscreen)
			{
				RECT rect;
				
				GetWindowRect(viv_hwnd,&rect);
	
				viv_wide = rect.right - rect.left;
				viv_high = rect.bottom - rect.top;
			}
		}
	}
	
	{
		int rw;
		int rh;
		
		viv_get_render_size(&rw,&rh);
		
		viv_view_set((int)(viv_view_ix * (double)rw),(int)(viv_view_iy * (double)rh));
	}

	if (viv_status_hwnd)
	{
		viv_status_update();
		
		SendMessage(viv_status_hwnd,WM_SIZE,0,0);
	}
}

static void viv_command(int command)
{
	// Parse the menu selections:
	switch (command)
	{
		case VIV_ID_FILE_EXIT:
			viv_exit();
			break;
		
		case VIV_ID_NAV_PREV:
			viv_next(1);
			break;
		
		case VIV_ID_NAV_NEXT:
			viv_next(0);
			break;
		
		case VIV_ID_NAV_HOME:
			viv_home(0);
			break;
		
		case VIV_ID_NAV_END:
			viv_home(1);
			break;
		
		case VIV_ID_NAV_SORT_NAME:
		case VIV_ID_NAV_SORT_SIZE:
		case VIV_ID_NAV_SORT_DATE_MODIFIED:
		case VIV_ID_NAV_SORT_DATE_CREATED:

			if (viv_nav_sort == command - VIV_ID_NAV_SORT_NAME)
			{
				viv_nav_sort_ascending = !viv_nav_sort_ascending;
			}
			else
			{
				viv_nav_sort = command - VIV_ID_NAV_SORT_NAME;
				switch(viv_nav_sort)
				{
					case VIV_NAV_SORT_NAME:
						viv_nav_sort_ascending = 1;
						break;
						
					default:
						viv_nav_sort_ascending = 0;
						break;
				}
			}
			
			break;
		
		case VIV_ID_NAV_SORT_ASCENDING:
			viv_nav_sort_ascending = 1;
			break;
			
		case VIV_ID_NAV_SORT_DESCENDING:
			viv_nav_sort_ascending = 0;
			break;
		
		case VIV_ID_SLIDESHOW_PAUSE:
			viv_pause();
			break;
			
		case VIV_ID_SLIDESHOW_RATE_DEC:
			viv_increase_rate(1);
			break;
			
		case VIV_ID_SLIDESHOW_RATE_INC:
			viv_increase_rate(0);
			break;

		case VIV_ID_ANIMATION_PLAY_PAUSE:
			viv_animation_pause();
			break;

		case VIV_ID_ANIMATION_JUMP_FORWARD_MEDIUM:
			viv_frame_skip(5000);
			break;

		case VIV_ID_ANIMATION_JUMP_BACKWARD_MEDIUM:
			viv_frame_skip(-5000);
			break;

		case VIV_ID_ANIMATION_FRAME_STEP:
			viv_frame_step();
			break;

		case VIV_ID_ANIMATION_FRAME_PREV:
			viv_frame_prev();
			break;

		case VIV_ID_ANIMATION_RATE_DEC:
			viv_increase_animation_rate(1);
			break;
			
		case VIV_ID_ANIMATION_RATE_INC:
			viv_increase_animation_rate(0);
			break;

		case VIV_ID_ANIMATION_RATE_RESET:
			viv_reset_animation_rate();
			break;

		case VIV_ID_SLIDESHOW_RATE_250: viv_set_rate(250); break;
		case VIV_ID_SLIDESHOW_RATE_500: viv_set_rate(500); break;
		case VIV_ID_SLIDESHOW_RATE_1000: viv_set_rate(1000); break;
		case VIV_ID_SLIDESHOW_RATE_2000: viv_set_rate(2000); break;
		case VIV_ID_SLIDESHOW_RATE_3000: viv_set_rate(3000); break;
		case VIV_ID_SLIDESHOW_RATE_4000: viv_set_rate(4000); break;
		case VIV_ID_SLIDESHOW_RATE_5000: viv_set_rate(5000); break;
		case VIV_ID_SLIDESHOW_RATE_6000: viv_set_rate(6000); break;
		case VIV_ID_SLIDESHOW_RATE_7000: viv_set_rate(7000); break;
		case VIV_ID_SLIDESHOW_RATE_8000: viv_set_rate(8000); break;
		case VIV_ID_SLIDESHOW_RATE_9000: viv_set_rate(9000); break;
		case VIV_ID_SLIDESHOW_RATE_10000: viv_set_rate(10000); break;
		case VIV_ID_SLIDESHOW_RATE_20000: viv_set_rate(20000); break;
		case VIV_ID_SLIDESHOW_RATE_30000: viv_set_rate(30000); break;
		case VIV_ID_SLIDESHOW_RATE_40000: viv_set_rate(40000); break;
		case VIV_ID_SLIDESHOW_RATE_50000: viv_set_rate(50000); break;
		case VIV_ID_SLIDESHOW_RATE_60000: viv_set_rate(60000); break;
		case VIV_ID_SLIDESHOW_RATE_CUSTOM: viv_set_custom_rate(); break;
			
		case VIV_ID_VIEW_MENU:
			viv_is_show_menu = !viv_is_show_menu;
			viv_update_frame();
			break;
			
		case VIV_ID_VIEW_STATUS:
			viv_is_show_status = !viv_is_show_status;
			viv_update_frame();
			break;
			
		case VIV_ID_VIEW_PRESET_1:
			viv_is_show_menu = 0;
			viv_is_show_status = 0;
			viv_is_show_caption = 0;
			viv_is_show_thickframe = 0;
			viv_update_frame();
			break;

		case VIV_ID_VIEW_PRESET_2:
			viv_is_show_menu = 0;
			viv_is_show_status = 0;
			viv_is_show_caption = 0;
			viv_is_show_thickframe = 1;
			viv_update_frame();
			break;

		case VIV_ID_VIEW_PRESET_3:
			viv_is_show_menu = 1;
			viv_is_show_status = 1;
			viv_is_show_caption = 1;
			viv_is_show_thickframe = 1;
			viv_update_frame();
			break;
			
		case VIV_ID_VIEW_ALLOW_SHRINKING:
			viv_1to1 = 0;
			viv_allow_shrinking = !viv_allow_shrinking;
			
			viv_on_size();
			InvalidateRect(viv_hwnd,0,FALSE);

			break;
			
		case VIV_ID_VIEW_KEEP_ASPECT_RATIO:
			viv_1to1 = 0;
			viv_keep_aspect_ratio = !viv_keep_aspect_ratio;
			
			viv_on_size();
			InvalidateRect(viv_hwnd,0,FALSE);
			break;
			
		case VIV_ID_VIEW_FILL_WINDOW:
		
			viv_1to1 = 0;
			if (viv_is_fullscreen)
			{
				viv_fullscreen_fill_window = !viv_fullscreen_fill_window;
			}
			else
			{
				viv_fill_window = !viv_fill_window;
			}
			
			viv_on_size();
			InvalidateRect(viv_hwnd,0,FALSE);
			break;
		
		case VIV_ID_VIEW_FULLSCREEN:

			viv_toggle_fullscreen();
			break;		
			
		case VIV_ID_VIEW_1TO1:

			viv_view_1to1();
			break;		
			
		case VIV_ID_VIEW_SLIDE_SHOW:

			viv_slide_show();
						
			break;
			
		case VIV_ID_VIEW_WINDOW_SIZE_50:
		case VIV_ID_VIEW_WINDOW_SIZE_100:
		case VIV_ID_VIEW_WINDOW_SIZE_200:
		case VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT:
			if (viv_is_fullscreen)
			{
				viv_toggle_fullscreen();
			}
		
			// get out of fullscreen mode.
			if (viv_is_window_maximized(viv_hwnd))
			{
				ShowWindow(viv_hwnd,SW_RESTORE);
			}
		
			if ((viv_image_wide) && (viv_image_high))
			{
				RECT rect;
				int wide;
				int high;
				int midx;
				int midy;
				
				GetWindowRect(viv_hwnd,&rect);
				midx = (rect.left + rect.right) / 2;
				midy = (rect.top + rect.bottom) / 2;
				
				rect.left = 0;
				rect.top = 0;
				
				switch(command)
				{
					case VIV_ID_VIEW_WINDOW_SIZE_50:
						rect.right = viv_image_wide / 2;
						rect.bottom = viv_image_high / 2;
						break;
						
					case VIV_ID_VIEW_WINDOW_SIZE_100:
						rect.right = viv_image_wide;
						rect.bottom = viv_image_high;
						break;
						
					case VIV_ID_VIEW_WINDOW_SIZE_200:
						rect.right = viv_image_wide * 2;
						rect.bottom = viv_image_high * 2;
						break;
						
					case VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT:
					{
						HMONITOR hmonitor;
						MONITORINFO mi;
						
						hmonitor = MonitorFromWindow(viv_hwnd,MONITOR_DEFAULTTOPRIMARY);
						
						mi.cbSize = sizeof(MONITORINFO);
						GetMonitorInfo(hmonitor,&mi);										
						
						rect.right = ((mi.rcMonitor.right - mi.rcMonitor.left) * 3) / 5;
						rect.bottom = ((mi.rcMonitor.bottom - mi.rcMonitor.top) * 3) / 5;
						break;
					}
				}
				
				AdjustWindowRect(&rect,GetWindowStyle(viv_hwnd),GetMenu(viv_hwnd) ? TRUE : FALSE);

//printf("%d %d | %d %d %d %d\n",midx,midy,rect.left,rect.top,rect.right,rect.bottom)								;
				
				wide = rect.right - rect.left;
				high = rect.bottom - rect.top;
				
				rect.left = midx - wide / 2;
				rect.top = midy - high / 2;
				rect.right = midx - (wide / 2) + wide;
				rect.bottom = midy - (high / 2) + high;
				
				os_make_rect_completely_visible(&rect);
				
				SetWindowPos(viv_hwnd,0,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOCOPYBITS);
			}			
			break;
			
		case VIV_ID_VIEW_ONTOP_ALWAYS:
			viv_ontop = 1;
			viv_update_ontop();
			break;
						
		case VIV_ID_VIEW_ONTOP_WHILE_PLAYING_OR_ANIMATING:
			viv_ontop = 2;
			viv_update_ontop();
			break;
						
		case VIV_ID_VIEW_ONTOP_NEVER:
			viv_ontop = 0;
			viv_update_ontop();
			break;
						
		case VIV_ID_VIEW_OPTIONS:
			viv_options();
			break;
			
		case VIV_ID_FILE_COPY:
			viv_copy(0);
			break;

		case VIV_ID_FILE_CUT:
			viv_copy(1);
			break;
			
		case VIV_ID_FILE_DELETE:
			viv_delete();
			break;

		case VIV_ID_FILE_PREVIEW:
			viv_file_preview();
			break;

		case VIV_ID_FILE_PRINT:
			viv_file_print();
			break;

		case VIV_ID_FILE_SET_DESKTOP_WALLPAPER:
			viv_file_set_desktop_wallpaper();
			break;
	
		case VIV_ID_FILE_EDIT:
			viv_file_edit();
			break;

		case VIV_ID_FILE_OPEN_FILE_LOCATION:
			viv_open_file_location();
			break;
			
		case VIV_ID_FILE_PROPERTIES:
			viv_properties();
			break;
		
	}
	
	if ((command >= VIV_ID_NAV_PLAYLIST_START) && (command < VIV_ID_NAV_PLAYLIST_END))
	{
		if (command - VIV_ID_NAV_PLAYLIST_START < viv_nav_item_count)
		{
			viv_open(&viv_nav_items[command - VIV_ID_NAV_PLAYLIST_START]->fd);
		}
	}
}

static void viv_exit(void)
{
	viv_save_settings(viv_appdata);
	PostQuitMessage(0);
}

static LRESULT CALLBACK viv_proc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message) 
	{	
		case WM_NCHITTEST:

			if (!viv_is_fullscreen)
			{
				if (DefWindowProc(hwnd,message,wParam,lParam) == HTCLIENT)
				{
					if (!viv_is_show_thickframe)
					{
						int x;
						int y;
						RECT rect;
						
						GetWindowRect(hwnd,&rect);

						x = GET_X_LPARAM(lParam); 
						y = GET_Y_LPARAM(lParam); 				
						
						if (x < rect.left + GetSystemMetrics(SM_CXSIZEFRAME))
						{
							if (y < rect.top + GetSystemMetrics(SM_CYSIZEFRAME))
							{
								return HTTOPLEFT;
							}
							if (y > rect.bottom - GetSystemMetrics(SM_CYSIZEFRAME))
							{
								return HTBOTTOMLEFT;
							}

							return HTLEFT;
						}
						else
						if (x > rect.right - GetSystemMetrics(SM_CXSIZEFRAME))
						{
							if (y < rect.top + GetSystemMetrics(SM_CYSIZEFRAME))
							{
								return HTTOPRIGHT;
							}
							if (y > rect.bottom - GetSystemMetrics(SM_CYSIZEFRAME))
							{
								return HTBOTTOMRIGHT;
							}

							return HTRIGHT;
						}
						else
						if (y < rect.top + GetSystemMetrics(SM_CYSIZEFRAME))
						{
							if (x < rect.left + GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXSIZEFRAME))
							{
								return HTTOPLEFT;
							}
							if (x > rect.right - GetSystemMetrics(SM_CXSIZEFRAME) - GetSystemMetrics(SM_CXSIZEFRAME))
							{
								return HTTOPRIGHT;
							}
							
							return HTTOP;
						}
						if (y > rect.bottom - GetSystemMetrics(SM_CYSIZEFRAME))
						{
							if (x < rect.left + GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXSIZEFRAME))
							{
								return HTBOTTOMLEFT;
							}
							if (x > rect.right - GetSystemMetrics(SM_CXSIZEFRAME) - GetSystemMetrics(SM_CXSIZEFRAME))
							{
								return HTBOTTOMRIGHT;
							}
							
							return HTBOTTOM;
						}
					}
				}
			}
			
			break;
			
		case WM_NCLBUTTONDOWN:

			if (!viv_is_fullscreen)
			{
				if (!viv_is_show_thickframe)
				{
					DWORD flag;
					
					flag = 0;
					
					switch(wParam)
					{
						case HTTOP:
							flag = WMSZ_TOP;
							break;
						case HTTOPLEFT:
							flag = WMSZ_TOPLEFT;
							break;
						case HTTOPRIGHT:
							flag = WMSZ_TOPRIGHT;
							break;
						case HTLEFT:
							flag = WMSZ_LEFT;
							break;
						case HTRIGHT:
							flag = WMSZ_RIGHT;
							break;
						case HTBOTTOM:
							flag = WMSZ_BOTTOM;
							break;
						case HTBOTTOMLEFT:
							flag = WMSZ_BOTTOMLEFT;
							break;
						case HTBOTTOMRIGHT:
							flag = WMSZ_BOTTOMRIGHT;
							break;							
					}

					if (flag)
					{					
			            PostMessage(hwnd,WM_SYSCOMMAND,(SC_SIZE | flag),lParam);
			            return 0;
					}
				}
			}
			
			break;
		
		case WM_DESTROY:
			// don't free the menu again.
			viv_main_menu = 0;
			break;
			
		case WM_ENDSESSION:
			viv_save_settings(viv_appdata);
			return 0;
			
		case VIV_WM_REPLY:
		{
			viv_reply_t *e;
			
			EnterCriticalSection(&viv_cs);
			e = viv_reply_start;
			viv_reply_start = 0;
			viv_reply_last = 0;
			LeaveCriticalSection(&viv_cs);
			
			while(e)
			{
				viv_reply_t *next_e;
				
				next_e = e->next;
				
				switch(e->type)
				{
					case VIV_REPLY_LOAD_IMAGE_COMPLETE:
						
						if (viv_load_image_thread)
						{
							CloseHandle(viv_load_image_thread);
							
							viv_load_image_thread = 0;
						}
					
						if (viv_load_image_filename)
						{
							mem_free(viv_load_image_filename);

							viv_load_image_filename = 0;
						}
						
						if (viv_load_image_next_fd)
						{
							WIN32_FIND_DATA *fd;
							
							fd = viv_load_image_next_fd;
							viv_load_image_next_fd = 0;
							
							viv_open(fd);
							
							mem_free(fd);
						}
						
						viv_status_update();
						
						break;
					
					case VIV_REPLY_LOAD_IMAGE_FIRST_FRAME:
					{
						viv_reply_load_image_first_frame_t *first_frame;

						viv_clear();

						first_frame = (viv_reply_load_image_first_frame_t *)(e + 1);
						
						viv_image_wide = first_frame->wide;
						viv_image_high = first_frame->high;
						viv_frame_count = first_frame->frame_count;
						viv_frame_delays = first_frame->frame_delays;

						first_frame->frame_delays = 0;
						
						if (viv_frame_hbitmap)
						{
							int i;
							
							for(i=0;i<viv_frame_loaded_count;i++)
							{
								if (viv_frame_hbitmap[i])
								{
									DeleteObject(viv_frame_hbitmap[i]);
								}
							}

							mem_free(viv_frame_hbitmap);
						}

						// allocate hbitmaps.
						viv_frame_hbitmap = (HBITMAP *)mem_alloc(sizeof(HBITMAP) * viv_frame_count);
						viv_frame_hbitmap[0] = first_frame->hbitmap;
						first_frame->hbitmap = 0;
						viv_frame_loaded_count = 1;

						// draw first frame and start timer.
						viv_frame_position = 0;
						viv_frame_looped = 0;
						
						if (viv_frame_count > 1)
						{
							viv_timer_start();
						}

						viv_status_update();
						InvalidateRect(viv_hwnd,NULL,FALSE);
						
						break;
					}

					case VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME:
					
						// we could have been cleared.
						if (viv_frame_hbitmap)
						{
							viv_frame_hbitmap[viv_frame_loaded_count] = *(HBITMAP *)(e + 1);
							viv_frame_loaded_count++;
							
							*(HBITMAP *)(e + 1) = 0;
						}
					
						break;
				}

				viv_reply_free(e);
				
				e = next_e;
			}
			
			break;
		}
			
		case VIV_WM_TIMER:
		
			if (viv_timer_handle)	
			{
				if (viv_animation_play)
				{
					viv_timer_tick++;
					
					for(;;)
					{
						DWORD n;
						DWORD d;
						DWORD delay;
						
						n = 1;
						d = 1;
						
						if (viv_animation_rate > 0)
						{
							n = 1;
							d = viv_animation_rate;
						}
						else
						if (viv_animation_rate < 0)
						{
							n = -viv_animation_rate;
							d = 1;
						}

						delay = (viv_frame_delays[viv_frame_position] * n) / d;
						
						if (!delay)
						{
							delay = 1;
						}
						
						if (viv_timer_tick >= delay)
						{
							if (viv_frame_loaded_count != viv_frame_count)
							{
								if (viv_frame_position + 1 >= viv_frame_loaded_count)
								{	
									break;
								}
							}
						
							viv_frame_position++;
							if (viv_frame_position == viv_frame_count)
							{
								viv_frame_looped = 1;
	/*							
								if (viv_loop_animations_once)
								{
									viv_next(0);
									break;
								}
	*/						
								viv_frame_position = 0;
							}
							
							viv_frame_changed = 1;

							viv_timer_tick -= delay;
						}
						else
						{
							break;
						}
					}
				}
			}

			break;

		case WM_INITMENU:
		{	
			HMENU hmenu;

			hmenu = GetMenu(hwnd);

			viv_check_menus(hmenu);
			
			break;
		}
					
		case WM_INITMENUPOPUP:
		{	
			if ((HMENU)wParam == viv_nav_playlist_menu)
			{
				WIN32_FIND_DATA fd;
				viv_nav_item_t *navitemstart;
				viv_nav_item_t *navitemlast;
				viv_nav_item_t *navitem;
				
				navitemstart = 0;
				navitemlast = 0;
				
				printf("menu playlist\n");
				
				os_remove_all_menu_items(viv_nav_playlist_menu);
				viv_nav_item_free_all();
				
				if (viv_dragdrop_start)
				{
					viv_dragdrop_t *d;
					
					d = viv_dragdrop_start;
					while(d)
					{
						navitem = (viv_nav_item_t *)mem_alloc(sizeof(viv_nav_item_t));
						
						wstring_copy(navitem->fd.cFileName,MAX_PATH,d->filename);
						
						viv_fd_copy_fileinfo(&navitem->fd,&d->fileinfo);

						if (navitemstart)
						{
							navitemlast->next = navitem;
						}
						else
						{
							navitemstart = navitem;
						}
						
						navitem->next = 0;
						navitemlast = navitem;
						viv_nav_item_count++;				
					
						d = d->next;
					}
				}
				else
				{				
					HANDLE h;
					
					h = FindFirstFile(L"*.*",&fd);
					
					if (h != INVALID_HANDLE_VALUE)
					{
						for(;;)
						{
							if (viv_is_valid_filename(fd.cFileName))
							{
								navitem = (viv_nav_item_t *)mem_alloc(sizeof(viv_nav_item_t));
								os_copy_memory(&navitem->fd,&fd,sizeof(WIN32_FIND_DATA));
								
								if (navitemstart)
								{
									navitemlast->next = navitem;
								}
								else
								{
									navitemstart = navitem;
								}
								
								navitem->next = 0;
								navitemlast = navitem;
								viv_nav_item_count++;
							}
							
							if (!FindNextFile(h,&fd)) break;
						}

						FindClose(h);
					}
				}
				
				if (viv_nav_item_count)
				{
					viv_nav_item_t **d;
					
					viv_nav_items = (viv_nav_item_t **)mem_alloc(viv_nav_item_count * sizeof(viv_nav_item_t *));
					d = viv_nav_items;
					
					navitem = navitemstart;
					while(navitem)
					{
						*d++ = navitem;
						
						navitem = navitem->next;
					}
				}
				
				qsort(viv_nav_items,viv_nav_item_count,sizeof(viv_nav_item_t *),viv_nav_playlist_compare);
				
				{
					int i;
					int checked;
					int run;
					
					run = viv_nav_item_count;
					if (run > VIV_ID_NAV_PLAYLIST_END-VIV_ID_NAV_PLAYLIST_START)
					{
						run = VIV_ID_NAV_PLAYLIST_END-VIV_ID_NAV_PLAYLIST_START;
					}
					
					for(i=0;i<run;i++)
					{
						wchar_t menutext[MAX_PATH+MAX_PATH];
						wchar_t *d;
						wchar_t *p;
						
						checked = (wcscmp(viv_nav_items[i]->fd.cFileName,viv_current_fd->cFileName) == 0);
						
						p = PathFindFileName(viv_nav_items[i]->fd.cFileName);
						d = menutext;
						
						while(*p)
						{
							if (*p == '&')
							{
								*d++ = '&';
								*d++ = '&';
							}
							else
							{
								*d++ = *p;
							}
							
							p++;
						}
						*d = 0;
	
						AppendMenu(viv_nav_playlist_menu,MF_STRING|(checked?(MF_CHECKED|MFT_RADIOCHECK):(MF_UNCHECKED|MFT_RADIOCHECK)),VIV_ID_NAV_PLAYLIST_START + i,menutext);
					}
				}
			}
			
			break;
		}

		case WM_DROPFILES:
		{
			wchar_t filename[MAX_PATH];
			DWORD count;
			
			viv_dragdrop_clearall();
			
			count = DragQueryFile((HDROP)wParam,0xFFFFFFFF,0,0);
			
			if (count >= 1)
			{
				DragQueryFile((HDROP)wParam,0,filename,MAX_PATH);
			
				viv_open_from_full_filename(filename);
				
				if (count >= 2)
				{
					DWORD i;
					
					for(i=0;i<count;i++)
					{
						DragQueryFile((HDROP)wParam,i,filename,MAX_PATH);
					
						viv_dragdrop_add_filename(filename);
					}
				}
			}
			
			SetForegroundWindow(hwnd);
			
			break;
		}
			
		case WM_TIMER:
			
			switch(wParam)
			{
				case VIV_ID_SLIDE_SHOW_TIMER:

					if (viv_is_slideshow)
					{
						if (viv_frame_delays)
						{
							if (!viv_frame_looped)
							{
								break;
							}
						}
						
						viv_next(0);
					}
					
					break;
					
				case VIV_ID_HIDE_CURSOR_TIMER:
					if (viv_is_hide_cursor_timer)
					{
						KillTimer(hwnd,VIV_ID_HIDE_CURSOR_TIMER);
						viv_is_hide_cursor_timer = 0;
						
						if (!viv_in_popup_menu)
						{
							if (viv_cursor_shown)
							{
								ShowCursor(FALSE);
								
								viv_cursor_shown = 0;
							}
						}
					}
					break;
					
				case VIV_ID_UPDATE_FRAME_TIMER:
					if (viv_frame_changed)
					{
						viv_frame_changed = 0;
						InvalidateRect(viv_hwnd,0,FALSE);
					}
					break;
			}
			
			break;
			
		case WM_LBUTTONDBLCLK:
			viv_toggle_fullscreen();
			break;
			
		case WM_LBUTTONDOWN:
			
			if (!viv_is_show_caption)
			{
				SendMessage(hwnd,WM_NCLBUTTONDOWN,(WPARAM)HTCAPTION,lParam);
				return 0;
			}				

			viv_doing = 1;
			viv_doing_x = GET_X_LPARAM(lParam);
			viv_doing_y = GET_Y_LPARAM(lParam);
			SetCapture(hwnd);
			break;
			
		case WM_XBUTTONDOWN:

			switch(HIWORD(wParam))
			{
				case XBUTTON1:
					viv_next(1);
					break;

				case XBUTTON2:
					viv_next(0);
					break;
			}
			
			break;
			
		case WM_MBUTTONDOWN:
			viv_doing = 2;
			viv_doing_x = GET_X_LPARAM(lParam);
			viv_doing_y = GET_Y_LPARAM(lParam);
			SetCapture(hwnd);
			{
				POINT pt;
				GetCursorPos(&pt);
				
				viv_mdoing_x = pt.x;
				viv_mdoing_y = pt.y;
			}
			ShowCursor(FALSE);
			break;
			
		case WM_CONTEXTMENU:
		{
			HMENU hmenu;
			HMENU rate_menu;
			HMENU animation_rate_menu;
			HMENU nav_sort_menu;
			POINT pt;
			DWORD tpm_flags;
			
			tpm_flags = 0;
			
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			
			if ((pt.x == -1) && (pt.y == -1))
			{
				RECT rect;
				
				GetWindowRect(hwnd,&rect);
				
				pt.x = (rect.right + rect.left) / 2;
				pt.y = (rect.bottom + rect.top) / 2;
				
				tpm_flags |= TPM_VCENTERALIGN | TPM_CENTERALIGN;
			}
			
//			ClientToScreen(hwnd,&pt);
			
			hmenu = CreatePopupMenu();
			rate_menu = CreatePopupMenu();
			animation_rate_menu = CreatePopupMenu();
			nav_sort_menu = CreatePopupMenu();
					
			AppendMenuW(hmenu,MF_STRING,VIV_ID_NAV_NEXT,L"&Next\tRight Arrow");
			AppendMenuW(hmenu,MF_STRING,VIV_ID_NAV_PREV,L"P&revious\tLeft Arrow");
			AppendMenuW(hmenu,MF_POPUP,(UINT_PTR)nav_sort_menu,L"S&ort");

			AppendMenuW(hmenu,MF_SEPARATOR,0,0);

			AppendMenuW(hmenu,MF_STRING,VIV_ID_FILE_OPEN_FILE_LOCATION,L"Open F&ile Location...");
			AppendMenuW(hmenu,MF_STRING,VIV_ID_FILE_EDIT,L"&Edit...");
			AppendMenuW(hmenu,MF_STRING,VIV_ID_FILE_PREVIEW,L"Pre&view...");
			AppendMenuW(hmenu,MF_STRING,VIV_ID_FILE_PRINT,L"&Print...");
			AppendMenuW(hmenu,MF_STRING,VIV_ID_FILE_SET_DESKTOP_WALLPAPER,L"Set &Desktop Wallpaper");

			AppendMenuW(hmenu,MF_SEPARATOR,0,0);
			
			AppendMenuW(nav_sort_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_NAV_SORT_NAME,L"&Name");
			AppendMenuW(nav_sort_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_NAV_SORT_SIZE,L"&Size");
			AppendMenuW(nav_sort_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_NAV_SORT_DATE_MODIFIED,L"Date &Modified");
			AppendMenuW(nav_sort_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_NAV_SORT_DATE_CREATED,L"Date &Created");
			AppendMenuW(nav_sort_menu,MF_SEPARATOR,0,0);
			AppendMenuW(nav_sort_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_NAV_SORT_ASCENDING,L"&Ascending");
			AppendMenuW(nav_sort_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_NAV_SORT_DESCENDING,L"&Descending");

			if (!viv_is_show_menu)
			{
				AppendMenuW(hmenu,MF_STRING,VIV_ID_VIEW_MENU,L"&Menu");
				AppendMenuW(hmenu,MF_SEPARATOR,0,0);
			}
			
			AppendMenuW(hmenu,MF_STRING,VIV_ID_VIEW_ALLOW_SHRINKING,L"&Allow Shrinking");
			AppendMenuW(hmenu,MF_STRING,VIV_ID_VIEW_KEEP_ASPECT_RATIO,L"&Keep Aspect Ratio");
			AppendMenuW(hmenu,MF_STRING,VIV_ID_VIEW_FILL_WINDOW,L"&Fill Window");
			AppendMenuW(hmenu,MF_STRING,VIV_ID_VIEW_1TO1,L"1:1\tCtrl+Alt+0");
			AppendMenuW(hmenu,MF_STRING,VIV_ID_VIEW_FULLSCREEN,L"F&ullscreen\tAlt+Enter");
			AppendMenuW(hmenu,MF_STRING,VIV_ID_VIEW_SLIDE_SHOW,L"Play &Slideshow\tF11");
			
			AppendMenuW(hmenu,MF_SEPARATOR,0,0);
			
			AppendMenuW(hmenu,MF_STRING,VIV_ID_SLIDESHOW_PAUSE,L"&Play/Pause\tSpace");
			AppendMenuW(hmenu,MF_SEPARATOR,0,0);

			AppendMenuW(hmenu,MF_POPUP,(UINT_PTR)rate_menu,L"Slideshow &Rate");
			AppendMenuW(rate_menu,MF_STRING,VIV_ID_SLIDESHOW_RATE_DEC,L"&Decrease Rate\tDown");
			AppendMenuW(rate_menu,MF_STRING,VIV_ID_SLIDESHOW_RATE_INC,L"&Increase Rate\tUp");
			AppendMenuW(rate_menu,MF_SEPARATOR,0,0);
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_250,L"250 Milliseconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_500,L"500 Milliseconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_1000,L"&1 Second");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_2000,L"&2 Seconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_3000,L"&3 Seconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_4000,L"&4 Seconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_5000,L"&5 Seconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_6000,L"&6 Seconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_7000,L"&7 Seconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_8000,L"&8 Seconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_9000,L"&9 Seconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_10000,L"1&0 Seconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_20000,L"20 Seconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_30000,L"30 Seconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_40000,L"40 Seconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_50000,L"50 Seconds");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_60000,L"1 Minute");
			AppendMenuW(rate_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_CUSTOM,L"Custom...");

			AppendMenuW(hmenu,MF_SEPARATOR,0,0);

			AppendMenuW(hmenu,MF_STRING,VIV_ID_FILE_CUT,L"C&ut\tCtrl+X");
			AppendMenuW(hmenu,MF_STRING,VIV_ID_FILE_COPY,L"&Copy\tCtrl+C");
			AppendMenuW(hmenu,MF_STRING,VIV_ID_FILE_DELETE,L"&Delete\tDel");
			AppendMenuW(hmenu,MF_SEPARATOR,0,0);
			AppendMenuW(hmenu,MF_STRING,VIV_ID_FILE_PROPERTIES,L"P&roperties");

			viv_check_menus(hmenu);
			
			if (!viv_cursor_shown)
			{
				ShowCursor(TRUE);
				
				viv_cursor_shown = 1;
			}

			viv_in_popup_menu = 1;
			
			TrackPopupMenu(hmenu,tpm_flags,pt.x,pt.y,0,hwnd,0);

			// start the hide cursor timer again.
			viv_in_popup_menu = 0;
			viv_show_cursor();
			
			DestroyMenu(hmenu);
			
			break;
		}
			
		case WM_MOUSEMOVE:
				
			viv_show_cursor();

			if (viv_doing)
			{
				int mx;
				int my;
				
				if (viv_doing == 2)
				{
					POINT pt;
					GetCursorPos(&pt);

					mx = viv_mdoing_x - pt.x;
					my = viv_mdoing_y - pt.y;
					
					if ((mx) || (my))
					{
						SetCursorPos(viv_mdoing_x,viv_mdoing_y);
					}
				}
				else
				{
					int x;
					int y;
					
					x = GET_X_LPARAM(lParam); 
					y = GET_Y_LPARAM(lParam); 

					mx = x - viv_doing_x;
					my = y - viv_doing_y;
					
					viv_doing_x = x;
					viv_doing_y = y;
				}
				
				if ((mx) || (my))
				{
					viv_view_set(viv_view_x - mx,viv_view_y - my);
				}				
			}
			
			break;
			
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		
			viv_doing_cancel();
			
			break;
			
		case WM_MOUSEWHEEL:
		{
			int cursor_x;
			int cursor_y;
			int rx;
			int ry;
			int rw;
			int rh;
			int old_rw;
			int old_rh;
			int old_cursor_px;
			int old_cursor_py;
			int wide;
			int high;
			RECT rect;
			POINT pt;
			__int64 new_cursor_x;
			__int64 new_cursor_y;
			
			GetClientRect(hwnd,&rect);
			wide = rect.right - rect.left;
			high = rect.bottom - rect.top - viv_get_status_high();
			
			pt.x = GET_X_LPARAM(lParam); 
			pt.y = GET_Y_LPARAM(lParam);
			
			ScreenToClient(hwnd,&pt);

			cursor_x = pt.x; 
			cursor_y = pt.y;
		
			viv_get_render_size(&rw,&rh);
			
/*
			if (viv_zoom_pos == 1)
			{
				if ((rw < viv_image_wide) || (rw < viv_image_wide))
				{
					rw = viv_image_wide;
					rh = viv_image_high;
				}
			}
			*/
			rx = (wide / 2) - (rw / 2) - viv_view_x;
			ry = (high / 2) - (rh / 2) - viv_view_y;
			
			old_cursor_px = (cursor_x - rx);
			old_cursor_py = (cursor_y - ry);
			old_rw = rw;
			old_rh = rh;

			if (old_cursor_px < 0)
			{
				old_cursor_px = 0;
			}

			if (old_cursor_px > 20 * rw)
			{
				old_cursor_px = 20 * rw;
			}
			
			if (old_cursor_py < 0)
			{
				old_cursor_py = 0;
			}

			if (old_cursor_py > 20 * rh)
			{
				old_cursor_py = 20 * rh;
			}
			
			if (viv_1to1)
			{
				viv_1to1 = 0;
				
				if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
				{
					for(viv_zoom_pos = 0;viv_zoom_pos<VIV_ZOOM_MAX;viv_zoom_pos++)
					{
						viv_get_render_size(&rw,&rh);
						
						if (rw > old_rw)
						{
							break;
						}
					}
				}
				else
				{
					for(viv_zoom_pos = VIV_ZOOM_MAX-1;viv_zoom_pos>=0;viv_zoom_pos--)
					{
						viv_get_render_size(&rw,&rh);
						
						if (rw < old_rw)
						{
							break;
						}
					}
				}
			}
			else
			{
				if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
				{
					viv_zoom_pos++;
				}
				else
				{
					viv_zoom_pos--;
				}
			}
			
			if (viv_zoom_pos < 0) viv_zoom_pos = 0;
			if (viv_zoom_pos > VIV_ZOOM_MAX-1) viv_zoom_pos = VIV_ZOOM_MAX-1;
			
			viv_get_render_size(&rw,&rh);
/*
			if (viv_zoom_pos == 1)
			{
				if ((rw < viv_image_wide) || (rw < viv_image_wide))
				{
					rw = viv_image_wide;
					rh = viv_image_high;
				}
			}
*/
			// 
			// new_cursor_x = 
//			rx = (wide / 2) - (rw / 2)

//printf("%d %d\n",(wide / 2) - (rw / 2),(high / 2) - (rh / 2));
//printf("%d %d\n",old_cursor_px,(old_cursor_px * 100) / old_rw);
						
//printf("old %d %d new %d %d\n",old_cursor_px,old_cursor_py,(wide / 2) - (rw / 2) - cursor_x + ((old_cursor_px * rw) / old_rw),(high / 2) - (rh / 2) - cursor_y + ((old_cursor_py * rh) / old_rh))		;
//printf("wide / 2 = %d, rw/2=%d, old_cursor_px * rw=%d\n",wide/2,rw/2,old_cursor_px * rw)		;

			if (old_rw)
			{
				new_cursor_x = ((__int64)old_cursor_px * (__int64)rw) / (__int64)old_rw;
			}
			else
			{
				new_cursor_x = 0;
			}
			
			if (old_rh)
			{
				new_cursor_y = ((__int64)old_cursor_py * (__int64)rh) / (__int64)old_rh;
			}
			else
			{
				new_cursor_y = 0;
			}

			viv_view_set((wide / 2) - (rw / 2) - cursor_x + new_cursor_x,(high / 2) - (rh / 2) - cursor_y + new_cursor_y);
			
			InvalidateRect(hwnd,0,FALSE);
			
			break;
		}
		
		case WM_COPYDATA:
		{
			COPYDATASTRUCT *cds;
				
			cds = (COPYDATASTRUCT *)lParam;

			switch(cds->dwData)
			{
				case 0:
					
					DWORD showcmd;
					const char *p;
					const char *e;
					wchar_t cl[MAX_PATH];
					wchar_t cwd[MAX_PATH];

					SetForegroundWindow(hwnd);
					
					p = (char *)cds->lpData;
					e = p + cds->cbData;
					
					// is there a show command ?
					if (e-p >= sizeof(DWORD))
					{
						showcmd = *(DWORD *)p;
						p += sizeof(DWORD);
						
						p = _viv_get_copydata_string(p,e,cl,MAX_PATH);
						p = _viv_get_copydata_string(p,e,cwd,MAX_PATH);
						
						viv_dragdrop_clearall();
						
						SetCurrentDirectory(cwd);
						
						viv_process_command_line(cl);
						
						ShowWindow(viv_hwnd,showcmd);
					}
										
					return 1;
			}
			
			break;
		}
		
		case WM_CLOSE:
			viv_exit();
			return 0;
		
		case WM_SYSCOMMAND:	
			
			switch(wParam)
			{
				case SC_MONITORPOWER:
				case SC_SCREENSAVE:

					// prevent sleep / monitor power off.

					if (viv_is_slideshow)
					{
						return 0;
					}
					
					if (viv_timer_handle)
					{
						if (viv_animation_play)
						{
							return 0;
						}
					}
					break;
			}
			break;
		
		case WM_SIZE:
		
			viv_on_size();

			break;
			
		case WM_MOVE:
		{
			if (!IsIconic(hwnd))
			{
				if (!IsMaximized(hwnd))
				{
					if (!viv_is_fullscreen)
					{
						RECT rect;
						
						GetWindowRect(hwnd,&rect);
			
						viv_x = rect.left;
						viv_y = rect.top;
					}
				}
			}
			
			break;
		}
			
		case WM_COMMAND:
		{
			viv_command(LOWORD(wParam));
		
			break;
		}
		
		case WM_PAINT:
		{
			RECT rect;
			int wide;
			int high;
			PAINTSTRUCT ps;
			
			GetClientRect(hwnd,&rect);
			wide = rect.right - rect.left;
			high = rect.bottom - rect.top - viv_get_status_high();

			BeginPaint(hwnd,&ps);

			if (viv_frame_count)
			{
				int rx;
				int ry;
				int rw;
				int rh;
				HDC mem_hdc;
				
				viv_get_render_size(&rw,&rh);

				rw = (int)(rw * viv_dst_zoom_x);
				rh = (int)(rh * viv_dst_zoom_y);
				
	#if 0
				if (viv_zoom_pos == 1)
				{
					if ((rw < viv_image_wide) || (rw < viv_image_wide))
					{
						rw = viv_image_wide;
						rh = viv_image_high;
					}
				}
	#endif
				
				rx = ((viv_dst_pos_x * wide) / 1000) - (rw / 2) - viv_view_x;
				ry = ((viv_dst_pos_y * high) / 1000) - (rh / 2) - viv_view_y;
				
	//			rh += (-dst_left_offset + dst_right_offset) * wide / 1000;
	//			rh += (-dst_top_offset + dst_bottom_offset) * high / 1000;


				if (viv_renderer == VIV_RENDERER_OPENGL)
				{
					if (viv_hglrc)
					{
						viv_wglMakeCurrent(ps.hdc,viv_hglrc);
						
						viv_glEnable(GL_TEXTURE_2D);
						
						viv_glClearColor(0,0,0,0);
						viv_glClear(GL_COLOR_BUFFER_BIT);
						
static int got_textures = 0;
static GLuint m_texture;
if (!got_textures)
{
						viv_glGenTextures(1,&m_texture);
						viv_glBindTexture(GL_TEXTURE_2D,m_texture);

						if (viv_frame_hbitmap[viv_frame_position])
						{
							mem_hdc = CreateCompatibleDC(ps.hdc);
							if (mem_hdc)
							{
								DWORD *pixels;
								BITMAPINFO bmi;
								
								os_zero_memory(&bmi,sizeof(BITMAPINFO));
								bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
								bmi.bmiHeader.biBitCount = 32;
								bmi.bmiHeader.biHeight = -viv_image_high;
								bmi.bmiHeader.biWidth = viv_image_wide;
								bmi.bmiHeader.biPlanes = 1;
								
								pixels = (DWORD *)mem_alloc(viv_image_wide * viv_image_high * 4);
								
								if (!GetDIBits(ps.hdc,viv_frame_hbitmap[viv_frame_position],0,viv_image_high,pixels,&bmi,DIB_RGB_COLORS))
								{
									printf("bad dibits\n");
								}
									
								viv_glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,viv_image_wide,viv_image_high,0,GL_BGRA_EXT,GL_UNSIGNED_BYTE,pixels);

								viv_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
								viv_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
								viv_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
								viv_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 

								mem_free(pixels);
									
								DeleteDC(mem_hdc);
							}
						}
						
						got_textures = 1;
					}
							
						viv_glViewport(rx,high-rh-ry,rw,rh);
						viv_glMatrixMode(GL_PROJECTION);
						viv_glLoadIdentity();
						viv_glOrtho(-1.0f,1.0f,1.0f,-1.0f,-0.5,0.5);
						viv_glMatrixMode(GL_MODELVIEW);
						viv_glLoadIdentity();

						viv_glEnable(GL_TEXTURE_2D);
						viv_glBindTexture(GL_TEXTURE_2D,m_texture);
						viv_glBegin(GL_QUADS);
						viv_glTexCoord2f(0.0f,0.0f);
						viv_glVertex2f(-1.0f,-1.0f);
						viv_glTexCoord2f(1.0f,0.0f);
						viv_glVertex2f(1.0f,-1.0f);
						viv_glTexCoord2f(1.0f,1.0f);
						viv_glVertex2f(1.0f,1.0f);
						viv_glTexCoord2f(0.0f,1.0f);
						viv_glVertex2f(-1.0f,1.0f);
						viv_glEnd();

						SwapBuffers(ps.hdc);
						
						viv_wglMakeCurrent(ps.hdc,0);

						ExcludeClipRect(ps.hdc,0,0,wide,high);
					}
				}
				else
				{
					mem_hdc = CreateCompatibleDC(ps.hdc);
					if (mem_hdc)
					{
						if (viv_frame_hbitmap[viv_frame_position])
						{
							HGDIOBJ last_hbitmap;
							
							last_hbitmap = SelectObject(mem_hdc,viv_frame_hbitmap[viv_frame_position]);

							if ((rw == viv_image_wide) && (rh == viv_image_high))
							{
								BitBlt(ps.hdc,rx,ry,rw,rh,mem_hdc,0,0,SRCCOPY);
							}
							else
							{
								int last_stretch_mode;
								int did_set_stretch_blt_mode;
								
								did_set_stretch_blt_mode = 0;
								
								if ((rw < viv_image_wide) || (rh < viv_image_high))
								{
									if (viv_shrink_blit_mode == VIV_SHRINK_BLIT_MODE_HALFTONE)
									{
										last_stretch_mode = SetStretchBltMode(ps.hdc,HALFTONE);
									}
									else
									{
										last_stretch_mode = SetStretchBltMode(ps.hdc,COLORONCOLOR);
									}
									
									did_set_stretch_blt_mode = 1;
								}
								else
								if ((rw > viv_image_wide) || (rh > viv_image_high))
								{
									if (viv_mag_filter == VIV_MAG_FILTER_HALFTONE)
									{
										last_stretch_mode = SetStretchBltMode(ps.hdc,HALFTONE);
									}
									else
									{
										last_stretch_mode = SetStretchBltMode(ps.hdc,COLORONCOLOR);
									}
									
									did_set_stretch_blt_mode = 1;
								}
								
								StretchBlt(ps.hdc,rx,ry,rw,rh,mem_hdc,0,0,viv_image_wide,viv_image_high,SRCCOPY);

								if (did_set_stretch_blt_mode)
								{
									SetStretchBltMode(ps.hdc,last_stretch_mode);
								}
							}
							
							ExcludeClipRect(ps.hdc,rx,ry,rx+rw,ry+rh);

							SelectObject(mem_hdc,last_hbitmap);
						}
						
						DeleteDC(mem_hdc);
					}
				}
			}

			if (viv_is_fullscreen)
			{
				FillRect(ps.hdc,&rect,(HBRUSH)GetStockObject(BLACK_BRUSH));
			}
			else
			{
				FillRect(ps.hdc,&rect,(HBRUSH)GetStockObject(WHITE_BRUSH));
			}
			
			EndPaint(hwnd,&ps);
			
			return 0;
		}
			
		case WM_ERASEBKGND:
			return 1;
	}
	
	return DefWindowProc(hwnd,message,wParam,lParam);
}

static void viv_read_int(const utf8_t *start,const utf8_t *key,int *data)
{	
	const utf8_t *p1;
	const utf8_t *p2;
	
	p1 = start;
	p2 = key;
	
	while(*p2)
	{
		if (*p2 != *p1)
		{
			return;
		}
		
		p1++;
		p2++;
	}

	if (*p1 == '=')
	{
		p1++;

		// match
		*data = utf8_to_int(p1);
	}
}

static int viv_load_settings_by_location(const wchar_t *path,int is_root)
{
	HANDLE h;
	wchar_t filename[MAX_PATH];
	int ret;
	
	ret = 0;
	
	PathCombineW(filename,path,L"voidImageViewer.ini");

	h = CreateFileW(filename,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
	if (h != INVALID_HANDLE_VALUE)
	{
		DWORD size;
		utf8_t *buf;
		
		size = GetFileSize(h,0);
		
		buf = (utf8_t *)mem_alloc(size + 1);
		if (buf)
		{
			DWORD numread;
			
			if (ReadFile(h,buf,size,&numread,0))
			{
				if (numread == size)
				{
					utf8_t *p;
					
					buf[size] = 0;
					
					p = buf;
					
					while(*p)
					{
						const utf8_t *start;
						
						start = p;
						
						while(*p)
						{
							if (*p == '\n')
							{
								*p++ = 0;
								break;
							}
							if ((*p == '\r') || (p[1] == '\n'))
							{
								*p = 0;
								p += 2;
								break;
							}
							
							p++;
						}
						
						viv_read_int(start,(const utf8_t *)"x",&viv_x);
						viv_read_int(start,(const utf8_t *)"y",&viv_y);
						viv_read_int(start,(const utf8_t *)"wide",&viv_wide);
						viv_read_int(start,(const utf8_t *)"high",&viv_high);
						viv_read_int(start,(const utf8_t *)"slide_show_rate",&viv_slide_show_rate);
						viv_read_int(start,(const utf8_t *)"allow_shrinking",&viv_allow_shrinking);
						viv_read_int(start,(const utf8_t *)"shrink_blit_mode",&viv_shrink_blit_mode);
						viv_read_int(start,(const utf8_t *)"mag_filter",&viv_mag_filter);
						viv_read_int(start,(const utf8_t *)"keep_aspect_ratio",&viv_keep_aspect_ratio);
						viv_read_int(start,(const utf8_t *)"fill_window",&viv_fill_window);
						viv_read_int(start,(const utf8_t *)"fullscreen_fill_window",&viv_fullscreen_fill_window);
						viv_read_int(start,(const utf8_t *)"sort",&viv_nav_sort);
						viv_read_int(start,(const utf8_t *)"sort_ascending",&viv_nav_sort_ascending);
						viv_read_int(start,(const utf8_t *)"multiple_instances",&viv_multiple_instances);
						viv_read_int(start,(const utf8_t *)"show_status",&viv_is_show_status);
						
						if (is_root)
						{
							viv_read_int(start,(const utf8_t *)"appdata",&viv_appdata);
						}
					}
				}
			}
			
			mem_free(buf);
		}

		ret = 1;
	
		CloseHandle(h);
	}
	
	return ret;
}

static void viv_load_settings(void)
{
	wchar_t path[MAX_PATH];

	GetModuleFileNameW(0,path,MAX_PATH);
	PathRemoveFileSpecW(path);
	
	viv_load_settings_by_location(path,1);
	
	if (viv_appdata)
	{
		if (wstring_get_appdata_path(path))
		{
			wchar_t appdata_wbuf[MAX_PATH];
			
			PathCombine(appdata_wbuf,path,L"voidImageViewer");

			viv_load_settings_by_location(appdata_wbuf,0);
		}
	}
}

static int viv_is_ws(wchar_t c)
{
	switch(c)
	{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			return 1;
	}
	
	return 0;
}

static wchar_t *viv_skip_ws(wchar_t *p)
{
	while(*p)
	{
		if (!viv_is_ws(*p))
		{
			break;
		}
		
		p++;
	}
	
	return p;
}

static wchar_t *viv_get_word(wchar_t *p,wchar_t *buf)
{
	wchar_t *d;
	int is_quote;
	
	d = buf;
	is_quote = 0;

	while(*p)
	{
		if ((*p == '"') && (p[1] == '"'))
		{
			p += 2;
			*d++ = '"';
		}
		else
		if (*p == '"')
		{
			is_quote = !is_quote;
			p++;
		}
		else
		if ((!is_quote) && (viv_is_ws(*p)))
		{
			break;
		}
		else
		{
			*d++ = *p;
			p++;
		}
	}
	
	*d = 0;
	
	return p;
}

static void viv_process_command_line(wchar_t *cl)
{
	wchar_t *p;
	wchar_t buf[MAX_PATH];
	wchar_t single[MAX_PATH];
	int file_count;
	int start_slideshow;
	int start_fullscreen;
	int start_window;
	
	start_slideshow = 0;
	start_fullscreen = 0;
	start_window = 0;
	p = cl;
	file_count = 0;
	single[0] = 0;
	
	viv_dragdrop_clearall();
	
	p = viv_skip_ws(p);
	
	// skip filename
	p = viv_get_word(p,buf);
	p = viv_skip_ws(p);

	// skip first parameter.
	for(;;)
	{
		// no more text?
		if (!*p)
		{
			break;
		}
		
		p = viv_get_word(p,buf);
		p = viv_skip_ws(p);
		
		if (buf[0] == '/')
		{
			if (wcsicmp(buf+1,L"slideshow") == 0)
			{
				start_slideshow = 1;
			}
			else
			if (wcsicmp(buf+1,L"fullscreen") == 0)
			{
				start_fullscreen = 1;	
				start_window = 0;
			}
			else
			if (wcsicmp(buf+1,L"window") == 0)
			{
				start_fullscreen = 0;	
				start_window = 1;
			}
			else
			if (wcsicmp(buf+1,L"x") == 0)
			{
				p = viv_get_word(p,buf);
				p = viv_skip_ws(p);

				viv_x = wstring_to_int(buf);
			}
			else
			if (wcsicmp(buf+1,L"y") == 0)
			{
				p = viv_get_word(p,buf);
				p = viv_skip_ws(p);

				viv_y = wstring_to_int(buf);
			}		
			else
			if (wcsicmp(buf+1,L"rate") == 0)
			{
				p = viv_get_word(p,buf);
				p = viv_skip_ws(p);

				viv_slide_show_rate = wstring_to_int(buf);
			}
			else
			{
				MessageBox(0,L"Usage:\nvoidImageViewer.exe [/switches] [filename(s)]\n\nSwitches:\n/slideshow\tStart a slideshow.\n/fullscreen\tStart fullscreen\n/window\t\tStart windowed\n/rate <milliseconds>\tSet the slideshow rate in milliseconds.\n",L"voidImageViewer",MB_OK|MB_ICONINFORMATION);
			}
/*			
		viv_reg_write_dword(hkey,L"x",viv_x);
		viv_reg_write_dword(hkey,L"y",viv_y);
		viv_reg_write_dword(hkey,L"wide",viv_wide);
		viv_reg_write_dword(hkey,L"high",viv_high);
		viv_reg_write_dword(hkey,L"slide_show_rate",viv_slide_show_rate);
		viv_reg_write_dword(hkey,L"allow_shrinking",viv_allow_shrinking);
		viv_reg_write_dword(hkey,L"shrink_blit_mode",viv_shrink_blit_mode);
		viv_reg_write_dword(hkey,L"keep_aspect_ratio",viv_keep_aspect_ratio);
		viv_reg_write_dword(hkey,L"fill_window",viv_fill_window);
		viv_reg_write_dword(hkey,L"fullscreen_fill_window",viv_fullscreen_fill_window);
		viv_reg_write_dword(hkey,L"sort",viv_nav_sort);
		viv_reg_write_dword(hkey,L"sort_ascending",viv_nav_sort_ascending);
		viv_reg_write_dword(hkey,L"multiple_instances",viv_multiple_instances);
			*/
		}
		else
		{
			if (*buf)
			{
				if (file_count == 1)
				{
					// add the last single
					viv_dragdrop_add_filename(single);
				}

				if (file_count >= 1)
				{
					// add this file
					viv_dragdrop_add_filename(buf);
				}
			
				if (file_count == 0)
				{
					wstring_copy(single,MAX_PATH,buf);
				}
				
				file_count++;
			}
		}
	}

	if (file_count >= 1)
	{
		// open the first image specified.
		viv_open_from_full_filename(single);
	}
	else
	{
		viv_home(0);
	}
	
	if (start_slideshow)
	{
		viv_slide_show();
	}
	
	if (start_fullscreen)
	{
		if (!viv_is_fullscreen)
		{
			viv_toggle_fullscreen();
		}
	}
	
	if (start_window)
	{
		if (viv_is_fullscreen)
		{
			viv_toggle_fullscreen();
		}
	}
}

static int viv_init(void)
{
	WNDCLASSEXW wcex;
	RECT rect;
	HMENU file_menu;
	HMENU view_menu;
	HMENU play_menu;
	HMENU rate_menu;
	HMENU nav_menu;
	HMENU nav_sort_menu;
	HMENU animation_rate_menu;
	HMENU window_size_menu;
	HMENU preset_menu;
	HMENU ontop_menu;
	STARTUPINFO si;
	
	viv_current_fd = (WIN32_FIND_DATA *)mem_alloc(sizeof(WIN32_FIND_DATA));
	
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);
	
	// init common controls
	
	{
		INITCOMMONCONTROLSEX icex;

		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_WIN95_CLASSES;
		InitCommonControlsEx(&icex);
	}

	InitializeCriticalSection(&viv_cs);

	viv_shell32_hmodule = LoadLibrary(L"shell32.dll");
	if (viv_shell32_hmodule)
	{
		viv_SHOpenFolderAndSelectItems = (HRESULT (__stdcall *)(LPCITEMIDLIST,UINT,LPCITEMIDLIST *,DWORD))GetProcAddress(viv_shell32_hmodule,"SHOpenFolderAndSelectItems");
//		viv_SHOpenWithDialog = (HRESULT (__cdecl *)(HWND,const OPENASINFO *))GetProcAddress(viv_shell32_hmodule,"SHOpenWithDialog");
	}

	viv_UxTheme_hmodule = LoadLibrary(L"UxTheme.dll");
	if (viv_UxTheme_hmodule)
	{
		viv_EnableThemeDialogTexture = (HRESULT (__stdcall *)(HWND,DWORD))GetProcAddress(viv_UxTheme_hmodule,"EnableThemeDialogTexture");
	}

	// load settings
	viv_load_settings();
	
	// mutex
	if (!viv_multiple_instances)
	{
		SetLastError(0);

		viv_mutex = CreateMutexW(NULL,0,L"VOIDIMAGEVIEWER");
		
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			HWND hwnd;
			
			hwnd = FindWindow(L"VOIDIMAGEVIEWER",0);
			
			if (hwnd)
			{
				COPYDATASTRUCT cds;
				wchar_t *command_line;
				wchar_t cwd[MAX_PATH];
				int size;
				char *buf;
				char *d;

				// allow this process to set focus
				SetForegroundWindow(hwnd);
				
				command_line = GetCommandLineW();
				GetCurrentDirectory(MAX_PATH,cwd);
				
				si.cb = sizeof(STARTUPINFO);
				GetStartupInfo(&si);
				
				// calc size
				size = sizeof(DWORD) + (((int)wcslen(command_line) + 1) * sizeof(wchar_t)) + (((int)wcslen(cwd) + 1) * sizeof(wchar_t));
				buf = (char *)mem_alloc(size);
				
				// fill in
				d = buf;
				*(DWORD *)d = si.wShowWindow;
				d += sizeof(DWORD);
				StrCpy((wchar_t *)d,command_line);
				d += ((wcslen(command_line) + 1) * sizeof(wchar_t));
				StrCpy((wchar_t *)d,cwd);
				d += ((wcslen(cwd) + 1) * sizeof(wchar_t));

				// setup copydata struct
				cds.lpData = buf;
				cds.cbData = size;
				cds.dwData = 0;

				SendMessage(hwnd,WM_COPYDATA,(WPARAM)0,(LPARAM)&cds);
				
				mem_free(buf);
			}
			
			viv_kill();
			
			return 0;
		}
	}
	
	// Initialize global strings
	os_zero_memory(&wcex,sizeof(wcex));
	wcex.cbSize = sizeof(wcex); 

	wcex.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = (WNDPROC)viv_proc;
	wcex.hInstance = GetModuleHandle(0);
	wcex.hIcon = LoadIcon(GetModuleHandle(0),(LPCTSTR)IDI_ICON1);
	wcex.hCursor = LoadCursor(NULL,IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszClassName = L"VOIDIMAGEVIEWER";
	wcex.hIconSm = LoadIcon(wcex.hInstance,(LPCTSTR)IDI_ICON1);

	RegisterClassExW(&wcex);
	
	viv_main_menu = CreateMenu();
	file_menu = CreatePopupMenu();
	view_menu = CreatePopupMenu();
	play_menu = CreatePopupMenu();
	rate_menu = CreatePopupMenu();
	animation_rate_menu = CreatePopupMenu();
	nav_menu = CreatePopupMenu();
	nav_sort_menu = CreatePopupMenu();
	viv_nav_playlist_menu = CreatePopupMenu();
	window_size_menu = CreatePopupMenu();
	preset_menu = CreatePopupMenu();
	ontop_menu = CreatePopupMenu();

	AppendMenuW(viv_main_menu,MF_POPUP,(UINT_PTR)file_menu,L"&File");
	
	AppendMenuW(file_menu,MF_STRING,VIV_ID_FILE_OPEN_FILE_LOCATION,L"Open F&ile Location...");
	AppendMenuW(file_menu,MF_STRING,VIV_ID_FILE_EDIT,L"&Edit...");
	AppendMenuW(file_menu,MF_STRING,VIV_ID_FILE_PREVIEW,L"Pre&view...");
	AppendMenuW(file_menu,MF_STRING,VIV_ID_FILE_PRINT,L"&Print...");
	AppendMenuW(file_menu,MF_STRING,VIV_ID_FILE_SET_DESKTOP_WALLPAPER,L"Set &Desktop Wallpaper");
	AppendMenuW(file_menu,MF_SEPARATOR,0,0);
	AppendMenuW(file_menu,MF_STRING,VIV_ID_FILE_CUT,L"C&ut\tCtrl+X");
	AppendMenuW(file_menu,MF_STRING,VIV_ID_FILE_COPY,L"&Copy\tCtrl+C");
	AppendMenuW(file_menu,MF_STRING,VIV_ID_FILE_DELETE,L"&Delete\tDel");
	AppendMenuW(file_menu,MF_SEPARATOR,0,0);
	AppendMenuW(file_menu,MF_STRING,VIV_ID_FILE_PROPERTIES,L"P&roperties");
	AppendMenuW(file_menu,MF_SEPARATOR,0,0);
	AppendMenuW(file_menu,MF_STRING,VIV_ID_FILE_EXIT,L"E&xit");
	
	AppendMenuW(viv_main_menu,MF_POPUP,(UINT_PTR)view_menu,L"&View");
	
	AppendMenuW(view_menu,MF_STRING,VIV_ID_VIEW_MENU,L"&Menu");
	AppendMenuW(view_menu,MF_STRING,VIV_ID_VIEW_STATUS,L"&Status");
	
	AppendMenuW(preset_menu,MF_STRING,VIV_ID_VIEW_PRESET_1,L"&Minimal\t1");
	AppendMenuW(preset_menu,MF_STRING,VIV_ID_VIEW_PRESET_2,L"&Compact\t2");
	AppendMenuW(preset_menu,MF_STRING,VIV_ID_VIEW_PRESET_3,L"&Normal\t3");
	AppendMenuW(view_menu,MF_POPUP,(UINT_PTR)preset_menu,L"&Preset");
	
	AppendMenuW(view_menu,MF_SEPARATOR,0,0);
	AppendMenuW(view_menu,MF_STRING,VIV_ID_VIEW_ALLOW_SHRINKING,L"&Allow Shrinking");
	AppendMenuW(view_menu,MF_STRING,VIV_ID_VIEW_KEEP_ASPECT_RATIO,L"&Keep Aspect Ratio");
	AppendMenuW(view_menu,MF_STRING,VIV_ID_VIEW_FILL_WINDOW,L"&Fill Window");
	AppendMenuW(view_menu,MF_STRING,VIV_ID_VIEW_1TO1,L"1:1\tCtrl+Alt+0");
	AppendMenuW(view_menu,MF_STRING,VIV_ID_VIEW_FULLSCREEN,L"F&ullscreen\tAlt+Enter");
	AppendMenuW(view_menu,MF_STRING,VIV_ID_VIEW_SLIDE_SHOW,L"Play &Slideshow\tF11");
	AppendMenuW(view_menu,MF_SEPARATOR,0,0);
	AppendMenuW(window_size_menu,MF_STRING,VIV_ID_VIEW_WINDOW_SIZE_50,L"50%\tAlt+1");
	AppendMenuW(window_size_menu,MF_STRING,VIV_ID_VIEW_WINDOW_SIZE_100,L"100\tAlt+2");
	AppendMenuW(window_size_menu,MF_STRING,VIV_ID_VIEW_WINDOW_SIZE_200,L"200\tAlt+3");
	AppendMenuW(window_size_menu,MF_STRING,VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT,L"&Auto Fit");
	AppendMenuW(view_menu,MF_POPUP,(UINT_PTR)window_size_menu,L"&Window Size");
	AppendMenuW(view_menu,MF_SEPARATOR,0,0);
	AppendMenuW(ontop_menu,MF_STRING,VIV_ID_VIEW_ONTOP_ALWAYS,L"&Always\tCtrl+T");
	AppendMenuW(ontop_menu,MF_STRING,VIV_ID_VIEW_ONTOP_WHILE_PLAYING_OR_ANIMATING,L"&While playing or animating");
	AppendMenuW(ontop_menu,MF_STRING,VIV_ID_VIEW_ONTOP_NEVER,L"&Never");
	AppendMenuW(view_menu,MF_POPUP,(UINT_PTR)ontop_menu,L"On &Top");
	AppendMenuW(view_menu,MF_STRING,VIV_ID_VIEW_OPTIONS,L"Options...\tO");
		
	AppendMenuW(viv_main_menu,MF_POPUP,(UINT_PTR)play_menu,L"&Slideshow");
	
	AppendMenuW(play_menu,MF_STRING,VIV_ID_SLIDESHOW_PAUSE,L"&Play/Pause\tSpace");
	AppendMenuW(play_menu,MF_SEPARATOR,0,0);

//	AppendMenuW(play_menu,MF_POPUP,(UINT_PTR)rate_menu,L"&Rate");
	AppendMenuW(play_menu,MF_STRING,VIV_ID_SLIDESHOW_RATE_DEC,L"&Decrease Rate\tDown");
	AppendMenuW(play_menu,MF_STRING,VIV_ID_SLIDESHOW_RATE_INC,L"&Increase Rate\tUp");
	AppendMenuW(play_menu,MF_SEPARATOR,0,0);
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_250,L"250 Milliseconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_500,L"500 Milliseconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_1000,L"&1 Second");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_2000,L"&2 Seconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_3000,L"&3 Seconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_4000,L"&4 Seconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_5000,L"&5 Seconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_6000,L"&6 Seconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_7000,L"&7 Seconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_8000,L"&8 Seconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_9000,L"&9 Seconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_10000,L"1&0 Seconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_20000,L"20 Seconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_30000,L"30 Seconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_40000,L"40 Seconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_50000,L"50 Seconds");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_60000,L"1 Minute");
	AppendMenuW(play_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_SLIDESHOW_RATE_CUSTOM,L"Custom...");

	AppendMenuW(viv_main_menu,MF_POPUP,(UINT_PTR)animation_rate_menu,L"&Animation");
	
	AppendMenuW(animation_rate_menu,MF_STRING,VIV_ID_ANIMATION_PLAY_PAUSE,L"&Play/Pause\tCtrl+Space");
	AppendMenuW(animation_rate_menu,MF_SEPARATOR,0,0);
	AppendMenuW(animation_rate_menu,MF_STRING,VIV_ID_ANIMATION_JUMP_FORWARD_MEDIUM,L"Jump Forward\tCtrl+Right");
	AppendMenuW(animation_rate_menu,MF_STRING,VIV_ID_ANIMATION_JUMP_BACKWARD_MEDIUM,L"Jump Backward\tCtrl+Left");
	AppendMenuW(animation_rate_menu,MF_SEPARATOR,0,0);
//	AppendMenuW(animation_rate_menu,MF_STRING,VIV_ID_ANIMATION_FRAME_STEP,L"F&rame Step\tCtrl+Right");
//	AppendMenuW(animation_rate_menu,MF_STRING,VIV_ID_ANIMATION_FRAME_PREV,L"Pre&vious Frame\tCtrl+Left");
//	AppendMenuW(animation_rate_menu,MF_SEPARATOR,0,0);
	AppendMenuW(animation_rate_menu,MF_STRING,VIV_ID_ANIMATION_RATE_DEC,L"&Decrease Rate\tCtrl+Down");
	AppendMenuW(animation_rate_menu,MF_STRING,VIV_ID_ANIMATION_RATE_INC,L"&Increase Rate\tCtrl+Up");
	AppendMenuW(animation_rate_menu,MF_STRING,VIV_ID_ANIMATION_RATE_RESET,L"R&eset Rate\tCtrl+R");
	

	AppendMenuW(viv_main_menu,MF_POPUP,(UINT_PTR)nav_menu,L"&Navigate");
	
	AppendMenuW(nav_menu,MF_STRING,VIV_ID_NAV_NEXT,L"&Next\tRight Arrow");
	AppendMenuW(nav_menu,MF_STRING,VIV_ID_NAV_PREV,L"P&revious\tLeft Arrow");
	AppendMenuW(nav_menu,MF_STRING,VIV_ID_NAV_HOME,L"&Home\tHome");
	AppendMenuW(nav_menu,MF_STRING,VIV_ID_NAV_END,L"&End\tEnd");
	AppendMenuW(nav_menu,MF_SEPARATOR,0,0);
	AppendMenuW(nav_menu,MF_POPUP,(UINT_PTR)nav_sort_menu,L"S&ort");
	AppendMenuW(nav_menu,MF_POPUP,(UINT_PTR)viv_nav_playlist_menu,L"Playlist");
	AppendMenuW(nav_sort_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_NAV_SORT_NAME,L"&Name");
	AppendMenuW(nav_sort_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_NAV_SORT_SIZE,L"&Size");
	AppendMenuW(nav_sort_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_NAV_SORT_DATE_MODIFIED,L"Date &Modified");
	AppendMenuW(nav_sort_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_NAV_SORT_DATE_CREATED,L"Date &Created");
	AppendMenuW(nav_sort_menu,MF_SEPARATOR,0,0);
	AppendMenuW(nav_sort_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_NAV_SORT_ASCENDING,L"&Ascending");
	AppendMenuW(nav_sort_menu,MF_STRING|MF_UNCHECKED|MFT_RADIOCHECK,VIV_ID_NAV_SORT_DESCENDING,L"&Descending");
	
	rect.left = viv_x;
	rect.top = viv_y;
	rect.right = viv_x + viv_wide;
	rect.bottom = viv_y + viv_high;
	
	os_make_rect_completely_visible(&rect);
	
	viv_hwnd = CreateWindowExW(
		0,
		L"VOIDIMAGEVIEWER",
		L"void Image Viewer",
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,
		0,viv_main_menu,GetModuleHandle(0),NULL);
		
	viv_status_show(viv_is_show_status);

	DragAcceptFiles(viv_hwnd,TRUE);

	viv_update_title();

	viv_opengl_init();

	viv_process_command_line(GetCommandLineW());

	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);

	ShowWindow(viv_hwnd,si.wShowWindow);
	UpdateWindow(viv_hwnd);
	
	return 1;
}

static void viv_kill(void)
{
	// don't load another image..
	if (viv_load_image_next_fd)
	{
		mem_free(viv_load_image_next_fd);
		
		viv_load_image_next_fd = 0;
	}
	
	// stop load_image immediately...
	if (viv_load_image_thread)
	{
		viv_load_image_terminate = 1;
		
		// it's critical we wait for load image to finish before we kill the main window.
		WaitForSingleObject(viv_load_image_thread,INFINITE);
		
		CloseHandle(viv_load_image_thread);
	}
	
	if (viv_load_image_filename)
	{
		mem_free(viv_load_image_filename);
	}
	
	viv_clear();

	if (viv_hwnd)
	{
		DestroyWindow(viv_hwnd);
	}
	
	viv_nav_item_free_all();
	
	if (viv_main_menu)
	{
		DestroyMenu(viv_main_menu);
	}

	if (viv_mutex)
	{
		CloseHandle(viv_mutex);
	}

	viv_reply_clear_all();

	viv_opengl_destroy();

	if (viv_shell32_hmodule)
	{
		FreeLibrary(viv_shell32_hmodule);
	}
	
	if (viv_UxTheme_hmodule)
	{
		FreeLibrary(viv_UxTheme_hmodule);
	}
	
	if (viv_stobject_hmodule)
	{
		FreeLibrary(viv_stobject_hmodule);
	}

	if (viv_opengl32_hmodule)
	{
		FreeLibrary(viv_opengl32_hmodule);
	}

	CoUninitialize();
	
	DeleteCriticalSection(&viv_cs);

	mem_free(viv_current_fd);
}

static int viv_main(void)
{
	if (viv_init())
	{

		for(;;)
		{
			viv_frame_changed = 0;
			
			// Main message loop:
			for(;;)
			{
				MSG msg;
				
				if (!PeekMessage(&msg,0,0,0,PM_REMOVE)) break;
				
				if (msg.message == WM_QUIT) goto exit;
				
				if (!viv_is_msg(&msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			
			if (viv_frame_changed)	
			{
				viv_frame_changed = 0;
				InvalidateRect(viv_hwnd,0,FALSE);
			}
			
			WaitMessage();
		}
		
	exit:
		
		
		viv_kill();
	}

	return 0;
}

int APIENTRY WinMain(HINSTANCE ,HINSTANCE ,LPSTR ,int )
{
	return viv_main();
}

int __cdecl main(int ,char **)
{
	return viv_main();
}

static void viv_write_int(HANDLE h,const utf8_t *key,int value)
{
	unsigned char buf[MAX_PATH];
	wchar_t wbuf[MAX_PATH];
	
	wstring_format_number(wbuf,value);
	
	WideCharToMultiByte(CP_UTF8,0,wbuf,-1,(char *)buf,MAX_PATH,0,0);
	
	viv_write_utf8(h,(const unsigned char *)key);
	viv_write_utf8(h,(const unsigned char *)"=");
	viv_write_utf8(h,buf);
	viv_write_utf8(h,(const unsigned char *)"\r\n");
}

static void viv_write_utf8(HANDLE h,const unsigned char *s)
{
	DWORD num_written;
	
	WriteFile(h,s,string_length(s),&num_written,0);
}

static int viv_save_settings_by_location(const wchar_t *path,int is_root)
{
	HANDLE h;
	wchar_t tempname[MAX_PATH];
	wchar_t filename[MAX_PATH];
	int ret;
	
	ret = 0;
	
	PathCombineW(filename,path,L"voidImageViewer.ini");

	wstring_copy(tempname,MAX_PATH,filename);
	wstring_cat(tempname,MAX_PATH,L".tmp");

	h = CreateFileW(tempname,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if (h != INVALID_HANDLE_VALUE)
	{
		viv_write_utf8(h,(const utf8_t *)"[voidImageViewer]\r\n");
		
		if ((is_root) && (viv_appdata))
		{
			viv_write_int(h,(const utf8_t *)"appdata",viv_appdata);
		}
		else
		{
			viv_write_int(h,(const utf8_t *)"x",viv_x);
			viv_write_int(h,(const utf8_t *)"y",viv_y);
			viv_write_int(h,(const utf8_t *)"wide",viv_wide);
			viv_write_int(h,(const utf8_t *)"high",viv_high);
			viv_write_int(h,(const utf8_t *)"slide_show_rate",viv_slide_show_rate);
			viv_write_int(h,(const utf8_t *)"allow_shrinking",viv_allow_shrinking);
			viv_write_int(h,(const utf8_t *)"shrink_blit_mode",viv_shrink_blit_mode);
			viv_write_int(h,(const utf8_t *)"mag_filter",viv_mag_filter);
			viv_write_int(h,(const utf8_t *)"keep_aspect_ratio",viv_keep_aspect_ratio);
			viv_write_int(h,(const utf8_t *)"fill_window",viv_fill_window);
			viv_write_int(h,(const utf8_t *)"fullscreen_fill_window",viv_fullscreen_fill_window);
			viv_write_int(h,(const utf8_t *)"sort",viv_nav_sort);
			viv_write_int(h,(const utf8_t *)"sort_ascending",viv_nav_sort_ascending);
			viv_write_int(h,(const utf8_t *)"multiple_instances",viv_multiple_instances);
			viv_write_int(h,(const utf8_t *)"show_status",viv_is_show_status);

		}

		CloseHandle(h);

		if (!MoveFileExW(tempname,filename,MOVEFILE_REPLACE_EXISTING))
		{
			if (CopyFile(tempname,filename,FALSE))
			{
				DeleteFile(tempname);
			}
		}

		ret = 1;
	}
	
	return ret;
}

static void viv_save_settings(int appdata)
{
	if (appdata)
	{
		wchar_t path[MAX_PATH];

		if (wstring_get_appdata_path(path))
		{
			wchar_t appdata_wbuf[MAX_PATH];
			
			PathCombine(appdata_wbuf,path,L"voidImageViewer");

			viv_save_settings_by_location(appdata_wbuf,0);
		}
	}
	else
	{
		wchar_t path[MAX_PATH];
		
		GetModuleFileNameW(0,path,MAX_PATH);
		PathRemoveFileSpecW(path);

		viv_save_settings_by_location(path,1);
	}
}

static void os_MonitorRectFromWindowRect(const RECT *window_rect,RECT *monitor_rect)
{
	HMONITOR hmonitor;
	MONITORINFO mi;
	
	hmonitor = MonitorFromRect(window_rect,MONITOR_DEFAULTTOPRIMARY);
	if (hmonitor)
	{
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(hmonitor,&mi);
		
		CopyRect(monitor_rect,&mi.rcWork);
	}
	else
	{
		// work area
		SystemParametersInfo(SPI_GETWORKAREA,0,(PVOID)monitor_rect,0);
	}
}

// make a rect fully visible
// pushes the rect onto the screen,the same as menus.
// does not allow monitor overlapping.
// if you have a window that is fully visible, but half is displayed on one display, and half on another, this function will
// push it onto just one of the displays. (the one with the most area shown)
// make sure you call os_is_rect_completely_visible before calling this.
static void os_make_rect_completely_visible(RECT *prect)
{
	int wide;
	int high;
	RECT monitor_rect;

	// we hit something so reposition the window on this monitor.
	// note that MonitorFromRect can return null, even when MONITOR_DEFAULTTOPRIMARY is specified.
	os_MonitorRectFromWindowRect(prect,&monitor_rect);
	
	// get window width and height,
	wide = prect->right - prect->left;
	high = prect->bottom - prect->top;

	// push the window 
	if (prect->right > monitor_rect.right) 
	{
		prect->left = monitor_rect.right - wide;
		prect->right = monitor_rect.right;
	}
	
	if (prect->bottom > monitor_rect.bottom) 
	{
		prect->top = monitor_rect.bottom - high;
		prect->bottom = monitor_rect.bottom;
	}

	// push the window 
	if (prect->left < monitor_rect.left) 
	{
		prect->left = monitor_rect.left;
		prect->right = monitor_rect.left + wide;
	}
	
	if (prect->top < monitor_rect.top) 
	{
		prect->top = monitor_rect.top;
		prect->bottom = monitor_rect.top + high;
	}
}

static wchar_t *string_alloc(const wchar_t *s)
{
	int wlen;
	wchar_t *p;
	
	wlen = wstring_length(s);
	p = (wchar_t *)mem_alloc((wlen + 1) * sizeof(wchar_t));
	
	CopyMemory(p,s,(wlen + 1) * sizeof(wchar_t));
		
	return p;
}

static int _viv_compare(WIN32_FIND_DATA *a,WIN32_FIND_DATA *b)
{
	int ret;
	
	ret = 0;
	
	switch(viv_nav_sort)
	{
		case VIV_NAV_SORT_NAME:
			ret = wcsicmp(PathFindFileName(a->cFileName),PathFindFileName(b->cFileName));
			break;

		case VIV_NAV_SORT_SIZE:
		{
			LARGE_INTEGER sizea;
			LARGE_INTEGER sizeb;
			
			sizea.HighPart = a->nFileSizeHigh;
			sizea.LowPart = a->nFileSizeLow;

			sizeb.HighPart = b->nFileSizeHigh;
			sizeb.LowPart = b->nFileSizeLow;

			if (sizea.QuadPart < sizeb.QuadPart)
			{
				ret = -1;
			}
			else
			if (sizea.QuadPart > sizeb.QuadPart)
			{
				ret = 1;
			}
			else
			{
				// we want name ascending when we are size descending.
				ret = -wcsicmp(PathFindFileName(a->cFileName),PathFindFileName(b->cFileName));
			}
			
			break;
		}

		case VIV_NAV_SORT_DATE_MODIFIED:
		{
			LARGE_INTEGER datea;
			LARGE_INTEGER dateb;
			
			datea.HighPart = a->ftLastWriteTime.dwHighDateTime;
			datea.LowPart = a->ftLastWriteTime.dwLowDateTime;

			dateb.HighPart = b->ftLastWriteTime.dwHighDateTime;
			dateb.LowPart = b->ftLastWriteTime.dwLowDateTime;

			if (datea.QuadPart < dateb.QuadPart)
			{
				ret = -1;
			}
			else
			if (datea.QuadPart > dateb.QuadPart)
			{
				ret = 1;
			}
			else
			{
				ret = -wcsicmp(PathFindFileName(a->cFileName),PathFindFileName(b->cFileName));
			}
			
			break;
		}

		case VIV_NAV_SORT_DATE_CREATED:
		{
			LARGE_INTEGER datea;
			LARGE_INTEGER dateb;
			
			datea.HighPart = a->ftCreationTime.dwHighDateTime;
			datea.LowPart = a->ftCreationTime.dwLowDateTime;

			dateb.HighPart = b->ftCreationTime.dwHighDateTime;
			dateb.LowPart = b->ftCreationTime.dwLowDateTime;

			if (datea.QuadPart < dateb.QuadPart)
			{
				ret = -1;
			}
			else
			if (datea.QuadPart > dateb.QuadPart)
			{
				ret = 1;
			}
			else
			{
				ret = -wcsicmp(PathFindFileName(a->cFileName),PathFindFileName(b->cFileName));
			}
			
			break;
		}
	}
	
	if (!viv_nav_sort_ascending)
	{
		ret *= -1;
	}
	
	return ret;
}

static void viv_next(int prev)
{
	if (*viv_current_fd->cFileName)	
	{
		WIN32_FIND_DATA start_fd;
		WIN32_FIND_DATA best_fd;
		int got_start;
		int got_best;

		got_best = 0;
		got_start = 0;
		
		if (viv_dragdrop_start)
		{
			viv_dragdrop_t *d;
			WIN32_FIND_DATA fd;
			
			d = viv_dragdrop_start;
			while(d)
			{
				int compare_ret;
				
				wstring_copy(fd.cFileName,MAX_PATH,d->filename);
				
				viv_fd_copy_fileinfo(&fd,&d->fileinfo);
				
				compare_ret = _viv_compare(&fd,viv_current_fd);

				if (compare_ret != 0)
				{
					if (prev)
					{
						if (compare_ret < 0)
						{
							if ((!got_best) || (_viv_compare(&fd,&best_fd) > 0))
							{
								os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
								got_best = 1;
							}
						}
					}
					else
					{
						if (compare_ret > 0)
						{
							if ((!got_best) || (_viv_compare(&fd,&best_fd) < 0))
							{
								os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
								got_best = 1;
							}
						}		
					}
				}
				
				// compare with start..
				if (prev)
				{
					if ((!got_start) || (_viv_compare(&fd,&start_fd) > 0))
					{
						os_copy_memory(&start_fd,&fd,sizeof(WIN32_FIND_DATA));
						
						got_start = 1;
					}
				}
				else
				{
					if ((!got_start) || (_viv_compare(&fd,&start_fd) < 0))
					{
						os_copy_memory(&start_fd,&fd,sizeof(WIN32_FIND_DATA));
						got_start = 1;
					}
				}					
			
				d = d->next;
			}
		}
		else
		{
			WIN32_FIND_DATA fd;
			HANDLE h;
			
			h = FindFirstFile(L"*.*",&fd);
			if (h != INVALID_HANDLE_VALUE)
			{
				for(;;)
				{
					if (viv_is_valid_filename(fd.cFileName))
					{
						int compare_ret;
						
						compare_ret = _viv_compare(&fd,viv_current_fd);

						if (compare_ret != 0)
						{
							if (prev)
							{
								if (compare_ret < 0)
								{
									if ((!got_best) || (_viv_compare(&fd,&best_fd) > 0))
									{
										os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
										got_best = 1;
									}
								}
							}
							else
							{
								if (compare_ret > 0)
								{
									if ((!got_best) || (_viv_compare(&fd,&best_fd) < 0))
									{
										os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
										got_best = 1;
									}
								}		
							}
						}
						
						// compare with start..
						if (prev)
						{
							if ((!got_start) || (_viv_compare(&fd,&start_fd) > 0))
							{
								os_copy_memory(&start_fd,&fd,sizeof(WIN32_FIND_DATA));
								
								got_start = 1;
							}
						}
						else
						{
							if ((!got_start) || (_viv_compare(&fd,&start_fd) < 0))
							{
								os_copy_memory(&start_fd,&fd,sizeof(WIN32_FIND_DATA));
								got_start = 1;
							}
						}
					}
					
					if (!FindNextFile(h,&fd)) break;
				}

				FindClose(h);
			}
		}

		if (got_best)
		{
			viv_open(&best_fd);
		}
		else
		if (got_start)
		{
			// don't open the same image again.
			if (wcsicmp(start_fd.cFileName,viv_current_fd->cFileName) != 0)
			{
				viv_open(&start_fd);
			}
		}
		else
		{
			viv_blank();
		}
	}
	else
	{
		viv_home(0);
	}
}

static void viv_home(int end)
{
	WIN32_FIND_DATA fd;
	WIN32_FIND_DATA best_fd;
	int got_best;
	
	got_best = 0;
	
	if (viv_dragdrop_start)
	{
		viv_dragdrop_t *d;
		
		d = viv_dragdrop_start;
		while(d)
		{
//printf("%S\n",d->filename);

			wstring_copy(fd.cFileName,MAX_PATH,d->filename);
			viv_fd_copy_fileinfo(&fd,&d->fileinfo);
			
			if (end)
			{
				if ((!got_best) || (_viv_compare(&fd,&best_fd) > 0))
				{
					os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
					got_best = 1;
				}
			}
			else
			{
				if ((!got_best) || (_viv_compare(&fd,&best_fd) < 0))
				{
					os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
					got_best = 1;
				}		
			}					
		
			d = d->next;
		}
	}
	else
	{
		HANDLE h;
	
		h = FindFirstFile(L"*.*",&fd);
		
		if (h != INVALID_HANDLE_VALUE)
		{
			for(;;)
			{
				if (viv_is_valid_filename(fd.cFileName))
				{
					if (end)
					{
						if ((!got_best) || (_viv_compare(&fd,&best_fd) > 0))
						{
							os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
							got_best = 1;
						}
					}
					else
					{
						if ((!got_best) || (_viv_compare(&fd,&best_fd) < 0))
						{
							os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
							got_best = 1;
						}		
					}
				}
				
				if (!FindNextFile(h,&fd)) break;
			}

			FindClose(h);
		}
	}
	
	if (got_best)
	{	
		viv_open(&best_fd);
	}
	else
	{
		viv_blank();
	}
}

static int viv_is_valid_filename(wchar_t *s)
{
	wchar_t *e;
	
	e = 0;
	while(*s)
	{
		if (*s == '.')
		{
			e = s + 1;
		}
		
		s++;
	}

	if (e)
	{
		if (wcsicmp(e,L"jpg") == 0) return 1;
		if (wcsicmp(e,L"jpeg") == 0) return 1;
		if (wcsicmp(e,L"png") == 0) return 1;
		if (wcsicmp(e,L"gif") == 0) return 1;
		if (wcsicmp(e,L"bmp") == 0) return 1;
		if (wcsicmp(e,L"tif") == 0) return 1;
		if (wcsicmp(e,L"tiff") == 0) return 1;
		if (wcsicmp(e,L"ico") == 0) return 1;
	}
	
	return 0;
}

static int viv_is_msg(MSG *msg)
{
	if (msg->hwnd == viv_hwnd)
	{
		switch (msg->message)
		{
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:
			
				switch(msg->wParam)
				{
					case VK_OEM_MINUS:
					case VK_SUBTRACT:
					{
						RECT rect;
						POINT pt;
						GetClientRect(viv_hwnd,&rect);
						pt.x = (rect.right - rect.left) / 2;
						pt.y = (rect.bottom - rect.top - viv_get_status_high()) / 2;
						ClientToScreen(viv_hwnd,&pt);
						SendMessage(viv_hwnd,WM_MOUSEWHEEL,MAKEWPARAM(0,-120),MAKELPARAM(pt.x,pt.y));
						break;
					}
						
					case VK_OEM_PLUS:
					case VK_ADD:
					{
						RECT rect;
						POINT pt;
						GetClientRect(viv_hwnd,&rect);
						pt.x = (rect.right - rect.left) / 2;
						pt.y = (rect.bottom - rect.top - viv_get_status_high()) / 2;
						ClientToScreen(viv_hwnd,&pt);
						SendMessage(viv_hwnd,WM_MOUSEWHEEL,MAKEWPARAM(0,120),MAKELPARAM(pt.x,pt.y));
						break;
					}
						
					case '0':
						if (viv_is_key_state(1,0,1))
						{
							viv_view_1to1();
							return 1;
						}
						else
						if (viv_is_key_state(1,0,0))
						{
							viv_1to1 = 0;
							viv_zoom_pos = 0;
							viv_view_set(viv_view_x,viv_view_y);
							InvalidateRect(viv_hwnd,0,FALSE);
							return 1;
						}
						
						break;
						
					case '1':
					case '2':
					case '3':
					case '4':
						if (viv_is_key_state(0,0,1))
						{
							switch(msg->wParam)
							{
								case '1':
									viv_command(VIV_ID_VIEW_WINDOW_SIZE_50);
									break;
									
								case '2':
									viv_command(VIV_ID_VIEW_WINDOW_SIZE_100);
									break;
									
								case '3':
									viv_command(VIV_ID_VIEW_WINDOW_SIZE_200);
									break;
									
								case '4':
									viv_command(VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT);
									break;
							}
														
							return 1;
						}
						else
						if (viv_is_key_state(0,0,0))
						{
							switch(msg->wParam)
							{
								case '1':
									viv_command(VIV_ID_VIEW_PRESET_1);
									return 1;

								case '2':
									viv_command(VIV_ID_VIEW_PRESET_2);
									return 1;

								case '3':
									viv_command(VIV_ID_VIEW_PRESET_3);
									return 1;
							}
						}
						
						break;
												
					case 'O':
						viv_options();
						return 1;
						
					case VK_SPACE:
						if (viv_is_key_state(1,0,0))
						{
							viv_animation_pause();
						}
						else
						if (viv_is_key_state(0,0,0))
						{
							viv_pause();
						}
						return 1;
						
					case 'C':
						if (viv_is_key_state(1,0,0))
						{
							viv_copy(0);
							return 1;
						}
						
						break;	
												
					case 'R':
					
						if (viv_is_key_state(1,0,0))
						{
							viv_reset_animation_rate();
							return 1;
						}
						
						break;		

					case 'X':
						if (viv_is_key_state(1,0,0))
						{
							viv_copy(1);
							return 1;
						}
						
						break;		
			
					case VK_DELETE:
						viv_delete();
						return 1;
						
					case VK_RIGHT:
					case VK_NEXT:
						if (viv_is_key_state(1,0,0))
						{
							viv_command(VIV_ID_ANIMATION_JUMP_FORWARD_MEDIUM);
						}
						else
						{
							viv_next(0);
						}
						return 1;
						
					case VK_LEFT:
					case VK_PRIOR:
						if (viv_is_key_state(1,0,0))
						{
							viv_command(VIV_ID_ANIMATION_JUMP_BACKWARD_MEDIUM);
						}
						else
						{
							viv_next(1);
						}
						return 1;
						
					case VK_UP:
						if (viv_is_key_state(0,0,0))
						{
							viv_increase_rate(0);
							return 1;
						}					
						if (viv_is_key_state(1,0,0))
						{
							viv_increase_animation_rate(0);
							return 1;
						}
						break;
						
					case VK_DOWN:
						if (viv_is_key_state(0,0,0))
						{
							viv_increase_rate(1);
							return 1;
						}
						if (viv_is_key_state(1,0,0))
						{
							viv_increase_animation_rate(1);
							return 1;
						}
						break;
						
					case VK_HOME:
						viv_home(0);
						return 1;
						
					case VK_END:
						viv_home(1);
						return 1;
						
					case VK_F11:
						viv_slide_show();
						return 1;
						
					case VK_ESCAPE:

						if (viv_is_slideshow)
						{
							KillTimer(viv_hwnd,VIV_ID_SLIDE_SHOW_TIMER);
							
							viv_is_slideshow = 0;

							viv_update_ontop();
						}
				
						if (viv_is_fullscreen)
						{
							viv_toggle_fullscreen();
							
							return 1;
						}
					
						break;

					case VK_RETURN:

						if (viv_is_key_state(0,0,1))
						{
							viv_toggle_fullscreen();
							return 1;
						}
						
						break;
						
					case VK_NUMPAD7:
						if (viv_is_key_state(1,0,0))
						{
							dst_pos_set(viv_dst_pos_x - 5,viv_dst_pos_y - 5);
						}
						break;

					case VK_NUMPAD8:
						if (viv_is_key_state(1,0,0))
						{
							dst_pos_set(viv_dst_pos_x,viv_dst_pos_y - 5);
						}
						else
						if (viv_is_key_state(0,0,0))
						{
							dst_zoom_set(viv_dst_zoom_x,viv_dst_zoom_y * 1.02f);
						}
						break;

					case VK_NUMPAD9:
						if (viv_is_key_state(1,0,0))
						{
							dst_pos_set(viv_dst_pos_x + 5,viv_dst_pos_y - 5);
						}
						else
						if (viv_is_key_state(0,0,0))
						{
							dst_zoom_set(viv_dst_zoom_x * 1.02f,viv_dst_zoom_y * 1.02f);
						}
						
						break;	
											
					case VK_NUMPAD4:
						if (viv_is_key_state(1,0,0))
						{
							dst_pos_set(viv_dst_pos_x - 5,viv_dst_pos_y);
						}
						else
						if (viv_is_key_state(0,0,0))
						{
							dst_zoom_set(viv_dst_zoom_x * 0.98f,viv_dst_zoom_y);
						}
						break;

					case VK_NUMPAD6:
						if (viv_is_key_state(1,0,0))
						{
							dst_pos_set(viv_dst_pos_x + 5,viv_dst_pos_y);
						}
						else
						if (viv_is_key_state(0,0,0))
						{
							dst_zoom_set(viv_dst_zoom_x * 1.02f,viv_dst_zoom_y);
						}
						
						break;
						
					case VK_NUMPAD1:
						if (viv_is_key_state(1,0,0))
						{
							dst_pos_set(viv_dst_pos_x - 5,viv_dst_pos_y + 5);
						}
						else
						if (viv_is_key_state(0,0,0))
						{
							dst_zoom_set(viv_dst_zoom_x * 0.98f,viv_dst_zoom_y * 0.98f);
						}
						break;

					case VK_NUMPAD2:
						if (viv_is_key_state(1,0,0))
						{
							dst_pos_set(viv_dst_pos_x,viv_dst_pos_y + 5);
						}
						else
						if (viv_is_key_state(0,0,0))
						{
							dst_zoom_set(viv_dst_zoom_x,viv_dst_zoom_y * 0.98f);
						}
						break;

					case VK_NUMPAD3:
						if (viv_is_key_state(1,0,0))
						{
							dst_pos_set(viv_dst_pos_x + 5,viv_dst_pos_y + 5);
						}
						
						break;
						
					case VK_NUMPAD5:
						if (viv_is_key_state(1,0,0))
						{
							viv_dst_pos_x = 500;
							viv_dst_pos_y = 500;
							InvalidateRect(viv_hwnd,NULL,FALSE);
						}
						else
						if (viv_is_key_state(0,0,0))
						{
							dst_zoom_set(1.0f,1.0f);
						}
						
						break;
				}

				break;
		}
	}

	return 0;
}

static void viv_view_set(int view_x,int view_y)
{
	RECT rect;
	int wide;
	int high;
	int rx;
	int ry;
	int rw;
	int rh;
	
	GetClientRect(viv_hwnd,&rect);
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top - viv_get_status_high();

	viv_get_render_size(&rw,&rh);
/*		
		if (viv_zoom_pos == 1)
		{
			if ((rw < viv_image_wide) || (rw < viv_image_wide))
			{
				rw = viv_image_wide;
				rh = viv_image_high;
			}
		}
*/
		
	rx = (wide / 2) - (rw / 2) - view_x;
	ry = (high / 2) - (rh / 2) - view_y;

	if (viv_keep_centered)
	{
		if (rw > wide)
		{
			if (rx > 0)
			{
				view_x = (wide / 2) - (rw / 2);
			}

			if (rx + rw < wide)
			{
				// rx = wide - rw;
				view_x = (wide / 2) - (rw / 2) - (wide - rw);
			}
		}
		else
		{
			view_x = 0;
		}
	
		if (rh > high)
		{
			if (ry > 0)
			{
				view_y = (high / 2) - (rh / 2);
			}

			if (ry + rh < high)
			{
				view_y = (high / 2) - (rh / 2) - (high - rh);
			}
		}
		else
		{
			view_y = 0;
		}
	}
	else
	{
		if (rx + rw - 1 < 0)
		{
			view_x = (wide / 2) - (rw / 2) + rw - 1;
		}
		
		if (rx > wide-1)
		{
			view_x = (wide / 2) - (rw / 2) - (wide-1);
		}
		
		if (ry + rh - 1 < 0)
		{
			view_y = (high / 2) - (rh / 2) + rh - 1;
		}
		
		if (ry > high-1)
		{
			view_y = (high / 2) - (rh / 2) - (high-1);
		}
	}
	
	if ((viv_view_x != view_x) || (viv_view_y != view_y))
	{
		viv_view_x = view_x;
		viv_view_y = view_y;
		viv_view_ix = (double)view_x / (double)rw;
		viv_view_iy = (double)view_y / (double)rh;

		InvalidateRect(viv_hwnd,0,FALSE);
	}
}

static void viv_toggle_fullscreen(void)
{
	DWORD style;
	
	style = GetWindowLong(viv_hwnd,GWL_STYLE);
	
	viv_1to1 = 0;
	
	if (viv_is_fullscreen)
	{
		if (viv_is_show_caption)	
		{
			style |= WS_CAPTION | WS_SYSMENU;
		}
		else
		{
			style &= ~(WS_CAPTION | WS_SYSMENU);
		}
		
		if (viv_is_show_thickframe)	
		{
			style |= WS_THICKFRAME;
		}
		else
		{
			style &= ~WS_THICKFRAME;
		}

		if (viv_is_show_menu)	
		{
			SetMenu(viv_hwnd,viv_main_menu);
		}
		else
		{
			SetMenu(viv_hwnd,0);
		}
		
		viv_status_show(viv_is_show_status);
		
		SetWindowLong(viv_hwnd,GWL_STYLE,style);

		SetWindowPos(viv_hwnd,HWND_TOP,viv_fullscreen_rect.left,viv_fullscreen_rect.top,viv_fullscreen_rect.right - viv_fullscreen_rect.left,viv_fullscreen_rect.bottom - viv_fullscreen_rect.top,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOCOPYBITS);
	
		if (viv_fullscreen_is_maxed)
		{	
			ShowWindow(viv_hwnd,SW_MAXIMIZE);
		}
		
		KillTimer(viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER);
		viv_is_hide_cursor_timer = 0;
		
		if (!viv_cursor_shown)
		{
			ShowCursor(TRUE);
			
			viv_cursor_shown = 1;
		}
	}
	else
	{
		HMONITOR hmonitor;
		MONITORINFO mi;
		
		hmonitor = MonitorFromWindow(viv_hwnd,MONITOR_DEFAULTTOPRIMARY);
		
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hmonitor,&mi);
		
		// clear zooming if the current render size is smaller than the full screen size.
		/*
		if (viv_zoom_pos)
		{
			int rw;
			int rh;
			
			viv_get_render_size(&rw,&rh);
			
			if ((rw <= mi.rcMonitor.right - mi.rcMonitor.left) &&  (rh <= mi.rcMonitor.bottom - mi.rcMonitor.top))
			{
				viv_zoom_pos = 0;
			}
		}*/
		
		SetWindowLong(viv_hwnd,GWL_STYLE,style & ~(WS_CAPTION|WS_THICKFRAME|WS_SYSMENU));
		
		viv_fullscreen_is_maxed = IsZoomed(viv_hwnd);
		if (viv_fullscreen_is_maxed)
		{
			ShowWindow(viv_hwnd,SW_SHOWNORMAL);
		}
		
		GetWindowRect(viv_hwnd,&viv_fullscreen_rect);
		
		SetMenu(viv_hwnd,0);
		viv_status_show(0);

		SetWindowPos(viv_hwnd,HWND_TOP,mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right - mi.rcMonitor.left,mi.rcMonitor.bottom - mi.rcMonitor.top,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOCOPYBITS);

		SetTimer(viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER,VIV_HIDE_CURSOR_DELAY,0);
		viv_is_hide_cursor_timer = 1;
		
		{
			POINT pt;
			
			GetCursorPos(&pt);
			
			viv_hide_cursor_x = pt.x;
			viv_hide_cursor_y = pt.y;
		}
	}
	
	viv_is_fullscreen = !viv_is_fullscreen;
	
//	viv_on_size();
//	InvalidateRect(viv_hwnd,0,FALSE);
}

static void viv_slide_show(void)
{
	if (!viv_is_fullscreen)
	{
		viv_toggle_fullscreen();
	}
	
	if (!viv_is_slideshow)
	{
		SetTimer(viv_hwnd,VIV_ID_SLIDE_SHOW_TIMER,viv_slide_show_rate,0);
		
		viv_is_slideshow = 1;
		viv_update_ontop();
	}
}

static void viv_get_render_size(int *prw,int *prh)
{
	RECT rect;
	int wide;
	int high;
	int rw;
	int rh;
	int fill_window;
	int max_zoom_wide;
	int max_zoom_high;
	
	if (!((viv_image_wide) && (viv_image_high)))
	{
		*prw = 0;
		*prh = 0;
		
		return;
	}
	
	GetClientRect(viv_hwnd,&rect);
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top - viv_get_status_high();
	
	if (!((wide) && (high)))
	{
		*prw = 0;
		*prh = 0;
		
		return;
	}
		
	if (viv_is_fullscreen)
	{
		fill_window = viv_fullscreen_fill_window;
	}
	else
	{
		fill_window = viv_fill_window;
	}
	
	if (viv_keep_aspect_ratio)
	{
		if ((high * viv_image_wide) / viv_image_high < wide)
		{
			// tall image.
			rh = high;
			rw = (high * viv_image_wide) / viv_image_high;
		}
		else
		{
			// long image.
			rw = wide;
			rh = (wide * viv_image_high) / viv_image_wide;
		}

		if (!fill_window)
		{
			if ((rw > viv_image_wide) || (rh > viv_image_high))
			{
				rw = viv_image_wide;
				rh = viv_image_high;
			}
		}
	}
	else
	{
		rw = wide;
		rh = high;

		if (!fill_window)
		{
			if (rw > viv_image_wide)
			{
				rw = viv_image_wide;
			}			
	
			if (rh > viv_image_high)
			{
				rh = viv_image_high;
			}
		}
	}

	if (!viv_allow_shrinking)
	{
		if (viv_keep_aspect_ratio)
		{
			if ((rw < viv_image_wide) || (rh < viv_image_high))
			{
				rw = viv_image_wide;
				rh = viv_image_high;
			}
		}
		else
		{
			if (rw < viv_image_wide)
			{
				rw = viv_image_wide;
			}

			if (rh < viv_image_high)
			{
				rh = viv_image_high;
			}
		}
	}		
	
	max_zoom_wide = rw * 20;
	max_zoom_high = rh * 20;
	
	if (max_zoom_wide < viv_image_wide)
	{
		max_zoom_wide = viv_image_wide;
	}
	
	if (max_zoom_high < viv_image_high)
	{
		max_zoom_high = viv_image_high;
	}
	
	if (viv_zoom_pos)
	{
		rw = rw + (float)((max_zoom_wide) - rw) * (1 - cos(((float)(viv_zoom_pos+1) * 1.570796f) / VIV_ZOOM_MAX));
		rh = rh + (float)((max_zoom_high) - rh) * (1 - cos(((float)(viv_zoom_pos+1) * 1.570796f) / VIV_ZOOM_MAX));
	}
	
	if (viv_1to1)
	{
		rw = viv_image_wide;
		rh = viv_image_high;
	}
	
	*prw = rw;
	*prh = rh;
}

static void viv_set_custom_rate(void)
{
	if (DialogBox(GetModuleHandle(0),MAKEINTRESOURCE(IDD_CUSTOM_RATE),viv_hwnd,viv_custom_rate_proc))
	{
		switch(viv_slide_show_custom_rate_type)
		{
			case 0:
				viv_slide_show_rate = viv_slide_show_custom_rate;
				break;
		
			case 1: 
				viv_slide_show_rate = viv_slide_show_custom_rate * 1000;
				break;
				
			case 2: 
				viv_slide_show_rate = viv_slide_show_custom_rate * 1000 * 60;
				break;
		}
		
		if (viv_slide_show_rate < 1)
		{
			viv_slide_show_rate = 1;
		}
		
		if (viv_is_slideshow)
		{
			KillTimer(viv_hwnd,VIV_ID_SLIDE_SHOW_TIMER);

			SetTimer(viv_hwnd,VIV_ID_SLIDE_SHOW_TIMER,viv_slide_show_rate,0);
		}			
	}
}

static void viv_set_rate(int rate)
{
	viv_slide_show_rate = rate;
	
	if (viv_is_slideshow)
	{
		KillTimer(viv_hwnd,VIV_ID_SLIDE_SHOW_TIMER);

		SetTimer(viv_hwnd,VIV_ID_SLIDE_SHOW_TIMER,viv_slide_show_rate,0);
	}			
}

static void viv_check_menus(HMENU hmenu)
{
	int is_slide_show;
	int fill_window;
	int rw;
	int rh;
	int slideshow_rate_id;
	
	is_slide_show = 0;

	if (viv_is_fullscreen)
	{
		if (viv_is_slideshow)
		{	
			is_slide_show = 1;
			viv_update_ontop();
		}
		
		fill_window = viv_fullscreen_fill_window;
	}
	else
	{
		fill_window = viv_fill_window;
	}
				
	viv_get_render_size(&rw,&rh);

	CheckMenuItem(hmenu,VIV_ID_VIEW_MENU,viv_is_show_menu ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_STATUS,viv_is_show_status ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_ALLOW_SHRINKING,viv_allow_shrinking ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_KEEP_ASPECT_RATIO,viv_keep_aspect_ratio ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_FILL_WINDOW,fill_window ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_1TO1,((rw == viv_image_wide) && (rh == viv_image_high)) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_FULLSCREEN,viv_is_fullscreen ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_SLIDE_SHOW,is_slide_show ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_ONTOP_ALWAYS,viv_ontop == 1 ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_ONTOP_WHILE_PLAYING_OR_ANIMATING,viv_ontop == 2 ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_ONTOP_NEVER,viv_ontop == 0 ? MF_CHECKED : MF_UNCHECKED);
	
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_PAUSE,viv_is_slideshow ? MF_CHECKED : MF_UNCHECKED);

	switch(viv_slide_show_rate)
	{
		case 250: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_250; break;
		case 500: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_500; break;
		case 1000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_1000; break;
		case 2000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_2000; break;
		case 3000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_3000; break;
		case 4000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_4000; break;
		case 5000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_5000; break;
		case 6000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_6000; break;
		case 7000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_7000; break;
		case 8000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_8000; break;
		case 9000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_9000; break;
		case 10000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_10000; break;
		case 20000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_20000; break;
		case 30000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_30000; break;
		case 40000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_40000; break;
		case 50000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_50000; break;
		case 60000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_60000; break;

		default:
			slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_CUSTOM;
			break;
	}

	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_250,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_250 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_500,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_500 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_1000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_1000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_2000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_2000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_3000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_3000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_4000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_4000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_5000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_5000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_6000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_6000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_7000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_7000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_8000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_8000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_9000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_9000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_10000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_10000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_20000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_20000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_30000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_30000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_40000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_40000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_50000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_50000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_60000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_60000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_CUSTOM,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_CUSTOM ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));

	CheckMenuItem(hmenu,VIV_ID_ANIMATION_PLAY_PAUSE,viv_animation_play ? MF_CHECKED : MF_UNCHECKED);
	
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_NAME,viv_nav_sort == VIV_NAV_SORT_NAME ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_SIZE,viv_nav_sort == VIV_NAV_SORT_SIZE ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_DATE_MODIFIED,viv_nav_sort == VIV_NAV_SORT_DATE_MODIFIED ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_DATE_CREATED,viv_nav_sort == VIV_NAV_SORT_DATE_CREATED ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));

	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_ASCENDING,viv_nav_sort_ascending ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_DESCENDING,(!viv_nav_sort_ascending) ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	
	CheckMenuItem(hmenu,VIV_ID_VIEW_1TO1,((rw == viv_image_wide) && (rh == viv_image_high)) ? MF_CHECKED : MF_UNCHECKED);
}

static void viv_delete(void)
{
	if (*viv_current_fd->cFileName)
	{
		SHFILEOPSTRUCT fo;
		wchar_t filename_list[MAX_PATH+1];
		wchar_t *d;
		wchar_t *s;
		
		d = filename_list;
		s = viv_current_fd->cFileName;
		
		while(*s)
		{
			*d++ = *s++;
		}

		*d++ = 0;
		*d = 0;
		
		ZeroMemory(&fo,sizeof(SHFILEOPSTRUCT));
		fo.hwnd = viv_hwnd;
		fo.wFunc = FO_DELETE;
		fo.pFrom = filename_list;
		fo.fFlags = (GetKeyState(VK_SHIFT) < 0) ? 0 : FOF_ALLOWUNDO;

		if (SHFileOperation(&fo) == 0)
		{
			if (!fo.fAnyOperationsAborted)
			{
				viv_dragdrop_delete(viv_current_fd->cFileName);
			
				viv_next(0);
			}
		}
	}
}

static void viv_copy(int cut)
{
	if (*viv_current_fd->cFileName)
	{
		if (OpenClipboard(viv_hwnd))
		{
			EmptyClipboard();
	
			if (viv_frame_count)
			{
				if (viv_frame_hbitmap[viv_frame_position])
				{
					HDC mem_hdc;
					HDC screen_hdc;
					
					screen_hdc = GetDC(0);
					if (screen_hdc)
					{
						mem_hdc = CreateCompatibleDC(screen_hdc);
						if (mem_hdc)
						{
							HBITMAP mem_hbitmap;
	//FIXME:						
	/*						
							mem_hbitmap = CreateCompatibleBitmap(screen_hdc,viv_image_wide,viv_image_high);
							if (mem_hbitmap)
							{
								HGDIOBJ last_mem_hbitmap;
								HGDIOBJ last_back_hbitmap;
								
								last_mem_hbitmap = SelectObject(mem_hdc,mem_hbitmap);
								last_back_hbitmap = SelectObject(viv_backbuffer_hdc,viv_frame_hbitmap[viv_frame_position]);
								
								BitBlt(mem_hdc,0,0,viv_image_wide,viv_image_high,viv_backbuffer_hdc,0,0,SRCCOPY);

								SelectObject(viv_backbuffer_hdc,last_back_hbitmap);
								SelectObject(mem_hdc,last_mem_hbitmap);
								
								SetClipboardData(CF_BITMAP,mem_hbitmap);
							}
							*/
							DeleteDC(mem_hdc);
						}
						
						ReleaseDC(0,screen_hdc);
					}
				}
			}

			{
				HGLOBAL hmem;
				int wlen;
				wchar_t full_path_name[MAX_PATH];
				wchar_t cwd[MAX_PATH];
				
				GetCurrentDirectory(MAX_PATH,cwd);
				PathCombine(full_path_name,cwd,viv_current_fd->cFileName);
				
				wlen = wstring_length(full_path_name);
				
				hmem = GlobalAlloc(GMEM_MOVEABLE,(wlen + 1 + 1) * sizeof(wchar_t) + sizeof(DROPFILES));
				if (hmem)
				{
					DROPFILES *df;
					
					// build the dropfiles struct
					df = (DROPFILES *)GlobalLock(hmem);
					if (df)
					{
						df->pFiles = sizeof(DROPFILES);
						df->fWide = 1;
						df->fNC = 0;
						df->pt.x = 0;
						df->pt.y = 0;
						
						os_copy_memory(df+1,full_path_name,wlen * sizeof(wchar_t));
						((wchar_t *)(df + 1))[wlen] = 0;
						((wchar_t *)(df + 1))[wlen+1] = 0;

						GlobalUnlock(hmem);
					}

					// looking at example code,none seem to free hglobal.
					// apparently the system now owns the handle 
					SetClipboardData(CF_HDROP,hmem);
				}
			}
			
			{
				HGLOBAL hmem;
				
				hmem = GlobalAlloc(GMEM_MOVEABLE,sizeof(DWORD));
				if (hmem)
				{
					DWORD *effect;
					
					// build the dropfiles struct
					effect = (DWORD *)GlobalLock(hmem);
					if (effect)
					{
						*effect = cut ? DROPEFFECT_MOVE : (DROPEFFECT_COPY|DROPEFFECT_LINK);

						GlobalUnlock(hmem);
					}

					// looking at example code,none seem to free hglobal.
					// apparently the system now owns the handle 
					SetClipboardData(_viv_get_CF_PREFERREDDROPEFFECT(),hmem);
				}
			}
						
			CloseClipboard();
		}
	}
}

static int viv_is_key_state(int control,int shift,int alt)
{
	if (GetKeyState(VK_CONTROL) < 0)
	{
		if (!control) return 0;
	}
	else
	{
		if (control) return 0;
	}
	
	if (GetKeyState(VK_SHIFT) < 0)
	{
		if (!shift) return 0;
	}
	else
	{
		if (shift) return 0;
	}
	
	if (GetKeyState(VK_MENU) < 0)
	{
		if (!alt) return 0;
	}
	else
	{
		if (alt) return 0;
	}
	
	return 1;
}

static CLIPFORMAT _viv_get_CF_PREFERREDDROPEFFECT(void)
{
	if (!_viv_CF_PREFERREDDROPEFFECT)
	{
		_viv_CF_PREFERREDDROPEFFECT = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
	}
	
	return _viv_CF_PREFERREDDROPEFFECT;
}

static void viv_pause(void)
{
	if (viv_is_slideshow)
	{
		KillTimer(viv_hwnd,VIV_ID_SLIDE_SHOW_TIMER);
		
		viv_is_slideshow = 0;
		viv_update_ontop();
	}
	else
	{
		SetTimer(viv_hwnd,VIV_ID_SLIDE_SHOW_TIMER,viv_slide_show_rate,0);
		
		viv_is_slideshow = 1;
		viv_update_ontop();
	}
}

static void viv_increase_rate(int dec)
{
	if (dec)	
	{
		int i;
		
		for(i=0;i<VIV_PREDEFINED_RATE_COUNT;i++)
		{
			if (viv_slide_show_rate < viv_predefined_rates[i])
			{
				viv_set_rate(viv_predefined_rates[i]);
				
				break;
			}
		}
	}
	else
	{
		int i;
		
		for(i=0;i<VIV_PREDEFINED_RATE_COUNT;i++)
		{
			if (viv_slide_show_rate > viv_predefined_rates[VIV_PREDEFINED_RATE_COUNT-i-1])
			{
				viv_set_rate(viv_predefined_rates[VIV_PREDEFINED_RATE_COUNT-i-1]);
				
				break;
			}
		}
	}
}

static void viv_increase_animation_rate(int dec)
{
	if (dec)	
	{
		if (viv_animation_rate > -16)
		{
			viv_animation_rate--;
		}
	}
	else
	{
		if (viv_animation_rate < 16)
		{
			viv_animation_rate++;
		}
	}
}

static void viv_reset_animation_rate(void)
{
	if (viv_animation_rate)
	{
		viv_animation_rate = 0;
	}
}

static void viv_file_preview(void)
{
	if (*viv_current_fd->cFileName)
	{
		wchar_t filename[MAX_PATH];
		__unaligned ITEMIDLIST *pidl;
		
		_wfullpath(filename,viv_current_fd->cFileName,MAX_PATH);
		
		pidl = viv_ILCreateFromPath(filename);
		if (pidl)
		{
			SHELLEXECUTEINFO sei;
			
			os_zero_memory(&sei,sizeof(SHELLEXECUTEINFO));
			sei.cbSize = sizeof(SHELLEXECUTEINFO);
			sei.fMask = SEE_MASK_INVOKEIDLIST;
			sei.hwnd = viv_hwnd;
			sei.lpVerb = L"preview";
			sei.lpIDList = pidl;
			sei.nShow = SW_SHOWNORMAL;

			ShellExecuteEx(&sei);
		}
	}
}

static void viv_file_print(void)
{
	if (*viv_current_fd->cFileName)
	{
		wchar_t filename[MAX_PATH];
		__unaligned ITEMIDLIST *pidl;
		
		_wfullpath(filename,viv_current_fd->cFileName,MAX_PATH);
		
		pidl = viv_ILCreateFromPath(filename);
		if (pidl)
		{
			SHELLEXECUTEINFO sei;
			
			os_zero_memory(&sei,sizeof(SHELLEXECUTEINFO));
			sei.cbSize = sizeof(SHELLEXECUTEINFO);
			sei.fMask = SEE_MASK_INVOKEIDLIST;
			sei.hwnd = viv_hwnd;
			sei.lpVerb = L"print";
			sei.lpIDList = pidl;
			sei.nShow = SW_SHOWNORMAL;

			ShellExecuteEx(&sei);
		}
	}
}

static void viv_file_set_desktop_wallpaper(void)
{
	if (*viv_current_fd->cFileName)
	{
		if (!viv_stobject_hmodule)
		{
			viv_stobject_hmodule = LoadLibraryA("stobject.dll");
		}
		
		if (viv_stobject_hmodule)
		{
			wchar_t filename[MAX_PATH];
			__unaligned ITEMIDLIST *pidl;
			
			_wfullpath(filename,viv_current_fd->cFileName,MAX_PATH);
			
			pidl = viv_ILCreateFromPath(filename);
			if (pidl)
			{
				SHELLEXECUTEINFO sei;
				
				os_zero_memory(&sei,sizeof(SHELLEXECUTEINFO));
				sei.cbSize = sizeof(SHELLEXECUTEINFO);
				sei.fMask = SEE_MASK_INVOKEIDLIST;
				sei.hwnd = viv_hwnd;
				sei.lpVerb = L"setdesktopwallpaper";
				sei.lpIDList = pidl;
				sei.nShow = SW_SHOWNORMAL;

				ShellExecuteEx(&sei);
			}
			
//			FreeLibrary(hmod);
		}
	}
}

static void viv_file_edit(void)
{
	if (*viv_current_fd->cFileName)
	{
		wchar_t filename[MAX_PATH];
		__unaligned ITEMIDLIST *pidl;
		
		_wfullpath(filename,viv_current_fd->cFileName,MAX_PATH);
		
		pidl = viv_ILCreateFromPath(filename);
		if (pidl)
		{
			SHELLEXECUTEINFO sei;
			
			os_zero_memory(&sei,sizeof(SHELLEXECUTEINFO));
			sei.cbSize = sizeof(SHELLEXECUTEINFO);
			sei.fMask = SEE_MASK_INVOKEIDLIST;
			sei.hwnd = viv_hwnd;
			sei.lpVerb = L"edit";
			sei.lpIDList = pidl;
			sei.nShow = SW_SHOWNORMAL;

			ShellExecuteEx(&sei);
		}
	}
}

static void viv_open_file_location(void)
{
	if (*viv_current_fd->cFileName)
	{
		int openpathok;
		
		openpathok = 0;
		
		if (viv_SHOpenFolderAndSelectItems)
		{
			wchar_t path_part[MAX_PATH];
			ITEMIDLIST *folder_idlist;
			
			viv_get_path_part(path_part,viv_current_fd->cFileName);
		
			// if path_part_buf.buf is an empty string, os_ILCreateFromPath will
			// correctly return the desktop pidl (an empty pidl).
			folder_idlist = viv_ILCreateFromPath(path_part);

		printf("folder_idlist %p\n",folder_idlist);
			if (folder_idlist)
			{
				ITEMIDLIST *idlist;
				
				idlist = viv_ILCreateFromPath(viv_current_fd->cFileName);
		printf("idlist %S %p\n",viv_current_fd->cFileName,idlist);
				if (idlist)
				{
					HRESULT hres;
					
		printf("ENTER os_SHOpenFolderAndSelectItems\n");
		
					hres = viv_SHOpenFolderAndSelectItems(folder_idlist,1,(LPCITEMIDLIST *)&idlist,0);
					
		printf("os_SHOpenFolderAndSelectItems %08x\n",hres);
					if (SUCCEEDED(hres))
					{
						openpathok = 1;
					}
					else
					if (hres == E_ABORT)
					{
						// aborted, QTBar after 10seconds.
						openpathok = 1;
					}

					CoTaskMemFree(idlist);
				}

				CoTaskMemFree(folder_idlist);
			}
		}
		
		if (!openpathok)
		{
			wchar_t path[MAX_PATH];
			
			_wfullpath(path,viv_current_fd->cFileName,MAX_PATH);
			
			PathRemoveFileSpec(path);
			
			ShellExecute(viv_hwnd,0,path,0,0,SW_SHOWNORMAL);
		}
	}
}


static void viv_properties(void)
{
	if (*viv_current_fd->cFileName)
	{
		wchar_t filename[MAX_PATH];
		__unaligned ITEMIDLIST *pidl;
		
		_wfullpath(filename,viv_current_fd->cFileName,MAX_PATH);
		
		pidl = viv_ILCreateFromPath(filename);
		if (pidl)
		{
			SHELLEXECUTEINFO sei;
			
			os_zero_memory(&sei,sizeof(SHELLEXECUTEINFO));
			sei.cbSize = sizeof(SHELLEXECUTEINFO);
			sei.fMask = SEE_MASK_INVOKEIDLIST;
			sei.hwnd = viv_hwnd;
			sei.lpVerb = L"properties";
			sei.lpIDList = pidl;
			sei.nShow = SW_SHOWNORMAL;

			ShellExecuteEx(&sei);
		}
	}
}

static void viv_doing_cancel(void)
{
	if (viv_doing)
	{
		if (viv_doing == 2)
		{
			ShowCursor(TRUE);
		}
		
		ReleaseCapture();
		
		viv_doing = 0;
	}
}

static const char *_viv_get_copydata_string(const char *p,const char *e,wchar_t *buf,int max)
{
	wchar_t *d;
	
	d = buf;
	
	while(p + sizeof(wchar_t) <= e)
	{
		if (!*(wchar_t *)p)
		{
			p += sizeof(wchar_t);
			break;
		}
		
		if (max > 1)
		{
			*d++ = *(wchar_t *)p;
			max--;
		}
		
		p+=sizeof(wchar_t);
	}
	
	if (max > 0)
	{
		*d = 0;
	}
	
	return p;
}


static void os_remove_all_menu_items(HMENU hmenu)
{	
	// remove all items
	// we loop until all the items are removed
	// we remove the first item in the menu each time we delete an item
	// use deletemenu to recursively destroy submenus.
	// removemenu does not destroy sub menus.
	for(;;)
	{
		if (!DeleteMenu(hmenu,0,MF_BYPOSITION)) break;
	}
}

static void viv_nav_item_free_all(void)
{
	if (viv_nav_items)
	{
		int i;
		
		for(i=0;i<viv_nav_item_count;i++)
		{
			mem_free(viv_nav_items[i]);
		}
		
		mem_free(viv_nav_items);
		viv_nav_items = 0;
		viv_nav_item_count = 0;
	}
}

static int __cdecl viv_nav_playlist_compare(const void *a,const void *b)
{
	return _viv_compare(&(*(viv_nav_item_t **)a)->fd,&(*(viv_nav_item_t **)b)->fd);
}

static void viv_blank(void)
{
	viv_clear();

	viv_current_fd->cFileName[0] = 0;

	viv_status_update();
	viv_update_title();

	InvalidateRect(viv_hwnd,0,FALSE);
}

static INT_PTR CALLBACK viv_options_general_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM /*lParam*/)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			int all_checked;
			
			all_checked = 1;
			
			if (viv_multiple_instances) CheckDlgButton(hwnd,IDC_MULTIPLE_INSTANCES,BST_CHECKED);
			if (viv_is_association(L".bmp")) CheckDlgButton(hwnd,IDC_BMP,BST_CHECKED); else all_checked = 0;
			if (viv_is_association(L".gif")) CheckDlgButton(hwnd,IDC_GIF,BST_CHECKED);else all_checked = 0;
			if (viv_is_association(L".ico")) CheckDlgButton(hwnd,IDC_ICO,BST_CHECKED);else all_checked = 0;
			if (viv_is_association(L".jpeg")) CheckDlgButton(hwnd,IDC_JPEG,BST_CHECKED);else all_checked = 0;
			if (viv_is_association(L".jpg")) CheckDlgButton(hwnd,IDC_JPG,BST_CHECKED);else all_checked = 0;
			if (viv_is_association(L".png")) CheckDlgButton(hwnd,IDC_PNG,BST_CHECKED);else all_checked = 0;
			if (viv_is_association(L".tif")) CheckDlgButton(hwnd,IDC_TIF,BST_CHECKED);else all_checked = 0;
			if (viv_is_association(L".tiff")) CheckDlgButton(hwnd,IDC_TIFF,BST_CHECKED);else all_checked = 0;
			if (all_checked) CheckDlgButton(hwnd,IDC_CHECKALL,BST_CHECKED);
			
			return TRUE;
		}
			
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDC_CHECKALL:
				{
					DWORD check;

					if (IsDlgButtonChecked(hwnd,IDC_CHECKALL))
					{
						check = BST_CHECKED;
					}
					else
					{
						check = BST_UNCHECKED;
					}

					CheckDlgButton(hwnd,IDC_BMP,check);
					CheckDlgButton(hwnd,IDC_GIF,check);
					CheckDlgButton(hwnd,IDC_ICO,check);
					CheckDlgButton(hwnd,IDC_JPEG,check);
					CheckDlgButton(hwnd,IDC_JPG,check);
					CheckDlgButton(hwnd,IDC_PNG,check);
					CheckDlgButton(hwnd,IDC_TIF,check);
					CheckDlgButton(hwnd,IDC_TIFF,check);

					break;
				}
			}
					
			break;
	}
	
	return FALSE;
}

static INT_PTR CALLBACK viv_options_view_proc(HWND /*hwnd*/,UINT msg,WPARAM /*wParam*/,LPARAM /*lParam*/)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			return TRUE;
			
		case WM_COMMAND:
			break;
	}
	
	return FALSE;
}

static INT_PTR CALLBACK viv_options_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM /*lParam*/)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			HWND general_form;
			HWND view_form;
			RECT parent_rect;
			RECT dialog_rect;
			
			GetWindowRect(GetParent(hwnd),&parent_rect);
			GetWindowRect(hwnd,&dialog_rect);
			
			SetWindowPos(hwnd,0,parent_rect.left + ((parent_rect.right - parent_rect.left) / 2) - ((dialog_rect.right - dialog_rect.left) / 2),parent_rect.top + ((parent_rect.bottom - parent_rect.top) / 2) - ((dialog_rect.bottom - dialog_rect.top) / 2),0,0,SWP_NOSIZE|SWP_NOZORDER);

			// init tree			
			{
				TV_INSERTSTRUCT tvitem;
				
				tvitem.hInsertAfter = TVI_LAST;
				tvitem.hParent = TVI_ROOT;
				tvitem.item.mask = TVIF_TEXT;
				tvitem.item.pszText = L"General";
				
				TreeView_InsertItem(GetDlgItem(hwnd,IDC_TREE1),&tvitem);

				tvitem.item.pszText = L"View";
				
				TreeView_InsertItem(GetDlgItem(hwnd,IDC_TREE1),&tvitem);
			}

			// tab
			{
				TCITEM tcitem;
				
				tcitem.mask = TCIF_TEXT;
				tcitem.pszText = L"General";
				TabCtrl_InsertItem(GetDlgItem(hwnd,IDC_TAB1),0,&tcitem);

				tcitem.pszText = L"View";
				TabCtrl_InsertItem(GetDlgItem(hwnd,IDC_TAB2),0,&tcitem);
			}

/*
			{
				RECT rect;
				
				GetClientRect(GetDlgItem(hwnd,IDC_PAGEPLACEHOLDER),&rect);
				MapWindowPoints(GetDlgItem(hwnd,IDC_PAGEPLACEHOLDER),hwnd,(LPPOINT)&rect,2);

				general_form = CreateDialog(GetModuleHandle(0),MAKEINTRESOURCE(IDD_FORMVIEW1),hwnd,viv_options_general_proc);
				SetWindowLong(general_form,GWL_ID,IDD_FORMVIEW1);

				if (viv_EnableThemeDialogTexture)
				{
					viv_EnableThemeDialogTexture(general_form,ETDT_ENABLETAB);
				}

				SetWindowPos(general_form,0,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			}
		*/
		
			ShowWindow(GetDlgItem(hwnd,IDC_TAB1),SW_HIDE);
		
			{
				RECT rect;
				
				GetClientRect(GetDlgItem(hwnd,IDC_PAGEPLACEHOLDER),&rect);
				MapWindowPoints(GetDlgItem(hwnd,IDC_PAGEPLACEHOLDER),hwnd,(LPPOINT)&rect,2);

				view_form = CreateDialog(GetModuleHandle(0),MAKEINTRESOURCE(IDD_FORMVIEW2),hwnd,viv_options_view_proc);
				SetWindowLong(view_form,GWL_ID,IDD_FORMVIEW2);

				if (viv_EnableThemeDialogTexture)
				{
					viv_EnableThemeDialogTexture(view_form,ETDT_ENABLETAB);
				}

				SetWindowPos(view_form,0,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
				
				ComboBox_AddString(GetDlgItem(view_form,IDC_COMBO1),L"COLORONCOLOR (Performance)");
				ComboBox_AddString(GetDlgItem(view_form,IDC_COMBO1),L"HALFTONE (Quality)");
				
				if (viv_shrink_blit_mode == VIV_SHRINK_BLIT_MODE_HALFTONE)
				{
					ComboBox_SetCurSel(GetDlgItem(view_form,IDC_COMBO1),1);
				}
				else
				{
					ComboBox_SetCurSel(GetDlgItem(view_form,IDC_COMBO1),0);
				}

				ComboBox_AddString(GetDlgItem(view_form,IDC_COMBO2),L"COLORONCOLOR (Performance)");
				ComboBox_AddString(GetDlgItem(view_form,IDC_COMBO2),L"HALFTONE (Quality)");
				
				if (viv_mag_filter == VIV_MAG_FILTER_HALFTONE)
				{
					ComboBox_SetCurSel(GetDlgItem(view_form,IDC_COMBO2),1);
				}
				else
				{
					ComboBox_SetCurSel(GetDlgItem(view_form,IDC_COMBO2),0);
				}
			}
			

//			ShowWindow(general_form,SW_SHOWNORMAL);
			ShowWindow(view_form,SW_SHOWNORMAL);

			return FALSE;
		}
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
				
					if (LOWORD(wParam) == IDOK)
					{
						HWND general_form;
						int old_shrink_blit_mode;
						
						general_form = GetDlgItem(hwnd,IDD_FORMVIEW);
						viv_multiple_instances = 0;
						
						if (IsDlgButtonChecked(general_form,IDC_MULTIPLE_INSTANCES)) viv_multiple_instances = 1;
						if (IsDlgButtonChecked(general_form,IDC_BMP)) viv_install_association(L".bmp",L"Bitmap Image",0); else viv_uninstall_association(L".bmp");
						if (IsDlgButtonChecked(general_form,IDC_GIF)) viv_install_association(L".gif",L"Animated GIF Image",0); else viv_uninstall_association(L".gif");
						if (IsDlgButtonChecked(general_form,IDC_ICO)) viv_install_association(L".ico",L"Icon File",L"%1"); else viv_uninstall_association(L".ico");
						if (IsDlgButtonChecked(general_form,IDC_JPEG)) viv_install_association(L".jpeg",L"JPEG Image",0); else viv_uninstall_association(L".jpeg");
						if (IsDlgButtonChecked(general_form,IDC_JPG)) viv_install_association(L".jpg",L"JPEG Image",0); else viv_uninstall_association(L".jpg");
						if (IsDlgButtonChecked(general_form,IDC_PNG)) viv_install_association(L".png",L"PNG Image",0); else viv_uninstall_association(L".png");
						if (IsDlgButtonChecked(general_form,IDC_TIF)) viv_install_association(L".tif",L"TIFF Image",0); else viv_uninstall_association(L".tif");
						if (IsDlgButtonChecked(general_form,IDC_TIFF)) viv_install_association(L".tiff",L"TIFF Image",0); else viv_uninstall_association(L".tiff");
						
						old_shrink_blit_mode = viv_shrink_blit_mode;
						
						if (ComboBox_GetCurSel(GetDlgItem(general_form,IDC_COMBO1)) == 1)
						{
							viv_shrink_blit_mode = VIV_SHRINK_BLIT_MODE_HALFTONE;
						}
						else
						{
							viv_shrink_blit_mode = VIV_SHRINK_BLIT_MODE_COLORONCOLOR;
						}
						
						if (old_shrink_blit_mode != viv_shrink_blit_mode)
						{
							InvalidateRect(viv_hwnd,0,FALSE);
						}
					
						viv_save_settings(viv_appdata);
					}
				
					EndDialog(hwnd,0);
					break;
			}

			break;
	}
	
	return FALSE;
}

static void viv_options(void)
{	
	DialogBox(GetModuleHandle(0),MAKEINTRESOURCE(IDD_DIALOG1),viv_hwnd,viv_options_proc);
}


// use the default class description, ie: TXT File
static void viv_install_association(const wchar_t *association,const wchar_t *description,const wchar_t *icon_location)
{
	HKEY hkey;
	wchar_t class_name[MAX_PATH];
	wchar_t key[MAX_PATH];
	wchar_t default_icon[MAX_PATH];
	
	// make sure we uninstall old associations first.
	viv_uninstall_association(association);

	wstring_copy(class_name,MAX_PATH,L"voidImageViewer");
	wstring_cat(class_name,MAX_PATH,association);

	wstring_copy(default_icon,MAX_PATH,L"voidImageViewer");
	wstring_cat(default_icon,MAX_PATH,association);
	wstring_cat(default_icon,MAX_PATH,L"\\DefaultIcon");

	wstring_copy(key,MAX_PATH,class_name);
	wstring_cat(key,MAX_PATH,L"\\shell\\open\\command");
	
	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,default_icon,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		if (icon_location)
		{
			viv_set_registry_string(hkey,0,icon_location);
		}
		else
		{
			wchar_t filename[MAX_PATH];
			wchar_t command[MAX_PATH];

			GetModuleFileName(0,filename,MAX_PATH);
			
			wstring_copy(command,MAX_PATH,filename);
			wstring_cat(command,MAX_PATH,L", 1");
			
			viv_set_registry_string(hkey,0,command);
		}
			
		RegCloseKey(hkey);
	}	
		
	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,class_name,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		viv_set_registry_string(hkey,0,description);
		
		RegCloseKey(hkey);
	}		
	
	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,key,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t filename[MAX_PATH];
		wchar_t command[MAX_PATH];

		GetModuleFileName(0,filename,MAX_PATH);
		
		wstring_copy(command,MAX_PATH,L"\"");
		wstring_cat(command,MAX_PATH,filename);
		wstring_cat(command,MAX_PATH,L"\" \"%1\"");
		
		viv_set_registry_string(hkey,0,command);
		
		RegCloseKey(hkey);
	}

	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,association,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t wbuf[MAX_PATH];
		
		if (!viv_get_registry_string(hkey,L"voidImageViewer.Backup",wbuf,MAX_PATH))
		{
			if (!viv_get_registry_string(hkey,0,wbuf,MAX_PATH))
			{
				*wbuf = 0;
			}
			
			viv_set_registry_string(hkey,L"voidImageViewer.Backup",wbuf);
		}

		viv_set_registry_string(hkey,0,class_name);
		
		RegCloseKey(hkey);
	}
}

static void viv_uninstall_association(const wchar_t *association)
{
	HKEY hkey;
	wchar_t class_name[MAX_PATH];
	
	wstring_copy(class_name,MAX_PATH,L"voidImageViewer");
	wstring_cat(class_name,MAX_PATH,association);
	
	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,association,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t wbuf[MAX_PATH];
		
		if (viv_get_registry_string(hkey,L"voidImageViewer.Backup",wbuf,MAX_PATH))
		{
			viv_set_registry_string(hkey,0,wbuf);

			RegDeleteValueW(hkey,L"voidImageViewer.Backup");
		}

		RegCloseKey(hkey);
	}
	
	SHDeleteKeyW(HKEY_CLASSES_ROOT,class_name);
}

static int viv_is_association(const wchar_t *association)
{
	int ret;
	HKEY hkey;
	wchar_t class_name[MAX_PATH];
	wchar_t key[MAX_PATH];
	
	wstring_copy(class_name,MAX_PATH,L"voidImageViewer");
	wstring_cat(class_name,MAX_PATH,association);
	
	wstring_copy(key,MAX_PATH,class_name);
	wstring_cat(key,MAX_PATH,L"\\shell\\open\\command");
	
	ret = 0;

	if (RegOpenKeyExW(HKEY_CLASSES_ROOT,association,0,KEY_QUERY_VALUE,&hkey) == ERROR_SUCCESS)
	{
		wchar_t wbuf[MAX_PATH];

		if (viv_get_registry_string(hkey,0,wbuf,MAX_PATH))
		{
			if (wstring_compare(wbuf,class_name) == 0)
			{
				ret++;
			}
		}

		RegCloseKey(hkey);
	}

	if (RegOpenKeyExW(HKEY_CLASSES_ROOT,key,0,KEY_QUERY_VALUE,&hkey) == ERROR_SUCCESS)
	{
		wchar_t wbuf[MAX_PATH];

		if (viv_get_registry_string(hkey,0,wbuf,MAX_PATH))
		{
			wchar_t filename[MAX_PATH];
			wchar_t command[MAX_PATH];

			GetModuleFileName(0,filename,MAX_PATH);
			
			wstring_copy(command,MAX_PATH,L"\"");
			wstring_cat(command,MAX_PATH,filename);
			wstring_cat(command,MAX_PATH,L"\" \"%1\"");
			
			if (wstring_compare(wbuf,command) == 0)
			{
				ret++;
			}
		}

		RegCloseKey(hkey);
	}

	return ret == 2;
}

static int viv_get_registry_string(HKEY hkey,const wchar_t *value,wchar_t *wbuf,int size_in_wchars)
{
	DWORD cbData;
	DWORD type;
	
	cbData = size_in_wchars * sizeof(wchar_t);
	
	if (RegQueryValueExW(hkey,value,0,&type,(BYTE *)wbuf,&cbData) == ERROR_SUCCESS)
	{
		if ((type == REG_SZ) || (type == REG_EXPAND_SZ))
		{
			return 1;
		}
	}
	
	return 0;
}

static int viv_set_registry_string(HKEY hkey,const wchar_t *value,const wchar_t *wbuf)
{
	if (RegSetValueExW(hkey,value,0,REG_SZ,(BYTE *)wbuf,(wstring_length(wbuf) + 1) * sizeof(wchar_t)) == ERROR_SUCCESS)
	{
		return 1;
	}
	
	return 0;
}

static void wstring_copy(wchar_t *d,int size,const wchar_t *s)
{
	size--;
	
	while(*s)
	{
		if (!size) break;
		
		*d++ = *s++;
	}
	
	*d = 0;
}

static void wstring_cat(wchar_t *buf,int size,const wchar_t *s)
{
	wchar_t *d;
	
	size--;
	
	d = buf;
	while(*d)
	{
		d++;
		size--;
	}
	
	while(*s)
	{
		if (!size) break;
		
		*d++ = *s;
		s++;
	}
	
	*d = 0;
}

static int wstring_compare(const wchar_t *s1,const wchar_t *s2)
{
	const wchar_t *p1;
	const wchar_t *p2;
	int c;
	
	p1 = s1;
	p2 = s2;
	
	for(;;)
	{
		c = *p1 - *p2;
		if (c) return c;
		
		if (!*p1) break;
		if (!*p2) break;
		
		p1++;
		p2++;
	}
	
	return 0;
}

static void viv_timer_start(void)
{
	if (!viv_timer_handle)	
	{
		CreateTimerQueueTimer(&viv_timer_handle,NULL,viv_timer_callback,0,1,1,0);
	}

	if (!viv_is_update_frame_timer)	
	{
		SetTimer(viv_hwnd,VIV_ID_UPDATE_FRAME_TIMER,1,0);

		viv_is_update_frame_timer = 1;
	}
}

static void viv_show_cursor(void)
{
	if ((viv_is_fullscreen && (!viv_in_popup_menu)))
	{
		POINT pt;
		
		GetCursorPos(&pt);
		
		printf("%d %d, last %d %d\n",pt.x,pt.y,viv_hide_cursor_x,viv_hide_cursor_y);
		
		if ((pt.x != viv_hide_cursor_x) || (pt.y != viv_hide_cursor_y))
		{
			if (!viv_cursor_shown)
			{
				ShowCursor(TRUE);
				
				viv_cursor_shown = 1;
			}

			if (viv_is_hide_cursor_timer)
			{
				KillTimer(viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER);
				SetTimer(viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER,VIV_HIDE_CURSOR_DELAY,0);
			}
			else
			{
				SetTimer(viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER,VIV_HIDE_CURSOR_DELAY,0);
				
				viv_is_hide_cursor_timer = 1;
			}
			
			viv_hide_cursor_x = pt.x;
			viv_hide_cursor_y = pt.y;
		}
	}
}

static void viv_animation_pause(void)
{
	viv_animation_play = !viv_animation_play;
}

static void viv_frame_step(void)
{
	viv_frame_looped = 0;
	
	if (viv_animation_play)
	{
		viv_animation_play = 0;
	}
	else
	{
		if (viv_frame_count > 1)			
		{
			if ((viv_frame_loaded_count == viv_frame_count) || (viv_frame_position + 1 < viv_frame_loaded_count))
			{
				viv_frame_position++;
				if (viv_frame_position == viv_frame_count)
				{
					viv_frame_position = 0;
				}
			
				InvalidateRect(viv_hwnd,NULL,FALSE);
				UpdateWindow(viv_hwnd);
			}
		}
	}
}

static void viv_frame_prev(void)
{
	viv_frame_looped = 0;

	if (viv_animation_play)
	{
		viv_animation_play = 0;
	}
	else
	{
		if (viv_frame_count > 1)
		{
			if (viv_frame_position > 0)
			{
				viv_frame_position--;
			}
			else
			{
				viv_frame_position = viv_frame_loaded_count - 1;
			}
		
			InvalidateRect(viv_hwnd,NULL,FALSE);
			UpdateWindow(viv_hwnd);
		}
	}
}

static void viv_view_1to1(void)
{
	if (viv_1to1)	
	{
		if (viv_have_old_zoom)
		{
			viv_1to1 = 0;
			viv_zoom_pos = viv_old_zoom_pos;
			viv_view_set(viv_view_x,viv_view_y);
			InvalidateRect(viv_hwnd,0,FALSE);
			
			return;
		}
	}
	
	viv_have_old_zoom = 1;
	viv_old_zoom_pos = viv_zoom_pos;
	
	viv_1to1 = 1;
	viv_zoom_pos = 0;
	viv_view_set(viv_view_x,viv_view_y);
	InvalidateRect(viv_hwnd,0,FALSE);
}

static void viv_dragdrop_clearall(void)
{	
	viv_dragdrop_t *d;
	
	d = viv_dragdrop_start;
	while(d)
	{
		viv_dragdrop_t *next_d;
		
		next_d = d->next;
		
		mem_free(d->filename);
		mem_free(d);
		
		d = next_d;
	}
	
	viv_dragdrop_start = 0;
	viv_dragdrop_last = 0;
}

static void viv_dragdrop_delete(const wchar_t *filename)
{
	viv_dragdrop_t *d;
	
	d = viv_dragdrop_start;
	while(d)
	{
		if (wcscmp(d->filename,filename) == 0)
		{
			if (viv_dragdrop_start == d)
			{
				viv_dragdrop_start = d->next;
			}
			else
			{
				d->prev->next = d->next;
			}
		
			if (viv_dragdrop_last == d)
			{
				viv_dragdrop_last = d->prev;
			}
			else
			{
				d->next->prev = d->prev;
			}
		
			mem_free(d->filename);
			mem_free(d);
		
			break;
		}
		
		d = d->next;
	}
}

static viv_dragdrop_t *viv_dragdrop_add(const wchar_t *filename,viv_fileinfo_t *fileinfo)
{
	viv_dragdrop_t *d;
	
	d = (viv_dragdrop_t *)mem_alloc(sizeof(viv_dragdrop_t));
	
	d->filename = string_alloc(filename);
	viv_fileinfo_copy(&d->fileinfo,fileinfo);
	
	if (viv_dragdrop_start)
	{
		viv_dragdrop_last->next = d;
		d->prev = viv_dragdrop_last;
	}
	else
	{
		viv_dragdrop_start = d;
		d->prev = 0;
	}
	
	viv_dragdrop_last = d;
	d->next = 0;
	
	return d;
}

static void viv_dragdrop_add_path(const wchar_t *filename)
{
	WIN32_FIND_DATA fd;
	wchar_t buf[MAX_PATH];
	HANDLE h;

	wstring_copy(buf,MAX_PATH,filename);
	wstring_cat(buf,MAX_PATH,L"\\*.*");

	h = FindFirstFile(buf,&fd);
	if (h != INVALID_HANDLE_VALUE)
	{
		for(;;)
		{
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!((wcscmp(fd.cFileName,L".") == 0) || (wcscmp(fd.cFileName,L"..") == 0)))
				{
					wstring_copy(buf,MAX_PATH,filename);
					wstring_cat(buf,MAX_PATH,L"\\");
					wstring_cat(buf,MAX_PATH,fd.cFileName);
					
					viv_dragdrop_add_path(buf);
				}
			}
			else
			{
				if (viv_is_valid_filename(fd.cFileName))
				{
					viv_fileinfo_t fileinfo;
					
					wstring_copy(buf,MAX_PATH,filename);
					wstring_cat(buf,MAX_PATH,L"\\");
					wstring_cat(buf,MAX_PATH,fd.cFileName);
					
					viv_fileinfo_copy_fd(&fileinfo,&fd);
					
					viv_dragdrop_add(buf,&fileinfo);
				}
			}

			if (!FindNextFile(h,&fd)) break;
		}
		
		FindClose(h);
	}
}

static void viv_dragdrop_add_filename(const wchar_t *filename)
{
	WIN32_FIND_DATA fd;
	
	if (GetFileAttributesEx(filename,GetFileExInfoStandard,&fd))
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			viv_dragdrop_add_path(filename);
		}
		else
		{
			viv_fileinfo_t fileinfo;
			
			viv_fileinfo_copy_fd(&fileinfo,&fd);

			viv_dragdrop_add(filename,&fileinfo);
		}
	}
}

static int wstring_to_int(wchar_t *s)
{
	int i;
	int sign;
	
	sign = 1;
	i = 0;
	
	if (*s == '-')
	{
		sign = -1;
		s++;
	}
	
	while(*s)
	{
		if ((*s >= '0') && (*s <= '9'))
		{
			i *= 10;
			i += *s - '0';
		}
		
		s++;
	}
	
	return i * sign;
}

static void viv_fileinfo_copy(viv_fileinfo_t *dst,viv_fileinfo_t *src)
{
	os_copy_memory(dst,src,sizeof(viv_fileinfo_t));
}

static void viv_fileinfo_copy_fd(viv_fileinfo_t *fileinfo,WIN32_FIND_DATA *fd)
{
	((LARGE_INTEGER *)&fileinfo->size)->HighPart = fd->nFileSizeHigh;
	((LARGE_INTEGER *)&fileinfo->size)->LowPart = fd->nFileSizeLow;
	((LARGE_INTEGER *)&fileinfo->date_modified)->HighPart = fd->ftLastWriteTime.dwHighDateTime;
	((LARGE_INTEGER *)&fileinfo->date_modified)->LowPart = fd->ftLastWriteTime.dwLowDateTime;
	((LARGE_INTEGER *)&fileinfo->date_created)->HighPart = fd->ftCreationTime.dwHighDateTime;
	((LARGE_INTEGER *)&fileinfo->date_created)->LowPart = fd->ftCreationTime.dwLowDateTime;
}

static void viv_fd_copy_fileinfo(WIN32_FIND_DATA *fd,viv_fileinfo_t *fileinfo)
{
	fd->nFileSizeHigh = ((LARGE_INTEGER *)&fileinfo->size)->HighPart;
	fd->nFileSizeLow = ((LARGE_INTEGER *)&fileinfo->size)->LowPart;
	fd->ftLastWriteTime.dwHighDateTime = ((LARGE_INTEGER *)&fileinfo->date_modified)->HighPart;
	fd->ftLastWriteTime.dwLowDateTime = ((LARGE_INTEGER *)&fileinfo->date_modified)->LowPart;
	fd->ftCreationTime.dwHighDateTime = ((LARGE_INTEGER *)&fileinfo->date_created)->HighPart;
	fd->ftCreationTime.dwLowDateTime = ((LARGE_INTEGER *)&fileinfo->date_created)->LowPart;
}

static ITEMIDLIST *viv_ILCreateFromPath(const wchar_t *path)
{
	IShellFolder *psf;
	ITEMIDLIST *pidl;
	ITEMIDLIST *ret;
	
	ret = 0;

	if (SUCCEEDED(SHGetDesktopFolder(&psf)))
	{
		if (SUCCEEDED(psf->ParseDisplayName(NULL,0,(LPOLESTR)path,NULL,&pidl,NULL)))
		{
			ret = pidl;
		}

		psf->Release();
	}	           
	
	return ret;
}

static void viv_get_path_part(wchar_t *buf,const wchar_t *s)
{
	const wchar_t *p;
	const wchar_t *last;
	wchar_t *d;
	
	d = buf;
	p = s;
	last = 0;
	
	while(*p)
	{
		if (*p == '\\')
		{
			last = p;
		}
		
		p++;
	}
	
	if (last)
	{
		p = s;
		
		while(p != last)
		{
			*d++ = *p;
			
			p++;
		}
	}

	*d = 0;
}

static void viv_timer_stop(void)
{
	if (viv_timer_handle)
	{
		DeleteTimerQueueTimer(NULL,viv_timer_handle,INVALID_HANDLE_VALUE);
		
		viv_timer_handle = 0;
	}
	
	if (viv_is_update_frame_timer)	
	{
		KillTimer(viv_hwnd,VIV_ID_UPDATE_FRAME_TIMER);

		viv_is_update_frame_timer = 0;
	}
}

static void NTAPI viv_timer_callback(PVOID /*lpParameter*/,BOOLEAN /*TimerOrWaitFired*/)
{
	PostMessage(viv_hwnd,VIV_WM_TIMER,0,0);
} 

static int viv_is_window_maximized(HWND hwnd)
{
	// are we minimized with the WPF_RESTORETOMAXIMIZED flag set?
	if (IsIconic(hwnd))
	{
		WINDOWPLACEMENT wp;

		// IsZoomed does not work when the window is minimized.
		// its ok to use GetWindowPlacement here as we do not touch the normal rect.
		wp.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(hwnd,&wp);

		if (wp.flags & WPF_RESTORETOMAXIMIZED)
		{
			return 1;
		}
	}

	if (IsZoomed(hwnd))
	{
		return 1;
	}

	return 0;
}

static wchar_t *wstring_format_number(wchar_t *buf,int number)
{
	wchar_t backbuf[MAX_PATH];
	wchar_t *n;
	wchar_t *d;
	
	d = buf;
	
	if (number < 0)
	{
		number = -number;
		*d++ = '-';
	}

	if (!number)
	{
		*d++ = '0';
		*d = 0;
		return d;
	}
	
	n = backbuf;
	
	while(number)
	{
		*n++ = (number % 10) + '0';
		number /= 10;
	}

	while(n > backbuf)
	{
		n--;
		
		*d = *n;
		
		d++;
	}
	*d = 0;
	
	return d;
}

static int wstring_get_appdata_path(wchar_t *buf)
{
	LPITEMIDLIST pidl;
	int ret;
	
	ret = 0;

	if (SUCCEEDED(SHGetSpecialFolderLocation(0,CSIDL_APPDATA,&pidl)))
	{
		if (SHGetPathFromIDList(pidl,buf))
		{
			ret = 1;
		}

		CoTaskMemFree(pidl);
	}
	
	return ret;
}

static int utf8_to_int(const utf8_t *str)
{
	int i;
	int sign;
	register const utf8_t *s;
	
	s = str;
	
	i = 0;
	
	// check for hex.
	// hex can not have negative numbers
	if (*s == '0')
	{
		if ((s[1] == 'x') || (s[1] == 'X'))
		{
			// hex.
			s += 2;
			
			while (*s)
			{
				if ((*s >= '0') && (*s <= '9'))
				{
					i <<= 4;
					i += *s - '0';
				}
				else
				if ((*s >= 'A') && (*s <= 'F'))
				{
					i <<= 4;
					i += *s - 'A' + 10;
				}
				else
				if ((*s >= 'a') && (*s <= 'f'))
				{
					i <<= 4;
					i += *s - 'a' + 10;
				}
				else
				{
					break;
				}
				
				s++;
			}
			
			return i;
		}
	}
	
	// check for '-'
	if (*s == '-')
	{
		sign = -1;
		s++;
	}
	else
	{
		sign = 1;
	}
	
	// dec
	while (*s)
	{
		if ((*s >= '0') && (*s <= '9'))
		{
			i *= 10;
			i += *s - '0';
		}
		else
		{
			break;
		}
		
		s++;
	}
	
	return i * sign;
}

static INT_PTR CALLBACK viv_custom_rate_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM /*lParam*/)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			RECT parent_rect;
			RECT dialog_rect;

			GetWindowRect(GetParent(hwnd),&parent_rect);
			GetWindowRect(hwnd,&dialog_rect);
			
			SetWindowPos(hwnd,0,parent_rect.left + ((parent_rect.right - parent_rect.left) / 2) - ((dialog_rect.right - dialog_rect.left) / 2),parent_rect.top + ((parent_rect.bottom - parent_rect.top) / 2) - ((dialog_rect.bottom - dialog_rect.top) / 2),0,0,SWP_NOSIZE|SWP_NOZORDER);

			SetDlgItemInt(hwnd,IDC_CUSTOM_RATE_EDIT,viv_slide_show_custom_rate,FALSE);

			ComboBox_AddString(GetDlgItem(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO),L"milliseconds");
			ComboBox_AddString(GetDlgItem(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO),L"seconds");
			ComboBox_AddString(GetDlgItem(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO),L"minutes");
			
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO),viv_slide_show_custom_rate_type);

			return TRUE;
		}
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
					viv_slide_show_custom_rate_type = ComboBox_GetCurSel(GetDlgItem(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO));
					viv_slide_show_custom_rate = GetDlgItemInt(hwnd,IDC_CUSTOM_RATE_EDIT,NULL,FALSE);
					EndDialog(hwnd,1);
					break;

				case IDCANCEL:
					EndDialog(hwnd,0);
					break;
			}

			break;
	}
	
	return FALSE;
}

static void viv_update_frame(void)
{
	if (!viv_is_fullscreen)
	{
		DWORD oldstyle;
		DWORD newstyle;
		RECT windowrect;
		RECT clientrect;
		RECT newrect;
		RECT oldrect;
		
		oldstyle = GetWindowLong(viv_hwnd,GWL_STYLE);
		newstyle = oldstyle;
		
		GetClientRect(viv_hwnd,&clientrect);
		printf("clientrect %d %d %d %d\n",clientrect.left,clientrect.top,clientrect.right,clientrect.bottom);
		
		CopyRect(&oldrect,&clientrect);
		AdjustWindowRect(&oldrect,oldstyle,GetMenu(viv_hwnd) ? TRUE : FALSE);
		
		printf("oldrect %d %d %d %d %d\n",oldrect.left,oldrect.top,oldrect.right,oldrect.bottom,GetMenu(viv_hwnd) ? TRUE : FALSE);
	
		if (viv_is_show_caption)	
		{
			newstyle |= WS_CAPTION | WS_SYSMENU;
		}
		else
		{
			newstyle &= ~(WS_CAPTION | WS_SYSMENU);
		}
		
		if (viv_is_show_thickframe)	
		{
			newstyle |= WS_THICKFRAME;
		}
		else
		{
			newstyle &= ~WS_THICKFRAME;
		}

		if (viv_is_show_menu)	
		{
			if (GetMenu(viv_hwnd) != viv_main_menu)
			{
				SetMenu(viv_hwnd,viv_main_menu);
			}
		}
		else
		{
			if (GetMenu(viv_hwnd) != 0)
			{
				SetMenu(viv_hwnd,0);
			}
		}
		
		viv_status_show(viv_is_show_status);

		CopyRect(&newrect,&clientrect);
		AdjustWindowRect(&newrect,newstyle,viv_is_show_menu ? TRUE : FALSE);

		printf("newrect %d %d %d %d %d\n",newrect.left,newrect.top,newrect.right,newrect.bottom,viv_is_show_menu ? TRUE : FALSE);

		GetWindowRect(viv_hwnd,&windowrect);
		
		windowrect.left += newrect.left - oldrect.left;
		windowrect.top += newrect.top - oldrect.top;
		windowrect.right += newrect.right - oldrect.right;
		windowrect.bottom += newrect.bottom - oldrect.bottom;
	
		SetWindowLong(viv_hwnd,GWL_STYLE,newstyle);
		
		SetWindowPos(viv_hwnd,HWND_TOP,windowrect.left,windowrect.top,windowrect.right - windowrect.left,windowrect.bottom - windowrect.top,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOCOPYBITS);
	}
}

static void viv_update_ontop(void)
{
	int is_top_most;
	
	is_top_most = 0;
	
	switch(viv_ontop)
	{
		case 1:
			is_top_most = 1;
			break;
			
		case 2:
			is_top_most = (viv_is_slideshow) || ((viv_frame_count > 1) && (viv_animation_play));
			break;
	}

	SetWindowPos(viv_hwnd,is_top_most ? HWND_TOPMOST : HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
}

static void dst_pos_set(int x,int y)
{
	if (x < 0)
	{
		x = 0;
	}

	if (y < 0)
	{
		y = 0;
	}

	if (x > 1000)
	{
		x = 1000;
	}

	if (y > 1000)
	{
		y = 1000;
	}

	if ((x != viv_dst_pos_x) || (y != viv_dst_pos_y))
	{
		viv_dst_pos_x = x;
		viv_dst_pos_y = y;
		
		InvalidateRect(viv_hwnd,NULL,FALSE);
	}
}

static void dst_zoom_set(float x,float y)
{
	if (x < 0.2f)
	{
		x = 0.2f;
	}

	if (y < 0.2f)
	{
		y = 0.2f;
	}

	if (x > 3.0f)
	{
		x = 3.0f;
	}

	if (y > 3.0f)
	{
		y = 3.0f;
	}

	if ((x != viv_dst_zoom_x) || (y != viv_dst_zoom_y))
	{
		viv_dst_zoom_x = x;
		viv_dst_zoom_y = y;
		
		InvalidateRect(viv_hwnd,NULL,FALSE);
	}
}

static void viv_frame_skip(int size)
{
	if (viv_frame_count > 1)			
	{
		if (size > 0)
		{
			while(size > 0)
			{
				if ((viv_frame_loaded_count == viv_frame_count) || (viv_frame_position + 1 < viv_frame_loaded_count))
				{
					viv_frame_position++;
					if (viv_frame_position == viv_frame_count)
					{
						viv_frame_position = 0;
					}
				}

				size -= viv_frame_delays[viv_frame_position];
			}
		}
		else
		if (size < 0)
		{
			while(size < 0)
			{
				viv_frame_position--;
				if (viv_frame_position < 0)
				{
					viv_frame_position = viv_frame_loaded_count - 1;
				}
				
				size -= viv_frame_delays[viv_frame_position];
			}
		}
			
		InvalidateRect(viv_hwnd,NULL,FALSE);
		UpdateWindow(viv_hwnd);
	}
}

static unsigned __stdcall viv_load_image_proc(void *param)
{
	GdiplusStartupInput gdiplusStartupInput;
	viv_reply_load_image_first_frame_t first_frame;
	IStream *stream;

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);

	// Initialize GDI+.
	GdiplusStartup(&viv_gdiplusToken, &gdiplusStartupInput, NULL);

	first_frame.wide = 0;
	first_frame.high = 0;
	first_frame.frame_count = 0;
	first_frame.frame_delays = 0;
	first_frame.hbitmap = 0;

	stream = 0;

	printf("load %S...\n",viv_load_image_filename);
	
	{
		HANDLE h;
		
		h = CreateFile(viv_load_image_filename,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,0,0);
		if (h != INVALID_HANDLE_VALUE)
		{
			DWORD size;
			HANDLE global_handle;

			size = GetFileSize(h,0);
			
			global_handle = GlobalAlloc(GMEM_MOVEABLE,size);
			if (global_handle)
			{
				char *buf;
				
				buf = (char *)GlobalLock(global_handle);
				
				if (buf)
				{
					DWORD numread;
					DWORD totreadsize;
					char *d;
					
					d = buf;
					totreadsize = size;
					
					while(totreadsize)
					{
						DWORD readsize;

						if (viv_load_image_terminate)
						{
							break;
						}
						
						readsize = 1 * 1024 * 1024;
						
						if (readsize > totreadsize)
						{
							readsize = totreadsize;
						}
						
						if (ReadFile(h,d,readsize,&numread,0))
						{
							if (!numread)
							{
								break;
							}

							d += numread;
							totreadsize -= numread;
						}
						else
						{
							break;
						}
					}

					GlobalUnlock(global_handle);
					
					if (totreadsize == 0)
					{
						HRESULT hresult;
						
						hresult = CreateStreamOnHGlobal(global_handle,TRUE,&stream);
						
						if (SUCCEEDED(hresult))
						{
							// the stream owns this handle now.
							global_handle = 0;
						}
						else
						{
							printf("CreateStreamOnHGlobal %x\n",hresult);
							stream = 0;
						}
					}
				}
				
				if (global_handle)
				{
					GlobalFree(global_handle);
				}
			}

			CloseHandle(h);
		}
		else
		{
			printf("CreateFile %x\n",GetLastError());
		}
	}

	printf("stream %p\n",stream);

	if (stream)
	{
		if (!viv_load_image_terminate)
		{
			Image *image;
			
			image = new Image(stream);
			
			printf("image %p\n",image);
		
			if (image)
			{
				if (!viv_load_image_terminate)
				{
					Status last_status;
					
					last_status = image->GetLastStatus();
					if (last_status == Ok)
					{
						first_frame.wide = image->GetWidth();
						first_frame.high = image->GetHeight();
						
						//First of all we should get the number of frame dimensions
						//Images considered by GDI+ as:
						//frames[animation_frame_index][how_many_animation];
						UINT count = image->GetFrameDimensionsCount();

						//Now we should get the identifiers for the frame dimensions 
						{
							GUID *DimensionIDs;
							
							DimensionIDs = new GUID[count];
							image->GetFrameDimensionsList(DimensionIDs, count);
							
							//For gif image , we only care about animation set#0
							WCHAR strGuid[39];
							StringFromGUID2(DimensionIDs[0], strGuid, 39);
							first_frame.frame_count = image->GetFrameCount(&DimensionIDs[0]);

							delete DimensionIDs;
						}

						// get frame delays.
						if (first_frame.frame_count > 1)
						{
							PropertyItem *frame_delay;
							UINT size;
							DWORD i;

							//PropertyTagFrameDelay is a pre-defined identifier 
							//to present frame-delays by GDI+
							size = image->GetPropertyItemSize(PropertyTagFrameDelay);

							printf("frame delay size %d\n",size);

							frame_delay = (PropertyItem *)mem_alloc(size);
							image->GetPropertyItem(PropertyTagFrameDelay,size,frame_delay);
							
							first_frame.frame_delays = (DWORD *)mem_alloc(first_frame.frame_count * sizeof(int));

							for(i=0;i<first_frame.frame_count;i++)
							{
								first_frame.frame_delays[i] = (((UINT *)frame_delay[0].value)[i]) * 10;
								
								if (!first_frame.frame_delays[i])
								{
									first_frame.frame_delays[i] = 100;
								}
							}
							
							mem_free(frame_delay);
						}

						// draw frames.

						{
							HDC screen_hdc;
						
							screen_hdc = GetDC(0);
							
							if (screen_hdc)
							{
								HDC mem_hdc;
							
								mem_hdc = CreateCompatibleDC(screen_hdc);
								if (mem_hdc)
								{
									DWORD i;
									
									for(i=0;i<first_frame.frame_count;i++)
									{
										HBITMAP hbitmap;
										HGDIOBJ last_hbitmap;
										
										if (viv_load_image_terminate)
										{
											break;
										}
										
										hbitmap = CreateCompatibleBitmap(screen_hdc,first_frame.wide,first_frame.high);
										if (hbitmap)
										{
											last_hbitmap = SelectObject(mem_hdc,hbitmap);
											
											image->SelectActiveFrame(&FrameDimensionTime,i);

											if (image->GetFlags() & ImageFlagsHasAlpha)
											{
												{
													RECT rect;
													
													rect.left = 0;
													rect.top = 0;
													rect.right = first_frame.wide;
													rect.bottom = first_frame.high;

													FillRect(mem_hdc,&rect,(HBRUSH)GetStockObject(WHITE_BRUSH));
												}
												
												
												{
													Graphics *g;
													int ret;
													
													g = new Graphics(mem_hdc);
													
													g->SetCompositingMode(CompositingModeSourceOver);
													g->SetCompositingQuality(CompositingQualityHighSpeed);
													g->SetInterpolationMode(InterpolationModeNearestNeighbor);
													g->SetPixelOffsetMode(PixelOffsetModeNone);
													g->SetSmoothingMode(SmoothingModeNone);
												    
						//	printf("DrawImage...\n");

													ret = g->DrawImage(image,0,0,first_frame.wide,first_frame.high);
													
													if (ret)
													{
														printf("DrawImage failed %d\n",ret);
													}

													delete g;
												}
											}
											else
											{
												
												{
													Graphics *g;
													int ret;
													
													g = new Graphics(mem_hdc);
													
													g->SetCompositingMode(CompositingModeSourceCopy);
													g->SetCompositingQuality(CompositingQualityHighSpeed);
													g->SetInterpolationMode(InterpolationModeNearestNeighbor);
													g->SetPixelOffsetMode(PixelOffsetModeNone);
													g->SetSmoothingMode(SmoothingModeNone);
												    
						//	printf("DrawImage...\n");

													ret = g->DrawImage(image,0,0,first_frame.wide,first_frame.high);
													
													if (ret)
													{
														printf("DrawImage failed %d\n",ret);
													}

													delete g;
												}						
											}
											
											SelectObject(mem_hdc,last_hbitmap);
										
											if (i)
											{
												viv_reply_add(VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME,sizeof(HBITMAP),&hbitmap);
											}
											else
											{

												first_frame.hbitmap = hbitmap;
												viv_reply_add(VIV_REPLY_LOAD_IMAGE_FIRST_FRAME,sizeof(viv_reply_load_image_first_frame_t),&first_frame);
											}
										}
									}
									
									DeleteDC(mem_hdc);
								}
								
								ReleaseDC(0,screen_hdc);
							}
						}

						printf("image loaded\n");
					}
					else
					{
						printf("Failed to load image %S %d\n",viv_load_image_filename,last_status);
					}
				}

				delete image;
			}
			else
			{
				printf("Failed to load image %S\n",viv_load_image_filename);
			}
		}

		stream->Release();				
	}
	else
	{
		printf("Failed to create stream from %S\n",viv_load_image_filename);
	}

	GdiplusShutdown(viv_gdiplusToken);
			
	CoUninitialize();

	viv_reply_add(VIV_REPLY_LOAD_IMAGE_COMPLETE,0,0);
					
    _endthreadex(0);
    return 0;
}

static void viv_reply_free(viv_reply_t *e)
{
	switch (e->type)
	{
		case VIV_REPLY_LOAD_IMAGE_FIRST_FRAME:
			
			if (((viv_reply_load_image_first_frame_t *)(e+1))->frame_delays)
			{
				mem_free(((viv_reply_load_image_first_frame_t *)(e+1))->frame_delays);
			}
		
			if (((viv_reply_load_image_first_frame_t *)(e+1))->hbitmap)
			{
				DeleteObject(((viv_reply_load_image_first_frame_t *)(e+1))->hbitmap);
			}
		
			break;

		case VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME:
		
			if (*(HBITMAP *)(e + 1))
			{	
				DeleteObject(*(HBITMAP *)(e + 1));
			}
		
			break;
	}
	
	mem_free(e);
}

static viv_reply_t *viv_reply_add(DWORD type,DWORD size,void *data)
{
	viv_reply_t *e;
	int is_first;
	
	is_first = 0;
	
	e = (viv_reply_t *)mem_alloc(sizeof(viv_reply_t) + size);
	
	e->type = type;
	e->size = size;
	
	os_copy_memory(e + 1,data,size);
	
	EnterCriticalSection(&viv_cs);
	
	if (viv_reply_start)
	{
		viv_reply_last->next = e;
	}
	else
	{
		viv_reply_start = e;
		is_first = 1;
	}
	
	viv_reply_last = e;
	e->next = 0;
	
	LeaveCriticalSection(&viv_cs);
	
	if (is_first)
	{
		PostMessage(viv_hwnd,VIV_WM_REPLY,0,0);
	}
	
	return e;
}

static void viv_reply_clear_all(void)
{
	viv_reply_t *e;
	
	EnterCriticalSection(&viv_cs);
	
	e = viv_reply_start;
	viv_reply_start = 0;
	viv_reply_last = 0;
	
	LeaveCriticalSection(&viv_cs);
	
	while(e)
	{
		viv_reply_t *next_e;
		
		next_e = e->next;
		
		viv_reply_free(e);
		
		e = next_e;
	}
}

static int viv_opengl_init(void)
{
	int ret;

	ret = 0;
	
	if (!viv_opengl32_hmodule)
	{
		viv_opengl32_hmodule = LoadLibraryA("opengl32.dll");
	}
	
	if (viv_opengl32_hmodule)
	{
		viv_opengl_proc_t *proc;
		HDC hdc;
		
		proc = viv_opengl_procs;
		for(;;)
		{
			if (!proc->proc)
			{
				break;
			}
			
			*proc->proc = GetProcAddress(viv_opengl32_hmodule,proc->name);
			if (!*proc->proc)
			{
				goto error;
			}
			
			proc++;
		}
		
		// init opengl.
		hdc = GetDC(viv_hwnd);
		if (hdc)
		{
			PIXELFORMATDESCRIPTOR pfd;
			int pixel_format;
			
			os_zero_memory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));
			pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_TYPE_RGBA;
			pfd.iLayerType = PFD_MAIN_PLANE;
			pfd.cColorBits = 24;
			pfd.cDepthBits = 32;

			pixel_format = ChoosePixelFormat(hdc,&pfd);
			if (pixel_format)
			{
				if (SetPixelFormat(hdc,pixel_format,&pfd))
				{
					if (DescribePixelFormat(hdc,pixel_format,sizeof(PIXELFORMATDESCRIPTOR),&pfd))
					{
						viv_hglrc = viv_wglCreateContext(hdc);
						if (viv_hglrc)
						{
							printf("opengl ok\n");
							
							ret = 1;
						}
						else
						{
							printf("viv_wglCreateContext %d\n",viv_glGetError());
						}
					}
					else
					{
						printf("DescribePixelFormat %d\n",GetLastError());
					}
				}
				else
				{
					printf("SetPixelFormat %x\n",GetLastError());
				}
			}
			else
			{
				printf("ChoosePixelFormat %d\n",GetLastError());
			}
			
			ReleaseDC(viv_hwnd,hdc);
		}
		else
		{
			printf("GetDC %d\n",GetLastError());
		}
	}
	else
	{
		printf("opengl32 %d\n",GetLastError());
	}
	
error:

	return ret;
}

static void viv_opengl_destroy(void)
{
	if (viv_hglrc)
	{
		viv_wglDeleteContext(viv_hglrc);
		
		viv_hglrc = 0;
	}
}

static void viv_status_show(int show)
{
	if (show)
	{
		if (!viv_status_hwnd)
		{
			viv_status_hwnd = CreateWindowExW(
				0,
				STATUSCLASSNAME,
				L"",
				WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | SBARS_SIZEGRIP | WS_VISIBLE,
				0,0,0,0,
				viv_hwnd,(HMENU)VIV_ID_STATUS,GetModuleHandle(0),NULL);
				
			// set status font
			/*
			{
				HFONT hfont;
				int status_high;
				
				hfont = os_default_hfont;

				if ((*config_fonts[CONFIG_FONT_STATUS_BAR].font) || (config_fonts[CONFIG_FONT_STATUS_BAR].size))
				{
					ui->status_bar_hfont = ui_hfont_create(config_fonts[CONFIG_FONT_STATUS_BAR].font,config_fonts[CONFIG_FONT_STATUS_BAR].size);
					hfont = ui->status_bar_hfont;
				}
				
				status_high = os_get_font_height_from_hwnd(ui->hwnd,hfont);
				SendMessage(ui->status_hwnd,WM_SETFONT,(WPARAM)hfont,0);

				// set status height.
			}
			*/	
		}
	}
	else
	{
		if (viv_status_hwnd)
		{
			DestroyWindow(viv_status_hwnd);

			viv_status_hwnd = 0;
		}	
	}
	
	viv_on_size();
}

static void viv_status_update(void)
{
	if (viv_status_hwnd)
	{
		int part_array[2];
		RECT rect;
		wchar_t widebuf[64];
		wchar_t highbuf[64];
		wchar_t bppbuf[64];
		wchar_t wbuf[MAX_PATH];
		HDC hdc;
		int dimension_wide;
		
		GetClientRect(viv_hwnd,&rect);
		
		wstring_format_number(widebuf,viv_image_wide);
		wstring_format_number(highbuf,viv_image_high);
		
		wstring_copy(wbuf,MAX_PATH,widebuf);
		wstring_cat(wbuf,MAX_PATH,L" x ");
		wstring_cat(wbuf,MAX_PATH,highbuf);
		
		dimension_wide = 100;

		hdc = GetDC(viv_status_hwnd);
		if (hdc)
		{
			HFONT hfont;
			
			hfont = (HFONT)SendMessage(viv_status_hwnd,WM_GETFONT,0,0);
			
			if (hfont)
			{
				SIZE size;
				
				if (GetTextExtentPoint(hdc,wbuf,wstring_length(wbuf),&size))
				{
					dimension_wide = size.cx + GetSystemMetrics(SM_CXEDGE) * 5;
				}
			}
			
			ReleaseDC(viv_status_hwnd,hdc);
		}
		
		
		part_array[0] = rect.right - rect.left - dimension_wide;
		if (part_array[0] < 0)
		{
			part_array[0] = 0;
		}
		
		part_array[1] = -1;
		
		SendMessage(viv_status_hwnd,SB_SETPARTS,2,(LPARAM)part_array);
	
		viv_status_set(0,viv_load_image_thread ? L"Loading..." : L"");
		viv_status_set(1,wbuf);
	}
}

static void viv_status_set(int part,const wchar_t *text)
{
	wchar_t oldtext[MAX_PATH];
	
	SendMessage(viv_status_hwnd,SB_GETTEXTW,(WPARAM)part,(LPARAM)oldtext);
	
	if (wcscmp(oldtext,text) != 0)
	{	
		SendMessage(viv_status_hwnd,SB_SETTEXTW,(WPARAM)part,(LPARAM)text);
	}
}

static int viv_get_status_high(void)
{
	if (viv_status_hwnd)
	{
		RECT rect;
		
		GetWindowRect(viv_status_hwnd,&rect);
		
		return rect.bottom - rect.top;
	}
	
	return 0;
}

#endif 
