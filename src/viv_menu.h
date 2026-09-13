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
// viv_menu.h - command and key tables, menus, menu localization.
#ifndef VIV_MENU_H
#define VIV_MENU_H

#include "viv.h"

// exported to other domains / the viv.c core
void _viv_check_menus(HMENU hmenu);
void _viv_get_command_name(wchar_t *wbuf,int command_index);
int _viv_command_index_from_command_id(int command_id);
void _viv_get_key_text(wchar_t *wbuf,DWORD keyflags);
HMENU _viv_create_menu(void);
void _viv_key_add(_viv_key_list_t *key_list,int command_index,DWORD keyflags);
void _viv_key_list_copy(_viv_key_list_t *dst,const _viv_key_list_t *src);
void _viv_key_clear_all(_viv_key_list_t *list);
void _viv_key_list_init(_viv_key_list_t *list);
int _viv_get_current_key_mod_flags(void);
void _viv_key_remove(_viv_key_list_t *keylist,int command_index,DWORD keyflags);

// owner drawn popup rows (viv_wndproc dispatches the messages here).
// the row types and pools are part of the contract: the builders in the
// menu, recent and context domains allocate rows, the painters here
// resolve them back to live text at draw time.
#define _VIV_MENU_DRAW_COMMAND		0
#define _VIV_MENU_DRAW_POPUP		1
#define _VIV_MENU_DRAW_SEPARATOR	2
#define _VIV_MENU_DRAW_RECENT		3
#define _VIV_MENU_DRAW_LOCALIZED	4
// rc.13: the settings dropdowns join the owner drawn pipeline - their
// labels are ad hoc (not command or localization table entries), so the
// row carries a text pointer the builder sets after the alloc.
#define _VIV_MENU_DRAW_TEXT		5


#define _VIV_MENU_POOL_MAIN		0
#define _VIV_MENU_POOL_RECENT		1
#define _VIV_MENU_POOL_CONTEXT		2
#define _VIV_MENU_POOL_SETTINGS	3
void *_viv_menu_row_alloc(int pool,int type,int command_index,int localization_id,int recent_index);
void _viv_menu_row_pool_reset(int pool);
// attach an ad hoc label to a _VIV_MENU_DRAW_TEXT row (the store behind
// the pointer outlives the popup: the builder resets the pool first).
void _viv_menu_row_set_text(void *row,const wchar_t *text);
int _viv_menu_measure_item(MEASUREITEMSTRUCT *measure_item);
int _viv_menu_draw_item(DRAWITEMSTRUCT *draw_item);
int _viv_menu_char_item(HMENU hmenu,wchar_t ch,int popup);
void _viv_menu_row_item_text(void *row,wchar_t *wbuf);

// rc.13: the popup layer theming (the #32768 class brush + the dwm
// chrome probe) extracted from the main window's wm_initmenupopup so
// every popup owner - the menu bar, the canvas context menu and the
// settings dropdowns - runs the same code.
void _viv_menu_popup_theme(void);

#endif
