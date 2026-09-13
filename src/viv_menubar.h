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
// viv_menubar.h - the remade top bar: a fully self drawn menu bar.
#ifndef VIV_MENUBAR_H
#define VIV_MENUBAR_H

// create (or re-show) the bar / hide it. the frame menu is gone for good:
// the strip is a child window the app paints end to end, so the light and
// the dark ui share one layout and one paint path. the height is exposed
// for the client-side layout math (the on-size sweep subtracts it).
void _viv_menubar_show(int show);
int _viv_menubar_high(void);
void _viv_menubar_resize(int wide);

// re-read the root items (the font, the labels, the widths): the dpi
// change, the language rebuild and the menu rebuild all run through here.
void _viv_menubar_layout(void);

// the theme flip only repaints (the metrics are theme independent).
void _viv_menubar_repaint(void);

// the keyboard entry points: wm_syschar (alt + mnemonic) returns 1 when
// the key opened a menu, wm_syskeydown f10 opens the first one.
int _viv_menubar_open_mnemonic(int key);
void _viv_menubar_idle_pump(void);
void _viv_menubar_kill(void);
void _viv_menubar_open_first(void);

#endif // VIV_MENUBAR_H
