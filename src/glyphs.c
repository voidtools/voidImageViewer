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
// vector glyph icon rendering. the sixteen toolbar / zoom control icons
// are drawn as 48x48 grid line art (round capped strokes, the media transport
// and magnifier metaphors of the previous .ico frames) into a 32bpp argb
// bitmap through the gdi+ flat api, then converted to an HICON. icons are
// cached per (glyph, theme, size) so the hot draw paths never rebuild.

#include "viv.h"
#include "glyphs.h"

// point in the 48x48 design grid.
typedef struct _glyphs_point_s
{
	int x;
	int y;
}_glyphs_point_t;

typedef struct _glyphs_point_f_s
{
	float x;
	float y;
}_glyphs_point_f_t;

// one stroke: a polyline with a round capped pen of the given grid width.
// points == NULL draws a perfect ellipse through the ellipse field instead
// (the magnifier ring: a hand written 21 point ring always wobbles between
// radii, each vertex sat a slightly different distance from the center, and
// the wobble reads as a lumpy lens once the pill renders past 40 pixels).
typedef struct _glyphs_ellipse_s
{
	float cx;
	float cy;
	float rx;
	float ry;
}_glyphs_ellipse_t;

// one stroke: a true arc through the arc field (the rotate glyph: a sampled
// polyline always wobbles the same way a hand written ring did, so the
// three quarter circle is one arc primitive instead).
typedef struct _glyphs_arc_s
{
	float cx;
	float cy;
	float rx;
	float ry;
	float start; // degrees, clockwise from the x axis (the y down grid).
	float sweep; // degrees, positive = clockwise.
}_glyphs_arc_t;

typedef struct _glyphs_stroke_s
{
	int point_count; // 48 grid units; 0 = the ellipse / arc stroke below.
	int width; // 48 grid units.
	const _glyphs_point_t *points; // NULL = use ellipse or arc.
	const _glyphs_ellipse_t *ellipse; // used when points == NULL.
	const _glyphs_arc_t *arc; // used when points and ellipse are NULL.
}_glyphs_stroke_t;

typedef struct _glyphs_glyph_s
{
	int stroke_count;
	const _glyphs_stroke_t *strokes;
}_glyphs_glyph_t;

// gdi+ flat api table, resolved from gdiplus.dll on first use.
static int (__stdcall *_glyphs_gdipCreatePen1)(unsigned int argb,float width,int unit,void **pen) = 0;
static int (__stdcall *_glyphs_gdipSetPenStartCap)(void *pen,int cap) = 0;
static int (__stdcall *_glyphs_gdipSetPenEndCap)(void *pen,int cap) = 0;
static int (__stdcall *_glyphs_gdipDeletePen)(void *pen) = 0;
static int (__stdcall *_glyphs_gdipDrawLinesI)(void *graphics,void *pen,const _glyphs_point_t *points,int count) = 0;
static int (__stdcall *_glyphs_gdipDrawLinesF)(void *graphics,void *pen,const _glyphs_point_f_t *points,int count) = 0;
static int (__stdcall *_glyphs_gdipDrawEllipseF)(void *graphics,void *pen,float x,float y,float width,float height) = 0;
static int (__stdcall *_glyphs_gdipDrawArcF)(void *graphics,void *pen,float x,float y,float width,float height,float start_angle,float sweep_angle) = 0;
static int (__stdcall *_glyphs_gdipCreateBitmapFromScan0)(int wide,int high,int stride,int format,unsigned char *scan0,void **bitmap) = 0;
static int (__stdcall *_glyphs_gdipGetImageGraphicsContext)(void *image,void **graphics) = 0;
static int (__stdcall *_glyphs_gdipSetSmoothingMode)(void *graphics,int mode) = 0;
static int (__stdcall *_glyphs_gdipDeleteGraphics)(void *graphics) = 0;
static int (__stdcall *_glyphs_gdipDisposeImage)(void *image) = 0;
static int (__stdcall *_glyphs_gdipCreateHICONFromBitmap)(void *bitmap,HICON *icon) = 0;

// 0 = not loaded, 1 = ready, 2 = permanently unavailable.
static int _glyphs_state = 0;
static ULONG_PTR _glyphs_gdiplus_token = 0;

// cache: linear table, the oldest entry is evicted when full.
#define _GLYPHS_CACHE_MAX 36

typedef struct _glyphs_cache_entry_s
{
	int glyph;
	int dark;
	int size;
	HICON icon;
}_glyphs_cache_entry_t;

static _glyphs_cache_entry_t _glyphs_cache[_GLYPHS_CACHE_MAX];
static int _glyphs_cache_count = 0;

// theme stroke colors (argb).
#define _GLYPHS_COLOR_DARK  0xFFE8E8E8
#define _GLYPHS_COLOR_LIGHT 0xFF3C4043

// gdi+ constants used below.
// PixelFormat32bppARGB, UnitPixel, LineCapRound, SmoothingModeAntiAlias.
#define _GLYPHS_PIXEL_FORMAT_32ARGB 0x26200A
#define _GLYPHS_UNIT_PIXEL 2
#define _GLYPHS_LINE_CAP_ROUND 2
#define _GLYPHS_SMOOTHING_ANTIALIAS 4

// the magnifier ring: a true ellipse centered (21,21) with radius 13. the
// optical center of the whole glyph sits at (23.5,23.5) because the handle
// reaches to the bottom right, so the ring is deliberately offset one and a
// half units up-left of the 24x24 grid center to keep the composite centered.
static const _glyphs_ellipse_t _glyphs_zoom_ring = { 21.0f,21.0f,13.0f,13.0f };

// prev: a left pointing triangle with a bar on its left.
static const _glyphs_point_t _glyphs_prev_tri[] = { {32,12},{16,24},{32,36},{32,12} };
static const _glyphs_point_t _glyphs_prev_bar[] = { {8,14},{8,34} };
static const _glyphs_stroke_t _glyphs_prev_strokes[] =
{
	{4,4,_glyphs_prev_tri},
	{2,6,_glyphs_prev_bar}
};

// next: a right pointing triangle with a bar on its right.
static const _glyphs_point_t _glyphs_next_tri[] = { {16,12},{32,24},{16,36},{16,12} };
static const _glyphs_point_t _glyphs_next_bar[] = { {40,14},{40,34} };
static const _glyphs_stroke_t _glyphs_next_strokes[] =
{
	{4,4,_glyphs_next_tri},
	{2,6,_glyphs_next_bar}
};

// play: a right pointing triangle.
static const _glyphs_point_t _glyphs_play_tri[] = { {18,12},{36,24},{18,36},{18,12} };
static const _glyphs_stroke_t _glyphs_play_strokes[] =
{
	{4,4,_glyphs_play_tri}
};

// pause: two vertical bars.
static const _glyphs_point_t _glyphs_pause_bar1[] = { {18,14},{18,34} };
static const _glyphs_point_t _glyphs_pause_bar2[] = { {30,14},{30,34} };
static const _glyphs_stroke_t _glyphs_pause_strokes[] =
{
	{2,8,_glyphs_pause_bar1},
	{2,8,_glyphs_pause_bar2}
};

// bestfit: corner brackets with a single headed diagonal arrow (shrink
// the image into the frame).
static const _glyphs_point_t _glyphs_fit_tl[] = { {12,20},{12,12},{20,12} };
static const _glyphs_point_t _glyphs_fit_tr[] = { {28,12},{36,12},{36,20} };
static const _glyphs_point_t _glyphs_fit_br[] = { {36,28},{36,36},{28,36} };
static const _glyphs_point_t _glyphs_fit_bl[] = { {20,36},{12,36},{12,28} };
static const _glyphs_point_t _glyphs_fit_shaft[] = { {30,18},{18,30} };
static const _glyphs_point_t _glyphs_fit_head[] = { {18,22},{18,30},{26,30} };
static const _glyphs_stroke_t _glyphs_bestfit_strokes[] =
{
	{3,4,_glyphs_fit_tl},
	{3,4,_glyphs_fit_tr},
	{3,4,_glyphs_fit_br},
	{3,4,_glyphs_fit_bl},
	{2,4,_glyphs_fit_shaft},
	{3,4,_glyphs_fit_head}
};

// 1:1: corner brackets with a double headed diagonal arrow (the actual
// size, both directions).
static const _glyphs_point_t _glyphs_1to1_shaft[] = { {16,32},{32,16} };
static const _glyphs_point_t _glyphs_1to1_head1[] = { {16,24},{16,32},{24,32} };
static const _glyphs_point_t _glyphs_1to1_head2[] = { {24,16},{32,16},{32,24} };
static const _glyphs_stroke_t _glyphs_1to1_strokes[] =
{
	{3,4,_glyphs_fit_tl},
	{3,4,_glyphs_fit_tr},
	{3,4,_glyphs_fit_br},
	{3,4,_glyphs_fit_bl},
	{2,4,_glyphs_1to1_shaft},
	{3,4,_glyphs_1to1_head1},
	{3,4,_glyphs_1to1_head2}
};

// zoom out: magnifier with a minus, handle to the bottom right.
// the previous revision declared three point polyline strokes over two
// element arrays: the renderer walked one element past each array and drew
// a stray segment to whatever (x,y) pair lived in the adjacent memory, which
// is the crooked tail the field screenshots caught on the cross and the
// handle. the counts now match the arrays, the ring is a true ellipse, the
// handle starts exactly on the 45 degree ring point, and the cross strokes
// grew one unit longer and one wider so they survive the small toolbar sizes
// instead of collapsing into a capped smudge.
static const _glyphs_point_t _glyphs_zoomout_handle[] = { {30,30},{38,38} };
static const _glyphs_point_t _glyphs_zoomout_minus[] = { {15,21},{27,21} };
static const _glyphs_stroke_t _glyphs_zoomout_strokes[] =
{
	{0,6,0,&_glyphs_zoom_ring},
	{2,5,_glyphs_zoomout_handle},
	{2,5,_glyphs_zoomout_minus}
};

// zoom in: magnifier with a plus. the cross is exactly symmetric about the
// ring center (21,21): both arms run 15 to 27 on their axis.
static const _glyphs_point_t _glyphs_zoomin_handle[] = { {30,30},{38,38} };
static const _glyphs_point_t _glyphs_zoomin_minus[] = { {15,21},{27,21} };
static const _glyphs_point_t _glyphs_zoomin_plus[] = { {21,15},{21,27} };
static const _glyphs_stroke_t _glyphs_zoomin_strokes[] =
{
	{0,6,0,&_glyphs_zoom_ring},
	{2,5,_glyphs_zoomin_handle},
	{2,5,_glyphs_zoomin_minus},
	{2,5,_glyphs_zoomin_plus}
};

// folder open: the back tab panel with the tilted front flap (the classic
// open folder; the flap reaches past the back panel's right edge).
static const _glyphs_point_t _glyphs_folder_back[] = { {6,34},{6,13},{16,13},{20,17},{38,17},{38,21} };
static const _glyphs_point_t _glyphs_folder_flap[] = { {6,34},{12,22},{42,22},{36,34},{6,34} };
static const _glyphs_stroke_t _glyphs_folder_open_strokes[] =
{
	{6,4,_glyphs_folder_back},
	{5,4,_glyphs_folder_flap}
};

// rotate cw: a three quarter arc as a true arc primitive, the arrow head at
// the top end pointing clockwise (the 45 degree notch sits between the head
// and the arc tail).
static const _glyphs_arc_t _glyphs_rotate_arc = { 24.0f,24.0f,13.0f,13.0f,315.0f,315.0f };
static const _glyphs_point_t _glyphs_rotate_head[] = { {19,6},{24,11},{19,16} };
static const _glyphs_stroke_t _glyphs_rotate_strokes[] =
{
	{0,5,0,0,&_glyphs_rotate_arc},
	{3,5,_glyphs_rotate_head}
};

// info: a thin ring with the i (dot and stem) kept clear of the ring band.
static const _glyphs_ellipse_t _glyphs_info_ring = { 24.0f,24.0f,15.5f,15.5f };
static const _glyphs_ellipse_t _glyphs_info_dot = { 24.0f,16.0f,2.0f,2.0f };
static const _glyphs_point_t _glyphs_info_stem[] = { {24,24},{24,33} };
static const _glyphs_stroke_t _glyphs_info_strokes[] =
{
	{0,4,0,&_glyphs_info_ring},
	{0,4,0,&_glyphs_info_dot},
	{2,4,_glyphs_info_stem}
};

// settings: a simplified eight tooth gear (the ring, the radial teeth, the
// center hole). the diagonal teeth round to the grid, the half unit they
// give back disappears under the round caps.
static const _glyphs_ellipse_t _glyphs_gear_ring = { 24.0f,24.0f,10.0f,10.0f };
static const _glyphs_ellipse_t _glyphs_gear_hole = { 24.0f,24.0f,4.0f,4.0f };
static const _glyphs_point_t _glyphs_gear_tooth_e[] = { {33,24},{39,24} };
static const _glyphs_point_t _glyphs_gear_tooth_w[] = { {15,24},{9,24} };
static const _glyphs_point_t _glyphs_gear_tooth_s[] = { {24,33},{24,39} };
static const _glyphs_point_t _glyphs_gear_tooth_n[] = { {24,15},{24,9} };
static const _glyphs_point_t _glyphs_gear_tooth_ne[] = { {30,18},{35,13} };
static const _glyphs_point_t _glyphs_gear_tooth_se[] = { {30,30},{35,35} };
static const _glyphs_point_t _glyphs_gear_tooth_sw[] = { {18,30},{13,35} };
static const _glyphs_point_t _glyphs_gear_tooth_nw[] = { {18,18},{13,13} };
static const _glyphs_stroke_t _glyphs_settings_strokes[] =
{
	{0,4,0,&_glyphs_gear_ring},
	{2,6,_glyphs_gear_tooth_e},
	{2,6,_glyphs_gear_tooth_w},
	{2,6,_glyphs_gear_tooth_s},
	{2,6,_glyphs_gear_tooth_n},
	{2,6,_glyphs_gear_tooth_ne},
	{2,6,_glyphs_gear_tooth_se},
	{2,6,_glyphs_gear_tooth_sw},
	{2,6,_glyphs_gear_tooth_nw},
	{0,4,0,&_glyphs_gear_hole}
};

// picture: the frame, the sun and a two peak mountain line that runs into
// the frame sides.
static const _glyphs_ellipse_t _glyphs_picture_sun = { 17.0f,18.0f,3.0f,3.0f };
static const _glyphs_point_t _glyphs_picture_frame[] = { {8,10},{40,10},{40,38},{8,38},{8,10} };
static const _glyphs_point_t _glyphs_picture_mountains[] = { {8,33},{17,24},{23,30},{30,22},{40,33} };
static const _glyphs_stroke_t _glyphs_picture_strokes[] =
{
	{5,4,_glyphs_picture_frame},
	{0,4,0,&_glyphs_picture_sun},
	{5,4,_glyphs_picture_mountains}
};

// gamepad: a stadium body (two lines plus two true arc caps), the d pad
// cross and the two buttons.
static const _glyphs_point_t _glyphs_pad_top[] = { {16,14},{32,14} };
static const _glyphs_point_t _glyphs_pad_bottom[] = { {32,34},{16,34} };
static const _glyphs_arc_t _glyphs_pad_right_cap = { 32.0f,24.0f,10.0f,10.0f,270.0f,180.0f };
static const _glyphs_arc_t _glyphs_pad_left_cap = { 16.0f,24.0f,10.0f,10.0f,90.0f,180.0f };
static const _glyphs_point_t _glyphs_pad_dpad_v[] = { {16,20},{16,28} };
static const _glyphs_point_t _glyphs_pad_dpad_h[] = { {12,24},{20,24} };
static const _glyphs_ellipse_t _glyphs_pad_button1 = { 35.0f,21.0f,1.5f,1.5f };
static const _glyphs_ellipse_t _glyphs_pad_button2 = { 29.0f,27.0f,1.5f,1.5f };
static const _glyphs_stroke_t _glyphs_gamepad_strokes[] =
{
	{2,4,_glyphs_pad_top},
	{0,4,0,0,&_glyphs_pad_right_cap},
	{2,4,_glyphs_pad_bottom},
	{0,4,0,0,&_glyphs_pad_left_cap},
	{2,4,_glyphs_pad_dpad_v},
	{2,4,_glyphs_pad_dpad_h},
	{0,4,0,&_glyphs_pad_button1},
	{0,4,0,&_glyphs_pad_button2}
};

static const _glyphs_glyph_t _glyphs_table[GLYPH_COUNT] =
{
	{2,_glyphs_prev_strokes},
	{1,_glyphs_play_strokes},
	{2,_glyphs_pause_strokes},
	{2,_glyphs_next_strokes},
	{6,_glyphs_bestfit_strokes},
	{7,_glyphs_1to1_strokes},
	{3,_glyphs_zoomout_strokes},
	{4,_glyphs_zoomin_strokes},
	{2,_glyphs_folder_open_strokes},
	{3,_glyphs_zoomout_strokes},
	{4,_glyphs_zoomin_strokes},
	{2,_glyphs_rotate_strokes},
	{3,_glyphs_info_strokes},
	{10,_glyphs_settings_strokes},
	{3,_glyphs_picture_strokes},
	{8,_glyphs_gamepad_strokes}
};

// resolve the gdi+ flat api table. gdi+ must have been started before
// the flat api is used: viv.c does it during init, and a second startup
// with our own token is explicitly allowed, so do it here as well.
static int _glyphs_load(void)
{
	HMODULE module;

	if (_glyphs_state)
	{
		return (_glyphs_state == 1) ? 1 : 0;
	}

	module = LoadLibraryA("gdiplus.dll");

	if (!module)
	{
		_glyphs_state = 2;

		return 0;
	}

	_glyphs_gdipCreatePen1 = (void *)GetProcAddress(module,"GdipCreatePen1");
	_glyphs_gdipSetPenStartCap = (void *)GetProcAddress(module,"GdipSetPenStartCap");
	_glyphs_gdipSetPenEndCap = (void *)GetProcAddress(module,"GdipSetPenEndCap");
	_glyphs_gdipDeletePen = (void *)GetProcAddress(module,"GdipDeletePen");
	_glyphs_gdipDrawLinesI = (void *)GetProcAddress(module,"GdipDrawLinesI");
	_glyphs_gdipDrawLinesF = (void *)GetProcAddress(module,"GdipDrawLines");
	_glyphs_gdipDrawEllipseF = (void *)GetProcAddress(module,"GdipDrawEllipse");
	_glyphs_gdipDrawArcF = (void *)GetProcAddress(module,"GdipDrawArc");
	_glyphs_gdipCreateBitmapFromScan0 = (void *)GetProcAddress(module,"GdipCreateBitmapFromScan0");
	_glyphs_gdipGetImageGraphicsContext = (void *)GetProcAddress(module,"GdipGetImageGraphicsContext");
	_glyphs_gdipSetSmoothingMode = (void *)GetProcAddress(module,"GdipSetSmoothingMode");
	_glyphs_gdipDeleteGraphics = (void *)GetProcAddress(module,"GdipDeleteGraphics");
	_glyphs_gdipDisposeImage = (void *)GetProcAddress(module,"GdipDisposeImage");
	_glyphs_gdipCreateHICONFromBitmap = (void *)GetProcAddress(module,"GdipCreateHICONFromBitmap");

	if ((!_glyphs_gdipCreatePen1) || (!_glyphs_gdipSetPenStartCap) || (!_glyphs_gdipSetPenEndCap) ||
		(!_glyphs_gdipDeletePen) || (!_glyphs_gdipDrawLinesI) || (!_glyphs_gdipDrawLinesF) ||
		(!_glyphs_gdipDrawEllipseF) || (!_glyphs_gdipDrawArcF) ||
		(!_glyphs_gdipCreateBitmapFromScan0) ||
		(!_glyphs_gdipGetImageGraphicsContext) || (!_glyphs_gdipSetSmoothingMode) ||
		(!_glyphs_gdipDeleteGraphics) || (!_glyphs_gdipDisposeImage) || (!_glyphs_gdipCreateHICONFromBitmap))
	{
		_glyphs_state = 2;

		return 0;
	}

	if (os_GdiplusStartup)
	{
		os_GdiplusStartupInput_t input;

		input.GdiplusVersion = 1;
		input.DebugEventCallback = 0;
		input.SuppressBackgroundThread = 0;
		input.SuppressExternalCodecs = 0;

		os_GdiplusStartup(&_glyphs_gdiplus_token,&input,0);
	}

	_glyphs_state = 1;

	return 1;
}

// render one glyph into a new HICON.
static HICON _glyphs_build(int glyph_id,int dark,int size)
{
	unsigned char *bits;
	void *bitmap;
	void *graphics;
	HICON icon;
	int stride;

	bitmap = 0;
	graphics = 0;
	icon = 0;

	stride = size * 4;

	bits = (unsigned char *)mem_alloc(safe_size_mul((size_t)stride,(size_t)size));

	if (!bits)
	{
		return 0;
	}

	os_zero_memory(bits,(size_t)stride * (size_t)size);

	// the bitmap wraps our buffer without copying it: disposeImage does
	// not free it, we do after the icon was created.
	if (_glyphs_gdipCreateBitmapFromScan0(size,size,stride,_GLYPHS_PIXEL_FORMAT_32ARGB,bits,&bitmap) == 0)
	{
		if (_glyphs_gdipGetImageGraphicsContext(bitmap,&graphics) == 0)
		{
			float scale;
			unsigned int argb;
			int strokei;

			_glyphs_gdipSetSmoothingMode(graphics,_GLYPHS_SMOOTHING_ANTIALIAS);

			scale = (float)size / 48.0f;

			argb = dark ? _GLYPHS_COLOR_DARK : _GLYPHS_COLOR_LIGHT;

			for(strokei=0;strokei<_glyphs_table[glyph_id].stroke_count;strokei++)
			{
				const _glyphs_stroke_t *stroke;
				void *pen;

				stroke = &_glyphs_table[glyph_id].strokes[strokei];

				pen = 0;

				{
					float pen_width;
					
					pen_width = ((float)stroke->width) * scale;
					
					// a sub pixel stroke renders as a faint gray blur at the small
					// toolbar sizes: keep every stroke at least 1.25 device pixels wide.
					if (pen_width < 1.25f)
					{
						pen_width = 1.25f;
					}
					
					if (_glyphs_gdipCreatePen1(argb,pen_width,_GLYPHS_UNIT_PIXEL,&pen) == 0)
					{
						_glyphs_point_f_t *pts;

						// round caps: the strokes end in soft dots instead of
						// square cuts, the signature of the glyph family.
						_glyphs_gdipSetPenStartCap(pen,_GLYPHS_LINE_CAP_ROUND);
						_glyphs_gdipSetPenEndCap(pen,_GLYPHS_LINE_CAP_ROUND);

					if (stroke->points)
					{
						pts = (_glyphs_point_f_t *)mem_alloc(safe_size_mul(sizeof(_glyphs_point_f_t),(size_t)stroke->point_count));

						if (pts)
						{
							int pi;

							for(pi=0;pi<stroke->point_count;pi++)
							{
								pts[pi].x = ((float)stroke->points[pi].x) * scale;
								pts[pi].y = ((float)stroke->points[pi].y) * scale;
							}

							// float coordinates: the float draw api keeps the 48 grid
							// geometry off the pixel lattice, so the small icons stay
							// round instead of lumpy.
							_glyphs_gdipDrawLinesF(graphics,pen,pts,stroke->point_count);

							mem_free(pts);
						}
					}
					else if (stroke->ellipse)
					{
						// the true ellipse stroke: the ring is drawn as a
						// single arc primitive instead of a sampled polyline,
						// so no vertex wobble can reach the lens at any size.
						_glyphs_gdipDrawEllipseF(graphics,pen,(stroke->ellipse->cx - stroke->ellipse->rx) * scale,(stroke->ellipse->cy - stroke->ellipse->ry) * scale,(stroke->ellipse->rx * 2.0f) * scale,(stroke->ellipse->ry * 2.0f) * scale);
					}
					else if (stroke->arc)
					{
						// the true arc stroke: the rotate glyph three quarter
						// circle is one primitive (degrees, the same round capped
						// pen), so no sampled polyline wobble can reach the curve.
						_glyphs_gdipDrawArcF(graphics,pen,(stroke->arc->cx - stroke->arc->rx) * scale,(stroke->arc->cy - stroke->arc->ry) * scale,(stroke->arc->rx * 2.0f) * scale,(stroke->arc->ry * 2.0f) * scale,stroke->arc->start,stroke->arc->sweep);
					}

						_glyphs_gdipDeletePen(pen);
					}
				}
			}

			_glyphs_gdipDeleteGraphics(graphics);
		}

		if (_glyphs_gdipCreateHICONFromBitmap(bitmap,&icon) != 0)
		{
			icon = 0;
		}

		_glyphs_gdipDisposeImage(bitmap);
	}

	mem_free(bits);

	return icon;
}

HICON glyphs_icon(int glyph_id,int dark,int size)
{
	HICON icon;
	int i;

	if (!_glyphs_load())
	{
		return 0;
	}

	if ((glyph_id < 0) || (glyph_id >= GLYPH_COUNT) || (size <= 0))
	{
		return 0;
	}

	for(i=0;i<_glyphs_cache_count;i++)
	{
		if ((_glyphs_cache[i].glyph == glyph_id) && (_glyphs_cache[i].dark == (dark ? 1 : 0)) && (_glyphs_cache[i].size == size))
		{
			return _glyphs_cache[i].icon;
		}
	}

	// evict the oldest entry when the table is full.
	if (_glyphs_cache_count == _GLYPHS_CACHE_MAX)
	{
		if (_glyphs_cache[0].icon)
		{
			DestroyIcon(_glyphs_cache[0].icon);
		}

		// source and destination overlap: copymemory (memcpy) is undefined for
		// overlapping ranges, movememory (memmove) is not.
		os_move_memory(&_glyphs_cache[0],&_glyphs_cache[1],sizeof(_glyphs_cache[0]) * (_GLYPHS_CACHE_MAX - 1));

		_glyphs_cache_count--;
	}

	icon = _glyphs_build(glyph_id,dark ? 1 : 0,size);
	
	// a failed build never caches: a zero entry would shadow every retry
	// (a later flush would blank that glyph for good).
	if (!icon)
	{
		return 0;
	}

	_glyphs_cache[_glyphs_cache_count].glyph = glyph_id;
	_glyphs_cache[_glyphs_cache_count].dark = dark ? 1 : 0;
	_glyphs_cache[_glyphs_cache_count].size = size;
	_glyphs_cache[_glyphs_cache_count].icon = icon;

	return _glyphs_cache[_glyphs_cache_count++].icon;
}

void glyphs_flush_cache(void)
{
	int i;

	for(i=0;i<_glyphs_cache_count;i++)
	{
		if (_glyphs_cache[i].icon)
		{
			DestroyIcon(_glyphs_cache[i].icon);
		}
	}

	_glyphs_cache_count = 0;
}

