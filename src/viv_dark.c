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
// viv_dark.c - dark dialog infrastructure (subclassing, theming, drawing).
// Pure physical move from viv.c (R70 split): bodies are unchanged.
#include "viv.h"
#include "viv_state.h"
#include "viv_dark.h"
#include "viv_chrome.h"
#include "viv_dialogs.h"
#include "viv_render.h"

// forward declarations (order preserved from viv.c)
HBRUSH _viv_dialog_dark_brush(void);
static int _viv_dialog_dark_combo_item_height(HWND hwnd);
static BOOL CALLBACK _viv_dark_dialog_children(HWND hwnd,LPARAM lParam);
void _viv_dark_dialog(HWND hwnd);
static BOOL CALLBACK _viv_dark_dialogs_enum(HWND hwnd,LPARAM lParam);
void _viv_dark_dialogs_refresh(void);
static INT_PTR _viv_dialog_dark_ctlcolor(HDC hdc);
static int _viv_dialog_dark_erase(HWND hwnd,HDC hdc);
static INT_PTR _viv_dialog_dark_draw_item(HWND hwnd,DRAWITEMSTRUCT *draw_item);
static HANDLE _viv_dialog_dark_theme(HWND hwnd);
static void _viv_dialog_dark_theme_drop(HWND hwnd);
static int _viv_dialog_dark_glyph_state(int type,int check,UINT item_state);
static INT_PTR _viv_dialog_dark_notify(HWND hwnd,NMHDR *header);
INT_PTR _viv_dialog_dark_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);


// the dark dialog background brush (lazy created, deleted at kill).
HBRUSH _viv_dialog_dark_brush(void)
{
	if (!_viv_dialog_dark_hbrush)
	{
		_viv_dialog_dark_hbrush = CreateSolidBrush(viv_theme_color(VIV_TK_FACE));
	}
	
	return _viv_dialog_dark_hbrush;
}
// give a dialog the dark chrome: a dark title bar and the dark explorer
// control style. call from WM_INITDIALOG.
// the dark explorer style does not cascade from a dialog to its child
// controls: every button, combobox, listbox and edit must opt in
// individually (allow dark mode, then the dark theme class) or it keeps
// drawing with the light visual style on the dark background. this is
// what made the options pages look half themed (dark background, light
// comboboxes and check glyphs).
// the owner drawn combo item height at the control font.
static int _viv_dialog_dark_combo_item_height(HWND hwnd)
{
	HDC hdc;
	HFONT font;
	HFONT old_font;
	TEXTMETRICW tm;
	int high;
	
	high = (16 * os_logical_high) / 96;
	
	hdc = GetDC(hwnd);
	
	if (hdc)
	{
		font = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
		
		old_font = font ? (HFONT)SelectObject(hdc,font) : 0;
		
		os_zero_memory(&tm,sizeof(tm));
		
		if (GetTextMetricsW(hdc,&tm))
		{
			high = tm.tmHeight + ((6 * os_logical_high) / 96);
		}
		
		if (old_font)
		{
			SelectObject(hdc,old_font);
		}
		
		ReleaseDC(hwnd,hdc);
	}
	
	return high;
}
static BOOL CALLBACK _viv_dark_dialog_children(HWND hwnd,LPARAM lParam)
{
	wchar_t class_name[64];
	LONG_PTR style;
	int dark;
	
	(void)lParam;
	
	dark = _viv_is_dark();
	
	// the immersive flag and the theme class have to follow the app theme
	// together: the dark classes painted on a light dialog leave black
	// fields, black frames and black buttons on the light face (the field
	// report: the light options page showed black comboboxes and black
	// select-all buttons).
	os_allow_dark_mode_for_window(hwnd,dark ? 1 : 0);
	
	class_name[0] = 0;
	
	if (GetClassNameW(hwnd,class_name,64))
	{
		if (string_compare(class_name,L"ComboBox") == 0)
		{
			if (dark)
			{
				// comboboxes have no parts in the darkmode explorer class: the dark
				// dialogs kept light frames and arrows on 1903+ builds. the common
				// dialog class carries the dark combo parts there, and the owner
				// drawn items below carry the field and the list rows everywhere.
				os_dark_combobox_theme(hwnd);
			}
			else
			{
				// back to the standard light class: the control may carry the
				// dark class from an earlier dark state of the same open dialog.
				os_light_window_theme(hwnd);
			}
		}
		else
		if (string_compare(class_name,L"SysTreeView32") == 0)
		{
			// the tree never follows the dark explorer class for its item text:
			// the theme draws the labels in its own gray, which reads as low
			// contrast on the dark face (the field report). pin the face and the
			// text color - the same values the options initdialog applies - and
			// hand them back to the system on the light flip (0xffffffff is
			// clr_default).
			SendMessage(hwnd,TVM_SETBKCOLOR,0,dark ? viv_theme_color(VIV_TK_FACE) : (COLORREF)0xFFFFFFFF);
			SendMessage(hwnd,TVM_SETTEXTCOLOR,0,dark ? viv_theme_color(VIV_TK_TEXT) : (COLORREF)0xFFFFFFFF);
			
			if (dark)
			{
				os_dark_window_theme(hwnd);
			}
			else
			{
				os_light_window_theme(hwnd);
			}
		}
		else
		{
			if (dark)
			{
				os_dark_window_theme(hwnd);
			}
			else
			{
				os_light_window_theme(hwnd);
			}
		}
		
		if (dark)
		{
			if (string_compare(class_name,L"Button") == 0)
			{
				int type;
				
				style = GetWindowLongPtr(hwnd,GWL_STYLE);
				
				type = (int)(style & BS_TYPEMASK);
				
				// push buttons owner draw on every build: the native dark button
				// carries a light bottom edge (the field report called it a chin)
				// and paints a hard rectangle that fights the rest of the dark
				// chrome, while the flat face below matches the combo fields
				// exactly. the glyph controls (checkboxes, radios) never flip: the
				// owner draw bit sits in the bs_typemask field, so the flip
				// replaces bs_autocheckbox itself and the automatic check state
				// machine goes with it - clicks stop toggling, bm_getcheck reads
				// zero forever and isdlgbuttonchecked sees a dead control (the
				// withdrawn 1.1.09 build flipped them on every machine and the
				// field report showed every options checkbox frozen in the dark
				// ui; the same dead flip sat latent on the pre 1903 fallback).
				// their faces paint through the custom draw path instead (see
				// _viv_dialog_dark_notify): the themed button paints its own label
				// color and the wm_ctlcolorstatic text never reaches it - the
				// field report read black checkbox labels on the dark face (0.78:1
				// against the dialog color) - so the custom draw takes the whole
				// face: the dark background, the theme glyph in its checked, mixed,
				// hot and disabled states, the light label and the focus frame,
				// with the skip traveling through dwlp_msgresult the dialog
				// contract requires. the bitmap color swatches keep their own
				// painting in every state.
				if ((!(style & (BS_BITMAP | BS_ICON))) && ((type == BS_PUSHBUTTON) || (type == BS_DEFPUSHBUTTON)))
				{
					SetWindowLongPtr(hwnd,GWL_STYLE,(style & ~((LONG_PTR)BS_TYPEMASK)) | BS_OWNERDRAW);
					
					// the prop carries the original button type (the owner draw bit
					// overwrites the type field, so the way back needs it).
					SetPropW(hwnd,_VIV_DARK_OWNERDRAW_PROP,(HANDLE)(type + 1));
					
					InvalidateRect(hwnd,0,TRUE);
				}
			}
			else
			if (string_compare(class_name,L"ComboBox") == 0)
			{
				// every windows build: even where the dark controls exist the
				// explorer class carries no combo parts, so the field and the
				// list rows always owner draw through the dialog dark handler.
				style = GetWindowLongPtr(hwnd,GWL_STYLE);
				
				if (!(style & CBS_OWNERDRAWFIXED))
				{
					int field_height;
					
					// the closed field height the control was created with: the
					// flip must keep it exactly. an item height rebuilt from the
					// font metrics drifts a few pixels at some dpis - the field
					// then grows or shrinks against its label row and leaves a
					// stray strip under the control (the field report: a chin
					// under the dark combos and sizes that no longer matched
					// the light ui).
					field_height = (int)SendMessage(hwnd,CB_GETITEMHEIGHT,(WPARAM)-1,0);
					
					if (field_height > 0)
					{
						SetWindowLongPtr(hwnd,GWL_STYLE,style | CBS_OWNERDRAWFIXED);
						
						// a runtime flip does not resend the measure item message: set
						// the item height directly (the selected field and the list rows)
						// at the captured native height.
						SendMessage(hwnd,CB_SETITEMHEIGHT,(WPARAM)-1,field_height);
						SendMessage(hwnd,CB_SETITEMHEIGHT,(WPARAM)0,field_height);
						
						// the prop marks the flip and carries the captured height (the
						// 0x100 base keeps it apart from the button type codes) for the
						// measure item fallback.
						SetPropW(hwnd,_VIV_DARK_OWNERDRAW_PROP,(HANDLE)(0x100 + field_height));
						
						InvalidateRect(hwnd,0,TRUE);
					}
				}
			}
		}
		else
		{
			// the ui went back to light: restore the system painting on the
			// controls this module flipped.
			if (GetPropW(hwnd,_VIV_DARK_OWNERDRAW_PROP))
			{
				style = GetWindowLongPtr(hwnd,GWL_STYLE);
				
				if ((string_compare(class_name,L"Button") == 0) && (((int)(LONG_PTR)GetPropW(hwnd,_VIV_DARK_OWNERDRAW_PROP)) < 0x100))
				{
					LONG_PTR type;
					
					type = (LONG_PTR)GetPropW(hwnd,_VIV_DARK_OWNERDRAW_PROP) - 1;
					
					// restore the original button type (the owner draw bit sat in
					// the type field).
					SetWindowLongPtr(hwnd,GWL_STYLE,(style & ~((LONG_PTR)BS_TYPEMASK)) | type);
				}
				else
				if ((string_compare(class_name,L"ComboBox") == 0) && (((int)(LONG_PTR)GetPropW(hwnd,_VIV_DARK_OWNERDRAW_PROP)) >= 0x100))
				{
					SetWindowLongPtr(hwnd,GWL_STYLE,style & ~((LONG_PTR)CBS_OWNERDRAWFIXED));
				}
				
				RemovePropW(hwnd,_VIV_DARK_OWNERDRAW_PROP);
				
				InvalidateRect(hwnd,0,TRUE);
			}
		}
	}
	
	return TRUE;
}
// give a dialog the dark chrome: a dark title bar, the dark explorer
// control style and the same style on every child control. call from
// WM_INITDIALOG (the dialog manager creates the children before it).
void _viv_dark_dialog(HWND hwnd)
{
	// the win11 frame treatment for every dialog: rounded corners and a
	// caption color that follows the chrome (a silent no-op earlier).
	os_window_modern_chrome(hwnd,viv_theme_color(VIV_TK_CHROME));
	
	if (_viv_is_dark())
	{
		os_dark_titlebar(hwnd,1);
		
		os_dark_window_theme(hwnd);
		
		EnumChildWindows(hwnd,_viv_dark_dialog_children,0);
	}
	else
	{
		// the light ui hands the flipped controls back to the system and
		// returns the dialog itself to the light chrome: the title bar and
		// the control classes may carry the dark state from an earlier flip
		// of the same open dialog.
		os_dark_titlebar(hwnd,0);
		
		os_light_window_theme(hwnd);
		
		EnumChildWindows(hwnd,_viv_dark_dialog_children,0);
	}
}
static BOOL CALLBACK _viv_dark_dialogs_enum(HWND hwnd,LPARAM lParam)
{
	wchar_t class_name[16];
	
	(void)lParam;
	
	// the thread enumerator guarantees these windows belong to us: only
	// the dialog windows need the refresh.
	if ((GetClassNameW(hwnd,class_name,16)) && (string_compare(class_name,L"#32770") == 0))
	{
		_viv_dark_dialog(hwnd);
		
		InvalidateRect(hwnd,0,TRUE);
	}
	
	return TRUE;
}
void _viv_dark_dialogs_refresh(void)
{
	EnumThreadWindows(GetCurrentThreadId(),_viv_dark_dialogs_enum,0);
}
// dark color reply for the dialog control color messages (statics, edits
// and lists). returns the brush, or 0 to keep the default light painting.
static INT_PTR _viv_dialog_dark_ctlcolor(HDC hdc)
{
	if (_viv_is_dark())
	{
		SetTextColor(hdc,viv_theme_color(VIV_TK_TEXT));
		SetBkColor(hdc,viv_theme_color(VIV_TK_FACE));
		
		return (INT_PTR)_viv_dialog_dark_brush();
	}
	
	return 0;
}
// erase a dialog background with the dark palette. call from
// WM_ERASEBKGND; returns 1 when the background was painted.
static int _viv_dialog_dark_erase(HWND hwnd,HDC hdc)
{
	if (_viv_is_dark())
	{
		RECT rect;
		
		GetClientRect(hwnd,&rect);
		
		FillRect(hdc,&rect,_viv_dialog_dark_brush());
		
		return 1;
	}
	
	return 0;
}
// shared dark handling for the dialog messages: WM_CTLCOLORSTATIC,
// WM_CTLCOLOREDIT, WM_CTLCOLORLISTBOX and WM_ERASEBKGND. returns the
// dialog proc reply, or -1 when the caller should run its own switch.
// shared dark handling for the dialog messages: WM_CTLCOLORSTATIC,
// WM_CTLCOLOREDIT, WM_CTLCOLORLISTBOX and WM_ERASEBKGND. returns the
// dialog proc reply, or -1 when the caller should run its own switch.
// owner drawn dialog controls: the push buttons and the combo boxes
// paint with the dark palette while the dark ui is active. the glyph
// controls (checkboxes, radios) never flip to owner draw - an owner
// draw flip replaces their automatic check state machine - so their
// faces paint through the custom draw path instead
// (_viv_dialog_dark_notify). the light ui never flips the owner draw
// styles (and unflips them on the way back), so this only runs in the
// dark.
static INT_PTR _viv_dialog_dark_draw_item(HWND hwnd,DRAWITEMSTRUCT *draw_item)
{
	wchar_t text[STRING_SIZE];
	
	if (!_viv_is_dark())
	{
		return 0;
	}
	
	(void)hwnd;
	
	switch(draw_item->CtlType)
	{
		case ODT_BUTTON:
		{
			UINT style;
			RECT rect;
			HBRUSH face_brush;
			HFONT font;
			HFONT old_font;
			COLORREF text_color;
			int pressed;
			
			// the original button type rides in the flip property (the owner
			// draw bit occupies the type field while it is set).
			style = (UINT)((LONG_PTR)GetPropW(draw_item->hwndItem,_VIV_DARK_OWNERDRAW_PROP) - 1);
			
			// only the push buttons were flipped (the glyph controls paint
			// their labels through the custom draw path - an owner draw flip
			// would replace their automatic check state machine).
			if ((style != BS_PUSHBUTTON) && (style != BS_DEFPUSHBUTTON))
			{
				return 0;
			}
			
			// the label text uses the control font. the draw item dc carries no
			// defined font, so the owner draw picks it explicitly: the drawn
			// text then matches the native rendering face for face (the field
			// report read the fonts different between the drawn and the native
			// controls).
			font = (HFONT)SendMessage(draw_item->hwndItem,WM_GETFONT,0,0);
			
			old_font = font ? (HFONT)SelectObject(draw_item->hDC,font) : 0;
			
			text[0] = 0;
			GetWindowTextW(draw_item->hwndItem,text,STRING_SIZE);
			
			CopyRect(&rect,&draw_item->rcItem);
			
			pressed = (draw_item->itemState & ODS_SELECTED) ? 1 : 0;
			
			// push buttons: the lifted face with a light frame.
			face_brush = _viv_dark_chrome_brush(1);
			
			FillRect(draw_item->hDC,&rect,face_brush);
			FrameRect(draw_item->hDC,&rect,_viv_dark_chrome_brush(2));
			
			if (style == BS_DEFPUSHBUTTON)
			{
				RECT outer;
				
				CopyRect(&outer,&draw_item->rcItem);
				
				InflateRect(&outer,-2,-2);
				
				FrameRect(draw_item->hDC,&outer,_viv_dark_chrome_brush(2));
			}
			
			if (pressed)
			{
				OffsetRect(&rect,1,1);
			}
			
			text_color = (draw_item->itemState & ODS_DISABLED) ? viv_theme_color(VIV_TK_TEXTOFF) : viv_theme_color(VIV_TK_TEXT);
			
			SetBkMode(draw_item->hDC,TRANSPARENT);
			SetTextColor(draw_item->hDC,text_color);
			
			DrawTextW(draw_item->hDC,text,-1,&rect,DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			
			if (old_font)
			{
				SelectObject(draw_item->hDC,old_font);
			}
			
			return TRUE;
		}
		
		case ODT_COMBOBOX:
		{
			RECT rect;
			HBRUSH face_brush;
			COLORREF text_color;
			HFONT font;
			HFONT old_font;
			int selected;
			
			text[0] = 0;
			
			// the row text uses the control font: the draw item dc carries no
			// defined font, so the owner draw picks it explicitly (the same
			// rule as the buttons - the drawn face matches the native one).
			font = (HFONT)SendMessage(draw_item->hwndItem,WM_GETFONT,0,0);
			
			old_font = font ? (HFONT)SelectObject(draw_item->hDC,font) : 0;
			
			if (draw_item->itemID == (UINT)-1)
			{
				// the closed field: the current selection.
				int cur;
				
				cur = (int)SendMessage(draw_item->hwndItem,CB_GETCURSEL,0,0);
				
				if (cur != -1)
				{
					SendMessageW(draw_item->hwndItem,CB_GETLBTEXT,cur,(LPARAM)text);
				}
			}
			else
			{
				SendMessageW(draw_item->hwndItem,CB_GETLBTEXT,draw_item->itemID,(LPARAM)text);
			}
			
			CopyRect(&rect,&draw_item->rcItem);
			
			// the closed field and the highlighted list rows take the hover
			// tone, the plain rows take the dialog face.
			selected = (draw_item->itemState & (ODS_SELECTED | ODS_COMBOBOXEDIT)) ? 1 : 0;
			
			face_brush = selected ? _viv_dark_chrome_brush(1) : _viv_dark_chrome_brush(3);
			text_color = (draw_item->itemState & ODS_DISABLED) ? viv_theme_color(VIV_TK_TEXTOFF) : viv_theme_color(VIV_TK_TEXT);
			
			FillRect(draw_item->hDC,&rect,face_brush);
			
			SetBkMode(draw_item->hDC,TRANSPARENT);
			SetTextColor(draw_item->hDC,text_color);
			
			DrawTextW(draw_item->hDC,text,-1,&rect,DT_SINGLELINE | DT_LEFT | DT_VCENTER);
			
			if ((draw_item->itemState & ODS_FOCUS) && (!(draw_item->itemState & ODS_COMBOBOXEDIT)))
			{
				DrawFocusRect(draw_item->hDC,&rect);
			}
			
			if (old_font)
			{
				SelectObject(draw_item->hDC,old_font);
			}
			
			return TRUE;
		}
	}
	
	return 0;
}
static HANDLE _viv_dialog_dark_theme(HWND hwnd)
{
	HANDLE theme;
	
	theme = (HANDLE)GetPropW(hwnd,_VIV_DARK_BUTTON_THEME_PROP);
	
	if (!theme)
	{
		theme = os_theme_open_button(hwnd);
		
		if (theme)
		{
			SetPropW(hwnd,_VIV_DARK_BUTTON_THEME_PROP,theme);
		}
	}
	
	return theme;
}
static void _viv_dialog_dark_theme_drop(HWND hwnd)
{
	HANDLE theme;
	
	theme = (HANDLE)GetPropW(hwnd,_VIV_DARK_BUTTON_THEME_PROP);
	
	if (theme)
	{
		os_theme_close(theme);
		
		RemovePropW(hwnd,_VIV_DARK_BUTTON_THEME_PROP);
	}
}
// the theme state for the drawn glyph: the check state rides with the
// control message (bm_getcheck), the disabled, hot and pressed states
// ride with the custom draw item state - the map lands on the exact
// cbs_* / rbs_* state id the native button would have picked for the
// same control. radio has no mixed state; disabled wins over hot and
// pressed the way the visual styles table orders them.
static int _viv_dialog_dark_glyph_state(int type,int check,UINT item_state)
{
	int state;
	
	if ((type == BS_AUTORADIOBUTTON) || (type == BS_RADIOBUTTON))
	{
		state = check ? OS_RBS_CHECKEDNORMAL : OS_RBS_UNCHECKEDNORMAL;
	}
	else
	{
		state = (check == BST_INDETERMINATE) ? OS_BS_MIXEDNORMAL : (check ? OS_BS_CHECKEDNORMAL : OS_BS_UNCHECKEDNORMAL);
	}
	
	if (item_state & CDIS_DISABLED)
	{
		state += 3;
	}
	else if (item_state & CDIS_HOT)
	{
		state += 1;
	}
	else if (item_state & CDIS_SELECTED)
	{
		state += 2;
	}
	
	return state;
}
// the glyph controls in the dark ui: the themed button paints its own
// label color (the wm_ctlcolorstatic text never reaches it - the field
// report read black checkbox labels at 0.78:1 against the dialog face),
// so the checkboxes and the radios custom draw the whole face here: the
// dark dialog background, the theme glyph in the control's own checked,
// mixed, disabled and hot state, the light label and the focus frame.
// the button styles never change: the automatic check state machine, the
// radio grouping and every isdlgbuttonchecked read keep working in both
// themes (the withdrawn 1.1.09 build flipped the glyph controls to owner
// draw instead, which replaced bs_autocheckbox in the style and left
// every options checkbox dead in the dark ui).
//
// the return value has to travel through dwlp_msgresult: a dialog
// procedure cannot return a notify result directly (the dialog manager
// keeps the message result in the window data, so a plain non-zero
// return hands the control a zero - cdrf_dodefault - and the first
// 1.1.09 redo shipped exactly that: the skip never reached the buttons,
// the system painted its full default face on top of the custom draw,
// the labels read black on black and the two xor focus frames cancelled
// each other). this function returns the cdrf code and the caller
// (_viv_dialog_dark_proc) does the setwindowlongptr + return true pair
// the nm_customdraw documentation prescribes for dialog procedures.
static INT_PTR _viv_dialog_dark_notify(HWND hwnd,NMHDR *header)
{
	wchar_t class_name[64];
	wchar_t text[STRING_SIZE];
	LONG_PTR style;
	int type;
	
	// only the dark ui repaints the faces; the light ui keeps the native
	// painting untouched.
	if (!_viv_is_dark())
	{
		return -1;
	}
	
	// only the button custom draw belongs here: the tree view, the tabs
	// and every other common control keep their own notifications flowing
	// to the dialog procedures (the options tree switches its pages on
	// those).
	if (header->code != NM_CUSTOMDRAW)
	{
		return -1;
	}
	
	class_name[0] = 0;
	
	if ((!header->hwndFrom) || (!GetClassNameW(header->hwndFrom,class_name,64)) || (string_compare(class_name,L"Button") != 0))
	{
		return -1;
	}
	
	style = GetWindowLongPtr(header->hwndFrom,GWL_STYLE);
	
	type = (int)(style & BS_TYPEMASK);
	
	// the glyph controls only: the push buttons paint through the owner
	// draw flip, the bitmap swatches keep their own painting and the group
	// boxes carry no label problem.
	if ((type != BS_AUTOCHECKBOX) && (type != BS_AUTORADIOBUTTON) && (type != BS_CHECKBOX) && (type != BS_RADIOBUTTON) && (type != BS_3STATE) && (type != BS_AUTO3STATE))
	{
		return -1;
	}
	
	if (((NMCUSTOMDRAW *)header)->dwDrawStage == CDDS_PREPAINT)
	{
		NMCUSTOMDRAW *custom_draw;
		HANDLE theme;
		RECT rect;
		RECT glyph_rect;
		HFONT font;
		HFONT old_font;
		SIZE digit;
		int glyph_wide;
		int glyph_high;
		int check;
		int part;
		int state;
		
		custom_draw = (NMCUSTOMDRAW *)header;
		
		// the button theme opens once per dialog and stays cached in the
		// window data (see _viv_dialog_dark_theme).
		theme = _viv_dialog_dark_theme(hwnd);
		
		part = ((type == BS_AUTORADIOBUTTON) || (type == BS_RADIOBUTTON)) ? OS_BP_RADIOBUTTON : OS_BP_CHECKBOX;
		
		check = (int)SendMessage(header->hwndFrom,BM_GETCHECK,0,0);
		
		state = _viv_dialog_dark_glyph_state(type,check,custom_draw->uItemState);
		
		glyph_wide = 0;
		
		glyph_high = 0;
		
		if ((!os_theme_part_size(theme,custom_draw->hdc,part,state,&glyph_wide,&glyph_high)) || (glyph_wide <= 0) || (glyph_high <= 0))
		{
			// no usable theme part (the visual styles off): the native
			// painting stays - the glyph controls read the system way there,
			// exactly as the light ui does.
			return CDRF_DODEFAULT;
		}
		
		// the label text uses the control font: the drawn face then matches
		// the native rendering (the light and the dark ui read the same).
		font = (HFONT)SendMessage(header->hwndFrom,WM_GETFONT,0,0);
		
		old_font = font ? (HFONT)SelectObject(custom_draw->hdc,font) : 0;
		
		GetTextExtentPoint32W(custom_draw->hdc,L"0",1,&digit);
		
		// the skip takes the whole painting, the item background included,
		// so the rect takes the dark dialog face first (the parent
		// background the native path would have asked for).
		FillRect(custom_draw->hdc,&custom_draw->rc,_viv_dialog_dark_brush());
		
		// the glyph: the theme part at the left edge, vertically centered
		// on the control the way the native layout centers the text.
		CopyRect(&glyph_rect,&custom_draw->rc);
		
		glyph_rect.right = glyph_rect.left + glyph_wide;
		
		glyph_rect.top = glyph_rect.top + ((glyph_rect.bottom - glyph_rect.top - glyph_high) / 2);
		
		glyph_rect.bottom = glyph_rect.top + glyph_high;
		
		os_theme_draw_part(theme,custom_draw->hdc,part,state,&glyph_rect);
		
		// the label sits right of the glyph: the glyph box plus half a digit
		// of gap, vertically centered on the control (the native layout).
		CopyRect(&rect,&custom_draw->rc);
		
		OffsetRect(&rect,glyph_wide + (digit.cx / 2),0);
		
		text[0] = 0;
		
		GetWindowTextW(header->hwndFrom,text,STRING_SIZE);
		
		SetBkMode(custom_draw->hdc,TRANSPARENT);
		
		SetTextColor(custom_draw->hdc,(style & WS_DISABLED) ? viv_theme_color(VIV_TK_TEXTOFF) : viv_theme_color(VIV_TK_TEXT));
		
		DrawTextW(custom_draw->hdc,text,-1,&rect,DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
		
		// the focus frame, drawn exactly once: the skip keeps the native
		// painting away, so the frame cannot xor against a second frame the
		// way the broken return shipped it (two draws cancelled each other
		// and the keyboard walk lost its dot in the dark).
		if (GetFocus() == header->hwndFrom)
		{
			RECT focus_rect;
			
			CopyRect(&focus_rect,&custom_draw->rc);
			
			InflateRect(&focus_rect,-1,-1);
			
			DrawFocusRect(custom_draw->hdc,&focus_rect);
		}
		
		if (old_font)
		{
			SelectObject(custom_draw->hdc,old_font);
		}
		
		// the skip means the whole face is ours now - the background, the
		// glyph, the label and the focus frame above.
		return CDRF_SKIPDEFAULT;
	}
	
	return CDRF_DODEFAULT;
}
INT_PTR _viv_dialog_dark_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			// one type system: the message font at this dialog window own
			// dpi replaces the template face on the dialog and every child
			// control. the break keeps the -1 return so each dialog's own
			// initialization still runs: the dark flip measures the final
			// font and the localized labels draw with it.
			_viv_dialog_apply_font(hwnd);
			
			break;
		}
		
		case WM_DPICHANGED:
		{
			// a dialog dragged across monitors re-reads the font at the new
			// dpi (the layout grid stays; the type follows).
			_viv_dialog_apply_font(hwnd);
			
			break;
		}
		
		case WM_NCDESTROY:
		{
			// the dialog dies after its children: the font it adopted
			// dies with it, and no live window can still hold the handle
			// (the children are gone by the time ncdestroy runs - the
			// release is safe by construction).
			HFONT font;
			
			font = (HFONT)GetPropW(hwnd,_VIV_DIALOG_FONT_PROP);
			
			if (font)
			{
				RemovePropW(hwnd,_VIV_DIALOG_FONT_PROP);
				
				DeleteObject(font);
			}
			
			break;
		}
		
		case WM_DRAWITEM:
		{
			INT_PTR dark_reply;
			
			// the owner drawn dark controls: the push buttons and the combo
			// boxes paint here (the glyph control faces paint through the
			// custom draw case above).
			dark_reply = _viv_dialog_dark_draw_item(hwnd,(DRAWITEMSTRUCT *)lParam);
			
			if (dark_reply)
			{
				return dark_reply;
			}
			
			break;
		}
		
		case WM_MEASUREITEM:
		{
			if ((((MEASUREITEMSTRUCT *)lParam)->CtlType == ODT_COMBOBOX) && (_viv_is_dark()))
			{
				HWND combo_hwnd;
				int captured_height;
				
				combo_hwnd = GetDlgItem(hwnd,(int)wParam);
				captured_height = combo_hwnd ? (int)(LONG_PTR)GetPropW(combo_hwnd,_VIV_DARK_OWNERDRAW_PROP) : 0;
				
				if (captured_height > 0x100)
				{
					// the captured native height keeps the owner drawn rows at the
					// exact height the themed control used (no size drift).
					((MEASUREITEMSTRUCT *)lParam)->itemHeight = captured_height - 0x100;
				}
				else
				{
					((MEASUREITEMSTRUCT *)lParam)->itemHeight = _viv_dialog_dark_combo_item_height(combo_hwnd);
				}
				
				return TRUE;
			}
			
			break;
		}
		case WM_NOTIFY:
		{
			INT_PTR dark_reply;
			
			// the glyph controls paint through the custom draw path (the
			// filter inside passes every other notification through to the
			// dialog's own switch).
			dark_reply = _viv_dialog_dark_notify(hwnd,(NMHDR *)lParam);
			
			if (dark_reply != -1)
			{
				// a dialog procedure cannot return a notify result directly:
				// the dialog manager keeps the message result in the window
				// data, so a plain non-zero return hands the control a zero
				// (= cdrf_dodefault) - the skip never reached the buttons in
				// the first 1.1.09 redo and the system painted its full
				// default face on top of the custom draw. the result travels
				// through dwlp_msgresult with a true return, the way the
				// nm_customdraw documentation prescribes for dialog
				// procedures.
				SetWindowLongPtr(hwnd,DWLP_MSGRESULT,dark_reply);
				
				return TRUE;
			}
			
			break;
		}
		
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		{
			INT_PTR dark_reply;
			
			dark_reply = _viv_dialog_dark_ctlcolor((HDC)wParam);
			
			if (dark_reply)
			{
				return dark_reply;
			}
			
			break;
		}
		
		case WM_DESTROY:
		{
			// the cached button theme closes with the dialog that opened it
			// (the next dialog opens a fresh handle).
			_viv_dialog_dark_theme_drop(hwnd);
			
			break;
		}
		
		case WM_THEMECHANGED:
		{
			// the uxtheme handles die with the visual style change: the
			// cache drops here and the next custom draw reopens it against
			// the new style (a stale handle draws garbage or nothing).
			_viv_dialog_dark_theme_drop(hwnd);
			
			break;
		}
		
		case WM_ERASEBKGND:
		
			if (_viv_dialog_dark_erase(hwnd,(HDC)wParam))
			{
				return 1;
			}
			
			break;
	}
	
	(void)lParam;
	
	return -1;
}
