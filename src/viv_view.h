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
// viv_view.h - command dispatch, navigation, zoom, mouse and touch gesture actions.
#ifndef VIV_VIEW_H
#define VIV_VIEW_H

#include "viv.h"

// exported to other domains / the viv.c core
void _viv_command(int command_id);
void _viv_command_with_is_key_repeat(int command_id,int is_key_repeat);
int _viv_next(int prev,int reset_slideshow_timer,int is_preload,int wait_for_current_load);
void _viv_home(int end,int is_preload);
void _viv_view_set(int view_x,int view_y,int invalidate);
void _viv_slideshow(void);
void _viv_pause(void);
void _viv_mousemove(void);
void _viv_view_1to1(void);
void _viv_zoom_set_percent(int percent,int screen_x,int screen_y,int force);
void _viv_zoom_in(int out,int have_xy,int x,int y);
void _viv_view_scroll(int mx,int my);
void _viv_update_1to1_scroll(int x,int y);
void _viv_do_mousewheel_action(int action,int delta,int x,int y);
void _viv_do_left_click_action(int action);
void _viv_start_move_window(void);
int _viv_is_touch_click(void);
void _viv_touch_double_click(void);
int _viv_on_gesture(HWND hwnd,void *gesture_info_handle);

#endif
