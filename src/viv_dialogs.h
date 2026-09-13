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
// viv_dialogs.h - modal dialogs: options, jump-to, about, rename, zoom, rate.
#ifndef VIV_DIALOGS_H
#define VIV_DIALOGS_H

#include "viv.h"

// exported to other domains / the viv.c core
void _viv_dialog_apply_font(HWND hwnd);
void _viv_rename(void);
void _viv_options(void);
INT_PTR CALLBACK _viv_custom_rate_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK _viv_about_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
// opens the in place zoom editor on the status bar pane (the name
// keeps the old dialog's call sites stable).
void _viv_set_zoom_dialog(void);
void _viv_command_line_options(void);
void _viv_show_jumpto(void);

#endif
