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
// viv_chrome.h - window dressing: rebar, toolbar, status bar, menu bar, fullscreen, cursors.
#ifndef VIV_CHROME_H
#define VIV_CHROME_H

#include "viv.h"

// exported to other domains / the viv.c core
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
int _viv_status_draw_item(DRAWITEMSTRUCT *draw_item);
int _viv_get_status_high(void);
int _viv_get_controls_high(void);
int _viv_get_view_top(void);
void _viv_status_set_temp_text(wchar_t *text);
void _viv_status_update_temp_pos_zoom(void);
void _viv_status_update_slideshow_rate(void);
void _viv_zoomui_update(void);
void _viv_show_cursor(void);
void _viv_hide_cursor(void);
int _viv_should_show_cursor(void);
void _viv_update_show_cursor(void);
void _viv_start_hide_cursor_timer(void);

#endif
