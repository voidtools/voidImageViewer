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
// viv_chrome.c - window dressing: rebar, toolbar, status bar, fullscreen, cursors.
// the menu bar strip is owned by viv_menubar.c (the remade top bar).
// Pure physical move from viv.c (R70 split): bodies are unchanged.
#include "viv.h"
#include "viv_state.h"
#include "viv_toolbar.h"
#include "viv_chrome.h"
#include "viv_dark.h"
#include "viv_menubar.h"
#include "viv_render.h"
#include "viv_view.h"

// forward declarations (order preserved from viv.c)
static LRESULT (CALLBACK *_viv_old_status_proc)(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) = NULL; // old status bar proc

void _viv_update_title(void);
void _viv_on_size(void);
HBRUSH _viv_dark_chrome_brush(int which);
int _viv_paint_begin(HDC hdc,int wide,int high);
void _viv_paint_kill(void);
void _viv_toggle_fullscreen(void);
HFONT _viv_menu_font(void);
void _viv_menu_font_drop(void);
void _viv_apply_dark_mode(int repaint);
int _viv_is_window_maximized(HWND hwnd);
void _viv_update_ontop(void);
void _viv_update_prevent_sleep(void);
void _viv_status_show(int show);
void _viv_controls_show(int show);
void _viv_status_update(void);
static void _viv_status_set(int part,const wchar_t *text);
int _viv_status_draw_item(DRAWITEMSTRUCT *draw_item);
int _viv_get_status_high(void);
int _viv_get_controls_high(void);
static LRESULT CALLBACK _viv_status_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static LRESULT CALLBACK _viv_status_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);



static LRESULT CALLBACK _viv_status_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)

{

	switch (msg) 

	{	

		case WM_ERASEBKGND:

		{

			RECT rect;

			

				// the comctl status class paints its own face from the light

				// palette even under the dark theme class (the field report: the

				// flat light slab under the dark canvas). the strip erases with

				// the dark face in the dark ui; the owner drawn panes paint over

				// it and the light ui keeps the native face.

				GetClientRect(hwnd,&rect);

				

				FillRect((HDC)wParam,&rect,_viv_is_dark() ? _viv_dialog_dark_brush() : (HBRUSH)(COLOR_BTNFACE + 1));

				

				return 1;

		}

		

		case WM_PAINT:

		{

			LRESULT result;

			

			// the control paints the panes, the sunken top edge and the

			// size grip. in the dark ui the edge and the grip are the two

			// bits left light: repaint them with the chrome palette after

			// the native pass, so the bottom band reads as one dark chrome.

			result = CallWindowProc(_viv_old_status_proc,hwnd,msg,wParam,lParam);

			

			if (_viv_is_dark())

			{

				RECT client_rect;

				RECT rect;

				HDC hdc;

				int wide;

				

				hdc = GetDC(hwnd);

				

				if (hdc)

				{

					GetClientRect(hwnd,&client_rect);

					wide = client_rect.right - client_rect.left;

					

					// the top edge: the same pair the rebar strip draws

					// (shadow over highlight), so the two bands read as one.

					rect.left = 0;

					rect.top = 0;

					rect.right = wide;

					rect.bottom = 1;

					FillRect(hdc,&rect,_viv_dark_chrome_brush(1));

					

					rect.top = 1;

					rect.bottom = 2;

					FillRect(hdc,&rect,_viv_dark_chrome_brush(2));

					

					// the size grip: the native one paints light dots. repaint

					// the box with the dark face and dot it ourselves, only when

					// the native one is up (the parent is resizable and not

					// maximized - the control suppresses it otherwise).

					if (((GetWindowLong(_viv_hwnd,GWL_STYLE)) & WS_THICKFRAME) && (!IsZoomed(_viv_hwnd)) && (!_viv_is_fullscreen))

					{

						RECT grip_rect;

						int grip_wide;

						int grip_high;

						int step;

						int dot_x;

						int dot_y;

						

						grip_wide = GetSystemMetrics(SM_CXVSCROLL);

						grip_high = GetSystemMetrics(SM_CYVSCROLL);

						

						if ((grip_wide > 0) && (grip_high > 0) && (grip_wide < wide) && (grip_high < (client_rect.bottom - client_rect.top)))

						{

							grip_rect.left = client_rect.right - grip_wide;

							grip_rect.top = client_rect.bottom - grip_high;

							grip_rect.right = client_rect.right;

							grip_rect.bottom = client_rect.bottom;

							

							FillRect(hdc,&grip_rect,_viv_dialog_dark_brush());

							

							step = (grip_wide < grip_high) ? (grip_wide / 6) : (grip_high / 6);

							

							if (step < 2)

							{

								step = 2;

							}

							

							// the classic triangle of dots anchored at the corner.

							for(dot_x=0;dot_x<3;dot_x++)

							{

								for(dot_y=0;dot_y<(3 - dot_x);dot_y++)

								{

									SetPixel(hdc,grip_rect.right - 2 - (dot_x * step),grip_rect.bottom - 2 - (dot_y * step),RGB(0x9A,0x9A,0x9A));

								}

							}

						}

					}

					

					ReleaseDC(hwnd,hdc);

				}

			}

			

			return result;

		}

		

		case WM_DRAWITEM:

		

			// the owner drawn panes (defensive route: see _viv_status_draw_item).

			if (_viv_status_draw_item((DRAWITEMSTRUCT *)lParam))

			{

				return 1;

			}

		

			break;

		

		case WM_CTLCOLOREDIT:

			// the zoom pane's in place editor: the strip palette owns the
			// colors so the field reads as part of the chrome, not a foreign
			// control. (the selection highlight stays the system color - the
			// edit control offers no custom one; it lives only while the
			// editor is open.)
			SetBkColor((HDC)wParam,_viv_is_dark() ? RGB(0x20,0x20,0x20) : (COLORREF)GetSysColor(COLOR_BTNFACE));
			SetTextColor((HDC)wParam,_viv_is_dark() ? RGB(0xE8,0xE8,0xE8) : (COLORREF)GetSysColor(COLOR_BTNTEXT));
			
			return (LRESULT)(_viv_is_dark() ? _viv_dialog_dark_brush() : GetSysColorBrush(COLOR_BTNFACE));
			


	}

	

	return CallWindowProc(_viv_old_status_proc,hwnd,msg,wParam,lParam);

}



static LRESULT CALLBACK _viv_fullscreen_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
void _viv_status_set_temp_text(wchar_t *text);
void _viv_status_update_temp_pos_zoom(void);
void _viv_status_update_slideshow_rate(void);
void _viv_zoomui_update(void);
void _viv_show_cursor(void);
void _viv_hide_cursor(void);
int _viv_should_show_cursor(void);
void _viv_update_show_cursor(void);
void _viv_start_hide_cursor_timer(void);


//static HWND _viv_tooltip_hwnd = 0;
static RECT _viv_fullscreen_rect;
static int _viv_fullscreen_zoom_offset = 0;
static BYTE _viv_prevent_on_size = 0; // don't process WM_SIZE changes.
static BYTE _viv_is_prevent_sleep = 0;
void _viv_update_title(void)
{
	wchar_t window_title[STRING_SIZE+STRING_SIZE];
	const wchar_t *filename;

	window_title[0] = 0;
	filename = L"";

	switch(config_title_bar_format)
	{
		case 0: // full path
			filename = _viv_current_fd->cFileName;
			break;

		case 1: // filename only
		default:
			filename = string_get_filename_part(_viv_current_fd->cFileName);
			break;
			
		case 2: // none
			break;
	}
	
	if (*filename)
	{
		string_cat(window_title,filename);
		string_cat_utf8(window_title," - ");
	}
		
	string_cat_utf8(window_title,localization_get_string(LOCALIZATION_ID_APP_NAME));
	
	SetWindowTextW(_viv_hwnd,window_title);
}
void _viv_on_size(void)
{
	if (!_viv_prevent_on_size)
	{
		// keep the zoom level inside the live ladder: a window resize changes
		// the best fit size and with it the reachable zoom range. (inside the
		// guard: fullscreen toggle intermediates must not re-clamp.)
		_viv_zoom_pos = _viv_clamp_zoom_pos(_viv_zoom_pos);
		
		RECT rect;
		int wide;
		int high;
		
		GetClientRect(_viv_hwnd,&rect);
		wide = rect.right - rect.left;
		high = rect.bottom - rect.top;

		if (!IsIconic(_viv_hwnd))
		{
			if (!_viv_is_fullscreen)
			{
				int is_maximized;
				
				is_maximized = _viv_is_window_maximized(_viv_hwnd);
				
				config_maximized = is_maximized;
				
				if (!is_maximized)
				{
					RECT window_rect;
					
					GetWindowRect(_viv_hwnd,&window_rect);
					
					config_wide = window_rect.right - window_rect.left;
					config_high = window_rect.bottom - window_rect.top;
				}
			}
		}
		
		// the top bar strip: a client side child now (the frame menu is
		// gone). it owns the top of the client area; everything below lays
		// out against the reduced height.
		if (_viv_menubar_high())
		{
			_viv_menubar_resize(wide);
			
			high -= _viv_menubar_high();
		}
		
		if (_viv_status_hwnd)
		{
			_viv_status_update();
			
			SendMessage(_viv_status_hwnd,WM_SIZE,0,0);
			
			high -= _viv_get_status_high();
		}

		if (_viv_toolbar_high())
		{
			// the strip anchors under the menu bar (the layout owns the y), so
			// the height math only asks the strip how tall it is.
			_viv_toolbar_layout(wide);

			high -= _viv_toolbar_high();
		}
			zoomui_layout(wide,high);
		
		{
		
			int rw;
			int rh;
			double new_view_x;
			double new_view_y;

			_viv_get_render_size(&rw,&rh);
		
			new_view_x = (int)(((_viv_view_ix * rw) / _viv_image_wide) + 0.5) + (((_viv_dst_pos_x - 250) * (wide*2)) / 1000) - (wide / 2) - (rw / 2);
			new_view_y = (int)(((_viv_view_iy * rh) / _viv_image_high) + 0.5) + (((_viv_dst_pos_y - 250) * (high*2)) / 1000) - (high / 2) - (rh / 2);
	
	debug_printf("RESTORE VIEW x %d y %d ix %d iy %d rw %d rh %d wide %d high %d\n",(int)(new_view_x),(int)(new_view_y),(int)_viv_view_ix,(int)_viv_view_iy,rw,rh,wide,high)		;
	//		_viv_view_set((int)(_viv_view_ix * (double)rw),(int)(_viv_view_iy * (double)rh),1);
			_viv_view_set((int)new_view_x,(int)new_view_y,1);
		}

		_viv_toolbar_update_buttons();
	}
}
static HBITMAP _viv_paint_hbitmap = 0;
static HGDIOBJ _viv_paint_last_hbitmap = 0;
static int _viv_paint_wide = 0;
static int _viv_paint_high = 0;
HBRUSH _viv_dark_chrome_brush(int which)
{
	// the four chrome faces live in the shared theme cache now: the
	// indices map to face, line, muted and frame, so a theme or accent
	// flip re-skins every legacy dark surface for free.
	static const int tokens[4] = {VIV_TK_CHROME,VIV_TK_CHROME_LINE,VIV_TK_CHROME_MUTED,VIV_TK_FRAME};
	
	if ((which < 0) || (which > 3))
	{
		return 0;
	}
	
	return viv_theme_brush(tokens[which]);
}
// prepare the paint backbuffer for a client area of wide x high pixels.
// returns 1 when the caller should draw into _viv_paint_hdc instead of the screen dc.
int _viv_paint_begin(HDC hdc,int wide,int high)
{
	if (_viv_paint_hdc)
	{
		// reuse the existing bitmap when the client size has not changed.
		if ((wide == _viv_paint_wide) && (high == _viv_paint_high))
		{
			return 1;
		}
		
		// the size changed, discard the old bitmap.
		if (_viv_paint_last_hbitmap)
		{
			SelectObject(_viv_paint_hdc,_viv_paint_last_hbitmap);
			_viv_paint_last_hbitmap = 0;
		}
		
		DeleteObject(_viv_paint_hbitmap);
		_viv_paint_hbitmap = 0;
		
		_viv_paint_wide = 0;
		_viv_paint_high = 0;
	}
	else
	{
		_viv_paint_hdc = CreateCompatibleDC(hdc);
		
		if (!_viv_paint_hdc)
		{
			return 0;
		}
	}
	
	if ((wide <= 0) || (high <= 0))
	{
		return 0;
	}
	
	_viv_paint_hbitmap = CreateCompatibleBitmap(hdc,wide,high);
	
	if (!_viv_paint_hbitmap)
	{
		return 0;
	}
	
	_viv_paint_last_hbitmap = SelectObject(_viv_paint_hdc,_viv_paint_hbitmap);
	
	if (!_viv_paint_last_hbitmap)
	{
		DeleteObject(_viv_paint_hbitmap);
		_viv_paint_hbitmap = 0;
		
		return 0;
	}
	
	_viv_paint_wide = wide;
	_viv_paint_high = high;
	
	return 1;
}
// free the cached paint backbuffer.
void _viv_paint_kill(void)
{
	if (_viv_paint_hdc)
	{
		if (_viv_paint_last_hbitmap)
		{
			SelectObject(_viv_paint_hdc,_viv_paint_last_hbitmap);
			_viv_paint_last_hbitmap = 0;
		}
		
		DeleteDC(_viv_paint_hdc);
		_viv_paint_hdc = 0;
	}
	
	if (_viv_paint_hbitmap)
	{
		DeleteObject(_viv_paint_hbitmap);
		_viv_paint_hbitmap = 0;
	}
	
	_viv_paint_wide = 0;
	_viv_paint_high = 0;
}
void _viv_toggle_fullscreen(void)
{
	DWORD style;
	int old_rw;
	int old_rh;
	int zoom_wide_array[_VIV_ZOOM_MAX];
	int zoom_high_array[_VIV_ZOOM_MAX];
	
	_viv_get_render_size(&old_rw,&old_rh);

	// precalculate all zoom levels for comparison later to find the zoom offset.
	{
		int backup_zoom;

		backup_zoom = _viv_zoom_pos;
		
		for(_viv_zoom_pos=0;_viv_zoom_pos<_VIV_ZOOM_MAX;_viv_zoom_pos++)
		{
			int rw;
			int rh;
			
			_viv_get_render_size(&rw,&rh);
			
			zoom_wide_array[_viv_zoom_pos] = rw;
			zoom_high_array[_viv_zoom_pos] = rh;
		}
		
		_viv_zoom_pos = backup_zoom;
	}
				
	_viv_prevent_on_size = 1;

debug_printf("toggle fullscreen %d\n",!_viv_is_fullscreen);

	style = GetWindowLong(_viv_hwnd,GWL_STYLE);
	
	_viv_1to1 = 0;
	
	if (_viv_is_fullscreen)
	{
		// set before restore
		// otherwise get_render_size will return the wrong size.
		_viv_is_fullscreen = 0;

		if (config_show_caption)	
		{
			style |= WS_CAPTION;
		}
		else
		{
			style &= ~(WS_CAPTION);
		}
		
		if (config_show_thickframe)	
		{
			style |= WS_THICKFRAME;
		}
		else
		{
			style &= ~WS_THICKFRAME;
		}

		_viv_menubar_show(config_show_menu);
		
		_viv_status_show(config_show_status);
		_viv_controls_show(config_show_controls);
		_viv_zoomui_update();
		
		SetWindowLong(_viv_hwnd,GWL_STYLE,style);

		SetWindowPos(_viv_hwnd,HWND_TOP,_viv_fullscreen_rect.left,_viv_fullscreen_rect.top,_viv_fullscreen_rect.right - _viv_fullscreen_rect.left,_viv_fullscreen_rect.bottom - _viv_fullscreen_rect.top,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOCOPYBITS);
	
		if (_viv_fullscreen_is_maxed)
		{	
			ShowWindow(_viv_hwnd,SW_MAXIMIZE);
		}
		
		_viv_zoom_pos += _viv_fullscreen_zoom_offset;
		_viv_zoom_pos = _viv_clamp_zoom_pos(_viv_zoom_pos);
	}
	else
	{
		RECT monitor_rect;
			
		// set fullscreen before we resize.
		_viv_is_fullscreen = 1;
			
		os_MonitorRectFromWindow(_viv_hwnd,1,&monitor_rect);
		
		SetWindowLong(_viv_hwnd,GWL_STYLE,style & ~(WS_CAPTION|WS_THICKFRAME));
		
		_viv_fullscreen_is_maxed = IsZoomed(_viv_hwnd);
		if (_viv_fullscreen_is_maxed)
		{
			ShowWindow(_viv_hwnd,SW_SHOWNORMAL);
		}
		
		GetWindowRect(_viv_hwnd,&_viv_fullscreen_rect);
		
		_viv_menubar_show(0);
		_viv_status_show(0);
		_viv_controls_show(0);
		_viv_zoomui_update();

		SetWindowPos(_viv_hwnd,HWND_TOP,monitor_rect.left,monitor_rect.top,monitor_rect.right - monitor_rect.left,monitor_rect.bottom - monitor_rect.top,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOCOPYBITS);

		_viv_prevent_on_deactivate = 1;

		// create a dummy fullscreen window and destroy it immediately
		// this window is created fullscreen correctly, and when our normal window gains focus it will keep the fullscreen setting (no taskbar)
		// without this dummy window, sometimes the taskbar will not disappear.
		
		{
			HWND fullscreen_hwnd;
			
			// Initialize global strings
			os_RegisterClassEx(
				0,
				_viv_fullscreen_proc,
				0,
				LoadCursor(NULL,IDC_ARROW),
				(HBRUSH)(COLOR_BTNFACE+1),
				"_VIV_FULLSCREEN",
				0);

			fullscreen_hwnd = os_CreateWindowEx(
				0,
				"_VIV_FULLSCREEN",
				localization_get_string(LOCALIZATION_ID_APP_NAME),
				WS_POPUP|WS_VISIBLE,
				monitor_rect.left,monitor_rect.top,monitor_rect.right - monitor_rect.left,monitor_rect.bottom - monitor_rect.top,
				0,0,os_hinstance,NULL);

			SetForegroundWindow(fullscreen_hwnd);

			DestroyWindow(fullscreen_hwnd);
		}

		// find zoom offset.
		{
			int zoom_index;
			
			_viv_fullscreen_zoom_offset = 0;
			
			if (config_fullscreen_fill_window)
			{
				for(zoom_index=0;zoom_index<_VIV_ZOOM_MAX;zoom_index++)
				{
					// debug_printf("%d %d\n",zoom_wide_array[zoom_index] , monitor_rect.right - monitor_rect.left);
				
					if ((zoom_wide_array[zoom_index] > monitor_rect.right - monitor_rect.left) || (zoom_high_array[zoom_index] > monitor_rect.bottom - monitor_rect.top))
					{
						break;
					}
							
					_viv_fullscreen_zoom_offset = zoom_index;
				}	
				
				if (_viv_fullscreen_zoom_offset > _viv_zoom_pos)
				{
					_viv_fullscreen_zoom_offset = _viv_zoom_pos;
				}
			}
			else
			if (config_fill_window)
			{
				int backup_zoom_pos;
				
				backup_zoom_pos = _viv_zoom_pos;
				
				for(_viv_zoom_pos=0;_viv_zoom_pos<_VIV_ZOOM_MAX;_viv_zoom_pos++)
				{
					int rw;
					int rh;
					
					_viv_get_render_size(&rw,&rh);
					
					// debug_printf("%d %d\n",zoom_wide_array[zoom_index] , monitor_rect.right - monitor_rect.left);
				
					if ((rw > old_rw) || (rh > old_rh))
					{
						break;
					}
							
					_viv_fullscreen_zoom_offset = -_viv_zoom_pos;
				}

				_viv_zoom_pos = backup_zoom_pos;
				
				if (_viv_fullscreen_zoom_offset < -(_VIV_ZOOM_MAX-1-_viv_zoom_pos))
				{
					_viv_fullscreen_zoom_offset = -(_VIV_ZOOM_MAX-1-_viv_zoom_pos);
				}
			}
			
//			debug_printf("ZOOM OFFSET %d\n",_viv_fullscreen_zoom_offset);
		}
	
		_viv_zoom_pos -= _viv_fullscreen_zoom_offset;
		_viv_zoom_pos = _viv_clamp_zoom_pos(_viv_zoom_pos);
		
		_viv_prevent_on_deactivate = 0;
	}
	
	_viv_prevent_on_size = 0;

	_viv_on_size();
	
//	InvalidateRect(_viv_hwnd,0,FALSE);

	// update mouseover
	// so we show the cursor corectly in
	// _viv_update_show_cursor
	{
		POINT cursor_pt;
		HWND hwnd_mouseover;
		int is_mouseover;
		
		GetCursorPos(&cursor_pt);
		is_mouseover = 0;
		
		hwnd_mouseover = WindowFromPoint(cursor_pt);
		if (hwnd_mouseover == _viv_hwnd)
		{
			RECT client_rect;
			
			ScreenToClient(_viv_hwnd,&cursor_pt);
			
			GetClientRect(_viv_hwnd,&client_rect);
			
			if (PtInRect(&client_rect,cursor_pt))
			{
				is_mouseover = 1;
			}
		}
		
		_viv_is_mouseover = is_mouseover;
	}

	_viv_update_show_cursor();
}
static HFONT _viv_menu_font_handle = 0; // the cached menu bar font
static int _viv_menu_font_dpi = 0; // the dpi the menu bar font was created for
// the menu bar font: the system menu font at the window's current dpi,
// cached until the dpi or the theme changes. freed with the process.
HFONT _viv_menu_font(void)
{
	if ((!_viv_menu_font_handle) || (_viv_menu_font_dpi != os_logical_wide))
	{
		LOGFONTW lf;
		
		if (_viv_menu_font_handle)
		{
			DeleteObject(_viv_menu_font_handle);
			
			_viv_menu_font_handle = 0;
		}
		
		if (os_menu_font(&lf))
		{
			_viv_menu_font_handle = CreateFontIndirectW(&lf);
			
			_viv_menu_font_dpi = os_logical_wide;
		}
	}
	
	return _viv_menu_font_handle;
}
// drop the cached menu font (the dpi or the theme changed: the system
// metrics may have followed).
void _viv_menu_font_drop(void)
{
	if (_viv_menu_font_handle)
	{
		DeleteObject(_viv_menu_font_handle);
		
		_viv_menu_font_handle = 0;
	}
	
	_viv_menu_font_dpi = 0;
}
// apply the dark chrome to the main window: frame (title bar), status bar
// and zoom controls. the menu theme was set app wide before the first
// window was created (os_dark_set_app_mode).
void _viv_apply_dark_mode(int repaint)
{
	int dark;
	
	dark = _viv_is_dark();
	
	// the token layer re-resolves before anything repaints from it: the
	// cached brushes follow the flip (every surface that paints through
	// viv_theme_brush rides this, and the auto flip paths used to leave
	// the cache on the old palette - the half-skinned ui the field kept
	// catching).
	viv_theme_refresh();
	
	{
		HWND menu_hwnd;

		// the #32768 class holds a token brush; the refresh just deleted
		// it. an open popup would erase with a dead handle - re-install
		// now (every menu open re-installs it again anyway).
		menu_hwnd = FindWindowW(L"#32768",0);

		if (menu_hwnd)
		{
			SetClassLongPtrW(menu_hwnd,GCLP_HBRBACKGROUND,(LONG_PTR)viv_theme_brush(VIV_TK_FACE));
		}
	}
	
	os_dark_titlebar(_viv_hwnd,dark);
	
	// windows 11 chrome: rounded corners and a caption color that
	// matches the canvas. a silent no-op on windows 10 and older.
	os_window_modern_chrome(_viv_hwnd,_viv_windowed_background());
	
	// the common controls follow the immersive dark flag per window:
	// flag the control windows too so the status bar and the toolbars
	// retheme natively (pre windows 10 1903 the flags are no-ops and the
	// owner drawn panes / our strip painting carry the dark ui alone).
	if (_viv_status_hwnd)
	{
		os_dark_titlebar(_viv_status_hwnd,dark);
		
		// the status bar follows the theme both ways: the dark explorer
		// class in the dark ui, the light class back on the flip (the bar
		// would otherwise keep carrying the dark class in the light ui).
		if (dark)
		{
			os_dark_window_theme(_viv_status_hwnd);
		}
		else
		{
			os_light_window_theme(_viv_status_hwnd);
		}
		
		InvalidateRect(_viv_status_hwnd,0,FALSE);
	}
	
	
	// the strip repaints through its own latch (the metrics are theme
	// independent, so a flip never re-measures).
	_viv_toolbar_set_dark(dark);
	
	// the remade menu bar repaints on the flip: its layout is theme
	// independent, so no re-measure and no owner draw toggling.
	_viv_menubar_repaint();
	
	// the strip metrics are theme independent: the on size sweep right
	// below re-lays the strip against the current client width.
	_viv_on_size();
	
	// the open dialogs re-theme live: the options dialog is usually on
	// screen when its own dark mode combo changes the setting.
	_viv_dark_dialogs_refresh();
	
	// retheme the menu bar: a frame change repaints the non client area
	// after the app mode switch (the menus retheme on the next open).
	SetWindowPos(_viv_hwnd,0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
	
	zoomui_set_dark(dark);
	
	
	// every flip repaints the whole window: the per-control
	// InvalidateRect calls above cover their own windows, but a flip
	// that lands between partial paints left stale light pixels on the
	// rebar strip and the client canvas (the field report: the white
	// band right of the toolbar until a manual resize or refresh). the
	// startup call is free - nothing has painted yet.
	InvalidateRect(_viv_hwnd,0,FALSE);
	
	if (repaint)
	{
		// forced repaint callers also sweep the children now instead of
		// waiting for the message loop. the frame joins the sweep: without
		// rdw_frame the official semantics keep the non client area out of
		// the immediate update, the pending wm_ncpaint gets consumed without
		// painting, the dark gap fill that lives in that message never runs
		// and the menu bar keeps the system light strip until a manual resize
		// or refresh - the white band the field caught on the flip.
		RedrawWindow(_viv_hwnd,0,0,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_FRAME);
	}
}
int _viv_is_window_maximized(HWND hwnd)
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
void _viv_update_ontop(void)
{
	int is_top_most;
	
	is_top_most = 0;
	
	switch(config_ontop)
	{
		case 1:
			is_top_most = 1;
			break;
			
		case 2:
			is_top_most = (_viv_is_slideshow) || ((_viv_frame_count > 1) && (_viv_animation_play));
			break;
	}

	SetWindowPos(_viv_hwnd,is_top_most ? HWND_TOPMOST : HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
}
void _viv_update_prevent_sleep(void)
{
	BYTE _is_prevent_sleep;
	
	_is_prevent_sleep = 0;
	
	if (config_prevent_sleep)
	{
		if ((_viv_is_slideshow) || (_viv_is_animation_timer))
		{
			_is_prevent_sleep = 1;
		}
	}
	
	// _is_prevent_sleep changed?
	if (!!_viv_is_prevent_sleep != !!_is_prevent_sleep)
	{
		if (os_SetThreadExecutionState)
		{	
			os_SetThreadExecutionState(_is_prevent_sleep ? (ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED) : ES_CONTINUOUS);
		}
		
		_viv_is_prevent_sleep = _is_prevent_sleep;
	}
}
void _viv_status_show(int show)
{
	if (show)
	{
		if (!_viv_status_hwnd)
		{
			// WS_EX_COMPOSITED stops flickering issues
			_viv_status_hwnd = os_CreateWindowEx(
				WS_EX_COMPOSITED,
				STATUSCLASSNAMEA,
				"",
				WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | SBARS_SIZEGRIP | WS_VISIBLE,
				0,0,0,0,
				_viv_hwnd,(HMENU)VIV_ID_STATUS,os_hinstance,NULL);
				
			_viv_old_status_proc = os_set_window_proc(_viv_status_hwnd,_viv_status_proc);
			
			// a fresh bar starts with empty panes: flush the text store
			// (handles can be recycled, so the identity check alone is not
			// enough).
			os_zero_memory(_viv_status_part_text,sizeof(_viv_status_part_text));
			
			// pick up the current dark flags: the bar can be created long
			// after the last theme switch.
			if (_viv_is_dark())
			{
				os_dark_titlebar(_viv_status_hwnd,1);
				
				os_dark_window_theme(_viv_status_hwnd);
			}
		}
	}
	else
	{
		if (_viv_status_hwnd)
		{
			DestroyWindow(_viv_status_hwnd);

			_viv_status_hwnd = 0;

			os_zero_memory(_viv_status_part_text,sizeof(_viv_status_part_text));
		}	
	}
	
	_viv_on_size();
}
void _viv_controls_show(int show)
{
	if (show)
	{
		if (!_viv_toolbar_high())
		{
			// the remade strip: one self drawn window, the comctl toolbar,
			// its image list and the pinned widths are gone.
			_viv_toolbar_create(_viv_hwnd);
		}
	}
	else
	{
		_viv_toolbar_destroy();
	}

	_viv_on_size();
}
// the playlist position of the loaded file, for the index pane. the walk
// is cached on the fd pointer, so a steady image costs one compare.
static int _viv_status_nav_index(void)
{
	static const WIN32_FIND_DATA *last_fd;
	static int last_index;
	_viv_playlist_t *item;
	int index;

	if (_viv_playlist_count <= 1)
	{
		return 0;
	}

	if ((last_fd == _viv_frame_fd) && (last_index))
	{
		return last_index;
	}

	index = 1;

	for(item=_viv_playlist_start;item;item=item->next,index++)
	{
		if ((&item->fd == _viv_frame_fd) || (string_compare(item->fd.cFileName,_viv_frame_fd->cFileName) == 0))
		{
			last_fd = _viv_frame_fd;
			last_index = index;

			return index;
		}
	}

	last_fd = _viv_frame_fd;
	last_index = 0;

	return 0;
}

void _viv_status_update(void)
{
	if (_viv_status_hwnd)
	{
		int part_array[_VIV_STATUS_PART_MAX];
		RECT rect;
		wchar_t widebuf[STRING_SIZE];
		wchar_t highbuf[STRING_SIZE];
		wchar_t dimension_buf[STRING_SIZE];
		wchar_t frame_buf[STRING_SIZE];
		wchar_t pixel_pos_buf[STRING_SIZE];
		wchar_t pixel_rgb_buf[STRING_SIZE];
		wchar_t preload_buf[STRING_SIZE];
		wchar_t zoom_buf[STRING_SIZE];
		wchar_t date_buf[STRING_SIZE];
		HDC hdc;
		int dimension_wide;
		int frame_wide;
		int preload_wide;
		int pixel_pos_wide;
		int pixel_rgb_wide;
		int zoom_wide;
		int date_wide;
		int minwide;
		
		GetClientRect(_viv_hwnd,&rect);
		
		*zoom_buf = 0;
		*preload_buf = 0;
		*pixel_pos_buf = 0;
		*pixel_rgb_buf = 0;
		
		if ((_viv_image_wide) && (_viv_image_high))
		{
			string_format_number(widebuf,_viv_image_wide);
			string_format_number(highbuf,_viv_image_high);
		
			string_copy(dimension_buf,widebuf);
			string_cat_utf8(dimension_buf,(const utf8_t *)" x ");
			string_cat(dimension_buf,highbuf);
		}
		else
		{
			string_copy_utf8_string(dimension_buf,(const utf8_t *)"");
		}

		// the zoom pane text. shown whenever an image is loaded.
		if ((_viv_image_wide) && (_viv_image_high))
		{
			string_printf(zoom_buf,localization_get_string(LOCALIZATION_ID_STATUS_BAR_POS_ZOOM_FORMAT),_viv_zoom_percent());
		}

		if (*_viv_frame_fd->cFileName)
		{
			LARGE_INTEGER size;
			
			size.HighPart = _viv_frame_fd->nFileSizeHigh;
			size.LowPart = _viv_frame_fd->nFileSizeLow;
			
			if (size.QuadPart)
			{
				NUMBERFMT numberfmt;
				localization_id_t size_unit_id;
				
				if (*dimension_buf)
				{
					string_cat_utf8(dimension_buf," ");
				}
				
				string_cat_utf8(dimension_buf,"(");
				
				// adaptive size units: a 4 gb scan used to show as a row of comma
				// separated kilobytes. pick b / kb / mb / gb by magnitude, with
				// one decimal place above a megabyte (integer math only).
				if (size.QuadPart < 1024)
				{
					string_format_number(widebuf,size.QuadPart);
					
					size_unit_id = LOCALIZATION_ID_STATUS_BAR_SIZE_BYTES_FORMAT;
				}
				else
				if (size.QuadPart < ((LONGLONG)1024 * 1024))
				{
					string_format_number(widebuf,(size.QuadPart + 1023) / 1024);
					
					size_unit_id = LOCALIZATION_ID_STATUS_BAR_SIZE_KB_FORMAT;
				}
				else
				{
					VIV_UINT64 divisor;
					VIV_UINT64 tenths;
					DWORD whole;
					DWORD frac;
					
					divisor = (size.QuadPart < ((LONGLONG)1024 * 1024 * 1024)) ? ((VIV_UINT64)1024 * 1024) : ((VIV_UINT64)1024 * 1024 * 1024);
					
					size_unit_id = (divisor == ((VIV_UINT64)1024 * 1024)) ? LOCALIZATION_ID_STATUS_BAR_SIZE_MB_FORMAT : LOCALIZATION_ID_STATUS_BAR_SIZE_GB_FORMAT;
					
					// one decimal place, rounded: (bytes * 10 + divisor / 2) / divisor
					tenths = (((VIV_UINT64)size.QuadPart * 10) + (divisor / 2)) / divisor;
					whole = (DWORD)(tenths / 10);
					frac = (DWORD)(tenths % 10);
					
					string_format_number(widebuf,whole);
					string_cat_utf8(widebuf,(const utf8_t *)".");
					widebuf[string_get_length(widebuf)] = (wchar_t)('0' + frac);
					widebuf[string_get_length(widebuf) + 1] = 0;
				}
				
				numberfmt.NumDigits = (size.QuadPart < ((LONGLONG)1024 * 1024)) ? 0 : 1;
				numberfmt.LeadingZero = 0;
				numberfmt.Grouping = 3;
				numberfmt.lpThousandSep = L",";
				numberfmt.lpDecimalSep = L".";
				numberfmt.NegativeOrder = 0;
				
				GetNumberFormat(LOCALE_USER_DEFAULT,0,widebuf,&numberfmt,highbuf,STRING_SIZE);
				
				string_cat(dimension_buf,highbuf);
				string_cat_utf8(dimension_buf,localization_get_string(size_unit_id));
				string_cat_utf8(dimension_buf,")");
			}
		}
		
		if (_viv_frame_count > 1)
		{
			int frame_pos;
			string_format_number(highbuf,_viv_frame_count);

			string_copy_utf8_string(frame_buf,(const utf8_t *)"");
			
			if (config_frame_minus)
			{
				frame_pos = _viv_frame_count - (_viv_frame_position);
				string_cat_utf8(frame_buf,(const utf8_t *)"- ");
			}
			else
			{
				frame_pos = _viv_frame_position + 1;
			}

			string_format_number(widebuf,frame_pos);
			
			string_cat(frame_buf,widebuf);
			string_cat_utf8(frame_buf,(const utf8_t *)" / ");
			string_cat(frame_buf,highbuf);
		}
		else
		{
			string_copy_utf8_string(frame_buf,(const utf8_t *)"");
		}
		
		// this is just noise..
		
		if ((_viv_load_is_preload) && (_viv_preload_state == 0) && (!_viv_should_activate_preload_on_load) && (!_viv_load_image_terminate) && (!_viv_preload_frame_loaded_count))
		{
			string_copy_utf8_string(preload_buf,localization_get_string(LOCALIZATION_ID_STATUS_BAR_PRELOAD));
		}

		// the file time pane: the local last-modified stamp of the loaded
		// file (the remake status row: rgb and the date at the right edge).
		*date_buf = 0;

		if (*_viv_frame_fd->cFileName)
		{
			FILETIME local_filetime;
			SYSTEMTIME systemtime;

			if (FileTimeToLocalFileTime(&_viv_frame_fd->ftLastWriteTime,&local_filetime) &&
			    FileTimeToSystemTime(&local_filetime,&systemtime))
			{
				wchar_t time_buf[64];
				int date_len;

				// the localized stamp: the system formatters take wide
				// formats (the old wide-literal printf went through the
				// narrow parser and rendered a lone "0").
				date_buf[0] = 0;

				GetDateFormatW(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&systemtime,0,date_buf,STRING_SIZE - 1);

				date_len = string_get_length(date_buf);

				if ((date_len) && (date_len < STRING_SIZE - 2))
				{
					date_buf[date_len] = L' ';
					date_buf[date_len + 1] = 0;
				}

				GetTimeFormatW(LOCALE_USER_DEFAULT,TIME_NOSECONDS,&systemtime,0,time_buf,64);

				string_cat(date_buf,time_buf);
			}
		}

		if (config_pixel_info)
		{
			if ((_viv_src_pixel_x >= 0) && (_viv_src_pixel_y >= 0))
			{		
				string_printf(pixel_pos_buf,"POS: %d,%d",_viv_src_pixel_x,_viv_src_pixel_y);
				string_printf(pixel_rgb_buf,localization_get_string(LOCALIZATION_ID_STATUS_BAR_RGB_FORMAT),_viv_src_pixel_r,_viv_src_pixel_g,_viv_src_pixel_b);
			}
		}
		
		date_wide = 0;
		dimension_wide = 0;
		frame_wide = 0;
		preload_wide = 0;
		pixel_pos_wide = 0;
		pixel_rgb_wide = 0;
		zoom_wide = 0;
		minwide = (72 * os_logical_wide) / 96;

		hdc = GetDC(_viv_status_hwnd);
		if (hdc)
		{
			HFONT hfont;
			
			hfont = (HFONT)SendMessage(_viv_status_hwnd,WM_GETFONT,0,0);
			if (hfont)
			{
				SIZE size;
				HGDIOBJ last_font;
				
				last_font = SelectObject(hdc,hfont);
				
				if (GetTextExtentPoint32(hdc,dimension_buf,string_get_length(dimension_buf),&size))
				{
					dimension_wide = size.cx + GetSystemMetrics(SM_CXEDGE) * 5;
					
					if (dimension_wide < minwide)
					{
						dimension_wide = minwide;
					}
				}

				if (*frame_buf)
				{
					if (GetTextExtentPoint32(hdc,frame_buf,string_get_length(frame_buf),&size))
					{
						frame_wide = size.cx + GetSystemMetrics(SM_CXEDGE) * 5;
						
						if (frame_wide < minwide)
						{
							frame_wide = minwide;
						}
					}
				}

				if (*preload_buf)
				{
					if (GetTextExtentPoint32(hdc,preload_buf,string_get_length(preload_buf),&size))
					{
						preload_wide = size.cx + GetSystemMetrics(SM_CXEDGE) * 5;
					}
				}

				if (*pixel_pos_buf)
				{
					if (GetTextExtentPoint32(hdc,pixel_pos_buf,string_get_length(pixel_pos_buf),&size))
					{
						pixel_pos_wide = size.cx + GetSystemMetrics(SM_CXEDGE) * 5;
					}
				}

				if (*pixel_rgb_buf)
				{
					if (GetTextExtentPoint32(hdc,pixel_rgb_buf,string_get_length(pixel_rgb_buf),&size))
					{
						pixel_rgb_wide = size.cx + GetSystemMetrics(SM_CXEDGE) * 5;
					}
				}
				if (*zoom_buf)
				{
					if (GetTextExtentPoint32(hdc,zoom_buf,string_get_length(zoom_buf),&size))
					{
						zoom_wide = size.cx + GetSystemMetrics(SM_CXEDGE) * 5;
						
						if (zoom_wide < minwide)
						{
							zoom_wide = minwide;
						}
					}
				}

				// the stamp measures on its own: a failed load with a
				// filename still owns the date pane (it used to hide
				// behind the zoom pane's block).
				if (*date_buf)
				{
					if (GetTextExtentPoint32(hdc,date_buf,string_get_length(date_buf),&size))
					{
						date_wide = size.cx + GetSystemMetrics(SM_CXEDGE) * 5;
					}
				}

				SelectObject(hdc,last_font);
			}
			
			ReleaseDC(_viv_status_hwnd,hdc);
		}
		
		// add size box
		dimension_wide += GetSystemMetrics(SM_CXVSCROLL) + GetSystemMetrics(SM_CXBORDER);
		
		{
			int parti;
			int avail_wide;
			int flex_wide;
			
			parti = 0;
			
			avail_wide = rect.right - rect.left;
			
			if (avail_wide < 0)
			{
				avail_wide = 0;
			}
			
			// the preload pane joins the zoom pane in the left cluster only
			// while a preload is actually running, and is capped at a quarter
			// of the bar: a long localization must never eat the message pane.
			if (preload_wide > (avail_wide / 4))
			{
				preload_wide = avail_wide / 4;
			}
			
			// the resolution pane is pinned to the bottom right and is never
			// dropped. when a huge image (or a tiny window) makes the panes
			// wider than the bar, the optional right panes give way, widest
			// last, so the dimension text clips instead of vanishing off the
			// edge of the window.
			while ((zoom_wide + preload_wide + dimension_wide + frame_wide + pixel_pos_wide + pixel_rgb_wide + date_wide > avail_wide)
			&& (frame_wide || pixel_rgb_wide || pixel_pos_wide || date_wide))
			{
				if (frame_wide)
				{
					frame_wide = 0;
				}
				else
				if (pixel_rgb_wide)
				{
					pixel_rgb_wide = 0;
				}
				else
				if (date_wide)
				{
					date_wide = 0;
				}
				else
				{
					pixel_pos_wide = 0;
				}
			}
			
			if (zoom_wide + preload_wide + dimension_wide > avail_wide)
			{
				// last resort: clip the resolution pane itself. it stays
				// visible at the bottom right with an ellipsized text.
				dimension_wide = avail_wide - zoom_wide - preload_wide;
				
				if (dimension_wide < 0)
				{
					dimension_wide = 0;
				}
			}
			
			// pane 0: the zoom pane (the in place zoom editor's home).
			part_array[parti] = zoom_wide;
			parti++;
			
			// pane 1 (when preloading): the preload indicator, on the left.
			if (preload_wide)
			{
				part_array[parti] = part_array[parti - 1] + preload_wide;
				parti++;
			}
			
			// the message pane flexes between the left and right clusters.
			flex_wide = avail_wide - zoom_wide - preload_wide - dimension_wide - frame_wide - pixel_pos_wide - pixel_rgb_wide - date_wide;
			
			if (flex_wide < 0)
			{
				flex_wide = 0;
			}
			
			part_array[parti] = part_array[parti - 1] + flex_wide;
			parti++;
			
			if (pixel_pos_wide)
			{
				part_array[parti] = part_array[parti - 1] + pixel_pos_wide;
				parti++;
			}
			
			if (pixel_rgb_wide)
			{
				part_array[parti] = part_array[parti - 1] + pixel_rgb_wide;
				parti++;
			}
			
			if (frame_wide)
			{
				part_array[parti] = part_array[parti - 1] + frame_wide;
				parti++;
			}
			
			if (date_wide)
			{
				part_array[parti] = part_array[parti - 1] + date_wide;
				parti++;
			}

			// the last pane: the file date, pinned to the bottom right (a
			// -1 right edge extends to the window edge).
			// the last pane: the resolution, pinned to the bottom right (a
			// -1 right edge extends to the window edge).
			part_array[parti] = -1;
			parti++;
			
			SendMessage(_viv_status_hwnd,SB_SETPARTS,parti,(LPARAM)part_array);
		}

		{
			wchar_t *text;
			wchar_t text_buf[STRING_SIZE];
			
			text =  L"";
			
			if (_viv_status_temp_text)
			{
				text = _viv_status_temp_text;
			}
			else
			if ((_viv_load_image_thread) && ((!_viv_load_is_preload) || (_viv_should_activate_preload_on_load)))
			{
				string_copy_utf8_string(text_buf,localization_get_string(LOCALIZATION_ID_STATUS_BAR_LOADING));
				text = text_buf;
			}
			else
			if (_viv_file_not_found)
			{
				string_copy_utf8_string(text_buf,localization_get_string(LOCALIZATION_ID_STATUS_BAR_FILE_NOT_FOUND));
				text = text_buf;
			}
			else
			if (_viv_load_failed)
			{
				string_copy_utf8_string(text_buf,localization_get_string(LOCALIZATION_ID_STATUS_BAR_FAILED_TO_LOAD_IMAGE));
				text = text_buf;
			}
			else
			if (_viv_is_slideshow)
			{
				string_copy_utf8_string(text_buf,localization_get_string(LOCALIZATION_ID_STATUS_BAR_SLIDESHOW_PLAYING));
				text = text_buf;
			}
			else
			if (*_viv_frame_fd->cFileName)
			{
				int nav_index;

				nav_index = _viv_status_nav_index();

				if (nav_index > 0)
				{
					string_printf(text_buf,localization_get_string(LOCALIZATION_ID_STATUS_BAR_POSITION_FORMAT),nav_index,_viv_playlist_count);
					string_cat_utf8(text_buf,(const utf8_t *)"  ");
					string_cat(text_buf,_viv_frame_fd->cFileName);
				}
				else
				{
					string_copy(text_buf,_viv_frame_fd->cFileName);
				}

				text = text_buf;
			}
		
			// pane 0 is the zoom pane; the preload pane (when active)
			// follows it and the message pane takes the rest of the left
			// cluster.
			_viv_status_set(0,zoom_buf);
			
			if (preload_wide)
			{
				_viv_status_set(1,preload_buf);
				
				_viv_status_set(2,text);
			}
			else
			{
				_viv_status_set(1,text);
			}
		}
		
		{
			int parti;
			
			// the right cluster starts after the message pane; the widths
			// (not the buffers) decide which panes exist so the layout and
			// the texts can never drift apart.
			parti = preload_wide ? 3 : 2;
			
			if (pixel_pos_wide)
			{
				_viv_status_set(parti,pixel_pos_buf);
				parti++;
			}
			
			if (pixel_rgb_wide)
			{
				_viv_status_set(parti,pixel_rgb_buf);
				parti++;
			}
			
			if (frame_wide)
			{
				_viv_status_set(parti,frame_buf);
				parti++;
			}

			if (date_wide)
			{
				_viv_status_set(parti,date_buf);
				parti++;
			}
			
			_viv_status_set(parti,dimension_buf);
			parti++;
		}
	}
}
static void _viv_status_set(int part,const wchar_t *text)
{
	static HWND part_text_hwnd;
	static BYTE part_live[_VIV_STATUS_PART_MAX];
	
	if ((part < 0) || (part >= _VIV_STATUS_PART_MAX))
	{
		return;
	}
	
	// the store must match the live control: a recreated status bar (or a
	// hidden one: updates keep flowing) starts with empty panes, so the
	// store flushes whenever the window identity changes - otherwise an
	// unchanged text would skip its SB_SETTEXTW and leave the pane blank.
	if (part_text_hwnd != _viv_status_hwnd)
	{
		part_text_hwnd = _viv_status_hwnd;
		
		os_zero_memory(_viv_status_part_text,sizeof(_viv_status_part_text));
		os_zero_memory(part_live,sizeof(part_live));
	}
	
	// a pane the control was never told about never draws: the empty-bar
	// case showed the light comctl face because no WM_DRAWITEM fired at
	// all (the bottom white bar). send once per pane per bar so the owner
	// draw fill owns the strip even when every text is empty.
	if ((string_compare(_viv_status_part_text[part],text) != 0) || (!part_live[part]))
	{
		// SBT_OWNERDRAW: the pane text lives in our store; the item data
		// carries the pane index to WM_DRAWITEM.
		string_copy(_viv_status_part_text[part],text);
		
		SendMessage(_viv_status_hwnd,SB_SETTEXTW,(WPARAM)(part | SBT_OWNERDRAW),(LPARAM)part);
		
		part_live[part] = 1;
	}
}
// owner draw one status pane: the dark palette for the dark ui, a
// faithful light replica otherwise. the texts come from the pane store;
// an overlong text (a capped resolution pane) ellipsizes instead of
// bleeding into the next pane.
int _viv_status_draw_item(DRAWITEMSTRUCT *draw_item)
{
	HDC hdc;
	RECT text_rect;
	HFONT hfont;
	HGDIOBJ last_font;
	COLORREF text_color;
	int part;
	
	if (!draw_item)
	{
		return 0;
	}
	
	hdc = draw_item->hDC;
	
	if (!hdc)
	{
		return 0;
	}
	
	part = (int)draw_item->itemData;
	
	if ((part < 0) || (part >= _VIV_STATUS_PART_MAX))
	{
		return 0;
	}
	
	if (_viv_is_dark())
	{
		FillRect(hdc,&draw_item->rcItem,_viv_dialog_dark_brush());
		
		text_color = RGB(0xE8,0xE8,0xE8);
	}
	else
	{
		FillRect(hdc,&draw_item->rcItem,(HBRUSH)(COLOR_BTNFACE + 1));
		
		text_color = GetSysColor(COLOR_BTNTEXT);
	}
	
	// draw the text inset like the native panes.
	text_rect = draw_item->rcItem;
	text_rect.left += GetSystemMetrics(SM_CXEDGE) * 2;
	
	hfont = (HFONT)SendMessage(_viv_status_hwnd,WM_GETFONT,0,0);
	last_font = 0;
	
	if (hfont)
	{
		last_font = SelectObject(hdc,hfont);
	}
	
	SetBkMode(hdc,TRANSPARENT);
	SetTextColor(hdc,text_color);
	
	DrawTextW(hdc,_viv_status_part_text[part],-1,&text_rect,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
	
	if (last_font)
	{
		SelectObject(hdc,last_font);
	}
	
	return 1;
}
int _viv_get_status_high(void)
{
	if (_viv_status_hwnd)
	{
		RECT rect;
		
		GetWindowRect(_viv_status_hwnd,&rect);
		
		return rect.bottom - rect.top;
	}
	
	return 0;
}
// the top chrome stack: the menubar plus the toolbar strip, both client
// side children that own the top of the client area. the image viewport
// starts below them (and above the status bar); the helpers return 0 for
// hidden strips, so fullscreen collapses this to the plain client top.
int _viv_get_view_top(void)
{
	return _viv_menubar_high() + _viv_get_controls_high();
}

int _viv_get_controls_high(void)
{
	// the window fit math only wants the strip height; the strip owns
	// the touch scaling now.
	return _viv_toolbar_high();
}
static LRESULT CALLBACK _viv_fullscreen_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_PAINT:
		{
			RECT rect;
			PAINTSTRUCT ps;
			
			GetClientRect(hwnd,&rect);
			BeginPaint(hwnd,&ps);
			/*
			{
				HBRUSH hbrush;
				
				hbrush = CreateSolidBrush(RGB(config_fullscreen_background_color_r,config_fullscreen_background_color_g,config_fullscreen_background_color_b));
				
				if (hbrush)
				{
					FillRect(ps.hdc,&rect,hbrush);
				
					DeleteObject(hbrush);
				}
			}
			*/
			EndPaint(hwnd,&ps);
			break;
		}
			
		case WM_ERASEBKGND:
			return 1;
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}
void _viv_status_set_temp_text(wchar_t *text)
{
	if (_viv_status_temp_text)
	{
		KillTimer(_viv_hwnd,VIV_ID_STATUS_TEMP_TEXT_TIMER);
		
		mem_free(_viv_status_temp_text);
		
		_viv_status_temp_text = 0;
	}
	
	if (text)
	{
		_viv_status_temp_text = string_alloc(text);
	}
	
	_viv_status_update();
	
	if (_viv_status_temp_text)
	{
		SetTimer(_viv_hwnd,VIV_ID_STATUS_TEMP_TEXT_TIMER,3000,0);
	}
}
void _viv_status_update_temp_pos_zoom(void)
{
	// the zoom percent now lives in its own always visible status bar pane,
	// so the old three second temporary text flash is gone: a plain status
	// refresh repaints the pane.
	_viv_status_update();
}
void _viv_status_update_slideshow_rate(void)
{
	wchar_t wbuf[STRING_SIZE];
	const utf8_t *res;
	int r;
	
	if ((config_slideshow_rate / 60000) && ((config_slideshow_rate % 60000) == 0))
	{
		r = config_slideshow_rate / 60000;
		res = localization_get_string(LOCALIZATION_ID_STATUS_BAR_MINUTES);
	}
	else
	if ((config_slideshow_rate / 1000) && ((config_slideshow_rate % 1000) == 0))
	{
		r = config_slideshow_rate / 1000;
		res = localization_get_string(LOCALIZATION_ID_STATUS_BAR_SECONDS);
	}
	else
	{
		r = config_slideshow_rate;
		res = localization_get_string(LOCALIZATION_ID_STATUS_BAR_MILLISECONDS);
	}
	
	string_printf(wbuf,localization_get_string(LOCALIZATION_ID_STATUS_BAR_SLIDESHOW_RATE_FORMAT),r,res);
	
	_viv_status_set_temp_text(wbuf);
}
void _viv_zoomui_update(void)
{
	if (!zoomui_is_created())
	{
		zoomui_init(_viv_hwnd);

		// layout the new zoom controls.
		_viv_on_size();
	}

	// fullscreen shows the six button overlay bar (auto hiding when
	// idle); windowed mode keeps the two button pill.
	zoomui_set_fullscreen(_viv_is_fullscreen);

	zoomui_show(config_show_zoom_controls);
}
void _viv_show_cursor(void)
{
	if (_viv_is_hide_cursor_timer)
	{
		KillTimer(_viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER);
	
		_viv_is_hide_cursor_timer = 0;
	}

	if (!_viv_is_cursor_shown)
	{
		ShowCursor(TRUE);
		
		_viv_is_cursor_shown = 1;
	}
}
void _viv_hide_cursor(void)
{
	if (_viv_is_hide_cursor_timer)
	{
		KillTimer(_viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER);
	
		_viv_is_hide_cursor_timer = 0;
	}

	if (_viv_is_cursor_shown)
	{
		ShowCursor(FALSE);
		
		_viv_is_cursor_shown = 0;
	}
}
int _viv_should_show_cursor(void)
{
	if (!_viv_in_popup_menu)
	{
		if ((*_viv_current_fd->cFileName) && (!_viv_file_not_found) && (!_viv_load_failed))
		{
			if (GetForegroundWindow() == _viv_hwnd)
			{
				if (_viv_is_mouseover)
				{
					if (!GetCapture())
					{
						if ((_viv_is_fullscreen) || (config_windowed_hide_cursor))
						{
							// if (!((_viv_is_alt) && (_viv_is_tracking_mouse)))
							{
								return 0;
							}
						}
					}
				}
			}
		}
	}
			
	return 1;
}
void _viv_update_show_cursor(void)
{
	if (_viv_should_show_cursor())
	{
		_viv_show_cursor();
	}
	else
	{
		_viv_start_hide_cursor_timer();
	}
}
void _viv_start_hide_cursor_timer(void)
{
	if (!_viv_is_hide_cursor_timer)
	{
		SetTimer(_viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER,_VIV_HIDE_CURSOR_DELAY,0);
		
		_viv_is_hide_cursor_timer = 1;
	}
}
