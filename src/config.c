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
// customizable settings

#include "viv.h"

static int _config_icompare_ascii(const utf8_t *s1,const char *lowercase_ascii_s2);
static void _config_load_settings_by_location(const wchar_t *path,int is_root);
static void _config_write_int(HANDLE h,const char *ascii_key,int value);
static void _config_write_string(HANDLE h,const char *ascii_key,const wchar_t *s);
static void _config_write_utf8(HANDLE h,const utf8_t *s);
static void _config_recent_key_name(utf8_t *buf,int index);
static void _config_save_settings_by_location(const wchar_t *path,int is_root);

BYTE config_appdata = 0; // store settings in %APPDATA%\voidimageviewer or in the same location as voidimageviewer.exe
BYTE config_language = 0; // ui language: 0 = auto (follow the system language), 1 = english, 2 = simplified chinese.
BYTE config_dark_mode = 2; // ui theme: 0 = light, 1 = dark, 2 = auto (follow the windows theme).
BYTE config_ui_accent = 0; // ui accent color index into the viv_theme accent table (0..4).
BYTE config_backdrop_mode = CONFIG_BACKDROP_MODE_FOLLOW; // backdrop under transparent pixels: 0 = follow window background, 1 = black, 2 = white, 3 = custom, 4 = checkerboard
BYTE config_backdrop_color_r = 128; // custom backdrop color
BYTE config_backdrop_color_g = 128;
BYTE config_backdrop_color_b = 128;
BYTE config_keep_centered = 1; // when zooming out, don't recenter the image. (keep cursor under the same pixel)
int config_x = 0;
int config_y = 0;
int config_wide = 0;
int config_high = 0;
BYTE config_maximized = 0;
int config_slideshow_rate = 5000;
BYTE config_allow_shrinking = 1; // prevent resizing an image below 100%
BYTE config_shrink_blit_mode = CONFIG_SHRINK_BLIT_MODE_HALFTONE; // shrink filter
BYTE config_mag_filter = CONFIG_MAG_FILTER_COLORONCOLOR; // magnify filter
// default to sorting by filename in natural order, ascending.
// this matches the windows explorer default, so next/previous navigation
// walks the folder in the order the user sees it in explorer.
BYTE config_nav_sort = CONFIG_NAV_SORT_NAME; // current navigation sort.
BYTE config_nav_sort_ascending = 1; // sort navigation ascending or descending.
BYTE config_keep_aspect_ratio = 1; // stretch images with the original aspect ratio.
BYTE config_fill_window = 0; // stretch the image to fill the window
BYTE config_fullscreen_fill_window = 1; // same as fill_window, except this setting is used when we are fullscreen
BYTE config_auto_zoom = 0; // automatically resize the window to fit the newly loaded image
BYTE config_auto_zoom_type = 1; // 0 = 50%, 1 = 100%, 2 = 200%
int config_auto_fit_wide_mul = 3;
int config_auto_fit_wide_div = 5;
int config_auto_fit_high_mul = 3;
int config_auto_fit_high_div = 5;
BYTE config_frame_minus = 0; // show frame counter or remaining frames in status bar
BYTE config_multiple_instances = 0; // all multiple instances or use a single instance.
BYTE config_show_status = 1;
BYTE config_show_controls = 1;
BYTE config_show_zoom_controls = 0; // show the floating zoom controls. (defaults to on for touch devices)
BYTE config_zoom_auto_hide = 1; // auto-hide the fullscreen zoom overlay when idle.
// recent files mru: newest first, at most CONFIG_RECENT_FILE_COUNT entries.
wchar_t *config_recent_files[CONFIG_RECENT_FILE_COUNT] = {0};
int config_recent_file_count = 0;
BYTE config_prevent_sleep = 1;
BYTE config_loop_animations_once = 1;
BYTE config_mouse_wheel_action = 0; // 0 = zoom, 1 = next/prev, 2=prev/next
BYTE config_ctrl_mouse_wheel_action = 0; // 0 = zoom, 1 = next/prev, 2=prev/next
BYTE config_left_click_action = 0; // 0 = scroll, 1 = play/pause slideshow, 2 = play/pause animation, 3=zoom in, 4=next, 5=1:1 scroll, 6=move-window
BYTE config_right_click_action = 0; // 0 = context menu, 1=zoom out, 2=prev, 
BYTE config_xbutton_action = 2; // 1=zoom, 2=next
BYTE config_windowed_background_color_r = 255;
BYTE config_windowed_background_color_g = 255;
BYTE config_windowed_background_color_b = 255;
BYTE config_fullscreen_background_color_r = 0;
BYTE config_fullscreen_background_color_g = 0;
BYTE config_fullscreen_background_color_b = 0;
BYTE config_options_last_page = 0;
int config_short_jump = 500;
int config_medium_jump = 1000;
int config_long_jump = 2000;
BYTE config_shuffle = 0;
BYTE config_browse_file_open_dialog = 1;
BYTE config_ontop = 0; // 0 = never, 1 = always, 2 = while slideshow or animating.
int config_slideshow_custom_rate = 3; // custom slideshow rate (see type below)
BYTE config_slideshow_custom_rate_type = 1; // 0 = milliseconds, 1 = seconds, 2 = minutes
BYTE config_scroll_window = 1;
BYTE config_preload_next = 1;
BYTE config_cache_last = 1;
BYTE config_icm = 1;
BYTE config_show_menu = 1;
BYTE config_show_caption = 1;
BYTE config_show_thickframe = 1;
BYTE config_toolbar_move_window = 1;
BYTE config_windowed_hide_cursor = 1;
BYTE config_pixel_info = 0;
BYTE config_orientation = 1;
BYTE config_title_bar_format = 1; // 0=full path, 1=filename, 2=none
int config_add_command_line_timeout = 500; // in milliseconds

static void _config_load_settings_by_location(const wchar_t *path,int is_root)
{
	ini_t *ini;
	wchar_t filename[STRING_SIZE];
	
	string_path_combine_utf8(filename,path,(const utf8_t *)"voidImageViewer.ini");
	
	ini = ini_open(filename,"voidImageViewer");
	if (ini)
	{
		config_x = ini_get_int(ini,(const utf8_t *)"x",config_x);
		config_y = ini_get_int(ini,(const utf8_t *)"y",config_y);
		config_wide = ini_get_int(ini,(const utf8_t *)"wide",config_wide);
		config_high = ini_get_int(ini,(const utf8_t *)"high",config_high);
		config_maximized = ini_get_int(ini,(const utf8_t *)"maximized",config_maximized);
		config_slideshow_rate = ini_get_int(ini,(const utf8_t *)"slideshow_rate",config_slideshow_rate);
		config_allow_shrinking = ini_get_int(ini,(const utf8_t *)"allow_shrinking",config_allow_shrinking);
		config_shrink_blit_mode = ini_get_int(ini,(const utf8_t *)"shrink_blit_mode",config_shrink_blit_mode);
		config_mag_filter = ini_get_int(ini,(const utf8_t *)"mag_filter",config_mag_filter);
		config_keep_aspect_ratio = ini_get_int(ini,(const utf8_t *)"keep_aspect_ratio",config_keep_aspect_ratio);
		config_fill_window = ini_get_int(ini,(const utf8_t *)"fill_window",config_fill_window);
		config_fullscreen_fill_window = ini_get_int(ini,(const utf8_t *)"fullscreen_fill_window",config_fullscreen_fill_window);
		config_nav_sort = ini_get_int(ini,(const utf8_t *)"sort",config_nav_sort);
		config_nav_sort_ascending = ini_get_int(ini,(const utf8_t *)"sort_ascending",config_nav_sort_ascending);
		config_multiple_instances = ini_get_int(ini,(const utf8_t *)"multiple_instances",config_multiple_instances);
		config_show_caption = ini_get_int(ini,(const utf8_t *)"show_caption",config_show_caption);
		config_show_thickframe = ini_get_int(ini,(const utf8_t *)"show_thickframe",config_show_thickframe);
		config_show_menu = ini_get_int(ini,(const utf8_t *)"show_menu",config_show_menu);
		config_show_status = ini_get_int(ini,(const utf8_t *)"show_status",config_show_status);
		config_pixel_info = ini_get_int(ini,(const utf8_t *)"statusbar_pixel_info",config_pixel_info);
		config_show_controls = ini_get_int(ini,(const utf8_t *)"show_controls",config_show_controls);
		// the digitizer probe is a first-run default only: msdn warns
		// sm_digitizer has no plug-and-play awareness, so the value gets
		// pinned into the ini on the first save and is never re-probed.
		config_show_zoom_controls = ini_get_int(ini,(const utf8_t *)"show_zoom_controls",os_is_touch_available());
			config_zoom_auto_hide = ini_get_int(ini,(const utf8_t *)"zoom_auto_hide",config_zoom_auto_hide);
		
		// ui language. stored as a readable string: auto, english or chinese.
		{
			const utf8_t *language_value;
			
			language_value = ini_get_string(ini,(const utf8_t *)"language");
			
			if (language_value)
			{
				if (_config_icompare_ascii(language_value,"english") == 0)
				{
					config_language = 1;
				}
				else
				if (_config_icompare_ascii(language_value,"chinese") == 0)
				{
					config_language = 2;
				}
				else
				{
					// "auto" (and anything we don't understand) follows the system language.
					config_language = 0;
				}
			}
		}
		
		// dark mode. stored as a readable string: auto, dark or light.
		{
			const utf8_t *dark_mode_value;
			
			dark_mode_value = ini_get_string(ini,(const utf8_t *)"dark_mode");
			
			if (dark_mode_value)
			{
				if (_config_icompare_ascii(dark_mode_value,"dark") == 0)
				{
					config_dark_mode = 1;
				}
				else
				if (_config_icompare_ascii(dark_mode_value,"light") == 0)
				{
					config_dark_mode = 0;
				}
				else
				{
					// "auto" (and anything we don't understand) follows the windows theme.
					config_dark_mode = 2;
				}
			}
		}

		// accent color. an index into the viv_theme accent table, clamped.
		config_ui_accent = (BYTE)ini_get_int(ini,(const utf8_t *)"accent_color",0);
		
		if (config_ui_accent >= VIV_THEME_ACCENT_COUNT)
		{
			config_ui_accent = 0;
		}
		config_backdrop_mode = ini_get_int(ini,(const utf8_t *)"backdrop_mode",config_backdrop_mode);
		config_backdrop_color_r = ini_get_int(ini,(const utf8_t *)"backdrop_color_r",config_backdrop_color_r);
		config_backdrop_color_g = ini_get_int(ini,(const utf8_t *)"backdrop_color_g",config_backdrop_color_g);
		config_backdrop_color_b = ini_get_int(ini,(const utf8_t *)"backdrop_color_b",config_backdrop_color_b);
		
		// recent files mru: recent_file_0 .. recent_file_9 hold the most
		// recently opened paths, newest first. the walk stops at the first
		// empty slot so a hand-edited hole cannot reorder the list.
		{
			int i;
			
			for(i=0;i<CONFIG_RECENT_FILE_COUNT;i++)
			{
				utf8_t key_buf[32];
				const utf8_t *recent_value;
				
				_config_recent_key_name(key_buf,i);
				recent_value = ini_get_string(ini,key_buf);
				
				if ((recent_value) && (*recent_value) && (config_recent_file_count < CONFIG_RECENT_FILE_COUNT))
				{
					wchar_t *recent_filename;
					
					recent_filename = string_alloc_utf8(recent_value);
					
					if (recent_filename)
					{
						config_recent_files[config_recent_file_count] = recent_filename;
						config_recent_file_count++;
					}
				}
			}
		}
		
		// the walk above already stops at the cap; this clamp is the load-side
		// guard of the same invariant (the menu id block, the save loop and the
		// popup builder all assume count <= CONFIG_RECENT_FILE_COUNT), so any
		// future writer that grows the count cannot walk past it.
		if (config_recent_file_count > CONFIG_RECENT_FILE_COUNT)
		{
			config_recent_file_count = CONFIG_RECENT_FILE_COUNT;
		}
		config_auto_zoom = ini_get_int(ini,(const utf8_t *)"auto_zoom",config_auto_zoom);
		config_auto_zoom_type = ini_get_int(ini,(const utf8_t *)"auto_zoom_type",config_auto_zoom_type);
		config_auto_fit_wide_mul = ini_get_int(ini,(const utf8_t *)"auto_fit_wide_mul",config_auto_fit_wide_mul);
		config_auto_fit_wide_div = ini_get_int(ini,(const utf8_t *)"auto_fit_wide_div",config_auto_fit_wide_div);
		config_auto_fit_high_mul = ini_get_int(ini,(const utf8_t *)"auto_fit_high_mul",config_auto_fit_high_mul);
		config_auto_fit_high_div = ini_get_int(ini,(const utf8_t *)"auto_fit_high_div",config_auto_fit_high_div);
		config_frame_minus = ini_get_int(ini,(const utf8_t *)"frame_minus",config_frame_minus);
		config_mouse_wheel_action = ini_get_int(ini,(const utf8_t *)"mouse_wheel_action",config_mouse_wheel_action);
		config_ctrl_mouse_wheel_action = ini_get_int(ini,(const utf8_t *)"ctrl_mouse_wheel_action",config_ctrl_mouse_wheel_action);
		config_left_click_action = ini_get_int(ini,(const utf8_t *)"left_click_action",config_left_click_action);
		config_right_click_action = ini_get_int(ini,(const utf8_t *)"right_click_action",config_right_click_action);
		config_xbutton_action = ini_get_int(ini,(const utf8_t *)"xbutton_action",config_xbutton_action);
		config_keep_centered = ini_get_int(ini,(const utf8_t *)"keep_centered",config_keep_centered);
		config_windowed_background_color_r = ini_get_int(ini,(const utf8_t *)"windowed_background_color_r",config_windowed_background_color_r);
		config_windowed_background_color_g = ini_get_int(ini,(const utf8_t *)"windowed_background_color_g",config_windowed_background_color_g);
		config_windowed_background_color_b = ini_get_int(ini,(const utf8_t *)"windowed_background_color_b",config_windowed_background_color_b);
		config_windowed_hide_cursor = ini_get_int(ini,(const utf8_t *)"windowed_hide_cursor",config_windowed_hide_cursor);
		config_fullscreen_background_color_r = ini_get_int(ini,(const utf8_t *)"fullscreen_background_color_r",config_fullscreen_background_color_r);
		config_fullscreen_background_color_g = ini_get_int(ini,(const utf8_t *)"fullscreen_background_color_g",config_fullscreen_background_color_g);
		config_fullscreen_background_color_b = ini_get_int(ini,(const utf8_t *)"fullscreen_background_color_b",config_fullscreen_background_color_b);
		config_options_last_page = ini_get_int(ini,(const utf8_t *)"options_last_page",config_options_last_page);
		config_short_jump = ini_get_int(ini,(const utf8_t *)"short_jump",config_short_jump);
		config_medium_jump = ini_get_int(ini,(const utf8_t *)"medium_jump",config_medium_jump);
		config_long_jump = ini_get_int(ini,(const utf8_t *)"long_jump",config_long_jump);
		config_loop_animations_once = ini_get_int(ini,(const utf8_t *)"loop_animations_once",config_loop_animations_once);
		config_prevent_sleep = ini_get_int(ini,(const utf8_t *)"prevent_sleep",config_prevent_sleep);
		config_shuffle = ini_get_int(ini,(const utf8_t *)"shuffle",config_shuffle);
		config_browse_file_open_dialog = ini_get_int(ini,(const utf8_t *)"browse_file_open_dialog",config_browse_file_open_dialog);
		config_ontop = ini_get_int(ini,(const utf8_t *)"ontop",config_ontop);
		config_slideshow_custom_rate = ini_get_int(ini,(const utf8_t *)"slideshow_custom_rate",config_slideshow_custom_rate);
		config_slideshow_custom_rate_type = ini_get_int(ini,(const utf8_t *)"slideshow_custom_rate_type",config_slideshow_custom_rate_type);
		config_scroll_window = ini_get_int(ini,(const utf8_t *)"scroll_window",config_scroll_window);
		config_preload_next = ini_get_int(ini,(const utf8_t *)"preload_next",config_preload_next);
		config_cache_last = ini_get_int(ini,(const utf8_t *)"cache_last",config_cache_last);
		config_icm = ini_get_int(ini,(const utf8_t *)"icm",config_icm);
		config_orientation = ini_get_int(ini,(const utf8_t *)"orientation",config_orientation);
		config_toolbar_move_window = ini_get_int(ini,(const utf8_t *)"toolbar_move_window",config_toolbar_move_window);
		config_title_bar_format = ini_get_int(ini,(const utf8_t *)"title_bar_format",config_title_bar_format);
		config_add_command_line_timeout = ini_get_int(ini,(const utf8_t *)"add_command_line_timeout",config_add_command_line_timeout);

		if (is_root)
		{
			config_appdata = ini_get_int(ini,(const utf8_t *)(const utf8_t *)"appdata",config_appdata);
		}

		{
			int i;
			
			for(i=0;i<viv_get_command_count();i++)
			{
				utf8_t key_buf[STRING_SIZE];
				
				// get ini key name.
				if (viv_menu_name_to_ini_name(key_buf,i))
				{
					const utf8_t *key_list;
					const utf8_t *p;
					
					key_list = ini_get_string(ini,key_buf);
					
					if (key_list)
					{
						p = key_list;
						
						// clear this key.
						viv_key_clear_all(i);
						
						while(*p)
						{
							const utf8_t *start;
							
							start = p;

							while(*p)
							{
								if (*p == ',')
								{
									*p++;
									break;
								}
								
								p++;
							}
						
							viv_key_add(i,utf8_to_int(start));
						}
					}
				}
			}
		}

		ini_close(ini);
	}
}

void config_load_settings(void)
{
	wchar_t path[STRING_SIZE];
	
	string_get_exe_path(path);
	
	_config_load_settings_by_location(path,1);
		
	if (config_appdata)
	{
		if (string_get_appdata_voidimageviewer_path(path))
		{
			_config_load_settings_by_location(path,0);
		}
	}
}


static int _config_icompare_ascii(const utf8_t *s1,const char *lowercase_ascii_s2)
{
	// case-insensitive compare of an ascii/utf-8 string against a lowercase ascii keyword.
	// returns 0 when equal.
	
	while (*s1 && *lowercase_ascii_s2)
	{
		char c;
		
		c = (char)*s1;
		
		if ((c >= 'A') && (c <= 'Z'))
		{
			c += 'a' - 'A';
		}
		
		if (c != *lowercase_ascii_s2)
		{
			return 1;
		}
		
		s1++;
		lowercase_ascii_s2++;
	}
	
	if (*s1 || *lowercase_ascii_s2)
	{
		return 1;
	}
	
	return 0;
}

static void _config_write_int(HANDLE h,const char *ascii_key,int value)
{
	wchar_t wbuf[STRING_SIZE];
	
	string_format_number(wbuf,value);
	
	_config_write_string(h,ascii_key,wbuf);
}

static void _config_write_string(HANDLE h,const char *ascii_key,const wchar_t *s)
{
	utf8_t buf[STRING_SIZE*3];
	int ret;
	
	// check the conversion: on failure the buffer is left unterminated and
	// the writes below would read past its end (a full-width string can
	// hit the boundary: STRING_SIZE wchars x 3 utf8 bytes + nul).
	ret = WideCharToMultiByte(CP_UTF8,0,s,-1,(char *)buf,STRING_SIZE*3,0,0);
	
	if ((ret <= 0) || (ret >= (int)sizeof(buf)))
	{
		buf[0] = 0;
	}
	
	_config_write_utf8(h,(const utf8_t *)ascii_key);
	_config_write_utf8(h,(const utf8_t *)"=");
	_config_write_utf8(h,buf);
	_config_write_utf8(h,(const utf8_t *)"\r\n");
}

static void _config_recent_key_name(utf8_t *buf,int index)
{
	// "recent_file_0" .. "recent_file_9"
	buf[0] = 'r';
	buf[1] = 'e';
	buf[2] = 'c';
	buf[3] = 'e';
	buf[4] = 'n';
	buf[5] = 't';
	buf[6] = '_';
	buf[7] = 'f';
	buf[8] = 'i';
	buf[9] = 'l';
	buf[10] = 'e';
	buf[11] = '_';
	buf[12] = (utf8_t)('0' + (index % 10));
	buf[13] = 0;
}

static void _config_write_utf8(HANDLE h,const utf8_t *s)
{
	DWORD num_written;
	
	WriteFile(h,s,utf8_length(s),&num_written,0);
}

static void _config_save_settings_by_location(const wchar_t *path,int is_root)
{
	HANDLE h;
	wchar_t tempname[STRING_SIZE];
	wchar_t filename[STRING_SIZE];
	
	string_path_combine_utf8(filename,path,(const utf8_t *)"voidImageViewer.ini");

	string_copy(tempname,filename);
	string_cat_utf8(tempname,(const utf8_t *)".tmp");

	h = CreateFile(tempname,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if (h != INVALID_HANDLE_VALUE)
	{
		_config_write_utf8(h,(const utf8_t *)"[voidImageViewer]\r\n");
		
		if ((is_root) && (config_appdata))
		{
			_config_write_int(h,"appdata",config_appdata);
		}
		else
		{
			_config_write_int(h,"x",config_x);
			_config_write_int(h,"y",config_y);
			_config_write_int(h,"wide",config_wide);
			_config_write_int(h,"high",config_high);
			_config_write_int(h,"maximized",config_maximized);
			_config_write_int(h,"slideshow_rate",config_slideshow_rate);
			_config_write_int(h,"allow_shrinking",config_allow_shrinking);
			_config_write_int(h,"shrink_blit_mode",config_shrink_blit_mode);
			_config_write_int(h,"mag_filter",config_mag_filter);
			_config_write_int(h,"keep_aspect_ratio",config_keep_aspect_ratio);
			_config_write_int(h,"fill_window",config_fill_window);
			_config_write_int(h,"fullscreen_fill_window",config_fullscreen_fill_window);
			_config_write_int(h,"sort",config_nav_sort);
			_config_write_int(h,"sort_ascending",config_nav_sort_ascending);
			_config_write_int(h,"multiple_instances",config_multiple_instances);
			_config_write_int(h,"show_caption",config_show_caption);
			_config_write_int(h,"show_thickframe",config_show_thickframe);
			_config_write_int(h,"show_menu",config_show_menu);
			_config_write_int(h,"show_status",config_show_status);
			_config_write_int(h,"statusbar_pixel_info",config_pixel_info);
			_config_write_int(h,"show_controls",config_show_controls);
			_config_write_int(h,"show_zoom_controls",config_show_zoom_controls);
				_config_write_int(h,"zoom_auto_hide",config_zoom_auto_hide);
			_config_write_string(h,"language",config_language == 1 ? L"english" : (config_language == 2 ? L"chinese" : L"auto"));
		_config_write_string(h,"dark_mode",config_dark_mode == 1 ? L"dark" : (config_dark_mode == 0 ? L"light" : L"auto"));
		_config_write_int(h,"accent_color",config_ui_accent);
		_config_write_int(h,"backdrop_mode",config_backdrop_mode);
		_config_write_int(h,"backdrop_color_r",config_backdrop_color_r);
		_config_write_int(h,"backdrop_color_g",config_backdrop_color_g);
		_config_write_int(h,"backdrop_color_b",config_backdrop_color_b);
			_config_write_int(h,"auto_zoom",config_auto_zoom);
			_config_write_int(h,"auto_zoom_type",config_auto_zoom_type);
			_config_write_int(h,"auto_fit_wide_mul",config_auto_fit_wide_mul);
			_config_write_int(h,"auto_fit_wide_div",config_auto_fit_wide_div);
			_config_write_int(h,"auto_fit_high_mul",config_auto_fit_high_mul);
			_config_write_int(h,"auto_fit_high_div",config_auto_fit_high_div);
			_config_write_int(h,"frame_minus",config_frame_minus);
			_config_write_int(h,"mouse_wheel_action",config_mouse_wheel_action);
			_config_write_int(h,"ctrl_mouse_wheel_action",config_ctrl_mouse_wheel_action);
			_config_write_int(h,"left_click_action",config_left_click_action);
			_config_write_int(h,"right_click_action",config_right_click_action);
			_config_write_int(h,"xbutton_action",config_xbutton_action);
			_config_write_int(h,"keep_centered",config_keep_centered);
			_config_write_int(h,"windowed_background_color_r",config_windowed_background_color_r);
			_config_write_int(h,"windowed_background_color_g",config_windowed_background_color_g);
			_config_write_int(h,"windowed_background_color_b",config_windowed_background_color_b);
			_config_write_int(h,"windowed_hide_cursor",config_windowed_hide_cursor);
			_config_write_int(h,"fullscreen_background_color_r",config_fullscreen_background_color_r);
			_config_write_int(h,"fullscreen_background_color_g",config_fullscreen_background_color_g);
			_config_write_int(h,"fullscreen_background_color_b",config_fullscreen_background_color_b);
			_config_write_int(h,"options_last_page",config_options_last_page);
			_config_write_int(h,"short_jump",config_short_jump);
			_config_write_int(h,"medium_jump",config_medium_jump);
			_config_write_int(h,"long_jump",config_long_jump);
			_config_write_int(h,"loop_animations_once",config_loop_animations_once);
			_config_write_int(h,"prevent_sleep",config_prevent_sleep);
			_config_write_int(h,"shuffle",config_shuffle);
			_config_write_int(h,"browse_file_open_dialog",config_browse_file_open_dialog);
			_config_write_int(h,"ontop",config_ontop);
			_config_write_int(h,"slideshow_custom_rate",config_slideshow_custom_rate);
			_config_write_int(h,"slideshow_custom_rate_type",config_slideshow_custom_rate_type);
			_config_write_int(h,"scroll_window",config_scroll_window);
			_config_write_int(h,"preload_next",config_preload_next);
			_config_write_int(h,"cache_last",config_cache_last);
			_config_write_int(h,"icm",config_icm);
			_config_write_int(h,"orientation",config_orientation);
			_config_write_int(h,"toolbar_move_window",config_toolbar_move_window);
			_config_write_int(h,"title_bar_format",config_title_bar_format);
			_config_write_int(h,"add_command_line_timeout",config_add_command_line_timeout);
				
				// save the recent files mru: filled slots carry paths, the rest
				// are written empty so removed entries do not survive a reload.
				{
					int i;
					
					for(i=0;i<CONFIG_RECENT_FILE_COUNT;i++)
					{
						utf8_t key_buf[32];
						
						_config_recent_key_name(key_buf,i);
						
						_config_write_string(h,key_buf,(i < config_recent_file_count) ? config_recent_files[i] : L"");
					}
				}
					
			// save keys
			{
				int i;
				
				for(i=0;i<viv_get_command_count();i++)
				{
					utf8_t key_buf[STRING_SIZE];
					
					if (viv_menu_name_to_ini_name(key_buf,i))
					{
						wchar_t keylist[STRING_SIZE];
						config_key_t *key;
						
						// get ini key name.
						
						key = viv_key_get_start(i);
						*keylist = 0;
						
						while(key)
						{
							wchar_t keyidbuf[STRING_SIZE];
							
							string_printf(keyidbuf,"%d",key->key);

							if (*keylist)							
							{
								string_cat_utf8(keylist,(const utf8_t *)",");
							}

							string_cat(keylist,keyidbuf);
						
							key = key->next;
						}

						_config_write_string(h,key_buf,keylist);
					}
				}
			}	

			// save context menu items
//			_config_write_int(h,"context_menu_items",);
		}

		CloseHandle(h);

		if (!MoveFileExW(tempname,filename,MOVEFILE_REPLACE_EXISTING))
		{
			if (CopyFile(tempname,filename,FALSE))
			{
				DeleteFile(tempname);
			}
		}
	}
}

void config_save_settings(int appdata)
{
	if (appdata)
	{
		wchar_t path[STRING_SIZE];

		if (string_get_appdata_path(path))
		{
			wchar_t appdata_wbuf[STRING_SIZE];
			
			string_path_combine_utf8(appdata_wbuf,path,(const utf8_t *)"voidImageViewer");
			
			CreateDirectory(appdata_wbuf,NULL);

			_config_save_settings_by_location(appdata_wbuf,0);
		}
	}
	else
	{
		wchar_t path[STRING_SIZE];
		
		string_get_exe_path(path);
		
		_config_save_settings_by_location(path,1);
	}
}
