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

// viv_msgbox.c - the themed message box.
//
// the classic MessageBox stays white (or uxtheme dark, build dependent) and
// carries none of the palette. this box paints the same modern panel the
// settings window uses: the theme face, the dialog font, a vector icon and
// two flat buttons on the theme tokens. the type flags are the MB_* subset
// the app actually uses, so the call sites read unchanged.

#include "viv.h"
#include "viv_state.h"
#include "viv_msgbox.h"
#include "viv_render.h"
#include "viv_chrome.h"

// one message box at a time: the modal pump owns these.
static HWND _viv_msgbox_hwnd = 0;
static int _viv_msgbox_done = 0;
static int _viv_msgbox_result = IDOK;
static wchar_t _viv_msgbox_text[STRING_SIZE];
static unsigned int _viv_msgbox_type = 0;

static int _viv_msgbox_dpi = 96;
static int _viv_msgbox_hover = -1;
static int _viv_msgbox_pressed = -1;
static int _viv_msgbox_focus = 0;
static HFONT _viv_msgbox_font = 0;

// the button rows: index 0 is the primary (right most), 1 the secondary.
#define _VIV_MSGBOX_BUTTONS		2

static RECT _viv_msgbox_button_rects[_VIV_MSGBOX_BUTTONS];

static int _viv_msgbox_dip(int d)
{
	return (d * _viv_msgbox_dpi) / 96;
}

static int _viv_msgbox_button_count(void)
{
	switch (_viv_msgbox_type & 0x0f)
	{
		case MB_OKCANCEL:
		case MB_YESNO:
			return 2;
	}

	return 1;
}

static const wchar_t *_viv_msgbox_button_text(int index)
{
	switch (_viv_msgbox_type & 0x0f)
	{
		case MB_YESNO:
			return localization_get_string(index == 0 ? LOCALIZATION_ID_MSGBOX_NO : LOCALIZATION_ID_MSGBOX_YES);

		case MB_OKCANCEL:
			return localization_get_string(index == 0 ? LOCALIZATION_ID_CANCEL_BUTTON : LOCALIZATION_ID_OK_BUTTON);
	}

	return localization_get_string(LOCALIZATION_ID_OK_BUTTON);
}

// the primary answers with the affirmative id, the secondary with its
// cancel/deny id (the ids match the MessageBox contract).
static int _viv_msgbox_button_result(int index)
{
	switch (_viv_msgbox_type & 0x0f)
	{
		case MB_YESNO:
			return index == 0 ? IDNO : IDYES;

		case MB_OKCANCEL:
			return index == 0 ? IDCANCEL : IDOK;
	}

	return IDOK;
}

static int _viv_msgbox_default_button(void)
{
	return _viv_msgbox_button_count() - 1;
}

static int _viv_msgbox_cancel_button(void)
{
	switch (_viv_msgbox_type & 0x0f)
	{
		case MB_OKCANCEL:
		case MB_YESNO:
			return 0;
	}

	return 0;
}

static void _viv_msgbox_layout_buttons(void)
{
	RECT rect;
	int i;
	int wide;
	int high;
	int x;

	GetClientRect(_viv_msgbox_hwnd,&rect);

	wide = _viv_msgbox_dip(88);
	high = _viv_msgbox_dip(32);
	x = rect.right - _viv_msgbox_dip(20);

	for(i=0;i<_VIV_MSGBOX_BUTTONS;i++)
	{
		if (i < _viv_msgbox_button_count())
		{
			_viv_msgbox_button_rects[i].left = x - wide;
			_viv_msgbox_button_rects[i].right = x;
			_viv_msgbox_button_rects[i].top = rect.bottom - _viv_msgbox_dip(20) - high;
			_viv_msgbox_button_rects[i].bottom = _viv_msgbox_button_rects[i].top + high;

			x = _viv_msgbox_button_rects[i].left - _viv_msgbox_dip(8);
		}
		else
		{
			_viv_msgbox_button_rects[i].left = 0;
			_viv_msgbox_button_rects[i].top = 0;
			_viv_msgbox_button_rects[i].right = 0;
			_viv_msgbox_button_rects[i].bottom = 0;
		}
	}
}

static int _viv_msgbox_hit_test(int x,int y)
{
	int i;

	for(i=0;i<_VIV_MSGBOX_BUTTONS;i++)
	{
		if ((i < _viv_msgbox_button_count()) &&
			(x >= _viv_msgbox_button_rects[i].left) && (x < _viv_msgbox_button_rects[i].right) &&
			(y >= _viv_msgbox_button_rects[i].top) && (y < _viv_msgbox_button_rects[i].bottom))
		{
			return i;
		}
	}

	return -1;
}

static void _viv_msgbox_draw_button(HDC hdc,int index)
{
	RECT rect;
	int hot;
	int pressed;
	int focused;
	COLORREF fill;
	COLORREF text;
	HBRUSH brush;

	rect = _viv_msgbox_button_rects[index];

	hot = (_viv_msgbox_hover == index);
	pressed = (_viv_msgbox_pressed == index);
	focused = (_viv_msgbox_focus == index);

	// the primary button is the accent fill, the secondary stays neutral.
	if (index == _viv_msgbox_default_button())
	{
		fill = pressed ? viv_theme_color(VIV_TK_ACCENT_DOWN) : (hot ? viv_theme_color(VIV_TK_ACCENT_HOT) : viv_theme_color(VIV_TK_ACCENT));
		text = viv_theme_color(VIV_TK_ON_ACCENT);
	}
	else
	{
		fill = pressed ? viv_theme_color(VIV_TK_DOWN) : (hot ? viv_theme_color(VIV_TK_HOVER) : viv_theme_color(VIV_TK_FACE));
		text = viv_theme_color(VIV_TK_TEXT);
	}

	brush = CreateSolidBrush(fill);

	// rc.13: the settings window's pill geometry - one rounded fill rides
	// both faces (the square fillrect was the non-win11 eyesore next to
	// the rounded settings controls). the primary's pen matches its fill
	// (no border), the secondary keeps the quiet line tone.
	{
		HPEN pen;
		HPEN old_pen;
		HBRUSH old_brush;
		int radius;

		radius = _viv_msgbox_dip(6);

		pen = CreatePen(PS_SOLID,1,index == _viv_msgbox_default_button() ? fill : viv_theme_color(VIV_TK_LINE));
		old_pen = (HPEN)SelectObject(hdc,pen);
		old_brush = (HBRUSH)SelectObject(hdc,brush);

		RoundRect(hdc,rect.left,rect.top,rect.right,rect.bottom,radius * 2,radius * 2);

		SelectObject(hdc,old_brush);
		SelectObject(hdc,old_pen);

		DeleteObject(pen);
	}

	DeleteObject(brush);

	if (focused)
	{
		RECT ring;
		HPEN pen;
		HPEN old_pen;
		HGDIOBJ old_null_brush;

		ring = rect;
		InflateRect(&ring,-_viv_msgbox_dip(3),-_viv_msgbox_dip(3));

		// the ring follows the same radius (the old square framerec ring
		// poked out of the rounded face).
		pen = CreatePen(PS_SOLID,1,viv_theme_color(index == _viv_msgbox_default_button() ? VIV_TK_ON_ACCENT : VIV_TK_TEXT2));
		old_pen = (HPEN)SelectObject(hdc,pen);
		old_null_brush = SelectObject(hdc,GetStockObject(NULL_BRUSH));

		RoundRect(hdc,ring.left,ring.top,ring.right,ring.bottom,_viv_msgbox_dip(3) * 2,_viv_msgbox_dip(3) * 2);

		SelectObject(hdc,old_null_brush);
		SelectObject(hdc,old_pen);

		DeleteObject(pen);
	}

	SetBkMode(hdc,TRANSPARENT);
	SetTextColor(hdc,text);

	if (_viv_msgbox_font)
	{
		SelectObject(hdc,_viv_msgbox_font);
	}

	DrawTextW(hdc,_viv_msgbox_button_text(index),-1,&rect,DT_SINGLELINE | DT_CENTER | DT_VCENTER);
}

static void _viv_msgbox_draw_icon(HDC hdc,int x,int y,int size)
{
	HBRUSH brush;
	HBRUSH old_brush;
	HPEN pen;
	HPEN old_pen;
	COLORREF tone;
	int cx;
	int cy;

	cx = x + size / 2;
	cy = y + size / 2;

	// the status tones: the accent carries info and question, the warning
	// and error tones are fixed brand constants (they never follow the
	// accent so a green accent never softens a failure).
	switch (_viv_msgbox_type & 0xf0)
	{
		case MB_ICONERROR:
			tone = RGB(0xD6,0x44,0x5E);
			break;

		case MB_ICONWARNING:
			tone = RGB(0xC7,0x74,0x20);
			break;

		default:
			tone = viv_theme_color(VIV_TK_ACCENT);
			break;
	}

	brush = CreateSolidBrush(tone);
	old_brush = (HBRUSH)SelectObject(hdc,brush);

	pen = CreatePen(PS_SOLID,1,tone);
	old_pen = (HPEN)SelectObject(hdc,pen);

	if ((_viv_msgbox_type & 0xf0) == MB_ICONWARNING)
	{
		POINT pts[3];

		pts[0].x = cx;
		pts[0].y = y + _viv_msgbox_dip(2);
		pts[1].x = x + size - _viv_msgbox_dip(1);
		pts[1].y = y + size - _viv_msgbox_dip(2);
		pts[2].x = x + _viv_msgbox_dip(1);
		pts[2].y = y + size - _viv_msgbox_dip(2);

		Polygon(hdc,pts,3);
	}
	else
	{
		Ellipse(hdc,x,y,x + size,y + size);
	}

	SelectObject(hdc,old_brush);
	DeleteObject(brush);

	SelectObject(hdc,old_pen);
	DeleteObject(pen);

	// the glyph inside, in the on accent tone.
	{
		int white;

		white = _viv_msgbox_dip(2) < 1 ? 1 : _viv_msgbox_dip(2);

		pen = CreatePen(PS_SOLID,(_viv_msgbox_type & 0xf0) == MB_ICONERROR ? white + 1 : white,viv_theme_color(VIV_TK_ON_ACCENT));
		old_pen = (HPEN)SelectObject(hdc,pen);

		switch (_viv_msgbox_type & 0xf0)
		{
			case MB_ICONERROR:
				MoveToEx(hdc,cx - _viv_msgbox_dip(6),cy - _viv_msgbox_dip(6),0);
				LineTo(hdc,cx + _viv_msgbox_dip(6),cy + _viv_msgbox_dip(6));
				MoveToEx(hdc,cx + _viv_msgbox_dip(6),cy - _viv_msgbox_dip(6),0);
				LineTo(hdc,cx - _viv_msgbox_dip(6),cy + _viv_msgbox_dip(6));
				break;

			case MB_ICONWARNING:
				MoveToEx(hdc,cx,cy - _viv_msgbox_dip(8),0);
				LineTo(hdc,cx,cy + _viv_msgbox_dip(2));

				// the dot: a short stub on the same pen.
				MoveToEx(hdc,cx,cy + _viv_msgbox_dip(6),0);
				LineTo(hdc,cx,cy + _viv_msgbox_dip(6) + white);
				break;

			case MB_ICONQUESTION:
				// the question curve, three strokes on the circle tone.
				MoveToEx(hdc,cx - _viv_msgbox_dip(5),cy - _viv_msgbox_dip(4),0);
				LineTo(hdc,cx - _viv_msgbox_dip(2),cy - _viv_msgbox_dip(7));
				LineTo(hdc,cx + _viv_msgbox_dip(3),cy - _viv_msgbox_dip(5));
				LineTo(hdc,cx + _viv_msgbox_dip(2),cy);
				LineTo(hdc,cx,cy + _viv_msgbox_dip(2));

				MoveToEx(hdc,cx,cy + _viv_msgbox_dip(6),0);
				LineTo(hdc,cx,cy + _viv_msgbox_dip(6) + white);
				break;

			default:
				// the information i.
				MoveToEx(hdc,cx,cy - _viv_msgbox_dip(1),0);
				LineTo(hdc,cx,cy + _viv_msgbox_dip(7));

				MoveToEx(hdc,cx,cy - _viv_msgbox_dip(7),0);
				LineTo(hdc,cx,cy - _viv_msgbox_dip(7) + white);
				break;
		}

		SelectObject(hdc,old_pen);
		DeleteObject(pen);
	}
}

static void _viv_msgbox_paint(void)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;
	RECT text_rect;
	HFONT old_font;
	int i;

	hdc = BeginPaint(_viv_msgbox_hwnd,&ps);

	GetClientRect(_viv_msgbox_hwnd,&rect);

	FillRect(hdc,&rect,viv_theme_brush(VIV_TK_FACE));

	// skip the icon when the type carries no icon flag.
	if (_viv_msgbox_type & 0xf0)
	{
		_viv_msgbox_draw_icon(hdc,_viv_msgbox_dip(24),_viv_msgbox_dip(24),_viv_msgbox_dip(32));
	}

	old_font = 0;

	if (_viv_msgbox_font)
	{
		old_font = (HFONT)SelectObject(hdc,_viv_msgbox_font);
	}

	SetBkMode(hdc,TRANSPARENT);
	SetTextColor(hdc,viv_theme_color(VIV_TK_TEXT));

	text_rect.left = _viv_msgbox_dip(24 + 32 + 20);
	text_rect.right = rect.right - _viv_msgbox_dip(24);
	text_rect.top = _viv_msgbox_dip(22);
	text_rect.bottom = rect.bottom - _viv_msgbox_dip(24 + 32 + 16);

	DrawTextW(hdc,_viv_msgbox_text,-1,&text_rect,DT_WORDBREAK | DT_LEFT | DT_TOP);

	for(i=0;i<_VIV_MSGBOX_BUTTONS;i++)
	{
		if (i < _viv_msgbox_button_count())
		{
			_viv_msgbox_draw_button(hdc,i);
		}
	}

	if (old_font)
	{
		SelectObject(hdc,old_font);
	}

	EndPaint(_viv_msgbox_hwnd,&ps);
}

static void _viv_msgbox_fonts_create(void)
{
	LOGFONTW lf;

	if (os_dialog_font(&lf,_viv_msgbox_hwnd))
	{
		lf.lfHeight = -_viv_msgbox_dip(12);
		lf.lfWeight = FW_NORMAL;

		_viv_msgbox_font = CreateFontIndirectW(&lf);
	}
}

static void _viv_msgbox_fonts_delete(void)
{
	if (_viv_msgbox_font)
	{
		DeleteObject(_viv_msgbox_font);
		_viv_msgbox_font = 0;
	}
}

static void _viv_msgbox_fire(int index)
{
	if ((index < 0) || (index >= _viv_msgbox_button_count()))
	{
		return;
	}

	_viv_msgbox_result = _viv_msgbox_button_result(index);
	_viv_msgbox_done = 1;

	PostMessage(_viv_msgbox_hwnd,WM_CLOSE,0,0);
}

static LRESULT CALLBACK _viv_msgbox_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
		{
			_viv_msgbox_hwnd = hwnd;

			os_dark_titlebar(hwnd,_viv_is_dark());
			os_window_modern_chrome(hwnd,viv_theme_color(VIV_TK_CHROME));

			// the entry measure pass already made the pair: reuse it.
			if (!_viv_msgbox_font)
			{
				_viv_msgbox_fonts_create();
			}

			return 0;
		}

		case WM_DPICHANGED:
		{
			RECT *suggested;

			// follow the monitor: new dpi, rebuilt fonts, suggested rect.
			_viv_msgbox_dpi = (int)LOWORD(wParam);

			_viv_msgbox_fonts_delete();
			_viv_msgbox_fonts_create();

			_viv_msgbox_layout_buttons();

			InvalidateRect(hwnd,0,FALSE);

			suggested = (RECT *)lParam;

			if (suggested)
			{
				SetWindowPos(hwnd,0,suggested->left,suggested->top,suggested->right - suggested->left,suggested->bottom - suggested->top,SWP_NOZORDER | SWP_NOACTIVATE);
			}

			return 0;
		}

		case WM_PAINT:
			_viv_msgbox_paint();
			return 0;

		case WM_ERASEBKGND:
			return 1;

		case WM_MOUSEMOVE:
		{
			POINT pt;
			int hit;

			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);

			hit = _viv_msgbox_hit_test(pt.x,pt.y);

			if (hit != _viv_msgbox_hover)
			{
				_viv_msgbox_hover = hit;

				InvalidateRect(hwnd,0,FALSE);
			}

			// track the leave so the hover face clears.
			{
				TRACKMOUSEEVENT tme;

				os_zero_memory(&tme,sizeof(tme));
				tme.cbSize = sizeof(tme);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd;

				TrackMouseEvent(&tme);
			}
			return 0;
		}

		case WM_MOUSELEAVE:
			if (_viv_msgbox_hover != -1)
			{
				_viv_msgbox_hover = -1;

				InvalidateRect(hwnd,0,FALSE);
			}
			return 0;

		case WM_LBUTTONDOWN:
		{
			POINT pt;

			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);

			_viv_msgbox_pressed = _viv_msgbox_hit_test(pt.x,pt.y);

			if (_viv_msgbox_pressed != -1)
			{
				SetCapture(hwnd);

				InvalidateRect(hwnd,0,FALSE);
			}
			return 0;
		}

		case WM_LBUTTONUP:
		{
			POINT pt;
			int pressed;

			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);

			pressed = _viv_msgbox_pressed;

			if (GetCapture() == hwnd)
			{
				ReleaseCapture();
			}

			_viv_msgbox_pressed = -1;

			InvalidateRect(hwnd,0,FALSE);

			if ((pressed != -1) && (pressed == _viv_msgbox_hit_test(pt.x,pt.y)))
			{
				_viv_msgbox_fire(pressed);
			}
			return 0;
		}

		case WM_CAPTURECHANGED:
			_viv_msgbox_pressed = -1;
			InvalidateRect(hwnd,0,FALSE);
			return 0;

		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_TAB:
					if (_viv_msgbox_button_count() > 1)
					{
						_viv_msgbox_focus = 1 - _viv_msgbox_focus;

						InvalidateRect(hwnd,0,FALSE);
					}
					return 0;

				case VK_RETURN:
				case VK_SPACE:
					_viv_msgbox_fire(_viv_msgbox_focus);
					return 0;

				case VK_ESCAPE:
					_viv_msgbox_fire(_viv_msgbox_cancel_button());
					return 0;
			}
			break;

		case WM_CLOSE:
			// a naked close still answers instead of leaving the initialized
			// idok: like the cancel button, ok for a lone ok box (a fire has
			// already stored its own result).
			if (!_viv_msgbox_done)
			{
				_viv_msgbox_result = (_viv_msgbox_type & 0x0f) == MB_OK ? IDOK : IDCANCEL;
			}

			DestroyWindow(hwnd);
			return 0;

		case WM_NCDESTROY:
			_viv_msgbox_fonts_delete();
			_viv_msgbox_hwnd = 0;
			_viv_msgbox_done = 1;
			return 0;
	}

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

int viv_msgbox(HWND parent,const wchar_t *caption,const wchar_t *text,unsigned int type)
{
	WNDCLASSEXW wc;
	HDC hdc;
	RECT text_rect;
	RECT window_rect;
	int text_high;
	int text_wide;
	int wide;
	int high;
	MSG msg;
	HFONT old_font;
	DWORD style;

	if (!text)
	{
		return IDOK;
	}

	// one box at a time: the shared statics are single-slot.
	if (_viv_msgbox_hwnd)
	{
		SetForegroundWindow(_viv_msgbox_hwnd);
		SetActiveWindow(_viv_msgbox_hwnd);

		return IDOK;
	}

	string_copy(_viv_msgbox_text,text);
	_viv_msgbox_type = type;
	_viv_msgbox_done = 0;
	_viv_msgbox_result = IDOK;
	_viv_msgbox_hover = -1;
	_viv_msgbox_pressed = -1;
	_viv_msgbox_focus = 0;

	if (!GetClassInfoExW(os_hinstance,L"VIV_MSGBOX",&wc))
	{
		os_zero_memory(&wc,sizeof(wc));
		wc.cbSize = sizeof(wc);
		wc.lpfnWndProc = _viv_msgbox_proc;
		wc.hInstance = os_hinstance;
		wc.hCursor = LoadCursor(0,IDC_ARROW);
		wc.lpszClassName = L"VIV_MSGBOX";

		RegisterClassExW(&wc);
	}

	// the box centers on its parent, so the parent's monitor dpi is
	// the right scale for the fonts and the layout: os_logical_*
	// tracks the main window, which matches in the common case but
	// goes wrong on mixed-dpi monitor pairs (and os_window_dpi(0)
	// falls back to exactly that global value for a null parent).
	_viv_msgbox_dpi = os_window_dpi(parent);

	// measure the text at the layout width, then size the panel around it.
	hdc = GetDC(parent ? parent : 0);

	text_wide = _viv_msgbox_dip(400);

	text_rect.left = 0;
	text_rect.right = text_wide;
	text_rect.top = 0;
	text_rect.bottom = 0;

	old_font = 0;

	_viv_msgbox_fonts_create();

	if (_viv_msgbox_font)
	{
		old_font = (HFONT)SelectObject(hdc,_viv_msgbox_font);
	}

	DrawTextW(hdc,_viv_msgbox_text,-1,&text_rect,DT_WORDBREAK | DT_CALCRECT);

	// the height clamp: a long text would push the buttons below the work
	// area, so widen the column (up to a cap) and re-measure.
	{
		RECT work_rect;
		int work_wide;
		int work_high;
		int max_wide;
		int frame_high;
		int window_high;

		os_MonitorRectFromWindow(parent,0,&work_rect);

		work_wide = work_rect.right - work_rect.left;
		work_high = work_rect.bottom - work_rect.top;

		// the caption and frame are not measured yet: a rough allowance.
		frame_high = _viv_msgbox_dip(72);

		max_wide = work_wide - _viv_msgbox_dip(24 + 32 + 20 + 24) - frame_high;

		window_high = _viv_msgbox_dip(22 + 16 + 32 + 20) + frame_high + (text_rect.bottom - text_rect.top);

		while ((window_high > work_high) && (text_wide < max_wide))
		{
			text_wide += _viv_msgbox_dip(100);

			if (text_wide > max_wide)
			{
				text_wide = max_wide;
			}

			text_rect.left = 0;
			text_rect.top = 0;
			text_rect.right = text_wide;
			text_rect.bottom = 0;

			DrawTextW(hdc,_viv_msgbox_text,-1,&text_rect,DT_WORDBREAK | DT_CALCRECT);

			window_high = _viv_msgbox_dip(22 + 16 + 32 + 20) + frame_high + (text_rect.bottom - text_rect.top);
		}
	}

	if (old_font)
	{
		SelectObject(hdc,old_font);
	}

	ReleaseDC(parent ? parent : 0,hdc);

	text_high = text_rect.bottom - text_rect.top;
	if (text_high < _viv_msgbox_dip(32))
	{
		text_high = _viv_msgbox_dip(32);
	}

	wide = _viv_msgbox_dip(24 + 32 + 20) + text_wide + _viv_msgbox_dip(24);
	high = _viv_msgbox_dip(22) + text_high + _viv_msgbox_dip(16) + _viv_msgbox_dip(32) + _viv_msgbox_dip(20);

	window_rect.left = 0;
	window_rect.top = 0;
	window_rect.right = wide;
	window_rect.bottom = high;

	// a yes/no box carries no close box (the native one disables it too).
	if ((_viv_msgbox_type & 0x0f) == MB_YESNO)
	{
		style = WS_POPUP | WS_CAPTION;
	}
	else
	{
		style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
	}

	// adjustwindowrect takes no dpi, a manual compensation would duplicate it.
	AdjustWindowRect(&window_rect,style,FALSE);

	// pass the owner: no taskbar button of its own, the parent stays behind.
	_viv_msgbox_hwnd = CreateWindowExW(0,L"VIV_MSGBOX",caption,style,
		0,0,window_rect.right - window_rect.left,window_rect.bottom - window_rect.top,
		parent,0,os_hinstance,0);

	if (!_viv_msgbox_hwnd)
	{
		// drop the measure pair: the failure path must not leak it.
		_viv_msgbox_fonts_delete();

		return IDOK;
	}

	// center on the parent (or the work area).
	{
		RECT owner_rect;
		RECT box_rect;
		int x;
		int y;

		box_rect.left = 0;
		box_rect.top = 0;
		box_rect.right = window_rect.right - window_rect.left;
		box_rect.bottom = window_rect.bottom - window_rect.top;

		if ((parent) && (GetWindowRect(parent,&owner_rect)))
		{
			x = owner_rect.left + ((owner_rect.right - owner_rect.left) - box_rect.right) / 2;
			y = owner_rect.top + ((owner_rect.bottom - owner_rect.top) - box_rect.bottom) / 3;
		}
		else
		{
			SystemParametersInfoW(SPI_GETWORKAREA,0,&owner_rect,0);

			x = owner_rect.left + ((owner_rect.right - owner_rect.left) - box_rect.right) / 2;
			y = owner_rect.top + ((owner_rect.bottom - owner_rect.top) - box_rect.bottom) / 2;
		}

		SetWindowPos(_viv_msgbox_hwnd,HWND_TOP,x,y,0,0,SWP_NOSIZE | SWP_NOACTIVATE);
	}

	_viv_msgbox_focus = _viv_msgbox_default_button();

	_viv_msgbox_layout_buttons();

	if (parent)
	{
		EnableWindow(parent,FALSE);
	}

	ShowWindow(_viv_msgbox_hwnd,SW_SHOW);
	UpdateWindow(_viv_msgbox_hwnd);

	// the modal pump: the box eats its keys, everything else keeps the
	// normal dispatch (the parent is disabled so it only sees the paint).
	os_zero_memory(&msg,sizeof(msg));

	while ((!_viv_msgbox_done) && (GetMessageW(&msg,0,0,0) > 0))
	{
		if ((msg.message == WM_KEYDOWN) && (msg.hwnd == _viv_msgbox_hwnd))
		{
			SendMessageW(_viv_msgbox_hwnd,WM_KEYDOWN,msg.wParam,msg.lParam);

			continue;
		}

		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	// a quit that ended the pump must reach the outer loop: re-post it.
	if (msg.message == WM_QUIT)
	{
		PostQuitMessage((int)msg.wParam);
	}

	if (IsWindow(_viv_msgbox_hwnd))
	{
		DestroyWindow(_viv_msgbox_hwnd);
	}

	if (parent)
	{
		EnableWindow(parent,TRUE);
		SetActiveWindow(parent);
	}

	return _viv_msgbox_result;
}
