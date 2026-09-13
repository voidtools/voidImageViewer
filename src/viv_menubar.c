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
// viv_menubar.c - the top bar, remade: a fully self drawn menu bar.
//
// The old design kept the system frame menu and patched it dark: owner drawn
// root items, a captured "system pad", a non client tail fill for the strip
// the system kept light, and a re-apply sweep to survive the theme races. Five
// rounds of field reports lived on that seam (the white right strip, the pad
// capture drifting 5.5x wide, the flip races) because the strip had two
// painters: the system measured and painted the bar, the app patched around
// it. This module owns the strip end to end: the window has no frame menu at
// all, the bar is a child window over the client, the layout (label extent
// plus a fixed dpi scaled air) is theme independent, and the popups are the
// same HMENU tree opened with TrackPopupMenuEx.
#include "viv.h"
#include "viv_state.h"
#include "viv_chrome.h"
#include "viv_render.h"
#include "viv_dark.h"
#include "viv_menu.h"
#include "viv_menubar.h"
#include "viv_recent.h"

#define _VIV_MENUBAR_ITEM_MAX 16

static HWND _viv_menubar_hwnd = 0;
static int _viv_menubar_item_count = 0;
static int _viv_menubar_item_wide[_VIV_MENUBAR_ITEM_MAX];
static wchar_t _viv_menubar_item_text[_VIV_MENUBAR_ITEM_MAX][STRING_SIZE];
static int _viv_menubar_bar_high = 0;
static int _viv_menubar_hover = -1; // the item under the mouse
static int _viv_menubar_open = -1; // the item whose popup is up
static BYTE _viv_menubar_pressed = 0; // the button is held on the bar
static BYTE _viv_menubar_tracking = 0; // the mouse leave tracking is armed
static int _viv_menubar_deferred = -1; // the item to open once the current popup loop returns (the hover follow)

// the measured width must not carry the '&' accelerators: the bar draws
// with DT_HIDEPREFIX until alt is held, so a label measured with the
// ampersand in place ran several pixels wider than the glyph run it
// painted.
static int _viv_menubar_strip_ampersand(wchar_t *dst,const wchar_t *src)
{
	int read;
	int write;

	read = 0;
	write = 0;

	while(src[read])
	{
		if (src[read] == L'&')
		{
			read++;

			if (!src[read])
			{
				break;
			}
		}

		dst[write] = src[read];

		write++;
		read++;
	}

	dst[write] = 0;

	return write;
}

void _viv_menubar_kill(void)
{
	// the strip colors live in the shared theme brush cache now: the
	// theme refresh owns their lifetime, nothing to free here.
}

// the menu modal loop idles on the owner: pump the pending mouse moves
// so the bar keeps hovering (and following) while a popup is up.
void _viv_menubar_idle_pump(void)
{
	MSG msg;
	int guard;

	if (_viv_menubar_open == -1)
	{
		return;
	}

	for(guard=0;guard<16;guard++)
	{
		if (!PeekMessage(&msg,0,WM_MOUSEFIRST,WM_MOUSELAST,PM_NOREMOVE))
		{
			break;
		}

		// -1 is the error return; treating it as success dispatches a
		// garbage message (the BOOL trap).
		if (GetMessage(&msg,0,WM_MOUSEFIRST,WM_MOUSELAST) <= 0)
		{
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (_viv_menubar_deferred >= 0)
		{
			break;
		}
	}
}


static void _viv_menubar_item_rect(int itemi,RECT *rect)
{
	int x;
	int i;
	
	x = 0;
	
	for(i=0;i<itemi;i++)
	{
		x += _viv_menubar_item_wide[i];
	}
	
	rect->left = x;
	rect->top = 0;
	rect->right = x + _viv_menubar_item_wide[itemi];
	rect->bottom = _viv_menubar_bar_high;
}

static int _viv_menubar_hit_test(int x,int y)
{
	int itemi;
	
	if ((y < 0) || (y >= _viv_menubar_bar_high))
	{
		return -1;
	}
	
	for(itemi=0;itemi<_viv_menubar_item_count;itemi++)
	{
		if (x < _viv_menubar_item_wide[itemi])
		{
			return itemi;
		}
		
		x -= _viv_menubar_item_wide[itemi];
	}
	
	return -1;
}

static void _viv_menubar_invalidate_item(int itemi)
{
	RECT rect;
	
	if ((itemi >= 0) && (itemi < _viv_menubar_item_count) && (_viv_menubar_hwnd))
	{
		_viv_menubar_item_rect(itemi,&rect);
		
		InvalidateRect(_viv_menubar_hwnd,&rect,FALSE);
	}
}

// open one root popup. the state refresh the frame menu used to get from
// wm_initmenu runs here (and again from wm_initmenupopup - both idempotent),
// the pressed item keeps its lifted face while the modal loop runs, and the
// command ids reach the same wm_command path the frame menu used.
static void _viv_menubar_open_popup(int itemi)
{
	HMENU popup;
	RECT rect;
	POINT pt;
	
	if ((itemi < 0) || (itemi >= _viv_menubar_item_count))
	{
		return;
	}
	
	popup = GetSubMenu(_viv_hmenu,itemi);
	
	if (!popup)
	{
		return;
	}
	
	_viv_check_menus(_viv_hmenu);
	
	_viv_menubar_item_rect(itemi,&rect);
	
	pt.x = rect.left;
	pt.y = rect.bottom;
	
	ClientToScreen(_viv_menubar_hwnd,&pt);
	
	// the hover follow: a move onto another root item while a popup is
	// up cancels the loop and reopens at the new item (the classic bar
	// followed the mouse across the roots; the synthetic arrow trick does
	// not exist for trackpopupmenu, so the close plus reopen is the
	// honest equivalent - same tick, no visible flash at popup scale).
	// the in-popup flag covers the whole loop (the context menu site and
	// the cursor hider share it): the slideshow timer's recent-file swap
	// defers, and the cursor stays visible while a menu is up.
	_viv_in_popup_menu = 1;
	
	do
	{
		_viv_menubar_deferred = -1;

		_viv_menubar_open = itemi;
		_viv_menubar_hover = itemi;

		InvalidateRect(_viv_menubar_hwnd,0,FALSE);
		UpdateWindow(_viv_menubar_hwnd);

		TrackPopupMenuEx(popup,TPM_LEFTALIGN | TPM_RIGHTBUTTON,pt.x,pt.y,_viv_hwnd,0);

		if (_viv_menubar_deferred >= 0)
		{
			itemi = _viv_menubar_deferred;

			_viv_menubar_item_rect(itemi,&rect);

			pt.x = rect.left;
			pt.y = rect.bottom;

			ClientToScreen(_viv_menubar_hwnd,&pt);

			popup = GetSubMenu(_viv_hmenu,itemi);

			if (!popup)
			{
				_viv_menubar_deferred = -1;

				break;
			}

			_viv_check_menus(_viv_hmenu);

			continue;
		}

		break;
	} while(1);

	_viv_in_popup_menu = 0;
	
	_viv_recent_menu_flush();
	
	// the loop is gone: the release that closed the menu can land on a
	// sibling item, so the hover is allowed to re-arm on the next move.
	_viv_menubar_open = -1;
	_viv_menubar_hover = -1;
	_viv_menubar_deferred = -1;

	InvalidateRect(_viv_menubar_hwnd,0,FALSE);
}

// re-read the root items: the labels and the widths at the current font.
// the air is a fixed constant (the classic top level air, dpi scaled), so
// the light and the dark ui lay out identically - the capture that drifted
// 5.5x wide between the themes is gone with the second measurement system.
void _viv_menubar_layout(void)
{
	MENUITEMINFOW mii;
	HDC hdc;
	HFONT font;
	HFONT old_font;
	int index;
	int count;
	int pad;
	int high;
	int min_high;
	
	_viv_menubar_item_count = 0;
	_viv_menubar_bar_high = 0;
	_viv_menubar_hover = -1;
	_viv_menubar_open = -1;
	
	if ((!_viv_hwnd) || (!_viv_hmenu))
	{
		return;
	}
	
	hdc = GetDC(_viv_hwnd);
	
	if (!hdc)
	{
		return;
	}
	
	count = GetMenuItemCount(_viv_hmenu);
	
	if (count > _VIV_MENUBAR_ITEM_MAX)
	{
		count = _VIV_MENUBAR_ITEM_MAX;
	}
	
	pad = (4 * os_logical_wide) / 96;
	
	high = 0;
	
	font = _viv_menu_font();
	old_font = 0;
	
	if (font)
	{
		old_font = SelectObject(hdc,font);
	}
	
	for(index=0;index<count;index++)
	{
		SIZE size;
		wchar_t plain[STRING_SIZE];

		_viv_menubar_item_text[_viv_menubar_item_count][0] = 0;
		
		os_zero_memory(&mii,sizeof(mii));
		mii.cbSize = sizeof(mii);
		// owner drawn roots carry no stored string (the row rides in the
		// item data), so the label re-derives from the live row; the plain
		// string fallback rows still fill the buffer the classic way.
		mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_FTYPE | MIIM_DATA;
		mii.dwTypeData = _viv_menubar_item_text[_viv_menubar_item_count];
		mii.cch = STRING_SIZE - 1;
		
		if ((!GetMenuItemInfoW(_viv_hmenu,index,TRUE,&mii)) || (!mii.hSubMenu))
		{
			continue;
		}
		
		if ((mii.fType & MFT_OWNERDRAW) && (mii.dwItemData))
		{
			_viv_menu_row_item_text((void *)mii.dwItemData,_viv_menubar_item_text[_viv_menubar_item_count]);
		}
		
		size.cx = 0;
		size.cy = 0;
		
		// measure the drawn form (accelerators stripped): the hidden
		// ampersand never paints until alt is held.
		_viv_menubar_strip_ampersand(plain,_viv_menubar_item_text[_viv_menubar_item_count]);

		GetTextExtentPoint32W(hdc,plain,string_get_length(plain),&size);

		_viv_menubar_item_wide[_viv_menubar_item_count] = size.cx + (pad * 2);
		
		if (size.cy > high)
		{
			high = size.cy;
		}
		
		_viv_menubar_item_count++;
	}
	
	if (old_font)
	{
		SelectObject(hdc,old_font);
	}
	
	ReleaseDC(_viv_hwnd,hdc);
	
	if (_viv_menubar_item_count == 0)
	{
		return;
	}
	
	// the strip height: the label plus vertical air, floored at the classic
	// bar height so a failed metric query can never collapse the strip.
	// the strip height: the label plus vertical air, floored at the
	// remake bar height (28 dip) so a failed metric query can never
	// collapse the strip.
	min_high = (28 * os_logical_high) / 96;

	_viv_menubar_bar_high = high + ((6 * os_logical_high) / 96);
	
	if (_viv_menubar_bar_high < min_high)
	{
		_viv_menubar_bar_high = min_high;
	}
	
	// the hover indexes are stale after a re-read: the next mouse move
	// re-arms them.
	_viv_menubar_hover = -1;
	
	if (_viv_menubar_hwnd)
	{
		InvalidateRect(_viv_menubar_hwnd,0,FALSE);
	}
}

int _viv_menubar_high(void)
{
	if ((_viv_menubar_hwnd) && (IsWindowVisible(_viv_menubar_hwnd)))
	{
		return _viv_menubar_bar_high;
	}
	
	return 0;
}

void _viv_menubar_resize(int wide)
{
	if (_viv_menubar_hwnd)
	{
		SetWindowPos(_viv_menubar_hwnd,0,0,0,wide,_viv_menubar_bar_high,SWP_NOZORDER|SWP_NOACTIVATE);
	}
}

void _viv_menubar_repaint(void)
{
	if (_viv_menubar_hwnd)
	{
		InvalidateRect(_viv_menubar_hwnd,0,FALSE);
	}
}

// alt + mnemonic: the wm_syschar character against the '&' in each label.
int _viv_menubar_open_mnemonic(int key)
{
	int itemi;
	int i;
	int mnemonic;
	int ch;
	
	if ((key < '0') || (key > 'z'))
	{
		return 0;
	}
	
	ch = (key >= 'a') ? (key - ('a' - 'A')) : key;
	
	for(itemi=0;itemi<_viv_menubar_item_count;itemi++)
	{
		i = 0;
		
		while(_viv_menubar_item_text[itemi][i])
		{
			if (_viv_menubar_item_text[itemi][i] == L'&')
			{
				mnemonic = _viv_menubar_item_text[itemi][i + 1];
				
				if ((mnemonic >= 'a') && (mnemonic <= 'z'))
				{
					mnemonic -= ('a' - 'A');
				}
				
				if (mnemonic == ch)
				{
					_viv_menubar_open_popup(itemi);
					
					return 1;
				}
			}
			
			i++;
		}
	}
	
	return 0;
}

void _viv_menubar_open_first(void)
{
	_viv_menubar_open_popup(0);
}

static LRESULT CALLBACK _viv_menubar_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			RECT rect;
			HDC hdc;
			HFONT font;
			HFONT old_font;
			COLORREF text_color;
			HBRUSH face;
			int itemi;
			int inactive;
			int hot;
			int press;
			int show_accel;
			
			hdc = BeginPaint(hwnd,&ps);
			
			GetClientRect(hwnd,&rect);
			
			// the face is one flat fill: dark chrome in the dark ui, the
			// system menu face in the light ui. no system painter ever
			// touches this strip again - that was the white bar.
			FillRect(hdc,&rect,viv_theme_brush(VIV_TK_FRAME));
			
			font = _viv_menu_font();
			old_font = 0;
			
			if (font)
			{
				old_font = SelectObject(hdc,font);
			}
			
			SetBkMode(hdc,TRANSPARENT);
			
			inactive = GetActiveWindow() != _viv_hwnd;
			
			// the underline policy mirrors the system bar: hidden until
			// the alt key is held.
			show_accel = GetKeyState(VK_MENU) < 0;
			
			for(itemi=0;itemi<_viv_menubar_item_count;itemi++)
			{
				_viv_menubar_item_rect(itemi,&rect);
				
				// the pressed item is its own face: the mouse is down on the bar
				// and the popup has not taken over yet (the open item keeps the
				// hover face while its menu runs).
				press = (_viv_menubar_pressed && (itemi == _viv_menubar_hover) && (_viv_menubar_open == -1));
				hot = (itemi == _viv_menubar_hover) || (itemi == _viv_menubar_open);

				// one token path for both themes: the press item keeps its face

				// while the popup runs, the open item keeps the hover face, and

				// the inactive strip drops the labels a tone.

				face = press ? viv_theme_brush(VIV_TK_DOWN) : (hot ? viv_theme_brush(VIV_TK_HOVER) : viv_theme_brush(VIV_TK_FRAME));

				text_color = viv_theme_color(inactive ? VIV_TK_TEXT2 : VIV_TK_TEXT);

				
				FillRect(hdc,&rect,face);
				
				SetTextColor(hdc,text_color);
				
				DrawTextW(hdc,_viv_menubar_item_text[itemi],-1,&rect,DT_SINGLELINE | DT_CENTER | DT_VCENTER | (show_accel ? 0 : DT_HIDEPREFIX));
			}
			
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
			
			FillRect((HDC)wParam,&rect,viv_theme_brush(VIV_TK_FRAME));
			
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
				
				_viv_menubar_tracking = 1;
			}
			
			hit = _viv_menubar_hit_test(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
			
			if (hit != _viv_menubar_hover)
			{
				_viv_menubar_invalidate_item(_viv_menubar_hover);
				_viv_menubar_invalidate_item(hit);
				
			_viv_menubar_hover = hit;
			}

			// the follow: a popup is up and the mouse moved to a different
			// root - cancel the tracking loop, the open popup call reopens at
			// the new item.
			if ((_viv_menubar_open != -1) && (hit >= 0) && (hit != _viv_menubar_open) && (_viv_menubar_deferred == -1))
			{
				_viv_menubar_deferred = hit;

				InvalidateRect(_viv_menubar_hwnd,0,FALSE);
				UpdateWindow(_viv_menubar_hwnd);

				EndMenu();
			}

			return 0;
		}
		
		case WM_MOUSELEAVE:
		
			_viv_menubar_tracking = 0;
			
			// during a captured press the leave is stale: the release
			// handler owns the hover accounting.
			if (GetCapture() == hwnd)
			{
				return 0;
			}
			
			if (_viv_menubar_hover != -1)
			{
				_viv_menubar_invalidate_item(_viv_menubar_hover);
				
				_viv_menubar_hover = -1;
			}
			
			return 0;
		
		case WM_LBUTTONDOWN:
		{
			int hit;
			
			hit = _viv_menubar_hit_test(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
			
			if (hit >= 0)
			{
				// press and hold: the popup opens on the release (the
				// classic drag across the items then releases on the one
				// to open).
				_viv_menubar_pressed = 1;
				
				SetCapture(hwnd);
				
				if (hit != _viv_menubar_hover)
				{
					_viv_menubar_invalidate_item(_viv_menubar_hover);
					_viv_menubar_invalidate_item(hit);
					
					_viv_menubar_hover = hit;
				}
			}
			
			return 0;
		}
		
		case WM_LBUTTONUP:
		{
			int hit;
			
			if (_viv_menubar_pressed)
			{
				_viv_menubar_pressed = 0;
				
				ReleaseCapture();
				
				hit = _viv_menubar_hit_test(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
				
				if (hit >= 0)
				{
					_viv_menubar_open_popup(hit);
				}
				else
				{
					_viv_menubar_invalidate_item(_viv_menubar_hover);
					
					_viv_menubar_hover = -1;
				}
			}
			
			return 0;
		}
		
		case WM_CAPTURECHANGED:
		
			_viv_menubar_pressed = 0;
			
			return 0;
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

void _viv_menubar_show(int show)
{
	if (show)
	{
		if (!_viv_menubar_hwnd)
		{
			// no class brush: the strip face is painted by the erase and
			// the paint handlers above, a draw that bypasses them must
			// not erase white (the 1.1.03 lesson).
			os_RegisterClassEx(
				0,
				_viv_menubar_proc,
				0,
				LoadCursor(NULL,IDC_ARROW),
				NULL,
				"_VIV_MENUBAR",
				0);
			
			_viv_menubar_hwnd = os_CreateWindowEx(
				0,
				"_VIV_MENUBAR",
				"",
				WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE,
				0,0,0,0,
				_viv_hwnd,(HMENU)VIV_ID_MENUBAR,os_hinstance,NULL);
			
			_viv_menubar_layout();
		}
		else
		{
			ShowWindow(_viv_menubar_hwnd,SW_SHOW);
		}
	}
	else
	{
		if (_viv_menubar_hwnd)
		{
			ShowWindow(_viv_menubar_hwnd,SW_HIDE);
			
			_viv_menubar_hover = -1;
			_viv_menubar_open = -1;
		}
	}
	
	_viv_on_size();
}
