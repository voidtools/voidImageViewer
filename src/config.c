
// Copyright (C) 2017 David Carpenter
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
// 
// See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// customizable settings

#include "viv.h"

static void _config_load_settings_by_location(const wchar_t *path,int is_root);
static void _config_write_int(HANDLE h,const char *ascii_key,int value);
static void _config_write_string(HANDLE h,const char *ascii_key,const wchar_t *s);
static void _config_write_utf8(HANDLE h,const utf8_t *s);
static void _config_save_settings_by_location(const wchar_t *path,int is_root);

int config_appdata = 0; // store settings in %APPDATA%\voidimageviewer or in the same location as voidimageviewer.exe
int config_keep_centered = 1; // when zooming out, don't recenter the image. (keep cursor under the same pixel)
int config_x = 0;
int config_y = 0;
int config_wide = 640;
int config_high = 480;
int config_slideshow_rate = 5000;
int config_allow_shrinking = 1; // prevent resizing an image below 100%
int config_shrink_blit_mode = CONFIG_SHRINK_BLIT_MODE_HALFTONE; // shrink filter
int config_mag_filter = CONFIG_MAG_FILTER_COLORONCOLOR; // magnify filter
int config_nav_sort = CONFIG_NAV_SORT_DATE_MODIFIED; // current navigation sort.
int config_nav_sort_ascending = 0; // sort navigation ascending or descending.
int config_keep_aspect_ratio = 1; // stretch images with the original aspect ratio.
int config_fill_window = 0; // stretch the image to fill the window
int config_fullscreen_fill_window = 1; // same as fill_window, except this setting is used when we are fullscreen
int config_auto_zoom = 0; // automatically resize the window to fit the newly loaded image
int config_auto_zoom_type = 1; // 0 = 50%, 1 = 100%, 2 = 200%
int config_frame_minus = 0; // show frame counter or remaining frames in status bar
int config_multiple_instances = 0; // all multiple instances or use a single instance.
int config_is_show_status = 1;
int config_is_show_controls = 1;
int config_allow_sleep = 1;
int config_loop_animations_once = 1;
int config_mouse_wheel_action = 0; // 0 = zoom, 1 = next/prev, 2=prev/next
int config_ctrl_mouse_wheel_action = 0; // 0 = zoom, 1 = next/prev, 2=prev/next
int config_left_click_action = 0; // 0 = scroll, 1 = play/pause slideshow, 2 = play/pause animation, 3=zoom in, 4=next, 5=1:1 scroll
int config_right_click_action = 0; // 0 = context menu, 1=zoom out, 2=prev, 
int config_windowed_background_color_r = 255;
int config_windowed_background_color_g = 255;
int config_windowed_background_color_b = 255;
int config_fullscreen_background_color_r = 0;
int config_fullscreen_background_color_g = 0;
int config_fullscreen_background_color_b = 0;
int config_options_last_page = 0;
int config_short_jump = 500;
int config_medium_jump = 1000;
int config_long_jump = 2000;
int config_shuffle = 0;
int config_browse_file_open_dialog = 1;
int config_ontop = 0; // 0 = never, 1 = always, 2 = while slideshow or animating.
int config_slideshow_custom_rate = 3; // custom slideshow rate (see type below)
int config_slideshow_custom_rate_type = 1; // 0 = milliseconds, 1 = seconds, 2 = minutes

static void _config_load_settings_by_location(const wchar_t *path,int is_root)
{
	ini_t *ini;
	wchar_t filename[STRING_SIZE];
	
	string_path_combine_utf8(filename,path,(const utf8_t *)"voidImageViewer.ini");
	
	ini = ini_open(filename,"voidImageViewer");
	if (ini)
	{
		ini_get_int(ini,(const utf8_t *)"x",&config_x);
		ini_get_int(ini,(const utf8_t *)"y",&config_y);
		ini_get_int(ini,(const utf8_t *)"wide",&config_wide);
		ini_get_int(ini,(const utf8_t *)"high",&config_high);
		ini_get_int(ini,(const utf8_t *)"slideshow_rate",&config_slideshow_rate);
		ini_get_int(ini,(const utf8_t *)"allow_shrinking",&config_allow_shrinking);
		ini_get_int(ini,(const utf8_t *)"shrink_blit_mode",&config_shrink_blit_mode);
		ini_get_int(ini,(const utf8_t *)"mag_filter",&config_mag_filter);
		ini_get_int(ini,(const utf8_t *)"keep_aspect_ratio",&config_keep_aspect_ratio);
		ini_get_int(ini,(const utf8_t *)"fill_window",&config_fill_window);
		ini_get_int(ini,(const utf8_t *)"fullscreen_fill_window",&config_fullscreen_fill_window);
		ini_get_int(ini,(const utf8_t *)"sort",&config_nav_sort);
		ini_get_int(ini,(const utf8_t *)"sort_ascending",&config_nav_sort_ascending);
		ini_get_int(ini,(const utf8_t *)"multiple_instances",&config_multiple_instances);
		ini_get_int(ini,(const utf8_t *)"show_status",&config_is_show_status);
		ini_get_int(ini,(const utf8_t *)"show_controls",&config_is_show_controls);
		ini_get_int(ini,(const utf8_t *)"auto_zoom",&config_auto_zoom);
		ini_get_int(ini,(const utf8_t *)"auto_zoom_type",&config_auto_zoom_type);
		ini_get_int(ini,(const utf8_t *)"frame_minus",&config_frame_minus);
		ini_get_int(ini,(const utf8_t *)"mouse_wheel_action",&config_mouse_wheel_action);
		ini_get_int(ini,(const utf8_t *)"ctrl_mouse_wheel_action",&config_ctrl_mouse_wheel_action);
		ini_get_int(ini,(const utf8_t *)"left_click_action",&config_left_click_action);
		ini_get_int(ini,(const utf8_t *)"right_click_action",&config_right_click_action);
		ini_get_int(ini,(const utf8_t *)"keep_centered",&config_keep_centered);
		ini_get_int(ini,(const utf8_t *)"windowed_background_color_r",&config_windowed_background_color_r);
		ini_get_int(ini,(const utf8_t *)"windowed_background_color_g",&config_windowed_background_color_g);
		ini_get_int(ini,(const utf8_t *)"windowed_background_color_b",&config_windowed_background_color_b);
		ini_get_int(ini,(const utf8_t *)"fullscreen_background_color_r",&config_fullscreen_background_color_r);
		ini_get_int(ini,(const utf8_t *)"fullscreen_background_color_g",&config_fullscreen_background_color_g);
		ini_get_int(ini,(const utf8_t *)"fullscreen_background_color_b",&config_fullscreen_background_color_b);
		ini_get_int(ini,(const utf8_t *)"options_last_page",&config_options_last_page);
		ini_get_int(ini,(const utf8_t *)"short_jump",&config_short_jump);
		ini_get_int(ini,(const utf8_t *)"medium_jump",&config_medium_jump);
		ini_get_int(ini,(const utf8_t *)"long_jump",&config_long_jump);
		ini_get_int(ini,(const utf8_t *)"loop_animations_once",&config_loop_animations_once);
		ini_get_int(ini,(const utf8_t *)"allow_sleep",&config_allow_sleep);
		ini_get_int(ini,(const utf8_t *)"shuffle",&config_shuffle);
		ini_get_int(ini,(const utf8_t *)"browse_file_open_dialog",&config_browse_file_open_dialog);
		ini_get_int(ini,(const utf8_t *)"ontop",&config_ontop);
		ini_get_int(ini,(const utf8_t *)"slideshow_custom_rate",&config_slideshow_custom_rate);
		ini_get_int(ini,(const utf8_t *)"slideshow_custom_rate_type",&config_slideshow_custom_rate_type);

		if (is_root)
		{
			ini_get_int(ini,(const utf8_t *)(const utf8_t *)"appdata",&config_appdata);
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
			_config_write_int(h,"show_status",config_is_show_status);
			_config_write_int(h,"show_controls",config_is_show_controls);
			_config_write_int(h,"auto_zoom",config_auto_zoom);
			_config_write_int(h,"auto_zoom_type",config_auto_zoom_type);
			_config_write_int(h,"frame_minus",config_frame_minus);
			_config_write_int(h,"mouse_wheel_action",config_mouse_wheel_action);
			_config_write_int(h,"ctrl_mouse_wheel_action",config_ctrl_mouse_wheel_action);
			_config_write_int(h,"left_click_action",config_left_click_action);
			_config_write_int(h,"right_click_action",config_right_click_action);
			_config_write_int(h,"keep_centered",config_keep_centered);
			_config_write_int(h,"windowed_background_color_r",config_windowed_background_color_r);
			_config_write_int(h,"windowed_background_color_g",config_windowed_background_color_g);
			_config_write_int(h,"windowed_background_color_b",config_windowed_background_color_b);
			_config_write_int(h,"fullscreen_background_color_r",config_fullscreen_background_color_r);
			_config_write_int(h,"fullscreen_background_color_g",config_fullscreen_background_color_g);
			_config_write_int(h,"fullscreen_background_color_b",config_fullscreen_background_color_b);
			_config_write_int(h,"options_last_page",config_options_last_page);
			_config_write_int(h,"short_jump",config_short_jump);
			_config_write_int(h,"medium_jump",config_medium_jump);
			_config_write_int(h,"long_jump",config_long_jump);
			_config_write_int(h,"loop_animations_once",config_loop_animations_once);
			_config_write_int(h,"allow_sleep",config_allow_sleep);
			_config_write_int(h,"shuffle",config_shuffle);
			_config_write_int(h,"browse_file_open_dialog",config_browse_file_open_dialog);
			_config_write_int(h,"ontop",config_ontop);
			_config_write_int(h,"slideshow_custom_rate",config_slideshow_custom_rate);
			_config_write_int(h,"slideshow_custom_rate_type",config_slideshow_custom_rate_type);
		
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
