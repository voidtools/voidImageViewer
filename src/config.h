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

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	CONFIG_NAV_SORT_NAME=0,
	CONFIG_NAV_SORT_SIZE,
	CONFIG_NAV_SORT_DATE_MODIFIED,
	CONFIG_NAV_SORT_DATE_CREATED,
	CONFIG_NAV_SORT_FULL_PATH_AND_FILENAME,
};

#define CONFIG_KEYFLAG_CTRL					0x0100
#define CONFIG_KEYFLAG_SHIFT				0x0200
#define CONFIG_KEYFLAG_ALT					0x0400
#define CONFIG_KEYFLAG_VK_MASK				0x00ff
#define CONFIG_KEYFLAG_MOD_MASK				0xff00

// a keyboard shortcut

typedef struct config_key_s
{
	struct config_key_s *next;
	WORD key;
	
}config_key_t;

#define CONFIG_SHRINK_BLIT_MODE_COLORONCOLOR	0
#define CONFIG_SHRINK_BLIT_MODE_HALFTONE		1

#define CONFIG_MAG_FILTER_COLORONCOLOR			0
#define CONFIG_MAG_FILTER_HALFTONE				1

void config_load_settings(void);
void config_save_settings(int appdata);

extern BYTE config_appdata;
extern BYTE config_keep_centered;
extern int config_x;
extern int config_y;
extern int config_wide;
extern int config_high;
extern BYTE config_maximized;
extern int config_slideshow_rate;
extern BYTE config_allow_shrinking;
extern BYTE config_shrink_blit_mode;
extern BYTE config_mag_filter;
extern BYTE config_nav_sort;
extern BYTE config_nav_sort_ascending;
extern BYTE config_keep_aspect_ratio;
extern BYTE config_fill_window;
extern BYTE config_fullscreen_fill_window;
extern BYTE config_auto_zoom;
extern BYTE config_auto_zoom_type;
extern int config_auto_fit_wide_mul;
extern int config_auto_fit_wide_div;
extern int config_auto_fit_high_mul;
extern int config_auto_fit_high_div;
extern BYTE config_frame_minus;
extern BYTE config_multiple_instances;
extern BYTE config_show_status;
extern BYTE config_show_controls;
extern BYTE config_prevent_sleep;
extern BYTE config_loop_animations_once;
extern BYTE config_mouse_wheel_action;
extern BYTE config_ctrl_mouse_wheel_action;
extern BYTE config_left_click_action;
extern BYTE config_right_click_action;
extern BYTE config_xbutton_action;
extern BYTE config_windowed_background_color_r;
extern BYTE config_windowed_background_color_g;
extern BYTE config_windowed_background_color_b;
extern BYTE config_fullscreen_background_color_r;
extern BYTE config_fullscreen_background_color_g;
extern BYTE config_fullscreen_background_color_b;
extern BYTE config_options_last_page;
extern int config_short_jump;
extern int config_medium_jump;
extern int config_long_jump;
extern BYTE config_shuffle;
extern BYTE config_browse_file_open_dialog;
extern BYTE config_ontop;
extern int config_slideshow_custom_rate;
extern BYTE config_slideshow_custom_rate_type;
extern BYTE config_scroll_window;
extern BYTE config_preload_next;
extern BYTE config_cache_last;
extern BYTE config_icm;
extern BYTE config_show_menu;
extern BYTE config_show_caption;
extern BYTE config_show_thickframe;
extern BYTE config_toolbar_move_window;
extern BYTE config_windowed_hide_cursor;
extern BYTE config_auto_zoom;
extern BYTE config_pixel_info;
extern BYTE config_orientation;
extern BYTE config_title_bar_format;

#ifdef __cplusplus
}
#endif
