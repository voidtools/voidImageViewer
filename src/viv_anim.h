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
// viv_anim.h - webp animation frames, rates and the timer queue.
#ifndef VIV_ANIM_H
#define VIV_ANIM_H

#include "viv.h"

// exported to other domains / the viv.c core
void _viv_clear_frames(_viv_frame_t *frames,int loaded_count);
void _viv_increase_animation_rate(int dec);
void _viv_reset_animation_rate(void);
void _viv_animation_pause(void);
void _viv_frame_step(void);
void _viv_frame_prev(void);
void _viv_timer_stop(void);
void _viv_update_frame(void);
void _viv_frame_skip(int size);
int _viv_webp_info_proc(_viv_webp_t *viv_webp,DWORD frame_count,DWORD wide,DWORD high,int has_alpha);
int _viv_webp_frame_proc(_viv_webp_t *viv_webp,BYTE *pixels,int delay);
void _viv_start_first_frame(void);
UINT _viv_frame_delay_at(const os_PropertyItem_t *pd,SIZE_T pd_size,DWORD i);

#endif
