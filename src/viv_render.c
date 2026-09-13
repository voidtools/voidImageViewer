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
// viv_render.c - stretching, mipmaps, orientation, backdrop and zoom math.
// Pure physical move from viv.c (R70 split): bodies are unchanged.
#include "viv.h"
#include "viv_state.h"
#include "viv_render.h"
#include "viv_chrome.h"
#include "viv_load.h"
#include "viv_menu.h"

// forward declarations (order preserved from viv.c)
void _viv_mipmap_free(_viv_mipmap_t *mipmap);
static double _viv_clamp_double(double value,double maximum);
void _viv_get_render_size(int *prw,int *prh);
int _viv_zoom_pos_max(void);
int _viv_is_dark(void);
static COLORREF _viv_dark_mat_color(BYTE r,BYTE g,BYTE b);
COLORREF _viv_windowed_background(void);
static HBRUSH _viv_backdrop_solid_brush(void);
static HBRUSH _viv_backdrop_checker_brush(void);
void _viv_fill_backdrop(HDC hdc,int wide,int high);
void _viv_backdrop_apply(void);
void _viv_update_src_pixel(int force,int update_statusbar);
int _viv_zoom_percent(void);
int _viv_zoom_pos_for_percent(int percent,int strict);
HBITMAP _viv_orientate_hbitmap(HBITMAP hbitmap,int orientation);
HBITMAP _viv_get_mipmap(HBITMAP hbitmap,int image_wide,int image_high,int render_wide,int render_high,int *pmip_wide,int *pmip_high,_viv_mipmap_t **out_mip);
static int _viv_ceil(double x);
void _viv_stretch_blt(HDC dst_hdc,int dst_x,int dst_y,int dst_wide,int dst_high,HDC src_hdc,int src_wide,int src_high,int clip_x,int clip_y,int clip_wide,int clip_high);
BOOL _viv_StretchBltStitch(HDC hdcDest,int xDest,int yDest,int wDest,int hDest,HDC hdcSrc,int xSrc,int ySrc,int wSrc,int hSrc,DWORD rop,int clip_x,int clip_y,int clip_wide,int clip_high);
static BOOL _viv_get_src_pixel_pos(int client_x,int client_y,POINT *out_pixel_pt);
static void _viv_get_src_pixel_rgb(int src_x,int src_y,COLORREF *out_colorref);
int _viv_zoom_pos_floor(void);
int _viv_clamp_zoom_pos(int zoom_pos);


void _viv_mipmap_free(_viv_mipmap_t *mipmap)
{
	DeleteObject(mipmap->hbitmap);
	
	if (mipmap->mipmap)
	{
		_viv_mipmap_free(mipmap->mipmap);
	}
	
	mem_free(mipmap);
}
// multiply and cap in double space, so an extreme panorama can not
// overflow the int conversion in _viv_get_render_size.
static double _viv_clamp_double(double value,double maximum)
{
	if (value > maximum)
	{
		return maximum;
	}
	
	return value;
}
void _viv_get_render_size(int *prw,int *prh)
{
	RECT rect;
	int wide;
	int high;
	int rw;
	int rh;
	int fill_window;
	
	if (!((_viv_image_wide) && (_viv_image_high)))
	{
		*prw = 0;
		*prh = 0;
		
		return;
	}
	
	GetClientRect(_viv_hwnd,&rect);
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_view_top();
	
	if (!((wide) && (high)))
	{
		*prw = 0;
		*prh = 0;
		
		return;
	}

	if ((_viv_1to1) || (_viv_doing == _VIV_DOING_1TO1SCROLL))
	{
		*prw = _viv_image_wide;
		*prh = _viv_image_high;
		
		return;
	}
			
	if (_viv_is_fullscreen)
	{
		fill_window = config_fullscreen_fill_window;
	}
	else
	{
		fill_window = config_fill_window;
	}
	
	if (config_keep_aspect_ratio)
	{
		if ((high * _viv_image_wide) / _viv_image_high < wide)
		{
			// tall image.
			// add  _viv_image_high - 1 so when we resize the window to 50% it stretches to the screen edges correctly.
			rh = high;
			rw = ((high * (__int64)_viv_image_wide) + _viv_image_high - 1) / _viv_image_high;

			// make sure we have some width.			
			if (rw <= 0)
			{
				rw = 1;
			}
		}
		else
		{
			// long image.
			// add _viv_image_wide - 1 so when we resize the window to 50% it stretches to the screen edges correctly.
			rw = wide;
			rh = ((wide * (__int64)_viv_image_high) + _viv_image_wide - 1) / _viv_image_wide;

			// make sure we have some height	
			if (rh <= 0)
			{
				rh = 1;
			}
		}

		if (!fill_window)
		{
			if ((rw > _viv_image_wide) || (rh > _viv_image_high))
			{
				rw = _viv_image_wide;
				rh = _viv_image_high;
			}
		}
	}
	else
	{
		rw = wide;
		rh = high;

		if (!fill_window)
		{
			if (rw > _viv_image_wide)
			{
				rw = _viv_image_wide;
			}			
	
			if (rh > _viv_image_high)
			{
				rh = _viv_image_high;
			}
		}
	}

//	debug_printf("GET_RENDER_SIZE wide %d high %d rw %d rh %d fill_window %d\n",wide,high,rw,rh,fill_window);

	if (!config_allow_shrinking)
	{
		if (config_keep_aspect_ratio)
		{
			if ((rw < _viv_image_wide) || (rh < _viv_image_high))
			{
				rw = _viv_image_wide;
				rh = _viv_image_high;
			}
		}
		else
		{
			if (rw < _viv_image_wide)
			{
				rw = _viv_image_wide;
			}

			if (rh < _viv_image_high)
			{
				rh = _viv_image_high;
			}
		}
	}		
	
	// the zoom ladder is geometric: each step multiplies the best fit size
	// by 1.01. the ladder tops out at 16x the NATIVE image size per axis
	// (exactly 1600% on the status bar; deep zoom stays reachable for photos
	// much larger than the window), and the last reachable step snaps
	// exactly to the cap. the cap never drops below the pos 0 fit, so fill
	// window keeps its upscale (rc.7: the old 16x-the-LARGER rule let a
	// window larger than the image, or fill window, push the ceiling to
	// 16x the fit - the 2478% report).
	if (_viv_zoom_pos)
	{
		double scale;
		double max_w;
		double max_h;
		
		// the below-fit extension uses the reciprocal of the table entry
		// (1.01^-n is exactly 1/1.01^n), so negative positions never index
		// the scale table out of bounds.
		scale = (_viv_zoom_pos > 0) ? _viv_zoom_scales[_viv_zoom_pos] : (1.0 / _viv_zoom_scales[-_viv_zoom_pos]);
		
		// the caps are computed in double space: an int cap could itself
		// overflow for an absurd panorama.
		max_w = 16.0 * (double)_viv_image_wide;
		max_h = 16.0 * (double)_viv_image_high;
		
		// never shrink below the pos 0 size: the fill window upscale is a
		// layout decision, not a zoom level.
		if (max_w < (double)rw)
		{
			max_w = (double)rw;
		}
		
		if (max_h < (double)rh)
		{
			max_h = (double)rh;
		}
		
		rw = (int)_viv_clamp_double((double)rw * scale,max_w);
		rh = (int)_viv_clamp_double((double)rh * scale,max_h);
		
		// a deep below-fit zoom of a tiny image can round to a zero render
		// size: keep one pixel so the draw and view math never divide by
		// zero.
		if (rw < 1)
		{
			rw = 1;
		}
		
		if (rh < 1)
		{
			rh = 1;
		}
	}
	
	*prw = rw;
	*prh = rh;
}
// cache for _viv_zoom_pos_max: the ladder top depends only on the image,
// the viewport and the layout settings. the signature check below is
// O(1), so a wheel tick or a window resize no longer re-walks the 1024
// entry ladder (and no longer saves/restores the zoom globals) on every
// event.
static int _viv_zoom_pos_max_cache = -1;
static int _viv_zoom_pos_max_cache_image_wide = -1;
static int _viv_zoom_pos_max_cache_image_high = -1;
static int _viv_zoom_pos_max_cache_view_wide = -1;
static int _viv_zoom_pos_max_cache_view_high = -1;
static int _viv_zoom_pos_max_cache_fill_window = -1;
static int _viv_zoom_pos_max_cache_keep_aspect = -1;
static int _viv_zoom_pos_max_cache_allow_shrinking = -1;
// the first ladder position that reaches the 16x native cap. positions
// beyond this render identically (capped), so the wheel clamps here
// instead of spinning in a dead zone of identical sizes.
int _viv_zoom_pos_max(void)
{
	RECT rect;
	int wide;
	int high;
	int fill_window;
	int rw;
	int rh;
	int pos;
	int max_pos;
	
	max_pos = 0;
	
	// build the dependency signature of the ladder top: the image, the
	// viewport and the layout settings. (the zoom position itself is not
	// part of it: the top is always measured with the zoom state cleared.)
	GetClientRect(_viv_hwnd,&rect);
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_view_top();
	
	if (_viv_is_fullscreen)
	{
		fill_window = config_fullscreen_fill_window;
	}
	else
	{
		fill_window = config_fill_window;
	}
	
	if ((_viv_zoom_pos_max_cache >= 0)
		&& (_viv_zoom_pos_max_cache_image_wide == _viv_image_wide)
		&& (_viv_zoom_pos_max_cache_image_high == _viv_image_high)
		&& (_viv_zoom_pos_max_cache_view_wide == wide)
		&& (_viv_zoom_pos_max_cache_view_high == high)
		&& (_viv_zoom_pos_max_cache_fill_window == fill_window)
		&& (_viv_zoom_pos_max_cache_keep_aspect == config_keep_aspect_ratio)
		&& (_viv_zoom_pos_max_cache_allow_shrinking == config_allow_shrinking))
	{
		return _viv_zoom_pos_max_cache;
	}
	
	// measure the unzoomed best fit size with the zoom state cleared.
	{
		int backup_pos;
		int backup_1to1;
		int backup_doing;
		
		backup_pos = _viv_zoom_pos;
		backup_1to1 = _viv_1to1;
		backup_doing = _viv_doing;
		
		_viv_zoom_pos = 0;
		_viv_1to1 = 0;
		_viv_doing = _VIV_DOING_NOTHING;
		
		_viv_get_render_size(&rw,&rh);
		
		_viv_zoom_pos = backup_pos;
		_viv_1to1 = backup_1to1;
		_viv_doing = backup_doing;
	}
	
	if ((rw) && (rh))
	{
		double max_w;
		double max_h;
		
		// 16x native (1600%), never below the measured pos 0 fit (fill
		// window keeps its upscale): mirrors _viv_get_render_size.
		max_w = 16.0 * (double)_viv_image_wide;
		max_h = 16.0 * (double)_viv_image_high;
		
		if (max_w < (double)rw)
		{
			max_w = (double)rw;
		}
		
		if (max_h < (double)rh)
		{
			max_h = (double)rh;
		}
		
		// walk the ladder for the first position that hits the cap on
		// either axis.
		for(pos=0;pos<_VIV_ZOOM_MAX;pos++)
		{
			if (((double)rw * (double)_viv_zoom_scales[pos]) >= max_w)
			{
				break;
			}
			
			if (((double)rh * (double)_viv_zoom_scales[pos]) >= max_h)
			{
				break;
			}
		}
		
		if (pos < _VIV_ZOOM_MAX)
		{
			max_pos = pos;
		}
		else
		{
			max_pos = _VIV_ZOOM_MAX-1;
		}
	}
	else
	{
		// no image (or degenerate window): keep the full table range so
		// startup code paths are untouched.
		max_pos = _VIV_ZOOM_MAX-1;
	}
	
	_viv_zoom_pos_max_cache = max_pos;
	_viv_zoom_pos_max_cache_image_wide = _viv_image_wide;
	_viv_zoom_pos_max_cache_image_high = _viv_image_high;
	_viv_zoom_pos_max_cache_view_wide = wide;
	_viv_zoom_pos_max_cache_view_high = high;
	_viv_zoom_pos_max_cache_fill_window = fill_window;
	_viv_zoom_pos_max_cache_keep_aspect = config_keep_aspect_ratio;
	_viv_zoom_pos_max_cache_allow_shrinking = config_allow_shrinking;
	
	return max_pos;
}
// is the dark ui active? config: 0 = light, 1 = dark, 2 = follow the windows theme.
int _viv_is_dark(void)
{
	if (config_dark_mode == 1)
	{
		return 1;
	}
	
	if (config_dark_mode == 0)
	{
		return 0;
	}
	
	return os_dark_system_dark();
}
// map a user chosen mat color into the dark ui: the hue survives, the
// brightness lands at or under the dark chrome face (0x20). the default
// white is bit-exact with the dark palette canvas. a color that is
// already dark passes through unchanged so the rules never fight.
static COLORREF _viv_dark_mat_color(BYTE r,BYTE g,BYTE b)
{
	int luminance;
	
	if ((r == 255) && (g == 255) && (b == 255))
	{
		return RGB(0x20,0x20,0x20);
	}
	
	luminance = (((int)r * 30) + ((int)g * 59) + ((int)b * 11)) / 100;
	
	if (luminance >= 48)
	{
		return RGB((BYTE)(((WORD)r * 0x20) / 255),(BYTE)(((WORD)g * 0x20) / 255),(BYTE)(((WORD)b * 0x20) / 255));
	}
	
	return RGB(r,g,b);
}
// the windowed background color. the light ui shows the exact configured
// color; the dark ui keeps the hue but never lets a light mat glare out
// of the dark chrome (the field report: the mat around an open image and
// the empty window canvas both ignored the theme, image open or not).
COLORREF _viv_windowed_background(void)
{
	if (_viv_is_dark())
	{
		return _viv_dark_mat_color(config_windowed_background_color_r,config_windowed_background_color_g,config_windowed_background_color_b);
	}
	
	return RGB(config_windowed_background_color_r,config_windowed_background_color_g,config_windowed_background_color_b);
}
// the solid backdrop brush (follow/black/white/custom), cached like the
// window background brush: the load thread paints it for every frame that
// has alpha, so it must not allocate per frame. the follow mode reads the
// dark-aware windowed background so the backdrop tracks the dark ui.
static HBRUSH _viv_backdrop_solid_brush(void)
{
	COLORREF color;
	
	switch(config_backdrop_mode)
	{
		case CONFIG_BACKDROP_MODE_BLACK:
			color = RGB(0,0,0);
			break;
		
		case CONFIG_BACKDROP_MODE_WHITE:
			color = RGB(255,255,255);
			break;
		
		case CONFIG_BACKDROP_MODE_CUSTOM:
			// the custom backdrop follows the mat rule too: a light color
			// under transparent pixels would glare out of the dark ui.
			color = _viv_is_dark() ? _viv_dark_mat_color(config_backdrop_color_r,config_backdrop_color_g,config_backdrop_color_b) : RGB(config_backdrop_color_r,config_backdrop_color_g,config_backdrop_color_b);
			break;
		
		default:
			// follow: the canvas the image already floats on - dark palette
			// aware and fullscreen aware, so the follow backdrop matches the
			// mat the window is painted with instead of always the windowed
			// one (fullscreen used to show the windowed color under the
			// fullscreen mat: a two-tone split around the image).
			color = _viv_is_fullscreen ? RGB(config_fullscreen_background_color_r,config_fullscreen_background_color_g,config_fullscreen_background_color_b) : _viv_windowed_background();
			break;
	}
	
	if ((!_viv_backdrop_solid_hbrush) || (_viv_backdrop_solid_color != color))
	{
		if (_viv_backdrop_solid_hbrush)
		{
			DeleteObject(_viv_backdrop_solid_hbrush);
		}
		
		_viv_backdrop_solid_hbrush = CreateSolidBrush(color);
		_viv_backdrop_solid_color = color;
	}
	
	return _viv_backdrop_solid_hbrush;
}
// the checkerboard brush: a 2x2 cell pattern bitmap that gdi tiles in a
// single FillRect. one fill call replaces the loop of per-cell FillRects,
// and the brush is cached so the load thread allocates nothing per frame.
static HBRUSH _viv_backdrop_checker_brush(void)
{
	int cell;
	
	cell = (8 * os_logical_high) / 96;
	
	if (cell < 4)
	{
		cell = 4;
	}
	
	if ((_viv_backdrop_checker_hbrush) && (_viv_backdrop_checker_cell == cell))
	{
		return _viv_backdrop_checker_hbrush;
	}
	
	if (_viv_backdrop_checker_hbrush)
	{
		DeleteObject(_viv_backdrop_checker_hbrush);
		
		_viv_backdrop_checker_hbrush = 0;
	}
	
	if (_viv_backdrop_checker_hbitmap)
	{
		DeleteObject(_viv_backdrop_checker_hbitmap);
		
		_viv_backdrop_checker_hbitmap = 0;
	}
	
	_viv_backdrop_checker_cell = cell;
	
	{
		int size;
		DWORD *bits;
		
		size = cell * 2;
		
		bits = mem_alloc(safe_size_mul(safe_size_mul((SIZE_T)size,(SIZE_T)size),4));
		
		if (bits)
		{
			int x;
			int y;
			
			for(y=0;y<size;y++)
			{
				DWORD *row;
				
				row = (DWORD *)((BYTE *)bits + (y * size * 4));
				
				for(x=0;x<size;x++)
				{
					// 0xaabbggrr little endian: white and light gray cells.
					row[x] = (((x / cell) ^ (y / cell)) & 1) ? 0xffcccccc : 0xffffffff;
				}
			}
			
			_viv_backdrop_checker_hbitmap = CreateBitmap(size,size,1,32,bits);
			
			mem_free(bits);
			
			if (_viv_backdrop_checker_hbitmap)
			{
				_viv_backdrop_checker_hbrush = CreatePatternBrush(_viv_backdrop_checker_hbitmap);
				
				if (!_viv_backdrop_checker_hbrush)
				{
					DeleteObject(_viv_backdrop_checker_hbitmap);
					
					_viv_backdrop_checker_hbitmap = 0;
					_viv_backdrop_checker_cell = 0;
				}
			}
		}
	}
	
	return _viv_backdrop_checker_hbrush;
}
// fill the alpha image backdrop: what shows under transparent pixels.
// called on the load thread for every frame that has alpha (also every
// animation frame): every mode is a single cached-brush FillRect.
void _viv_fill_backdrop(HDC hdc,int wide,int high)
{
	RECT rect;
	HBRUSH hbrush;
	
	rect.left = 0;
	rect.top = 0;
	rect.right = wide;
	rect.bottom = high;
	
	if (config_backdrop_mode == CONFIG_BACKDROP_MODE_CHECKERBOARD)
	{
		hbrush = _viv_backdrop_checker_brush();
	}
	else
	{
		hbrush = _viv_backdrop_solid_brush();
	}
	
	if (hbrush)
	{
		FillRect(hdc,&rect,hbrush);
	}
}
// apply a backdrop mode change: update the menu radios and reload the
// image so alpha images pick up the new backdrop.
void _viv_backdrop_apply(void)
{
	_viv_check_menus(_viv_hmenu);
	
	_viv_refresh();
}
void _viv_update_src_pixel(int force,int update_statusbar)
{
//	if ((config_pixel_info) || ((_viv_is_alt) && (_viv_is_tracking_mouse)))
	if (config_pixel_info)
	{
		POINT src_pixel_pt;
		POINT client_pt;
		RECT client_rect;
		POINT pt;
		
		GetCursorPos(&pt);
		
		client_pt.x = pt.x;
		client_pt.y = pt.y;
		ScreenToClient(_viv_hwnd,&client_pt);
		GetClientRect(_viv_hwnd,&client_rect);
		
		if (_viv_get_src_pixel_pos(client_pt.x,client_pt.y,&src_pixel_pt))
		{
			
		}
		else
		{
			src_pixel_pt.x = -1;
			src_pixel_pt.y = -1;
		}

		if ((force) || (src_pixel_pt.x != _viv_src_pixel_x) || (src_pixel_pt.y != _viv_src_pixel_y))
		{
			COLORREF src_pixel_rgb;
			
			_viv_src_pixel_x = src_pixel_pt.x;
			_viv_src_pixel_y = src_pixel_pt.y;
			
			if ((_viv_src_pixel_x >= 0) && (_viv_src_pixel_y >= 0))
			{
				_viv_get_src_pixel_rgb(_viv_src_pixel_x,_viv_src_pixel_y,&src_pixel_rgb);
			}
			
			_viv_src_pixel_r = GetRValue(src_pixel_rgb);
			_viv_src_pixel_g = GetGValue(src_pixel_rgb);
			_viv_src_pixel_b = GetBValue(src_pixel_rgb);

			if (update_statusbar)
			{
				_viv_status_update();
			}
			
/*
			if ((src_pixel_pt.x != -1) && (src_pixel_pt.y != -1))
			{
				_viv_tooltip_update();
			}
			else
			{
				_viv_tooltip_hide();
			}*/
		}
	}
	else
	{
		_viv_src_pixel_x = -1;
		_viv_src_pixel_y = -1;
	}

/*	
	if ((_viv_is_alt) && (_viv_is_tracking_mouse))
	{
		_viv_tooltip_update_track_position();
	}
	else
	{
		_viv_tooltip_hide();
	}
	*/
}
int _viv_zoom_percent(void)
{
	// the displayed zoom percent: the average of the x and y render scale,
	// rounded to the nearest integer. this is exactly the number the status
	// bar zoom pane shows, so percent based stepping lands on the values
	// that are displayed.
	int rw;
	int rh;
	
	if ((!_viv_image_wide) || (!_viv_image_high))
	{
		return 100;
	}
	
	_viv_get_render_size(&rw,&rh);
	
	if ((!rw) || (!rh))
	{
		return 100;
	}
	
	{
		double zoom_x;
		double zoom_y;
		
		zoom_x = (double)rw / (double)_viv_image_wide;
		zoom_y = (double)rh / (double)_viv_image_high;
		
		return (int)((((zoom_x + zoom_y) / 2.0) * 100.0) + 0.5);
	}
}
// the ladder position whose displayed percent is closest to percent.
// the render size, and so the displayed percent, is monotonic in the
// position, so a binary search finds it in ~10 measurements instead of
// walking up to 1024 positions. NOTE: the search runs through
// _viv_zoom_pos, so the caller must use the returned position.
int _viv_zoom_pos_for_percent(int percent,int strict)
{
	int lo;
	int hi;
	
	lo = _viv_zoom_pos_floor();
	hi = _viv_zoom_pos_max() + 1; // exclusive upper bound
	
	while (lo < hi)
	{
		int mid;
		
		mid = lo + ((hi - lo) / 2);
		
		_viv_zoom_pos = mid;
		
		if (_viv_zoom_percent() >= percent)
		{
			hi = mid;
		}
		else
		{
			lo = mid + 1;
		}
	}
	
	// the target can be above the top of the ladder: clamp.
	if (lo > _viv_zoom_pos_max())
	{
		lo = _viv_zoom_pos_max();
	}
	
	// the search lands on the first position that reaches the target
	// percent. the position below it may be closer: pick the better one.
	// strict callers (the force fallback) want the first position that
	// actually reaches the target so the jump always moves forward.
	if ((lo > 0) && (!strict))
	{
		int below;
		int at;
		
		_viv_zoom_pos = lo - 1;
		below = _viv_zoom_percent();
		
		_viv_zoom_pos = lo;
		at = _viv_zoom_percent();
		
		if ((percent - below) < (at - percent))
		{
			lo = lo - 1;
		}
	}
	
	return lo;
}
// #define PHOTO_ORIENTATION_NORMAL            1u
// #define PHOTO_ORIENTATION_FLIPHORIZONTAL    2u
// #define PHOTO_ORIENTATION_ROTATE180         3u
// #define PHOTO_ORIENTATION_FLIPVERTICAL      4u
// #define PHOTO_ORIENTATION_TRANSPOSE         5u
// #define PHOTO_ORIENTATION_ROTATE270         6u
// #define PHOTO_ORIENTATION_TRANSVERSE        7u
// #define PHOTO_ORIENTATION_ROTATE90          8u
HBITMAP _viv_orientate_hbitmap(HBITMAP hbitmap,int orientation)
{
	BITMAP bitmap;
	HBITMAP ret_hbitmap;
	
	ret_hbitmap = 0;
	
	if (GetObject(hbitmap,sizeof(BITMAP),&bitmap))
	{
		HDC screen_hdc;
		
		screen_hdc = GetDC(0);
		if (screen_hdc)
		{
			HDC mem_hdc;
			
			mem_hdc = CreateCompatibleDC(screen_hdc);
			if (mem_hdc)
			{
				int ret_wide;
				int ret_high;
				
				ret_wide = bitmap.bmWidth;
				ret_high = bitmap.bmHeight;
				
				switch(orientation)
				{
					case 5: // #define PHOTO_ORIENTATION_TRANSPOSE         5u
					case 6: // #define PHOTO_ORIENTATION_ROTATE270         6u
					case 7: // #define PHOTO_ORIENTATION_TRANSVERSE        7u
					case 8: // #define PHOTO_ORIENTATION_ROTATE90          8u
						
						ret_wide = bitmap.bmHeight;
						ret_high = bitmap.bmWidth;
						
						break;
				}
				
				ret_hbitmap = CreateCompatibleBitmap(screen_hdc,ret_wide,ret_high);
				if (ret_hbitmap)
				{
					int ret;
					DWORD *old_pixels;
					DWORD *new_pixels;
					BITMAPINFOHEADER bi;
					
					ret = 0;
					
					os_zero_memory(&bi,sizeof(BITMAPINFOHEADER));
					bi.biSize = sizeof(BITMAPINFOHEADER);
					bi.biWidth = bitmap.bmWidth;
					bi.biHeight = -bitmap.bmHeight;
					bi.biPlanes = 1;
					bi.biBitCount = 32;
					bi.biCompression = BI_RGB;
										
					// SIZE_T casts: the 32 bit int intermediate can overflow
					// before it ever reaches mem_alloc's uintptr_t parameter.
					old_pixels = mem_alloc(safe_size_mul(safe_size_mul((SIZE_T)bitmap.bmWidth,(SIZE_T)bitmap.bmHeight),sizeof(DWORD)));
					new_pixels = mem_alloc(safe_size_mul(safe_size_mul((SIZE_T)ret_wide,(SIZE_T)ret_high),sizeof(DWORD)));
				
					if (GetDIBits(mem_hdc,hbitmap,0,bitmap.bmHeight,old_pixels,(BITMAPINFO *)&bi,DIB_RGB_COLORS))
					{
						int y;

						switch(orientation)
						{
							case 2: // #define PHOTO_ORIENTATION_FLIPHORIZONTAL    2u
							
								for(y=0;y<bitmap.bmHeight;y++)
								{
									int x;
									
									for(x=0;x<bitmap.bmWidth;x++)
									{
										new_pixels[x + (y * ret_wide)] = old_pixels[bitmap.bmWidth - x - 1 + (y * bitmap.bmWidth)];
									}
								}
								
								break;
							
							case 3: // #define PHOTO_ORIENTATION_ROTATE180         3u
							
								for(y=0;y<bitmap.bmHeight;y++)
								{
									int x;
									
									for(x=0;x<bitmap.bmWidth;x++)
									{
										new_pixels[x + (y * ret_wide)] = old_pixels[bitmap.bmWidth - x - 1 + ((bitmap.bmHeight - y - 1) * bitmap.bmWidth)];
									}
								}
								
								break;
								
							case 4: // #define PHOTO_ORIENTATION_FLIPVERTICAL      4u
							
								for(y=0;y<bitmap.bmHeight;y++)
								{
									int x;
									
									for(x=0;x<bitmap.bmWidth;x++)
									{
										new_pixels[x + (y * ret_wide)] = old_pixels[x + ((bitmap.bmHeight - y - 1) * bitmap.bmWidth)];
									}
								}
								
								break;
								
							case 5: // #define PHOTO_ORIENTATION_TRANSPOSE         5u
								
								for (y = 0; y < bitmap.bmWidth; y++)
								{
									int x;
									
									for (x = 0; x < bitmap.bmHeight; x++)
									{
										// Transpose: new(x, y) = old(y, x)
										new_pixels[x + (y * ret_wide)] = old_pixels[y + (x * bitmap.bmWidth)];
									}
								}

								break;
								
							case 6: // #define PHOTO_ORIENTATION_ROTATE270         6u
								
								for(y=0;y<bitmap.bmWidth;y++)
								{
									int x;
									
									for(x=0;x<bitmap.bmHeight;x++)
									{
										new_pixels[x + (y * ret_wide)] = old_pixels[y + ((bitmap.bmHeight-x-1) * bitmap.bmWidth)];
									}
								}		
								
								break;
	
							case 7: // #define PHOTO_ORIENTATION_TRANSVERSE        7u
								
								for (y = 0; y < bitmap.bmWidth; y++)
								{
									int x;
									
									for (x = 0; x < bitmap.bmHeight; x++)
									{
										new_pixels[x + (y * ret_wide)] = old_pixels[(bitmap.bmWidth - 1 - y) + ((bitmap.bmHeight - 1 - x) * bitmap.bmWidth)];
									}
								}

								break;
								
							case 8: // #define PHOTO_ORIENTATION_ROTATE90          8u
							
								for(y=0;y<bitmap.bmWidth;y++)
								{
									int x;
									
									for(x=0;x<bitmap.bmHeight;x++)
									{
										new_pixels[x + (y * ret_wide)] = old_pixels[(bitmap.bmWidth-y-1) + (x * bitmap.bmWidth)];
									}
								}

								break;
						}
						
						os_zero_memory(&bi,sizeof(BITMAPINFOHEADER));
						bi.biSize = sizeof(BITMAPINFOHEADER);
						bi.biWidth = ret_wide;
						bi.biHeight = -ret_high;
						bi.biPlanes = 1;
						bi.biBitCount = 32;
						bi.biCompression = BI_RGB;
						
						if (SetDIBits(mem_hdc,ret_hbitmap,0,ret_high,new_pixels,(BITMAPINFO *)&bi,DIB_RGB_COLORS))
						{
							ret = 1;
						}
					}
					
					if (!ret)
					{
						DeleteObject(ret_hbitmap);
						
						ret_hbitmap = 0;	
					}
					
					mem_free(new_pixels);
					mem_free(old_pixels);
				}
				
				DeleteDC(mem_hdc);
			}
			
			ReleaseDC(0,screen_hdc);
		}
	}
	
	return ret_hbitmap;
}
HBITMAP _viv_get_mipmap(HBITMAP hbitmap,int image_wide,int image_high,int render_wide,int render_high,int *pmip_wide,int *pmip_high,_viv_mipmap_t **out_mip)
{
	int mip_wide;
	int mip_high;
	_viv_mipmap_t **pmip;
	int depth;
	HBITMAP best_hbitmap;
	int best_wide;
	int best_high;

//	debug_printf("GETMIP %d x %d => %d x %d\n",image_wide,image_high,render_wide,render_high);
		
	mip_wide = (image_wide + 1) / 2;
	mip_high = (image_high + 1) / 2;
		
	if (mip_wide < 1)
	{
		mip_wide = 1;
	}
	
	if (mip_high < 1)
	{
		mip_high = 1;
	}
		
	if ((mip_wide == 1) && (mip_high == 1))
	{
		*pmip_wide = image_wide;
		*pmip_high = image_high;

//			debug_printf("MIP %d: %d %d\n",1,image_wide,image_high);

		return hbitmap;
	}
	
	// use the full image only when it is being magnified (render at or above
	// the full image size on either axis). between the half size and the full
	// size the half mipmap is magnified instead: entering the full image there
	// ran the slow shrink filter (HALFTONE default) over every pixel of the
	// image on every paint, which was the zoom stall when a pinch or wheel
	// zoom crossed the half-size boundary. the half mipmap is 4x cheaper and
	// the difference is invisible at these zoom levels.
	if ((render_wide >= image_wide) || (render_high >= image_high))
	{
		*pmip_wide = image_wide;
		*pmip_high = image_high;

//			debug_printf("MIP %d: %d %d\n",1,image_wide,image_high);

		return hbitmap;
	}
	
	best_hbitmap = hbitmap;
	best_wide = image_wide;
	best_high = image_high;
	
	pmip = out_mip;
	
	depth = 1;
	
	for(;;)
	{
		if (!*pmip)
		{
			HDC screen_hdc;
			HDC mem_hdc;
			HDC mem2_hdc;
			HGDIOBJ last_hbitmap;
			HGDIOBJ last2_hbitmap;
			int last_stretch_mode;
			
			debug_printf("GETMIPMAP %d: %d %d\n",depth,mip_wide,mip_high);
			
			// create mipmap..
			*pmip = mem_alloc(sizeof(_viv_mipmap_t));
			
			screen_hdc = GetDC(0);
			if (screen_hdc)
			{
				mem_hdc = CreateCompatibleDC(screen_hdc);
				if (mem_hdc)
				{
					mem2_hdc = CreateCompatibleDC(screen_hdc);
					if (mem2_hdc)
					{
						(*pmip)->mipmap = NULL;
						(*pmip)->hbitmap = CreateCompatibleBitmap(screen_hdc,mip_wide,mip_high);
						
						last_hbitmap = SelectObject(mem_hdc,(*pmip)->hbitmap);
						last2_hbitmap = SelectObject(mem2_hdc,best_hbitmap);
						
						last_stretch_mode = SetStretchBltMode(mem_hdc,HALFTONE);
						
						// for crazy large images -https://github.com/voidtools/voidImageViewer/issues/45
						// use stitching, since we are using HALFTONE here, we will end up with sharp tile edges
						// but's its better than showing a black image.
						if (_viv_StretchBltStitch(mem_hdc,0,0,mip_wide,mip_high,mem2_hdc,0,0,best_wide,best_high,SRCCOPY,0,0,mip_wide,mip_high))
						{
							// OK
						}
						else
						{
							debug_printf("get_mipmap %d %d failed %u\n",mip_wide,mip_high,GetLastError());
						}

						SetStretchBltMode(mem_hdc,last_stretch_mode);
						
						SelectObject(mem_hdc,last_hbitmap);
						SelectObject(mem2_hdc,last2_hbitmap);

						DeleteDC(mem2_hdc);
					}
					
					DeleteDC(mem_hdc);
				}
				
				ReleaseDC(0,screen_hdc);
			}
		}

		best_wide = mip_wide;
		best_high = mip_high;
		best_hbitmap = (*pmip)->hbitmap;
		
		mip_wide = (image_wide + 1) / (2<<depth);
		mip_high = (image_high + 1) / (2<<depth);
		
		if (mip_wide < 1)
		{
			mip_wide = 1;
		}
		
		if (mip_high < 1)
		{
			mip_high = 1;
		}
		
		if ((mip_wide == 1) && (mip_high == 1))
		{
			*pmip_wide = best_wide;
			*pmip_high = best_high;
			
//				debug_printf("MIP %d: %d %d\n",depth,best_wide,best_high);
			
			return best_hbitmap;
		}
		
		if ((render_wide >= mip_wide) || (render_high >= mip_high))
		{
			*pmip_wide = best_wide;
			*pmip_high = best_high;
			
//				debug_printf("MIP %d: %d %d\n",depth,best_wide,best_high);
			
			return best_hbitmap;
		}
		
		pmip = &(*pmip)->mipmap;
		depth++;
	}
}
static int _viv_ceil(double x) 
{
    int xi = (int)x;
    
    return (x > (double)xi) ? xi + 1 : xi;
}
// stretch a src-HDC with a selected bitmap to a distination-HDC.
// only stretch the specified clipping region (hopefully small from a scroll)
// this is way faster than StretchBlt as StretchBlt stretches the entire image, ignoring the clipping region.
void _viv_stretch_blt(HDC dst_hdc,int dst_x,int dst_y,int dst_wide,int dst_high,HDC src_hdc,int src_wide,int src_high,int clip_x,int clip_y,int clip_wide,int clip_high)
{
	// clamp clip rectangle inside dst region
	if (clip_x < dst_x) 
	{
		clip_wide -= (dst_x - clip_x);
		clip_x = dst_x;
	}
	
	if (clip_y < dst_y) 
	{
		clip_high -= (dst_y - clip_y);
		clip_y = dst_y;
	}
	
	if (clip_x + clip_wide > dst_x + dst_wide) 
	{
		clip_wide = (dst_x + dst_wide) - clip_x;
	}
	
	if (clip_y + clip_high > dst_y + dst_high) 
	{
		clip_high = (dst_y + dst_high) - clip_y;
	}

	// something to do?
	if ((dst_wide > 0) && (dst_high > 0) && (clip_wide > 0) && (clip_high > 0))
	{
		int src_clip_x;
		int src_clip_y;
		int src_clip_wide;
		int src_clip_high;
		HDC dst_mem_hdc;
		
		src_clip_x = ((clip_x - dst_x) * (__int64)src_wide) / dst_wide;
		src_clip_y = ((clip_y - dst_y) * (__int64)src_high) / dst_high;
		src_clip_wide = (((clip_wide - 1 + clip_x - dst_x) * (__int64)src_wide) / dst_wide) + 1 - src_clip_x;
		src_clip_high = (((clip_high - 1 + clip_y - dst_y) * (__int64)src_high) / dst_high) + 1 - src_clip_y;
		
	//debug_printf("%d %d %d %d - %d %d %d %d\n",src_clip_x,src_clip_y,src_clip_wide,src_clip_high,clip_x,clip_y,clip_wide,clip_high);
			
		dst_mem_hdc = CreateCompatibleDC(dst_hdc);
		if (dst_mem_hdc)
		{
			HBITMAP dst_hbitmap;

			dst_hbitmap = CreateCompatibleBitmap(dst_hdc,clip_wide,clip_high);
			if (dst_hbitmap)
			{
				HGDIOBJ last_dst_hbitmap;

				last_dst_hbitmap = SelectObject(dst_mem_hdc,dst_hbitmap);
				
				// debug_printf("STRETCH %d %d %d %d - %d %d clip %d %d\n",dst_x-clip_x,dst_y-clip_y,dst_wide,dst_high,src_wide,src_high,clip_wide,clip_high);
//						StretchBlt(dst_hdc,dst_x,dst_y,dst_wide,dst_high,src_hdc,0,0,src_wide,src_high,SRCCOPY);
			
				SetStretchBltMode(dst_mem_hdc,GetStretchBltMode(dst_hdc));
			
				if (_viv_StretchBltStitch(dst_mem_hdc,dst_x-clip_x,dst_y-clip_y,dst_wide,dst_high,src_hdc,0,0,src_wide,src_high,SRCCOPY,0,0,clip_wide,clip_high))
				{
				}
				else
				{
					debug_printf("StretchBlt %d %d %d %d %d %d failed %u",dst_x-clip_x,dst_y-clip_y,dst_wide,dst_high,src_wide,src_high,GetLastError());
				}
				
				if (BitBlt(dst_hdc,clip_x,clip_y,clip_wide,clip_high,dst_mem_hdc,0,0,SRCCOPY))
				{
					
				}
				else
				{
					debug_printf("BitBlt %d %d %d %d failed %u",clip_x,clip_y,clip_wide,clip_high,GetLastError());
				}

				SelectObject(dst_mem_hdc,last_dst_hbitmap);
				
				// Cleanup
				DeleteObject(dst_hbitmap);
			}

			DeleteDC(dst_mem_hdc);
		}
	}
}
// stitch multiple 1024x1024 stretches together.
// required to render images over 32768x32768
// only works if using COLORONCOLOR
BOOL _viv_StretchBltStitch(HDC hdcDest,int xDest,int yDest,int wDest,int hDest,HDC hdcSrc,int xSrc,int ySrc,int wSrc,int hSrc,DWORD rop,int clip_x,int clip_y,int clip_wide,int clip_high)
{
debug_printf("STRETCHBLT1024 %d %d %d %d SRC %d %d %d %d\n",xDest,yDest,wDest,hDest,xSrc,ySrc,wSrc,hSrc);

	if ((wDest < 32768) && (hDest < 32768) && (wSrc < 32768) && (hSrc < 32768))
	{
		return StretchBlt(hdcDest,xDest,yDest,wDest,hDest,hdcSrc,xSrc,ySrc,wSrc,hSrc,rop);
	}
	else
	if ((wDest>0) && (hDest>0) && (wSrc>0) && (hSrc>0))
	{
		int src_y;
		int src_yrun;
		int dst_y;
		int clip_right;
		int clip_bottom;
		
		dst_y = yDest;
		src_y = ySrc;
		src_yrun = hSrc;
		clip_right = clip_x + clip_wide;
		clip_bottom = clip_y + clip_high;
		
		while(src_yrun)
		{
			int src_high;
			int dst_y2;
			
			if (dst_y >= clip_bottom)
			{
				break;
			}
			
			src_high = src_yrun > _VIV_STRETCH_BLT_STITCH_SIZE ? _VIV_STRETCH_BLT_STITCH_SIZE : src_yrun;

			dst_y2 = (((src_y + src_high) * (__int64)hDest) / hSrc) + yDest;

			if (dst_y2 >= clip_y)
			{
				int src_x;
				int src_xrun;
				int dst_x;
				int dst_high;

				dst_x = xDest;
				dst_high = dst_y2 - dst_y;
				
				src_x = xSrc;
				src_xrun = wSrc;
				
				while(src_xrun)
				{
					int dst_x2;
					int dst_wide;
					int src_wide;
					
					src_wide = src_xrun > _VIV_STRETCH_BLT_STITCH_SIZE ? _VIV_STRETCH_BLT_STITCH_SIZE : src_xrun;
					
					dst_x2 = (((src_x + src_wide) * (__int64)wDest) / wSrc) + xDest;
					
					dst_wide = dst_x2 - dst_x;
					
					if (dst_x >= clip_right)
					{
						break;
					}

					if (dst_x2 > clip_x)
					{
	debug_printf("DST %d %d %d %d SRC %d %d %d %d CLIP %d %d\n",dst_x,dst_y,dst_wide,dst_high,src_x,src_y,src_wide,src_high,dst_x2,clip_x);
						if (!StretchBlt(hdcDest,dst_x,dst_y,dst_wide,dst_high,hdcSrc,src_x,src_y,src_wide,src_high,rop))
						{
							return FALSE;
						}
					}
					
					dst_x = dst_x2;
					src_x += src_wide;
					src_xrun -= src_wide;
				}
			}
			
			dst_y = dst_y2;
			src_y += src_high;
			src_yrun -= src_high;
		}
	}
	
	return TRUE;
}
// get the source pixel coordinate.
// doesn't use mipmaps.
static BOOL _viv_get_src_pixel_pos(int client_x,int client_y,POINT *out_pixel_pt)
{
	RECT client_rect;
	int wide;
	int high;
	int rx;
	int ry;
	int rw;
	int rh;
	
	GetClientRect(_viv_hwnd,&client_rect);
	wide = client_rect.right - client_rect.left;
	high = client_rect.bottom - client_rect.top - _viv_get_status_high() - _viv_get_view_top();

	if (_viv_frame_count)
	{
		_viv_get_render_size(&rw,&rh);


		rx = (((_viv_dst_pos_x - 250) * (wide*2)) / 1000) - (rw / 2) - _viv_view_x;
		// the viewport origin: the mouse lands in client coords.
		ry = (((_viv_dst_pos_y - 250) * (high*2)) / 1000) - (rh / 2) - _viv_view_y + _viv_get_view_top();
		
		if ((rw) && (rh))
		{
			if ((client_x >= rx) && (client_y >= ry) && (client_x < rx + rw) && (client_y < ry + rh))
			{
				out_pixel_pt->x = ((client_x - rx) * (__int64)_viv_image_wide) / rw;
				out_pixel_pt->y = ((client_y - ry) * (__int64)_viv_image_high) / rh;
				
				return TRUE;
			}
		}
	}
	
	return FALSE;
}
// get the source pixel color from the specified position.
// doesn't use mipmaps.
static void _viv_get_src_pixel_rgb(int src_x,int src_y,COLORREF *out_colorref)
{
	RECT client_rect;
	int wide;
	int high;
	int rx;
	int ry;
	int rw;
	int rh;
	
	GetClientRect(_viv_hwnd,&client_rect);
	wide = client_rect.right - client_rect.left;
	high = client_rect.bottom - client_rect.top - _viv_get_status_high() - _viv_get_view_top();

	if (_viv_frame_count)
	{
		HDC screen_hdc;
		
		screen_hdc = GetDC(0);
		
		if (screen_hdc)
		{
			HDC mem_hdc;
			
			_viv_get_render_size(&rw,&rh);


			rx = (((_viv_dst_pos_x - 250) * (wide*2)) / 1000) - (rw / 2) - _viv_view_x;
			ry = (((_viv_dst_pos_y - 250) * (high*2)) / 1000) - (rh / 2) - _viv_view_y;
			
			mem_hdc = CreateCompatibleDC(screen_hdc);
			if (mem_hdc)
			{
				HGDIOBJ last_hbitmap;
				
				last_hbitmap = SelectObject(mem_hdc,_viv_frames[_viv_frame_position].hbitmap);
				
				if (last_hbitmap)
				{
					*out_colorref = GetPixel(mem_hdc,src_x,src_y);
					
					SelectObject(mem_hdc,last_hbitmap);
				}
				
				DeleteDC(mem_hdc);
			}
			
			ReleaseDC(0,screen_hdc);
		}
	}
}
// the lowest ladder position the zoom may reach. position 0 (the best fit)
// was a hard floor historically: a fill-window upscale locked small images
// at a 200% minimum and a windowed fit locked large ones at their shrink
// size, with no way to zoom out to a thumbnail (the touch pinch field
// report). the ladder now extends below the fit by the same 16x factor the
// cap extends above native - but only while shrinking is allowed: the
// option exists exactly to keep images at or above their fit size.
int _viv_zoom_pos_floor(void)
{
	if (!config_allow_shrinking)
	{
		return 0;
	}
	
	return -_VIV_ZOOM_SHRINK_STEPS;
}
int _viv_clamp_zoom_pos(int zoom_pos)
{
	int pos_max;
	int pos_floor;
	
	pos_floor = _viv_zoom_pos_floor();
	
	// the below-fit floor is a constant (the extension is fit-relative), so
	// unlike the measured top there is nothing to walk: clamp straight to it.
	if (zoom_pos <= pos_floor)
	{
		return pos_floor;
	}
	
	{
		
		// the top of the ladder depends on the image, the window and the fill
		// settings (the 16x native cap), so it is measured, not hardcoded.
		pos_max = _viv_zoom_pos_max();
		
		if (zoom_pos > pos_max)
		{
			return pos_max;
		}
	}
	
	return zoom_pos;
}
