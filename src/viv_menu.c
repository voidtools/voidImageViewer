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
// viv_menu.c - command and key tables, menus, menu localization.
// Pure physical move from viv.c (R70 split): bodies are unchanged.
#include "viv.h"
#include "viv_state.h"
#include "viv_menu.h"
#include "viv_chrome.h"
#include "viv_recent.h"
#include "viv_render.h"

// forward declarations (order preserved from viv.c)
void _viv_check_menus(HMENU hmenu);
void _viv_get_command_name(wchar_t *wbuf,int command_index);
static void _viv_cat_command_menu_path(wchar_t *wbuf,int menu_index);
static int _viv_convert_menu_ini_name_ch(int ch);
static utf8_t *_viv_cat_command_menu_ini_name_path(utf8_t *buf,int menu_index);
static void _viv_get_menu_display_name(wchar_t *buf,const utf8_t *menu_name);
int _viv_command_index_from_command_id(int command_id);
static int _viv_vk_to_text(wchar_t *wbuf,int vk);
static void _viv_cat_key_mod(wchar_t *wbuf,int vk,const utf8_t *default_keytext);
void _viv_get_key_text(wchar_t *wbuf,DWORD keyflags);
HMENU _viv_create_menu(void);
void _viv_key_add(_viv_key_list_t *key_list,int command_index,DWORD keyflags);
static void _viv_key_clear(_viv_key_list_t *key_list,int command_index);
void _viv_key_list_copy(_viv_key_list_t *dst,const _viv_key_list_t *src);
void _viv_key_clear_all(_viv_key_list_t *list);
void _viv_key_list_init(_viv_key_list_t *list);
int _viv_get_current_key_mod_flags(void);
void _viv_key_remove(_viv_key_list_t *keylist,int command_index,DWORD keyflags);

typedef struct _viv_menu_draw_t
{
	int type;			// _VIV_MENU_DRAW_*
	int command_index;	// COMMAND / POPUP: index into _viv_commands
	int localization_id;	// LOCALIZED
	int recent_index;	// RECENT: index into config_recent_files
	const wchar_t *text;	// TEXT: the builder's label store
} _viv_menu_draw_t;

static _viv_menu_draw_t _viv_menu_draw_pool[_VIV_COMMAND_COUNT + 16];
static int _viv_menu_draw_count;
static _viv_menu_draw_t _viv_recent_draw_pool[CONFIG_RECENT_FILE_COUNT + 4];
static int _viv_recent_draw_count;
static _viv_menu_draw_t _viv_context_draw_pool[_VIV_COMMAND_COUNT + 16];
static int _viv_context_draw_count;
// rc.13: the settings dropdown rows (one popup at a time, reset per open).
static _viv_menu_draw_t _viv_settings_draw_pool[32];
static int _viv_settings_draw_count;

// dips at the primary monitor dpi (the menubar uses the same convention).
static int _viv_menu_dip(int d)
{
	return (d * os_logical_high) / 96;
}

void _viv_menu_row_pool_reset(int pool)
{
	switch (pool)
	{
		case _VIV_MENU_POOL_RECENT:
			_viv_recent_draw_count = 0;
			break;

		case _VIV_MENU_POOL_CONTEXT:
			_viv_context_draw_count = 0;
			break;

		case _VIV_MENU_POOL_SETTINGS:
			_viv_settings_draw_count = 0;
			break;

		default:
			_viv_menu_draw_count = 0;
			break;
	}
}

void *_viv_menu_row_alloc(int pool,int type,int command_index,int localization_id,int recent_index)
{
	_viv_menu_draw_t *table;
	int *count;
	int cap;
	_viv_menu_draw_t *row;

	table = _viv_menu_draw_pool;
	count = &_viv_menu_draw_count;
	cap = _VIV_COMMAND_COUNT + 16;

	switch (pool)
	{
		case _VIV_MENU_POOL_RECENT:
			table = _viv_recent_draw_pool;
			count = &_viv_recent_draw_count;
			cap = CONFIG_RECENT_FILE_COUNT + 4;
			break;

		case _VIV_MENU_POOL_SETTINGS:
			table = _viv_settings_draw_pool;
			count = &_viv_settings_draw_count;
			cap = 32;
			break;
		
		case _VIV_MENU_POOL_CONTEXT:
			table = _viv_context_draw_pool;
			count = &_viv_context_draw_count;
			break;
	}

	if (*count >= cap)
	{
		return 0;
	}

	row = &table[*count];
	(*count)++;

	row->type = type;
	row->command_index = command_index;
	row->localization_id = localization_id;
	row->recent_index = recent_index;
	row->text = 0;

	return row;
}

// rc.13: attach an ad hoc label to a text row. the store behind the
// pointer belongs to the builder (the settings dropdowns keep a static
// label array that the pool reset reclaims with the rows).
void _viv_menu_row_set_text(void *row,const wchar_t *text)
{
	_viv_menu_draw_t *draw;
	
	draw = (_viv_menu_draw_t *)row;
	
	if (draw)
	{
		draw->text = text;
	}
}



void _viv_check_menus(HMENU hmenu)
{
	int is_slideshow;
	int fill_window;
	int rw;
	int rh;
	int slideshow_rate_id;
	UINT is_image_enabled;
	
	is_slideshow = 0;

	if (_viv_is_fullscreen)
	{
		if (_viv_is_slideshow)
		{	
			is_slideshow = 1;
			_viv_status_update();
			_viv_toolbar_update_buttons();
			_viv_update_ontop();
		}
		
		fill_window = config_fullscreen_fill_window;
	}
	else
	{
		fill_window = config_fill_window;
	}
				
	_viv_get_render_size(&rw,&rh);
	
	is_image_enabled = ((*_viv_current_fd->cFileName) && (!_viv_file_not_found) && (!_viv_load_failed)) ? MF_ENABLED : MF_DISABLED;

	EnableMenuItem(hmenu,VIV_ID_FILE_CLOSE,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_EDIT_COPY,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_EDIT_COPY_FILENAME,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_EDIT_COPY_IMAGE,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_EDIT_COPY_TO,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_EDIT_CUT,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_FILE_DELETE,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_FILE_EDIT,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_EDIT_MOVE_TO,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_FILE_OPEN_FILE_LOCATION,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_FILE_PREVIEW,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_FILE_PRINT,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_FILE_PROPERTIES,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_FILE_SET_DESKTOP_WALLPAPER,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_FILE_SAVE_AS,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_FILE_RENAME,is_image_enabled);

	EnableMenuItem(hmenu,VIV_ID_EDIT_ROTATE_270,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_EDIT_ROTATE_90,is_image_enabled);
//	EnableMenuItem(hmenu,VIV_ID_SLIDESHOW_PAUSE,is_image_enabled);

	CheckMenuItem(hmenu,VIV_ID_VIEW_CAPTION,config_show_caption ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_THICKFRAME,config_show_thickframe ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_MENU,config_show_menu ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_CONTROLS,config_show_controls ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_ZOOM_CONTROLS,config_show_zoom_controls ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_ZOOM_AUTO_HIDE,config_zoom_auto_hide ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_BACKDROP_FOLLOW,config_backdrop_mode == CONFIG_BACKDROP_MODE_FOLLOW ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_VIEW_BACKDROP_BLACK,config_backdrop_mode == CONFIG_BACKDROP_MODE_BLACK ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_VIEW_BACKDROP_WHITE,config_backdrop_mode == CONFIG_BACKDROP_MODE_WHITE ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_VIEW_BACKDROP_CUSTOM,config_backdrop_mode == CONFIG_BACKDROP_MODE_CUSTOM ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_VIEW_BACKDROP_CHECKERBOARD,config_backdrop_mode == CONFIG_BACKDROP_MODE_CHECKERBOARD ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_VIEW_STATUS,config_show_status ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_ALLOW_SHRINKING,config_allow_shrinking ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_KEEP_ASPECT_RATIO,config_keep_aspect_ratio ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_FILL_WINDOW,fill_window ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_1TO1,((rw == _viv_image_wide) && (rh == _viv_image_high)) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_FULLSCREEN,_viv_is_fullscreen ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_SLIDESHOW,is_slideshow ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_ONTOP_ALWAYS,config_ontop == 1 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_VIEW_ONTOP_WHILE_PLAYING_OR_ANIMATING,config_ontop == 2 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_VIEW_ONTOP_NEVER,config_ontop == 0 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_PAUSE,_viv_is_slideshow ? MF_CHECKED : MF_UNCHECKED);

	switch(config_slideshow_rate)
	{
		case 250: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_250; break;
		case 500: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_500; break;
		case 1000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_1000; break;
		case 2000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_2000; break;
		case 3000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_3000; break;
		case 4000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_4000; break;
		case 5000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_5000; break;
		case 6000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_6000; break;
		case 7000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_7000; break;
		case 8000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_8000; break;
		case 9000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_9000; break;
		case 10000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_10000; break;
		case 20000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_20000; break;
		case 30000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_30000; break;
		case 40000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_40000; break;
		case 50000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_50000; break;
		case 60000: slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_60000; break;

		default:
			slideshow_rate_id = VIV_ID_SLIDESHOW_RATE_CUSTOM;
			break;
	}

	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_250,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_250 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_500,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_500 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_1000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_1000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_2000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_2000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_3000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_3000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_4000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_4000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_5000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_5000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_6000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_6000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_7000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_7000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_8000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_8000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_9000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_9000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_10000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_10000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_20000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_20000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_30000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_30000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_40000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_40000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_50000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_50000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_60000,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_60000 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_RATE_CUSTOM,slideshow_rate_id == VIV_ID_SLIDESHOW_RATE_CUSTOM ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));

	CheckMenuItem(hmenu,VIV_ID_ANIMATION_PLAY_PAUSE,_viv_animation_play ? MF_CHECKED : MF_UNCHECKED);
	
	CheckMenuItem(hmenu,VIV_ID_NAV_SHUFFLE,config_shuffle ? MF_CHECKED : MF_UNCHECKED);
	
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_NAME,config_nav_sort == CONFIG_NAV_SORT_NAME ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_FULL_PATH,config_nav_sort == CONFIG_NAV_SORT_FULL_PATH_AND_FILENAME ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_SIZE,config_nav_sort == CONFIG_NAV_SORT_SIZE ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_DATE_MODIFIED,config_nav_sort == CONFIG_NAV_SORT_DATE_MODIFIED ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_DATE_CREATED,config_nav_sort == CONFIG_NAV_SORT_DATE_CREATED ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));

	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_ASCENDING,config_nav_sort_ascending ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_DESCENDING,(!config_nav_sort_ascending) ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	
	CheckMenuItem(hmenu,VIV_ID_VIEW_1TO1,((rw == _viv_image_wide) && (rh == _viv_image_high)) ? MF_CHECKED : MF_UNCHECKED);
}
void _viv_get_command_name(wchar_t *wbuf,int command_index)
{
	wchar_t menu_wbuf[STRING_SIZE];
	
	*wbuf = 0;
	
	_viv_cat_command_menu_path(wbuf,_viv_commands[command_index].menu_id);

	_viv_get_menu_display_name(menu_wbuf,localization_get_string(_viv_commands[command_index].localization_id));
	
	string_cat(wbuf,menu_wbuf);
}
static void _viv_cat_command_menu_path(wchar_t *wbuf,int menu_index)
{
	int command_index;
	
	if (menu_index == _VIV_MENU_ROOT)
	{
		return;
	}
	
	command_index = _viv_command_index_from_command_id(menu_index);
	
	if (command_index != -1)
	{
		wchar_t menu_wbuf[STRING_SIZE];
		
		_viv_get_menu_display_name(menu_wbuf,localization_get_string(_viv_commands[command_index].localization_id));
		
		_viv_cat_command_menu_path(wbuf,_viv_commands[command_index].menu_id);
		
		string_cat(wbuf,menu_wbuf);
		string_cat_utf8(wbuf," | ");
	}
}
static int _viv_convert_menu_ini_name_ch(int ch)
{
	if ((ch >= 'A') && (ch <= 'Z'))
	{
		return ch - 'A' + 'a';	
	}

	if ((ch >= 'a') && (ch <= 'z'))
	{
		return ch;	
	}

	if ((ch >= '0') && (ch <= '9'))
	{
		return ch;	
	}

	if (ch == ' ')
	{
		return '_';
	}	
	
	return 0;
}
static utf8_t *_viv_cat_command_menu_ini_name_path(utf8_t *buf,int menu_index)
{
	int command_index;
	utf8_t *d;
	
	if (menu_index == _VIV_MENU_ROOT)
	{
		return buf;
	}
	
	d = buf;
	
	command_index = _viv_command_index_from_command_id(menu_index);
	
	if (command_index != -1)
	{
		const utf8_t *p;
		
		d = _viv_cat_command_menu_ini_name_path(d,_viv_commands[command_index].menu_id);
		
		p = localization_get_en_us_string(_viv_commands[command_index].localization_id);
		
		while(*p)
		{
			int ch;
			
			ch = _viv_convert_menu_ini_name_ch(*p);
			
			if (ch)
			{
				*d++ = ch;
			}

			p++;
		}
		
		*d++ = '_';
	}
	
	return d;
}
// make sure this is english only.
int viv_menu_name_to_ini_name(utf8_t *buf,int command_index)
{
	if (!(_viv_commands[command_index].flags & MF_POPUP))
	{
		if (!(_viv_commands[command_index].flags & MF_SEPARATOR))
		{
			if (!(_viv_commands[command_index].flags & MF_DELETE))
			{
				utf8_t *d;
				const utf8_t *p;
				
				d = buf;
				p = localization_get_en_us_string(_viv_commands[command_index].localization_id);
				
				d = _viv_cat_command_menu_ini_name_path(d,_viv_commands[command_index].menu_id);
				
				while(*p)
				{
					int ch;
					
					ch = _viv_convert_menu_ini_name_ch(*p);
					
					if (ch)
					{
						*d++ = ch;
					}

					p++;
				}
				
				*d++ = '_';
				*d++ = 'k';
				*d++ = 'e';
				*d++ = 'y';
				*d++ = 's';
				
				*d = 0;
				
				return 1;
			}
		}
	}
	
	return 0;
}
static void _viv_get_menu_display_name(wchar_t *buf,const utf8_t *menu_name)
{
	wchar_t *d;
	const wchar_t *p;

	string_copy_utf8_string(buf,menu_name);
	
	d = buf;
	p = buf;
	
	while(*p)
	{
		if ((*p == '&') && (p[1] == '&'))
		{
			*d++ = '&';
			p += 2;
			continue;
		}
		else
		if (*p == '&')
		{
		}
		else
		{
			*d++ = *p;	
		}
		
		p++;
	}
	
	*d = 0;
}
int _viv_command_index_from_command_id(int command_id)
{
	int i;
	
	for(i=0;i<_VIV_COMMAND_COUNT;i++)
	{
		if (_viv_commands[i].command_id == command_id)
		{
			return i;
		}
	}
	
	return -1;
}
static int _viv_vk_to_text(wchar_t *wbuf,int vk)
{
	UINT scan_code;
	
	scan_code = MapVirtualKeyExW(vk,0,GetKeyboardLayout(GetCurrentThreadId()));
	
	if (scan_code)
	{
		LONG lParam;
		
		lParam = scan_code << 16;
		
		switch (vk)
		{
			case VK_CONTROL:
			case VK_SHIFT:
			case VK_MENU:
			case VK_LWIN:
			case VK_RWIN:
				// dont care
				lParam |= (1 << 25);
				break;

			case VK_HOME:
			case VK_END:
			case VK_PRIOR:
			case VK_NEXT:
			case VK_INSERT:
			case VK_DELETE:
			case VK_NUMLOCK:
			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
			case VK_DIVIDE:
				// extended
				lParam |= (1 << 24);
				break;
		}		

		if (GetKeyNameText(lParam,wbuf,STRING_SIZE))
		{
			return 1;
		}
	}
	
	return 0;
}
static void _viv_cat_key_mod(wchar_t *wbuf,int vk,const utf8_t *default_keytext)
{
	wchar_t keytext[STRING_SIZE];
	
	if (!_viv_vk_to_text(keytext,vk))
	{
		string_copy_utf8_string(keytext,default_keytext);
	}
	
	string_cat(wbuf,keytext);
	string_cat_utf8(wbuf,(const utf8_t *)"+");
}
void _viv_get_key_text(wchar_t *wbuf,DWORD keyflags)
{
	wchar_t keytext[STRING_SIZE];
	
	*wbuf = 0;
	
	if (keyflags & CONFIG_KEYFLAG_CTRL)
	{
		_viv_cat_key_mod(wbuf,VK_CONTROL,"Ctrl");
	}
	
	if (keyflags & CONFIG_KEYFLAG_ALT)
	{
		_viv_cat_key_mod(wbuf,VK_MENU,"Alt");
	}
	
	if (keyflags & CONFIG_KEYFLAG_SHIFT)
	{
		_viv_cat_key_mod(wbuf,VK_SHIFT,"Shift");
	}
	
	if (_viv_vk_to_text(keytext,keyflags & CONFIG_KEYFLAG_VK_MASK))
	{
		string_cat(wbuf,keytext);
	}
}
HMENU _viv_create_menu(void)
{
	HMENU hmenu;
	
	hmenu = CreateMenu();
	
	_viv_menu_row_pool_reset(_VIV_MENU_POOL_MAIN);
	
	{
		int i;
		HMENU menus[_VIV_MENU_COUNT];
		
		for(i=1;i<_VIV_MENU_COUNT;i++)
		{
			menus[i] = 0;
		}
		menus[0] = hmenu;
		
		for(i=0;i<_VIV_COMMAND_COUNT;i++)
		{
			if (!(_viv_commands[i].flags & MF_OWNERDRAW))
			{
				int flags;
				void *row;

				flags = _viv_commands[i].flags & (~MF_DELETE);

				if (_viv_commands[i].flags & MF_SEPARATOR)
				{
					row = _viv_menu_row_alloc(_VIV_MENU_POOL_MAIN,_VIV_MENU_DRAW_SEPARATOR,0,0,0);

					if (row)
					{
						AppendMenu(menus[_viv_commands[i].menu_id],flags | MF_OWNERDRAW,_viv_commands[i].command_id,(LPCWSTR)row);
					}
					else
					{
						AppendMenu(menus[_viv_commands[i].menu_id],flags,_viv_commands[i].command_id,L"");
					}
				}
				else
				{
					if (_viv_commands[i].flags & MF_POPUP)
					{
						if (!menus[_viv_commands[i].command_id])
						{
							menus[_viv_commands[i].command_id] = CreatePopupMenu();
						}

						row = _viv_menu_row_alloc(_VIV_MENU_POOL_MAIN,_VIV_MENU_DRAW_POPUP,i,0,0);

						if (row)
						{
							AppendMenu(menus[_viv_commands[i].menu_id],flags | MF_OWNERDRAW,(UINT_PTR)menus[_viv_commands[i].command_id],(LPCWSTR)row);
						}
						else
						{
							wchar_t text_wbuf[STRING_SIZE];

							string_copy_utf8_string(text_wbuf,localization_get_string(_viv_commands[i].localization_id));
							AppendMenu(menus[_viv_commands[i].menu_id],flags,(UINT_PTR)menus[_viv_commands[i].command_id],text_wbuf);
						}
					}
					else
					{
						row = _viv_menu_row_alloc(_VIV_MENU_POOL_MAIN,_VIV_MENU_DRAW_COMMAND,i,0,0);

						if (row)
						{
							AppendMenu(menus[_viv_commands[i].menu_id],flags | MF_OWNERDRAW,_viv_commands[i].command_id,(LPCWSTR)row);
						}
						else
						{
							wchar_t text_wbuf[STRING_SIZE];

							string_copy_utf8_string(text_wbuf,localization_get_string(_viv_commands[i].localization_id));
							AppendMenu(menus[_viv_commands[i].menu_id],flags,_viv_commands[i].command_id,text_wbuf);
						}
					}
				}
			}
		}
		
		// the recent-files mru submenu is dynamic (paths from the ini), so
		// it is built after the static table walk and inserted before the
		// exit item of the file menu.
		{
			HMENU recent_menu;
			wchar_t text_wbuf[STRING_SIZE];
			
			recent_menu = _viv_create_recent_menu();
			
			string_copy_utf8_string(text_wbuf,localization_get_string(LOCALIZATION_ID_RECENT_FILES));
			
			{
				MENUITEMINFOW mii;
				int insert_pos;
				
				os_zero_memory(&mii,sizeof(mii));
				mii.cbSize = sizeof(mii);
				// owner drawn like every other row: the type flag lives in fType
				// (fState only takes MFS_ values; the old line was silently ignored
				// and the header painted in the system look).
				mii.fMask = MIIM_SUBMENU | MIIM_FTYPE | MIIM_ID | MIIM_DATA;
				mii.fType = MFT_OWNERDRAW;
				mii.wID = _VIV_MENU_FILE_RECENT;
				mii.hSubMenu = recent_menu;
				mii.dwTypeData = text_wbuf;
				mii.dwItemData = (ULONG_PTR)_viv_menu_row_alloc(_VIV_MENU_POOL_MAIN,_VIV_MENU_DRAW_LOCALIZED,0,LOCALIZATION_ID_RECENT_FILES,0);
				
				// right before the exit item (the last row of the file menu).
				insert_pos = GetMenuItemCount(menus[_VIV_MENU_FILE]) - 1;
				InsertMenuItemW(menus[_VIV_MENU_FILE],insert_pos,TRUE,&mii);
			}
		}
	}
	
	return hmenu;
}
void _viv_key_add(_viv_key_list_t *key_list,int command_index,DWORD keyflags)
{	
	config_key_t *key;
	
	key = mem_alloc(sizeof(config_key_t));
	
	key->key = keyflags;
	
	if (key_list->start[command_index])
	{
		key_list->last[command_index]->next = key;
	}
	else
	{
		key_list->start[command_index] = key;
	}
	
	key->next = 0;
	key_list->last[command_index] = key;
}
static void _viv_key_clear(_viv_key_list_t *key_list,int command_index)
{	
	config_key_t *key;
	
	key = key_list->start[command_index];
	
	while(key)
	{
		config_key_t *next_key;
		
		next_key = key->next;
		
		mem_free(key);
		
		key = next_key;
	}								

	key_list->start[command_index] = 0;
	key_list->last[command_index] = 0;
}
void _viv_key_list_copy(_viv_key_list_t *dst,const _viv_key_list_t *src)
{
	int i;

	_viv_key_clear_all(dst);
	
	for(i=0;i<_VIV_COMMAND_COUNT;i++)
	{
		config_key_t *key;

		key = src->start[i];
		
		while(key)
		{
			_viv_key_add(dst,i,key->key);
			
			key = key->next;
		}
	}
}
void _viv_key_clear_all(_viv_key_list_t *list)
{
	// free keys
	int i;
	
	for(i=0;i<_VIV_COMMAND_COUNT;i++)
	{
		_viv_key_clear(list,i);
	}
}
void _viv_key_list_init(_viv_key_list_t *list)
{
	// free keys
	int i;
	
	for(i=0;i<_VIV_COMMAND_COUNT;i++)
	{
		list->start[i] = 0;
		list->last[i] = 0;
	}
}
int _viv_get_current_key_mod_flags(void)
{
	int key_flags;
	
	key_flags = 0;
	
	if (GetKeyState(VK_CONTROL) < 0)
	{
		key_flags |= CONFIG_KEYFLAG_CTRL;
	}
	
	if (GetKeyState(VK_SHIFT) < 0)
	{
		key_flags |= CONFIG_KEYFLAG_SHIFT;
	}
	
	if (GetKeyState(VK_MENU) < 0)
	{
		key_flags |= CONFIG_KEYFLAG_ALT;
	}
	
	return key_flags;
}	
void _viv_key_remove(_viv_key_list_t *keylist,int command_index,DWORD keyflags)
{
	config_key_t *key;
	
	key = keylist->start[command_index];
	
	keylist->start[command_index] = 0;
	keylist->last[command_index] = 0;
	
	while(key)
	{
		config_key_t *next_key;
		
		next_key = key->next;
		
		if (key->key == keyflags)
		{
			mem_free(key);
		}
		else
		{
			if (keylist->start[command_index])
			{
				keylist->last[command_index]->next = key;
			}
			else
			{
				keylist->start[command_index] = key;
			}
			
			keylist->last[command_index] = key;
			key->next = 0;
		}

		key = next_key;
	}
}
void viv_key_add(int command_index,DWORD keyflags)
{	
	_viv_key_add(_viv_key_list,command_index,keyflags);
}
void viv_key_clear_all(int command_index)
{
	_viv_key_clear(_viv_key_list,command_index);
}
config_key_t *viv_key_get_start(int command_index)
{
	return _viv_key_list->start[command_index];
}
int viv_get_command_count(void)
{
	return _VIV_COMMAND_COUNT;
}

// ------------------------------------------------------------------
// owner drawn popup rows.
//
// every visible row in every popup carries a tiny _viv_menu_draw_t: the
// painters re-derive the label at draw time from the command table, the
// localization table and the live key bindings, so a language or shortcut
// change can never show a stale row and no label text is stored anywhere.
// the frame menu tree, the live recent rebuild and the canvas context menu
// each get their own pool so none can trample another's rows.
// ------------------------------------------------------------------

// the label for a row. command rows carry the live shortcut after a tab;
// the slideshow pause row reads as play/pause and the rate submenu header
// reads as rate, mirroring the canvas context menu overrides.
static void _viv_menu_row_text(_viv_menu_draw_t *draw,wchar_t *wbuf)
{
	*wbuf = 0;

	switch (draw->type)
	{
		case _VIV_MENU_DRAW_COMMAND:
		case _VIV_MENU_DRAW_POPUP:
		{
			int key_command_index;

			if ((draw->command_index < 0) || (draw->command_index >= _VIV_COMMAND_COUNT))
			{
				break;
			}

			switch (_viv_commands[draw->command_index].command_id)
			{
				case VIV_ID_SLIDESHOW_PAUSE:
					string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_PLAY_PAUSE));
					break;

				case _VIV_MENU_SLIDESHOW_RATE:
					string_copy_utf8_string(wbuf,localization_get_string(LOCALIZATION_ID_RATE));
					break;

				default:
					string_copy_utf8_string(wbuf,localization_get_string(_viv_commands[draw->command_index].localization_id));
					break;
			}

			if (draw->type == _VIV_MENU_DRAW_COMMAND)
			{
				key_command_index = draw->command_index;

				switch (_viv_commands[draw->command_index].command_id)
				{
					case VIV_ID_FILE_DELETE:
						key_command_index = _viv_command_index_from_command_id(VIV_ID_FILE_DELETE_RECYCLE);
						break;
				}

				if (key_command_index >= 0)
				{
					if (_viv_key_list->start[key_command_index])
					{
						wchar_t key_text[STRING_SIZE];

						_viv_get_key_text(key_text,_viv_key_list->start[key_command_index]->key);

						string_cat_utf8(wbuf,(const utf8_t *)"\t");
						string_cat(wbuf,key_text);
					}
				}
			}
			break;
		}

		case _VIV_MENU_DRAW_RECENT:
		{
			wchar_t num_wbuf[64];

			if ((draw->recent_index < 0) || (draw->recent_index >= config_recent_file_count))
			{
				break;
			}

			// the mru convention: an ampersand digit prefix selects the entry
			// from the keyboard while the submenu is open.
			string_format_number(num_wbuf,draw->recent_index + 1);
			string_copy(wbuf,L"&");
			string_cat(wbuf,num_wbuf);
			string_cat(wbuf,L" ");
			string_cat(wbuf,string_get_filename_part(config_recent_files[draw->recent_index]));
			break;
		}

		case _VIV_MENU_DRAW_LOCALIZED:
			string_copy_utf8_string(wbuf,localization_get_string(draw->localization_id));

		case _VIV_MENU_DRAW_TEXT:
			// the ad hoc settings dropdown labels.
			if (draw->text)
			{
				string_copy(wbuf,draw->text);
			}
			break;
	}
}

// public: resolve a menu row's live label. owner drawn rows carry no
// stored string on the item, so the menubar reads its root labels back
// through this (the item data holds the row, the text re-derives from
// the live localization and key tables at ask time).
void _viv_menu_row_item_text(void *row,wchar_t *wbuf)
{
	*wbuf = 0;

	if (row)
	{
		_viv_menu_row_text((_viv_menu_draw_t *)row,wbuf);
	}
}

int _viv_menu_measure_item(MEASUREITEMSTRUCT *measure_item)
{
	_viv_menu_draw_t *draw;
	HDC hdc;
	HFONT font;
	HFONT old_font;
	wchar_t text_wbuf[STRING_SIZE];
	wchar_t *tab;
	SIZE size;
	int wide;
	int dpi;

	if ((!measure_item) || (measure_item->CtlType != ODT_MENU))
	{
		return 0;
	}

	draw = (_viv_menu_draw_t *)measure_item->itemData;

	if (!draw)
	{
		return 0;
	}

	dpi = os_logical_high;

	if (draw->type == _VIV_MENU_DRAW_SEPARATOR)
	{
		// the menu width comes from the other rows.
		measure_item->itemWidth = 0;
		measure_item->itemHeight = (9 * dpi) / 96;

		return 1;
	}

	hdc = GetDC(_viv_hwnd);

	if (!hdc)
	{
		return 0;
	}

	_viv_menu_row_text(draw,text_wbuf);

	tab = wcschr(text_wbuf,L'\t');

	if (tab)
	{
		*tab = 0;
	}

	size.cx = 0;
	size.cy = 0;

	font = _viv_menu_font();
	old_font = 0;

	if (font)
	{
		old_font = SelectObject(hdc,font);
	}

	GetTextExtentPoint32W(hdc,text_wbuf,(int)wcslen(text_wbuf),&size);

	if (tab)
	{
		SIZE key_size;

		if (GetTextExtentPoint32W(hdc,tab + 1,(int)wcslen(tab + 1),&key_size))
		{
			// shortcut column: gap before, text, padding after.
			size.cx += key_size.cx + (28 * dpi) / 96;
		}
	}
	else
	{
		if (draw->type == _VIV_MENU_DRAW_POPUP)
		{
			size.cx += (18 * dpi) / 96;
		}
	}

	if (old_font)
	{
		SelectObject(hdc,old_font);
	}

	ReleaseDC(_viv_hwnd,hdc);

	// check gutter plus the right padding.
	wide = size.cx + (24 * dpi) / 96 + (14 * dpi) / 96;

	measure_item->itemWidth = wide;
	measure_item->itemHeight = size.cy + (9 * dpi) / 96;

	if (measure_item->itemHeight < (22 * dpi) / 96)
	{
		measure_item->itemHeight = (22 * dpi) / 96;
	}

	return 1;
}

static void _viv_menu_draw_mark(HDC hdc,const RECT *rect,int center_y)
{
	HPEN pen;
	HPEN old_pen;
	COLORREF accent;

	accent = viv_theme_color(VIV_TK_ACCENT);

	pen = CreatePen(PS_SOLID,_viv_menu_dip(2) < 1 ? 1 : _viv_menu_dip(2),accent);
	old_pen = (HPEN)SelectObject(hdc,pen);

	MoveToEx(hdc,rect->left + _viv_menu_dip(5),center_y,0);
	LineTo(hdc,rect->left + _viv_menu_dip(9),center_y + _viv_menu_dip(4));
	LineTo(hdc,rect->left + _viv_menu_dip(16),center_y - _viv_menu_dip(5));

	SelectObject(hdc,old_pen);
	DeleteObject(pen);
}

// the radio mark: a filled accent circle centered in the check gutter.
static void _viv_menu_draw_dot(HDC hdc,const RECT *rect,int center_y)
{
	HBRUSH brush;
	HBRUSH old_brush;
	int radius;
	int x;

	radius = _viv_menu_dip(4);

	if (radius < 2)
	{
		radius = 2;
	}

	// the dot centers in the same gutter the checkmark occupies.
	x = rect->left + _viv_menu_dip(10);

	brush = CreateSolidBrush(viv_theme_color(VIV_TK_ACCENT));
	old_brush = (HBRUSH)SelectObject(hdc,brush);

	Ellipse(hdc,x - radius,center_y - radius,x + radius + 1,center_y + radius + 1);

	SelectObject(hdc,old_brush);
	DeleteObject(brush);
}

static void _viv_menu_draw_arrow(HDC hdc,int right,int center_y)
{
	HBRUSH brush;
	HBRUSH old_brush;
	POINT pts[3];
	int x;
	int y;

	x = right - _viv_menu_dip(11);
	y = center_y;

	brush = CreateSolidBrush(viv_theme_color(VIV_TK_TEXT2));
	old_brush = (HBRUSH)SelectObject(hdc,brush);

	pts[0].x = x;			pts[0].y = y - _viv_menu_dip(4);
	pts[1].x = x + _viv_menu_dip(5);	pts[1].y = y;
	pts[2].x = x;			pts[2].y = y + _viv_menu_dip(4);

	Polygon(hdc,pts,3);

	SelectObject(hdc,old_brush);
	DeleteObject(brush);
}

int _viv_menu_draw_item(DRAWITEMSTRUCT *draw_item)
{
	_viv_menu_draw_t *draw;
	RECT rect;
	HDC hdc;
	HFONT font;
	HFONT old_font;
	wchar_t text_wbuf[STRING_SIZE];
	wchar_t *tab;
	int selected;
	int disabled;
	int center_y;

	if ((!draw_item) || (draw_item->CtlType != ODT_MENU))
	{
		return 0;
	}

	draw = (_viv_menu_draw_t *)draw_item->itemData;

	if (!draw)
	{
		return 0;
	}

	hdc = draw_item->hDC;
	rect = draw_item->rcItem;
	selected = (draw_item->itemState & (ODS_SELECTED | ODS_HOTLIGHT)) != 0;
	disabled = (draw_item->itemState & (ODS_GRAYED | ODS_INACTIVE)) != 0;
	center_y = rect.top + ((rect.bottom - rect.top) / 2);

	if (draw->type == _VIV_MENU_DRAW_SEPARATOR)
	{
		RECT line_rect;

		// hairline centered in the row, inset past the check gutter.
		FillRect(hdc,&rect,viv_theme_brush(VIV_TK_FACE));

		line_rect.left = rect.left + _viv_menu_dip(24);
		line_rect.right = rect.right - _viv_menu_dip(10);
		line_rect.top = center_y;
		line_rect.bottom = center_y + 1;

		FillRect(hdc,&line_rect,viv_theme_brush(VIV_TK_LINE));

		return 1;
	}

	FillRect(hdc,&rect,selected ? viv_theme_brush(VIV_TK_HOVER) : viv_theme_brush(VIV_TK_FACE));

	if (draw_item->itemState & ODS_CHECKED)
	{
		MENUITEMINFOW mii;
		int is_radio;

		// radio groups (backdrop, on top, sort, the rate ladder) draw the
		// dot instead of the checkmark; the type lives on the live item.
		os_zero_memory(&mii,sizeof(mii));
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_FTYPE;
		is_radio = 0;

		if ((draw_item->hwndItem) && (GetMenuItemInfoW((HMENU)draw_item->hwndItem,draw_item->itemID,FALSE,&mii)))
		{
			is_radio = (mii.fType & MFT_RADIOCHECK) != 0;
		}

		if (is_radio)
		{
			_viv_menu_draw_dot(hdc,&rect,center_y);
		}
		else
		{
			_viv_menu_draw_mark(hdc,&rect,center_y);
		}
	}

	font = _viv_menu_font();
	old_font = 0;

	if (font)
	{
		old_font = SelectObject(hdc,font);
	}

	SetBkMode(hdc,TRANSPARENT);

	_viv_menu_row_text(draw,text_wbuf);

	tab = wcschr(text_wbuf,L'\t');

	if (tab)
	{
		*tab = 0;
	}

	SetTextColor(hdc,viv_theme_color(disabled ? VIV_TK_TEXTOFF : VIV_TK_TEXT));

	// the label starts past the check gutter.
	rect.left += _viv_menu_dip(24);

	DrawTextW(hdc,text_wbuf,-1,&rect,DT_SINGLELINE | DT_VCENTER | DT_LEFT);

	if (tab)
	{
		RECT key_rect;

		key_rect = rect;
		key_rect.right -= _viv_menu_dip(12);

		SetTextColor(hdc,viv_theme_color(disabled ? VIV_TK_TEXTOFF : VIV_TK_TEXT2));

		DrawTextW(hdc,tab + 1,-1,&key_rect,DT_SINGLELINE | DT_VCENTER | DT_RIGHT | DT_NOPREFIX);
	}

	if (draw->type == _VIV_MENU_DRAW_POPUP)
	{
		_viv_menu_draw_arrow(hdc,rect.right,center_y);
	}

	if (old_font)
	{
		SelectObject(hdc,old_font);
	}

	return 1;
}

// rc.13: the popup layer theming, extracted from the main window's
// wm_initmenupopup: the #32768 class brush paints the margins between
// the owner drawn rows, and the dwm rounds the layer and colors its
// border where the attributes exist. every popup owner runs this - the
// menu bar, the canvas context menu and the settings dropdowns.
void _viv_menu_popup_theme(void)
{
	HWND menu_hwnd;
	
	menu_hwnd = FindWindowW(L"#32768",0);
	
	if (menu_hwnd)
	{
		SetClassLongPtrW(menu_hwnd,GCLP_HBRBACKGROUND,(LONG_PTR)viv_theme_brush(VIV_TK_FACE));
		
		os_menu_modern_chrome(menu_hwnd,viv_theme_color(VIV_TK_LINE));
	}
}

int _viv_menu_char_item(HMENU hmenu,wchar_t ch,int popup)
{
	int i;
	int count;

	if (!hmenu)
	{
		return -1;
	}

	count = GetMenuItemCount(hmenu);

	for(i=0;i<count;i++)
	{
		MENUITEMINFOW mii;
		_viv_menu_draw_t *draw;
		wchar_t text_wbuf[STRING_SIZE];
		wchar_t *source;

		os_zero_memory(&mii,sizeof(mii));
		mii.cbSize = sizeof(mii);
		// grayed rows never fire from the keyboard (system parity).
		mii.fMask = MIIM_DATA | MIIM_FTYPE | MIIM_STATE;

		if (!GetMenuItemInfoW(hmenu,i,TRUE,&mii))
		{
			continue;
		}

		draw = (_viv_menu_draw_t *)mii.dwItemData;

		if ((!draw) || (mii.fState & MF_GRAYED))
		{
			continue;
		}

		_viv_menu_row_text(draw,text_wbuf);

		source = wcschr(text_wbuf,L'&');

		while ((source) && (source[1] == L'&'))
		{
			source = wcschr(source + 2,L'&');
		}

		if ((source) && (source[1]))
		{
			if (towupper(source[1]) == towupper(ch))
			{
				return i;
			}
		}
	}

	return -1;
}
