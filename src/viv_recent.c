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
// viv_recent.c - recent-files mru (menu, persistence).
// Pure physical move from viv.c (R70 split): bodies are unchanged.
#include "viv.h"
#include "viv_state.h"
#include "viv_recent.h"
#include "viv_menu.h"
#include "viv_playlist.h"

// forward declarations (order preserved from viv.c)
void _viv_recent_save_defer(void);
void _viv_recent_save_fold(void);
void _viv_recent_file_push(const wchar_t *filename);
void _viv_recent_file_remove(int index);
void _viv_recent_file_clear(void);
HMENU _viv_create_recent_menu(void);
static void _viv_recent_menu_update(void);


void _viv_recent_save_defer(void)
{
	_viv_recent_save_dirty = 1;
	
	// settimer on an already-live id resets the countdown: a rapid burst
	// of opens keeps pushing the write back until the burst is over.
	if (_viv_hwnd)
	{
		SetTimer(_viv_hwnd,VIV_ID_RECENT_SAVE_TIMER,_VIV_RECENT_SAVE_DELAY,0);
	}
}
// the pending write folds into an unconditional save that is about to run
// (exit, session end): the timer is dead by then and the flag must not
// leak a stray write later.
void _viv_recent_save_fold(void)
{
	_viv_recent_save_dirty = 0;
	
	if (_viv_hwnd)
	{
		KillTimer(_viv_hwnd,VIV_ID_RECENT_SAVE_TIMER);
	}
}
void _viv_recent_file_push(const wchar_t *filename)
{
	int i;
	
	for(i=0;i<config_recent_file_count;i++)
	{
		if (_viv_icompare_filename(config_recent_files[i],filename) == 0)
		{
			// already in the list: move it to the top.
			if (i != 0)
			{
				wchar_t *entry;
				
				entry = config_recent_files[i];
				
				os_move_memory(&config_recent_files[1],&config_recent_files[0],i * sizeof(wchar_t *));
				
				config_recent_files[0] = entry;
				
				_viv_recent_save_defer();
				_viv_recent_menu_update();
			}
			
			return;
		}
	}
	
	// a new entry: drop the oldest when the list is full. the while (not a
	// single if) also repairs an impossible over-count state: the menu id
	// block, the save loop and this array all assume count <= the cap.
	while(config_recent_file_count >= CONFIG_RECENT_FILE_COUNT)
	{
		mem_free(config_recent_files[config_recent_file_count - 1]);
		config_recent_files[config_recent_file_count - 1] = 0;
		config_recent_file_count--;
	}
	
	os_move_memory(&config_recent_files[1],&config_recent_files[0],config_recent_file_count * sizeof(wchar_t *));
	
	config_recent_files[0] = string_alloc(filename);
	config_recent_file_count++;
	
	_viv_recent_save_defer();
	_viv_recent_menu_update();
}
void _viv_recent_file_remove(int index)
{
	if ((index >= 0) && (index < config_recent_file_count))
	{
		mem_free(config_recent_files[index]);
		
		os_move_memory(&config_recent_files[index],&config_recent_files[index + 1],(config_recent_file_count - index - 1) * sizeof(wchar_t *));
		
		config_recent_file_count--;
		config_recent_files[config_recent_file_count] = 0;
		
		_viv_recent_save_defer();
		_viv_recent_menu_update();
	}
}
void _viv_recent_file_clear(void)
{
	while(config_recent_file_count)
	{
		config_recent_file_count--;
		
		mem_free(config_recent_files[config_recent_file_count]);
		config_recent_files[config_recent_file_count] = 0;
	}
	
	_viv_recent_save_defer();
	_viv_recent_menu_update();
}
// build the recent-files mru popup from the in-memory list. shared by the
// full menu creation and the live swap after an mru change.
HMENU _viv_create_recent_menu(void)
{
	HMENU recent_menu;
	wchar_t text_wbuf[STRING_SIZE];
	
	recent_menu = CreatePopupMenu();
	
	if (recent_menu)
	{
		// the count is bounded at load and at push; the clamp here is the
		// last line of defense so the loop can never emit command ids past
		// the VIV_ID_FILE_RECENT_0 + count-1 block (the compile-time check
		// near the top keeps that block in lockstep with the array).
		int i;
		int count;
		
		count = (config_recent_file_count < CONFIG_RECENT_FILE_COUNT) ? config_recent_file_count : CONFIG_RECENT_FILE_COUNT;
		
		_viv_menu_row_pool_reset(_VIV_MENU_POOL_RECENT);
		
		if (count > 0)
		{
			for(i=0;i<count;i++)
			{
				void *row;
				
				row = _viv_menu_row_alloc(_VIV_MENU_POOL_RECENT,_VIV_MENU_DRAW_RECENT,i,0,0);
				
				if (row)
				{
					AppendMenu(recent_menu,MF_STRING | MF_OWNERDRAW,VIV_ID_FILE_RECENT_0 + i,(LPCWSTR)row);
				}
				else
				{
					wchar_t num_wbuf[64];
					
					// the mru convention: an ampersand digit prefix selects the
					// entry from the keyboard while the submenu is open.
					string_format_number(num_wbuf,i + 1);
					string_copy(text_wbuf,L"&");
					string_cat(text_wbuf,num_wbuf);
					string_cat(text_wbuf,L" ");
					string_cat(text_wbuf,string_get_filename_part(config_recent_files[i]));
					
					AppendMenu(recent_menu,MF_STRING,VIV_ID_FILE_RECENT_0 + i,text_wbuf);
				}
			}
			
			{
				void *row;
				
				row = _viv_menu_row_alloc(_VIV_MENU_POOL_RECENT,_VIV_MENU_DRAW_SEPARATOR,0,0,0);
				
				if (row)
				{
					AppendMenu(recent_menu,MF_SEPARATOR | MF_OWNERDRAW,0,(LPCWSTR)row);
				}
				else
				{
					AppendMenu(recent_menu,MF_SEPARATOR,0,L"");
				}
			}
			
			{
				void *row;
				
				row = _viv_menu_row_alloc(_VIV_MENU_POOL_RECENT,_VIV_MENU_DRAW_LOCALIZED,0,LOCALIZATION_ID_RECENT_FILES_CLEAR,0);
				
				if (row)
				{
					AppendMenu(recent_menu,MF_STRING | MF_OWNERDRAW,VIV_ID_FILE_RECENT_CLEAR,(LPCWSTR)row);
				}
				else
				{
					string_copy_utf8_string(text_wbuf,localization_get_string(LOCALIZATION_ID_RECENT_FILES_CLEAR));
					AppendMenu(recent_menu,MF_STRING,VIV_ID_FILE_RECENT_CLEAR,text_wbuf);
				}
			}
		}
		else
		{
			void *row;
			
			row = _viv_menu_row_alloc(_VIV_MENU_POOL_RECENT,_VIV_MENU_DRAW_LOCALIZED,0,LOCALIZATION_ID_RECENT_FILES_EMPTY,0);
			
			if (row)
			{
				AppendMenu(recent_menu,MF_STRING | MF_GRAYED | MF_OWNERDRAW,VIV_ID_FILE_RECENT_CLEAR,(LPCWSTR)row);
			}
			else
			{
				string_copy_utf8_string(text_wbuf,localization_get_string(LOCALIZATION_ID_RECENT_FILES_EMPTY));
				AppendMenu(recent_menu,MF_STRING | MF_GRAYED,VIV_ID_FILE_RECENT_CLEAR,text_wbuf);
			}
		}
	}
	
	return recent_menu;
}
// run a deferred mru swap (an open popup blocked it). called from the
// popup close sites - both the menubar loop and the context menu.
static BYTE _viv_recent_menu_pending = 0; // an open popup deferred the swap.

void _viv_recent_menu_flush(void)
{
	if (_viv_recent_menu_pending)
	{
		_viv_recent_menu_update();
	}
}

// the mru rows changed (open, remove, clear): swap just the recent popup
// inside the live file menu. the full rebuild (destroy + create + setmenu
// + dark re-apply) repaints the whole non-client area and was the second
// half of the open-image stutter; the bar items and every other row stay
// untouched here, so the owner draw state and the dark bar theme survive
// without re-applying.
static void _viv_recent_menu_update(void)
{
	HMENU file_menu;
	MENUITEMINFOW mii;
	int index;
	int count;
	
	if (!_viv_hmenu)
	{
		return;
	}
	
	// a popup may be displaying the old rows right now (the slideshow
	// timer rebuilds the mru while the file menu is open): swapping or
	// destroying the submenu under a live menu is undefined. defer to
	// the popup close - _viv_recent_menu_flush runs there.
	if (_viv_in_popup_menu)
	{
		_viv_recent_menu_pending = 1;
		
		return;
	}
	
	_viv_recent_menu_pending = 0;
	
	// find the live file menu: the only popup carrying the recent row id.
	file_menu = 0;
	
	count = GetMenuItemCount(_viv_hmenu);
	
	for(index=0;index<count;index++)
	{
		HMENU sub_menu;
		
		sub_menu = GetSubMenu(_viv_hmenu,index);
		
		if (sub_menu)
		{
			if (GetMenuState(sub_menu,_VIV_MENU_FILE_RECENT,MF_BYCOMMAND) != (UINT)-1)
			{
				file_menu = sub_menu;
				
				break;
			}
		}
	}
	
	if (!file_menu)
	{
		return;
	}
	
	// read the old popup out first so it can be destroyed after the swap.
	os_zero_memory(&mii,sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_SUBMENU;
	
	if (!GetMenuItemInfoW(file_menu,_VIV_MENU_FILE_RECENT,FALSE,&mii))
	{
		return;
	}
	
	{
		HMENU old_recent_menu;
		
		old_recent_menu = mii.hSubMenu;
		
		mii.hSubMenu = _viv_create_recent_menu();
		
		if (mii.hSubMenu)
		{
			if (SetMenuItemInfoW(file_menu,_VIV_MENU_FILE_RECENT,FALSE,&mii))
			{
				if (old_recent_menu)
				{
					DestroyMenu(old_recent_menu);
				}
			}
			else
			{
				DestroyMenu(mii.hSubMenu);
			}
		}
	}
}
