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
// viv_toolbar.h - the remade toolbar: a fully self drawn command strip.
#ifndef VIV_TOOLBAR_H
#define VIV_TOOLBAR_H

#include "viv.h"

// create (and show) the strip / destroy it. the strip is a child window the
// app paints end to end - the comctl toolbar, its image list and the pinned
// button widths are gone, the light and the dark ui share one paint path.
void _viv_toolbar_create(HWND parent);
void _viv_toolbar_destroy(void);

// re-read the labels at the current font and lay the buttons out across
// `wide` pixels, anchored directly under the menu bar (the menu bar height
// is the strip's y, so a hidden menu bar leaves the strip on top). the dpi
// change, the font drop, the language rebuild and every on size sweep run
// through here.
void _viv_toolbar_layout(int wide);

// re-read the per button enabled state and the play / pause face. the call
// sites that refreshed the comctl toolbar refresh this (menu checks, view
// changes, the size sweep).
void _viv_toolbar_update_buttons(void);

// the theme flip repaints through the latch (the metrics are theme
// independent, so a flip never re-measures).
void _viv_toolbar_set_dark(int dark);

// the strip height for the client side layout math (0 when absent or
// hidden).
int _viv_toolbar_high(void);

#endif // VIV_TOOLBAR_H
