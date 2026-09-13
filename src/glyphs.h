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
// vector glyph icons (toolbar + zoom controls).
// drawn from a 48x48 design grid (2.5 unit round capped strokes) through
// the gdi+ flat api at any requested size and in both theme colors,
// converted to HICONs and cached per (glyph, theme, size).

#ifndef _GLYPHS_H_INCLUDED_
#define _GLYPHS_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

// glyph ids. 0..7 are the media transport / zoom set (the old toolbar image
// list order, still shared with the zoom pill); 8..15 are the remade
// toolbar's set (open, the magnifier pair, rotate, info, settings, picture
// and the gamepad for the settings window nav).
#define GLYPH_PREV            0
#define GLYPH_PLAY            1
#define GLYPH_PAUSE           2
#define GLYPH_NEXT            3
#define GLYPH_BESTFIT         4
#define GLYPH_1TO1            5
#define GLYPH_ZOOMOUT         6
#define GLYPH_ZOOMIN          7
#define GLYPH_FOLDER_OPEN     8
#define GLYPH_MAGNIFIER_MINUS 9
#define GLYPH_MAGNIFIER_PLUS  10
#define GLYPH_ROTATE_CW       11
#define GLYPH_INFO            12
#define GLYPH_SETTINGS        13
#define GLYPH_PICTURE         14
#define GLYPH_GAMEPAD         15
#define GLYPH_COUNT           16

// return the glyph icon at the requested size and theme. the icon is
// owned by the glyphs cache: do not destroy it. building needs gdi+;
// on failure 0 is returned and callers should skip drawing.
HICON glyphs_icon(int glyph_id,int dark,int size);

// drop every cached icon. call when the palette or the window dpi
// changes (both are baked into the cached bitmaps).
void glyphs_flush_cache(void);

#ifdef __cplusplus
}
#endif

#endif

