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

// viv_theme - see viv_theme.h for the contract.
//
// the whole cost of the system is a few hundred bytes of constants: the dark
// table holds the design tokens, the light side resolves system colors at
// call time (so a system palette change needs no per token bookkeeping), and
// the five accents ship as hand tuned base/hot/down triples with no runtime
// color math anywhere. brushes are created lazily per token and dropped as a
// group on refresh.

#include "viv.h"
#include "viv_state.h"
#include "viv_render.h"
#include "viv_theme.h"

// the dark design tokens.
static const COLORREF _viv_theme_dark_colors[VIV_TK_COUNT] =
{
	RGB(0x23,0x23,0x28),	// VIV_TK_FACE
	RGB(0x1B,0x1B,0x1F),	// VIV_TK_NAV_FACE
	RGB(0x20,0x20,0x24),	// VIV_TK_CHROME
	RGB(0x3A,0x3A,0x40),	// VIV_TK_CHROME_LINE
	RGB(0x70,0x70,0x70),	// VIV_TK_CHROME_MUTED
	RGB(0x1B,0x1B,0x1E),	// VIV_TK_FRAME
	RGB(0x2A,0x2A,0x2F),	// VIV_TK_INPUT
	RGB(0x3A,0x3A,0x40),	// VIV_TK_LINE
	RGB(0x2E,0x2E,0x33),	// VIV_TK_HOVER
	RGB(0x38,0x38,0x3E),	// VIV_TK_DOWN
	RGB(0xE8,0xE8,0xEA),	// VIV_TK_TEXT
	RGB(0x9A,0x9A,0xA0),	// VIV_TK_TEXT2
	RGB(0x6A,0x6A,0x70),	// VIV_TK_TEXTOFF
	RGB(0x00,0x00,0x00),	// VIV_TK_ACCENT (the accent table wins)
	RGB(0x00,0x00,0x00),	// VIV_TK_ACCENT_HOT (the accent table wins)
	RGB(0x00,0x00,0x00),	// VIV_TK_ACCENT_DOWN (the accent table wins)
	RGB(0xFF,0xFF,0xFF)		// VIV_TK_ON_ACCENT
};

// the light side keeps system colors so a classic light windows theme still
// feels native. the custom entries carry the quiet fixed tones the field
// rounds calibrated (rc.13: hover contrast, nav layering, the unified
// menu-tone strip).
static COLORREF _viv_theme_light_color(int token)
{
	switch (token)
	{
		case VIV_TK_FACE: return GetSysColor(COLOR_BTNFACE);
		// rc.13: a fixed quiet step below the face - the old light value
		// equaled FACE exactly and the settings sidebar had zero
		// layering (the dark side always had its own tone).
		case VIV_TK_NAV_FACE: return RGB(0xE4,0xE4,0xE4);
		// rc.13: the strip ties to the system menu color - the private
		// E0E0E0 band sat between the menu face and the button face as
		// a third gray on non-win11 machines.
		case VIV_TK_CHROME: return GetSysColor(COLOR_MENU);
		case VIV_TK_CHROME_LINE: return GetSysColor(COLOR_3DSHADOW);
		case VIV_TK_CHROME_MUTED: return GetSysColor(COLOR_GRAYTEXT);
		case VIV_TK_FRAME: return GetSysColor(COLOR_MENU);
		case VIV_TK_INPUT: return GetSysColor(COLOR_WINDOW);
		case VIV_TK_LINE: return GetSysColor(COLOR_3DSHADOW);
		// rc.13: a fixed hover with real contrast - 3dlight sat ~3
		// points off the chrome strip and the menu/toolbar hover was
		// invisible in the light ui (the field report's dead look).
		case VIV_TK_HOVER: return RGB(0xDC,0xDC,0xDC);
		case VIV_TK_DOWN: return GetSysColor(COLOR_3DSHADOW);
		case VIV_TK_TEXT: return GetSysColor(COLOR_WINDOWTEXT);
		case VIV_TK_TEXT2: return GetSysColor(COLOR_GRAYTEXT);
		case VIV_TK_TEXTOFF: return GetSysColor(COLOR_GRAYTEXT);
		case VIV_TK_ON_ACCENT: return RGB(0xFF,0xFF,0xFF);
		default: return RGB(0,0,0);
	}
}

// the accent table: base, hot, down per accent. hand tuned triples, no
// runtime color math. index order matches the settings picker.
static const COLORREF _viv_theme_accent_colors[VIV_THEME_ACCENT_COUNT][3] =
{
	{RGB(0x1E,0x66,0xE8),RGB(0x3B,0x7B,0xEE),RGB(0x1A,0x57,0xC8)},	// azure
	{RGB(0x0E,0x98,0x88),RGB(0x2F,0xB3,0xA2),RGB(0x0A,0x7A,0x6E)},	// teal
	{RGB(0x7A,0x5A,0xF0),RGB(0x93,0x78,0xF4),RGB(0x63,0x48,0xCE)},	// violet
	{RGB(0xC7,0x74,0x20),RGB(0xDE,0x8C,0x36),RGB(0xA8,0x5E,0x14)},	// amber
	{RGB(0xD6,0x44,0x5E),RGB(0xE4,0x63,0x7A),RGB(0xB9,0x33,0x50)}	// rose
};

static HBRUSH _viv_theme_brushes[VIV_TK_COUNT];

COLORREF viv_theme_color(int token)
{
	if ((token < 0) || (token >= VIV_TK_COUNT))
	{
		return RGB(0,0,0);
	}

	// the accent tokens always read the accent table so the picker drives
	// both themes.
	switch (token)
	{
		case VIV_TK_ACCENT:
			return _viv_theme_accent_colors[config_ui_accent][0];

		case VIV_TK_ACCENT_HOT:
			return _viv_theme_accent_colors[config_ui_accent][1];

		case VIV_TK_ACCENT_DOWN:
			return _viv_theme_accent_colors[config_ui_accent][2];
	}

	if (_viv_is_dark())
	{
		return _viv_theme_dark_colors[token];
	}

	return _viv_theme_light_color(token);
}

HBRUSH viv_theme_brush(int token)
{
	if ((token < 0) || (token >= VIV_TK_COUNT))
	{
		return 0;
	}

	if (!_viv_theme_brushes[token])
	{
		_viv_theme_brushes[token] = CreateSolidBrush(viv_theme_color(token));
	}

	return _viv_theme_brushes[token];
}

void viv_theme_refresh(void)
{
	int i;

	for (i=0; i<VIV_TK_COUNT; i++)
	{
		if (_viv_theme_brushes[i])
		{
			DeleteObject(_viv_theme_brushes[i]);
			_viv_theme_brushes[i] = 0;
		}
	}
}

int viv_theme_accent(void)
{
	return config_ui_accent;
}

void viv_theme_set_accent(int accent)
{
	if ((accent < 0) || (accent >= VIV_THEME_ACCENT_COUNT))
	{
		accent = 0;
	}

	if (accent == config_ui_accent)
	{
		return;
	}

	config_ui_accent = (BYTE)accent;

	viv_theme_refresh();
}

COLORREF viv_theme_accent_swatch(int index)
{
	if ((index < 0) || (index >= VIV_THEME_ACCENT_COUNT))
	{
		return RGB(0,0,0);
	}

	return _viv_theme_accent_colors[index][0];
}
