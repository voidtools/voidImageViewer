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

static void _config_load_settings_by_location(const wchar_t *path,int is_root);
static void _config_write_int(HANDLE h,const char *ascii_key,int value);
static void _config_write_string(HANDLE h,const char *ascii_key,const wchar_t *s);
static void _config_write_utf8(HANDLE h,const utf8_t *s);
static void _config_save_settings_by_location(const wchar_t *path,int is_root);

BYTE config_appdata = 0; // store settings in %APPDATA%\voidimageviewer or in the same location as voidimageviewer.exe
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
BYTE config_nav_sort = CONFIG_NAV_SORT_DATE_MODIFIED; // current navigation sort.
BYTE config_nav_sort_ascending = 0; // sort navigation ascending or descending.
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


static void _config_write_int(HANDLE h,const char *ascii_key,int value)
{
	wchar_t wbuf[STRING_SIZE];
	
	string_format_number(wbuf,value);
	
	_config_write_string(h,ascii_key,wbuf);
}

static void _config_write_string(HANDLE h,const char *ascii_key,const wchar_t *s)
{
	utf8_t buf[STRING_SIZE*3];
	
	WideCharToMultiByte(CP_UTF8,0,s,-1,(char *)buf,STRING_SIZE*3,0,0);
	
	_config_write_utf8(h,(const utf8_t *)ascii_key);
	_config_write_utf8(h,(const utf8_t *)"=");
	_config_write_utf8(h,buf);
	_config_write_utf8(h,(const utf8_t *)"\r\n");
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
