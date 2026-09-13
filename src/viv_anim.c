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
// viv_anim.c - webp animation frames, rates and the timer queue.
// Pure physical move from viv.c (R70 split): bodies are unchanged.
#include "viv.h"
#include "viv_state.h"
#include "viv_anim.h"
#include "viv_menubar.h"
#include "viv_chrome.h"
#include "viv_load.h"
#include "viv_render.h"
#include "viv_view.h"
#include "viv_toolbar.h"

// forward declarations (order preserved from viv.c)
void _viv_clear_frames(_viv_frame_t *frames,int loaded_count);
void _viv_increase_animation_rate(int dec);
void _viv_reset_animation_rate(void);
VOID NTAPI _viv_timer_queue_timer_callback(PVOID param,BOOLEAN TimerOrWaitFired);
static void _viv_timer_start(void);
void _viv_animation_pause(void);
void _viv_frame_step(void);
void _viv_frame_prev(void);
void _viv_timer_stop(void);
void _viv_update_frame(void);
void _viv_frame_skip(int size);
int _viv_webp_info_proc(_viv_webp_t *viv_webp,DWORD frame_count,DWORD wide,DWORD high,int has_alpha);
int _viv_webp_frame_proc(_viv_webp_t *viv_webp,BYTE *pixels,int delay);
static void _viv_status_update_temp_animation_rate(void);
void _viv_start_first_frame(void);
UINT _viv_frame_delay_at(const os_PropertyItem_t *pd,SIZE_T pd_size,DWORD i);


static BYTE _viv_is_timer_queue_timer = 0;
static HANDLE _viv_timer_queue_timer_handle;
void _viv_clear_frames(_viv_frame_t *frames,int loaded_count)
{
	int i;
	
	for(i=0;i<loaded_count;i++)		
	{
		if (frames[i].hbitmap)
		{
			DeleteObject(frames[i].hbitmap);
		}
		
		if (frames[i].mipmap)
		{
			_viv_mipmap_free(frames[i].mipmap);
		}
	}
	
	mem_free(frames);
}
void _viv_increase_animation_rate(int dec)
{
	if (dec)	
	{
		if (_viv_animation_rate_pos)
		{
			_viv_animation_rate_pos--;
		}
	}
	else
	{
		if (_viv_animation_rate_pos < _VIV_ANIMATION_RATE_MAX-1)
		{
			_viv_animation_rate_pos++;
		}
	}
	
	_viv_status_update_temp_animation_rate();
}
void _viv_reset_animation_rate(void)
{
	_viv_animation_rate_pos = _VIV_ANIMATION_RATE_ONE;

	_viv_status_update_temp_animation_rate();
}
VOID NTAPI _viv_timer_queue_timer_callback(PVOID param,BOOLEAN TimerOrWaitFired)
{
	if (!_viv_is_animation_timer_event)
	{
		_viv_is_animation_timer_event = 1;
		
		PostMessage(_viv_hwnd,WM_TIMER,VIV_ID_ANIMATION_TIMER,0);
	}
}
static void _viv_timer_start(void)
{
	if (!_viv_is_animation_timer)
	{
		_viv_is_animation_timer = 1;
		
		_viv_animation_timer_tick_start = os_get_tick_count();
		_viv_timer_tick = 0;
		
		if ((os_CreateTimerQueueTimer) && (os_DeleteTimerQueueTimer))
		{
			_viv_is_timer_queue_timer = 0;
			
			if (os_CreateTimerQueueTimer(&_viv_timer_queue_timer_handle,NULL,_viv_timer_queue_timer_callback,NULL,1,1,0))
			{
				_viv_is_timer_queue_timer = 1;
			}
		}
		else
		{
			SetTimer(_viv_hwnd,VIV_ID_ANIMATION_TIMER,USER_TIMER_MINIMUM,0);
		}
		
		_viv_update_prevent_sleep();
	}
}
void _viv_animation_pause(void)
{
	_viv_animation_play = !_viv_animation_play;
	
	// rc.13: the clock flip re-reads the play face and the on-top
	// window (the ontop==2 mode follows the playing state, and the
	// toolbar face did not move when the animation paused).
	_viv_toolbar_update_buttons();
	
	_viv_update_ontop();
}
void _viv_frame_step(void)
{
	_viv_frame_looped = 0;
	
	if (_viv_animation_play)
	{
		_viv_animation_play = 0;
	}

	if (_viv_frame_count > 1)			
	{
		if ((_viv_frame_loaded_count == _viv_frame_count) || (_viv_frame_position + 1 < _viv_frame_loaded_count))
		{
			_viv_frame_position++;
			if (_viv_frame_position == _viv_frame_count)
			{
				_viv_frame_position = 0;
			}

			_viv_animation_timer_tick_start = os_get_tick_count();
			_viv_timer_tick = 0;
		
			_viv_update_src_pixel(1,0);
			_viv_status_update();
			
			InvalidateRect(_viv_hwnd,NULL,FALSE);
			UpdateWindow(_viv_hwnd);
		}
	}

	// rc.13: a manual frame step pauses the clock - the play face and
	// the on-top window follow it.
	_viv_toolbar_update_buttons();
	
	_viv_update_ontop();
}
void _viv_frame_prev(void)
{
	_viv_frame_looped = 0;

	if (_viv_animation_play)
	{
		_viv_animation_play = 0;
	}

	if (_viv_frame_count > 1)
	{
		if (_viv_frame_position > 0)
		{
			_viv_frame_position--;
		}
		else
		{
			_viv_frame_position = _viv_frame_loaded_count - 1;
		}
	
		_viv_animation_timer_tick_start = os_get_tick_count();
		_viv_timer_tick = 0;
		
		_viv_update_src_pixel(1,0);
		_viv_status_update();
		
		InvalidateRect(_viv_hwnd,NULL,FALSE);
		UpdateWindow(_viv_hwnd);
	}

	// rc.13: a manual frame step pauses the clock - the play face and
	// the on-top window follow it.
	_viv_toolbar_update_buttons();
	
	_viv_update_ontop();
}
void _viv_timer_stop(void)
{
	if (_viv_is_animation_timer)
	{
		_viv_is_animation_timer = 0;
		
		if ((os_CreateTimerQueueTimer) && (os_DeleteTimerQueueTimer))
		{
			if (_viv_is_timer_queue_timer)
			{
				os_DeleteTimerQueueTimer(NULL,_viv_timer_queue_timer_handle,NULL);
				
				_viv_is_timer_queue_timer = 0;
			}
		}
		else
		{
			KillTimer(_viv_hwnd,VIV_ID_ANIMATION_TIMER);
		}

		_viv_update_prevent_sleep();
	}
}
void _viv_update_frame(void)
{
	if (!_viv_is_fullscreen)
	{
		DWORD oldstyle;
		DWORD newstyle;
		RECT windowrect;
		RECT clientrect;
		RECT newrect;
		RECT oldrect;
		int was_maximized;
		
		was_maximized = 0;

		// get out of maximized state.
		if (_viv_is_window_maximized(_viv_hwnd))
		{
			ShowWindow(_viv_hwnd,SW_RESTORE);
			was_maximized = 1;
		}
		
		oldstyle = GetWindowLong(_viv_hwnd,GWL_STYLE);
		newstyle = oldstyle;
		
		GetClientRect(_viv_hwnd,&clientrect);
		debug_printf("clientrect %d %d %d %d\n",clientrect.left,clientrect.top,clientrect.right,clientrect.bottom);
		
		CopyRect(&oldrect,&clientrect);
		AdjustWindowRect(&oldrect,oldstyle,FALSE);
		
		oldrect.bottom += _viv_menubar_high() + _viv_get_status_high() + _viv_get_controls_high();
		
		debug_printf("oldrect %d %d %d %d %d\n",oldrect.left,oldrect.top,oldrect.right,oldrect.bottom,GetMenu(_viv_hwnd) ? TRUE : FALSE);
	
		if (config_show_caption)	
		{
			newstyle |= WS_CAPTION | WS_SYSMENU;
		}
		else
		{
			newstyle &= ~(WS_CAPTION | WS_SYSMENU);
		}
		
		if (config_show_thickframe)	
		{
			newstyle |= WS_THICKFRAME;
		}
		else
		{
			newstyle &= ~WS_THICKFRAME;
		}

		// the top bar is a client side child: showing it grows the
		// required client area instead of the frame.
		_viv_menubar_show(config_show_menu);
		
		_viv_status_show(config_show_status);
		_viv_controls_show(config_show_controls);
		_viv_zoomui_update();

		CopyRect(&newrect,&clientrect);
		AdjustWindowRect(&newrect,newstyle,config_show_menu ? TRUE : FALSE);

		newrect.bottom += _viv_get_status_high() + _viv_get_controls_high();

		debug_printf("newrect %d %d %d %d %d\n",newrect.left,newrect.top,newrect.right,newrect.bottom,config_show_menu ? TRUE : FALSE);
		
		GetWindowRect(_viv_hwnd,&windowrect);
		
		windowrect.left += newrect.left - oldrect.left;
		windowrect.top += newrect.top - oldrect.top;
		windowrect.right += newrect.right - oldrect.right;
		windowrect.bottom += newrect.bottom - oldrect.bottom;
	
		SetWindowLong(_viv_hwnd,GWL_STYLE,newstyle);
		
		SetWindowPos(_viv_hwnd,HWND_TOP,windowrect.left,windowrect.top,windowrect.right - windowrect.left,windowrect.bottom - windowrect.top,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOCOPYBITS);

		// if there is no catpion or thick frame we should not allow maximize
		// avoid our resize borders when maximized.
		if ((was_maximized) && (config_show_caption) && (config_show_thickframe))
		{
			ShowWindow(_viv_hwnd,SW_MAXIMIZE);
		}
	}
}
void _viv_frame_skip(int size)
{
	if (_viv_frame_count > 1)			
	{
		if (size > 0)
		{
			while(size > 0)
			{
				if ((_viv_frame_loaded_count == _viv_frame_count) || (_viv_frame_position + 1 < _viv_frame_loaded_count))
				{
					_viv_frame_position++;
					if (_viv_frame_position == _viv_frame_count)
					{
						_viv_frame_position = 0;
					}

					_viv_animation_timer_tick_start = os_get_tick_count();
					_viv_timer_tick = 0;
				}

				// a zero delay frame (a webp chunk with no duration) must still
				// make progress, otherwise this loop never terminates.
				size -= (_viv_frames[_viv_frame_position].delay > 0) ? _viv_frames[_viv_frame_position].delay : 1;
			}
		}
		else
		if (size < 0)
		{
			while(size < 0)
			{
				_viv_frame_position--;
				if (_viv_frame_position < 0)
				{
					_viv_frame_position = _viv_frame_loaded_count - 1;
				}
				
				_viv_animation_timer_tick_start = os_get_tick_count();
				_viv_timer_tick = 0;
				
				size += (_viv_frames[_viv_frame_position].delay > 0) ? _viv_frames[_viv_frame_position].delay : 1;
			}
		}
			
		_viv_update_src_pixel(1,0);
		_viv_status_update();
		
		InvalidateRect(_viv_hwnd,NULL,FALSE);
		UpdateWindow(_viv_hwnd);
	}
}
int _viv_webp_info_proc(_viv_webp_t *viv_webp,DWORD frame_count,DWORD wide,DWORD high,int has_alpha)
{
	int ret;
	
	ret = 0;
	
	viv_webp->wide = wide;
	viv_webp->high = high;
	viv_webp->has_alpha = has_alpha;
	viv_webp->frame_count = frame_count;

	viv_webp->screen_hdc = GetDC(NULL);
	if (viv_webp->screen_hdc)
	{
		viv_webp->mem_hdc = CreateCompatibleDC(viv_webp->screen_hdc);
		
		if (viv_webp->mem_hdc)
		{
			ret = 1;
		}
	}
	
	return ret;
}
int _viv_webp_frame_proc(_viv_webp_t *viv_webp,BYTE *pixels,int delay)
{
	// always load first frame..
	if ((viv_webp->frame_index) && (_viv_load_image_terminate))
	{
		return 0;
	}
	
	// the webp frame is composited into a 24bpp top-down dib: when the
	// frame has alpha the backdrop is painted first and the rgba pixels
	// are blended over it, exactly like the gdi+ path does for png/gif.
	// opaque frames simply overwrite every pixel.
	{
		BITMAPINFO bmi;
		HBITMAP hbitmap;
		void *bits;

		ZeroMemory(&bmi, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = viv_webp->wide;
		bmi.bmiHeader.biHeight = -(int)viv_webp->high; // Negative height to indicate top-down bitmap
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 24;
		bmi.bmiHeader.biCompression = BI_RGB;
		
		hbitmap = CreateDIBSection(viv_webp->screen_hdc,&bmi,DIB_RGB_COLORS,&bits,NULL,0);

		// Set RGB data to the bitmap
		if (hbitmap) 
		{
			int mip_wide;
			int mip_high;
			HGDIOBJ last_hbitmap;
			const BYTE *p;
			BYTE *d;
			DWORD high_run;
			int stride;

			last_hbitmap = SelectObject(viv_webp->mem_hdc,hbitmap);
			
			if (viv_webp->has_alpha)
			{
				// fill the backdrop under the transparent pixels (cached
				// brushes, a single FillRect).
				_viv_fill_backdrop(viv_webp->mem_hdc,viv_webp->wide,viv_webp->high);
			}
			
			// blend the rgba frame over the dib pixels. the webp scan0 is
			// rgba, the dib is bgr with 4 byte aligned rows: the source is
			// read ahead of the write cursor so the shared webp canvas
			// buffer is left untouched.
			stride = ((viv_webp->wide * 3) + 3) / 4 * 4;
			p = pixels;
			d = (BYTE *)bits;
			high_run = viv_webp->high;
			
			while(high_run)
			{
				BYTE *wd;
				DWORD wide_run;
				
				wide_run = viv_webp->wide;
				wd = d;
			
				while(wide_run)
				{
					int r;
					int g;
					int b;
					int a;
					
					r = p[0];
					g = p[1];
					b = p[2];
					a = p[3];
					
					p += 4;
					
					// alpha 255 fully replaces the backdrop pixel, alpha 0
					// keeps it.
					wd[0] = b + ((wd[0] - b) * (255 - a)) / 255;
					wd[1] = g + ((wd[1] - g) * (255 - a)) / 255;
					wd[2] = r + ((wd[2] - r) * (255 - a)) / 255;
					wd += 3;
					
					wide_run--;
				}
				
				d += stride;
				high_run--;
			}
			
			SelectObject(viv_webp->mem_hdc,last_hbitmap);
			if (viv_webp->orientation > 1)
			{
				HBITMAP new_hbitmap;
				
				new_hbitmap = _viv_orientate_hbitmap(hbitmap,viv_webp->orientation);
				if (new_hbitmap)
				{
					DeleteObject(hbitmap);
					hbitmap = new_hbitmap;
				}
			}
			
			if (viv_webp->frame_index == 0)
			{
				_viv_reply_load_image_first_frame_t first_frame;

				first_frame.wide = viv_webp->wide;
				first_frame.high = viv_webp->high;
				
				// apply orientation: transposed orientations (5-8) swap the axes.
				// the hbitmap above was already rotated to match, so the reported
				// dimensions swap with it. mirrors the gdi+ path.
				switch (viv_webp->orientation)
				{
					case 5: // #define PHOTO_ORIENTATION_TRANSPOSE         5u
					case 6: // #define PHOTO_ORIENTATION_ROTATE270         6u
					case 7: // #define PHOTO_ORIENTATION_TRANSVERSE        7u
					case 8: // #define PHOTO_ORIENTATION_ROTATE90          8u
						
						{
							int temp;
							temp = first_frame.wide;
							first_frame.wide = first_frame.high;
							first_frame.high = temp;
						}
						break;
				}
				first_frame.is_low_res = 0;
				first_frame.frame.hbitmap = hbitmap;
				first_frame.frame.mipmap = NULL;
				first_frame.frame.delay = 0;
				first_frame.frame_count = viv_webp->frame_count;
				
				if (viv_webp->frame_count > 1)
				{
					first_frame.frame.delay = delay;
				}

				// preload first mipmap
				_viv_get_mipmap(hbitmap,first_frame.wide,first_frame.high,_viv_load_render_wide/2,_viv_load_render_high/2,&mip_wide,&mip_high,&first_frame.frame.mipmap);

				_viv_reply_add(_VIV_REPLY_LOAD_IMAGE_FIRST_FRAME,sizeof(_viv_reply_load_image_first_frame_t),&first_frame);
			}
			else
			{
				_viv_frame_t frame;
				int frame_wide;
				int frame_high;
				
				frame.hbitmap = hbitmap;
				frame.mipmap = NULL;
				frame.delay = delay;
				
//printf("DELAY %d\n",delay)				;

				frame_wide = viv_webp->wide;
				frame_high = viv_webp->high;
				
				// apply orientation: transposed orientations (5-8) swap the axes
				// for the mipmap too (the hbitmap above is already rotated).
				switch (viv_webp->orientation)
				{
					case 5:
					case 6:
					case 7:
					case 8:
						
							frame_wide = viv_webp->high;
							frame_high = viv_webp->wide;
							break;
				}
				
				_viv_get_mipmap(hbitmap,frame_wide,frame_high,_viv_load_render_wide/2,_viv_load_render_high/2,&mip_wide,&mip_high,&frame.mipmap);
				
				_viv_reply_add(_VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME,sizeof(_viv_frame_t),&frame);
			}
			
			viv_webp->frame_index++;
		}
	}
	
	return 1;
}
static void _viv_status_update_temp_animation_rate(void)
{
	wchar_t wbuf[STRING_SIZE];

	string_printf(wbuf,localization_get_string(LOCALIZATION_ID_STATUS_BAR_ANIMATION_RATE_FORMAT),_viv_animation_rates[_viv_animation_rate_pos]);
			
	_viv_status_set_temp_text(wbuf);
}
void _viv_start_first_frame(void)
{
	// draw first frame and start timer.
	_viv_frame_position = 0;
	_viv_frame_looped = 0;
	_viv_animation_timer_tick_start = os_get_tick_count();
	_viv_timer_tick = 0;
	
	if (_viv_frame_count > 1)
	{
		_viv_timer_start();
	}

	_viv_update_src_pixel(1,0);
	_viv_status_update();
	
	if (_viv_doing == _VIV_DOING_1TO1SCROLL)
	{
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(_viv_hwnd,&pt);
		
		_viv_update_1to1_scroll(pt.x,pt.y);
	}

	// show cursor.
	_viv_update_show_cursor();

//debug_printf("---\n",_viv_load_image_next_fd);
//debug_printf("_viv_load_image_next_fd %p\n",_viv_load_image_next_fd);
//debug_printf("---\n",_viv_load_image_next_fd);
/*
	if (_viv_load_image_next_fd)
	{
		// load priority paint..
		// we will paint on the 'next next' image we load..
		_viv_low_priority_paint = 1;
		return;
	}
	
	_viv_low_priority_paint = 0;
	InvalidateRect(_viv_hwnd,NULL,FALSE);

	// building mipmaps as needed hangs the UI.
	// this makes rendering lag while holding down right.
	// avoid painting when user is holding down right..
	//
	// because we build the mipmap for the first image in the load thread this is now instant..
	// ok it's still awful, let the next load refresh..
//	if (_viv_load_image_next_fd)
	{
//		UpdateWindow(_viv_hwnd);
	}*/
	
	if (!_viv_is_fullscreen)
	{
		if (config_auto_zoom)
		{
			switch(config_auto_zoom_type)
			{
				case 0:
				case 1:
				case 2:
				case 3:
					// don't scroll any regions when sizing.
					InvalidateRect(_viv_hwnd,NULL,FALSE);
					_viv_command(VIV_ID_VIEW_WINDOW_SIZE_50 + config_auto_zoom_type);
					break;
			}
		}
	}

	InvalidateRect(_viv_hwnd,NULL,FALSE);
	UpdateWindow(_viv_hwnd);
}			
UINT _viv_frame_delay_at(const os_PropertyItem_t *pd,SIZE_T pd_size,DWORD i)
{
	UINT value;
	UINT count;
	const BYTE *v;
	
	value = 0;
	
	if ((!pd) || (pd_size <= sizeof(os_PropertyItem_t)))
	{
		return 0;
	}
	
	// gdi+ points value into the property buffer: keep the dereference
	// inside the buffer we actually own.
	v = (const BYTE *)pd->value;
	
	if ((v < (const BYTE *)pd) || ((SIZE_T)(v - (const BYTE *)pd) >= pd_size))
	{
		return 0;
	}
	
	if (!(count = pd->length / sizeof(UINT)))
	{
		return 0;
	}
	
	if (!_viv_safe_copy_data(pd,pd_size,&(((const UINT *)v)[i % count]),&value,sizeof(UINT)))
	{
		value = 0;
	}
	
	return value;
}
