//
// Copyright 2026 voidtools / David Carpenter
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
// viv_settings.c - the modern settings window (gui remake round).
//
// one owned popup window replaces the old tabbed options dialog: a left
// navigation (general / view / controls) and one self drawn content area.
// there are no child controls - a single wm_paint pass draws every row on
// a double buffered dc, a rect state machine tracks hover / press / focus,
// and the dropdowns are hmenu popups opened with trackpopupmenuex (the app
// wide dark menu mode makes them follow the theme).
//
// behavior model: the window opens with a snapshot of every config field it
// can touch, each change applies immediately (the same calls the options
// dialog ran from its ok path), cancel rewinds the snapshot and ok saves
// the settings. the keyboard shortcuts editor works on a private copy of
// the key list that is handed to the real list only on ok.
#include "viv.h"
#include "viv_state.h"
#include "viv_chrome.h"
#include "viv_load.h"
#include "viv_menu.h"
#include "viv_render.h"
#include "viv_install.h"
#include "viv_menubar.h"
#include "viv_settings.h"

// per monitor dpi change message. (not defined in older SDKs)
#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

#ifndef BN_CLICKED
#define BN_CLICKED 0
#endif

// window metrics (dip at 96 dpi).
#define _VIV_SETTINGS_CLIENT_WIDE	660
#define _VIV_SETTINGS_CLIENT_HIGH       628
#define _VIV_SETTINGS_NAV_WIDE		150
#define _VIV_SETTINGS_NAV_ITEM_HIGH	36
#define _VIV_SETTINGS_NAV_TOP		12
#define _VIV_SETTINGS_NAV_X			8
#define _VIV_SETTINGS_CONTENT_PAD	24
#define _VIV_SETTINGS_CONTENT_TOP	18
#define _VIV_SETTINGS_SECTION_HIGH	30
#define _VIV_SETTINGS_DESC_HIGH		18
#define _VIV_SETTINGS_ROW_HIGH		34
#define _VIV_SETTINGS_ROW_HIGH_DESC	52
#define _VIV_SETTINGS_COLOR_HIGH	36
#define _VIV_SETTINGS_CHECK_HIGH	26
#define _VIV_SETTINGS_CHECK_COLS	4
#define _VIV_SETTINGS_VALUE_WIDE	180
#define _VIV_SETTINGS_VALUE_HIGH	28
#define _VIV_SETTINGS_SWITCH_WIDE	44
#define _VIV_SETTINGS_SWITCH_HIGH	24
#define _VIV_SETTINGS_SWATCH_WIDE	64
#define _VIV_SETTINGS_SWATCH_HIGH	22
#define _VIV_SETTINGS_BUTTON_WIDE	160
#define _VIV_SETTINGS_BUTTON_HIGH	32
#define _VIV_SETTINGS_KEYBUTTON_WIDE	88
#define _VIV_SETTINGS_KEYBUTTON_HIGH	26
#define _VIV_SETTINGS_FOOTER_HIGH	76
#define _VIV_SETTINGS_TITLE_HIGH        44
#define _VIV_SETTINGS_TITLE_X_WIDE      36
#define _VIV_SETTINGS_ACCENT_D          20
#define _VIV_SETTINGS_ACCENT_GAP        10
#define _VIV_SETTINGS_ACCENT_STRIP      140

// run key (startup shortcut). written directly here: os.c has no
// registry helper for a plain hkcu value.
#define _VIV_SETTINGS_RUN_KEY_PATH	L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define _VIV_SETTINGS_RUN_KEY_VALUE	L"voidImageViewerPLUS"

// pages. the order matches _VIV_OPTIONS_PAGE_COUNT and
// config_options_last_page: 0 general, 1 view, 2 controls.
#define _VIV_SETTINGS_PAGE_GENERAL	0
#define _VIV_SETTINGS_PAGE_VIEW		1
#define _VIV_SETTINGS_PAGE_CONTROLS	2

// control kinds. the paint and the state machine switch on these.
#define _VIV_SETTINGS_CT_SECTION	0
#define _VIV_SETTINGS_CT_DESC		1
#define _VIV_SETTINGS_CT_NAV		2
#define _VIV_SETTINGS_CT_DROPDOWN	3
#define _VIV_SETTINGS_CT_SWITCH		4
#define _VIV_SETTINGS_CT_CHECK		5
#define _VIV_SETTINGS_CT_COLOR		6
#define _VIV_SETTINGS_CT_BUTTON		7
#define _VIV_SETTINGS_CT_KEYBUTTON	8
#define _VIV_SETTINGS_CT_ACCENT         9

// control ids (window local; not viv command ids).
#define _VIV_SETTINGS_ID_NONE		0
#define _VIV_SETTINGS_ID_NAV		1			// + page (param)
#define _VIV_SETTINGS_ID_LANGUAGE	10
#define _VIV_SETTINGS_ID_THEME		11
#define _VIV_SETTINGS_ID_MULTIPLE	12
#define _VIV_SETTINGS_ID_RUNKEY		13
#define _VIV_SETTINGS_ID_SELECT_ALL	14
#define _VIV_SETTINGS_ID_ASSOC		15			// + extension index (param)
#define _VIV_SETTINGS_ID_ACCENT         16
#define _VIV_SETTINGS_ID_APPDATA		17
#define _VIV_SETTINGS_ID_STARTMENU		18
#define _VIV_SETTINGS_ID_SHRINK		20
#define _VIV_SETTINGS_ID_MAG		21
#define _VIV_SETTINGS_ID_TITLE		22
#define _VIV_SETTINGS_ID_AUTOZOOM	23
#define _VIV_SETTINGS_ID_AUTOTYPE	24
#define _VIV_SETTINGS_ID_LOOP		25
#define _VIV_SETTINGS_ID_PRELOAD	26
#define _VIV_SETTINGS_ID_CACHE		27
#define _VIV_SETTINGS_ID_WINCOLOR	28
#define _VIV_SETTINGS_ID_FSCOLOR	29
#define _VIV_SETTINGS_ID_LEFT		30
#define _VIV_SETTINGS_ID_RIGHT		31
#define _VIV_SETTINGS_ID_WHEEL		32
#define _VIV_SETTINGS_ID_COMMAND	33
#define _VIV_SETTINGS_ID_KEYS		34
#define _VIV_SETTINGS_ID_KEY_ADD	35
#define _VIV_SETTINGS_ID_KEY_EDIT	36
#define _VIV_SETTINGS_ID_KEY_REMOVE	37
#define _VIV_SETTINGS_ID_CANCEL		40
#define _VIV_SETTINGS_ID_OK			41

// palette indices.
#define _VIV_SETTINGS_C_FACE		0
#define _VIV_SETTINGS_C_NAV_FACE	1
#define _VIV_SETTINGS_C_INPUT		2
#define _VIV_SETTINGS_C_LINE		3
#define _VIV_SETTINGS_C_HOVER		4
#define _VIV_SETTINGS_C_PRESS		5
#define _VIV_SETTINGS_C_TEXT		6
#define _VIV_SETTINGS_C_TEXT2		7
#define _VIV_SETTINGS_C_TEXTOFF		8
#define _VIV_SETTINGS_C_ACCENT		9
#define _VIV_SETTINGS_C_ACCENT_HOT	10
#define _VIV_SETTINGS_C_ACCENT_DOWN	11
#define _VIV_SETTINGS_C_ON_ACCENT	12
#define _VIV_SETTINGS_C_NAV_HOVER	13
#define _VIV_SETTINGS_COUNT			14

// one row of the rect state machine.
typedef struct _viv_settings_ctl_s
{
	int type;
	int id;
	int param;		// nav page / association index / enabled state
	RECT rect;		// the full row (the hit target)
	RECT value;		// the right hand field (dropdown box / switch / swatch)

} _viv_settings_ctl_t;

// every control of the current page. the array order is the tab order.
#define _VIV_SETTINGS_CTL_MAX 30

static HWND _viv_settings_hwnd = 0;
static int _viv_settings_is_registered = 0;
static int _viv_settings_dpi = 96;
static int _viv_settings_page = _VIV_SETTINGS_PAGE_GENERAL;

static _viv_settings_ctl_t _viv_settings_ctls[_VIV_SETTINGS_CTL_MAX];
static int _viv_settings_ctl_count = 0;

static int _viv_settings_hot = -1;		// ctl under the cursor
static int _viv_settings_pressed = -1;	// ctl pressed with capture
static int _viv_settings_focus = -1;	// ctl with keyboard focus
static BYTE _viv_settings_tracking = 0;	// the mouse leave tracking is armed

static HFONT _viv_settings_font = 0;
static HFONT _viv_settings_font_bold = 0;
static HFONT _viv_settings_font_small = 0;

// the title row close button hover / press state and the hovered
// swatch inside the accent color row (-1 none).
static int _viv_settings_title_hot = 0;
static int _viv_settings_title_pressed = 0;
static int _viv_settings_hot_swatch = -1;
// the pending states of the two admin rows (appdata storage and the
// start menu shortcuts). they cannot apply live: the change is done
// by a relaunched elevated instance, so the toggle takes effect on
// ok (the options dialog behavior) and cancel just drops it.
static int _viv_settings_appdata;
static int _viv_settings_startmenu;
// the open window snapshot: cancel rewinds to these.
static int _viv_settings_snap_language;
static int _viv_settings_snap_dark_mode;
static int _viv_settings_snap_accent;
static int _viv_settings_snap_multiple_instances;
static int _viv_settings_snap_run_key;
static int _viv_settings_snap_shrink_blit;
static int _viv_settings_snap_mag;
static int _viv_settings_snap_title;
static int _viv_settings_snap_auto_zoom;
static int _viv_settings_snap_auto_type;
static int _viv_settings_snap_loop;
static int _viv_settings_snap_preload;
static int _viv_settings_snap_cache;
static int _viv_settings_snap_left;
static int _viv_settings_snap_right;
static int _viv_settings_snap_wheel;
static int _viv_settings_snap_assoc[_VIV_ASSOCIATION_COUNT];
static BYTE _viv_settings_snap_windowed_r;
static BYTE _viv_settings_snap_windowed_g;
static BYTE _viv_settings_snap_windowed_b;
static BYTE _viv_settings_snap_fullscreen_r;
static BYTE _viv_settings_snap_fullscreen_g;
static BYTE _viv_settings_snap_fullscreen_b;

// association check states (registry queried once at open, updated on
// each toggle so painting does not hit the registry).
static int _viv_settings_assoc[_VIV_ASSOCIATION_COUNT];

// the run key switch state, cached the same way: the paint pass reads
// this, the registry is touched at open, on page enter and on writes.
static int _viv_settings_run_key_state;

// keyboard shortcut editor: a private copy of the key list. the real
// list is replaced only on ok (the options dialog behavior).
static _viv_key_list_t _viv_settings_keylist;
static int _viv_settings_command_index = 0;
static int _viv_settings_command_item_count = 0;
static int _viv_settings_key_index = -1;

// the key capture mode (the inline replacement for the edit key dialog).
static int _viv_settings_capture_active = 0;
static int _viv_settings_capture_edit = 0;
static DWORD _viv_settings_capture_key = 0;

static LRESULT CALLBACK _viv_settings_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

static void _viv_settings_fonts_create(void);
static void _viv_settings_fonts_delete(void);
static void _viv_settings_layout(void);
static void _viv_settings_paint(HWND hwnd);
static int _viv_settings_hit_test(int x,int y);
static void _viv_settings_invalidate(void);
static void _viv_settings_track_leave(HWND hwnd);
static void _viv_settings_focus_set(int index,int invalidate);
static int _viv_settings_focus_next(int from,int dir);
static void _viv_settings_focus_cycle(int dir);
static void _viv_settings_activate(int index,int x,int y);
static void _viv_settings_theme_update(void);
static void _viv_settings_snapshot(void);
static void _viv_settings_restore(void);
static void _viv_settings_ok(void);
static void _viv_settings_cancel(void);
static void _viv_settings_close(void);
static void _viv_settings_set_language(int language);
static void _viv_settings_language_refresh(void);
static void _viv_settings_dark_apply(void);
static void _viv_settings_accent_set(int accent);
static void _viv_settings_run_key_set(int on);
static int _viv_settings_run_key_present(void);
static int _viv_settings_command_at(int item_index);
static int _viv_settings_key_count(void);
static DWORD _viv_settings_key_at(int index);
static void _viv_settings_key_index_clamp(void);
static void _viv_settings_capture_begin(int edit);
static void _viv_settings_capture_end(void);
static void _viv_settings_capture_commit(void);
static void _viv_settings_window_size_px(int *wide,int *high);
static int _viv_settings_popup(HWND hwnd,const RECT *anchor,int kind,const void *context,int count,int current);
static void _viv_settings_popup_text(int kind,const void *context,int index,wchar_t *wbuf);
static int _viv_settings_token(int which);
static COLORREF _viv_settings_color(int which);
static int _viv_settings_dip(int d);
static void _viv_settings_ctl_add(int type,int id,int param,int x,int y,int wide,int high);
static void _viv_settings_draw_text_raw(HDC hdc,const RECT *rect,const wchar_t *text,HFONT font,COLORREF color,int flags);
static void _viv_settings_draw_label(HDC hdc,const RECT *rect,int localization_id,HFONT font,COLORREF color);
static void _viv_settings_draw_nav(HDC hdc,const _viv_settings_ctl_t *ctl,int selected,int hot,int inactive);
static void _viv_settings_draw_accent(HDC hdc,const _viv_settings_ctl_t *ctl,int hot,int focus);
static void _viv_settings_draw_title(HDC hdc,const RECT *client);
static void _viv_settings_title_close_rect(const RECT *client,RECT *rect);
static int _viv_settings_title_close_hit(int x,int y);
static int _viv_settings_swatch_from_x(const _viv_settings_ctl_t *ctl,int x);
static void _viv_settings_draw_dropdown(HDC hdc,const _viv_settings_ctl_t *ctl,int hot,int focus);
static void _viv_settings_draw_switch(HDC hdc,const _viv_settings_ctl_t *ctl,int on,int hot,int focus);
static void _viv_settings_draw_check(HDC hdc,const _viv_settings_ctl_t *ctl,int checked,int hot,int focus);
static void _viv_settings_draw_color(HDC hdc,const _viv_settings_ctl_t *ctl,COLORREF colorref,int hot,int focus);
static void _viv_settings_draw_button(HDC hdc,const _viv_settings_ctl_t *ctl,int hot,int pressed,int focus);
static void _viv_settings_draw_focus_ring(HDC hdc,const RECT *rect);
static void _viv_settings_capture_used_by_text(wchar_t *wbuf);
static void _viv_settings_fill_round(HDC hdc,const RECT *rect,int radius,COLORREF fill,COLORREF line);

// dips at the window dpi.
static int _viv_settings_dip(int d)
{
	return (d * _viv_settings_dpi) / 96;
}

// the palette. every settings color is a thin map onto the viv_theme
// tokens: a theme flip or an accent flip re-skins this window from the
// one shared table, with no per surface rgb and no local brush cache.
static int _viv_settings_token(int which)
{
	switch(which)
	{
		case _VIV_SETTINGS_C_FACE: return VIV_TK_FACE;
		case _VIV_SETTINGS_C_NAV_FACE: return VIV_TK_NAV_FACE;
		case _VIV_SETTINGS_C_INPUT: return VIV_TK_INPUT;
		case _VIV_SETTINGS_C_LINE: return VIV_TK_LINE;
		case _VIV_SETTINGS_C_HOVER: return VIV_TK_HOVER;
		case _VIV_SETTINGS_C_PRESS: return VIV_TK_DOWN;
		case _VIV_SETTINGS_C_TEXT: return VIV_TK_TEXT;
		case _VIV_SETTINGS_C_TEXT2: return VIV_TK_TEXT2;
		case _VIV_SETTINGS_C_TEXTOFF: return VIV_TK_TEXTOFF;
		case _VIV_SETTINGS_C_ACCENT: return VIV_TK_ACCENT;
		case _VIV_SETTINGS_C_ACCENT_HOT: return VIV_TK_ACCENT_HOT;
		case _VIV_SETTINGS_C_ACCENT_DOWN: return VIV_TK_ACCENT_DOWN;
		case _VIV_SETTINGS_C_ON_ACCENT: return VIV_TK_ON_ACCENT;
		case _VIV_SETTINGS_C_NAV_HOVER: return VIV_TK_HOVER;

		default:
			return VIV_TK_FACE;
	}

	return VIV_TK_FACE;
}

static COLORREF _viv_settings_color(int which)
{
	return viv_theme_color(_viv_settings_token(which));
}

// the fill brush of the paint pass. viv_theme owns the cache and
// flushes it on a theme or an accent flip (see _viv_settings_theme_update).
static HBRUSH _viv_settings_brush(int which)
{
	return viv_theme_brush(_viv_settings_token(which));
}

// run key: hkcu\...\run value "voidImageViewerPLUS" = quoted exe path.
static int _viv_settings_run_key_present(void)
{
	HKEY hkey;
	int ret;

	ret = 0;

	if (RegOpenKeyExW(HKEY_CURRENT_USER,_VIV_SETTINGS_RUN_KEY_PATH,0,KEY_QUERY_VALUE,&hkey) == ERROR_SUCCESS)
	{
		ret = (RegQueryValueExW(hkey,_VIV_SETTINGS_RUN_KEY_VALUE,0,NULL,NULL,NULL) == ERROR_SUCCESS) ? 1 : 0;

		RegCloseKey(hkey);
	}

	return ret;
}

static void _viv_settings_run_key_set(int on)
{
	HKEY hkey;

	if (RegCreateKeyExW(HKEY_CURRENT_USER,_VIV_SETTINGS_RUN_KEY_PATH,0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hkey,NULL) == ERROR_SUCCESS)
	{
		if (on)
		{
			wchar_t exe_filename[STRING_SIZE];
			wchar_t value_wbuf[STRING_SIZE];

			_viv_get_exe_filename(exe_filename);

			string_copy(value_wbuf,L"\"");
			string_cat(value_wbuf,exe_filename);
			string_cat(value_wbuf,L"\"");

			RegSetValueExW(hkey,_VIV_SETTINGS_RUN_KEY_VALUE,0,REG_SZ,(const BYTE *)value_wbuf,(string_get_length(value_wbuf)+1)*sizeof(wchar_t));
		}
		else
		{
			RegDeleteValueW(hkey,_VIV_SETTINGS_RUN_KEY_VALUE);
		}

		RegCloseKey(hkey);
	}
}

#ifdef VIVP_SELF_SHOT
static void vivp_dpi_probe(const char *tag)
{
	HANDLE fh;
	char buf[96];
	DWORD w;
	int len;

	fh = CreateFileW(L"C:\\shots\\dpi.log",FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_ALWAYS,0,0);

	if (fh == INVALID_HANDLE_VALUE)
	{
		return;
	}

	len = 0;

	while ((tag[len]) && (len < 60))
	{
		buf[len] = tag[len];
		len++;
	}

	buf[len++] = ' ';
	buf[len++] = (char)('0' + (_viv_settings_dpi / 100));
	buf[len++] = (char)('0' + ((_viv_settings_dpi / 10) % 10));
	buf[len++] = (char)('0' + (_viv_settings_dpi % 10));
	buf[len++] = '\r';
	buf[len++] = '\n';

	WriteFile(fh,buf,len,&w,0);
	CloseHandle(fh);
}
#else
#define vivp_dpi_probe(t) ((void)0)
#endif

// fonts: the message font face at the window dpi, sized by the design
// (13 dip rows, 12 dip descriptions, bold section titles).
static void _viv_settings_fonts_create(void)
{
	LOGFONTW lf;

	_viv_settings_fonts_delete();

	if (!os_dialog_font(&lf,_viv_settings_hwnd))
	{
		return;
	}

	lf.lfHeight = -_viv_settings_dip(13);
	lf.lfWeight = FW_NORMAL;

	_viv_settings_font = CreateFontIndirectW(&lf);

	lf.lfWeight = FW_BOLD;

	_viv_settings_font_bold = CreateFontIndirectW(&lf);

	lf.lfWeight = FW_NORMAL;
	lf.lfHeight = -_viv_settings_dip(12);

	_viv_settings_font_small = CreateFontIndirectW(&lf);
}

static void _viv_settings_fonts_delete(void)
{
	if (_viv_settings_font)
	{
		DeleteObject(_viv_settings_font);

		_viv_settings_font = 0;
	}

	if (_viv_settings_font_bold)
	{
		DeleteObject(_viv_settings_font_bold);

		_viv_settings_font_bold = 0;
	}

	if (_viv_settings_font_small)
	{
		DeleteObject(_viv_settings_font_small);

		_viv_settings_font_small = 0;
	}
}

// the full window rect for the client size.
static void _viv_settings_window_size_px(int *wide,int *high)
{
	RECT rect;

	rect.left = 0;
	rect.top = 0;
	rect.right = _viv_settings_dip(_VIV_SETTINGS_CLIENT_WIDE);
	rect.bottom = _viv_settings_dip(_VIV_SETTINGS_CLIENT_HIGH);

	AdjustWindowRect(&rect,WS_POPUP|WS_SYSMENU,FALSE);

	*wide = rect.right - rect.left;
	*high = rect.bottom - rect.top;
}

// add one control to the current page layout.
static void _viv_settings_ctl_add(int type,int id,int param,int x,int y,int wide,int high)
{
	_viv_settings_ctl_t *ctl;

	if (_viv_settings_ctl_count >= _VIV_SETTINGS_CTL_MAX)
	{
		return;
	}

	ctl = &_viv_settings_ctls[_viv_settings_ctl_count++];

	ctl->type = type;
	ctl->id = id;
	ctl->param = param;
	ctl->rect.left = x;
	ctl->rect.top = y;
	ctl->rect.right = x + wide;
	ctl->rect.bottom = y + high;
	ctl->value.left = 0;
	ctl->value.top = 0;
	ctl->value.right = 0;
	ctl->value.bottom = 0;
}

// is the control reachable by the mouse and the keyboard?
static int _viv_settings_ctl_enabled(const _viv_settings_ctl_t *ctl)
{
	switch(ctl->type)
	{
		case _VIV_SETTINGS_CT_SECTION:
		case _VIV_SETTINGS_CT_DESC:
			return 0;

		case _VIV_SETTINGS_CT_DROPDOWN:

			if (ctl->id == _VIV_SETTINGS_ID_AUTOTYPE)
			{
				return config_auto_zoom ? 1 : 0;
			}

			return 1;

		case _VIV_SETTINGS_CT_KEYBUTTON:

			if (ctl->id == _VIV_SETTINGS_ID_KEY_EDIT)
			{
				return _viv_settings_key_count() ? 1 : 0;
			}

			if (ctl->id == _VIV_SETTINGS_ID_KEY_REMOVE)
			{
				return _viv_settings_key_count() ? 1 : 0;
			}

			return 1;

		default:
			break;
	}

	return 1;
}

// build the rects of the current page. the footer and the navigation
// exist on every page; the content cursor flows top to bottom.
static void _viv_settings_layout(void)
{
	RECT client;
	int nav_wide;
	int content_x;
	int content_wide;
	int y;
	int x;
	int i;
	int cell_wide;
	int row_high;
	int footer_y;

	vivp_dpi_probe("layout");

	if (!_viv_settings_hwnd)
	{
		return;
	}

	GetClientRect(_viv_settings_hwnd,&client);

	_viv_settings_ctl_count = 0;

	// the navigation: one item per page.
	nav_wide = _viv_settings_dip(_VIV_SETTINGS_NAV_WIDE);

	for(i=0;i<_VIV_OPTIONS_PAGE_COUNT;i++)
	{
		_viv_settings_ctl_add(_VIV_SETTINGS_CT_NAV,_VIV_SETTINGS_ID_NAV,i,
			_viv_settings_dip(_VIV_SETTINGS_NAV_X),
			_viv_settings_dip(_VIV_SETTINGS_TITLE_HIGH) + _viv_settings_dip(_VIV_SETTINGS_NAV_TOP) + (i * _viv_settings_dip(_VIV_SETTINGS_NAV_ITEM_HIGH)),
			nav_wide - (_viv_settings_dip(_VIV_SETTINGS_NAV_X) * 2),
			_viv_settings_dip(_VIV_SETTINGS_NAV_ITEM_HIGH));
	}

	content_x = nav_wide + _viv_settings_dip(_VIV_SETTINGS_CONTENT_PAD);
	content_wide = (client.right - _viv_settings_dip(_VIV_SETTINGS_CONTENT_PAD)) - content_x;

	y = _viv_settings_dip(_VIV_SETTINGS_TITLE_HIGH) + _viv_settings_dip(_VIV_SETTINGS_CONTENT_TOP);

	switch(_viv_settings_page)
	{
		case _VIV_SETTINGS_PAGE_GENERAL:
		{
			// interface section: language and theme dropdowns.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_SECTION,_VIV_SETTINGS_ID_NONE,LOCALIZATION_ID_SETTINGS_SECTION_INTERFACE,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_SECTION_HIGH));
			y += _viv_settings_dip(_VIV_SETTINGS_SECTION_HIGH);

			_viv_settings_ctl_add(_VIV_SETTINGS_CT_DROPDOWN,_VIV_SETTINGS_ID_LANGUAGE,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_VALUE_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = content_x + content_wide;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH);
			y += row_high;

			_viv_settings_ctl_add(_VIV_SETTINGS_CT_DROPDOWN,_VIV_SETTINGS_ID_THEME,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_VALUE_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = content_x + content_wide;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH);
			y += row_high;

			// the accent color row: five round swatches, the active one
			// ringed and checked. a pick applies at once.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_ACCENT,_VIV_SETTINGS_ID_ACCENT,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_ACCENT_STRIP);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + (_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH) - _viv_settings_dip(_VIV_SETTINGS_ACCENT_D)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = content_x + content_wide;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_ACCENT_D);
			y += _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH);

			y += _viv_settings_dip(10);

			// startup and window section: two switch rows with a description.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_SECTION,_VIV_SETTINGS_ID_NONE,LOCALIZATION_ID_SETTINGS_SECTION_STARTUP,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_SECTION_HIGH));
			y += _viv_settings_dip(_VIV_SETTINGS_SECTION_HIGH);

			_viv_settings_ctl_add(_VIV_SETTINGS_CT_SWITCH,_VIV_SETTINGS_ID_MULTIPLE,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH_DESC));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH_DESC)) - _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = _viv_settings_ctls[_viv_settings_ctl_count-1].value.left + _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH);
			y += row_high;

			_viv_settings_ctl_add(_VIV_SETTINGS_CT_SWITCH,_VIV_SETTINGS_ID_RUNKEY,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH_DESC));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH_DESC)) - _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = _viv_settings_ctls[_viv_settings_ctl_count-1].value.left + _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH);
			y += row_high;

			// start menu shortcuts and the appdata storage: the two rows the
			// old dialog queued for the elevated helper instance. the toggle
			// is pending state, the ok path relaunches.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_SWITCH,_VIV_SETTINGS_ID_STARTMENU,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = _viv_settings_ctls[_viv_settings_ctl_count-1].value.left + _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH);
			y += row_high;

			_viv_settings_ctl_add(_VIV_SETTINGS_CT_SWITCH,_VIV_SETTINGS_ID_APPDATA,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = _viv_settings_ctls[_viv_settings_ctl_count-1].value.left + _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH);
			y += row_high;

			y += _viv_settings_dip(10);

			// file associations: section, description, a four column
			// checkbox grid and the select all cell at the row end.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_SECTION,_VIV_SETTINGS_ID_NONE,LOCALIZATION_ID_SETTINGS_SECTION_ASSOCIATIONS,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_SECTION_HIGH));
			y += _viv_settings_dip(_VIV_SETTINGS_SECTION_HIGH);

			_viv_settings_ctl_add(_VIV_SETTINGS_CT_DESC,_VIV_SETTINGS_ID_NONE,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_DESC_HIGH));
			y += _viv_settings_dip(_VIV_SETTINGS_DESC_HIGH) + _viv_settings_dip(4);

			cell_wide = content_wide / _VIV_SETTINGS_CHECK_COLS;

			for(i=0;i<_VIV_ASSOCIATION_COUNT;i++)
			{
				x = content_x + ((i % _VIV_SETTINGS_CHECK_COLS) * cell_wide);

				_viv_settings_ctl_add(_VIV_SETTINGS_CT_CHECK,_VIV_SETTINGS_ID_ASSOC,i,x,y,cell_wide,_viv_settings_dip(_VIV_SETTINGS_CHECK_HIGH));

				if ((i % _VIV_SETTINGS_CHECK_COLS) == (_VIV_SETTINGS_CHECK_COLS - 1))
				{
					y += _viv_settings_dip(_VIV_SETTINGS_CHECK_HIGH);
				}
			}

			// the select all cell sits at the end of the last grid row.
			x = content_x + ((_VIV_ASSOCIATION_COUNT % _VIV_SETTINGS_CHECK_COLS) * cell_wide);

			_viv_settings_ctl_add(_VIV_SETTINGS_CT_CHECK,_VIV_SETTINGS_ID_SELECT_ALL,0,x,y,cell_wide,_viv_settings_dip(_VIV_SETTINGS_CHECK_HIGH));

			y += _viv_settings_dip(_VIV_SETTINGS_CHECK_HIGH);

			break;
		}

		case _VIV_SETTINGS_PAGE_VIEW:
		{
			// shrink blit mode.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_DROPDOWN,_VIV_SETTINGS_ID_SHRINK,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_VALUE_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = content_x + content_wide;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH);
			y += row_high;

			// magnify blit mode.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_DROPDOWN,_VIV_SETTINGS_ID_MAG,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_VALUE_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = content_x + content_wide;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH);
			y += row_high;

			// title bar format.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_DROPDOWN,_VIV_SETTINGS_ID_TITLE,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_VALUE_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = content_x + content_wide;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH);
			y += row_high;

			y += _viv_settings_dip(10);

			// auto size window switch + its percent dropdown.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_SWITCH,_VIV_SETTINGS_ID_AUTOZOOM,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = _viv_settings_ctls[_viv_settings_ctl_count-1].value.left + _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH);
			y += row_high;

			_viv_settings_ctl_add(_VIV_SETTINGS_CT_DROPDOWN,_VIV_SETTINGS_ID_AUTOTYPE,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_VALUE_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = content_x + content_wide;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH);
			y += row_high;

			y += _viv_settings_dip(10);

			// animation playback.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_SWITCH,_VIV_SETTINGS_ID_LOOP,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = _viv_settings_ctls[_viv_settings_ctl_count-1].value.left + _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH);
			y += row_high;

			// preload next image.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_SWITCH,_VIV_SETTINGS_ID_PRELOAD,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = _viv_settings_ctls[_viv_settings_ctl_count-1].value.left + _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH);
			y += row_high;

			// cache last image.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_SWITCH,_VIV_SETTINGS_ID_CACHE,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = _viv_settings_ctls[_viv_settings_ctl_count-1].value.left + _viv_settings_dip(_VIV_SETTINGS_SWITCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_SWITCH_HIGH);
			y += row_high;

			y += _viv_settings_dip(10);

			// background colors.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_COLOR,_VIV_SETTINGS_ID_WINCOLOR,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_COLOR_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_SWATCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_COLOR_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_SWATCH_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = content_x + content_wide;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_SWATCH_HIGH);
			y += row_high;

			_viv_settings_ctl_add(_VIV_SETTINGS_CT_COLOR,_VIV_SETTINGS_ID_FSCOLOR,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_COLOR_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_SWATCH_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_COLOR_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_SWATCH_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = content_x + content_wide;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_SWATCH_HIGH);
			y += row_high;

			break;
		}

		case _VIV_SETTINGS_PAGE_CONTROLS:
		{
			// mouse actions.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_DROPDOWN,_VIV_SETTINGS_ID_LEFT,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_VALUE_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = content_x + content_wide;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH);
			y += row_high;

			_viv_settings_ctl_add(_VIV_SETTINGS_CT_DROPDOWN,_VIV_SETTINGS_ID_RIGHT,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_VALUE_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = content_x + content_wide;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH);
			y += row_high;

			_viv_settings_ctl_add(_VIV_SETTINGS_CT_DROPDOWN,_VIV_SETTINGS_ID_WHEEL,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_VALUE_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = content_x + content_wide;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH);
			y += row_high;

			y += _viv_settings_dip(10);

			// keyboard shortcuts: the command picker, the shortcut key
			// picker and the add / edit / remove buttons.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_SECTION,_VIV_SETTINGS_ID_NONE,LOCALIZATION_ID_COMMANDS_STATIC,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_SECTION_HIGH));
			y += _viv_settings_dip(_VIV_SETTINGS_SECTION_HIGH);

			_viv_settings_ctl_add(_VIV_SETTINGS_CT_DROPDOWN,_VIV_SETTINGS_ID_COMMAND,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_VALUE_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = content_x + content_wide;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH);
			y += row_high;

			// the group caption of the old controls page: the shortcut key
			// picker belongs to the selected command.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_SECTION,_VIV_SETTINGS_ID_NONE,LOCALIZATION_ID_SETTINGS_FOR_SELECTED_COMMAND,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_SECTION_HIGH));
			y += _viv_settings_dip(_VIV_SETTINGS_SECTION_HIGH);

			_viv_settings_ctl_add(_VIV_SETTINGS_CT_DROPDOWN,_VIV_SETTINGS_ID_KEYS,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_ROW_HIGH));
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.left = content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_VALUE_WIDE);
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.top = y + ((row_high = _viv_settings_dip(_VIV_SETTINGS_ROW_HIGH)) - _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH)) / 2;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.right = content_x + content_wide;
			_viv_settings_ctls[_viv_settings_ctl_count-1].value.bottom = _viv_settings_ctls[_viv_settings_ctl_count-1].value.top + _viv_settings_dip(_VIV_SETTINGS_VALUE_HIGH);
			y += row_high;

			// add / edit / remove, right aligned.
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_KEYBUTTON,_VIV_SETTINGS_ID_KEY_ADD,0,content_x + content_wide - (_viv_settings_dip(_VIV_SETTINGS_KEYBUTTON_WIDE) * 3 + _viv_settings_dip(8) * 2),y,_viv_settings_dip(_VIV_SETTINGS_KEYBUTTON_WIDE),_viv_settings_dip(_VIV_SETTINGS_KEYBUTTON_HIGH));
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_KEYBUTTON,_VIV_SETTINGS_ID_KEY_EDIT,0,content_x + content_wide - (_viv_settings_dip(_VIV_SETTINGS_KEYBUTTON_WIDE) * 2 + _viv_settings_dip(8)),y,_viv_settings_dip(_VIV_SETTINGS_KEYBUTTON_WIDE),_viv_settings_dip(_VIV_SETTINGS_KEYBUTTON_HIGH));
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_KEYBUTTON,_VIV_SETTINGS_ID_KEY_REMOVE,0,content_x + content_wide - _viv_settings_dip(_VIV_SETTINGS_KEYBUTTON_WIDE),y,_viv_settings_dip(_VIV_SETTINGS_KEYBUTTON_WIDE),_viv_settings_dip(_VIV_SETTINGS_KEYBUTTON_HIGH));

			y += _viv_settings_dip(_VIV_SETTINGS_KEYBUTTON_HIGH) + _viv_settings_dip(6);

			// the key capture hint lines (empty while not capturing).
			// param 0 carries the add / edit hint, param 1 the currently
			// used by line (see the paint pass).
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_DESC,_VIV_SETTINGS_ID_NONE,0,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_DESC_HIGH));
			y += _viv_settings_dip(_VIV_SETTINGS_DESC_HIGH);

			// the "currently used by" line (empty while not capturing).
			_viv_settings_ctl_add(_VIV_SETTINGS_CT_DESC,_VIV_SETTINGS_ID_NONE,1,content_x,y,content_wide,_viv_settings_dip(_VIV_SETTINGS_DESC_HIGH));
			y += _viv_settings_dip(_VIV_SETTINGS_DESC_HIGH);

			break;
		}
	}

	// the footer: cancel and ok, right aligned (the paint pass draws
	// the divider hairline above it).
	footer_y = client.bottom - _viv_settings_dip(_VIV_SETTINGS_FOOTER_HIGH) + (_viv_settings_dip(_VIV_SETTINGS_FOOTER_HIGH) - _viv_settings_dip(_VIV_SETTINGS_BUTTON_HIGH)) / 2;

	_viv_settings_ctl_add(_VIV_SETTINGS_CT_BUTTON,_VIV_SETTINGS_ID_CANCEL,0,client.right - _viv_settings_dip(_VIV_SETTINGS_CONTENT_PAD) - _viv_settings_dip(_VIV_SETTINGS_BUTTON_WIDE) * 2 - _viv_settings_dip(12),footer_y,_viv_settings_dip(_VIV_SETTINGS_BUTTON_WIDE),_viv_settings_dip(_VIV_SETTINGS_BUTTON_HIGH));
	_viv_settings_ctl_add(_VIV_SETTINGS_CT_BUTTON,_VIV_SETTINGS_ID_OK,0,client.right - _viv_settings_dip(_VIV_SETTINGS_CONTENT_PAD) - _viv_settings_dip(_VIV_SETTINGS_BUTTON_WIDE),footer_y,_viv_settings_dip(_VIV_SETTINGS_BUTTON_WIDE),_viv_settings_dip(_VIV_SETTINGS_BUTTON_HIGH));

	// the relayout runs on every page enter: the general page refreshes
	// the run key cache here (its switch paints from the cache).
	if (_viv_settings_page == _VIV_SETTINGS_PAGE_GENERAL)
	{
		_viv_settings_run_key_state = _viv_settings_run_key_present();
	}

	_viv_settings_key_index_clamp();
}

static void _viv_settings_invalidate(void)
{
	if (_viv_settings_hwnd)
	{
		InvalidateRect(_viv_settings_hwnd,0,FALSE);
	}
}

static void _viv_settings_track_leave(HWND hwnd)
{
	TRACKMOUSEEVENT tme;

	os_zero_memory(&tme,sizeof(tme));

	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = hwnd;

	TrackMouseEvent(&tme);

	_viv_settings_tracking = 1;
}

static int _viv_settings_hit_test(int x,int y)
{
	int i;
	POINT pt;

	pt.x = x;
	pt.y = y;

	for(i=0;i<_viv_settings_ctl_count;i++)
	{
		if (!_viv_settings_ctl_enabled(&_viv_settings_ctls[i]))
		{
			continue;
		}

		if (PtInRect(&_viv_settings_ctls[i].rect,pt))
		{
			return i;
		}
	}

	return -1;
}

// focus helpers. the array order is the tab order.
static void _viv_settings_focus_set(int index,int invalidate)
{
	if (_viv_settings_focus != index)
	{
		_viv_settings_focus = index;

		if (invalidate)
		{
			_viv_settings_invalidate();
		}
	}
}

static int _viv_settings_focus_next(int from,int dir)
{
	int i;

	i = from;

	for(;;)
	{
		i += dir;

		if (i < 0)
		{
			i = _viv_settings_ctl_count - 1;
		}

		if (i >= _viv_settings_ctl_count)
		{
			i = 0;
		}

		if (i == from)
		{
			return _viv_settings_ctl_count ? from : -1;
		}

		if (_viv_settings_ctl_enabled(&_viv_settings_ctls[i]))
		{
			return i;
		}
	}
}

static void _viv_settings_focus_cycle(int dir)
{
	int index;

	if (!_viv_settings_ctl_count)
	{
		return;
	}

	index = _viv_settings_focus;

	if ((index < 0) || (index >= _viv_settings_ctl_count))
	{
		index = 0;

		if (!_viv_settings_ctl_enabled(&_viv_settings_ctls[index]))
		{
			index = _viv_settings_focus_next(index,dir);
		}
	}
	else
	{
		index = _viv_settings_focus_next(index,dir);
	}

	// the tab pass moves the focus ring only: a navigation item reached
	// by tab (the wrap from ok) is not a selection - only a click or an
	// arrow walk switches the page.

	_viv_settings_focus_set(index,1);

	_viv_settings_invalidate();
}

// the key list of the current command.
static int _viv_settings_key_count(void)
{
	config_key_t *key;
	int count;

	count = 0;

	key = _viv_settings_keylist.start[_viv_settings_command_index];

	while(key)
	{
		count++;

		key = key->next;
	}

	return count;
}

static DWORD _viv_settings_key_at(int index)
{
	config_key_t *key;
	int i;

	i = 0;

	key = _viv_settings_keylist.start[_viv_settings_command_index];

	while(key)
	{
		if (i == index)
		{
			return key->key;
		}

		i++;

		key = key->next;
	}

	return 0;
}

static void _viv_settings_key_index_clamp(void)
{
	int count;

	count = _viv_settings_key_count();

	if (_viv_settings_key_index >= count)
	{
		_viv_settings_key_index = count - 1;
	}
}

// the command picker: the same filter as the options dialog's command
// list (no popups, no separators, no deleted entries).
static int _viv_settings_command_at(int item_index)
{
	int i;
	int count;

	count = 0;

	for(i=0;i<_VIV_COMMAND_COUNT;i++)
	{
		if (!(_viv_commands[i].flags & MF_POPUP))
		{
			if (!(_viv_commands[i].flags & MF_SEPARATOR))
			{
				if (!(_viv_commands[i].flags & MF_DELETE))
				{
					if (count == item_index)
					{
						return i;
					}

					count++;
				}
			}
		}
	}

	return 0;
}

// the popup item text source.
#define _VIV_SETTINGS_POPUP_IDS		0
#define _VIV_SETTINGS_POPUP_COMMANDS	1
#define _VIV_SETTINGS_POPUP_KEYS	2
// rc.13: the language list mixes sources (a localization string plus two
// self-named languages), so it reads by index.
#define _VIV_SETTINGS_POPUP_LANGUAGE	3

// rc.13: the owner drawn dropdown labels (one pool per open, reclaimed by
// the row pool reset - the same lifetime rule the recent list follows).
#define _VIV_SETTINGS_POPUP_MAX		32
#define _VIV_SETTINGS_POPUP_LABEL_CHARS	96
static wchar_t _viv_settings_popup_labels[_VIV_SETTINGS_POPUP_MAX][_VIV_SETTINGS_POPUP_LABEL_CHARS];

static void _viv_settings_popup_text(int kind,const void *context,int index,wchar_t *wbuf)
{
	wbuf[0] = 0;

	switch(kind)
	{
		case _VIV_SETTINGS_POPUP_IDS:
			string_copy_utf8_string(wbuf,localization_get_string(((const localization_id_t *)context)[index]));
			break;

		case _VIV_SETTINGS_POPUP_COMMANDS:
			_viv_get_command_name(wbuf,_viv_settings_command_at(index));
			break;

		case _VIV_SETTINGS_POPUP_KEYS:
			_viv_get_key_text(wbuf,_viv_settings_key_at(index));
			break;

		// rc.13: 0 = follow system, 1 = english (self named), 2 = chinese
		// (self named). the two self-named entries read through the
		// language table, not the localization string table.
		case _VIV_SETTINGS_POPUP_LANGUAGE:
			switch (index)
			{
				case 0:
					string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_SETTINGS_LANGUAGE_FOLLOW_SYSTEM));
					break;

				case 1:
					string_copy_utf8_string(wbuf,localization_get_language_name(LOCALIZATION_LANGUAGE_ENGLISH));
					break;

				default:
					string_copy_utf8_string(wbuf,localization_get_language_name(LOCALIZATION_LANGUAGE_CHINESE_SIMPLIFIED));
					break;
			}
			break;

		default:
			break;
	}
}

// open one dropdown list. returns the selected item index, -1 when
// cancelled. rc.13: the rows join the owner drawn menu pipeline - the
// native string rows painted the system blue highlight and the native
// checkmark next to the accent pill sidebar (the field's "looks bad on
// non-win11"), and the tpm_nonotify flag kept the shared popup theming
// (the class brush, the dwm chrome) from ever running. one row pool per
// open, radio dots for the current entry, the same painter as the main
// menus.
static int _viv_settings_popup(HWND hwnd,const RECT *anchor,int kind,const void *context,int count,int current)
{
	HMENU menu;
	POINT pt;
	int ret;
	int i;
	wchar_t wbuf[STRING_SIZE];

	menu = CreatePopupMenu();

	if (!menu)
	{
		return -1;
	}

	if (count > _VIV_SETTINGS_POPUP_MAX)
	{
		count = _VIV_SETTINGS_POPUP_MAX;
	}

	_viv_menu_row_pool_reset(_VIV_MENU_POOL_SETTINGS);

	for(i=0;i<count;i++)
	{
		void *row;
		MENUITEMINFOW mii;

		_viv_settings_popup_text(kind,context,i,wbuf);

		string_copy_with_bufsize(_viv_settings_popup_labels[i],_VIV_SETTINGS_POPUP_LABEL_CHARS,wbuf);

		row = _viv_menu_row_alloc(_VIV_MENU_POOL_SETTINGS,_VIV_MENU_DRAW_TEXT,0,0,0);

		if (!row)
		{
			break;
		}

		_viv_menu_row_set_text(row,_viv_settings_popup_labels[i]);

		os_zero_memory(&mii,sizeof(mii));
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STATE | MIIM_DATA;
		mii.wID = (UINT)(i + 1);
		mii.fType = MFT_OWNERDRAW | MFT_RADIOCHECK;
		mii.fState = (i == current) ? MFS_CHECKED : 0;
		mii.dwItemData = (ULONG_PTR)row;

		InsertMenuItemW(menu,(UINT)i,TRUE,&mii);
	}

	pt.x = anchor->left;
	pt.y = anchor->bottom;

	ClientToScreen(hwnd,&pt);

	ret = TrackPopupMenuEx(menu,TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,pt.x,pt.y,hwnd,0);

	DestroyMenu(menu);

	if ((ret <= 0) || (ret > count))
	{
		return -1;
	}

	return ret - 1;
}

// the language apply: the same calls the options dialog ok path runs.
static void _viv_settings_set_language(int language)
{
	config_language = (BYTE)language;

	if (language == 1)
	{
		localization_set_language(LOCALIZATION_LANGUAGE_ENGLISH);
	}
	else
	if (language == 2)
	{
		localization_set_language(LOCALIZATION_LANGUAGE_CHINESE_SIMPLIFIED);
	}
	else
	{
		// auto: follow the system language again.
		localization_init();
	}
}

// refresh every live surface after a language change: the options
// dialog does this on ok (menu rebuild, toolbar texts, tooltips).
static void _viv_settings_language_refresh(void)
{
	HMENU new_hmenu;

	new_hmenu = _viv_create_menu();

	if (_viv_hmenu)
	{
		DestroyMenu(_viv_hmenu);
	}

	_viv_hmenu = new_hmenu;

	// the fresh menu: the top bar re-reads the labels and re-lays them
	// out at the current font.
	_viv_menubar_layout();

	// recreate the toolbar so its texts and tooltips use the new language.
	_viv_controls_show(0);
	_viv_controls_show(config_show_controls);

	// the recreated toolbar has a fresh (light) tooltip control: re-apply
	// the dark chrome to it.
	_viv_apply_dark_mode(0);

	// update the floating zoom control tooltips.
	zoomui_localize();

	// relayout and redraw. (the title bar and status bar update here too)
	_viv_on_size();

	// this window's own native caption was set at open: re-localize it.
	os_SetWindowText_localization_id(_viv_settings_hwnd,LOCALIZATION_ID_SETTINGS);

	InvalidateRect(_viv_hwnd,0,FALSE);
}

// the theme apply: the options dialog's exact sequence.
static void _viv_settings_dark_apply(void)
{
	os_dark_set_app_mode(config_dark_mode);

	_viv_apply_dark_mode(1);

	// the app mode switch flushes the menu themes and the color policy
	// asynchronously: the one shot assert re-applies once it settled.
	SetTimer(_viv_hwnd,VIV_ID_DARK_RECHECK_TIMER,400,0);

	_viv_settings_theme_update();
}

// the accent apply: the shared token table flips at once; the main
// window and this window repaint from it.
static void _viv_settings_accent_set(int accent)
{
	if (accent == viv_theme_accent())
	{
		return;
	}

	viv_theme_set_accent(accent);

	if (_viv_hwnd)
	{
		InvalidateRect(_viv_hwnd,0,FALSE);
	}

	_viv_settings_theme_update();
}

// which swatch of the accent row sits at x (-1: the gaps).
static int _viv_settings_swatch_from_x(const _viv_settings_ctl_t *ctl,int x)
{
	int step;
	int i;

	if ((x < ctl->value.left) || (x >= ctl->value.right))
	{
		return -1;
	}

	step = _viv_settings_dip(_VIV_SETTINGS_ACCENT_D) + _viv_settings_dip(_VIV_SETTINGS_ACCENT_GAP);

	i = (x - ctl->value.left) / step;

	if (i >= VIV_THEME_ACCENT_COUNT)
	{
		return -1;
	}

	if (((x - ctl->value.left) - (i * step)) > _viv_settings_dip(_VIV_SETTINGS_ACCENT_D))
	{
		return -1;
	}

	return i;
}

// re-assert the settings window own chrome (caption color + dark mode).
static void _viv_settings_theme_update(void)
{
	if (!_viv_settings_hwnd)
	{
		return;
	}

	// the shared token brush cache rebuilds here: a theme or an accent
	// flip re-skins every viv_theme painter with it.
	viv_theme_refresh();

	os_dark_titlebar(_viv_settings_hwnd,_viv_is_dark());

	// windows 11 chrome: rounded corners. the popup carries no native
	// caption (the title row is painted); a silent no-op on windows 10.
	os_window_modern_chrome(_viv_settings_hwnd,_viv_settings_color(_VIV_SETTINGS_C_FACE));

	InvalidateRect(_viv_settings_hwnd,0,FALSE);
}

// the open snapshot: everything the window can touch.
static void _viv_settings_snapshot(void)
{
	int i;

	_viv_settings_snap_language = config_language;
	_viv_settings_snap_dark_mode = config_dark_mode;
	_viv_settings_snap_accent = viv_theme_accent();
	_viv_settings_snap_multiple_instances = config_multiple_instances;

	// the two admin rows: the pending state starts at the live state
	// (the ok path compares against the live state again).
	_viv_settings_appdata = config_appdata ? 1 : 0;
	_viv_settings_startmenu = _viv_is_start_menu_shortcuts() ? 1 : 0;
	_viv_settings_snap_run_key = _viv_settings_run_key_present();

	// the cache starts at the live registry state.
	_viv_settings_run_key_state = _viv_settings_snap_run_key;
	_viv_settings_snap_shrink_blit = config_shrink_blit_mode;
	_viv_settings_snap_mag = config_mag_filter;
	_viv_settings_snap_title = config_title_bar_format;
	_viv_settings_snap_auto_zoom = config_auto_zoom;
	_viv_settings_snap_auto_type = config_auto_zoom_type;
	_viv_settings_snap_loop = config_loop_animations_once;
	_viv_settings_snap_preload = config_preload_next;
	_viv_settings_snap_cache = config_cache_last;
	_viv_settings_snap_left = config_left_click_action;
	_viv_settings_snap_right = config_right_click_action;
	_viv_settings_snap_wheel = config_mouse_wheel_action;
	_viv_settings_snap_windowed_r = config_windowed_background_color_r;
	_viv_settings_snap_windowed_g = config_windowed_background_color_g;
	_viv_settings_snap_windowed_b = config_windowed_background_color_b;
	_viv_settings_snap_fullscreen_r = config_fullscreen_background_color_r;
	_viv_settings_snap_fullscreen_g = config_fullscreen_background_color_g;
	_viv_settings_snap_fullscreen_b = config_fullscreen_background_color_b;

	for(i=0;i<_VIV_ASSOCIATION_COUNT;i++)
	{
		_viv_settings_snap_assoc[i] = _viv_is_association(_viv_association_extensions[i]) ? 1 : 0;
		_viv_settings_assoc[i] = _viv_settings_snap_assoc[i];
	}

	// the key list copy for the shortcut editor.
	_viv_key_list_init(&_viv_settings_keylist);
	_viv_key_list_copy(&_viv_settings_keylist,_viv_key_list);

	_viv_settings_command_index = _viv_settings_command_at(0);
	_viv_settings_command_item_count = 0;

	for(i=0;i<_VIV_COMMAND_COUNT;i++)
	{
		if (!(_viv_commands[i].flags & MF_POPUP))
		{
			if (!(_viv_commands[i].flags & MF_SEPARATOR))
			{
				if (!(_viv_commands[i].flags & MF_DELETE))
				{
					_viv_settings_command_item_count++;
				}
			}
		}
	}

	_viv_settings_key_index = -1;
	_viv_settings_capture_active = 0;
	_viv_settings_capture_edit = 0;
	_viv_settings_capture_key = 0;
}

// cancel: rewind every config field and reverse the live effects.
static void _viv_settings_restore(void)
{
	int language_changed;
	int dark_changed;
	int i;

	language_changed = 0;
	dark_changed = 0;

	// language.
	if (config_language != _viv_settings_snap_language)
	{
		_viv_settings_set_language(_viv_settings_snap_language);

		language_changed = 1;
	}

	// dark mode.
	if (config_dark_mode != _viv_settings_snap_dark_mode)
	{
		config_dark_mode = (BYTE)_viv_settings_snap_dark_mode;

		dark_changed = 1;
	}

	if (config_multiple_instances != _viv_settings_snap_multiple_instances)
	{
		config_multiple_instances = _viv_settings_snap_multiple_instances ? 1 : 0;
	}

	// startup run key.
	if ((_viv_settings_run_key_present() ? 1 : 0) != _viv_settings_snap_run_key)
	{
		_viv_settings_run_key_set(_viv_settings_snap_run_key);
	}

	// the cache follows the rewind.
	_viv_settings_run_key_state = _viv_settings_snap_run_key;

	// file associations.
	for(i=0;i<_VIV_ASSOCIATION_COUNT;i++)
	{
		if ((_viv_is_association(_viv_association_extensions[i]) ? 1 : 0) != _viv_settings_snap_assoc[i])
		{
			if (_viv_settings_snap_assoc[i])
			{
				_viv_install_association_by_extension(_viv_association_extensions[i],localization_get_string(_viv_association_description_localization_id_array[i]),_viv_association_icon_locations[i]);
			}
			else
			{
				_viv_uninstall_association_by_extension(_viv_association_extensions[i]);
			}

			_viv_settings_assoc[i] = _viv_settings_snap_assoc[i];
		}
	}

	// blit modes: a change repaints the canvas.
	if (config_shrink_blit_mode != (BYTE)_viv_settings_snap_shrink_blit)
	{
		config_shrink_blit_mode = (BYTE)_viv_settings_snap_shrink_blit;

		InvalidateRect(_viv_hwnd,0,FALSE);
	}

	if (config_mag_filter != (BYTE)_viv_settings_snap_mag)
	{
		config_mag_filter = (BYTE)_viv_settings_snap_mag;

		InvalidateRect(_viv_hwnd,0,FALSE);
	}

	// title bar format.
	if (config_title_bar_format != (BYTE)_viv_settings_snap_title)
	{
		config_title_bar_format = (BYTE)_viv_settings_snap_title;

		_viv_update_title();
	}

	if (config_auto_zoom != (BYTE)_viv_settings_snap_auto_zoom)
	{
		config_auto_zoom = (BYTE)_viv_settings_snap_auto_zoom;
	}

	if (config_auto_zoom_type != (BYTE)_viv_settings_snap_auto_type)
	{
		config_auto_zoom_type = (BYTE)_viv_settings_snap_auto_type;
	}

	if (config_loop_animations_once != (BYTE)_viv_settings_snap_loop)
	{
		config_loop_animations_once = (BYTE)_viv_settings_snap_loop;
	}

	if (config_preload_next != (BYTE)_viv_settings_snap_preload)
	{
		config_preload_next = (BYTE)_viv_settings_snap_preload;
	}

	if (config_cache_last != (BYTE)_viv_settings_snap_cache)
	{
		config_cache_last = (BYTE)_viv_settings_snap_cache;
	}

	// windowed background color: re-tint the frame and reload.
	if ((config_windowed_background_color_r != _viv_settings_snap_windowed_r) ||
		(config_windowed_background_color_g != _viv_settings_snap_windowed_g) ||
		(config_windowed_background_color_b != _viv_settings_snap_windowed_b))
	{
		config_windowed_background_color_r = _viv_settings_snap_windowed_r;
		config_windowed_background_color_g = _viv_settings_snap_windowed_g;
		config_windowed_background_color_b = _viv_settings_snap_windowed_b;

		os_window_modern_chrome(_viv_hwnd,_viv_windowed_background());

		InvalidateRect(_viv_hwnd,0,FALSE);
		_viv_refresh();
	}

	// fullscreen background color.
	if ((config_fullscreen_background_color_r != _viv_settings_snap_fullscreen_r) ||
		(config_fullscreen_background_color_g != _viv_settings_snap_fullscreen_g) ||
		(config_fullscreen_background_color_b != _viv_settings_snap_fullscreen_b))
	{
		config_fullscreen_background_color_r = _viv_settings_snap_fullscreen_r;
		config_fullscreen_background_color_g = _viv_settings_snap_fullscreen_g;
		config_fullscreen_background_color_b = _viv_settings_snap_fullscreen_b;

		InvalidateRect(_viv_hwnd,0,FALSE);
	}

	// mouse actions.
	if (config_left_click_action != (BYTE)_viv_settings_snap_left)
	{
		config_left_click_action = (BYTE)_viv_settings_snap_left;
	}

	if (config_right_click_action != (BYTE)_viv_settings_snap_right)
	{
		config_right_click_action = (BYTE)_viv_settings_snap_right;
	}

	if (config_mouse_wheel_action != (BYTE)_viv_settings_snap_wheel)
	{
		config_mouse_wheel_action = (BYTE)_viv_settings_snap_wheel;
	}

	// the key list working copy is dropped: the real list was never
	// touched before ok.

	// the main window follows the rewound language and theme.
	if (dark_changed)
	{
		_viv_settings_dark_apply();
	}

	if (language_changed)
	{
		_viv_settings_language_refresh();
	}

	// accent color (applied live; snap it back on cancel).
	if (viv_theme_accent() != _viv_settings_snap_accent)
	{
		_viv_settings_accent_set(_viv_settings_snap_accent);
	}
}

// ok: everything already applied live. hand the edited key list to the
// real one, rebuild the menu, remember the last page and save. the two
// admin rows (appdata storage, start menu shortcuts) ride along as
// command line params for the elevated helper instance (the options
// dialog behavior: the helper does the work and exits).
static void _viv_settings_ok(void)
{
	HMENU new_hmenu;
	wchar_t params[STRING_SIZE];
	wchar_t exe_filename[STRING_SIZE];

	params[0] = 0;

	// the appdata storage switch.
	if ((config_appdata ? 1 : 0) != _viv_settings_appdata)
	{
		config_appdata = _viv_settings_appdata ? 1 : 0;

		_viv_append_admin_param(params,_viv_settings_appdata ? (const utf8_t *)"appdata" : (const utf8_t *)"noappdata");
	}

	// the start menu shortcuts switch.
	if ((_viv_is_start_menu_shortcuts() ? 1 : 0) != _viv_settings_startmenu)
	{
		_viv_append_admin_param(params,_viv_settings_startmenu ? (const utf8_t *)"startmenu" : (const utf8_t *)"nostartmenu");
	}

	_viv_key_list_copy(_viv_key_list,&_viv_settings_keylist);

	new_hmenu = _viv_create_menu();

	if (_viv_hmenu)
	{
		DestroyMenu(_viv_hmenu);
	}

	_viv_hmenu = new_hmenu;

	_viv_menubar_layout();

	config_options_last_page = (BYTE)_viv_settings_page;

	// run the queued admin work (the helper instance elevates itself,
	// does the change and exits), then save with the new location.
	if (params[0])
	{
		_viv_get_exe_filename(exe_filename);

		os_shell_execute(0,exe_filename,1,NULL,params);
	}

	config_save_settings(config_appdata);

	_viv_settings_close();
}

static void _viv_settings_cancel(void)
{
	_viv_settings_restore();

	_viv_settings_close();
}

static void _viv_settings_close(void)
{
	if (_viv_settings_hwnd)
	{
		DestroyWindow(_viv_settings_hwnd);
	}
}

// the key capture mode: the inline replacement for the edit key dialog.
static void _viv_settings_capture_begin(int edit)
{
	_viv_settings_capture_edit = edit;
	_viv_settings_capture_key = 0;

	if ((edit) && (_viv_settings_key_index >= 0))
	{
		// the edit key dialog pre-fills the field with the current key.
		_viv_settings_capture_key = _viv_settings_key_at(_viv_settings_key_index);
	}

	_viv_settings_capture_active = 1;

	_viv_settings_invalidate();
}

static void _viv_settings_capture_end(void)
{
	_viv_settings_capture_active = 0;
	_viv_settings_capture_key = 0;

	_viv_settings_invalidate();
}

static void _viv_settings_capture_commit(void)
{
	config_key_t *key;
	int i;
	int old_key;

	if (!_viv_settings_capture_active)
	{
		return;
	}

	old_key = (int)_viv_settings_key_at(_viv_settings_key_index);

	if (_viv_settings_capture_key)
	{
		// the dialog removes the new key from every command first
		// (_viv_edit_key_remove_currently_used_by) - unless a confirmed
		// edit kept the key: a no-op, the sweep would drop the binding
		// and nothing re-adds it (a different key still steals below).
		if ((!_viv_settings_capture_edit) || (old_key != (int)_viv_settings_capture_key))
		{
			for(i=0;i<_VIV_COMMAND_COUNT;i++)
			{
				_viv_key_remove(&_viv_settings_keylist,i,_viv_settings_capture_key);
			}
		}

		if (_viv_settings_capture_edit)
		{
			if (old_key != (int)_viv_settings_capture_key)
			{
				// edit: rewrite the selected entry in place.
				key = _viv_settings_keylist.start[_viv_settings_command_index];
				i = 0;

				while(key)
				{
					if (i == _viv_settings_key_index)
					{
						key->key = (WORD)_viv_settings_capture_key;

						break;
					}

					i++;

					key = key->next;
				}

				if (!key)
				{
					// the selected entry was the removed one: re-add.
					_viv_key_add(&_viv_settings_keylist,_viv_settings_command_index,_viv_settings_capture_key);
				}
			}
		}
		else
		{
			// add.
			_viv_key_add(&_viv_settings_keylist,_viv_settings_command_index,_viv_settings_capture_key);
		}
	}

	_viv_settings_capture_end();

	_viv_settings_key_index_clamp();

	_viv_settings_invalidate();
}

// a raw key arrives while capturing (the edit proc of the old dialog,
// minus the modifier only keys).
static int _viv_settings_capture_key_down(WPARAM vk)
{
	int key_flags;

	switch((int)vk)
	{
		case VK_CONTROL:
		case VK_SHIFT:
		case VK_MENU:
		case VK_LWIN:
		case VK_RWIN:
			return 1;

		default:
			break;
	}

	key_flags = _viv_get_current_key_mod_flags();

	_viv_settings_capture_key = (DWORD)(key_flags | (int)vk);

	_viv_settings_invalidate();

	return 1;
}

// run one dropdown row.
static void _viv_settings_run_dropdown(HWND hwnd,const _viv_settings_ctl_t *ctl)
{
	const _viv_settings_ctl_t *drop_ctl;

	drop_ctl = ctl;

	switch(drop_ctl->id)
	{
		case _VIV_SETTINGS_ID_LANGUAGE:
		{
			int selected;

			// rc.13: the inline native menu collapsed into the shared
			// owner drawn popup (one code path for every dropdown; the
			// language list reads its entries by index).
			selected = _viv_settings_popup(hwnd,&drop_ctl->value,_VIV_SETTINGS_POPUP_LANGUAGE,0,3,config_language);

			if ((selected >= 0) && (selected != (int)config_language))
			{
				_viv_settings_set_language(selected);

				_viv_settings_language_refresh();

				_viv_settings_invalidate();
			}

			break;
		}

		case _VIV_SETTINGS_ID_THEME:
		{
			// list order: automatic, light, dark (the design sheet and
			// the old dialog). config: 0 light, 1 dark, 2 auto.
			static const localization_id_t ids[3] = {LOCALIZATION_ID_DARK_MODE_AUTO,LOCALIZATION_ID_DARK_MODE_LIGHT,LOCALIZATION_ID_DARK_MODE_DARK};
			int selected;
			int mode;

			mode = (config_dark_mode == 2) ? 0 : (config_dark_mode == 0 ? 1 : 2);

			selected = _viv_settings_popup(hwnd,&drop_ctl->value,_VIV_SETTINGS_POPUP_IDS,ids,3,mode);

			if ((selected >= 0) && (selected != mode))
			{
				config_dark_mode = (BYTE)((selected == 0) ? 2 : ((selected == 1) ? 0 : 1));

				_viv_settings_dark_apply();

				_viv_settings_invalidate();
			}

			break;
		}

		case _VIV_SETTINGS_ID_SHRINK:
		{
			static const localization_id_t ids[2] = {LOCALIZATION_ID_BLIT_MODE_NEAREST_COMBOBOXITEM,LOCALIZATION_ID_BLIT_MODE_LINEAR_COMBOBOXITEM};
			int selected;
			int mode;

			mode = (config_shrink_blit_mode == CONFIG_SHRINK_BLIT_MODE_HALFTONE) ? 1 : 0;

			selected = _viv_settings_popup(hwnd,&drop_ctl->value,_VIV_SETTINGS_POPUP_IDS,ids,2,mode);

			if ((selected >= 0) && (selected != mode))
			{
				config_shrink_blit_mode = selected ? CONFIG_SHRINK_BLIT_MODE_HALFTONE : CONFIG_SHRINK_BLIT_MODE_COLORONCOLOR;

				InvalidateRect(_viv_hwnd,0,FALSE);

				_viv_settings_invalidate();
			}

			break;
		}

		case _VIV_SETTINGS_ID_MAG:
		{
			static const localization_id_t ids[2] = {LOCALIZATION_ID_BLIT_MODE_NEAREST_COMBOBOXITEM,LOCALIZATION_ID_BLIT_MODE_LINEAR_COMBOBOXITEM};
			int selected;
			int mode;

			mode = (config_mag_filter == CONFIG_MAG_FILTER_HALFTONE) ? 1 : 0;

			selected = _viv_settings_popup(hwnd,&drop_ctl->value,_VIV_SETTINGS_POPUP_IDS,ids,2,mode);

			if ((selected >= 0) && (selected != mode))
			{
				config_mag_filter = selected ? CONFIG_MAG_FILTER_HALFTONE : CONFIG_MAG_FILTER_COLORONCOLOR;

				InvalidateRect(_viv_hwnd,0,FALSE);

				_viv_settings_invalidate();
			}

			break;
		}

		case _VIV_SETTINGS_ID_TITLE:
		{
			static const localization_id_t ids[3] = {LOCALIZATION_ID_OPTIONS_TITLE_BAR_FORMAT_FULL_PATH_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_TITLE_BAR_FORMAT_FILENAME_ONLY_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_TITLE_BAR_FORMAT_NONE_COMBOBOXITEM};
			int selected;

			selected = _viv_settings_popup(hwnd,&drop_ctl->value,_VIV_SETTINGS_POPUP_IDS,ids,3,config_title_bar_format);

			if ((selected >= 0) && (selected != (int)config_title_bar_format))
			{
				config_title_bar_format = (BYTE)selected;

				_viv_update_title();

				_viv_settings_invalidate();
			}

			break;
		}

		case _VIV_SETTINGS_ID_AUTOTYPE:
		{
			static const localization_id_t ids[4] = {LOCALIZATION_ID_OPTIONS_VIEW_AUTO_SIZE_WINDOW_50_PERCENT_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_VIEW_AUTO_SIZE_WINDOW_100_PERCENT_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_VIEW_AUTO_SIZE_WINDOW_200_PERCENT_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_VIEW_AUTO_SIZE_WINDOW_AUTO_FIT_COMBOBOXITEM};
			int selected;

			selected = _viv_settings_popup(hwnd,&drop_ctl->value,_VIV_SETTINGS_POPUP_IDS,ids,4,config_auto_zoom_type);

			if ((selected >= 0) && (selected != (int)config_auto_zoom_type))
			{
				config_auto_zoom_type = (BYTE)selected;

				_viv_settings_invalidate();
			}

			break;
		}

		case _VIV_SETTINGS_ID_LEFT:
		{
			static const localization_id_t ids[7] = {LOCALIZATION_ID_OPTIONS_ACTION_SCROLL_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_PLAY_PAUSE_SLIDESHOW_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_PLAY_PAUSE_ANIMATION_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_ZOOM_IN_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_NEXT_IMAGE_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_ONE_TO_ONE_SCROLL_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_SCROLL_MOVE_WINDOW_COMBOBOXITEM};
			int selected;

			selected = _viv_settings_popup(hwnd,&drop_ctl->value,_VIV_SETTINGS_POPUP_IDS,ids,7,config_left_click_action);

			if ((selected >= 0) && (selected != (int)config_left_click_action))
			{
				config_left_click_action = (BYTE)selected;

				_viv_settings_invalidate();
			}

			break;
		}

		case _VIV_SETTINGS_ID_RIGHT:
		{
			static const localization_id_t ids[3] = {LOCALIZATION_ID_OPTIONS_ACTION_CONTEXT_MENU_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_ZOOM_OUT_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_PREVIOUS_IMAGE_COMBOBOXITEM};
			int selected;

			selected = _viv_settings_popup(hwnd,&drop_ctl->value,_VIV_SETTINGS_POPUP_IDS,ids,3,config_right_click_action);

			if ((selected >= 0) && (selected != (int)config_right_click_action))
			{
				config_right_click_action = (BYTE)selected;

				_viv_settings_invalidate();
			}

			break;
		}

		case _VIV_SETTINGS_ID_WHEEL:
		{
			static const localization_id_t ids[3] = {LOCALIZATION_ID_OPTIONS_ACTION_ZOOM_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_NEXT_PREV_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_PREV_NEXT_COMBOBOXITEM};
			int selected;

			selected = _viv_settings_popup(hwnd,&drop_ctl->value,_VIV_SETTINGS_POPUP_IDS,ids,3,config_mouse_wheel_action);

			if ((selected >= 0) && (selected != (int)config_mouse_wheel_action))
			{
				config_mouse_wheel_action = (BYTE)selected;

				_viv_settings_invalidate();
			}

			break;
		}

		case _VIV_SETTINGS_ID_COMMAND:
		{
			int selected;

			selected = _viv_settings_popup(hwnd,&drop_ctl->value,_VIV_SETTINGS_POPUP_COMMANDS,0,_viv_settings_command_item_count,-1);

			if (selected >= 0)
			{
				_viv_settings_command_index = _viv_settings_command_at(selected);
				_viv_settings_key_index = -1;

				_viv_settings_invalidate();
			}

			break;
		}

		case _VIV_SETTINGS_ID_KEYS:
		{
			int selected;

			if (!_viv_settings_key_count())
			{
				break;
			}

			selected = _viv_settings_popup(hwnd,&drop_ctl->value,_VIV_SETTINGS_POPUP_KEYS,0,_viv_settings_key_count(),_viv_settings_key_index);

			if (selected >= 0)
			{
				_viv_settings_key_index = selected;

				_viv_settings_invalidate();
			}

			break;
				}

		default:
			break;
	}
}

// activate one control (mouse click release or space / fire).
static void _viv_settings_activate(int index,int x,int y)
{
	_viv_settings_ctl_t *ctl;

	if ((index < 0) || (index >= _viv_settings_ctl_count))
	{
		return;
	}

	ctl = &_viv_settings_ctls[index];

	// disabled rows (the auto size type while auto size is off, the edit
	// and remove buttons without a shortcut) never fire.
	if (!_viv_settings_ctl_enabled(ctl))
	{
		return;
	}

	switch(ctl->type)
	{
		case _VIV_SETTINGS_CT_NAV:

			if (_viv_settings_page != ctl->param)
			{
				_viv_settings_page = ctl->param;

				_viv_settings_layout();

				_viv_settings_invalidate();
			}

			break;

		case _VIV_SETTINGS_CT_DROPDOWN:
			_viv_settings_run_dropdown(_viv_settings_hwnd,ctl);
			break;

		case _VIV_SETTINGS_CT_SWITCH:

			switch(ctl->id)
			{
				case _VIV_SETTINGS_ID_MULTIPLE:
					config_multiple_instances = config_multiple_instances ? 0 : 1;
					break;

				case _VIV_SETTINGS_ID_RUNKEY:

					// the registry is the source of truth: write, then
					// read the answer back into the cache.
					_viv_settings_run_key_set(_viv_settings_run_key_state ? 0 : 1);

					_viv_settings_run_key_state = _viv_settings_run_key_present();
					break;

				case _VIV_SETTINGS_ID_STARTMENU:
					_viv_settings_startmenu = _viv_settings_startmenu ? 0 : 1;
					break;

				case _VIV_SETTINGS_ID_APPDATA:
					_viv_settings_appdata = _viv_settings_appdata ? 0 : 1;
					break;

				case _VIV_SETTINGS_ID_AUTOZOOM:
					config_auto_zoom = config_auto_zoom ? 0 : 1;
					break;

				case _VIV_SETTINGS_ID_LOOP:
					config_loop_animations_once = config_loop_animations_once ? 0 : 1;
					break;

				case _VIV_SETTINGS_ID_PRELOAD:
					config_preload_next = config_preload_next ? 0 : 1;
					break;

				case _VIV_SETTINGS_ID_CACHE:
					config_cache_last = config_cache_last ? 0 : 1;
					break;

				default:
					break;
			}

			_viv_settings_invalidate();
			break;

		case _VIV_SETTINGS_CT_CHECK:
		{
			int exti;
			int target;

			if (ctl->id == _VIV_SETTINGS_ID_SELECT_ALL)
			{
				// all checked? then the select all clears, else it sets.
				target = 1;

				for(exti=0;exti<_VIV_ASSOCIATION_COUNT;exti++)
				{
					if (!_viv_settings_assoc[exti])
					{
						break;
					}
				}

				if (exti == _VIV_ASSOCIATION_COUNT)
				{
					target = 0;
				}

				for(exti=0;exti<_VIV_ASSOCIATION_COUNT;exti++)
				{
					if (_viv_settings_assoc[exti] != target)
					{
						if (target)
						{
							if (!_viv_is_association(_viv_association_extensions[exti]))
							{
								_viv_install_association_by_extension(_viv_association_extensions[exti],localization_get_string(_viv_association_description_localization_id_array[exti]),_viv_association_icon_locations[exti]);
							}
						}
						else
						{
							if (_viv_is_association(_viv_association_extensions[exti]))
							{
								_viv_uninstall_association_by_extension(_viv_association_extensions[exti]);
							}
						}

						_viv_settings_assoc[exti] = _viv_is_association(_viv_association_extensions[exti]) ? 1 : 0;
					}
				}
			}
			else
			{
				exti = ctl->param;

				target = _viv_settings_assoc[exti] ? 0 : 1;

				if (target)
				{
					if (!_viv_is_association(_viv_association_extensions[exti]))
					{
						_viv_install_association_by_extension(_viv_association_extensions[exti],localization_get_string(_viv_association_description_localization_id_array[exti]),_viv_association_icon_locations[exti]);
					}
				}
				else
				{
					if (_viv_is_association(_viv_association_extensions[exti]))
					{
						_viv_uninstall_association_by_extension(_viv_association_extensions[exti]);
					}
				}

				_viv_settings_assoc[exti] = _viv_is_association(_viv_association_extensions[exti]) ? 1 : 0;
			}

			_viv_settings_invalidate();
			break;
		}

		case _VIV_SETTINGS_CT_COLOR:
		{
			COLORREF colorref;

			if (ctl->id == _VIV_SETTINGS_ID_WINCOLOR)
			{
				colorref = RGB(config_windowed_background_color_r,config_windowed_background_color_g,config_windowed_background_color_b);

				if (os_choose_color(_viv_settings_hwnd,&colorref))
				{
					config_windowed_background_color_r = (BYTE)GetRValue(colorref);
					config_windowed_background_color_g = (BYTE)GetGValue(colorref);
					config_windowed_background_color_b = (BYTE)GetBValue(colorref);

					// the mat, the win11 caption tint and any follow mode
					// backdrop all read this color: re-tint the frame,
					// repaint the canvas and reload the image.
					os_window_modern_chrome(_viv_hwnd,_viv_windowed_background());

					InvalidateRect(_viv_hwnd,0,FALSE);
					_viv_refresh();

					_viv_settings_invalidate();
				}
			}
			else
			{
				colorref = RGB(config_fullscreen_background_color_r,config_fullscreen_background_color_g,config_fullscreen_background_color_b);

				if (os_choose_color(_viv_settings_hwnd,&colorref))
				{
					config_fullscreen_background_color_r = (BYTE)GetRValue(colorref);
					config_fullscreen_background_color_g = (BYTE)GetGValue(colorref);
					config_fullscreen_background_color_b = (BYTE)GetBValue(colorref);

					InvalidateRect(_viv_hwnd,0,FALSE);

					_viv_settings_invalidate();
				}
			}

			break;
		}

		case _VIV_SETTINGS_CT_BUTTON:

			if (ctl->id == _VIV_SETTINGS_ID_OK)
			{
				_viv_settings_ok();
			}
			else
			{
				_viv_settings_cancel();
			}

			break;

		case _VIV_SETTINGS_CT_ACCENT:
		{
			int swatch;

			// a mouse release maps x to the swatch under it (a gap is
			// -1 and does nothing); a keyboard fire (x < 0) cycles to
			// the next accent.
			swatch = (x < 0) ? -1 : _viv_settings_swatch_from_x(ctl,x);

			if ((swatch < 0) && (x < 0))
			{
				swatch = (viv_theme_accent() + 1) % VIV_THEME_ACCENT_COUNT;
			}

			if (swatch >= 0)
			{
				_viv_settings_accent_set(swatch);
			}

			break;
		}

		case _VIV_SETTINGS_CT_KEYBUTTON:

			switch(ctl->id)
			{
				case _VIV_SETTINGS_ID_KEY_ADD:
					_viv_settings_capture_begin(0);
					break;

				case _VIV_SETTINGS_ID_KEY_EDIT:
					_viv_settings_capture_begin(1);
					break;

				case _VIV_SETTINGS_ID_KEY_REMOVE:

					if (_viv_settings_key_index >= 0)
					{
						_viv_key_remove(&_viv_settings_keylist,_viv_settings_command_index,_viv_settings_key_at(_viv_settings_key_index));

						_viv_settings_key_index_clamp();

						_viv_settings_invalidate();
					}

					break;

					default:
						break;
			}

			break;

		default:
			break;
	}
}

// ---- painting ----

static void _viv_settings_fill_round(HDC hdc,const RECT *rect,int radius,COLORREF fill,COLORREF line)
{
	HBRUSH brush;
	HPEN pen;
	HPEN old_pen;
	HGDIOBJ old_brush;

	brush = CreateSolidBrush(fill);
	pen = CreatePen(PS_SOLID,1,line);

	old_pen = (HPEN)SelectObject(hdc,pen);
	old_brush = SelectObject(hdc,brush);

	RoundRect(hdc,rect->left,rect->top,rect->right,rect->bottom,radius * 2,radius * 2);

	SelectObject(hdc,old_brush);
	SelectObject(hdc,old_pen);

	DeleteObject(pen);
	DeleteObject(brush);
}

static void _viv_settings_draw_text_raw(HDC hdc,const RECT *rect,const wchar_t *text,HFONT font,COLORREF color,int flags)
{
	HFONT old_font;

	old_font = (HFONT)SelectObject(hdc,font);

	SetBkMode(hdc,TRANSPARENT);
	SetTextColor(hdc,color);

	DrawTextW(hdc,text,-1,(RECT *)rect,DT_SINGLELINE | DT_HIDEPREFIX | flags);

	SelectObject(hdc,old_font);
}

static void _viv_settings_draw_label(HDC hdc,const RECT *rect,int localization_id,HFONT font,COLORREF color)
{
	wchar_t wbuf[STRING_SIZE];

	string_copy_utf8_string(wbuf,localization_get_string((localization_id_t)localization_id));

	_viv_settings_draw_text_raw(hdc,rect,wbuf,font,color,DT_VCENTER | DT_END_ELLIPSIS);
}

static void _viv_settings_draw_nav(HDC hdc,const _viv_settings_ctl_t *ctl,int selected,int hot,int inactive)
{
	static const int glyph_ids[_VIV_OPTIONS_PAGE_COUNT] = {GLYPH_SETTINGS,GLYPH_PICTURE,GLYPH_GAMEPAD};
	static const int page_ids[_VIV_OPTIONS_PAGE_COUNT] = {LOCALIZATION_ID_SETTINGS_PAGE_GENERAL,LOCALIZATION_ID_SETTINGS_PAGE_VIEW,LOCALIZATION_ID_SETTINGS_PAGE_CONTROLS};
	RECT rect;
	RECT text_rect;
	COLORREF text_color;
	HICON icon;
	int size;

	CopyRect(&rect,&ctl->rect);

	if (selected)
	{
		_viv_settings_fill_round(hdc,&rect,_viv_settings_dip(6),_viv_settings_color(_VIV_SETTINGS_C_ACCENT),_viv_settings_color(_VIV_SETTINGS_C_ACCENT));

		text_color = _viv_settings_color(_VIV_SETTINGS_C_ON_ACCENT);
	}
	else
	{
		if (hot)
		{
			_viv_settings_fill_round(hdc,&rect,_viv_settings_dip(6),_viv_settings_color(_VIV_SETTINGS_C_NAV_HOVER),_viv_settings_color(_VIV_SETTINGS_C_NAV_HOVER));
		}

		text_color = _viv_settings_color(inactive ? _VIV_SETTINGS_C_TEXT2 : _VIV_SETTINGS_C_TEXT);
	}

	// the glyph: 16 dip box, vertically centered.
	size = _viv_settings_dip(16);

	icon = glyphs_icon(glyph_ids[ctl->param],_viv_is_dark(),size);

	if (icon)
	{
		DrawIconEx(hdc,rect.left + _viv_settings_dip(10),rect.top + ((rect.bottom - rect.top) - size) / 2,icon,size,size,0,NULL,DI_NORMAL);
	}

	text_rect.left = rect.left + _viv_settings_dip(10) + size + _viv_settings_dip(10);
	text_rect.top = rect.top;
	text_rect.right = rect.right - _viv_settings_dip(6);
	text_rect.bottom = rect.bottom;

	_viv_settings_draw_label(hdc,&text_rect,page_ids[ctl->param],_viv_settings_font,text_color);
}

static void _viv_settings_draw_dropdown(HDC hdc,const _viv_settings_ctl_t *ctl,int hot,int focus)
{
	wchar_t wbuf[STRING_SIZE];
	RECT text_rect;
	COLORREF line;
	COLORREF text_color;

	line = _viv_settings_color(hot ? _VIV_SETTINGS_C_ACCENT_HOT : _VIV_SETTINGS_C_LINE);

	_viv_settings_fill_round(hdc,&ctl->value,_viv_settings_dip(4),_viv_settings_color(_VIV_SETTINGS_C_INPUT),line);

	if (focus)
	{
		_viv_settings_draw_focus_ring(hdc,&ctl->value);
	}

	// the value text.
	wbuf[0] = 0;

	switch(ctl->id)
	{
		case _VIV_SETTINGS_ID_LANGUAGE:

			if (config_language == 1)
			{
				string_copy_utf8_string(wbuf,localization_get_language_name(LOCALIZATION_LANGUAGE_ENGLISH));
			}
			else
			if (config_language == 2)
			{
				string_copy_utf8_string(wbuf,localization_get_language_name(LOCALIZATION_LANGUAGE_CHINESE_SIMPLIFIED));
			}
			else
			{
				string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_SETTINGS_LANGUAGE_FOLLOW_SYSTEM));
			}

			break;

		case _VIV_SETTINGS_ID_THEME:

			switch(config_dark_mode)
			{
				case 0:
					string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_DARK_MODE_LIGHT));
					break;

				case 1:
					string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_DARK_MODE_DARK));
					break;

				default:
					string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_DARK_MODE_AUTO));
					break;
			}

			break;

		case _VIV_SETTINGS_ID_SHRINK:
			string_copy_utf8_string(wbuf,localization_get_string((config_shrink_blit_mode == CONFIG_SHRINK_BLIT_MODE_HALFTONE) ? LOCALIZATION_ID_BLIT_MODE_LINEAR_COMBOBOXITEM : LOCALIZATION_ID_BLIT_MODE_NEAREST_COMBOBOXITEM));
			break;

		case _VIV_SETTINGS_ID_MAG:
			string_copy_utf8_string(wbuf,localization_get_string((config_mag_filter == CONFIG_MAG_FILTER_HALFTONE) ? LOCALIZATION_ID_BLIT_MODE_LINEAR_COMBOBOXITEM : LOCALIZATION_ID_BLIT_MODE_NEAREST_COMBOBOXITEM));
			break;

		case _VIV_SETTINGS_ID_TITLE:

			switch(config_title_bar_format)
			{
				case 0:
					string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_OPTIONS_TITLE_BAR_FORMAT_FULL_PATH_COMBOBOXITEM));
					break;

				case 1:
					string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_OPTIONS_TITLE_BAR_FORMAT_FILENAME_ONLY_COMBOBOXITEM));
					break;

				default:
					string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_OPTIONS_TITLE_BAR_FORMAT_NONE_COMBOBOXITEM));
					break;
			}

			break;

		case _VIV_SETTINGS_ID_AUTOTYPE:

			switch(config_auto_zoom_type)
			{
				case 0:
					string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_OPTIONS_VIEW_AUTO_SIZE_WINDOW_50_PERCENT_COMBOBOXITEM));
					break;

				case 1:
					string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_OPTIONS_VIEW_AUTO_SIZE_WINDOW_100_PERCENT_COMBOBOXITEM));
					break;

				case 2:
					string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_OPTIONS_VIEW_AUTO_SIZE_WINDOW_200_PERCENT_COMBOBOXITEM));
					break;

				default:
					string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_OPTIONS_VIEW_AUTO_SIZE_WINDOW_AUTO_FIT_COMBOBOXITEM));
					break;
			}

			break;

		case _VIV_SETTINGS_ID_LEFT:

			if ((config_left_click_action >= 0) && (config_left_click_action <= 6))
			{
				static const localization_id_t ids[7] = {LOCALIZATION_ID_OPTIONS_ACTION_SCROLL_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_PLAY_PAUSE_SLIDESHOW_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_PLAY_PAUSE_ANIMATION_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_ZOOM_IN_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_NEXT_IMAGE_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_ONE_TO_ONE_SCROLL_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_SCROLL_MOVE_WINDOW_COMBOBOXITEM};

				string_copy_utf8_string(wbuf,localization_get_string(ids[config_left_click_action]));
			}

			break;

		case _VIV_SETTINGS_ID_RIGHT:

			if ((config_right_click_action >= 0) && (config_right_click_action <= 2))
			{
				static const localization_id_t ids[3] = {LOCALIZATION_ID_OPTIONS_ACTION_CONTEXT_MENU_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_ZOOM_OUT_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_PREVIOUS_IMAGE_COMBOBOXITEM};

				string_copy_utf8_string(wbuf,localization_get_string(ids[config_right_click_action]));
			}

			break;

		case _VIV_SETTINGS_ID_WHEEL:

			if ((config_mouse_wheel_action >= 0) && (config_mouse_wheel_action <= 2))
			{
				static const localization_id_t ids[3] = {LOCALIZATION_ID_OPTIONS_ACTION_ZOOM_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_NEXT_PREV_COMBOBOXITEM,LOCALIZATION_ID_OPTIONS_ACTION_PREV_NEXT_COMBOBOXITEM};

				string_copy_utf8_string(wbuf,localization_get_string(ids[config_mouse_wheel_action]));
			}

			break;

		case _VIV_SETTINGS_ID_COMMAND:
			_viv_get_command_name(wbuf,_viv_settings_command_index);
			break;

		case _VIV_SETTINGS_ID_KEYS:

			if (_viv_settings_key_index >= 0)
			{
				_viv_get_key_text(wbuf,_viv_settings_key_at(_viv_settings_key_index));
			}

			break;
	}

	text_color = _viv_settings_color(_VIV_SETTINGS_C_TEXT);

	if (!_viv_settings_ctl_enabled(ctl))
	{
		text_color = _viv_settings_color(_VIV_SETTINGS_C_TEXTOFF);
	}

	text_rect.left = ctl->value.left + _viv_settings_dip(10);
	text_rect.top = ctl->value.top;
	text_rect.right = ctl->value.right - _viv_settings_dip(18);
	text_rect.bottom = ctl->value.bottom;

	if (*wbuf)
	{
		_viv_settings_draw_text_raw(hdc,&text_rect,wbuf,_viv_settings_font,text_color,DT_VCENTER | DT_END_ELLIPSIS);
	}
	else
	{
		if (ctl->id == _VIV_SETTINGS_ID_KEYS)
		{
			// no shortcut on this command: a dim dash keeps the field
			// from reading empty.
			_viv_settings_draw_text_raw(hdc,&text_rect,L"-",_viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXTOFF),DT_VCENTER);
		}
	}

	// the chevron.
	{
		POINT pts[3];
		int cx;
		int cy;

		cx = ctl->value.right - _viv_settings_dip(11);
		cy = (ctl->value.top + ctl->value.bottom) / 2;

		pts[0].x = cx - _viv_settings_dip(3);
		pts[0].y = cy - _viv_settings_dip(1);
		pts[1].x = cx + _viv_settings_dip(3);
		pts[1].y = cy - _viv_settings_dip(1);
		pts[2].x = cx;
		pts[2].y = cy + _viv_settings_dip(3);

		{
			HBRUSH brush;
			HPEN pen;
			HPEN old_pen;
			HGDIOBJ old_brush;

			brush = CreateSolidBrush(text_color);
			pen = CreatePen(PS_SOLID,1,text_color);

			old_pen = (HPEN)SelectObject(hdc,pen);
			old_brush = SelectObject(hdc,brush);

			Polygon(hdc,pts,3);

			SelectObject(hdc,old_brush);
			SelectObject(hdc,old_pen);

			DeleteObject(pen);
			DeleteObject(brush);
		}
	}

}

static void _viv_settings_draw_switch(HDC hdc,const _viv_settings_ctl_t *ctl,int on,int hot,int focus)
{
	RECT knob;
	COLORREF fill;
	COLORREF line;
	COLORREF knob_color;

	if (on)
	{
		fill = _viv_settings_color(hot ? _VIV_SETTINGS_C_ACCENT_HOT : _VIV_SETTINGS_C_ACCENT);
		line = fill;
		knob_color = _viv_settings_color(_VIV_SETTINGS_C_ON_ACCENT);
	}
	else
	{
		fill = _viv_settings_color(hot ? _VIV_SETTINGS_C_HOVER : _VIV_SETTINGS_C_INPUT);
		line = _viv_settings_color(_VIV_SETTINGS_C_LINE);
		knob_color = _viv_settings_color(_VIV_SETTINGS_C_TEXT2);
	}

	if (!_viv_settings_ctl_enabled(ctl))
	{
		fill = _viv_settings_color(_VIV_SETTINGS_C_INPUT);
		line = _viv_settings_color(_VIV_SETTINGS_C_LINE);
		knob_color = _viv_settings_color(_VIV_SETTINGS_C_TEXTOFF);
	}

	_viv_settings_fill_round(hdc,&ctl->value,(ctl->value.bottom - ctl->value.top) / 2,fill,line);

	if (focus)
	{
		_viv_settings_draw_focus_ring(hdc,&ctl->value);
	}

	// the knob: a 3 dip inset circle.
	knob.left = (on ? ctl->value.right - _viv_settings_dip(3) - (ctl->value.bottom - ctl->value.top - _viv_settings_dip(6)) : ctl->value.left + _viv_settings_dip(3));
	knob.top = ctl->value.top + _viv_settings_dip(3);
	knob.right = knob.left + (ctl->value.bottom - ctl->value.top - _viv_settings_dip(6));
	knob.bottom = ctl->value.bottom - _viv_settings_dip(3);

	{
		HBRUSH brush;
		HPEN pen;
		HPEN old_pen;
		HGDIOBJ old_brush;

		brush = CreateSolidBrush(knob_color);
		pen = CreatePen(PS_SOLID,1,knob_color);

		old_pen = (HPEN)SelectObject(hdc,pen);
		old_brush = SelectObject(hdc,brush);

		Ellipse(hdc,knob.left,knob.top,knob.right,knob.bottom);

		SelectObject(hdc,old_brush);
		SelectObject(hdc,old_pen);

		DeleteObject(pen);
		DeleteObject(brush);
	}

}

static void _viv_settings_draw_check(HDC hdc,const _viv_settings_ctl_t *ctl,int checked,int hot,int focus)
{
	RECT box;
	RECT text_rect;
	wchar_t wbuf[STRING_SIZE];
	COLORREF fill;
	COLORREF line;
	COLORREF mark;
	COLORREF text_color;
	int size;

	size = _viv_settings_dip(15);

	box.left = ctl->rect.left;
	box.top = ctl->rect.top + ((ctl->rect.bottom - ctl->rect.top) - size) / 2;
	box.right = box.left + size;
	box.bottom = box.top + size;

	if (checked)
	{
		fill = _viv_settings_color(hot ? _VIV_SETTINGS_C_ACCENT_HOT : _VIV_SETTINGS_C_ACCENT);
		line = fill;
		mark = _viv_settings_color(_VIV_SETTINGS_C_ON_ACCENT);
	}
	else
	{
		fill = _viv_settings_color(_VIV_SETTINGS_C_INPUT);
		line = _viv_settings_color(hot ? _VIV_SETTINGS_C_ACCENT_HOT : _VIV_SETTINGS_C_LINE);
		mark = _viv_settings_color(_VIV_SETTINGS_C_INPUT);
	}

	text_color = _viv_settings_color(_VIV_SETTINGS_C_TEXT);

	if (!_viv_settings_ctl_enabled(ctl))
	{
		text_color = _viv_settings_color(_VIV_SETTINGS_C_TEXTOFF);
	}

	_viv_settings_fill_round(hdc,&box,_viv_settings_dip(3),fill,line);

	if (focus)
	{
		_viv_settings_draw_focus_ring(hdc,&box);
	}

	if (checked)
	{
		HPEN pen;
		HPEN old_pen;
		int x0;
		int y0;
		int x1;
		int y1;
		int x2;
		int y2;
		int wide;

		wide = box.right - box.left;

		x0 = box.left + (wide * 2) / 10;
		y0 = box.top + (wide * 5) / 10;
		x1 = box.left + (wide * 4) / 10;
		y1 = box.top + (wide * 7) / 10;
		x2 = box.left + (wide * 8) / 10;
		y2 = box.top + (wide * 3) / 10;

		pen = CreatePen(PS_SOLID,_viv_settings_dip(2) > 1 ? _viv_settings_dip(2) : 1,mark);

		old_pen = (HPEN)SelectObject(hdc,pen);

		MoveToEx(hdc,x0,y0,NULL);
		LineTo(hdc,x1,y1);
		LineTo(hdc,x2,y2);

		SelectObject(hdc,old_pen);

		DeleteObject(pen);
	}

	// the label.
	text_rect.left = box.right + _viv_settings_dip(8);
	text_rect.top = ctl->rect.top;
	text_rect.right = ctl->rect.right;
	text_rect.bottom = ctl->rect.bottom;

	if (ctl->id == _VIV_SETTINGS_ID_SELECT_ALL)
	{
		string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_SETTINGS_SELECT_ALL));
	}
	else
	{
		// the extension name upper case (the checkbox labels of the old
		// dialog: bmp -> BMP).
		const char *ext;
		int i;

		ext = _viv_association_extensions[ctl->param];

		i = 0;

		while((ext[i]) && (i < 15))
		{
			wbuf[i] = (wchar_t)((ext[i] >= 'a') && (ext[i] <= 'z') ? (ext[i] - 'a' + 'A') : ext[i]);

			i++;
		}

		wbuf[i] = 0;
	}

	_viv_settings_draw_text_raw(hdc,&text_rect,wbuf,_viv_settings_font,text_color,DT_VCENTER | DT_END_ELLIPSIS);

}

static void _viv_settings_draw_color(HDC hdc,const _viv_settings_ctl_t *ctl,COLORREF colorref,int hot,int focus)
{
	RECT text_rect;
	HBRUSH brush;
	HPEN pen;
	HPEN old_pen;
	HGDIOBJ old_brush;

	text_rect.left = ctl->rect.left;
	text_rect.top = ctl->rect.top;
	text_rect.right = ctl->value.left - _viv_settings_dip(12);
	text_rect.bottom = ctl->rect.bottom;

	if (ctl->id == _VIV_SETTINGS_ID_WINCOLOR)
	{
		_viv_settings_draw_label(hdc,&text_rect,LOCALIZATION_ID_WINDOWED_BACKGROUND_COLOR_STATIC,_viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));
	}
	else
	{
		_viv_settings_draw_label(hdc,&text_rect,LOCALIZATION_ID_FULLSCREEN_BACKGROUND_COLOR_STATIC,_viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));
	}

	brush = CreateSolidBrush(colorref);
	pen = CreatePen(PS_SOLID,1,_viv_settings_color(hot ? _VIV_SETTINGS_C_ACCENT_HOT : _VIV_SETTINGS_C_LINE));

	old_pen = (HPEN)SelectObject(hdc,pen);
	old_brush = SelectObject(hdc,brush);

	Rectangle(hdc,ctl->value.left,ctl->value.top,ctl->value.right,ctl->value.bottom);

	SelectObject(hdc,old_brush);
	SelectObject(hdc,old_pen);

	DeleteObject(pen);
	DeleteObject(brush);

	if (focus)
	{
		_viv_settings_draw_focus_ring(hdc,&ctl->value);
	}
}

static void _viv_settings_draw_button(HDC hdc,const _viv_settings_ctl_t *ctl,int hot,int pressed,int focus)
{
	wchar_t wbuf[STRING_SIZE];
	COLORREF fill;
	COLORREF line;
	COLORREF text_color;

	if (ctl->id == _VIV_SETTINGS_ID_OK)
	{
		if (pressed)
		{
			fill = _viv_settings_color(_VIV_SETTINGS_C_ACCENT_DOWN);
		}
		else
		{
			fill = _viv_settings_color(hot ? _VIV_SETTINGS_C_ACCENT_HOT : _VIV_SETTINGS_C_ACCENT);
		}

		line = fill;
		text_color = _viv_settings_color(_VIV_SETTINGS_C_ON_ACCENT);
	}
	else
	{
		if (pressed)
		{
			fill = _viv_settings_color(_VIV_SETTINGS_C_PRESS);
		}
		else
		{
			fill = _viv_settings_color(hot ? _VIV_SETTINGS_C_HOVER : _VIV_SETTINGS_C_INPUT);
		}

		line = _viv_settings_color(_VIV_SETTINGS_C_LINE);
		text_color = _viv_settings_color(_VIV_SETTINGS_C_TEXT);
	}

	if (!_viv_settings_ctl_enabled(ctl))
	{
		text_color = _viv_settings_color(_VIV_SETTINGS_C_TEXTOFF);
	}

	_viv_settings_fill_round(hdc,&ctl->rect,_viv_settings_dip(6),fill,line);

	if (focus)
	{
		_viv_settings_draw_focus_ring(hdc,&ctl->rect);
	}

	if (ctl->id == _VIV_SETTINGS_ID_OK)
	{
		string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_SETTINGS_OK));
	}
	else
	if (ctl->id == _VIV_SETTINGS_ID_CANCEL)
	{
		string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_SETTINGS_CANCEL));
	}
	else
	{
		switch(ctl->id)
		{
			case _VIV_SETTINGS_ID_KEY_ADD:
				string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_ADD_KEY_BUTTON));
				break;

			case _VIV_SETTINGS_ID_KEY_EDIT:
				string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_EDIT_KEY_BUTTON));
				break;

			default:
				string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_REMOVE_KEY_BUTTON));
				break;
		}
	}

	_viv_settings_draw_text_raw(hdc,&ctl->rect,wbuf,_viv_settings_font,text_color,DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);

}

static void _viv_settings_draw_focus_ring(HDC hdc,const RECT *rect)
{
	HPEN pen;
	HPEN old_pen;
	HGDIOBJ old_brush;

	pen = CreatePen(PS_SOLID,_viv_settings_dip(2) > 1 ? _viv_settings_dip(2) : 1,_viv_settings_color(_VIV_SETTINGS_C_ACCENT));

	old_pen = (HPEN)SelectObject(hdc,pen);
	old_brush = SelectObject(hdc,GetStockObject(NULL_BRUSH));

	Rectangle(hdc,rect->left - 1,rect->top - 1,rect->right + 1,rect->bottom + 1);

	SelectObject(hdc,old_brush);
	SelectObject(hdc,old_pen);

	DeleteObject(pen);
}

// the close button rect inside the title row.
static void _viv_settings_title_close_rect(const RECT *client,RECT *rect)
{
	rect->right = client->right - _viv_settings_dip(8);
	rect->left = rect->right - _viv_settings_dip(_VIV_SETTINGS_TITLE_X_WIDE);
	rect->top = (_viv_settings_dip(_VIV_SETTINGS_TITLE_HIGH) - _viv_settings_dip(_VIV_SETTINGS_TITLE_X_WIDE)) / 2;
	rect->bottom = rect->top + _viv_settings_dip(_VIV_SETTINGS_TITLE_X_WIDE);
}

// is the point inside the title row close button?
static int _viv_settings_title_close_hit(int x,int y)
{
	RECT client;
	RECT rect;
	POINT pt;

	if (!_viv_settings_hwnd)
	{
		return 0;
	}

	GetClientRect(_viv_settings_hwnd,&client);

	_viv_settings_title_close_rect(&client,&rect);

	pt.x = x;
	pt.y = y;

	return PtInRect(&rect,pt) ? 1 : 0;
}

// the title row: the gear glyph, the window title and the close button.
static void _viv_settings_draw_title(HDC hdc,const RECT *client)
{
	RECT rect;
	RECT close_rect;
	HICON icon;
	int size;

	size = _viv_settings_dip(16);

	icon = glyphs_icon(GLYPH_SETTINGS,_viv_is_dark(),size);

	if (icon)
	{
		DrawIconEx(hdc,_viv_settings_dip(14),(_viv_settings_dip(_VIV_SETTINGS_TITLE_HIGH) - size) / 2,icon,size,size,0,NULL,DI_NORMAL);
	}

	rect.left = _viv_settings_dip(14) + size + _viv_settings_dip(10);
	rect.top = 0;
	rect.right = client->right - _viv_settings_dip(_VIV_SETTINGS_TITLE_X_WIDE) - _viv_settings_dip(8);
	rect.bottom = _viv_settings_dip(_VIV_SETTINGS_TITLE_HIGH);

	_viv_settings_draw_label(hdc,&rect,LOCALIZATION_ID_SETTINGS,_viv_settings_font_bold ? _viv_settings_font_bold : _viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));

	_viv_settings_title_close_rect(client,&close_rect);

	if (_viv_settings_title_pressed)
	{
		_viv_settings_fill_round(hdc,&close_rect,_viv_settings_dip(6),_viv_settings_color(_VIV_SETTINGS_C_PRESS),_viv_settings_color(_VIV_SETTINGS_C_PRESS));
	}
	else
	if (_viv_settings_title_hot)
	{
		_viv_settings_fill_round(hdc,&close_rect,_viv_settings_dip(6),_viv_settings_color(_VIV_SETTINGS_C_HOVER),_viv_settings_color(_VIV_SETTINGS_C_HOVER));
	}

	// the x: two strokes.
	{
		HPEN pen;
		HPEN old_pen;
		int cx;
		int cy;
		int r;

		cx = (close_rect.left + close_rect.right) / 2;
		cy = (close_rect.top + close_rect.bottom) / 2;
		r = _viv_settings_dip(5);

		pen = CreatePen(PS_SOLID,_viv_settings_dip(2) > 1 ? _viv_settings_dip(2) : 1,_viv_settings_color(_VIV_SETTINGS_C_TEXT));

		old_pen = (HPEN)SelectObject(hdc,pen);

		MoveToEx(hdc,cx - r,cy - r,NULL);
		LineTo(hdc,cx + r,cy + r);
		MoveToEx(hdc,cx + r,cy - r,NULL);
		LineTo(hdc,cx - r,cy + r);

		SelectObject(hdc,old_pen);

		DeleteObject(pen);
	}
}

// the accent color row: the label and the five round swatches. the active
// swatch wears a 2 dip accent ring and a white check.
static void _viv_settings_draw_accent(HDC hdc,const _viv_settings_ctl_t *ctl,int hot,int focus)
{
	RECT text_rect;
	RECT circle;
	RECT ring;
	int d;
	int step;
	int i;
	int accent;

	text_rect.left = ctl->rect.left;
	text_rect.top = ctl->rect.top;
	text_rect.right = ctl->value.left - _viv_settings_dip(12);
	text_rect.bottom = ctl->rect.bottom;

	_viv_settings_draw_label(hdc,&text_rect,LOCALIZATION_ID_ACCENT_COLOR,_viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));

	d = _viv_settings_dip(_VIV_SETTINGS_ACCENT_D);
	step = d + _viv_settings_dip(_VIV_SETTINGS_ACCENT_GAP);

	accent = viv_theme_accent();

	for(i=0;i<VIV_THEME_ACCENT_COUNT;i++)
	{
		HBRUSH brush;
		HPEN pen;
		HPEN old_pen;
		HGDIOBJ old_brush;
		COLORREF line;

		circle.left = ctl->value.left + (i * step);
		circle.top = ctl->value.top;
		circle.right = circle.left + d;
		circle.bottom = ctl->value.bottom;

		line = _viv_settings_color(((hot) && (i == _viv_settings_hot_swatch)) ? _VIV_SETTINGS_C_TEXT2 : _VIV_SETTINGS_C_LINE);

		brush = CreateSolidBrush(viv_theme_accent_swatch(i));
		pen = CreatePen(PS_SOLID,1,line);

		old_pen = (HPEN)SelectObject(hdc,pen);
		old_brush = SelectObject(hdc,brush);

		Ellipse(hdc,circle.left,circle.top,circle.right,circle.bottom);

		SelectObject(hdc,old_brush);
		SelectObject(hdc,old_pen);

		DeleteObject(pen);
		DeleteObject(brush);

		if (i == accent)
		{
			// the selected swatch: a 2 dip accent ring just outside it.
			ring.left = circle.left - _viv_settings_dip(2);
			ring.top = circle.top - _viv_settings_dip(2);
			ring.right = circle.right + _viv_settings_dip(2);
			ring.bottom = circle.bottom + _viv_settings_dip(2);

			brush = CreateSolidBrush(viv_theme_color(VIV_TK_ACCENT));
			pen = CreatePen(PS_SOLID,_viv_settings_dip(2) > 1 ? _viv_settings_dip(2) : 1,viv_theme_color(VIV_TK_ACCENT));

			old_pen = (HPEN)SelectObject(hdc,pen);
			old_brush = SelectObject(hdc,GetStockObject(NULL_BRUSH));

			Ellipse(hdc,ring.left,ring.top,ring.right,ring.bottom);

			SelectObject(hdc,old_brush);
			SelectObject(hdc,old_pen);

			DeleteObject(pen);
			DeleteObject(brush);

			// the white check.
			{
				HPEN check_pen;
				int w;

				w = circle.right - circle.left;

				check_pen = CreatePen(PS_SOLID,_viv_settings_dip(2) > 1 ? _viv_settings_dip(2) : 1,_viv_settings_color(_VIV_SETTINGS_C_ON_ACCENT));

				old_pen = (HPEN)SelectObject(hdc,check_pen);

				MoveToEx(hdc,circle.left + (w * 25) / 100,circle.top + (w * 52) / 100,NULL);
				LineTo(hdc,circle.left + (w * 44) / 100,circle.top + (w * 72) / 100);
				LineTo(hdc,circle.left + (w * 76) / 100,circle.top + (w * 30) / 100);

				SelectObject(hdc,old_pen);

				DeleteObject(check_pen);
			}
		}
	}

	if (focus)
	{
		_viv_settings_draw_focus_ring(hdc,&ctl->rect);
	}
}

static void _viv_settings_paint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc;
	HDC mem;
	HBITMAP bitmap;
	HBITMAP old_bitmap;
	RECT client;
	RECT nav_rect;
	RECT line_rect;
	int i;
	int inactive;

	hdc = BeginPaint(hwnd,&ps);

	if (!hdc)
	{
		return;
	}

	GetClientRect(hwnd,&client);

	mem = CreateCompatibleDC(hdc);

	if (!mem)
	{
		EndPaint(hwnd,&ps);

		return;
	}

	bitmap = CreateCompatibleBitmap(hdc,client.right,client.bottom);

	old_bitmap = (HBITMAP)SelectObject(mem,bitmap);

	inactive = (GetActiveWindow() != hwnd) ? 1 : 0;

	// the window face.
	FillRect(mem,&client,_viv_settings_brush(_VIV_SETTINGS_C_FACE));

	// the navigation face with a hairline right border (it starts
	// under the title row).
	nav_rect.left = 0;
	nav_rect.top = _viv_settings_dip(_VIV_SETTINGS_TITLE_HIGH);
	nav_rect.right = _viv_settings_dip(_VIV_SETTINGS_NAV_WIDE);
	nav_rect.bottom = client.bottom;

	FillRect(mem,&nav_rect,_viv_settings_brush(_VIV_SETTINGS_C_NAV_FACE));

	line_rect.left = nav_rect.right;
	line_rect.top = nav_rect.top;
	line_rect.right = nav_rect.right + 1;
	line_rect.bottom = client.bottom;

	FillRect(mem,&line_rect,_viv_settings_brush(_VIV_SETTINGS_C_LINE));

	// the title row divider.
	line_rect.left = 0;
	line_rect.top = _viv_settings_dip(_VIV_SETTINGS_TITLE_HIGH);
	line_rect.right = client.right;
	line_rect.bottom = line_rect.top + 1;

	FillRect(mem,&line_rect,_viv_settings_brush(_VIV_SETTINGS_C_LINE));

	// the 1 px outer border (the popup window has no native frame).
	{
		RECT border_rect;

		CopyRect(&border_rect,&client);
		border_rect.right--;
		border_rect.bottom--;

		FrameRect(mem,&border_rect,_viv_settings_brush(_VIV_SETTINGS_C_LINE));
	}

	// the title row: gear, title, close button.
	_viv_settings_draw_title(mem,&client);

	// the footer divider.
	line_rect.left = 0;
	line_rect.top = client.bottom - _viv_settings_dip(_VIV_SETTINGS_FOOTER_HIGH);
	line_rect.right = client.right;
	line_rect.bottom = line_rect.top + 1;

	FillRect(mem,&line_rect,_viv_settings_brush(_VIV_SETTINGS_C_LINE));

	// every control of the page.
	for(i=0;i<_viv_settings_ctl_count;i++)
	{
		_viv_settings_ctl_t *ctl;
		int hot;
		int focus;

		ctl = &_viv_settings_ctls[i];

		hot = (_viv_settings_hot == i) ? 1 : 0;
		focus = (_viv_settings_focus == i) ? 1 : 0;

		switch(ctl->type)
		{
			case _VIV_SETTINGS_CT_SECTION:

				// the section caption rides in param (the layout stores the
				// localization id there).
				_viv_settings_draw_label(mem,&ctl->rect,(localization_id_t)ctl->param,_viv_settings_font_bold ? _viv_settings_font_bold : _viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));

				break;

			case _VIV_SETTINGS_CT_DESC:

				if (_viv_settings_page == _VIV_SETTINGS_PAGE_GENERAL)
				{
					// the associations description.
					_viv_settings_draw_label(mem,&ctl->rect,LOCALIZATION_ID_SETTINGS_ASSOCIATIONS_DESC,_viv_settings_font_small ? _viv_settings_font_small : _viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT2));
				}
				else
				{
					// the key capture hint lines. param 0 carries the add / edit
					// prompt, param 1 the used by line.
					wchar_t wbuf[STRING_SIZE];

					wbuf[0] = 0;

					if (ctl->param)
					{
						// the used by line (empty while idle).
						_viv_settings_capture_used_by_text(wbuf);
					}
					else
					if (_viv_settings_capture_active)
					{
						wchar_t key_wbuf[STRING_SIZE];

						key_wbuf[0] = 0;

						if (_viv_settings_capture_key)
						{
							_viv_get_key_text(key_wbuf,_viv_settings_capture_key);
						}

						string_copy(wbuf,localization_get_string(_viv_settings_capture_edit ? LOCALIZATION_ID_EDIT_KEYBOARD_SHORTCUT_CAPTION : LOCALIZATION_ID_ADD_KEYBOARD_SHORTCUT_CAPTION));
						string_cat(wbuf,L": ");
						string_cat(wbuf,key_wbuf);
					}

					_viv_settings_draw_text_raw(mem,&ctl->rect,wbuf,_viv_settings_font_small ? _viv_settings_font_small : _viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT2),DT_VCENTER | DT_END_ELLIPSIS);
				}

				break;

			case _VIV_SETTINGS_CT_NAV:
				_viv_settings_draw_nav(mem,ctl,_viv_settings_page == ctl->param ? 1 : 0,hot,inactive);
				break;

			case _VIV_SETTINGS_CT_ACCENT:
			        _viv_settings_draw_accent(mem,ctl,hot,focus);
			        break;

			case _VIV_SETTINGS_CT_DROPDOWN:
			{
				RECT label_rect;

				label_rect.left = ctl->rect.left;
				label_rect.top = ctl->rect.top;
				label_rect.right = ctl->value.left - _viv_settings_dip(12);
				label_rect.bottom = ctl->rect.bottom;

				switch(ctl->id)
				{
					case _VIV_SETTINGS_ID_LANGUAGE:
						_viv_settings_draw_label(mem,&label_rect,LOCALIZATION_ID_SETTINGS_LANGUAGE,_viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));
						break;

					case _VIV_SETTINGS_ID_THEME:
						_viv_settings_draw_label(mem,&label_rect,LOCALIZATION_ID_SETTINGS_THEME,_viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));
						break;

					case _VIV_SETTINGS_ID_SHRINK:
						_viv_settings_draw_label(mem,&label_rect,LOCALIZATION_ID_SHRINK_BLIT_MODE_STATIC,_viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));
						break;

					case _VIV_SETTINGS_ID_MAG:
						_viv_settings_draw_label(mem,&label_rect,LOCALIZATION_ID_MAGNIFY_BLIT_MODE,_viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));
						break;

					case _VIV_SETTINGS_ID_TITLE:
						_viv_settings_draw_label(mem,&label_rect,LOCALIZATION_ID_OPTIONS_TITLE_BAR_FORMAT_STATIC,_viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));
						break;

					case _VIV_SETTINGS_ID_AUTOTYPE:
						// the sub row of the auto size switch: no label.
						break;

					case _VIV_SETTINGS_ID_LEFT:
						_viv_settings_draw_label(mem,&label_rect,LOCALIZATION_ID_LEFT_CLICK_ACTION_STATIC,_viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));
						break;

					case _VIV_SETTINGS_ID_RIGHT:
						_viv_settings_draw_label(mem,&label_rect,LOCALIZATION_ID_RIGHT_CLICK_ACTION_STATIC,_viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));
						break;

					case _VIV_SETTINGS_ID_WHEEL:
						_viv_settings_draw_label(mem,&label_rect,LOCALIZATION_ID_MOUSE_WHEEL_ACTION_STATIC,_viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));
						break;

					case _VIV_SETTINGS_ID_COMMAND:
						// the section header carries the label.
						break;

					case _VIV_SETTINGS_ID_KEYS:
						_viv_settings_draw_label(mem,&label_rect,LOCALIZATION_ID_SHORTCUT_KEY,_viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));
						break;
				}

				_viv_settings_draw_dropdown(mem,ctl,hot,focus);
				break;
			}

			case _VIV_SETTINGS_CT_SWITCH:
			{
				RECT label_rect;
				RECT desc_rect;
				int label_id;
				int desc_id;

				label_id = 0;
				desc_id = 0;

				switch(ctl->id)
				{
					case _VIV_SETTINGS_ID_MULTIPLE:
						label_id = LOCALIZATION_ID_SETTINGS_ALLOW_MULTIPLE;
						desc_id = LOCALIZATION_ID_SETTINGS_ALLOW_MULTIPLE_DESC;
						break;

					case _VIV_SETTINGS_ID_RUNKEY:
						label_id = LOCALIZATION_ID_SETTINGS_STARTUP_SHORTCUT;
						desc_id = LOCALIZATION_ID_SETTINGS_STARTUP_SHORTCUT_DESC;
						break;

					case _VIV_SETTINGS_ID_STARTMENU:
						label_id = LOCALIZATION_ID_STARTMENU_SHORTCUTS;
						break;

					case _VIV_SETTINGS_ID_APPDATA:
						label_id = LOCALIZATION_ID_STORE_SETTINGS_APPDATA;
						break;

					case _VIV_SETTINGS_ID_AUTOZOOM:
						label_id = LOCALIZATION_ID_OPTIONS_VIEW_AUTO_SIZE_WINDOW_STATIC;
						break;

					case _VIV_SETTINGS_ID_LOOP:
						label_id = LOCALIZATION_ID_PLAY_ANIMATIONS_ONCE_STATIC;
						break;

					case _VIV_SETTINGS_ID_PRELOAD:
						label_id = LOCALIZATION_ID_PRELOAD_NEXT_IMAGE_STATIC;
						break;

					case _VIV_SETTINGS_ID_CACHE:
						label_id = LOCALIZATION_ID_CACHE_LAST_IMAGE_STATIC;
						break;
				}

				label_rect.left = ctl->rect.left;
				label_rect.top = ctl->rect.top;
				label_rect.right = ctl->value.left - _viv_settings_dip(12);
				label_rect.bottom = desc_id ? ctl->rect.top + (ctl->rect.bottom - ctl->rect.top) / 2 : ctl->rect.bottom;

				_viv_settings_draw_label(mem,&label_rect,label_id,_viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT));

				if (desc_id)
				{
					wchar_t wbuf[STRING_SIZE];

					string_copy_utf8_string(wbuf,localization_get_string((localization_id_t)desc_id));

					desc_rect.left = ctl->rect.left;
					desc_rect.top = label_rect.bottom;
					desc_rect.right = ctl->value.left - _viv_settings_dip(12);
					desc_rect.bottom = ctl->rect.bottom;

					_viv_settings_draw_text_raw(mem,&desc_rect,wbuf,_viv_settings_font_small ? _viv_settings_font_small : _viv_settings_font,_viv_settings_color(_VIV_SETTINGS_C_TEXT2),DT_VCENTER | DT_END_ELLIPSIS);
				}

				switch(ctl->id)
				{
					case _VIV_SETTINGS_ID_MULTIPLE:
						_viv_settings_draw_switch(mem,ctl,config_multiple_instances ? 1 : 0,hot,focus);
						break;

					case _VIV_SETTINGS_ID_RUNKEY:
						_viv_settings_draw_switch(mem,ctl,_viv_settings_run_key_state,hot,focus);
						break;

					case _VIV_SETTINGS_ID_STARTMENU:
						_viv_settings_draw_switch(mem,ctl,_viv_settings_startmenu,hot,focus);
						break;

					case _VIV_SETTINGS_ID_APPDATA:
						_viv_settings_draw_switch(mem,ctl,_viv_settings_appdata,hot,focus);
						break;

					case _VIV_SETTINGS_ID_AUTOZOOM:
						_viv_settings_draw_switch(mem,ctl,config_auto_zoom ? 1 : 0,hot,focus);
						break;

					case _VIV_SETTINGS_ID_LOOP:
						_viv_settings_draw_switch(mem,ctl,config_loop_animations_once ? 1 : 0,hot,focus);
						break;

					case _VIV_SETTINGS_ID_PRELOAD:
						_viv_settings_draw_switch(mem,ctl,config_preload_next ? 1 : 0,hot,focus);
						break;

					case _VIV_SETTINGS_ID_CACHE:
						_viv_settings_draw_switch(mem,ctl,config_cache_last ? 1 : 0,hot,focus);
						break;
				}

				break;
			}

			case _VIV_SETTINGS_CT_CHECK:
			{
				int checked;

				if (ctl->id == _VIV_SETTINGS_ID_SELECT_ALL)
				{
					int exti;

					checked = 1;

					for(exti=0;exti<_VIV_ASSOCIATION_COUNT;exti++)
					{
						if (!_viv_settings_assoc[exti])
						{
							checked = 0;

							break;
						}
					}
				}
				else
				{
					checked = _viv_settings_assoc[ctl->param];
				}

				_viv_settings_draw_check(mem,ctl,checked,hot,focus);
				break;
			}

			case _VIV_SETTINGS_CT_COLOR:
			{
				COLORREF colorref;

				if (ctl->id == _VIV_SETTINGS_ID_WINCOLOR)
				{
					colorref = RGB(config_windowed_background_color_r,config_windowed_background_color_g,config_windowed_background_color_b);
				}
				else
				{
					colorref = RGB(config_fullscreen_background_color_r,config_fullscreen_background_color_g,config_fullscreen_background_color_b);
				}

				_viv_settings_draw_color(mem,ctl,colorref,hot,focus);
				break;
			}

			case _VIV_SETTINGS_CT_BUTTON:
			case _VIV_SETTINGS_CT_KEYBUTTON:
				_viv_settings_draw_button(mem,ctl,hot,_viv_settings_pressed == i ? 1 : 0,focus);
				break;
		}

		// the keyboard focus ring (2 dip accent stroke).
		if ((focus) && (_viv_settings_ctl_enabled(ctl)) && (ctl->type != _VIV_SETTINGS_CT_NAV))
		{
			_viv_settings_draw_focus_ring(mem,&ctl->rect);
		}
	}

	BitBlt(hdc,0,0,client.right,client.bottom,mem,0,0,SRCCOPY);

	SelectObject(mem,old_bitmap);

	DeleteObject(bitmap);
	DeleteDC(mem);

	EndPaint(hwnd,&ps);
}

// the second description line during a capture: where the key is in
// use (the edit key dialog's "currently used by" list, one line).
static void _viv_settings_capture_used_by_text(wchar_t *wbuf)
{
	int command_index;
	int count;

	wbuf[0] = 0;

	if ((!_viv_settings_capture_active) || (!_viv_settings_capture_key))
	{
		return;
	}

	// the label leads the line (the edit key dialog's static).
	string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_SHORTCUT_KEY_CURRENTLY_USED_BY));
	string_cat(wbuf,L" ");

	count = 0;

	for(command_index=0;command_index<_VIV_COMMAND_COUNT;command_index++)
	{
		config_key_t *key;

		key = _viv_key_list->start[command_index];

		while(key)
		{
			if (key->key == _viv_settings_capture_key)
			{
				wchar_t command_wbuf[STRING_SIZE];

				_viv_get_command_name(command_wbuf,command_index);

				if (count)
				{
					string_cat(wbuf,L", ");
				}

				string_cat(wbuf,command_wbuf);

				count++;

				break;
			}

			key = key->next;
		}
	}

	if (!count)
	{
		// a free key keeps the line empty.
		wbuf[0] = 0;
	}
}

// ---- window ----

static LRESULT CALLBACK _viv_settings_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_NCHITTEST:
		{
			POINT pt;

			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);

			ScreenToClient(hwnd,&pt);

			// the title row drags the window; the close button stays a
			// client hit so it can press.
			if (pt.y < _viv_settings_dip(_VIV_SETTINGS_TITLE_HIGH))
			{
				RECT client;
				RECT close_rect;

				GetClientRect(hwnd,&client);

				_viv_settings_title_close_rect(&client,&close_rect);

				if (!PtInRect(&close_rect,pt))
				{
					return HTCAPTION;
				}
			}

			break;
		}

		case WM_ERASEBKGND:
			// all painting happens in wm_paint (double buffered).
			return 1;

		case WM_PAINT:
			_viv_settings_paint(hwnd);
			return 0;

		case WM_SIZE:
			_viv_settings_layout();
			_viv_settings_invalidate();
			break;

		case WM_MOUSEMOVE:
		{
			int x;
			int y;
			int hit;

			x = GET_X_LPARAM(lParam);
			y = GET_Y_LPARAM(lParam);

			hit = _viv_settings_hit_test(x,y);

			if ((!_viv_settings_tracking) && (!_viv_settings_pressed))
			{
				_viv_settings_track_leave(hwnd);
			}

			if (hit != _viv_settings_hot)
			{
				_viv_settings_hot = hit;

				_viv_settings_invalidate();
			}

			// the close button hover.
			{
				int title_hot;

				title_hot = _viv_settings_title_close_hit(x,y);

				if (title_hot != _viv_settings_title_hot)
				{
					_viv_settings_title_hot = title_hot;

					_viv_settings_invalidate();
				}
			}

			// the swatch hover inside the accent row.
			{
				int swatch;

				swatch = -1;

				if ((hit >= 0) && (_viv_settings_ctls[hit].type == _VIV_SETTINGS_CT_ACCENT))
				{
					swatch = _viv_settings_swatch_from_x(&_viv_settings_ctls[hit],x);
				}

				if (swatch != _viv_settings_hot_swatch)
				{
					_viv_settings_hot_swatch = swatch;

					_viv_settings_invalidate();
				}
			}

			return 0;
		}

		case WM_MOUSELEAVE:

			_viv_settings_tracking = 0;
			_viv_settings_hot_swatch = -1;

			if (_viv_settings_title_hot)
			{
				_viv_settings_title_hot = 0;

				_viv_settings_invalidate();
			}

			if (_viv_settings_hot != _viv_settings_pressed)
			{
				_viv_settings_hot = -1;

				_viv_settings_invalidate();
			}

			return 0;

		case WM_LBUTTONDOWN:
		{
			int x;
			int y;
			int hit;

			SetFocus(hwnd);

			x = GET_X_LPARAM(lParam);
			y = GET_Y_LPARAM(lParam);

			if (_viv_settings_capture_active)
			{
				// a click while capturing: abandon the capture.
				_viv_settings_capture_end();

				return 0;
			}

			if (_viv_settings_title_close_hit(x,y))
			{
				SetCapture(hwnd);

				_viv_settings_title_pressed = 1;
				_viv_settings_title_hot = 1;

				_viv_settings_invalidate();

				return 0;
			}

			hit = _viv_settings_hit_test(x,y);

			if (hit >= 0)
			{
				SetCapture(hwnd);

				_viv_settings_pressed = hit;
				_viv_settings_hot = hit;

				_viv_settings_focus_set(hit,0);

				_viv_settings_invalidate();
			}

			return 0;
		}

		case WM_LBUTTONUP:
		{
			int x;
			int y;
			int hit;
			int down;

			x = GET_X_LPARAM(lParam);
			y = GET_Y_LPARAM(lParam);

			hit = _viv_settings_hit_test(x,y);
			down = _viv_settings_pressed;

			if (GetCapture() == hwnd)
			{
				ReleaseCapture();
			}

			_viv_settings_pressed = -1;
			_viv_settings_hot = hit;

			_viv_settings_invalidate();

			// the close button fires on the release inside it.
			if (_viv_settings_title_pressed)
			{
				_viv_settings_title_pressed = 0;
				_viv_settings_title_hot = _viv_settings_title_close_hit(x,y);

				_viv_settings_invalidate();

				if (_viv_settings_title_close_hit(x,y))
				{
					_viv_settings_cancel();
				}

				return 0;
			}

			if ((down >= 0) && (hit == down))
			{
				_viv_settings_activate(down,x,y);
			}

			return 0;
		}

		case WM_CAPTURECHANGED:

			if ((HWND)lParam != hwnd)
			{
				if (_viv_settings_pressed != -1)
				{
					_viv_settings_pressed = -1;
					_viv_settings_title_pressed = 0;

					_viv_settings_hot_swatch = -1;

					_viv_settings_invalidate();
				}
			}

			return 0;

		case WM_SETFOCUS:
			// tab in: focus the navigation unless something is focused.
			if ((_viv_settings_focus < 0) && (_viv_settings_ctl_count))
			{
				_viv_settings_focus_set(0,0);
			}

			_viv_settings_invalidate();
			return 0;

		case WM_KILLFOCUS:
			_viv_settings_invalidate();
			return 0;

		case WM_ACTIVATE:

			if (LOWORD(wParam) == WA_INACTIVE)
			{
				if (_viv_settings_capture_active)
				{
					_viv_settings_capture_end();
				}

				if (GetCapture() == hwnd)
				{
					ReleaseCapture();
				}

				_viv_settings_pressed = -1;
				_viv_settings_title_pressed = 0;
				_viv_settings_hot = -1;

				_viv_settings_invalidate();
			}

			return 0;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			int vk;

			vk = (int)wParam;

			// the key capture eats everything (the edit key dialog's
			// want all keys behavior).
			if (_viv_settings_capture_active)
			{
				// bare enter confirms and bare escape cancels; the modifier
				// combinations fall through to the capture (ctrl+return is the
				// app's default open file location binding).
				if ((msg == WM_KEYDOWN) && !(GetKeyState(VK_CONTROL) & 0x8000) &&
					!(GetKeyState(VK_SHIFT) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000))
				{
					switch(vk)
					{
						case VK_ESCAPE:
							_viv_settings_capture_end();
							return 0;

						case VK_RETURN:
							_viv_settings_capture_commit();
							return 0;
					}
				}

				_viv_settings_capture_key_down(wParam);

				return 0;
			}

			switch(vk)
			{
				case VK_TAB:

					_viv_settings_focus_cycle(GetKeyState(VK_SHIFT) < 0 ? -1 : 1);

					return 0;

				case VK_UP:
				case VK_DOWN:
				{
					int index;

					// a dropdown in focus: the arrows open the list.
					if ((_viv_settings_focus >= 0) && (_viv_settings_ctls[_viv_settings_focus].type == _VIV_SETTINGS_CT_DROPDOWN))
					{
						_viv_settings_activate(_viv_settings_focus,-1,-1);

						return 0;
					}

					index = _viv_settings_focus_next(_viv_settings_focus < 0 ? 0 : _viv_settings_focus,(vk == VK_DOWN) ? 1 : -1);

					// focusing a navigation item switches to its page.
					if ((index >= 0) && (_viv_settings_ctls[index].type == _VIV_SETTINGS_CT_NAV))
					{
						_viv_settings_page = _viv_settings_ctls[index].param;

						_viv_settings_layout();
					}

					_viv_settings_focus_set(index,1);

					return 0;
				}

				case VK_LEFT:
				case VK_RIGHT:
				{
					// the checkbox grid walks the columns.
					if ((_viv_settings_focus >= 0) && (_viv_settings_ctls[_viv_settings_focus].type == _VIV_SETTINGS_CT_CHECK))
					{
						int index;
						int dir;

						dir = (vk == VK_RIGHT) ? 1 : -1;

						index = _viv_settings_focus + dir;

						if ((index >= 0) && (index < _viv_settings_ctl_count) && (_viv_settings_ctls[index].type == _VIV_SETTINGS_CT_CHECK))
						{
							_viv_settings_focus_set(index,1);
						}

						return 0;
					}

					// a dropdown in focus: open the list.
					if ((_viv_settings_focus >= 0) && (_viv_settings_ctls[_viv_settings_focus].type == _VIV_SETTINGS_CT_DROPDOWN))
					{
						_viv_settings_activate(_viv_settings_focus,-1,-1);

						return 0;
					}

					return 0;
				}

				case VK_SPACE:

					if (_viv_settings_focus >= 0)
					{
						_viv_settings_activate(_viv_settings_focus,-1,-1);
					}

					return 0;

				case VK_RETURN:

					_viv_settings_ok();

					return 0;

				case VK_ESCAPE:

					_viv_settings_cancel();

					return 0;
			}

			break;
		}

		case WM_CHAR:
			// the keydown handlers answered everything; swallow the
			// generated character (no default beeps).
			return 0;

		case WM_SYSCOMMAND:

			if ((wParam & 0xFFF0) == SC_CLOSE)
			{
				// the close box is a cancel.
				_viv_settings_cancel();

				return 0;
			}

			break;

		case WM_CLOSE:
			_viv_settings_cancel();
			return 0;

		case WM_DESTROY:
			break;

		case WM_NCDESTROY:

			_viv_settings_fonts_delete();

			_viv_key_clear_all(&_viv_settings_keylist);

			_viv_settings_hwnd = 0;
			_viv_settings_ctl_count = 0;
			_viv_settings_hot = -1;
			_viv_settings_pressed = -1;
			_viv_settings_focus = -1;
			_viv_settings_tracking = 0;
			_viv_settings_title_hot = 0;
			_viv_settings_title_pressed = 0;
			_viv_settings_hot_swatch = -1;
			_viv_settings_capture_active = 0;

			return 0;

		case WM_INITMENUPOPUP:

			// rc.13: the dropdown popups open from this owner window -
			// the shared popup theming (the class brush, the dwm chrome)
			// must run here exactly as it runs for the main window.
			_viv_menu_popup_theme();

			break;

		case WM_DRAWITEM:

			// the dropdown rows paint through the shared menu painter (no
			// status panes live on this window, so the menu branch is the
			// whole answer).
			if ((lParam) && (((DRAWITEMSTRUCT *)lParam)->CtlType == ODT_MENU))
			{
				if (_viv_menu_draw_item((DRAWITEMSTRUCT *)lParam))
				{
					return TRUE;
				}
			}

			break;

		case WM_MEASUREITEM:

			if ((lParam) && (((MEASUREITEMSTRUCT *)lParam)->CtlType == ODT_MENU))
			{
				if (_viv_menu_measure_item((MEASUREITEMSTRUCT *)lParam))
				{
					return TRUE;
				}
			}

			break;

		case WM_DELETEITEM:

			// the dropdown rows live in the static settings pool: nothing
			// per item to free (the same rule the main menus follow).
			if ((lParam) && (((DELETEITEMSTRUCT *)lParam)->CtlType == ODT_MENU))
			{
				return TRUE;
			}

			break;

		case WM_THEMECHANGED:

			_viv_settings_theme_update();

			return 0;

		case WM_SETTINGCHANGE:

			// the auto theme follows the system: refresh and repaint.
			if ((lParam) && (string_compare((const wchar_t *)lParam,L"ImmersiveColorSet") == 0))
			{
				os_dark_refresh();

				_viv_settings_theme_update();
			}

			break;

		case WM_DPICHANGED:
		{
			RECT *rect;

			_viv_settings_dpi = os_window_dpi(hwnd);
			vivp_dpi_probe("wm_dpichanged");

			_viv_settings_fonts_create();

			rect = (RECT *)lParam;

			if (rect)
			{
				SetWindowPos(hwnd,0,rect->left,rect->top,rect->right - rect->left,rect->bottom - rect->top,SWP_NOZORDER | SWP_NOACTIVATE);
			}

			_viv_settings_layout();
			_viv_settings_invalidate();

			return 0;
		}
	}

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

void _viv_settings_show(void)
{
	int wide;
	int high;

	if (_viv_settings_hwnd)
	{
		if (IsIconic(_viv_settings_hwnd))
		{
			ShowWindow(_viv_settings_hwnd,SW_RESTORE);
		}

		SetForegroundWindow(_viv_settings_hwnd);

		return;
	}

	// the window starts at the viewer's dpi (wm_dpichanged corrects the
	// value when the window lands on another monitor).
	_viv_settings_dpi = 96;

	if (_viv_hwnd)
	{
		_viv_settings_dpi = os_window_dpi(_viv_hwnd);
	}
	vivp_dpi_probe("show-from-main");

	if (!_viv_settings_is_registered)
	{
		// rc.13: the non-win11 frame fallback - the drop shadow class
		// style. a borderless popup gets no dwm shadow on windows 10/7
		// and the panel floated bare on the desktop (the field report);
		// win11 keeps the dwm's own shadow with the rounded corners.
		os_RegisterClassEx(os_is_win11() ? 0 : CS_DROPSHADOW,_viv_settings_proc,0,LoadCursor(NULL,IDC_ARROW),NULL,"_VIV_SETTINGS",0);

		_viv_settings_is_registered = 1;
	}

	_viv_settings_window_size_px(&wide,&high);

	_viv_settings_hwnd = os_CreateWindowEx(
		0,
		"_VIV_SETTINGS",
		"",
		WS_POPUP | WS_SYSMENU,
		CW_USEDEFAULT,CW_USEDEFAULT,wide,high,
		_viv_hwnd,0,os_hinstance,NULL);

	if (!_viv_settings_hwnd)
	{
		return;
	}

	_viv_settings_dpi = os_window_dpi(_viv_settings_hwnd);
	vivp_dpi_probe("show-after-create");

	_viv_settings_fonts_create();

	_viv_settings_snapshot();

	// restore the last page (clamped to the page count).
	_viv_settings_page = config_options_last_page;

	if ((_viv_settings_page < 0) || (_viv_settings_page >= _VIV_OPTIONS_PAGE_COUNT))
	{
		_viv_settings_page = _VIV_SETTINGS_PAGE_GENERAL;
	}

	_viv_settings_layout();

	// the keyboard focus starts on the restored page's navigation item.
	_viv_settings_focus_set(_viv_settings_page,0);

	os_SetWindowText_localization_id(_viv_settings_hwnd,LOCALIZATION_ID_SETTINGS);

	_viv_settings_theme_update();

	os_center_dialog(_viv_settings_hwnd);

	ShowWindow(_viv_settings_hwnd,SW_SHOW);

	SetForegroundWindow(_viv_settings_hwnd);
}

void _viv_settings_kill(void)
{
	// app exit teardown: a window still open here is force destroyed by
	// the owner and the live-applied changes stand (implicit ok - the
	// cancel escape hatch only exists while the app runs).

	if (_viv_settings_hwnd)
	{
		DestroyWindow(_viv_settings_hwnd);
	}
}
