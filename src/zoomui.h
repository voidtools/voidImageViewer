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
// Floating zoom controls (touch friendly)
// Provides zoom out / zoom in buttons that remain available in fullscreen
// mode and on touch devices where the mouse wheel is unavailable.
// In fullscreen the bar grows to six buttons (prev / play / pause / next
// / zoom out / zoom in) centered at the bottom and auto hides when idle.

#ifdef __cplusplus
extern "C" {
#endif

// create (or recycle) the zoom controls child window.
// parent is the main voidImageViewer window.
void zoomui_init(HWND parent);

// destroy the zoom controls window.
void zoomui_kill(void);

// show or hide the zoom controls.
void zoomui_show(int show);

// Refresh the tooltip texts after a language change.
void zoomui_localize(void);

// Switch the zoom controls palette (light / dark).
void zoomui_set_dark(int dark);

// is the zoom controls window created?
int zoomui_is_created(void);

// reposition the zoom controls.
// wide,high = the image area of the parent client (status bar / toolbar excluded).
void zoomui_layout(int wide,int high);

// switch between the windowed two button pill and the fullscreen six
// button overlay bar. call on every fullscreen toggle.
void zoomui_set_fullscreen(int fullscreen);

// notify user activity (mouse motion, key press, command): keeps the
// fullscreen overlay awake and shows it again when it auto hid.
void zoomui_activity(void);

#ifdef __cplusplus
}
#endif

