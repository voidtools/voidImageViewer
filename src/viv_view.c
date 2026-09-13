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
// viv_view.c - command dispatch, navigation, zoom and mouse actions.
// Pure physical move from viv.c (R70 split): bodies are unchanged.
#include "viv.h"
#include "viv_state.h"
#include "viv_view.h"
#include "viv_anim.h"
#include "viv_chrome.h"
#include "viv_dialogs.h"
#include "viv_load.h"
#include "viv_playlist.h"
#include "viv_recent.h"
#include "viv_render.h"

// forward declarations (order preserved from viv.c)
void _viv_command(int command_id);
void _viv_command_with_is_key_repeat(int command_id,int is_key_repeat);
int _viv_next(int prev,int reset_slideshow_timer,int is_preload,int wait_for_current_load);
void _viv_home(int end,int is_preload);
void _viv_view_set(int view_x,int view_y,int invalidate);
void _viv_slideshow(void);
static void _viv_set_custom_rate(void);
static void _viv_set_rate(int rate);
static void _viv_delete(int permanently);
static void _viv_copy(int cut);
static void _viv_copy_filename(void);
static void _viv_copy_image(void);
static int _viv_is_key_state(int control,int shift,int alt);
void _viv_pause(void);
static void _viv_increase_rate(int dec);
static void _viv_file_preview(void);
static void _viv_file_print(void);
static void _viv_file_set_desktop_wallpaper(void);
static void _viv_edit_rotate(int counterclockwise);
static void _viv_file_edit(void);
static void _viv_open_file_location(void);
static void _viv_properties(void);
void _viv_mousemove(void);
void _viv_view_1to1(void);
void _viv_zoom_set_percent(int percent,int screen_x,int screen_y,int force);
void _viv_zoom_in(int out,int have_xy,int x,int y);
void _viv_view_scroll(int mx,int my);
void _viv_update_1to1_scroll(int x,int y);
void _viv_do_mousewheel_action(int action,int delta,int x,int y);
void _viv_do_left_click_action(int action);
void _viv_start_move_window(void);


static int _viv_old_zoom_pos = 0; // restore this zoom level after leaving 1:1 mode.
static WORD _viv_slideshow_rate_presets[] = {250,500,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000,20000,30000,40000,50000,60000};
typedef char _viv_slideshow_rate_presets_count_assert[(sizeof(_viv_slideshow_rate_presets) / sizeof(WORD) == _VIV_SLIDESHOW_RATE_PRESET_COUNT) ? 1 : -1]; // the count literal pins the table
void _viv_command(int command_id)
{
	_viv_command_with_is_key_repeat(command_id,0);
}
void _viv_command_with_is_key_repeat(int command_id,int is_key_repeat)
{
	// Parse the menu selections:
	// any user command counts as activity for the overlay idle timer.
	zoomui_activity();

	switch (command_id)
	{
		case VIV_ID_VIEW_ZOOM_IN:
			_viv_zoom_in(0,0,0,0);
			break;

		case VIV_ID_VIEW_ZOOM_OUT:
			_viv_zoom_in(1,0,0,0);
			break;

		case VIV_ID_VIEW_ZOOM_RESET:
			_viv_1to1 = 0;
			_viv_zoom_pos = 0;
			_viv_view_set(_viv_view_x,_viv_view_y,1);
			InvalidateRect(_viv_hwnd,0,FALSE);
			_viv_status_update_temp_pos_zoom();
			break;
	
		case VIV_ID_HELP_HELP:
			ShellExecuteA(_viv_hwnd,NULL,localization_get_string(LOCALIZATION_ID_HELP_SUPPORT_URL),NULL,NULL,SW_SHOWNORMAL);
			break;
			
		case VIV_ID_HELP_COMMAND_LINE_OPTIONS:
			_viv_command_line_options();
			break;
			
		case VIV_ID_HELP_DONATE:
			ShellExecuteA(_viv_hwnd,NULL,localization_get_string(LOCALIZATION_ID_HELP_DONATE_URL),NULL,NULL,SW_SHOWNORMAL);
			break;
			
		case VIV_ID_HELP_ABOUT:
			DialogBox(os_hinstance,MAKEINTRESOURCE(IDD_ABOUT),_viv_hwnd,_viv_about_proc);
			break;
			
		case VIV_ID_HELP_WEBSITE:
			ShellExecuteA(_viv_hwnd,NULL,localization_get_string(LOCALIZATION_ID_HELP_WEBSITE_URL),NULL,NULL,SW_SHOWNORMAL);
			break;
			
		case VIV_ID_FILE_EXIT:
			_viv_exit();
			break;
		
		case VIV_ID_FILE_RECENT_CLEAR:
			_viv_recent_file_clear();
			break;
		
		case VIV_ID_FILE_RECENT_0:
		case VIV_ID_FILE_RECENT_1:
		case VIV_ID_FILE_RECENT_2:
		case VIV_ID_FILE_RECENT_3:
		case VIV_ID_FILE_RECENT_4:
		case VIV_ID_FILE_RECENT_5:
		case VIV_ID_FILE_RECENT_6:
		case VIV_ID_FILE_RECENT_7:
		case VIV_ID_FILE_RECENT_8:
		case VIV_ID_FILE_RECENT_9:
			{
				int recent_index;
				
				recent_index = command_id - VIV_ID_FILE_RECENT_0;
				
				if ((recent_index >= 0) && (recent_index < config_recent_file_count))
				{
					if (_viv_random)
					{
						mem_free(_viv_random);
						
						_viv_random = 0;
					}
					
					_viv_playlist_clearall();
					
					if (!_viv_open_from_filename(config_recent_files[recent_index]))
					{
						// the file is gone: drop the stale mru entry.
						_viv_recent_file_remove(recent_index);
					}
				}
			}
			break;
		
		case VIV_ID_NAV_PREV:
			_viv_next(1,1,0,is_key_repeat);
			break;
		
		case VIV_ID_NAV_NEXT:
			_viv_next(0,1,0,is_key_repeat);
			break;
		
		case VIV_ID_NAV_HOME:
			_viv_home(0,0);
			break;
		
		case VIV_ID_NAV_END:
			_viv_home(1,0);
			break;
			
		case VIV_ID_NAV_SHUFFLE:
			config_shuffle = !config_shuffle;
			
			if (!config_shuffle)
			{
				// create a new shuffle list.
				if (_viv_playlist_shuffle_indexes)
				{
					mem_free(_viv_playlist_shuffle_indexes);

					_viv_playlist_shuffle_indexes = 0;
					_viv_playlist_shuffle_allocated = 0;
				}
			}
			
			// clear preload
			_viv_clear_loading_preload();
			_viv_clear_preload();
			_viv_clear_last();
			
			// preload again.
			if (!_viv_load_image_thread)
			{
				_viv_preload_next();
			}
			break;
		
		case VIV_ID_NAV_SORT_NAME:
		case VIV_ID_NAV_SORT_SIZE:
		case VIV_ID_NAV_SORT_DATE_MODIFIED:
		case VIV_ID_NAV_SORT_DATE_CREATED:
		case VIV_ID_NAV_SORT_FULL_PATH:

			if (config_nav_sort == command_id - VIV_ID_NAV_SORT_NAME)
			{
				config_nav_sort_ascending = !config_nav_sort_ascending;
			}
			else
			{
				switch(command_id)
				{
					case VIV_ID_NAV_SORT_NAME:
						config_nav_sort = CONFIG_NAV_SORT_NAME;
						config_nav_sort_ascending = 1;
						break;

					case VIV_ID_NAV_SORT_FULL_PATH:
						config_nav_sort = CONFIG_NAV_SORT_FULL_PATH_AND_FILENAME;
						config_nav_sort_ascending = 1;
						break;
						
					case VIV_ID_NAV_SORT_DATE_MODIFIED:
						config_nav_sort = CONFIG_NAV_SORT_DATE_MODIFIED;
						config_nav_sort_ascending = 0;
						break;
						
					case VIV_ID_NAV_SORT_DATE_CREATED:
						config_nav_sort = CONFIG_NAV_SORT_DATE_CREATED;
						config_nav_sort_ascending = 0;
						break;
						
					case VIV_ID_NAV_SORT_SIZE:
						config_nav_sort = CONFIG_NAV_SORT_SIZE;
						config_nav_sort_ascending = 0;
						break;
				}
			}
			
			// clear preload and last
			_viv_clear_loading_preload();
			_viv_clear_preload();
			_viv_clear_last();
			
			break;
		
		case VIV_ID_NAV_SORT_ASCENDING:
			config_nav_sort_ascending = 1;
			
			// clear preload and last
			_viv_clear_loading_preload();
			_viv_clear_preload();
			_viv_clear_last();
			
			break;
			
		case VIV_ID_NAV_SORT_DESCENDING:
			config_nav_sort_ascending = 0;
			
			// clear preload and last
			_viv_clear_loading_preload();
			_viv_clear_preload();
			_viv_clear_last();
			
			break;
		
		case VIV_ID_SLIDESHOW_PLAY_ONLY:
			if (!_viv_is_slideshow)
			{
				_viv_pause();
			}
			break;

		case VIV_ID_SLIDESHOW_PAUSE_ONLY:
			if (_viv_is_slideshow)
			{
				_viv_pause();
			}
			break;
		
		case VIV_ID_SLIDESHOW_STOP:
			if (_viv_is_slideshow)
			{
				_viv_pause();
			}
			break;

		case VIV_ID_SLIDESHOW_PAUSE:
			_viv_pause();
			break;
			
		case VIV_ID_SLIDESHOW_RATE_DEC:
			if (!_viv_is_slideshow)
			{
				// up and down navigate when no slideshow is running: a user
				// pressing down on a still image expects the next picture, not
				// a silent rate change they cannot see anywhere.
				_viv_next(0,1,0,is_key_repeat);
			}
			else
			{
				_viv_increase_rate(1);
			}
			break;
			
		case VIV_ID_SLIDESHOW_RATE_INC:
			if (!_viv_is_slideshow)
			{
				// see the rate_dec note: the still-image fallback navigates.
				_viv_next(1,1,0,is_key_repeat);
			}
			else
			{
				_viv_increase_rate(0);
			}
			break;

		case VIV_ID_ANIMATION_PLAY_PAUSE:
			_viv_animation_pause();
			break;

		case VIV_ID_ANIMATION_JUMP_FORWARD_MEDIUM:
			_viv_frame_skip(config_medium_jump);
			break;

		case VIV_ID_ANIMATION_JUMP_BACKWARD_MEDIUM:
			_viv_frame_skip(-config_medium_jump);
			break;

		case VIV_ID_ANIMATION_JUMP_FORWARD_SHORT:
			_viv_frame_skip(config_short_jump);
			break;

		case VIV_ID_ANIMATION_JUMP_BACKWARD_SHORT:
			_viv_frame_skip(-config_short_jump);
			break;

		case VIV_ID_ANIMATION_JUMP_FORWARD_LONG:
			_viv_frame_skip(config_long_jump);
			break;

		case VIV_ID_ANIMATION_JUMP_BACKWARD_LONG:
			_viv_frame_skip(-config_long_jump);
			break;

		case VIV_ID_ANIMATION_FRAME_STEP:
			_viv_frame_step();
			break;

		case VIV_ID_ANIMATION_FRAME_PREV:
			_viv_frame_prev();
			break;

		case VIV_ID_ANIMATION_FRAME_HOME:
			_viv_frame_looped = 0;
			
			if (_viv_animation_play)
			{
				_viv_animation_play = 0;
			}

			if (_viv_frame_count > 1)			
			{
				_viv_frame_position = 0;
				_viv_animation_timer_tick_start = os_get_tick_count();
				_viv_timer_tick = 0;

				_viv_update_src_pixel(1,0);
				_viv_status_update();
					
				InvalidateRect(_viv_hwnd,NULL,FALSE);
				UpdateWindow(_viv_hwnd);
			}
			
			// rc.13: this frame jump pauses the animation clock - the play
			// face and the on-top window follow it.
			_viv_toolbar_update_buttons();
			
			_viv_update_ontop();
			break;

		case VIV_ID_ANIMATION_FRAME_END:

			_viv_frame_looped = 0;
			
			if (_viv_animation_play)
			{
				_viv_animation_play = 0;
			}

			if (_viv_frame_count > 1)
			{
				_viv_frame_position = _viv_frame_loaded_count - 1;
				_viv_animation_timer_tick_start = os_get_tick_count();
				_viv_timer_tick = 0;

				_viv_update_src_pixel(1,0);
				_viv_status_update();
					
				InvalidateRect(_viv_hwnd,NULL,FALSE);
				UpdateWindow(_viv_hwnd);
			}
			
			// rc.13: this frame jump pauses the animation clock - the play
			// face and the on-top window follow it.
			_viv_toolbar_update_buttons();
			
			_viv_update_ontop();
			break;

		case VIV_ID_ANIMATION_RATE_DEC:
			_viv_increase_animation_rate(1);
			break;
			
		case VIV_ID_ANIMATION_RATE_INC:
			_viv_increase_animation_rate(0);
			break;

		case VIV_ID_ANIMATION_RATE_RESET:
			_viv_reset_animation_rate();
			break;

		case VIV_ID_SLIDESHOW_RATE_250: _viv_set_rate(250); break;
		case VIV_ID_SLIDESHOW_RATE_500: _viv_set_rate(500); break;
		case VIV_ID_SLIDESHOW_RATE_1000: _viv_set_rate(1000); break;
		case VIV_ID_SLIDESHOW_RATE_2000: _viv_set_rate(2000); break;
		case VIV_ID_SLIDESHOW_RATE_3000: _viv_set_rate(3000); break;
		case VIV_ID_SLIDESHOW_RATE_4000: _viv_set_rate(4000); break;
		case VIV_ID_SLIDESHOW_RATE_5000: _viv_set_rate(5000); break;
		case VIV_ID_SLIDESHOW_RATE_6000: _viv_set_rate(6000); break;
		case VIV_ID_SLIDESHOW_RATE_7000: _viv_set_rate(7000); break;
		case VIV_ID_SLIDESHOW_RATE_8000: _viv_set_rate(8000); break;
		case VIV_ID_SLIDESHOW_RATE_9000: _viv_set_rate(9000); break;
		case VIV_ID_SLIDESHOW_RATE_10000: _viv_set_rate(10000); break;
		case VIV_ID_SLIDESHOW_RATE_20000: _viv_set_rate(20000); break;
		case VIV_ID_SLIDESHOW_RATE_30000: _viv_set_rate(30000); break;
		case VIV_ID_SLIDESHOW_RATE_40000: _viv_set_rate(40000); break;
		case VIV_ID_SLIDESHOW_RATE_50000: _viv_set_rate(50000); break;
		case VIV_ID_SLIDESHOW_RATE_60000: _viv_set_rate(60000); break;
		case VIV_ID_SLIDESHOW_RATE_CUSTOM: _viv_set_custom_rate(); break;
			
		case VIV_ID_VIEW_CAPTION:
			config_show_caption = !config_show_caption;
			_viv_update_frame();
			break;
			
		case VIV_ID_VIEW_THICKFRAME:
			config_show_thickframe = !config_show_thickframe;
			_viv_update_frame();
			break;
			
		case VIV_ID_VIEW_MENU:
			config_show_menu = !config_show_menu;
			_viv_update_frame();
			break;
			
		case VIV_ID_VIEW_STATUS:
			config_show_status = !config_show_status;
			_viv_update_frame();
			break;
			
		case VIV_ID_VIEW_CONTROLS:
			config_show_controls = !config_show_controls;
			_viv_update_frame();
			break;

		case VIV_ID_VIEW_ZOOM_CONTROLS:
			config_show_zoom_controls = !config_show_zoom_controls;
			_viv_zoomui_update();
			break;
			
		case VIV_ID_VIEW_ZOOM_AUTO_HIDE:
			config_zoom_auto_hide = !config_zoom_auto_hide;
			_viv_zoomui_update();
			break;
			
		case VIV_ID_VIEW_WINDOWED_BACKGROUND_COLOR:
		{
			COLORREF background_color;
			
			// the canvas color the image floats on (the transparency backdrop
			// is a separate setting). one picker next to the backdrop submenu:
			// the field report kept looking for this color under the backdrop
			// label and found nothing but the options page.
			background_color = RGB(config_windowed_background_color_r,config_windowed_background_color_g,config_windowed_background_color_b);
			
			if (os_choose_color(_viv_hwnd,&background_color))
			{
				config_windowed_background_color_r = GetRValue(background_color);
				config_windowed_background_color_g = GetGValue(background_color);
				config_windowed_background_color_b = GetBValue(background_color);
				
				// the mat, the win11 caption tint and any follow mode backdrop
				// all read this color: re-tint the frame, repaint the canvas and
				// reload the open image so the transparency under it picks the new
				// mat up immediately (the refresh is a no-op without an open file
				// - no load error on the blank window).
				os_window_modern_chrome(_viv_hwnd,_viv_windowed_background());
				
				InvalidateRect(_viv_hwnd,0,FALSE);
				_viv_refresh();
			}
			
			break;
		}
		
		case VIV_ID_VIEW_BACKDROP_FOLLOW:
			config_backdrop_mode = CONFIG_BACKDROP_MODE_FOLLOW;
			_viv_backdrop_apply();
			break;
			
		case VIV_ID_VIEW_BACKDROP_BLACK:
			config_backdrop_mode = CONFIG_BACKDROP_MODE_BLACK;
			_viv_backdrop_apply();
			break;
			
		case VIV_ID_VIEW_BACKDROP_WHITE:
			config_backdrop_mode = CONFIG_BACKDROP_MODE_WHITE;
			_viv_backdrop_apply();
			break;
			
		case VIV_ID_VIEW_BACKDROP_CUSTOM:
		{
			COLORREF backdrop_color;
			
			backdrop_color = RGB(config_backdrop_color_r,config_backdrop_color_g,config_backdrop_color_b);
			
			if (os_choose_color(_viv_hwnd,&backdrop_color))
			{
				config_backdrop_color_r = GetRValue(backdrop_color);
				config_backdrop_color_g = GetGValue(backdrop_color);
				config_backdrop_color_b = GetBValue(backdrop_color);
				config_backdrop_mode = CONFIG_BACKDROP_MODE_CUSTOM;
				
				_viv_backdrop_apply();
			}
			
			break;
		}
		
		case VIV_ID_VIEW_BACKDROP_CHECKERBOARD:
			config_backdrop_mode = CONFIG_BACKDROP_MODE_CHECKERBOARD;
			_viv_backdrop_apply();
			break;
			
		case VIV_ID_VIEW_PRESET_1:
			config_show_menu = 0;
			config_show_status = 0;
			config_show_controls = 0;
			config_show_caption = 0;
			config_show_thickframe = 0;
			_viv_update_frame();
			break;

		case VIV_ID_VIEW_PRESET_2:
			config_show_menu = 0;
			config_show_status = 0;
			config_show_controls = 0;
			config_show_caption = 0;
			config_show_thickframe = 1;
			_viv_update_frame();
			break;

		case VIV_ID_VIEW_PRESET_3:
			config_show_menu = 1;
			config_show_status = 1;
			config_show_controls = 1;
			config_show_caption = 1;
			config_show_thickframe = 1;
			_viv_update_frame();
			break;
			
		case VIV_ID_VIEW_REFRESH:
			_viv_refresh();
			break;
			
		case VIV_ID_VIEW_ALLOW_SHRINKING:
			_viv_1to1 = 0;
			config_allow_shrinking = !config_allow_shrinking;
			
			_viv_on_size();
			InvalidateRect(_viv_hwnd,0,FALSE);

			break;
			
		case VIV_ID_VIEW_KEEP_ASPECT_RATIO:
			_viv_1to1 = 0;
			config_keep_aspect_ratio = !config_keep_aspect_ratio;
			
			_viv_on_size();
			InvalidateRect(_viv_hwnd,0,FALSE);
			break;
			
		case VIV_ID_VIEW_FILL_WINDOW:
		
			_viv_1to1 = 0;
			if (_viv_is_fullscreen)
			{
				config_fullscreen_fill_window = !config_fullscreen_fill_window;
			}
			else
			{
				config_fill_window = !config_fill_window;
			}
			
			_viv_on_size();
			InvalidateRect(_viv_hwnd,0,FALSE);
			break;
		
		case VIV_ID_VIEW_FULLSCREEN:

			_viv_toggle_fullscreen();
			break;		
			
		case VIV_ID_VIEW_1TO1:
			_viv_view_1to1();
			break;		
			
		case VIV_ID_VIEW_BESTFIT:
			_viv_zoom_pos = 0;
			_viv_1to1 = 0;
			_viv_view_set(0,0,1);
			InvalidateRect(_viv_hwnd,0,FALSE);
			_viv_status_update_temp_pos_zoom();
			break;
			
		case VIV_ID_VIEW_SLIDESHOW:

			_viv_slideshow();
						
			break;
			
		case VIV_ID_VIEW_WINDOW_SIZE_50:
		case VIV_ID_VIEW_WINDOW_SIZE_100:
		case VIV_ID_VIEW_WINDOW_SIZE_200:
		case VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT:
			if (_viv_is_fullscreen)
			{
				_viv_toggle_fullscreen();
			}
		
			// get out of fullscreen mode.
			if (_viv_is_window_maximized(_viv_hwnd))
			{
				ShowWindow(_viv_hwnd,SW_RESTORE);
			}
		
			if (((_viv_image_wide) && (_viv_image_high)) || (command_id == VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT))
			{
				RECT rect;
				int wide;
				int high;
				int client_wide;
				int client_high;
				int midx;
				int midy;
				RECT monitor_rect;
				RECT window_rect;
				RECT client_rect;

				
				GetWindowRect(_viv_hwnd,&rect);
				midx = (rect.left + rect.right) / 2;
				midy = (rect.top + rect.bottom) / 2;
				
				rect.left = 0;
				rect.top = 0;
				
				switch(command_id)
				{
					case VIV_ID_VIEW_WINDOW_SIZE_50:
						rect.right = _viv_image_wide / 2;
						rect.bottom = _viv_image_high / 2;
						break;
						
					case VIV_ID_VIEW_WINDOW_SIZE_100:
						rect.right = _viv_image_wide;
						rect.bottom = _viv_image_high;
						break;
						
					case VIV_ID_VIEW_WINDOW_SIZE_200:
						rect.right = _viv_image_wide * 2;
						rect.bottom = _viv_image_high * 2;
						break;
						
					case VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT:
					
						// use full screen to calculate auto-fit size.
						os_MonitorRectFromWindow(_viv_hwnd,1,&monitor_rect);
						
						if (config_auto_fit_wide_div)
						{
							rect.right = ((monitor_rect.right - monitor_rect.left) * config_auto_fit_wide_mul) / config_auto_fit_wide_div;
						}

						if (config_auto_fit_high_div)
						{
							rect.bottom = ((monitor_rect.bottom - monitor_rect.top) * config_auto_fit_high_mul) / config_auto_fit_high_div;
						}
						
						break;
				}
				
				client_wide = rect.right - rect.left;
				client_high = rect.bottom - rect.top + _viv_get_status_high() + _viv_get_view_top();
				
				AdjustWindowRect(&rect,GetWindowStyle(_viv_hwnd),GetMenu(_viv_hwnd) ? TRUE : FALSE);

debug_printf("%d %d | %d %d %d %d\n",midx,midy,rect.left,rect.top,rect.right,rect.bottom);
				
				wide = rect.right - rect.left;
				high = rect.bottom - rect.top + _viv_get_status_high() + _viv_get_view_top();
				
				os_MonitorRectFromWindow(_viv_hwnd,0,&monitor_rect);
				
				if (wide > monitor_rect.right - monitor_rect.left)
				{
					wide = monitor_rect.right - monitor_rect.left;
				}
				
				if (high > monitor_rect.bottom - monitor_rect.top)
				{
					high = monitor_rect.bottom - monitor_rect.top;
				}
				
				rect.left = midx - (wide / 2);
				rect.top = midy - (high / 2);
				rect.right = midx - (wide / 2) + wide;
				rect.bottom = midy - (high / 2) + high;
				
				os_make_rect_completely_visible(_viv_hwnd,&rect);
				
debug_printf("SWP %d %d %d %d\n",rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top);

				// filling the window with White before SetWindowPos prevents garbage from showing.
				// the white flash is worse than the garbage.
				SetWindowPos(_viv_hwnd,0,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOCOPYBITS);

				// re-get the window size.
				GetWindowRect(_viv_hwnd,&window_rect);
				
				if ((window_rect.right - window_rect.left > wide) || (window_rect.bottom - window_rect.top > high))
				{
					// window is larger than requested.
					// reposition with new width/height.
					wide = window_rect.right - window_rect.left;
					high = window_rect.bottom - window_rect.top;
					
					rect.left = midx - (wide / 2);
					rect.top = midy - (high / 2);
					rect.right = midx - (wide / 2) + wide;
					rect.bottom = midy - (high / 2) + high;
					
					os_make_rect_completely_visible(_viv_hwnd,&rect);
					
	debug_printf("SWP2 %d %d %d %d\n",rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top)	;
					
					SetWindowPos(_viv_hwnd,0,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOCOPYBITS);
					
					// re-get the window size.
					GetWindowRect(_viv_hwnd,&window_rect);
				}

				// re-get the client rect.
				
				GetClientRect(_viv_hwnd,&client_rect);
				
				if ((client_wide > client_rect.right - client_rect.left) || (client_high > client_rect.bottom - client_rect.top))
				{
					// client rect is too small, adjust...
					wide = (window_rect.right - window_rect.left);
					high = (window_rect.bottom - window_rect.top);
					
					if (client_wide > client_rect.right - client_rect.left)
					{
						wide += client_wide - (client_rect.right - client_rect.left);
					}
					
					if (client_high > client_rect.bottom - client_rect.top)
					{
						high += client_high - (client_rect.bottom - client_rect.top);
					}
					
					rect.left = midx - (wide / 2);
					rect.top = midy - (high / 2);
					rect.right = midx - (wide / 2) + wide;
					rect.bottom = midy - (high / 2) + high;
					
					os_make_rect_completely_visible(_viv_hwnd,&rect);

					// avoid the 2nd SetWindowPos call if nothing changes.
					if ((rect.left != window_rect.left) || (rect.top != window_rect.top) || (rect.right != window_rect.right) || (rect.bottom != window_rect.bottom))
					{
	//SWP3 0 0 3174 5113					
		debug_printf("SWP3 %d %d %d %d\n",rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top)								;
						
						SetWindowPos(_viv_hwnd,0,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOCOPYBITS);					
					}
				}
			}	
			break;
			
		case VIV_ID_VIEW_ONTOP_ALWAYS:
			config_ontop = !config_ontop;
			_viv_update_ontop();
			break;
						
		case VIV_ID_VIEW_ONTOP_WHILE_PLAYING_OR_ANIMATING:
			config_ontop = 2;
			_viv_update_ontop();
			break;
						
		case VIV_ID_VIEW_ONTOP_NEVER:
			config_ontop = 0;
			_viv_update_ontop();
			break;
						
		case VIV_ID_VIEW_OPTIONS:
			// the remake settings window (the modal options dialog is retired
			// from the command path; its page code stays as the reference).
			_viv_settings_show();
			break;
			
		case VIV_ID_EDIT_COPY:
			_viv_copy(0);
			break;

		case VIV_ID_EDIT_COPY_FILENAME:
			_viv_copy_filename();
			break;

		case VIV_ID_EDIT_COPY_IMAGE:
			_viv_copy_image();
			break;

		case VIV_ID_EDIT_PASTE:
			SendMessage(_viv_hwnd,WM_PASTE,0,0);
			break;

		case VIV_ID_EDIT_CUT:
			_viv_copy(1);
			break;
			
		case VIV_ID_FILE_OPEN_FILE:
		case VIV_ID_FILE_ADD_FILE:
			{
				OPENFILENAME ofn;
				wchar_t tobuf[STRING_SIZE+1];
				wchar_t filter_wbuf[STRING_SIZE];
				wchar_t title_wbuf[STRING_SIZE];
				
				os_zero_memory(&ofn,sizeof(OPENFILENAME));

				string_copy(tobuf,_viv_last_open_file ? _viv_last_open_file : L"");
				
				string_printf(filter_wbuf,"%s (*.bmp;*.gif;*.ico;*.jpeg;*.jpg;*.png;*.tif;*.tiff;*.webp;*.emf;*.wmf)%c*.bmp;*.gif;*.ico;*.jpeg;*.jpg;*.png;*.tif;*.tiff;*.webp;*.emf;*.wmf%c%s (*.*)%c*.*%c",localization_get_string(LOCALIZATION_ID_OPEN_ALL_IMAGE_FILES),0,0,localization_get_string(LOCALIZATION_ID_OPEN_ALL_FILES),0,0);

				string_copy_utf8_string(title_wbuf,localization_get_string(LOCALIZATION_ID_OPEN_IMAGE_CAPTION));
				
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = _viv_hwnd;
				ofn.hInstance = os_hinstance;
				ofn.lpstrFilter = filter_wbuf;
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = tobuf;
				ofn.nMaxFile = STRING_SIZE;
				ofn.lpstrTitle = title_wbuf;
				ofn.Flags = OFN_ENABLESIZING | OFN_NOCHANGEDIR;
				
				if (GetOpenFileName(&ofn))
				{
					if (_viv_random)
					{
						mem_free(_viv_random);
						
						_viv_random = 0;
					}
					
					if (command_id == VIV_ID_FILE_OPEN_FILE)
					{
						_viv_playlist_clearall();
	
						_viv_open_from_filename(ofn.lpstrFile);
					}
					else
					{
						// add current?
						_viv_playlist_add_current_if_empty();
					
						_viv_playlist_add_filename(ofn.lpstrFile);
					}
					
					if (_viv_last_open_file)
					{
						mem_free(_viv_last_open_file);
					}
					
					_viv_last_open_file = string_alloc(ofn.lpstrFile);
				}
			}
			break;
			
		case VIV_ID_FILE_OPEN_FOLDER:
		case VIV_ID_FILE_ADD_FOLDER:
			
			{
				wchar_t filename[STRING_SIZE];
				
				string_copy(filename,_viv_last_open_folder ? _viv_last_open_folder : L"");
				
				if (os_browse_for_folder(_viv_hwnd,filename))
				{
					if (_viv_random)
					{
						mem_free(_viv_random);
						
						_viv_random = 0;
					}
					
					if (command_id == VIV_ID_FILE_OPEN_FOLDER)
					{
						_viv_playlist_clearall();
					}
					else
					{
						_viv_playlist_add_current_if_empty();
					}
					
					_viv_playlist_add_path(filename);

					if (_viv_last_open_folder)
					{
						mem_free(_viv_last_open_folder);
					}
					
					_viv_last_open_folder = string_alloc(filename);

					if (command_id == VIV_ID_FILE_OPEN_FOLDER)
					{
						_viv_home(0,0);
					}
				}
			}
			
			break;
			
		case VIV_ID_FILE_DELETE:
			_viv_delete((GetKeyState(VK_SHIFT) < 0) ? 1 : 0);
			break;

		case VIV_ID_FILE_DELETE_RECYCLE:
			_viv_delete(0);
			break;

		case VIV_ID_FILE_DELETE_PERMANENTLY:
			_viv_delete(1);
			break;

		case VIV_ID_FILE_RENAME:
			_viv_rename();
			break;
			
		case VIV_ID_NAV_JUMPTO:
			_viv_show_jumpto();
			break;
			
		case VIV_ID_FILE_PREVIEW:
			_viv_file_preview();
			break;

		case VIV_ID_FILE_PRINT:
			_viv_file_print();
			break;

		case VIV_ID_FILE_SET_DESKTOP_WALLPAPER:
			_viv_file_set_desktop_wallpaper();
			break;
			
		case VIV_ID_EDIT_ROTATE_270:
			_viv_edit_rotate(1);
			break;
			
		case VIV_ID_EDIT_ROTATE_90:
			_viv_edit_rotate(0);
			break;
			
		case VIV_ID_FILE_CLOSE:
			_viv_blank();
			break;
	
		case VIV_ID_FILE_EDIT:
			_viv_file_edit();
			break;

		case VIV_ID_FILE_OPEN_FILE_LOCATION:
			_viv_open_file_location();
			break;
			
		case VIV_ID_FILE_SAVE_AS:
			_viv_save_image_as();
			break;
			
		case VIV_ID_FILE_OPEN_EVERYTHING_SEARCH:	
			_viv_search_everything(0);
			break;
			
		case VIV_ID_FILE_ADD_EVERYTHING_SEARCH:	
			_viv_search_everything(1);
			break;
			
		case VIV_ID_FILE_PROPERTIES:
			_viv_properties();
			break;
			
		case VIV_ID_EDIT_COPY_TO:
		case VIV_ID_EDIT_MOVE_TO:
		{
			if (*_viv_current_fd->cFileName)
			{
				OPENFILENAME ofn;
				wchar_t tobuf[STRING_SIZE+1];
				wchar_t filter_wbuf[STRING_SIZE];
				wchar_t title_wbuf[STRING_SIZE];
				
				os_zero_memory(&ofn,sizeof(OPENFILENAME));
				
				string_copy(tobuf,_viv_current_fd->cFileName);

				string_printf(filter_wbuf,"%s (*.*)%c*.*%c",localization_get_string(LOCALIZATION_ID_OPEN_ALL_FILES),0,0);

				string_copy_utf8_string(title_wbuf,localization_get_string(command_id == VIV_ID_EDIT_COPY_TO ? LOCALIZATION_ID_COPY_TO_CAPTION : LOCALIZATION_ID_MOVE_TO_CAPTION));
				
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = _viv_hwnd;
				ofn.hInstance = os_hinstance;
				ofn.lpstrFilter = filter_wbuf;
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = tobuf;
				ofn.nMaxFile = STRING_SIZE;
				ofn.lpstrTitle = title_wbuf;
				ofn.Flags = OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
				
				if (GetSaveFileName(&ofn))
				{	
					SHFILEOPSTRUCT shfileop;
					wchar_t frombuf[STRING_SIZE+1];
					
					tobuf[string_get_length(tobuf) + 1] = 0;
					
					os_zero_memory(&shfileop,sizeof(SHFILEOPSTRUCT));
					
					string_copy(frombuf,_viv_current_fd->cFileName);
					frombuf[string_get_length(frombuf) + 1] = 0;
					
					shfileop.hwnd = _viv_hwnd;
					shfileop.wFunc = command_id == VIV_ID_EDIT_COPY_TO ? FO_COPY : FO_MOVE;
					shfileop.pFrom = frombuf;
					shfileop.pTo = tobuf;
					shfileop.fFlags = FOF_ALLOWUNDO;
					
					SHFileOperation(&shfileop);
				}
			}
				
			break;
		}
	}
}
int _viv_next(int prev,int reset_slideshow_timer,int is_preload,int wait_for_current_load)
{
	int ret;

	ret = 1;
	
debug_printf("_viv_next %d %d\n",prev,is_preload);

	if (is_preload)
	{
		if (_viv_random)
		{
			// preloading not supported..
			return 0;
		}
	}
	else
	{
		_viv_last_is_prev = prev;
	}

//TODO: review -when enabled, viv fills unresponsive/sluggish.
// when disabled, viv can load multiple images when the 'up' key is released.
//wait_for_current_load = 0;

// NEVER use the preload, always go to disk to find the next real image.
// users expects the next image. Not the one we preloaded ages ago.
// 99% it will be the preloaded image anyway..

	if ((!is_preload) && (_viv_load_image_thread) && ((_viv_load_frame_count <= 1) || (_viv_load_image_terminate)) && (wait_for_current_load))
	{
		//_viv_open_preload();
		
		// still loading.
		// wait for current load to finish.
		return 0;
	}
	else
	if (_viv_random)
	{
		_viv_send_random_everything_search();
	}
	else
	{
		_viv_do_initial_shuffle();

		if (*_viv_current_fd->cFileName)
		{
			WIN32_FIND_DATA start_fd;
			WIN32_FIND_DATA best_fd;
			int got_start;
			int got_best;

			got_best = 0;
			got_start = 0;
			
			if (_viv_playlist_start)
			{
				_viv_playlist_t *d;
				
				d = _viv_playlist_start;
				
debug_printf("next %d\n",config_shuffle);

				if (config_shuffle)
				{
					int index;
					
					index = _viv_playlist_shuffle_index_from_fd(_viv_current_fd);
					
	debug_printf("cur %d %d\n",index,_viv_current_fd->dwReserved1);
					
					if (index != -1)
					{
						if (prev)
						{
							index--;
							if (index < 0)
							{
								index = _viv_playlist_count - 1;
							}
						}
						else
						{
							index++;
							if (index >= _viv_playlist_count)
							{
								index = 0;
							}
						}
					}
					else
					{
						if (prev)
						{
							index = _viv_playlist_count - 1;
						}
						else
						{
							index = 0;
						}
					}
					
					os_copy_memory(&best_fd,&_viv_playlist_shuffle_indexes[index]->fd,sizeof(WIN32_FIND_DATA));

					got_best = 1;
				}
				else
				{
					_viv_playlist_t *current_d;
					
					current_d = _viv_playlist_from_fd(_viv_current_fd);
	debug_printf("cur %d\n",current_d);
					
					while(d)
					{
						if (d != current_d)
						{
							int compare_ret;
							
							compare_ret = _viv_fd_compare(&d->fd,_viv_current_fd);

							if (compare_ret != 0)
							{
								if (prev)
								{
									if (compare_ret < 0)
									{
										if ((!got_best) || (_viv_fd_compare(&d->fd,&best_fd) > 0))
										{
											os_copy_memory(&best_fd,&d->fd,sizeof(WIN32_FIND_DATA));
											got_best = 1;
										}
									}
								}
								else
								{
									if (compare_ret > 0)
									{
										if ((!got_best) || (_viv_fd_compare(&d->fd,&best_fd) < 0))
										{
											os_copy_memory(&best_fd,&d->fd,sizeof(WIN32_FIND_DATA));
											got_best = 1;
										}
									}		
								}
							}
							// the wrap target is only needed when the primary direction
							// found no candidate (the current image sits at the end of the
							// sorted view). tracking it inside this scan costs one collation
							// compare per file on every navigation (the natural name sort
							// calls CompareString); the deferred second pass after the scan
							// only runs in the wrap case.
							
						}
					
						d = d->next;
					}
					if (!got_best)
					{
						d = _viv_playlist_start;
						
						while(d)
						{
							if (d != current_d)
							{
								if (prev)
								{
									if ((!got_start) || (_viv_fd_compare(&d->fd,&start_fd) > 0))
									{
										os_copy_memory(&start_fd,&d->fd,sizeof(WIN32_FIND_DATA));
										
										got_start = 1;
									}
								}
								else
								{
									if ((!got_start) || (_viv_fd_compare(&d->fd,&start_fd) < 0))
									{
										os_copy_memory(&start_fd,&d->fd,sizeof(WIN32_FIND_DATA));
										got_start = 1;
									}
								}
							}
							
							d = d->next;
						}
					}
					
	debug_printf("gotbest %d %d\n",got_best,got_start);
				}
			}
			else
			{
				WIN32_FIND_DATA fd;
				HANDLE h;
				wchar_t search_wbuf[STRING_SIZE];
				wchar_t path_wbuf[STRING_SIZE];

debug_printf("FIND next\n");
				
				string_get_path_part(path_wbuf,_viv_current_fd->cFileName);
				
				string_copy(search_wbuf,path_wbuf);
				string_cat_utf8(search_wbuf,(const utf8_t *)"\\*.*");
				
				h = FindFirstFile(search_wbuf,&fd);
				if (h != INVALID_HANDLE_VALUE)
				{
					for(;;)
					{
						if (_viv_is_valid_filename(&fd))
						{
							int compare_ret;
							
							fd.dwReserved0 = 0;
							fd.dwReserved1 = 0;
							
							compare_ret = _viv_fd_compare(&fd,_viv_current_fd);

							if (compare_ret != 0)
							{
								if (prev)
								{
									if (compare_ret < 0)
									{
										if ((!got_best) || (_viv_fd_compare(&fd,&best_fd) > 0))
										{
											string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
											string_copy_with_bufsize(fd.cFileName,MAX_PATH,search_wbuf);
											os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
											got_best = 1;
										}
									}
								}
								else
								{
									if (compare_ret > 0)
									{
										if ((!got_best) || (_viv_fd_compare(&fd,&best_fd) < 0))
										{
											string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
											string_copy_with_bufsize(fd.cFileName,MAX_PATH,search_wbuf);
											os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
											got_best = 1;
										}
									}		
								}
							}
							// (the wrap target tracking was deferred: see the second
							// pass after the scan, it only runs in the wrap case.)
							
						}
						
						if (!FindNextFile(h,&fd)) break;
					}

					FindClose(h);
				}
					if (!got_best)
					{
						// deferred wrap target. the search pattern is rebuilt first:
						// the scan above reuses search_wbuf for candidate paths.
						string_copy(search_wbuf,path_wbuf);
						string_cat_utf8(search_wbuf,(const utf8_t *)"\\*.*");
						
						h = FindFirstFile(search_wbuf,&fd);
						
						if (h != INVALID_HANDLE_VALUE)
						{
							for(;;)
							{
								if (_viv_is_valid_filename(&fd))
								{
									if (prev)
									{
										if ((!got_start) || (_viv_fd_compare(&fd,&start_fd) > 0))
										{
											string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
											string_copy_with_bufsize(fd.cFileName,MAX_PATH,search_wbuf);
											os_copy_memory(&start_fd,&fd,sizeof(WIN32_FIND_DATA));
											
											got_start = 1;
										}
									}
									else
									{
										if ((!got_start) || (_viv_fd_compare(&fd,&start_fd) < 0))
										{
											string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
											string_copy_with_bufsize(fd.cFileName,MAX_PATH,search_wbuf);
											os_copy_memory(&start_fd,&fd,sizeof(WIN32_FIND_DATA));
											got_start = 1;
										}
									}
								}
								
								if (!FindNextFile(h,&fd)) break;
							}
							
							FindClose(h);
						}
					}
					
			}

			if (got_best)
			{
				_viv_open(&best_fd,is_preload);
			}
			else
			if (got_start)
			{
				// don't open the same image again.
				debug_printf("%S %S\n",start_fd.cFileName,_viv_current_fd->cFileName);
				if (string_compare(start_fd.cFileName,_viv_current_fd->cFileName) != 0)
				{
					_viv_open(&start_fd,is_preload);
				}
			}
			else
			{
				// dont blank because we may not have a best or start because we only have one image 
				// (compare_ret != 0)
//				_viv_blank();
				ret = 0;
			}
		}
		else
		{
			_viv_home(0,is_preload);
		}
	}

	// reset slideshow timer.
	if (reset_slideshow_timer)
	{
		if (_viv_is_slideshow)
		{
			KillTimer(_viv_hwnd,VIV_ID_SLIDESHOW_TIMER);
			SetTimer(_viv_hwnd,VIV_ID_SLIDESHOW_TIMER,config_slideshow_rate,0);
		}
	}
	
	return ret;
}
void _viv_home(int end,int is_preload)
{
	if (_viv_random)
	{
		_viv_send_random_everything_search();
	}
	else
	{
		WIN32_FIND_DATA best_fd;
		int got_best;
		
		got_best = 0;
		
		_viv_do_initial_shuffle();
		
		if (_viv_playlist_start)
		{
			if (config_shuffle)
			{
				int index;
				
				if (end)
				{
					index = _viv_playlist_count - 1;
				}
				else
				{
					index = 0;
				}
				
				os_copy_memory(&best_fd,&_viv_playlist_shuffle_indexes[index]->fd,sizeof(WIN32_FIND_DATA));

				got_best = 1;
			}
			else	
			{
				_viv_playlist_t *d;
				
				d = _viv_playlist_start;
				while(d)
				{
					if (end)
					{
						if ((!got_best) || (_viv_fd_compare(&d->fd,&best_fd) > 0))
						{
							os_copy_memory(&best_fd,&d->fd,sizeof(WIN32_FIND_DATA));
							got_best = 1;
						}
					}
					else
					{
						if ((!got_best) || (_viv_fd_compare(&d->fd,&best_fd) < 0))
						{
							os_copy_memory(&best_fd,&d->fd,sizeof(WIN32_FIND_DATA));
							got_best = 1;
						}		
					}					
				
					d = d->next;
				}
			}
		}
		else
		{
			HANDLE h;
			WIN32_FIND_DATA fd;
			wchar_t search_wbuf[STRING_SIZE];
			wchar_t path_wbuf[STRING_SIZE];
		
			if (*_viv_current_fd->cFileName)
			{
				string_get_path_part(path_wbuf,_viv_current_fd->cFileName);
			}
			else
			{
				GetCurrentDirectory(STRING_SIZE,path_wbuf);
			}
			
			string_copy(search_wbuf,path_wbuf);
			string_cat_utf8(search_wbuf,(const utf8_t *)"\\*.*");
		
			h = FindFirstFile(search_wbuf,&fd);
			
			if (h != INVALID_HANDLE_VALUE)
			{
				for(;;)
				{
					if (_viv_is_valid_filename(&fd))
					{
						fd.dwReserved0 = 0;
						fd.dwReserved1 = 0;
						
						if (end)
						{
							if ((!got_best) || (_viv_fd_compare(&fd,&best_fd) > 0))
							{
								string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
								string_copy_with_bufsize(fd.cFileName,MAX_PATH,search_wbuf);
								os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
								got_best = 1;
							}
						}
						else
						{
							if ((!got_best) || (_viv_fd_compare(&fd,&best_fd) < 0))
							{
								string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
								string_copy_with_bufsize(fd.cFileName,MAX_PATH,search_wbuf);
								os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
								got_best = 1;
							}		
						}
					}
					
					if (!FindNextFile(h,&fd)) break;
				}

				FindClose(h);
			}
		}
		
		if (got_best)
		{	
			_viv_open(&best_fd,is_preload);
		}
		else
		{
			if (is_preload)
			{
			}
			else
			{
				_viv_blank();
			}
		}
	}

	// reset slideshow timer.
	if (_viv_is_slideshow)
	{
		KillTimer(_viv_hwnd,VIV_ID_SLIDESHOW_TIMER);
		SetTimer(_viv_hwnd,VIV_ID_SLIDESHOW_TIMER,config_slideshow_rate,0);
	}
}
void _viv_view_set(int view_x,int view_y,int invalidate)
{
	RECT rect;
	int wide;
	int high;
	int rw;
	int rh;
	
	GetClientRect(_viv_hwnd,&rect);
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_view_top();

	_viv_get_render_size(&rw,&rh);
/*		
		if (_viv_zoom_pos == 1)
		{
			if ((rw < _viv_image_wide) || (rw < _viv_image_wide))
			{
				rw = _viv_image_wide;
				rh = _viv_image_high;
			}
		}
*/

	{
		int rx;
		int ry;
			
		rx = (wide / 2) - (rw / 2) - view_x;
		ry = (high / 2) - (rh / 2) - view_y;

		if (config_keep_centered)
		{
			if (rw > wide)
			{
				if (rx > 0)
				{
					view_x = (wide / 2) - (rw / 2);
				}

				if (rx + rw < wide)
				{
					// rx = wide - rw;
					view_x = (wide / 2) - (rw / 2) - (wide - rw);
				}
			}
			else
			{
				view_x = 0;
			}
		
			if (rh > high)
			{
				if (ry > 0)
				{
					view_y = (high / 2) - (rh / 2);
				}

				if (ry + rh < high)
				{
					view_y = (high / 2) - (rh / 2) - (high - rh);
				}
			}
			else
			{
				view_y = 0;
			}
		}
		else
		{
		
			if (rx + rw - 1 < 0)
			{
				view_x = (wide / 2) - (rw / 2) + rw - 1;
			}
			
			if (rx > wide-1)
			{
				view_x = (wide / 2) - (rw / 2) - (wide-1);
			}
			
			if (ry + rh - 1 < 0)
			{
				view_y = (high / 2) - (rh / 2) + rh - 1;
			}
			
			if (ry > high-1)
			{
				view_y = (high / 2) - (rh / 2) - (high-1);
			}
			
		}
	}
	
	if (rw)
	{
		int rx;

		rx = (((_viv_dst_pos_x - 250) * (wide*2)) / 1000) - (rw / 2) - view_x;

		// make sure the multiple is done as __int64 to avoid overflows
		_viv_view_ix = (((wide / 2) - rx) * (__int64)_viv_image_wide) / (double)rw;
	}
	else
	{
		// don't set to 0, just use last value.
//		_viv_view_ix = 0.0;
	}

	if (rh)
	{
		int ry;

		ry = (((_viv_dst_pos_y - 250) * (high*2)) / 1000) - (rh / 2) - view_y;
		
		// make sure the multiple is done as __int64 to avoid overflows
		_viv_view_iy = (((high / 2) - ry) * (__int64)_viv_image_high) / (double)rh;
	}
	else
	{
		// don't set to 0, just use last value.
//		_viv_view_iy = 0;
	}

	if ((_viv_view_x != view_x) || (_viv_view_y != view_y))
	{
		_viv_view_x = view_x;
		_viv_view_y = view_y;

		if (invalidate)
		{
			InvalidateRect(_viv_hwnd,0,FALSE);
		}
	}

debug_printf("SETVIEW %d %d ix %d iy %d rw %d rh %d wide %d high %d\n",_viv_view_x,_viv_view_y,(int)(_viv_view_ix),(int)(_viv_view_iy),rw,rh,wide,high);

	_viv_toolbar_update_buttons();
}
void _viv_slideshow(void)
{
	if (_viv_file_not_found)
	{
		return;
	}
	
	if (!_viv_is_fullscreen)
	{
		_viv_toggle_fullscreen();
	}
	
	if (!_viv_is_slideshow)
	{
		SetTimer(_viv_hwnd,VIV_ID_SLIDESHOW_TIMER,config_slideshow_rate,0);
		
		_viv_is_slideshow = 1;
		_viv_status_update();
		_viv_toolbar_update_buttons();
		_viv_update_ontop();
		_viv_update_prevent_sleep();
	}
}
static void _viv_set_custom_rate(void)
{
	if (DialogBox(os_hinstance,MAKEINTRESOURCE(IDD_CUSTOM_RATE),_viv_hwnd,_viv_custom_rate_proc))
	{
		switch(config_slideshow_custom_rate_type)
		{
			case 0:
				config_slideshow_rate = config_slideshow_custom_rate;
				break;
		
			case 1: 
				config_slideshow_rate = config_slideshow_custom_rate * 1000;
				break;
				
			case 2: 
				config_slideshow_rate = config_slideshow_custom_rate * 1000 * 60;
				break;
		}
		
		if (config_slideshow_rate < 1)
		{
			config_slideshow_rate = 1;
		}
		
		if (_viv_is_slideshow)
		{
			KillTimer(_viv_hwnd,VIV_ID_SLIDESHOW_TIMER);

			SetTimer(_viv_hwnd,VIV_ID_SLIDESHOW_TIMER,config_slideshow_rate,0);
		}			

		_viv_status_update_slideshow_rate();
	}
}
static void _viv_set_rate(int rate)
{
	config_slideshow_rate = rate;
	
	if (_viv_is_slideshow)
	{
		KillTimer(_viv_hwnd,VIV_ID_SLIDESHOW_TIMER);

		SetTimer(_viv_hwnd,VIV_ID_SLIDESHOW_TIMER,config_slideshow_rate,0);
	}			

	_viv_status_update_slideshow_rate();
}
static void _viv_delete(int permanently)
{
	if (*_viv_current_fd->cFileName)
	{
		SHFILEOPSTRUCT fo;
		WIN32_FIND_DATA fd;
		wchar_t filename_list[STRING_SIZE+1];
		
		string_copy_double_null(filename_list,_viv_current_fd->cFileName);
		os_copy_memory(&fd,_viv_current_fd,sizeof(WIN32_FIND_DATA));
		
		ZeroMemory(&fo,sizeof(SHFILEOPSTRUCT));
		fo.hwnd = _viv_hwnd;
		fo.wFunc = FO_DELETE;
		fo.pFrom = filename_list;
		fo.fFlags = permanently ? 0 : FOF_ALLOWUNDO;

		if (SHFileOperation(&fo) == 0)
		{
			if (!fo.fAnyOperationsAborted)
			{
				_viv_playlist_delete(&fd);
			
				if (!_viv_next(0,1,0,0))
				{
					_viv_blank();
				}
			}
		}
	}
}
static void _viv_copy(int cut)
{
	if (*_viv_current_fd->cFileName)
	{
		if (OpenClipboard(_viv_hwnd))
		{
			EmptyClipboard();
	
			_viv_set_clipboard_image();

			{
				HGLOBAL hmem;
				int wlen;
				
				wlen = string_get_length(_viv_current_fd->cFileName);
				
				hmem = GlobalAlloc(GMEM_MOVEABLE,safe_size_add(safe_size_mul_sizeof_wchar(safe_size_add(safe_size_add_one(wlen),1)),sizeof(DROPFILES)));
				if (hmem)
				{
					DROPFILES *df;
					
					// build the dropfiles struct
					df = (DROPFILES *)GlobalLock(hmem);
					if (df)
					{
						df->pFiles = sizeof(DROPFILES);
						df->fWide = 1;
						df->fNC = 0;
						df->pt.x = 0;
						df->pt.y = 0;
						
						os_copy_memory(df+1,_viv_current_fd->cFileName,wlen * sizeof(wchar_t));
						((wchar_t *)(df + 1))[wlen] = 0;
						((wchar_t *)(df + 1))[wlen+1] = 0;

						GlobalUnlock(hmem);
					}

					// looking at example code,none seem to free hglobal.
					// apparently the system now owns the handle 
					SetClipboardData(CF_HDROP,hmem);
				}
			}
			
			{
				HGLOBAL hmem;
				
				hmem = GlobalAlloc(GMEM_MOVEABLE,sizeof(DWORD));
				if (hmem)
				{
					DWORD *effect;
					
					// build the dropfiles struct
					effect = (DWORD *)GlobalLock(hmem);
					if (effect)
					{
						*effect = cut ? DROPEFFECT_MOVE : (DROPEFFECT_COPY|DROPEFFECT_LINK);

						GlobalUnlock(hmem);
					}

					// looking at example code,none seem to free hglobal.
					// apparently the system now owns the handle 
					SetClipboardData(_viv_get_CF_PREFERREDDROPEFFECT(),hmem);
				}
			}
						
			CloseClipboard();
		}
	}
}
static void _viv_copy_filename(void)
{
	if (*_viv_current_fd->cFileName)
	{
		if (OpenClipboard(_viv_hwnd))
		{
			EmptyClipboard();
			
			{
				HGLOBAL hmem;
				int wlen;
				
				wlen = string_get_length(_viv_current_fd->cFileName);
				
				hmem = GlobalAlloc(GMEM_MOVEABLE,safe_size_mul_sizeof_wchar(safe_size_add_one(wlen)));
				if (hmem)
				{
					wchar_t *wstring;
					
					// build the dropfiles struct
					wstring = (wchar_t *)GlobalLock(hmem);
					if (wstring)
					{
						os_copy_memory(wstring,_viv_current_fd->cFileName,wlen*sizeof(wchar_t));
						wstring[wlen] = 0;

						GlobalUnlock(hmem);
					}

					// looking at example code,none seem to free hglobal.
					// apparently the system now owns the handle 
					SetClipboardData(CF_UNICODETEXT,hmem);
				}
			}
			
			CloseClipboard();
		}
	}
}
static void _viv_copy_image(void)
{
	if (*_viv_current_fd->cFileName)
	{
		if (OpenClipboard(_viv_hwnd))
		{
			EmptyClipboard();
			
			_viv_set_clipboard_image();
			
			CloseClipboard();
		}
	}
}
static int _viv_is_key_state(int control,int shift,int alt)
{
	if (GetKeyState(VK_CONTROL) < 0)
	{
		if (!control) return 0;
	}
	else
	{
		if (control) return 0;
	}
	
	if (GetKeyState(VK_SHIFT) < 0)
	{
		if (!shift) return 0;
	}
	else
	{
		if (shift) return 0;
	}
	
	if (GetKeyState(VK_MENU) < 0)
	{
		if (!alt) return 0;
	}
	else
	{
		if (alt) return 0;
	}
	
	return 1;
}
void _viv_pause(void)
{
/*
	if (_viv_file_not_found)
	{
		MessageBeep(MB_OK);
		return;
	}
*/	
	if (_viv_is_slideshow)
	{
		KillTimer(_viv_hwnd,VIV_ID_SLIDESHOW_TIMER);
		
		_viv_is_slideshow = 0;
	}
	else
	{
		SetTimer(_viv_hwnd,VIV_ID_SLIDESHOW_TIMER,config_slideshow_rate,0);
		
		_viv_is_slideshow = 1;
	}

	_viv_status_update();
	_viv_toolbar_update_buttons();
	_viv_update_ontop();
	_viv_update_prevent_sleep();
}
static void _viv_increase_rate(int dec)
{
	if (dec)	
	{
		int i;
		
		for(i=0;i<_VIV_SLIDESHOW_RATE_PRESET_COUNT;i++)
		{
			if (config_slideshow_rate > _viv_slideshow_rate_presets[_VIV_SLIDESHOW_RATE_PRESET_COUNT-i-1])
			{
				_viv_set_rate(_viv_slideshow_rate_presets[_VIV_SLIDESHOW_RATE_PRESET_COUNT-i-1]);
				
				break;
			}
		}
	}
	else
	{
		int i;
		
		for(i=0;i<_VIV_SLIDESHOW_RATE_PRESET_COUNT;i++)
		{
			if (config_slideshow_rate < _viv_slideshow_rate_presets[i])
			{
				_viv_set_rate(_viv_slideshow_rate_presets[i]);
				
				break;
			}
		}
	}

	_viv_status_update_slideshow_rate();
}
static void _viv_file_preview(void)
{
	if (*_viv_current_fd->cFileName)
	{
		os_shell_execute(_viv_hwnd,_viv_current_fd->cFileName,0,"preview",0);
	}
}
static void _viv_file_print(void)
{
	if (*_viv_current_fd->cFileName)
	{
		os_shell_execute(_viv_hwnd,_viv_current_fd->cFileName,0,"print",0);
	}
}
static void _viv_file_set_desktop_wallpaper(void)
{
	if (*_viv_current_fd->cFileName)
	{
		wchar_t message_wbuf[STRING_SIZE];
		wchar_t caption_wbuf[STRING_SIZE];
		
		// a stray menu click used to rewrite the desktop silently; the menu
		// comment has asked for this dialog since the first release (and the
		// ctrl+d default key returns with it).
		string_copy_utf8_string(message_wbuf,localization_get_string(LOCALIZATION_ID_SET_DESKTOP_WALLPAPER_MESSAGE));
		string_copy_utf8_string(caption_wbuf,localization_get_string(LOCALIZATION_ID_SET_DESKTOP_WALLPAPER_CAPTION));
		
		if (viv_msgbox(_viv_hwnd,caption_wbuf,message_wbuf,MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
		{
			if (!_viv_stobject_hmodule)
			{
				_viv_stobject_hmodule = LoadLibraryA("stobject.dll");
			}
			
			if (_viv_stobject_hmodule)
			{
				os_shell_execute(_viv_hwnd,_viv_current_fd->cFileName,0,"setdesktopwallpaper",0);
			}
		}
	}
}
static void _viv_edit_rotate(int counterclockwise)
{
	if (*_viv_current_fd->cFileName)
	{
		// FIXME: we need to wait for image to load.
		if (_viv_frame_loaded_count == _viv_frame_count)
		{
			// this tends to fail if called too quickly after a previous call
			// can't seem to catch the error ..
			if (os_shell_execute(_viv_hwnd,_viv_current_fd->cFileName,1,counterclockwise ? "rotate270" : "rotate90",0))
			{
				int i;
				int temp;

				// rotate images in memory too
				
				for(i=0;i<_viv_frame_count;i++)
				{
					HBITMAP new_hbitmap;
					
					// i do the reverse to reverse the orientation.
					new_hbitmap = _viv_orientate_hbitmap(_viv_frames[i].hbitmap,counterclockwise ? 8 : 6);
					
					if (new_hbitmap)
					{
						DeleteObject(_viv_frames[i].hbitmap);
						
						_viv_frames[i].hbitmap = new_hbitmap;
					}
					
					// delete mipmaps
					if (_viv_frames[i].mipmap)
					{
						_viv_mipmap_free(_viv_frames[i].mipmap);
						
						_viv_frames[i].mipmap = NULL;
					}
				}
				
				temp = _viv_image_wide;
				_viv_image_wide = _viv_image_high;
				_viv_image_high = temp;
				
				_viv_view_set(_viv_view_x,_viv_view_y,1);
				
				_viv_update_src_pixel(1,0);
				_viv_status_update();

				InvalidateRect(_viv_hwnd,0,FALSE);
			}
		}
	}
}
static void _viv_file_edit(void)
{
	if (*_viv_current_fd->cFileName)
	{
		os_shell_execute(_viv_hwnd,_viv_current_fd->cFileName,0,"edit",0);
	}
}
static void _viv_open_file_location(void)
{
	if (*_viv_current_fd->cFileName)
	{
		int openpathok;
		
		openpathok = 0;
		
		if (os_SHOpenFolderAndSelectItems)
		{
			wchar_t path_part[STRING_SIZE];
			ITEMIDLIST *folder_idlist;
			
			string_get_path_part(path_part,_viv_current_fd->cFileName);
		
			// if path_part_buf.buf is an empty string, os_ILCreateFromPath will
			// correctly return the desktop pidl (an empty pidl).
			folder_idlist = os_ILCreateFromPath(path_part);

		debug_printf("folder_idlist %p\n",folder_idlist);
			if (folder_idlist)
			{
				ITEMIDLIST *idlist;
				
				idlist = os_ILCreateFromPath(_viv_current_fd->cFileName);
		debug_printf("idlist %S %p\n",_viv_current_fd->cFileName,idlist);
				if (idlist)
				{
					HRESULT hres;
					
		debug_printf("ENTER os_SHOpenFolderAndSelectItems\n");
		
					hres = os_SHOpenFolderAndSelectItems(folder_idlist,1,(LPCITEMIDLIST *)&idlist,0);
					
		debug_printf("os_SHOpenFolderAndSelectItems %08x\n",hres);
					if (SUCCEEDED(hres))
					{
						openpathok = 1;
					}
					else
					if (hres == E_ABORT)
					{
						// aborted, QTBar after 10seconds.
						openpathok = 1;
					}

					CoTaskMemFree(idlist);
				}

				CoTaskMemFree(folder_idlist);
			}
		}
		
		if (!openpathok)
		{
			wchar_t path[STRING_SIZE];
			
			string_get_path_part(path,_viv_current_fd->cFileName);
			
			os_shell_execute(_viv_hwnd,path,0,NULL,NULL);
		}
	}
}
static void _viv_properties(void)
{
	if (*_viv_current_fd->cFileName)
	{
		os_shell_execute(_viv_hwnd,_viv_current_fd->cFileName,0,"properties",0);
	}
}
void _viv_mousemove(void)
{
	POINT pt;
	
	GetCursorPos(&pt);
	
//	debug_printf("MOUSEMOVE %d %d, last %d %d\n",pt.x,pt.y,_viv_mousemove_x,_viv_mousemove_y);
	
	if ((pt.x != _viv_mousemove_x) || (pt.y != _viv_mousemove_y))
	{
		_viv_show_cursor();
		
		if (!_viv_should_show_cursor())
		{
			_viv_start_hide_cursor_timer();
		}		
	}
	
	_viv_mousemove_x = pt.x;
	_viv_mousemove_y = pt.y;
}
void _viv_view_1to1(void)
{
	if (_viv_1to1)	
	{
		if (_viv_have_old_zoom)
		{
			_viv_1to1 = 0;
			_viv_zoom_pos = _viv_clamp_zoom_pos(_viv_old_zoom_pos);
			_viv_view_set(_viv_view_x,_viv_view_y,1);
			InvalidateRect(_viv_hwnd,0,FALSE);
			_viv_status_update_temp_pos_zoom();
			
			return;
		}
	}
	
	_viv_have_old_zoom = 1;
	_viv_old_zoom_pos = _viv_zoom_pos;
	
	_viv_1to1 = 1;
	_viv_zoom_pos = 0;
	_viv_view_set(_viv_view_x,_viv_view_y,1);
	InvalidateRect(_viv_hwnd,0,FALSE);
	_viv_status_update_temp_pos_zoom();
}
void _viv_zoom_set_percent(int percent,int screen_x,int screen_y,int force)
{
	// set the zoom to the ladder position whose displayed percent is closest
	// to the requested percent, keeping the image point under the anchor
	// fixed (the same anchor math as the wheel zoom).
	int old_zoom_pos;
	int old_1to1;
	int old_rw;
	int old_rh;
	int rx;
	int ry;
	int old_cursor_px;
	int old_cursor_py;
	int new_rw;
	int new_rh;
	int wide;
	int high;
	int old_percent;
	RECT rect;
	POINT pt;
	__int64 new_cursor_x;
	__int64 new_cursor_y;
	
	old_zoom_pos = _viv_zoom_pos;
	old_1to1 = _viv_1to1;
	old_percent = _viv_zoom_percent();
	
	GetClientRect(_viv_hwnd,&rect);
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_view_top();
	
	pt.x = screen_x;
	pt.y = screen_y;
	
	ScreenToClient(_viv_hwnd,&pt);
	
	// the viewport origin: the anchor math below is viewport relative.
	pt.y -= _viv_get_view_top();
	
	_viv_get_render_size(&old_rw,&old_rh);
	
	rx = (wide / 2) - (old_rw / 2) - _viv_view_x;
	ry = (high / 2) - (old_rh / 2) - _viv_view_y;
	
	old_cursor_px = pt.x - rx;
	old_cursor_py = pt.y - ry;
	
	if (percent == 100)
	{
		// exact 100% is the 1:1 mode: native, pixel perfect size. entering it
		// mirrors _viv_view_1to1() so the 1:1 command still toggles back to
		// the previous zoom.
		if (!_viv_1to1)
		{
			_viv_have_old_zoom = 1;
			_viv_old_zoom_pos = _viv_zoom_pos;
			_viv_1to1 = 1;
			_viv_zoom_pos = 0;
		}
	}
	else
	{
		// percent stepping leaves 1:1 mode: the ladder takes over.
		if (_viv_1to1)
		{
			_viv_1to1 = 0;
		}
		
		_viv_zoom_pos = _viv_zoom_pos_for_percent(percent,0);
	}
	
	// clamp to the live ladder range so the percent step never lands in the
	// dead zone of positions that all render the same size.
	_viv_zoom_pos = _viv_clamp_zoom_pos(_viv_zoom_pos);
	
	// the snap target can be physically undisplayable: the ladder renders
	// in 1.01x steps, so past ~120% some integers are skipped (and past
	// ~1400% every value is ~14 apart). when the closest landing is not the
	// exact target and does not move in the click direction, jump to the
	// next multiple of 10 in the click direction instead: a button click
	// always makes a clean, visible step. (a precise landing is accepted
	// even when the snap moves against the click direction: that is the
	// "nearest multiple" contract for the click after a wheel gesture.
	// the dialog passes force 0: a typed value simply lands closest.)
	if (force && (!_viv_1to1))
	{
		int landed;
		
		landed = _viv_zoom_percent();
		
		if ((landed != percent) && ((force > 0) ? (_viv_zoom_pos <= old_zoom_pos) : (_viv_zoom_pos >= old_zoom_pos)))
		{
			int next;
			
			next = ((old_percent / 10) * 10) + ((force > 0) ? 10 : -10);
			
			if (next < 1)
			{
				next = 1;
			}
			
			_viv_zoom_pos = _viv_clamp_zoom_pos(_viv_zoom_pos_for_percent(next,1));
			
			// sparse ladder zones (past ~1400% one position is worth
			// ~14 points): a 10 point target can sit between two
			// positions and even the strict jump lands on the old one.
			// the click still owes the user a move: step one position
			// in the click direction.
			if ((force > 0) ? (_viv_zoom_pos <= old_zoom_pos) : (_viv_zoom_pos >= old_zoom_pos))
			{
				_viv_zoom_pos = _viv_clamp_zoom_pos(old_zoom_pos + ((force > 0) ? 1 : -1));
			}
		}
	}
	
	if ((_viv_zoom_pos != old_zoom_pos) || (_viv_1to1 != old_1to1))
	{
		_viv_get_render_size(&new_rw,&new_rh);
		
		if (old_rw)
		{
			new_cursor_x = ((__int64)old_cursor_px * (__int64)new_rw) / (__int64)old_rw;
		}
		else
		{
			new_cursor_x = 0;
		}
		
		if (old_rh)
		{
			new_cursor_y = ((__int64)old_cursor_py * (__int64)new_rh) / (__int64)old_rh;
		}
		else
		{
			new_cursor_y = 0;
		}
		
		_viv_view_set((wide / 2) - (new_rw / 2) - pt.x + new_cursor_x,(high / 2) - (new_rh / 2) - pt.y + new_cursor_y,1);
		
		InvalidateRect(_viv_hwnd,0,FALSE);
		
		_viv_status_update_temp_pos_zoom();
	}
}
void _viv_zoom_in(int out,int have_xy,int x,int y)
{
	POINT pt;
	int percent;
	int target;
	
	if (!_viv_image_wide)
	{
		return;
	}
	
	// at the true bottom of the ladder a zoom out click has nothing
	// below it: do nothing. (rc.13: the below-fit extension made position
	// 0 the bestfit anchor rather than the floor - the old pos==0 gate
	// ate the first out click on every freshly opened image, and the
	// field read it as "zoom needs two clicks".)
	if (out && (!_viv_1to1) && (_viv_zoom_pos <= _viv_zoom_pos_floor()))
	{
		return;
	}
	
	if (have_xy)
	{
		pt.x = x;
		pt.y = y;
	}
	else
	{
		RECT rect;
		GetClientRect(_viv_hwnd,&rect);
		pt.x = (rect.right - rect.left) / 2;
		pt.y = (_viv_get_view_top() + (rect.bottom - rect.top - _viv_get_status_high())) / 2;
	}

	ClientToScreen(_viv_hwnd,&pt);
	
	// discrete clicks (toolbar buttons, keyboard, the configured mouse
	// action) step whole 10% per click, and the snap is direction strict:
	// an out click lands on the multiple strictly below, an in click on
	// the one strictly above, so the first click after a wheel stop (or
	// from a fresh best fit) always moves in the click direction - the
	// rc.1 nearest-multiple snap stepped a wheel-stopped 31% zoom-in
	// down to 30 and the field read it as a dead first click. the wheel
	// and pinch gestures keep the proportional ladder stepping.
	percent = _viv_zoom_percent();
	
	if (out)
	{
		target = ((percent - 1) / 10) * 10;
	}
	else
	{
		target = ((percent / 10) * 10) + 10;
	}
	
	if (target < 1)
	{
		target = 1;
	}
	
	_viv_zoom_set_percent(target,pt.x,pt.y,out ? -1 : 1);
}
void _viv_view_scroll(int mx,int my)
{
	int old_view_x;
	int old_view_y;

	old_view_x = _viv_view_x;
	old_view_y = _viv_view_y;

	_viv_view_set(_viv_view_x - mx,_viv_view_y - my,0);
	
	if (config_scroll_window)
	{
		RECT rect;
		
		GetClientRect(_viv_hwnd,&rect);
		
		// limit the scroll to the image viewport so the scroll blit never
		// touches the status bar and the top strips.
		rect.top += _viv_get_view_top();
		rect.bottom -= _viv_get_status_high();
		
		if (ScrollWindowEx(_viv_hwnd,old_view_x - _viv_view_x,old_view_y - _viv_view_y,&rect,0,0,0,SW_INVALIDATE) == ERROR)
		{
			debug_printf("scroll error %d\n",GetLastError());
		}
		else
		{
			// repaint the exposed area right away. without this the newly exposed
			// strip stays stale while more mouse move messages stream in during a
			// drag, and stale pixels get scrolled back into the image, which looks
			// like tearing.
			UpdateWindow(_viv_hwnd);
		}
	}
	else
	{
		// this fixes the scroll bug for stamimail
		InvalidateRect(_viv_hwnd,0,FALSE);
	}
	
//	UpdateWindow(_viv_hwnd);
}
void _viv_update_1to1_scroll(int x,int y)
{
	int cursor_x;
	int cursor_y;
	int rx;
	int ry;
	int rw;
	int rh;
	int old_rw;
	int old_rh;
	int old_cursor_px;
	int old_cursor_py;
	int wide;
	int high;
	RECT rect;
	POINT pt;
	__int64 new_cursor_x;
	__int64 new_cursor_y;
	int zoom_backup;
	int backup_1to1;
			
	GetClientRect(_viv_hwnd,&rect);

	wide = rect.right - rect.left;
	high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_view_top();
	
	pt.x = x ;
	pt.y = y;
	
	cursor_x = pt.x; 
	cursor_y = pt.y - _viv_get_view_top();

	zoom_backup = _viv_zoom_pos;
	backup_1to1 = _viv_1to1;
	_viv_zoom_pos = 0;
	_viv_1to1 = 0;
	_viv_doing = _VIV_DOING_NOTHING;
	_viv_get_render_size(&old_rw,&old_rh);
	_viv_doing = _VIV_DOING_1TO1SCROLL;
	_viv_zoom_pos = zoom_backup;
	_viv_1to1 = backup_1to1;

	rx = (wide / 2) - (old_rw / 2);
	ry = (high / 2) - (old_rh / 2);
	
	old_cursor_px = (cursor_x - rx);
	old_cursor_py = (cursor_y - ry);

	_viv_get_render_size(&rw,&rh);

	if (old_rw)
	{
		new_cursor_x = ((__int64)old_cursor_px * (__int64)rw) / (__int64)old_rw;
	}
	else
	{
		new_cursor_x = 0;
	}
	
	if (old_rh)
	{
		new_cursor_y = ((__int64)old_cursor_py * (__int64)rh) / (__int64)old_rh;
	}
	else
	{
		new_cursor_y = 0;
	}

	_viv_view_set((wide / 2) - (rw / 2) - cursor_x + new_cursor_x,(high / 2) - (rh / 2) - cursor_y + new_cursor_y,1);

	InvalidateRect(_viv_hwnd,0,FALSE);
}
// x,y in screen coords
void _viv_do_mousewheel_action(int action,int delta,int x,int y)
{
	if (action == 0)
	{
		int cursor_x;
		int cursor_y;
		int rx;
		int ry;
		int rw;
		int rh;
		int old_rw;
		int old_rh;
		int old_cursor_px;
		int old_cursor_py;
		int wide;
		int high;
		RECT rect;
		POINT pt;
		__int64 new_cursor_x;
		__int64 new_cursor_y;
		int old_zoom_pos;
		
		old_zoom_pos = _viv_zoom_pos;
		
		GetClientRect(_viv_hwnd,&rect);
		wide = rect.right - rect.left;
		high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_view_top();
		
		pt.x = x;
		pt.y = y;
		
		ScreenToClient(_viv_hwnd,&pt);

		cursor_x = pt.x; 
		cursor_y = pt.y - _viv_get_view_top();
	
		_viv_get_render_size(&rw,&rh);
		
/*
		if (_viv_zoom_pos == 1)
		{
			if ((rw < _viv_image_wide) || (rw < _viv_image_wide))
			{
				rw = _viv_image_wide;
				rh = _viv_image_high;
			}
		}
		*/
		rx = (wide / 2) - (rw / 2) - _viv_view_x;
		ry = (high / 2) - (rh / 2) - _viv_view_y;
		
		old_cursor_px = (cursor_x - rx);
		old_cursor_py = (cursor_y - ry);
		old_rw = rw;
		old_rh = rh;
/*
		if (old_cursor_px < 0)
		{
			old_cursor_px = 0;
		}

		if (old_cursor_px > 20 * rw)
		{
			old_cursor_px = 20 * rw;
		}
		
		if (old_cursor_py < 0)
		{
			old_cursor_py = 0;
		}

		if (old_cursor_py > 20 * rh)
		{
			old_cursor_py = 20 * rh;
		}
		*/
		if (_viv_1to1)
		{
			_viv_1to1 = 0;
			
			if (delta > 0)
			{
				// binary search for the first ladder position that grows past the
				// 1:1 size. the render size is monotonic in the position, so this
				// finds the same position as the old linear scan with ~10
				// measurements instead of up to 1024.
				{
					int lo;
					int hi;
					
					lo = 0;
					hi = _VIV_ZOOM_MAX; // exclusive upper bound
					
					while (lo < hi)
					{
						int mid;
						
						mid = lo + ((hi - lo) / 2);
						
						_viv_zoom_pos = mid;
						
						_viv_get_render_size(&rw,&rh);
						
						if (rw > old_rw)
						{
							hi = mid;
						}
						else
						{
							lo = mid + 1;
						}
					}
					
					_viv_zoom_pos = lo;
				}
			}
			else
			{
				// binary search for the highest ladder position below the 1:1 size,
				// or the floor minus one when even the deepest below-fit zoom-out
				// is at least as large (the clamp settles that on the floor). the
				// render size is monotonic, so this matches the old linear scan from
				// the top with ~10 measurements instead of up to 1024.
				{
					int lo;
					int hi;
					
					lo = _viv_zoom_pos_floor();
					hi = _viv_zoom_pos_max() + 1; // exclusive upper bound
					
					while (lo < hi)
					{
						int mid;
						
						mid = lo + ((hi - lo) / 2);
						
						_viv_zoom_pos = mid;
						
						_viv_get_render_size(&rw,&rh);
						
						if (rw < old_rw)
						{
							lo = mid + 1;
						}
						else
						{
							hi = mid;
						}
					}
					
					_viv_zoom_pos = lo - 1;
				}
			}
		}

		// proportional stepping: a standard wheel notch (delta 120) applies
		// _VIV_ZOOM_STEPS_PER_NOTCH 1% zoom steps (about 10%). fast flicks send
		// multiples of 120 and zoom further. high resolution wheels and
		// trackpads send smaller deltas more often. the steps also apply after
		// the 1:1 transition above so gestures keep tracking the fingers.
		{
			int steps;
			
			steps = ((delta < 0) ? (0 - delta) : delta) * _VIV_ZOOM_STEPS_PER_NOTCH;
			
			steps = (steps + 60) / 120;
			
			if (!steps)
			{
				steps = 1;
			}
			
			if (delta > 0)
			{
				_viv_zoom_pos += steps;
			}
			else
			{
				_viv_zoom_pos -= steps;
			}
		}
		
		// clamp to the live ladder range (0 .. pos_max) so the wheel never
		// spins in a dead zone of positions that all render the same size.
		_viv_zoom_pos = _viv_clamp_zoom_pos(_viv_zoom_pos);
		
		if (_viv_zoom_pos != old_zoom_pos)
		{
			_viv_get_render_size(&rw,&rh);
	/*
			if (_viv_zoom_pos == 1)
			{
				if ((rw < _viv_image_wide) || (rw < _viv_image_wide))
				{
					rw = _viv_image_wide;
					rh = _viv_image_high;
				}
			}
	*/
			// 
			// new_cursor_x = 
	//			rx = (wide / 2) - (rw / 2)

	//debug_printf("%d %d\n",(wide / 2) - (rw / 2),(high / 2) - (rh / 2));
	//debug_printf("%d %d\n",old_cursor_px,(old_cursor_px * 100) / old_rw);
						
	//debug_printf("old %d %d new %d %d\n",old_cursor_px,old_cursor_py,(wide / 2) - (rw / 2) - cursor_x + ((old_cursor_px * rw) / old_rw),(high / 2) - (rh / 2) - cursor_y + ((old_cursor_py * rh) / old_rh))		;
	//debug_printf("wide / 2 = %d, rw/2=%d, old_cursor_px * rw=%d\n",wide/2,rw/2,old_cursor_px * rw)		;

			if (old_rw)
			{
				new_cursor_x = ((__int64)old_cursor_px * (__int64)rw) / (__int64)old_rw;
			}
			else
			{
				new_cursor_x = 0;
			}
			
			if (old_rh)
			{
				new_cursor_y = ((__int64)old_cursor_py * (__int64)rh) / (__int64)old_rh;
			}
			else
			{
				new_cursor_y = 0;
			}

			_viv_view_set((wide / 2) - (rw / 2) - cursor_x + new_cursor_x,(high / 2) - (rh / 2) - cursor_y + new_cursor_y,1);
			
			InvalidateRect(_viv_hwnd,0,FALSE);
			_viv_status_update_temp_pos_zoom();
		}
	}
	else
	if (action == 1)
	{
		if (delta > 0)
		{
			_viv_next(1,1,0,0);
		}
		else
		if (delta < 0)
		{
			_viv_next(0,1,0,0);
		}
	}
	else
	if (action == 2)
	{
		if (delta > 0)
		{
			_viv_next(0,1,0,0);
		}
		else
		if (delta < 0)
		{
			_viv_next(1,1,0,0);
		}
	}
}
// 0 = scroll, 1 = play/pause slideshow, 2 = play/pause animation, 3=zoom in, 4=next, 5=1:1 scroll, 6=move-window
void _viv_do_left_click_action(int action)
{
	POINT cursor_pt;
	
	GetCursorPos(&cursor_pt);
	
	ScreenToClient(_viv_hwnd,&cursor_pt);

	switch(action)
	{
		case 0: // scroll
		
			if (_viv_doing == _VIV_DOING_NOTHING)
			{
				
				
				if (!_viv_is_fullscreen)
				{
					if (!config_show_caption)
					{
						_viv_start_move_window();
					
						return;
					}				
				}
	
				_viv_doing = _VIV_DOING_SCROLL;
				_viv_doing_x = cursor_pt.x;
				_viv_doing_y = cursor_pt.y;
				SetCapture(_viv_hwnd);
			}
			
			break;

		case 1: // play/pause slideshow
			_viv_pause();				
			break;

		case 2: // play/pause animation 
			_viv_animation_pause();
			break;

		case 3: // zoom in
			_viv_zoom_in(0,1,cursor_pt.x,cursor_pt.y);
			break;

		case 4: // next
			_viv_next(0,1,0,0);
			break;
			
		case 5: // 1:1 scroll
		
			if (_viv_doing == _VIV_DOING_NOTHING)
			{
				if (!_viv_is_fullscreen)
				{
					if (!config_show_caption)
					{
						_viv_start_move_window();
					
						return;
					}				
				}
								
				_viv_doing = _VIV_DOING_1TO1SCROLL;

				// really need to see where the cursor is..
//						ShowCursor(FALSE);

				SetCapture(_viv_hwnd);
										
				_viv_update_1to1_scroll(cursor_pt.x,cursor_pt.y);
			}
			
			break;

		case 6: // move-window
		
			{
				RECT rect;
				int wide;
				int high;
				int rw;
				int rh;
				
				GetClientRect(_viv_hwnd,&rect);
				wide = rect.right - rect.left;
				high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_view_top();

				_viv_get_render_size(&rw,&rh);

//debug_printf("MOVEWINDOW %d %d\n",rw,wide);
				
				if ((rw > wide) || (rh > high))
				{
					// scroll
					_viv_do_left_click_action(0);
	
					return;
				}
			}
		
			if (_viv_doing == _VIV_DOING_NOTHING)
			{
				if (!_viv_is_fullscreen)
				{
					_viv_start_move_window();
					
					return;
				}
			}
			break;
	}	
}
void _viv_start_move_window(void)
{
	POINT cursor_pt;
					
	GetCursorPos(&cursor_pt);
		
	SendMessage(_viv_hwnd,WM_NCLBUTTONDOWN,(WPARAM)HTCAPTION,MAKELPARAM(cursor_pt.x,cursor_pt.y));
}

// touch gesture state.
static DWORD _viv_gesture_zoom_dist = 0; // reference distance between the two fingers.
static double _viv_gesture_zoom_ratio = 1.0; // accumulated pinch ratio since the last zoom step.
static int _viv_gesture_last_x = 0; // last two finger pan location. (screen coords)
static int _viv_gesture_last_y = 0;
static BYTE _viv_gesture_have_last = 0;
int _viv_is_touch_click(void)
{
	// 0xFF515700 is the signature of touch injected mouse messages.
	return ((GetMessageExtraInfo() & 0xFFFFFF00) == 0xFF515700) ? 1 : 0;
}
void _viv_touch_double_click(void)
{
	if (_viv_1to1)
	{
		// go to best fit.
		_viv_zoom_pos = 0;
		_viv_1to1 = 0;
		_viv_view_set(0,0,1);
		InvalidateRect(_viv_hwnd,0,FALSE);
		_viv_status_update_temp_pos_zoom();
	}
	else
	{
		_viv_view_1to1();
	}
}
static void _viv_gesture_reset(void)
{
	_viv_gesture_zoom_dist = 0;
	_viv_gesture_zoom_ratio = 1.0;
	_viv_gesture_have_last = 0;
}
// returns 1 if the gesture was handled. (caller returns 0)
int _viv_on_gesture(HWND hwnd,void *gesture_info_handle)
{
	os_GestureInfo_t gesture_info;

	if (!os_GetGestureInfo)
	{
		return 0;
	}

	os_zero_memory(&gesture_info,sizeof(gesture_info));

	gesture_info.cbSize = sizeof(gesture_info);

	if (!os_GetGestureInfo(gesture_info_handle,&gesture_info))
	{
		return 0;
	}

	switch(gesture_info.dwID)
	{
		case 1: // GID_BEGIN
		case 2: // GID_END
			// msdn: application behavior is undefined when gid_begin and
			// gid_end are consumed. resetting the gesture state is all we need,
			// so hand the message to defwindowproc (which also owns the info
			// handle for anything we do not consume).
			_viv_gesture_reset();
			return 0;

		case 3: // GID_ZOOM
		{
			DWORD dist;

			dist = (DWORD)gesture_info.ullArguments;

			// sanity check the reported finger distance. some windows builds
			// return the distance with a multi monitor offset added, or as a
			// wrapped negative, which can explode the zoom ratio. a real distance
			// is positive and never exceeds the virtual screen bounds.
			{
				int vw;
				int vh;

				vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
				vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);

				if ((vw > 0) && (vh > 0) && ((dist == 0) || (dist > (DWORD)(vw + vh))))
				{
					// implausible: drop this update and keep the old baseline.
					break;
				}
			}

			// two fingers closer than ~9.5mm (36 logical px) report a distance
			// made of digitizer quantization noise: the ratio of two tiny
			// distances can be anything, so a small wobble while the fingers
			// are almost touching used to explode the accumulated pinch ratio
			// (pinch collapse, then spread: instant max zoom). freeze the zoom
			// below the floor and drop the baseline: the next valid sample
			// re-baselines with no ratio applied.
			{
				DWORD min_dist;

				min_dist = (DWORD)((36 * os_logical_wide) / 96);

				if ((dist) && (dist < min_dist))
				{
					_viv_gesture_zoom_dist = 0;

					break;
				}
			}

			if (gesture_info.dwFlags & 0x01) // GF_BEGIN
			{
				_viv_gesture_zoom_dist = dist;
				_viv_gesture_zoom_ratio = 1.0;
			}
			else
			if (_viv_gesture_zoom_dist && dist)
			{
				double ratio;

				ratio = (double)dist / (double)_viv_gesture_zoom_dist;

				// clamp the per message ratio. two consecutive messages can not
				// honestly halve or double the finger distance; larger jumps come
				// from corrupted values or heavily coalesced input and must not
				// change the zoom by more than 2x in a single message.
				if (ratio > 2.0)
				{
					ratio = 2.0;
				}
				else
				if (ratio < 0.5)
				{
					ratio = 0.5;
				}

				_viv_gesture_zoom_ratio *= ratio;

				// convert the accumulated pinch ratio into whole 1% zoom steps and
				// apply them all in one call. the render sizes form a true
				// geometric 1.01x ladder, so the image follows the fingers.
				{
					int steps_in;
					int steps_out;

					steps_in = 0;
					steps_out = 0;

					while(_viv_gesture_zoom_ratio >= 1.01)
					{
						steps_in++;

						_viv_gesture_zoom_ratio /= 1.01;
					}

					while(_viv_gesture_zoom_ratio <= (1.0 / 1.01))
					{
						steps_out++;

						_viv_gesture_zoom_ratio *= 1.01;
					}

					if (steps_in)
					{
						// _VIV_ZOOM_STEPS_PER_NOTCH steps per 120 wheel delta units.
						_viv_do_mousewheel_action(0,steps_in * (120 / _VIV_ZOOM_STEPS_PER_NOTCH),gesture_info.ptsLocation.x,gesture_info.ptsLocation.y);
					}

					if (steps_out)
					{
						_viv_do_mousewheel_action(0,-(steps_out * (120 / _VIV_ZOOM_STEPS_PER_NOTCH)),gesture_info.ptsLocation.x,gesture_info.ptsLocation.y);
					}
				}
			}

			_viv_gesture_zoom_dist = dist;

			break;
		}

		case 4: // GID_PAN (two finger pan, with inertia frames)
		{
			int x;
			int y;

			x = gesture_info.ptsLocation.x;
			y = gesture_info.ptsLocation.y;

			if (gesture_info.dwFlags & 0x01) // GF_BEGIN
			{
				_viv_gesture_last_x = x;
				_viv_gesture_last_y = y;
				_viv_gesture_have_last = 1;
			}
			else
			if (_viv_gesture_have_last)
			{
				int mx;
				int my;

				mx = x - _viv_gesture_last_x;
				my = y - _viv_gesture_last_y;

				_viv_gesture_last_x = x;
				_viv_gesture_last_y = y;

				if (mx || my)
				{
					// pan the view: the image follows the fingers.
					_viv_view_scroll(mx,my);
				}
			}

			break;
		}

		case 6: // GID_TWOFINGERTAP
		{
			// reset the zoom.
			_viv_1to1 = 0;
			_viv_zoom_pos = 0;
			_viv_view_set(_viv_view_x,_viv_view_y,1);
			InvalidateRect(_viv_hwnd,0,FALSE);
			_viv_status_update_temp_pos_zoom();

			_viv_gesture_reset();

			break;
		}

		default:

			// not handled. pass to DefWindowProc.
			return 0;
	}

	// we handled the gesture. the info handle is now our responsibility.
	if (os_CloseGestureInfoHandle)
	{
		os_CloseGestureInfoHandle(gesture_info_handle);
	}

	return 1;
}
