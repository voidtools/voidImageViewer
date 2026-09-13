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
// viv_settings.h - the modern settings window (gui remake round).
// one self drawn owned popup with a left navigation and three pages
// (general / view / controls). every control is painted in one
// wm_paint pass, every change applies immediately and cancel restores
// the snapshot taken when the window opened.
#ifndef VIV_SETTINGS_H
#define VIV_SETTINGS_H

#include "viv.h"

#ifdef __cplusplus
extern "C" {
#endif

// show the settings window. a singleton: when the window already
// exists it is activated instead of created again.
void _viv_settings_show(void);

// destroy the settings window and release its resources. called from
// the main kill path.
void _viv_settings_kill(void);

#ifdef __cplusplus
}
#endif

#endif
