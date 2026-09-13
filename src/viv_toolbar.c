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
// viv_toolbar.c - the toolbar, remade: a fully self drawn command strip.
//
// The comctl toolbar needed an image list rebuilt on every theme and dpi
// change, its button widths re-pinned after every re-metric, and it always
// measured from the light palette with the dark chrome patched on around
// it. this module owns the strip end to end, cloned from the menubar
// remake: one child window of a class registered here, one wm_paint that
// draws the face, the top edge, the hover and the press, and the glyph
// icons drawn from the vector set at the exact size in the current theme
// color. every button carries an icon plus its label at the menu font, the
// enabled states follow the old toolbar rule (the no image gate of the
// menus included), and the commands fire as bn_clicked wm_command messages
// to the main window. when the window is too narrow, whole groups (a
// separator with the buttons to its right) hide from the right until only
// the open button is left - no wrapping, no overflow chevron.
#include "viv.h"
#include "viv_state.h"
#include "viv_chrome.h"
#include "viv_render.h"
#include "viv_menubar.h"
#include "viv_toolbar.h"

// not defined in older sdks.
#ifndef BN_CLICKED
#define BN_CLICKED 0
#endif

// design metrics in 96 dpi units, scaled through the dip macros below.
#define _VIV_TOOLBAR_BAR_HIGH 40 // the strip height.
#define _VIV_TOOLBAR_ICON_SIZE 20 // the glyph box.
#define _VIV_TOOLBAR_ICON_TEXT_GAP 6 // between the glyph and the label.
#define _VIV_TOOLBAR_BUTTON_PAD 12 // inside the button, left and right.
#define _VIV_TOOLBAR_BUTTON_GAP 8 // between the buttons.
#define _VIV_TOOLBAR_SEP_HIGH 16 // the separator line height.
#define _VIV_TOOLBAR_RADIUS 4 // the hover face corner radius.

// the strip content: fifteen slots, ten buttons and five separators. the
// groups are the overflow units - a separator carries the group of the
// buttons to its right, so a group always hides with its separator and no
// dangling line is left behind.
// open | prev next playpause | 1to1 bestfit | zoom out zoom in | rotate | info.
#define _VIV_TOOLBAR_ITEM_COUNT 15
#define _VIV_TOOLBAR_GROUP_MAX 5
#define _VIV_TOOLBAR_ITEM_PLAY 4 // the play / pause face slot.

// item types.
#define _VIV_TOOLBAR_TYPE_BUTTON 0
#define _VIV_TOOLBAR_TYPE_SEP 1

typedef struct _viv_toolbar_item_s
{
	int type; // _VIV_TOOLBAR_TYPE_*.
	int command_id; // the wm_command id (separators carry 0).
	int glyph_play; // the glyph of the idle face.
	int glyph_pause; // the glyph of the playing face.
	localization_id_t localization_play; // the idle label.
	localization_id_t localization_pause; // the playing label.
	int group; // the overflow group.
}_viv_toolbar_item_t;

static const _viv_toolbar_item_t _viv_toolbar_items[_VIV_TOOLBAR_ITEM_COUNT] =
{
	{_VIV_TOOLBAR_TYPE_BUTTON,VIV_ID_FILE_OPEN_FILE,GLYPH_FOLDER_OPEN,GLYPH_FOLDER_OPEN,LOCALIZATION_ID_TOOLBAR_OPEN,LOCALIZATION_ID_TOOLBAR_OPEN,0},
	{_VIV_TOOLBAR_TYPE_SEP,0,0,0,0,0,1},
	{_VIV_TOOLBAR_TYPE_BUTTON,VIV_ID_NAV_PREV,GLYPH_PREV,GLYPH_PREV,LOCALIZATION_ID_TOOLBAR_PREVIOUS_IMAGE_BUTTON,LOCALIZATION_ID_TOOLBAR_PREVIOUS_IMAGE_BUTTON,1},
	{_VIV_TOOLBAR_TYPE_BUTTON,VIV_ID_NAV_NEXT,GLYPH_NEXT,GLYPH_NEXT,LOCALIZATION_ID_TOOLBAR_NEXT_IMAGE_BUTTON,LOCALIZATION_ID_TOOLBAR_NEXT_IMAGE_BUTTON,1},
	{_VIV_TOOLBAR_TYPE_BUTTON,VIV_ID_VIEW_SLIDESHOW,GLYPH_PLAY,GLYPH_PAUSE,LOCALIZATION_ID_TOOLBAR_PLAY_SLIDESHOW_BUTTON,LOCALIZATION_ID_TOOLBAR_PAUSE_SLIDESHOW_BUTTON,1},
	{_VIV_TOOLBAR_TYPE_SEP,0,0,0,0,0,2},
	{_VIV_TOOLBAR_TYPE_BUTTON,VIV_ID_VIEW_1TO1,GLYPH_1TO1,GLYPH_1TO1,LOCALIZATION_ID_TOOLBAR_ACTUAL_SIZE_BUTTON,LOCALIZATION_ID_TOOLBAR_ACTUAL_SIZE_BUTTON,2},
	{_VIV_TOOLBAR_TYPE_BUTTON,VIV_ID_VIEW_BESTFIT,GLYPH_BESTFIT,GLYPH_BESTFIT,LOCALIZATION_ID_TOOLBAR_BEST_FIT_BUTTON,LOCALIZATION_ID_TOOLBAR_BEST_FIT_BUTTON,2},
	{_VIV_TOOLBAR_TYPE_SEP,0,0,0,0,0,3},
	{_VIV_TOOLBAR_TYPE_BUTTON,VIV_ID_VIEW_ZOOM_OUT,GLYPH_MAGNIFIER_MINUS,GLYPH_MAGNIFIER_MINUS,LOCALIZATION_ID_TOOLBAR_ZOOM_OUT_BUTTON,LOCALIZATION_ID_TOOLBAR_ZOOM_OUT_BUTTON,3},
	{_VIV_TOOLBAR_TYPE_BUTTON,VIV_ID_VIEW_ZOOM_IN,GLYPH_MAGNIFIER_PLUS,GLYPH_MAGNIFIER_PLUS,LOCALIZATION_ID_TOOLBAR_ZOOM_IN_BUTTON,LOCALIZATION_ID_TOOLBAR_ZOOM_IN_BUTTON,3},
	{_VIV_TOOLBAR_TYPE_SEP,0,0,0,0,0,4},
	{_VIV_TOOLBAR_TYPE_BUTTON,VIV_ID_EDIT_ROTATE_90,GLYPH_ROTATE_CW,GLYPH_ROTATE_CW,LOCALIZATION_ID_TOOLBAR_ROTATE,LOCALIZATION_ID_TOOLBAR_ROTATE,4},
	{_VIV_TOOLBAR_TYPE_SEP,0,0,0,0,0,5},
	{_VIV_TOOLBAR_TYPE_BUTTON,VIV_ID_FILE_PROPERTIES,GLYPH_INFO,GLYPH_INFO,LOCALIZATION_ID_TOOLBAR_IMAGE_INFO,LOCALIZATION_ID_TOOLBAR_IMAGE_INFO,5}
};

static HWND _viv_toolbar_hwnd = 0;
static int _viv_toolbar_wide = 0; // the strip width the last layout ran with.
static int _viv_toolbar_bar_high = 0;
static int _viv_toolbar_icon_size = 0;
static int _viv_toolbar_icon_text_gap = 0;
static int _viv_toolbar_button_pad = 0;
static int _viv_toolbar_button_gap = 0;
static int _viv_toolbar_sep_wide = 0; // the whole separator slot.
static int _viv_toolbar_sep_high = 0;
static int _viv_toolbar_radius = 0;

static int _viv_toolbar_item_glyph[_VIV_TOOLBAR_ITEM_COUNT]; // the face in use.
static int _viv_toolbar_item_x[_VIV_TOOLBAR_ITEM_COUNT]; // client coords.
static int _viv_toolbar_item_wide[_VIV_TOOLBAR_ITEM_COUNT];
static BYTE _viv_toolbar_item_visible[_VIV_TOOLBAR_ITEM_COUNT];
static BYTE _viv_toolbar_item_enabled[_VIV_TOOLBAR_ITEM_COUNT];
static wchar_t _viv_toolbar_item_text[_VIV_TOOLBAR_ITEM_COUNT][STRING_SIZE];

static int _viv_toolbar_dark = 0; // 1 = draw with the dark palette.
static int _viv_toolbar_playing = 0; // 1 = the pause face on the play slot.
static int _viv_toolbar_hover = -1; // the item under the mouse, or -1.
static int _viv_toolbar_pressed = -1; // the item held with capture, or -1.
static BYTE _viv_toolbar_tracking = 0; // the mouse leave tracking is armed.

// the faces resolve through the theme tokens now: one palette for every
// surface (the private dark values duplicated the token table), and a
// theme or accent flip re-skins the strip through the cache flush in the
// apply path. the light hover is the quiet 3dlight face, not the loud
// selection blue the first cut painted.
static void _viv_toolbar_measure(void);
static LRESULT CALLBACK _viv_toolbar_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
void _viv_start_move_window(void); // viv_view.c: the strip background drag

// dip macros: the 96 dpi design units at the window's current dpi.
#define _VIV_TOOLBAR_DIP_X(dip) (((dip) * os_logical_wide) / 96)
#define _VIV_TOOLBAR_DIP_Y(dip) (((dip) * os_logical_high) / 96)

static HBRUSH _viv_toolbar_face_brush(void)
{
	return viv_theme_brush(VIV_TK_CHROME);
}

static HBRUSH _viv_toolbar_hot_brush(void)
{
	return viv_theme_brush(VIV_TK_HOVER);
}

static HBRUSH _viv_toolbar_press_brush(void)
{
	return viv_theme_brush(VIV_TK_DOWN);
}

static HBRUSH _viv_toolbar_line_brush(void)
{
	return viv_theme_brush(VIV_TK_CHROME_LINE);
}

// the label and the glyph of one slot: the play slot swaps both while the
// slideshow or the animation runs.
static int _viv_toolbar_item_label_id(int itemi)
{
	if ((itemi == _VIV_TOOLBAR_ITEM_PLAY) && (_viv_toolbar_playing))
	{
		return _viv_toolbar_items[itemi].localization_pause;
	}
	
	return _viv_toolbar_items[itemi].localization_play;
}

static int _viv_toolbar_item_glyph_id(int itemi)
{
	if ((itemi == _VIV_TOOLBAR_ITEM_PLAY) && (_viv_toolbar_playing))
	{
		return _viv_toolbar_items[itemi].glyph_pause;
	}
	
	return _viv_toolbar_items[itemi].glyph_play;
}

// re-read the strip metrics, the labels and the widths at the current font,
// then lay the slots out left to right and hide whole overflow groups from
// the right while the strip is too narrow.
static void _viv_toolbar_measure(void)
{
	HDC hdc;
	HFONT font;
	HFONT old_font;
	SIZE size;
	int itemi;
	int x;
	int total;
	int group;
	
	// the strip metrics at the current dpi. the floors keep a failed metric
	// query from collapsing the strip or the glyphs.
	_viv_toolbar_bar_high = _VIV_TOOLBAR_DIP_Y(_VIV_TOOLBAR_BAR_HIGH);
	
	if (_viv_toolbar_bar_high < 24)
	{
		_viv_toolbar_bar_high = 24;
	}
	
	_viv_toolbar_icon_size = _VIV_TOOLBAR_DIP_X(_VIV_TOOLBAR_ICON_SIZE);
	
	if (_viv_toolbar_icon_size < 8)
	{
		_viv_toolbar_icon_size = 8;
	}
	
	_viv_toolbar_icon_text_gap = _VIV_TOOLBAR_DIP_X(_VIV_TOOLBAR_ICON_TEXT_GAP);
	_viv_toolbar_button_pad = _VIV_TOOLBAR_DIP_X(_VIV_TOOLBAR_BUTTON_PAD);
	_viv_toolbar_button_gap = _VIV_TOOLBAR_DIP_X(_VIV_TOOLBAR_BUTTON_GAP);
	_viv_toolbar_sep_wide = (_viv_toolbar_button_gap * 2) + 1; // the 1px line with its air.
	_viv_toolbar_sep_high = _VIV_TOOLBAR_DIP_Y(_VIV_TOOLBAR_SEP_HIGH);
	
	if (_viv_toolbar_sep_high < 4)
	{
		_viv_toolbar_sep_high = 4;
	}
	
	_viv_toolbar_radius = _VIV_TOOLBAR_DIP_Y(_VIV_TOOLBAR_RADIUS);
	
	if (_viv_toolbar_radius < 1)
	{
		_viv_toolbar_radius = 1;
	}
	
	hdc = GetDC(_viv_toolbar_hwnd);
	
	if (!hdc)
	{
		return;
	}
	
	font = _viv_menu_font();
	old_font = 0;
	
	if (font)
	{
		old_font = SelectObject(hdc,font);
	}
	
	x = 0;
	
	for(itemi=0;itemi<_VIV_TOOLBAR_ITEM_COUNT;itemi++)
	{
		if (_viv_toolbar_items[itemi].type == _VIV_TOOLBAR_TYPE_SEP)
		{
			_viv_toolbar_item_wide[itemi] = _viv_toolbar_sep_wide;
		}
		else
		{
			size.cx = 0;
			size.cy = 0;
			
			string_copy_utf8_string(_viv_toolbar_item_text[itemi],localization_get_string((localization_id_t)_viv_toolbar_item_label_id(itemi)));
			
			GetTextExtentPoint32W(hdc,_viv_toolbar_item_text[itemi],(int)string_get_length(_viv_toolbar_item_text[itemi]),&size);
			
			_viv_toolbar_item_wide[itemi] = (_viv_toolbar_button_pad * 2) + _viv_toolbar_icon_size + _viv_toolbar_icon_text_gap + size.cx;
		}
		
		_viv_toolbar_item_glyph[itemi] = _viv_toolbar_item_glyph_id(itemi);
		_viv_toolbar_item_x[itemi] = x;
		_viv_toolbar_item_visible[itemi] = 1;
		
		x += _viv_toolbar_item_wide[itemi];
	}
	
	if (old_font)
	{
		SelectObject(hdc,old_font);
	}
	
	ReleaseDC(_viv_toolbar_hwnd,hdc);
	
	// the overflow: whole groups hide from the right (a separator hides with
	// the buttons to its right) until the strip fits or only the open group
	// is left. no wrapping, no chevron.
	total = x;
	group = _VIV_TOOLBAR_GROUP_MAX;
	
	while ((total > _viv_toolbar_wide) && (group > 0))
	{
		for(itemi=0;itemi<_VIV_TOOLBAR_ITEM_COUNT;itemi++)
		{
			if ((_viv_toolbar_items[itemi].group == group) && (_viv_toolbar_item_visible[itemi]))
			{
				_viv_toolbar_item_visible[itemi] = 0;
				
				total -= _viv_toolbar_item_wide[itemi];
			}
		}
		
		group--;
	}
}

static void _viv_toolbar_invalidate_item(int itemi)
{
	RECT rect;
	
	if ((itemi >= 0) && (itemi < _VIV_TOOLBAR_ITEM_COUNT) && (_viv_toolbar_hwnd) && (_viv_toolbar_item_visible[itemi]))
	{
		rect.left = _viv_toolbar_item_x[itemi];
		rect.top = 0;
		rect.right = _viv_toolbar_item_x[itemi] + _viv_toolbar_item_wide[itemi];
		rect.bottom = _viv_toolbar_bar_high;
		
		InvalidateRect(_viv_toolbar_hwnd,&rect,FALSE);
	}
}

static int _viv_toolbar_hit_test(int x,int y)
{
	int itemi;
	
	if ((y < 0) || (y >= _viv_toolbar_bar_high))
	{
		return -1;
	}
	
	for(itemi=0;itemi<_VIV_TOOLBAR_ITEM_COUNT;itemi++)
	{
		if ((_viv_toolbar_item_visible[itemi]) && (_viv_toolbar_items[itemi].type == _VIV_TOOLBAR_TYPE_BUTTON) &&
			(x >= _viv_toolbar_item_x[itemi]) && (x < _viv_toolbar_item_x[itemi] + _viv_toolbar_item_wide[itemi]))
		{
			return itemi;
		}
	}
	
	return -1;
}

// fire a button: the same bn_clicked notification a real button would have
// sent, then the focus back to the viewer so the keyboard shortcuts keep
// working (the zoom pill's rule).
static void _viv_toolbar_fire(int itemi)
{
	int command_id;
	
	if ((itemi < 0) || (itemi >= _VIV_TOOLBAR_ITEM_COUNT))
	{
		return;
	}
	
	command_id = _viv_toolbar_items[itemi].command_id;
	
	// the play slot resolves from the live state (the pill rule): a
	// running slideshow pauses in place, an animated image toggles the
	// animation clock, idle starts the windowed slideshow - the slot
	// never forces the fullscreen jump the old command did.
	if (itemi == _VIV_TOOLBAR_ITEM_PLAY)
	{
		if (_viv_is_slideshow)
		{
			command_id = VIV_ID_SLIDESHOW_PAUSE_ONLY;
		}
		else if (_viv_animation_play)
		{
			command_id = VIV_ID_ANIMATION_PLAY_PAUSE;
		}
		else
		{
			command_id = VIV_ID_SLIDESHOW_PLAY_ONLY;
		}
	}
	
	SendMessage(_viv_hwnd,WM_COMMAND,MAKEWPARAM(command_id,BN_CLICKED),0);
	
	SetFocus(_viv_hwnd);
}

static LRESULT CALLBACK _viv_toolbar_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			RECT rect;
			RECT fill_rect;
			RECT text_rect;
			HDC hdc;
			HFONT font;
			HFONT old_font;
			HGDIOBJ old_pen;
			HICON icon;
			COLORREF text_color;
			COLORREF disabled_color;
			int itemi;
			int hot;
			int offset;
			int sep_x;
			int text_left;
			
			hdc = BeginPaint(hwnd,&ps);
			
			GetClientRect(hwnd,&rect);
			
			// the face is one flat fill: the dark toolbar face or the system
			// button face. no system painter ever touches this strip.
			FillRect(hdc,&rect,_viv_toolbar_face_brush());
			
			// the top edge: the one chrome line the strip carries (the classic
			// 3d shadow in the light ui, the palette edge line in the dark).
			fill_rect.left = rect.left;
			fill_rect.top = rect.top;
			fill_rect.right = rect.right;
			fill_rect.bottom = rect.top + 1;
			
			FillRect(hdc,&fill_rect,_viv_toolbar_line_brush());
			
			font = _viv_menu_font();
			old_font = 0;
			
			if (font)
			{
				old_font = SelectObject(hdc,font);
			}
			
			SetBkMode(hdc,TRANSPARENT);
			
			text_color = viv_theme_color(VIV_TK_TEXT);
			disabled_color = viv_theme_color(VIV_TK_TEXTOFF);
			
			// the hover faces draw through a null pen so the rounded fill
			// carries no border of its own.
			old_pen = SelectObject(hdc,GetStockObject(NULL_PEN));
			
			for(itemi=0;itemi<_VIV_TOOLBAR_ITEM_COUNT;itemi++)
			{
				if (!_viv_toolbar_item_visible[itemi])
				{
					continue;
				}
				
				if (_viv_toolbar_items[itemi].type == _VIV_TOOLBAR_TYPE_SEP)
				{
					sep_x = _viv_toolbar_item_x[itemi] + (_viv_toolbar_sep_wide / 2);
					
					fill_rect.left = sep_x;
					fill_rect.top = (_viv_toolbar_bar_high - _viv_toolbar_sep_high) / 2;
					fill_rect.right = sep_x + 1;
					fill_rect.bottom = fill_rect.top + _viv_toolbar_sep_high;
					
					FillRect(hdc,&fill_rect,_viv_toolbar_line_brush());
					
					continue;
				}
				
				hot = ((_viv_toolbar_item_enabled[itemi]) && ((itemi == _viv_toolbar_hover) || (itemi == _viv_toolbar_pressed))) ? 1 : 0;
				
				// the pressed content nudges one pixel down right, but only
				// while the press is still under the cursor.
				offset = ((itemi == _viv_toolbar_pressed) && (itemi == _viv_toolbar_hover)) ? 1 : 0;
				
				if (hot)
				{
					SelectObject(hdc,(itemi == _viv_toolbar_pressed) ? _viv_toolbar_press_brush() : _viv_toolbar_hot_brush());
					
					RoundRect(hdc,_viv_toolbar_item_x[itemi],0,_viv_toolbar_item_x[itemi] + _viv_toolbar_item_wide[itemi],_viv_toolbar_bar_high,_viv_toolbar_radius * 2,_viv_toolbar_radius * 2);
				}
				
				icon = glyphs_icon(_viv_toolbar_item_glyph[itemi],_viv_toolbar_dark,_viv_toolbar_icon_size);
				
				if (icon)
				{
					int icon_x;
					int icon_y;
					
					icon_x = _viv_toolbar_item_x[itemi] + _viv_toolbar_button_pad + offset;
					icon_y = ((_viv_toolbar_bar_high - _viv_toolbar_icon_size) / 2) + offset;
					
					if (!_viv_toolbar_item_enabled[itemi])
					{
						// the embossed disabled glyph through the stock raster
						// op (the zoom pill's disabled state, same trick: no
						// extra cache, same theme aware icon).
						DrawState(hdc,NULL,NULL,(LPARAM)icon,0,icon_x,icon_y,_viv_toolbar_icon_size,_viv_toolbar_icon_size,DST_ICON | DSS_DISABLED);
					}
					else
					{
						DrawIconEx(hdc,icon_x,icon_y,icon,_viv_toolbar_icon_size,_viv_toolbar_icon_size,0,NULL,DI_NORMAL);
					}
				}
				
				text_left = _viv_toolbar_item_x[itemi] + _viv_toolbar_button_pad + _viv_toolbar_icon_size + _viv_toolbar_icon_text_gap + offset;
				
				text_rect.left = text_left;
				text_rect.top = 0;
				text_rect.right = _viv_toolbar_item_x[itemi] + _viv_toolbar_item_wide[itemi] - _viv_toolbar_button_pad;
				text_rect.bottom = _viv_toolbar_bar_high;
				
				SetTextColor(hdc,_viv_toolbar_item_enabled[itemi] ? text_color : disabled_color);
				
				DrawTextW(hdc,_viv_toolbar_item_text[itemi],-1,&text_rect,DT_SINGLELINE | DT_LEFT | DT_VCENTER);
			}
			
			SelectObject(hdc,old_pen);
			
			if (old_font)
			{
				SelectObject(hdc,old_font);
			}
			
			EndPaint(hwnd,&ps);
			
			return 0;
		}
		
		case WM_ERASEBKGND:
		{
			RECT rect;
			
			GetClientRect(hwnd,&rect);
			
			FillRect((HDC)wParam,&rect,_viv_toolbar_face_brush());
			
			return 1;
		}
		
		case WM_MOUSEMOVE:
		{
			TRACKMOUSEEVENT tme;
			int hit;
			
			// re-arm the leave track on every uncaptured move: a press
			// swallows leave generation, and without the re-arm the hover
			// sticks after a drag-off release (the zoomui pattern).
			if (GetCapture() != hwnd)
			{
				os_zero_memory(&tme,sizeof(tme));
				tme.cbSize = sizeof(tme);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd;
				
				TrackMouseEvent(&tme);
				
				_viv_toolbar_tracking = 1;
			}
			
			hit = _viv_toolbar_hit_test(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
			
			if (hit != _viv_toolbar_hover)
			{
				// a captured drag moves the hover with the cursor (the press
				// visual tracks, the command still fires only on the release
				// over the same button).
				_viv_toolbar_invalidate_item(_viv_toolbar_hover);
				_viv_toolbar_invalidate_item(hit);
				
				_viv_toolbar_hover = hit;
			}
			
			return 0;
		}
		
		case WM_MOUSELEAVE:
		
			_viv_toolbar_tracking = 0;
			
			// during a captured press the leave is stale: the release
			// handler owns the hover accounting.
			if (GetCapture() == hwnd)
			{
				return 0;
			}
			
			if (_viv_toolbar_hover != -1)
			{
				_viv_toolbar_invalidate_item(_viv_toolbar_hover);
				
				_viv_toolbar_hover = -1;
			}
			
			return 0;
		
		case WM_LBUTTONDBLCLK:
		// the double click lands as a fresh press (the zoomui rule: the
		// face follows the physical button, not the click count).
		case WM_LBUTTONDOWN:
		{
			int hit;
			
			hit = _viv_toolbar_hit_test(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
			
			// a disabled button never takes the press: no capture, no face,
			// so the release can never fire it (the classic rule).
			if ((hit >= 0) && (_viv_toolbar_item_enabled[hit]))
			{
				// press and hold: the command fires on the release inside the
				// same button (drag off the strip to cancel, the classic rule).
				_viv_toolbar_invalidate_item(_viv_toolbar_hover);
				
				_viv_toolbar_pressed = hit;
				_viv_toolbar_hover = hit;
				
				SetCapture(hwnd);
				
				_viv_toolbar_invalidate_item(hit);
			}
			else if ((hit < 0) && (config_toolbar_move_window))
			{
				// the rebar band drag lives on: dragging the strip
				// background moves the window (the caption drag).
				_viv_start_move_window();
			}
			
			return 0;
		}
		
		case WM_LBUTTONUP:
		{
			int hit;
			int pressed;
			
			if (_viv_toolbar_pressed >= 0)
			{
				pressed = _viv_toolbar_pressed;
				
				_viv_toolbar_pressed = -1;
				
				ReleaseCapture();
				
				hit = _viv_toolbar_hit_test(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
				
				_viv_toolbar_invalidate_item(pressed);
				
				_viv_toolbar_hover = hit;
				
				_viv_toolbar_invalidate_item(hit);
				
				if ((hit == pressed) && (_viv_toolbar_item_enabled[hit]))
				{
					_viv_toolbar_fire(hit);
				}
			}
			
			return 0;
		}
		
		case WM_CAPTURECHANGED:
		{
			int pressed;
			
			// the capture went elsewhere (a menu, another window): drop the
			// press visual, the command never fired (the zoom pill rule).
			pressed = _viv_toolbar_pressed;
			
			_viv_toolbar_pressed = -1;
			
			_viv_toolbar_invalidate_item(pressed);
			
			return 0;
		}
		
		default:
		
			break;
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

void _viv_toolbar_create(HWND parent)
{
	RECT rect;
	
	if (_viv_toolbar_hwnd)
	{
		return;
	}
	
	// no class brush: the strip face is painted by the erase and the paint
	// handlers above, a draw that bypasses them must not erase white (the
	// 1.1.03 lesson).
	os_RegisterClassEx(
		CS_DBLCLKS,
		_viv_toolbar_proc,
		0,
		LoadCursor(NULL,IDC_ARROW),
		NULL,
		"_VIV_TOOLBAR",
		0);
	
	_viv_toolbar_hwnd = os_CreateWindowEx(
		0,
		"_VIV_TOOLBAR",
		"",
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD,
		0,0,0,0,
		parent,(HMENU)VIV_ID_TOOLBAR,os_hinstance,NULL);
	
	if (!_viv_toolbar_hwnd)
	{
		return;
	}
	
	// the palette latch starts from the live theme so a paint that lands
	// before the first apply dark mode sweep still draws the right face.
	_viv_toolbar_set_dark(_viv_is_dark());
	
	GetClientRect(parent,&rect);
	
	_viv_toolbar_layout(rect.right - rect.left);
	
	_viv_toolbar_update_buttons();
	
	ShowWindow(_viv_toolbar_hwnd,SW_SHOW);
}

void _viv_toolbar_destroy(void)
{
	if (_viv_toolbar_hwnd)
	{
		DestroyWindow(_viv_toolbar_hwnd);
		
		_viv_toolbar_hwnd = 0;
		_viv_toolbar_hover = -1;
		_viv_toolbar_pressed = -1;
		_viv_toolbar_tracking = 0;
		
		// rc.13: the play latch rides the face state, not the window -
		// a rebuild re-reads it anyway, but a stale latch must never
		// survive into the next create.
		_viv_toolbar_playing = 0;
	}
}

void _viv_toolbar_layout(int wide)
{
	if (!_viv_toolbar_hwnd)
	{
		return;
	}
	
	_viv_toolbar_wide = wide;
	
	_viv_toolbar_measure();
	
	// the strip spans the client width, anchored directly under the menu
	// bar (a hidden menu bar reports zero and the strip takes the top).
	SetWindowPos(_viv_toolbar_hwnd,0,0,_viv_menubar_high(),wide,_viv_toolbar_bar_high,SWP_NOZORDER | SWP_NOACTIVATE);
	
	InvalidateRect(_viv_toolbar_hwnd,0,FALSE);
}

void _viv_toolbar_update_buttons(void)
{
	int rw;
	int rh;
	int playing;
	int has_image;
	int changed;
	int itemi;
	
	if (!_viv_toolbar_hwnd)
	{
		return;
	}
	
	// the play / pause face follows the slideshow and the animation clock.
	// (rc.13: the animation clock only counts when frames exist - the
	// bare preference bit sits at 1 on every static image, so the face
	// showed the pause bars on an idle photo. the ontop rule and the
	// animation timer gate the same way.)
	playing = ((_viv_is_slideshow) || ((_viv_frame_count > 1) && (_viv_animation_play))) ? 1 : 0;
	
	// the menu bar's no image gate: no file, a missed file or a failed load
	// disables everything that acts on the image.
	has_image = ((*_viv_current_fd->cFileName) && (!_viv_file_not_found) && (!_viv_load_failed)) ? 1 : 0;
	
	_viv_get_render_size(&rw,&rh);
	
	changed = 0;
	
	for(itemi=0;itemi<_VIV_TOOLBAR_ITEM_COUNT;itemi++)
	{
		int enable;
		int command_id;
		
		command_id = _viv_toolbar_items[itemi].command_id;
		
		enable = 1;
		
		switch(command_id)
		{
			case VIV_ID_NAV_PREV:
			case VIV_ID_NAV_NEXT:
			
				// a single item playlist has nowhere to step to.
				enable = ((has_image) && (_viv_nav_item_count > 1)) ? 1 : 0;
				break;
			
			case VIV_ID_VIEW_1TO1:
			
				// already at 1:1 (the old toolbar rule, the no image gate added).
				enable = ((has_image) && !((rw == _viv_image_wide) && (rh == _viv_image_high))) ? 1 : 0;
				break;
			
			case VIV_ID_VIEW_BESTFIT:
			
				// already at best fit (the old toolbar rule, the no image gate added).
				enable = ((has_image) && !((_viv_zoom_pos == 0) && (!_viv_1to1))) ? 1 : 0;
				break;
			
			case VIV_ID_EDIT_ROTATE_90:
			case VIV_ID_FILE_PROPERTIES:
			
				enable = has_image;
				break;
			
			default:
			
				// open, the slideshow toggle and the zoom pair stay live.
				enable = 1;
				break;
		}
		
		if (enable != (int)_viv_toolbar_item_enabled[itemi])
		{
			_viv_toolbar_item_enabled[itemi] = (BYTE)enable;
			
			changed = 1;
		}
	}
	
	if (playing != _viv_toolbar_playing)
	{
		_viv_toolbar_playing = playing;
		
		// the face swap changes the label: re-read the strings, re-measure
		// and re-run the overflow (the widths move).
		_viv_toolbar_measure();
		
		changed = 1;
	}
	
	if (changed)
	{
		InvalidateRect(_viv_toolbar_hwnd,0,FALSE);
	}
}

void _viv_toolbar_set_dark(int dark)
{
	dark = dark ? 1 : 0;
	
	if (dark != _viv_toolbar_dark)
	{
		// the colors resolve through the tokens at paint time now; the
		// latch only owns this repaint (and the glyph dark variant).
		_viv_toolbar_dark = dark;
		
		if (_viv_toolbar_hwnd)
		{
			InvalidateRect(_viv_toolbar_hwnd,0,FALSE);
		}
	}
}

int _viv_toolbar_high(void)
{
	if ((_viv_toolbar_hwnd) && (IsWindowVisible(_viv_toolbar_hwnd)))
	{
		return _viv_toolbar_bar_high;
	}
	
	return 0;
}
