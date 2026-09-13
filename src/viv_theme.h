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

// viv_theme - the single source of the remake ui palette.
//
// every themed surface (chrome strips, menus, dialogs, the settings window,
// the zoom pill, tooltips, message boxes) pulls its colors from here through
// semantic tokens, so a theme or accent flip re-skins the whole app without
// per surface edits and without any hardcoded rgb left in the painters.
//
// dark mode draws the design tokens; light mode keeps the remake discipline
// and resolves system colors at call time so a classic light windows theme
// still feels native.

#ifndef VIV_THEME_H
#define VIV_THEME_H

// how many accents the settings picker shows.
#define VIV_THEME_ACCENT_COUNT	5

// semantic color tokens. resolve them through viv_theme_color / viv_theme_brush.
enum
{
	VIV_TK_FACE,			// dialogs, popup menus, the settings face, the pill tray
	VIV_TK_NAV_FACE,		// the settings sidebar (inset face)
	VIV_TK_CHROME,			// the menu bar / status bar / toolbar strip background
	VIV_TK_CHROME_LINE,		// hairlines drawn on the chrome strips
	VIV_TK_CHROME_MUTED,	// dim text on the chrome strips (grip, dead panes)
	VIV_TK_FRAME,			// the thin frame fill around the windowed canvas
	VIV_TK_INPUT,			// edit and combo field backgrounds
	VIV_TK_LINE,			// hairline borders on elevated surfaces
	VIV_TK_HOVER,			// hover fill for menu items, list rows, buttons
	VIV_TK_DOWN,			// pressed fill for menu items, list rows, buttons
	VIV_TK_TEXT,			// primary text
	VIV_TK_TEXT2,			// secondary text (captions, shortcuts, units)
	VIV_TK_TEXTOFF,			// disabled text
	VIV_TK_ACCENT,			// the accent fill (toggles, checks, primary buttons)
	VIV_TK_ACCENT_HOT,		// the accent fill hovered
	VIV_TK_ACCENT_DOWN,		// the accent fill pressed
	VIV_TK_ON_ACCENT,		// text and icons drawn on top of the accent
	VIV_TK_COUNT
};

// resolve a token for the active theme. dark mode reads the design tokens,
// light mode maps to system colors, accent tokens always read the accent table.
COLORREF viv_theme_color(int token);

// a cached solid brush for a token. the cache rebuilds on viv_theme_refresh.
HBRUSH viv_theme_brush(int token);

// drop the brush cache. call on a theme flip, an accent flip or when the
// system colors change (wm_themechanged / wm_settingchange paths).
void viv_theme_refresh(void);

// the active accent index (0..VIV_THEME_ACCENT_COUNT-1). backed by config_ui_accent.
int viv_theme_accent(void);

// clamp and store the accent index, then flush the brush cache.
void viv_theme_set_accent(int accent);

// the raw accent color at index for the settings picker swatches.
COLORREF viv_theme_accent_swatch(int index);

#endif // VIV_THEME_H
