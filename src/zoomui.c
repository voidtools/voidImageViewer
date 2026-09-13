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
// Floating zoom controls (touch friendly) implementation.
//
// one seven cell row serves both modes now: prev / play-pause / next, a
// hairline separator, then zoom out / zoom percent / zoom in. the row is
// horizontally centered at the bottom of the image area in windowed mode
// and in fullscreen alike, drawn on a WS_EX_LAYERED child window. the
// percent cell is a plain text cell: it reads the zoom ladder value at
// the current position with the same nearest integer rounding the status
// bar zoom pane uses, has no hover, no click and no tooltip, and is only
// HTCLIENT so clicks on it never fall through to the image. after two
// idle seconds in fullscreen the bar fades out in 15ms alpha steps and
// any mouse, key or command activity fades it back in. on windows 7,
// where layered child windows are not supported, the bar hides without
// the fade.
//
// single window self drawn pill (no embedded BUTTON child windows): one
// WS_CHILD window paints the stadium tray and every capsule cell in
// WM_PAINT, tracks hover / press with capture + hit testing, fires the
// real commands with WM_COMMAND, hosts one rect based tooltip, and
// answers HTTRANSPARENT outside the button and percent cells so the
// rounded corners and the tray gaps never eat clicks. no Mica, plain GDI
// only.

#include "viv.h"
#include "zoomui.h"
#include "viv_state.h"
#include "viv_chrome.h"

// per monitor dpi change message. (not defined in older SDKs)
#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

// layered window alpha. (not defined in older SDKs)
#ifndef LWA_ALPHA
#define LWA_ALPHA 2
#endif

#ifndef BN_CLICKED
#define BN_CLICKED 0
#endif

// the seven cells of the row, left to right. the same row serves
// windowed mode and fullscreen: the mode only decides the idle fade.
#define _ZOOMUI_CELL_COUNT 7
#define _ZOOMUI_CELL_PREV 0
#define _ZOOMUI_CELL_PLAYPAUSE 1
#define _ZOOMUI_CELL_NEXT 2
#define _ZOOMUI_CELL_SEP 3
#define _ZOOMUI_CELL_ZOOMOUT 4
#define _ZOOMUI_CELL_PCT 5
#define _ZOOMUI_CELL_ZOOMIN 6

// command ids for the fixed button cells. the play/pause cell resolves
// its command from the slideshow state at fire time (play when stopped,
// pause when running), the separator and the percent cell never fire.
#define _ZOOMUI_CELL_COMMAND_NONE 0

static const int _zoomui_cell_command_ids[_ZOOMUI_CELL_COUNT] =
{
	VIV_ID_NAV_PREV,
	_ZOOMUI_CELL_COMMAND_NONE,
	VIV_ID_NAV_NEXT,
	_ZOOMUI_CELL_COMMAND_NONE,
	VIV_ID_VIEW_ZOOM_OUT,
	_ZOOMUI_CELL_COMMAND_NONE,
	VIV_ID_VIEW_ZOOM_IN,
};

// glyphs for the fixed cells. the play/pause cell picks GLYPH_PLAY or
// GLYPH_PAUSE at paint time from the slideshow state; the separator and
// the percent cell draw no glyph at all.
#define _ZOOMUI_CELL_GLYPH_NONE 0

static const int _zoomui_cell_glyph_ids[_ZOOMUI_CELL_COUNT] =
{
	GLYPH_PREV,
	_ZOOMUI_CELL_GLYPH_NONE,
	GLYPH_NEXT,
	_ZOOMUI_CELL_GLYPH_NONE,
	GLYPH_ZOOMOUT,
	_ZOOMUI_CELL_GLYPH_NONE,
	GLYPH_ZOOMIN,
};

// fade parameters.
#define _ZOOMUI_TIMER_ID 1
#define _ZOOMUI_FADE_INTERVAL 15     // ms per alpha step.
#define _ZOOMUI_ALPHA_STEP 17        // ~240ms for a full 0..255 fade.
#define _ZOOMUI_ALPHA_OPAQUE 255
#define _ZOOMUI_IDLE_MS 2000         // idle before the fade out starts.

// percent / play state poll. zoom changes that do not pass through the
// pill (mouse wheel, pinch, the zoom pane editor) leave no message trace
// in this window, so the text cell refreshes itself from the ladder on
// a short poll instead.
#define _ZOOMUI_PCT_TIMER_ID 2
#define _ZOOMUI_PCT_POLL_INTERVAL 150 // ms between percent state checks.

static HWND _zoomui_hwnd = 0;
static HWND _zoomui_parent_hwnd = 0;
static HWND _zoomui_tooltip_hwnd = 0;

static int _zoomui_cell_wide = 0; // button capsule width (48 dip).
static int _zoomui_cell_high = 0; // button capsule height (44 dip).
static int _zoomui_cell_gap = 0; // spacing between the row cells (4 dip).
static int _zoomui_margin = 0; // breathing room inside the tray (6 dip).
static int _zoomui_sep_wide = 0; // separator rule width (1 dip).
static int _zoomui_sep_high = 0; // separator rule height (24 dip).
static int _zoomui_pct_pad = 0; // total padding around the percent text (16 dip).
static int _zoomui_pct_wide = 0; // percent cell width: text width + pad.
static int _zoomui_pct_percent = 100; // the percent the text cell shows.
static int _zoomui_is_slideshow_cached = 0; // the play/pause face follows this.
static RECT _zoomui_cell_rects[_ZOOMUI_CELL_COUNT]; // client coords.
static int _zoomui_area_wide = 0; // last zoomui_layout image area width.
static int _zoomui_area_high = 0; // last zoomui_layout image area height.
static int _zoomui_is_registered = 0;
static int _zoomui_hot_index = -1; // button cell under the cursor, or -1.
static int _zoomui_pressed_index = -1; // button cell pressed with capture, or -1.
static int _zoomui_pct_recenter = 0; // rc.13: a percent width change deferred through a press.
static int _zoomui_dark = 0; // 1 = draw with the dark mode palette.
static int _zoomui_is_fullscreen = 0; // 1 = the fullscreen overlay (idle fade).

static int _zoomui_layered_ok = 0; // WS_EX_LAYERED child support (win8+).
static int _zoomui_alpha = _ZOOMUI_ALPHA_OPAQUE; // current alpha value.
static int _zoomui_alpha_target = _ZOOMUI_ALPHA_OPAQUE;
static DWORD _zoomui_last_activity = 0; // GetTickCount of the last user input.
static int _zoomui_visible_wanted = 0; // the state zoomui_show() latched.

static void _zoomui_apply_tooltip_colors(void);

static void _zoomui_draw_button(HDC hdc,const RECT *rect,int celli,int is_pressed,int is_disabled,int is_hot,int has_focus);
static void _zoomui_draw_icon(HDC hdc,const RECT *rect,int celli,int offset,int is_disabled);
static void _zoomui_draw_separator(HDC hdc);
static void _zoomui_draw_percent(HDC hdc,const RECT *rect);
static LRESULT CALLBACK _zoomui_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void _zoomui_invalidate(void);
static int _zoomui_cell_at(int x,int y);
static int _zoomui_is_button_cell(int celli);
static int _zoomui_step_button_cell(int fromi,int dir);
static int _zoomui_is_button_disabled(int celli);
static void _zoomui_fire_button(int celli);
static void _zoomui_track_leave(HWND hwnd);
static void _zoomui_calc_metrics(void);
static void _zoomui_update_cell_rects(void);
static int _zoomui_cell_width(int celli);
static int _zoomui_row_wide(void);
static void _zoomui_place(int wide,int high);
static void _zoomui_measure_pct_wide(void);
static int _zoomui_percent(void);
int _viv_zoom_percent(void); // viv_render.c: the app-wide zoom percent
static void _zoomui_build_pct_text(wchar_t *buf,int percent);
static void _zoomui_poll_state(void);
static void _zoomui_ensure_poll_timer(void);
static void _zoomui_kill_poll_timer(void);
static localization_id_t _zoomui_tooltip_id_for_cell(int celli);
static void _zoomui_tooltip_create(void);
static void _zoomui_tooltip_destroy(void);
static void _zoomui_tooltip_update_rects(void);
static void _zoomui_tooltip_update_text(int celli);
static void _zoomui_clear_hover_press(int invalidate);

// the auto hide only applies to the fullscreen bar: the windowed row is
// a plain control and stays put.
static int _zoomui_autohide_enabled(void)
{
	return ((config_zoom_auto_hide) && (_zoomui_is_fullscreen)) ? 1 : 0;
}

static void _zoomui_ensure_timer(void)
{
	if (_zoomui_hwnd)
	{
		SetTimer(_zoomui_hwnd,_ZOOMUI_TIMER_ID,_ZOOMUI_FADE_INTERVAL,0);
	}
}

static void _zoomui_kill_timer(void)
{
	if (_zoomui_hwnd)
	{
		KillTimer(_zoomui_hwnd,_ZOOMUI_TIMER_ID);
	}
}

static void _zoomui_ensure_poll_timer(void)
{
	if (_zoomui_hwnd)
	{
		SetTimer(_zoomui_hwnd,_ZOOMUI_PCT_TIMER_ID,_ZOOMUI_PCT_POLL_INTERVAL,0);
	}
}

static void _zoomui_kill_poll_timer(void)
{
	if (_zoomui_hwnd)
	{
		KillTimer(_zoomui_hwnd,_ZOOMUI_PCT_TIMER_ID);
	}
}

// push the current alpha to the layered window. ignored when the
// layered child support is missing (the bar is simply opaque then).
static void _zoomui_set_alpha(int alpha)
{
	if ((_zoomui_hwnd) && (_zoomui_layered_ok))
	{
		SetLayeredWindowAttributes(_zoomui_hwnd,0,(BYTE)alpha,LWA_ALPHA);
	}
}

static void _zoomui_calc_metrics(void)
{
	// touch friendly sizing: 48x44 logical units per capsule.
	_zoomui_cell_wide = (48 * os_logical_wide) / 96;
	_zoomui_cell_high = (44 * os_logical_high) / 96;
	_zoomui_margin = (6 * os_logical_high) / 96;
	// breathing room between the row cells: the gaps keep the capsule
	// borders from reading as one double line in the middle of the row.
	_zoomui_cell_gap = (4 * os_logical_high) / 96;
	// the separator rule: one logical unit wide, 24 tall.
	_zoomui_sep_wide = (1 * os_logical_wide) / 96;
	_zoomui_sep_high = (24 * os_logical_high) / 96;
	// total padding around the percent text: 8 dip a side.
	_zoomui_pct_pad = (16 * os_logical_wide) / 96;

	if (_zoomui_cell_wide < 32)
	{
		_zoomui_cell_wide = 32;
	}

	if (_zoomui_cell_high < 28)
	{
		_zoomui_cell_high = 28;
	}

	if (_zoomui_cell_gap < 2)
	{
		_zoomui_cell_gap = 2;
	}

	if (_zoomui_sep_wide < 1)
	{
		_zoomui_sep_wide = 1;
	}

	if (_zoomui_sep_high < 12)
	{
		_zoomui_sep_high = 12;
	}

	if (_zoomui_pct_pad < 8)
	{
		_zoomui_pct_pad = 8;
	}
}

// the width of one row cell: capsules are 48 dip, the separator is its
// hairline and the percent cell hugs its measured text.
static int _zoomui_cell_width(int celli)
{
	if (celli == _ZOOMUI_CELL_SEP)
	{
		return _zoomui_sep_wide;
	}

	if (celli == _ZOOMUI_CELL_PCT)
	{
		return _zoomui_pct_wide;
	}

	return _zoomui_cell_wide;
}

// the whole row width: every cell plus the gaps between them.
static int _zoomui_row_wide(void)
{
	int i;
	int total;

	total = 0;

	for(i=0;i<_ZOOMUI_CELL_COUNT;i++)
	{
		total += _zoomui_cell_width(i);
	}

	total += (_ZOOMUI_CELL_COUNT - 1) * _zoomui_cell_gap;

	return total;
}

// recompute the per cell client rects from the current metrics. every
// cell spans the full capsule height; the separator centers its rule
// inside its rect at draw time. the container client size is:
//   wide = row_wide + 2*margin
//   high = cell_high + 2*margin
static void _zoomui_update_cell_rects(void)
{
	int i;
	int x;

	x = _zoomui_margin;

	for(i=0;i<_ZOOMUI_CELL_COUNT;i++)
	{
		_zoomui_cell_rects[i].left = x;
		_zoomui_cell_rects[i].top = _zoomui_margin;
		_zoomui_cell_rects[i].right = x + _zoomui_cell_width(i);
		_zoomui_cell_rects[i].bottom = _zoomui_margin + _zoomui_cell_high;

		x += _zoomui_cell_width(i) + _zoomui_cell_gap;
	}
}

// build the percent cell text: a plain whole percent.
static void _zoomui_build_pct_text(wchar_t *buf,int percent)
{
	string_printf(buf,"%d%%",percent);
}

// the percent the text cell shows: the zoom ladder value at the current
// position as a whole percent. the rounding is the status bar zoom pane
// the app-wide number: the rendered size over the native size, the same
// value the status bar zoom pane and the set-zoom dialog show (the
// ladder scale disagreed: a best-fit photo read "100%" while the rest
// of the ui read its true shrink factor).
static int _zoomui_percent(void)
{
	return _viv_zoom_percent();
}

// measure the percent text in the menu font and size the percent cell:
// text width + 16 dip, never narrower than one capsule so the row keeps
// its rhythm with short strings like "6%".
static void _zoomui_measure_pct_wide(void)
{
	wchar_t wbuf[STRING_SIZE];
	HDC hdc;
	HFONT font;
	HFONT old_font;
	SIZE size;
	int wide;

	_zoomui_build_pct_text(wbuf,_zoomui_pct_percent);

	size.cx = 0;
	size.cy = 0;

	hdc = GetDC(_zoomui_hwnd ? _zoomui_hwnd : 0);

	if (hdc)
	{
		font = _viv_menu_font();
		old_font = 0;

		if (font)
		{
			old_font = SelectObject(hdc,font);
		}

		GetTextExtentPoint32W(hdc,wbuf,string_get_length(wbuf),&size);

		if (old_font)
		{
			SelectObject(hdc,old_font);
		}

		ReleaseDC(_zoomui_hwnd ? _zoomui_hwnd : 0,hdc);
	}

	wide = size.cx + _zoomui_pct_pad;

	if (wide < _zoomui_cell_wide)
	{
		wide = _zoomui_cell_wide;
	}

	_zoomui_pct_wide = wide;
}

// position the row: bottom center of the image area in both modes, the
// fullscreen overlay and the windowed pill share the anchor. wide,high
// = the image area the parent passes through zoomui_layout.
static void _zoomui_place(int wide,int high)
{
	int container_wide;
	int container_high;
	int x;
	int y;

	if (!_zoomui_hwnd)
	{
		return;
	}

	container_wide = _zoomui_row_wide() + (_zoomui_margin * 2);
	container_high = _zoomui_cell_high + (_zoomui_margin * 2);

	if (wide < 0)
	{
		wide = 0;
	}

	if (high < 0)
	{
		high = 0;
	}

	// the row hangs centered at the bottom of the image area. the area
	// height arrives viewport relative: land the y in client coordinates
	// below the top strips (zero in fullscreen, where the strips hide).
	x = (wide - container_wide) / 2;
	y = _viv_get_view_top() + high - container_high - _zoomui_margin;

	// clamp inside the image area so a tiny window never pushes the
	// pill off screen; when the area is smaller than the pill, pin to
	// the origin and let the parent clip.
	if (container_wide >= wide)
	{
		x = 0;
	}
	else
	{
		if (x < 0)
		{
			x = 0;
		}

		if (x + container_wide > wide)
		{
			x = wide - container_wide;
		}
	}

	if (container_high >= high)
	{
		y = 0;
	}
	else
	{
		if (y < 0)
		{
			y = 0;
		}

		if (y + container_high > high)
		{
			y = high - container_high;
		}
	}

	SetWindowPos(_zoomui_hwnd,HWND_TOP,x,y,container_wide,container_high,SWP_NOACTIVATE);

	_zoomui_update_cell_rects();
	_zoomui_tooltip_update_rects();
}

// hit test a client point against the live cell rects. returns the raw
// cell index (separator and percent included) or -1 for the tray gaps,
// the stadium corners and everything outside the cells.
static int _zoomui_cell_at(int x,int y)
{
	int i;
	POINT pt;

	pt.x = x;
	pt.y = y;

	for(i=0;i<_ZOOMUI_CELL_COUNT;i++)
	{
		if (PtInRect(&_zoomui_cell_rects[i],pt))
		{
			return i;
		}
	}

	return -1;
}

// the cells that behave as buttons: everything except the separator and
// the percent text cell. only button cells hover, press, fire and carry
// keyboard focus.
static int _zoomui_is_button_cell(int celli)
{
	if ((celli < 0) || (celli >= _ZOOMUI_CELL_COUNT))
	{
		return 0;
	}

	if ((celli == _ZOOMUI_CELL_SEP) || (celli == _ZOOMUI_CELL_PCT))
	{
		return 0;
	}

	return 1;
}

// the next button cell walking dir (+1 / -1) from fromi, wrapping
// around the row; -1 starts at the first (dir > 0) or last (dir < 0)
// button cell. the separator and the percent cell are skipped: they are
// never a keyboard target.
static int _zoomui_step_button_cell(int fromi,int dir)
{
	int celli;
	int guard;

	if (fromi < 0)
	{
		return (dir < 0) ? _ZOOMUI_CELL_ZOOMIN : _ZOOMUI_CELL_PREV;
	}

	celli = fromi;

	for(guard=0;guard<_ZOOMUI_CELL_COUNT;guard++)
	{
		celli += dir;

		if (celli < 0)
		{
			celli = _ZOOMUI_CELL_COUNT - 1;
		}

		if (celli >= _ZOOMUI_CELL_COUNT)
		{
			celli = 0;
		}

		if (_zoomui_is_button_cell(celli))
		{
			return celli;
		}
	}

	return fromi;
}

// per button enabled state. the pill mirrors the container: disabling
// the zoomui window dims every button and blocks their commands while
// keeping the same hit area so clicks never fall through to the image
// by accident. (the viewer never disables single pill buttons today;
// the path exists so a future per command gate has a stable visual.)
static int _zoomui_is_button_disabled(int celli)
{
	(void)celli;

	if (_zoomui_hwnd)
	{
		if (!IsWindowEnabled(_zoomui_hwnd))
		{
			return 1;
		}
	}

	return 0;
}

// repaint the whole pill. the window is at most a few hundred pixels
// wide: a full repaint on hover / press change is cheaper than tracking
// per button regions and avoids seam artifacts where the capsule arcs
// meet the tray.
static void _zoomui_invalidate(void)
{
	if (_zoomui_hwnd)
	{
		InvalidateRect(_zoomui_hwnd,0,FALSE);
	}
}

static void _zoomui_clear_hover_press(int invalidate)
{
	_zoomui_hot_index = -1;
	_zoomui_pressed_index = -1;

	if ((invalidate) && (_zoomui_hwnd))
	{
		InvalidateRect(_zoomui_hwnd,0,FALSE);
	}
}

static void _zoomui_track_leave(HWND hwnd)
{
	TRACKMOUSEEVENT tme;

	os_zero_memory(&tme,sizeof(tme));

	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = hwnd;

	TrackMouseEvent(&tme);
}

// fire a pill button: send the real command to the main window with the
// same BN_CLICKED notification a BUTTON would have sent, then return
// focus to the viewer so keyboard shortcuts keep working. the
// play/pause cell resolves its command from the slideshow state: play
// when stopped, pause when running.
static void _zoomui_fire_button(int celli)
{
	int command_id;

	if ((!_zoomui_hwnd) || (!_zoomui_parent_hwnd))
	{
		return;
	}

	if (!_zoomui_is_button_cell(celli))
	{
		return;
	}

	if (_zoomui_is_button_disabled(celli))
	{
		return;
	}

	if (celli == _ZOOMUI_CELL_PLAYPAUSE)
	{
		command_id = _viv_is_slideshow ? VIV_ID_SLIDESHOW_PAUSE_ONLY : VIV_ID_SLIDESHOW_PLAY_ONLY;
	}
	else
	{
		command_id = _zoomui_cell_command_ids[celli];
	}

	SendMessage(_zoomui_parent_hwnd,WM_COMMAND,MAKEWPARAM(command_id,BN_CLICKED),(LPARAM)_zoomui_hwnd);

	// return focus to the viewer so keyboard shortcuts keep working.
	SetFocus(_zoomui_parent_hwnd);

	// the command may have zoomed or toggled the slideshow: sync the
	// percent text and the play/pause face right away.
	_zoomui_poll_state();
}

static void _zoomui_tooltip_destroy(void)
{
	if (_zoomui_tooltip_hwnd)
	{
		DestroyWindow(_zoomui_tooltip_hwnd);

		_zoomui_tooltip_hwnd = 0;
	}
}

// the tooltip text for one cell. the play/pause cell switches between
// the play and pause strings with the slideshow state, the rest are
// fixed. returns 0 for the cells that carry no tooltip (separator and
// percent text cell: no hover, no tip).
static localization_id_t _zoomui_tooltip_id_for_cell(int celli)
{
	switch (celli)
	{
		case _ZOOMUI_CELL_PREV:
			return LOCALIZATION_ID_ZOOMUI_TOOLTIP_PREV;

		case _ZOOMUI_CELL_PLAYPAUSE:
			return _viv_is_slideshow ? LOCALIZATION_ID_ZOOMUI_TOOLTIP_PAUSE : LOCALIZATION_ID_ZOOMUI_TOOLTIP_PLAY;

		case _ZOOMUI_CELL_NEXT:
			return LOCALIZATION_ID_ZOOMUI_TOOLTIP_NEXT;

		case _ZOOMUI_CELL_ZOOMOUT:
			return LOCALIZATION_ID_ZOOMUI_TOOLTIP_ZOOM_OUT;

		case _ZOOMUI_CELL_ZOOMIN:
			return LOCALIZATION_ID_ZOOMUI_TOOLTIP_ZOOM_IN;
	}

	return (localization_id_t)0;
}

// create the single rect based tooltip for the pill. one tool per live
// button cell (uId = cell index), TTF_SUBCLASS so the tooltip relays
// the mouse messages itself. the separator and the percent cell get no
// tool.
static void _zoomui_tooltip_create(void)
{
	int i;

	if ((!_zoomui_hwnd) || (_zoomui_tooltip_hwnd))
	{
		return;
	}

	_zoomui_tooltip_hwnd = os_CreateWindowEx(
		WS_EX_TOPMOST,
		TOOLTIPS_CLASSA,
		"",
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
		_zoomui_hwnd,
		0,
		os_hinstance,
		NULL);

	if (_zoomui_tooltip_hwnd)
	{
		for(i=0;i<_ZOOMUI_CELL_COUNT;i++)
		{
			TOOLINFOW ti;
			wchar_t wbuf[STRING_SIZE];
			localization_id_t id;

			id = _zoomui_tooltip_id_for_cell(i);

			if (!id)
			{
				continue;
			}

			os_zero_memory(&ti,sizeof(ti));

			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_SUBCLASS;
			ti.hwnd = _zoomui_hwnd;
			ti.uId = (UINT_PTR)i;
			ti.rect = _zoomui_cell_rects[i];
			ti.hinst = 0;
			string_copy_utf8_string(wbuf,localization_get_string(id));
			ti.lpszText = wbuf;

			SendMessage(_zoomui_tooltip_hwnd,TTM_ADDTOOLW,0,(LPARAM)&ti);
		}

		SendMessage(_zoomui_tooltip_hwnd,TTM_ACTIVATE,_zoomui_visible_wanted ? TRUE : FALSE,0);

		// match the tooltip colors to the current palette.
		_zoomui_apply_tooltip_colors();
	}
}

// refresh one tool: the rect follows the current cell rect and the text
// is re-fetched from localization (the play/pause cell resolves its
// string fresh each time).
static void _zoomui_tooltip_update_rects(void)
{
	int i;

	if (!_zoomui_tooltip_hwnd)
	{
		return;
	}

	for(i=0;i<_ZOOMUI_CELL_COUNT;i++)
	{
		TOOLINFOW ti;
		wchar_t wbuf[STRING_SIZE];
		localization_id_t id;

		id = _zoomui_tooltip_id_for_cell(i);

		if (!id)
		{
			continue;
		}

		os_zero_memory(&ti,sizeof(ti));

		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = _zoomui_hwnd;
		ti.uId = (UINT_PTR)i;
		ti.rect = _zoomui_cell_rects[i];
		ti.hinst = 0;
		string_copy_utf8_string(wbuf,localization_get_string(id));
		ti.lpszText = wbuf;

		SendMessage(_zoomui_tooltip_hwnd,TTM_SETTOOLINFOW,0,(LPARAM)&ti);
	}
}

// refresh one tool's text (language change, play state flip).
static void _zoomui_tooltip_update_text(int celli)
{
	TOOLINFOW ti;
	wchar_t wbuf[STRING_SIZE];
	localization_id_t id;

	if (!_zoomui_tooltip_hwnd)
	{
		return;
	}

	id = _zoomui_tooltip_id_for_cell(celli);

	if (!id)
	{
		return;
	}

	os_zero_memory(&ti,sizeof(ti));

	ti.cbSize = sizeof(ti);
	ti.hwnd = _zoomui_hwnd;
	ti.uId = (UINT_PTR)celli;
	ti.hinst = 0;
	string_copy_utf8_string(wbuf,localization_get_string(id));
	ti.lpszText = wbuf;

	SendMessage(_zoomui_tooltip_hwnd,TTM_UPDATETIPTEXTW,0,(LPARAM)&ti);
}

// the percent / play state refresh. zoom changes that never touch this
// window (mouse wheel, pinch, zoom pane editor) leave no message trace
// here, so a short poll keeps the text cell honest; the pill's own
// buttons call this directly after firing their command.
static void _zoomui_poll_state(void)
{
	int percent;
	int slideshow;

	if (!_zoomui_hwnd)
	{
		return;
	}

	if (!IsWindowVisible(_zoomui_hwnd))
	{
		return;
	}

	percent = _zoomui_percent();
	slideshow = _viv_is_slideshow ? 1 : 0;

	if ((percent == _zoomui_pct_percent) && (slideshow == _zoomui_is_slideshow_cached))
	{
		return;
	}

	if (percent != _zoomui_pct_percent)
	{
		int old_wide;

		_zoomui_pct_percent = percent;

		old_wide = _zoomui_pct_wide;

		_zoomui_measure_pct_wide();

		// a wider or narrower text re-centers the whole row - but never
		// under an active press: the cells would shift away from the
		// cursor between two rapid clicks (rc.13). the release re-centers.
		if (_zoomui_pct_wide != old_wide)
		{
			if (_zoomui_pressed_index >= 0)
			{
				_zoomui_pct_recenter = 1;
			}
			else
			{
				_zoomui_place(_zoomui_area_wide,_zoomui_area_high);
			}
		}
	}

	if (slideshow != _zoomui_is_slideshow_cached)
	{
		_zoomui_is_slideshow_cached = slideshow;

		_zoomui_tooltip_update_text(_ZOOMUI_CELL_PLAYPAUSE);
	}

	_zoomui_invalidate();
}

void zoomui_init(HWND parent)
{
	_zoomui_parent_hwnd = parent;

	if (!_zoomui_is_registered)
	{
		os_RegisterClassEx(
			CS_DBLCLKS,
			_zoomui_proc,
			0,
			LoadCursor(NULL,IDC_ARROW),
			0,
			"_VIV_ZOOMUI",
			0);

		_zoomui_is_registered = 1;
	}

	if (!_zoomui_hwnd)
	{
		_zoomui_hwnd = os_CreateWindowEx(
			WS_EX_TOOLWINDOW|WS_EX_LAYERED,
			"_VIV_ZOOMUI",
			"",
			WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP,
			0,0,0,0,
			parent,(HMENU)VIV_ID_ZOOMUI,os_hinstance,NULL);

		if (_zoomui_hwnd)
		{
			// probe the layered child window support (windows 8+): if
			// alpha blending is refused the style is removed and the
			// bar hides without the fade.
			if (SetLayeredWindowAttributes(_zoomui_hwnd,0,_ZOOMUI_ALPHA_OPAQUE,LWA_ALPHA))
			{
				_zoomui_layered_ok = 1;
			}
			else
			{
				SetWindowLong(_zoomui_hwnd,GWL_EXSTYLE,GetWindowLong(_zoomui_hwnd,GWL_EXSTYLE) & ~WS_EX_LAYERED);

				_zoomui_layered_ok = 0;
			}
		}
	}

	// metrics, the percent cell width and the cell rects: the row is
	// identical in both modes, so one pass serves windowed and
	// fullscreen.
	_zoomui_calc_metrics();

	_zoomui_pct_percent = _zoomui_percent();
	_zoomui_is_slideshow_cached = _viv_is_slideshow ? 1 : 0;
	_zoomui_measure_pct_wide();

	_zoomui_update_cell_rects();

	_zoomui_tooltip_create();
}

void zoomui_kill(void)
{
	_zoomui_kill_timer();
	_zoomui_kill_poll_timer();

	if ((GetCapture()) && (_zoomui_hwnd) && (GetCapture() == _zoomui_hwnd))
	{
		ReleaseCapture();
	}

	_zoomui_tooltip_destroy();

	if (_zoomui_hwnd)
	{
		DestroyWindow(_zoomui_hwnd);

		_zoomui_hwnd = 0;
	}

	_zoomui_clear_hover_press(0);

	_zoomui_layered_ok = 0;
	_zoomui_alpha = _ZOOMUI_ALPHA_OPAQUE;
	_zoomui_alpha_target = _ZOOMUI_ALPHA_OPAQUE;
	_zoomui_visible_wanted = 0;
	_zoomui_pct_percent = 100;
	_zoomui_is_slideshow_cached = 0;
	_zoomui_pct_wide = 0;
	_zoomui_area_wide = 0;
	_zoomui_area_high = 0;

	_zoomui_parent_hwnd = 0;
}

// tint the tooltip control with the palette: comctl tooltips have no dark
// theme of their own, the colors are set by message. the dark face rides
// the theme tokens so the tip matches the themed menus.
static void _zoomui_apply_tooltip_colors(void)
{
	if (_zoomui_tooltip_hwnd)
	{
		if (_zoomui_dark)
		{
			SendMessage(_zoomui_tooltip_hwnd,TTM_SETTIPBKCOLOR,viv_theme_color(VIV_TK_FACE),0);
			SendMessage(_zoomui_tooltip_hwnd,TTM_SETTIPTEXTCOLOR,viv_theme_color(VIV_TK_TEXT),0);
		}
		else
		{
			SendMessage(_zoomui_tooltip_hwnd,TTM_SETTIPBKCOLOR,GetSysColor(COLOR_INFOBK),0);
			SendMessage(_zoomui_tooltip_hwnd,TTM_SETTIPTEXTCOLOR,GetSysColor(COLOR_INFOTEXT),0);
		}
	}
}

// switch the palette between light and dark. called after creating and
// whenever the app dark mode or the windows theme changes.
void zoomui_set_dark(int dark)
{
	int old_wide;

	if (_zoomui_dark != (dark ? 1 : 0))
	{
		_zoomui_dark = dark ? 1 : 0;

		// the glyph colors are baked into the cached icons.
		glyphs_flush_cache();

		_zoomui_invalidate();
	}

	// the theme change that delivered this call may have swapped the
	// menu font: re-measure the percent cell so the row width stays
	// right.
	old_wide = _zoomui_pct_wide;

	_zoomui_measure_pct_wide();

	if (_zoomui_pct_wide != old_wide)
	{
		_zoomui_place(_zoomui_area_wide,_zoomui_area_high);
	}

	_zoomui_invalidate();

	// the tooltip control may exist before the first palette flip and a
	// fresh control always starts light: tint it on every call.
	_zoomui_apply_tooltip_colors();
}

int zoomui_is_created(void)
{
	return _zoomui_hwnd ? 1 : 0;
}

void zoomui_localize(void)
{
	int i;

	// refresh the tooltip texts after the language has changed. the
	// play/pause cell resolves its string fresh, so one pass covers
	// every button.
	for(i=0;i<_ZOOMUI_CELL_COUNT;i++)
	{
		_zoomui_tooltip_update_text(i);
	}
}

// switch between the windowed pill and the fullscreen overlay bar. the
// row layout is the same in both modes: only the idle fade differs.
// viv.c repositions the container right after this through the regular
// on_size layout.
void zoomui_set_fullscreen(int fullscreen)
{
	if (_zoomui_is_fullscreen != (fullscreen ? 1 : 0))
	{
		_zoomui_is_fullscreen = fullscreen ? 1 : 0;

		if (_zoomui_hwnd)
		{
			if ((GetCapture() == _zoomui_hwnd) && (_zoomui_hwnd))
			{
				ReleaseCapture();
			}

			_zoomui_clear_hover_press(0);

			_zoomui_invalidate();

			// a mode switch is activity: restart the idle clock.
			_zoomui_last_activity = GetTickCount();

			if ((IsWindowVisible(_zoomui_hwnd)) && (_zoomui_autohide_enabled()))
			{
				_zoomui_ensure_timer();
			}
		}
	}
}

// user input arrived (mouse motion, key press or a command): keep the
// overlay awake, and show it again with a fade in when it was hidden.
// the percent / play state sync rides along so the text cell stays
// fresh in windowed mode too.
void zoomui_activity(void)
{
	if ((!_zoomui_hwnd) || (!_zoomui_visible_wanted))
	{
		return;
	}

	_zoomui_last_activity = GetTickCount();

	if (!_zoomui_autohide_enabled())
	{
		// windowed row: no fade, just keep the text cell honest.
		_zoomui_poll_state();

		return;
	}

	_zoomui_alpha_target = _ZOOMUI_ALPHA_OPAQUE;

	if (!IsWindowVisible(_zoomui_hwnd))
	{
		_zoomui_alpha = 0;

		ShowWindow(_zoomui_hwnd,SW_SHOW);

		_zoomui_set_alpha(0);

		_zoomui_ensure_poll_timer();
	}

	_zoomui_ensure_timer();

	_zoomui_poll_state();
}

void zoomui_show(int show)
{
	_zoomui_visible_wanted = show ? 1 : 0;

	if (!_zoomui_hwnd)
	{
		return;
	}

	if (_zoomui_visible_wanted)
	{
		if (!IsWindowVisible(_zoomui_hwnd))
		{
			// no WM_MOUSELEAVE arrives when showing under the cursor.
			_zoomui_clear_hover_press(0);

			// the fade in starts from fully transparent (or is skipped
			// when the layered child support is missing).
			_zoomui_alpha = _zoomui_layered_ok ? 0 : _ZOOMUI_ALPHA_OPAQUE;
			_zoomui_alpha_target = _ZOOMUI_ALPHA_OPAQUE;
			_zoomui_last_activity = GetTickCount();

			ShowWindow(_zoomui_hwnd,SW_SHOW);

			_zoomui_set_alpha(_zoomui_alpha);

			_zoomui_ensure_poll_timer();
		}
		else
		{
			// already visible: cancel any in flight fade out.
			_zoomui_alpha_target = _ZOOMUI_ALPHA_OPAQUE;
		}

		if (_zoomui_autohide_enabled())
		{
			_zoomui_ensure_timer();
		}

		if (_zoomui_tooltip_hwnd)
		{
			SendMessage(_zoomui_tooltip_hwnd,TTM_ACTIVATE,TRUE,0);
		}

		// first show after a mode or language change: sync the text
		// cell and the tooltip face before anything paints stale.
		_zoomui_poll_state();
	}
	else
	{
		// no WM_MOUSELEAVE arrives when hiding under the cursor: drop
		// any capture first so the viewer never keeps a stale grab.
		if (GetCapture() == _zoomui_hwnd)
		{
			ReleaseCapture();
		}

		// no WM_MOUSELEAVE arrives when hiding under the cursor.
		_zoomui_clear_hover_press(0);

		_zoomui_kill_timer();
		_zoomui_kill_poll_timer();
		_zoomui_alpha = 0;
		_zoomui_alpha_target = 0;

		ShowWindow(_zoomui_hwnd,SW_HIDE);

		if (_zoomui_tooltip_hwnd)
		{
			SendMessage(_zoomui_tooltip_hwnd,TTM_ACTIVATE,FALSE,0);
		}
	}
}

// the fade / idle tick.
static void _zoomui_on_timer(void)
{
	DWORD now;
	int idle;

	if (!_zoomui_hwnd)
	{
		_zoomui_kill_timer();

		return;
	}

	if (!_zoomui_autohide_enabled())
	{
		// windowed pill or auto hide disabled: stay opaque.
		if (_zoomui_alpha != _ZOOMUI_ALPHA_OPAQUE)
		{
			_zoomui_alpha = _ZOOMUI_ALPHA_OPAQUE;

			_zoomui_set_alpha(_zoomui_alpha);
		}

		_zoomui_kill_timer();

		return;
	}

	now = GetTickCount();
	idle = (now - _zoomui_last_activity) >= _ZOOMUI_IDLE_MS;

	if (idle)
	{
		_zoomui_alpha_target = 0;
	}

	if (!_zoomui_layered_ok)
	{
		// windows 7: no layered child windows, hide without a fade.
		if (idle)
		{
			if (GetCapture() == _zoomui_hwnd)
			{
				ReleaseCapture();
			}

			_zoomui_clear_hover_press(0);

			ShowWindow(_zoomui_hwnd,SW_HIDE);

			_zoomui_kill_timer();
			_zoomui_kill_poll_timer();
		}

		return;
	}

	if (_zoomui_alpha != _zoomui_alpha_target)
	{
		if (_zoomui_alpha > _zoomui_alpha_target)
		{
			_zoomui_alpha -= _ZOOMUI_ALPHA_STEP;

			if (_zoomui_alpha < _zoomui_alpha_target)
			{
				_zoomui_alpha = _zoomui_alpha_target;
			}
		}
		else
		{
			_zoomui_alpha += _ZOOMUI_ALPHA_STEP;

			if (_zoomui_alpha > _zoomui_alpha_target)
			{
				_zoomui_alpha = _zoomui_alpha_target;
			}
		}

		_zoomui_set_alpha(_zoomui_alpha);

		// a fully transparent layered window still eats mouse clicks:
		// hide it for real once the fade has finished.
		if (_zoomui_alpha == 0)
		{
			if (GetCapture() == _zoomui_hwnd)
			{
				ReleaseCapture();
			}

			_zoomui_clear_hover_press(0);

			ShowWindow(_zoomui_hwnd,SW_HIDE);

			_zoomui_kill_timer();
			_zoomui_kill_poll_timer();

			return;
		}
	}
}

// wide,high = the image area of the parent client.
// (status bar and toolbar space already excluded)
void zoomui_layout(int wide,int high)
{
	if (!_zoomui_hwnd)
	{
		return;
	}

	_zoomui_area_wide = wide;
	_zoomui_area_high = high;

	_zoomui_calc_metrics();

	_zoomui_measure_pct_wide();

	_zoomui_place(wide,high);

	_zoomui_invalidate();
}

static void _zoomui_draw_button(HDC hdc,const RECT *rect,int celli,int is_pressed,int is_disabled,int is_hot,int has_focus)
{
	RECT fill_rect;
	HBRUSH brush;
	HPEN pen;
	HPEN old_pen;
	HGDIOBJ old_brush;
	int offset;

	offset = 0;

	if ((is_pressed) && (!is_disabled))
	{
		offset = 1;
	}

	CopyRect(&fill_rect,rect);

	// the button body is a capsule: the corner ellipse is the full
	// button height, so each end is a true semicircle and the row reads
	// as one rail of stadium cells instead of the square cornered
	// blocks (which sat next to the flat toolbar like a patch from
	// another toolkit). roundrect fills and outlines the same
	// silhouette in one call.
	{
		COLORREF fill_color;

		if ((is_disabled))
		{
			fill_color = viv_theme_color(VIV_TK_FACE);
		}
		else if ((is_pressed) && (is_hot))
		{
			fill_color = viv_theme_color(VIV_TK_DOWN);
		}
		else if (is_hot)
		{
			fill_color = viv_theme_color(VIV_TK_HOVER);
		}
		else
		{
			fill_color = viv_theme_color(VIV_TK_FACE);
		}

		brush = CreateSolidBrush(fill_color);
		pen = CreatePen(PS_SOLID,1,viv_theme_color((is_pressed) && (is_hot) ? VIV_TK_DOWN : VIV_TK_LINE));

		old_pen = SelectObject(hdc,pen);
		old_brush = SelectObject(hdc,brush);

		{
			int corner;

			// full stadium: the ellipse diameters equal the button
			// height so the end radius is exactly half the height at
			// every dpi. clamp to at least 2px so a degenerate rect
			// never collapses into sharp corners.
			corner = fill_rect.bottom - fill_rect.top;

			if (corner < 2)
			{
				corner = 2;
			}

			RoundRect(hdc,fill_rect.left,fill_rect.top,fill_rect.right,fill_rect.bottom,corner,corner);
		}

		SelectObject(hdc,old_brush);
		SelectObject(hdc,old_pen);
		DeleteObject(pen);
		DeleteObject(brush);
	}

	if (is_disabled)
	{
		SetTextColor(hdc,viv_theme_color(VIV_TK_TEXTOFF));
	}
	else
	{
		SetTextColor(hdc,viv_theme_color(VIV_TK_TEXT));
	}

	SetBkMode(hdc,TRANSPARENT);

	// the vector glyphs make the buttons unmistakable.
	_zoomui_draw_icon(hdc,rect,celli,offset,is_disabled);

	// keyboard focus: a dotted ring inside the hot capsule.
	if ((has_focus) && (is_hot) && (!is_disabled))
	{
		RECT focus_rect;

		CopyRect(&focus_rect,rect);

		InflateRect(&focus_rect,-4,-4);

		if ((focus_rect.right > focus_rect.left) && (focus_rect.bottom > focus_rect.top))
		{
			DrawFocusRect(hdc,&focus_rect);
		}
	}
}

// the glyph id for one button cell: the play/pause cell follows the
// slideshow state, the rest are fixed.
static int _zoomui_cell_glyph_id(int celli)
{
	if (celli == _ZOOMUI_CELL_PLAYPAUSE)
	{
		return _viv_is_slideshow ? GLYPH_PAUSE : GLYPH_PLAY;
	}

	return _zoomui_cell_glyph_ids[celli];
}

// draw the glyph icon, centered in the button.
static void _zoomui_draw_icon(HDC hdc,const RECT *rect,int celli,int offset,int is_disabled)
{
	int wide;
	int high;
	int size;
	HICON icon;

	wide = rect->right - rect->left;
	high = rect->bottom - rect->top;

	// the button is wider than tall, so the height drives the glyph box and
	// the sides absorb the extra width: the padding stays even on all four
	// sides of the glyph at every dpi (the old min() box leaned on the
	// width and squeezed the glyphs against the capsule arcs).
	size = high - 2 * (high / 9);

	if (size < 8)
	{
		size = 8;
	}

	icon = glyphs_icon(_zoomui_cell_glyph_id(celli),_zoomui_dark,size);

	if (icon)
	{
		int x;
		int y;

		x = ((wide - size) / 2) + offset;
		y = ((high - size) / 2) + offset;

		if (is_disabled)
		{
			// embossed disabled glyph through the stock raster op: no
			// extra cache, same theme aware icon.
			DrawState(hdc,NULL,NULL,(LPARAM)icon,0,x + rect->left,y + rect->top,size,size,DST_ICON | DSS_DISABLED);
		}
		else
		{
			DrawIconEx(hdc,x + rect->left,y + rect->top,icon,size,size,0,NULL,DI_NORMAL);
		}
	}
}

// draw the separator: a hairline rule centered vertically inside its
// cell, splitting the navigation cells from the zoom cells.
static void _zoomui_draw_separator(HDC hdc)
{
	RECT fill_rect;
	HBRUSH brush;
	int top;

	fill_rect = _zoomui_cell_rects[_ZOOMUI_CELL_SEP];

	top = fill_rect.top + ((_zoomui_cell_high - _zoomui_sep_high) / 2);

	fill_rect.top = top;
	fill_rect.bottom = top + _zoomui_sep_high;

	brush = CreateSolidBrush(viv_theme_color(VIV_TK_LINE));

	FillRect(hdc,&fill_rect,brush);

	DeleteObject(brush);
}

// draw the percent text cell: plain text in the menu font, centered in
// the cell. no fill, no border, no hover: the cell is a readout.
static void _zoomui_draw_percent(HDC hdc,const RECT *rect)
{
	wchar_t wbuf[STRING_SIZE];
	RECT text_rect;
	HFONT font;
	HFONT old_font;

	_zoomui_build_pct_text(wbuf,_zoomui_pct_percent);

	CopyRect(&text_rect,rect);

	SetBkMode(hdc,TRANSPARENT);

	if (_zoomui_is_button_disabled(_ZOOMUI_CELL_PCT))
	{
		SetTextColor(hdc,viv_theme_color(VIV_TK_TEXTOFF));
	}
	else
	{
		SetTextColor(hdc,viv_theme_color(VIV_TK_TEXT));
	}

	font = _viv_menu_font();
	old_font = 0;

	if (font)
	{
		old_font = SelectObject(hdc,font);
	}

	DrawTextW(hdc,wbuf,-1,&text_rect,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);

	if (old_font)
	{
		SelectObject(hdc,old_font);
	}
}

static LRESULT CALLBACK _zoomui_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
		case WM_NCHITTEST:
		{
			POINT pt;
			RECT client_rect;
			int hit;

			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);

			ScreenToClient(hwnd,&pt);

			GetClientRect(hwnd,&client_rect);

			if (!PtInRect(&client_rect,pt))
			{
				return HTTRANSPARENT;
			}

			// a fully transparent (fading out) or unwanted pill never
			// eats clicks: let them fall through to the viewer canvas.
			if ((!_zoomui_visible_wanted) || (_zoomui_alpha == 0) || (!IsWindowVisible(hwnd)))
			{
				return HTTRANSPARENT;
			}

			// only the button and percent cells hit: the stadium
			// corners, the tray gaps and the separator are click
			// through to the image behind. the percent cell swallows
			// clicks without acting on them (no hover, no click).
			hit = _zoomui_cell_at(pt.x,pt.y);

			if ((hit >= 0) && (hit != _ZOOMUI_CELL_SEP))
			{
				return HTCLIENT;
			}

			return HTTRANSPARENT;
		}

		case WM_MOUSEMOVE:
		{
			int x;
			int y;
			int hit;
			int hot;

			x = GET_X_LPARAM(lParam);
			y = GET_Y_LPARAM(lParam);

			hit = _zoomui_cell_at(x,y);

			// hover is a button-only state: the percent cell never
			// lights up.
			hot = _zoomui_is_button_cell(hit) ? hit : -1;

			if (GetCapture() == hwnd)
			{
				// captured drag: the hot capsule follows the cursor so
				// the press visual tracks, the command still fires only
				// on release over the same button.
				if (hot != _zoomui_hot_index)
				{
					_zoomui_hot_index = hot;

					_zoomui_invalidate();
				}
			}
			else
			{
				if (hot != _zoomui_hot_index)
				{
					_zoomui_hot_index = hot;

					_zoomui_invalidate();
				}

				_zoomui_track_leave(hwnd);
			}

			// hovering the bar counts as activity (idle fade timer).
			if (hit >= 0)
			{
				zoomui_activity();
			}

			return 0;
		}

		case WM_MOUSELEAVE:
		{
			// while captured the press owns the cursor: keep the hot
			// state until the button up resolves it.
			if (GetCapture() != hwnd)
			{
				if (_zoomui_hot_index != -1)
				{
					_zoomui_hot_index = -1;

					_zoomui_invalidate();
				}
			}

			return 0;
		}

		case WM_LBUTTONDOWN:
		{
			int x;
			int y;
			int hit;

			x = GET_X_LPARAM(lParam);
			y = GET_Y_LPARAM(lParam);

			hit = _zoomui_cell_at(x,y);

			// only button cells take the focus and the press: a click
			// on the percent text cell is swallowed, silent and focus
			// free.
			if (_zoomui_is_button_cell(hit))
			{
				SetFocus(hwnd);

				if (!_zoomui_is_button_disabled(hit))
				{
					SetCapture(hwnd);

					_zoomui_pressed_index = hit;
					_zoomui_hot_index = hit;

					_zoomui_invalidate();

					// hovering the bar counts as activity (idle fade timer).
					zoomui_activity();

					// dismiss the hover tip while pressed.
					if (_zoomui_tooltip_hwnd)
					{
						SendMessage(_zoomui_tooltip_hwnd,TTM_POP,0,0);
					}
				}
			}

			return 0;
		}

		case WM_LBUTTONDBLCLK:
		{
			int x;
			int y;
			int hit;

			x = GET_X_LPARAM(lParam);
			y = GET_Y_LPARAM(lParam);

			hit = _zoomui_cell_at(x,y);

			if ((_zoomui_is_button_cell(hit)) && (!_zoomui_is_button_disabled(hit)))
			{
				SetCapture(hwnd);

				_zoomui_pressed_index = hit;
				_zoomui_hot_index = hit;

				_zoomui_invalidate();

				zoomui_activity();

				if (_zoomui_tooltip_hwnd)
				{
					SendMessage(_zoomui_tooltip_hwnd,TTM_POP,0,0);
				}
			}

			return 0;
		}

		case WM_LBUTTONUP:
		{
			int x;
			int y;
			int hit;
			int down;
			int hot;

			x = GET_X_LPARAM(lParam);
			y = GET_Y_LPARAM(lParam);

			hit = _zoomui_cell_at(x,y);
			hot = _zoomui_is_button_cell(hit) ? hit : -1;
			down = _zoomui_pressed_index;

			if (GetCapture() == hwnd)
			{
				ReleaseCapture();
			}

			_zoomui_pressed_index = -1;
			_zoomui_hot_index = hot;

			_zoomui_invalidate();

			if ((down >= 0) && (hit == down))
			{
				_zoomui_fire_button(down);
			}

			// rc.13: the deferred re-center lands now that the press is
			// over (the row sat still through the rapid clicks).
			if (_zoomui_pct_recenter)
			{
				_zoomui_pct_recenter = 0;

				_zoomui_place(_zoomui_area_wide,_zoomui_area_high);
			}

			return 0;
		}

		case WM_CAPTURECHANGED:
		{
			// the capture went elsewhere (hide, menu, another press):
			// drop the press visual. the button up already fired before
			// releasing, so this path never double fires.
			if ((HWND)lParam != hwnd)
			{
				if (_zoomui_pressed_index != -1)
				{
					_zoomui_pressed_index = -1;

					_zoomui_invalidate();
				}
			}

			break;
		}

		case WM_CANCELMODE:
		{
			if (GetCapture() == hwnd)
			{
				ReleaseCapture();
			}

			if ((_zoomui_pressed_index != -1) || (_zoomui_hot_index != -1))
			{
				// keep the hover under the cursor: the next mousemove
				// restores it; the press is always dropped.
				_zoomui_pressed_index = -1;

				if (GetCapture() != hwnd)
				{
					_zoomui_hot_index = -1;
				}

				_zoomui_invalidate();
			}

			break;
		}

		case WM_SETFOCUS:
		{
			// tabbed in with no hover yet: highlight the first capsule
			// so space / enter has a visible target.
			if ((_zoomui_hot_index == -1) && (_zoomui_is_button_cell(_ZOOMUI_CELL_PREV)))
			{
				_zoomui_hot_index = _ZOOMUI_CELL_PREV;

				_zoomui_invalidate();
			}
			else
			{
				_zoomui_invalidate();
			}

			return 0;
		}

		case WM_KILLFOCUS:
		{
			_zoomui_invalidate();

			return 0;
		}

		case WM_GETDLGCODE:
		{
			// arrows move between capsules, chars feed space / enter;
			// tab itself stays free so focus can leave the pill.
			return DLGC_WANTARROWS | DLGC_WANTCHARS;
		}

		case WM_KEYDOWN:
		{
			int hot;

			hot = _zoomui_hot_index;

			if ((hot < 0) && (_zoomui_is_button_cell(_ZOOMUI_CELL_PREV)))
			{
				hot = _ZOOMUI_CELL_PREV;
			}

			switch ((int)wParam)
			{
				case VK_LEFT:
				case VK_UP:
				{
					_zoomui_hot_index = _zoomui_step_button_cell(_zoomui_hot_index,-1);

					_zoomui_invalidate();

					zoomui_activity();

					return 0;
				}

				case VK_RIGHT:
				case VK_DOWN:
				{
					_zoomui_hot_index = _zoomui_step_button_cell(_zoomui_hot_index,1);

					_zoomui_invalidate();

					zoomui_activity();

					return 0;
				}

				case VK_TAB:
				{
					// shift+tab walks backwards, plain tab forwards.
					if (GetKeyState(VK_SHIFT) < 0)
					{
						_zoomui_hot_index = _zoomui_step_button_cell(_zoomui_hot_index,-1);
					}
					else
					{
						_zoomui_hot_index = _zoomui_step_button_cell(_zoomui_hot_index,1);
					}

					_zoomui_invalidate();

					zoomui_activity();

					return 0;
				}

				case VK_SPACE:
				case VK_RETURN:
				{
					if ((hot >= 0) && (!_zoomui_is_button_disabled(hot)))
					{
						_zoomui_fire_button(hot);
					}

					return 0;
				}
			}

			break;
		}

		case WM_ENABLE:
		{
			_zoomui_invalidate();

			break;
		}

		case WM_DPICHANGED:
		{
			// the main window handler updates os_logical_* and rebuilds
			// the icon caches; refresh the metrics and the container for
			// the new scale so this window is correct whenever it paints
			// next. the parent on_size repositions right after.
			int container_wide;
			int container_high;

			_zoomui_calc_metrics();

			_zoomui_measure_pct_wide();
			_zoomui_update_cell_rects();

			container_wide = _zoomui_row_wide() + (_zoomui_margin * 2);
			container_high = _zoomui_cell_high + (_zoomui_margin * 2);

			SetWindowPos(hwnd,0,0,0,container_wide,container_high,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

			_zoomui_tooltip_update_rects();

			_zoomui_invalidate();

			return 0;
		}

		case WM_TIMER:
		{
			if ((int)wParam == _ZOOMUI_TIMER_ID)
			{
				_zoomui_on_timer();

				return 0;
			}

			if ((int)wParam == _ZOOMUI_PCT_TIMER_ID)
			{
				_zoomui_poll_state();

				return 0;
			}

			break;
		}

		case WM_ERASEBKGND:
		{
			// all painting happens in WM_PAINT (tray + capsules in one
			// pass): skipping the erase avoids flicker on hover.
			return 1;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT client_rect;
			int has_focus;
			int i;

			hdc = BeginPaint(hwnd,&ps);

			if (hdc)
			{
				GetClientRect(hwnd,&client_rect);

				has_focus = (GetFocus() == hwnd) ? 1 : 0;

				// the pill tray: one stadium in the capsule face color
				// with a hairline border. the capsule buttons float on
				// it with gaps around them, so the tray reads as a soft
				// rail under the row instead of the old raised square
				// block with its double edge.
				{
					HBRUSH brush;
					HPEN pen;
					HPEN old_pen;
					HGDIOBJ old_brush;
					int corner;

					brush = CreateSolidBrush(viv_theme_color(VIV_TK_FACE));
					pen = CreatePen(PS_SOLID,1,viv_theme_color(VIV_TK_LINE));

					old_pen = SelectObject(hdc,pen);
					old_brush = SelectObject(hdc,brush);

					// full stadium: the ellipse diameters equal the tray
					// height so the ends are true semicircles at every dpi.
					corner = client_rect.bottom - client_rect.top;

					if (corner < 2)
					{
						corner = 2;
					}

					RoundRect(hdc,client_rect.left,client_rect.top,client_rect.right,client_rect.bottom,corner,corner);

					SelectObject(hdc,old_brush);
					SelectObject(hdc,old_pen);
					DeleteObject(pen);
					DeleteObject(brush);
				}

				for(i=0;i<_ZOOMUI_CELL_COUNT;i++)
				{
					if (i == _ZOOMUI_CELL_SEP)
					{
						_zoomui_draw_separator(hdc);
					}
					else
					if (i == _ZOOMUI_CELL_PCT)
					{
						_zoomui_draw_percent(hdc,&_zoomui_cell_rects[i]);
					}
					else
					{
						int is_pressed;
						int is_disabled;
						int is_hot;

						is_pressed = (_zoomui_pressed_index == i) ? 1 : 0;
						is_disabled = _zoomui_is_button_disabled(i);
						is_hot = (_zoomui_hot_index == i) ? 1 : 0;

						_zoomui_draw_button(hdc,&_zoomui_cell_rects[i],i,is_pressed,is_disabled,is_hot,has_focus);
					}
				}
			}

			EndPaint(hwnd,&ps);

			return 0;
		}
	}

	return DefWindowProc(hwnd,msg,wParam,lParam);
}
