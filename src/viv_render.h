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
// viv_render.h - stretching, mipmaps, orientation, backdrop and zoom math.
#ifndef VIV_RENDER_H
#define VIV_RENDER_H

#include "viv.h"

// exported to other domains / the viv.c core
void _viv_mipmap_free(_viv_mipmap_t *mipmap);
void _viv_get_render_size(int *prw,int *prh);
int _viv_zoom_pos_max(void);
int _viv_is_dark(void);
COLORREF _viv_windowed_background(void);
void _viv_fill_backdrop(HDC hdc,int wide,int high);
void _viv_backdrop_apply(void);
void _viv_update_src_pixel(int force,int update_statusbar);
int _viv_zoom_percent(void);
int _viv_zoom_pos_for_percent(int percent,int strict);
HBITMAP _viv_orientate_hbitmap(HBITMAP hbitmap,int orientation);
HBITMAP _viv_get_mipmap(HBITMAP hbitmap,int image_wide,int image_high,int render_wide,int render_high,int *pmip_wide,int *pmip_high,_viv_mipmap_t **out_mip);
void _viv_stretch_blt(HDC dst_hdc,int dst_x,int dst_y,int dst_wide,int dst_high,HDC src_hdc,int src_wide,int src_high,int clip_x,int clip_y,int clip_wide,int clip_high);
BOOL _viv_StretchBltStitch(HDC hdcDest,int xDest,int yDest,int wDest,int hDest,HDC hdcSrc,int xSrc,int ySrc,int wSrc,int hSrc,DWORD rop,int clip_x,int clip_y,int clip_wide,int clip_high);
int _viv_zoom_pos_floor(void);
int _viv_clamp_zoom_pos(int zoom_pos);

#endif
