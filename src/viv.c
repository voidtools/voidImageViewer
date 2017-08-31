
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

// TODO: (*=done)
// OpenGL renderer
// Direct3D renderer
// graphics::GetHalftonePalette for 256 color mode.
// high dpi icons
// control toolbar customization
// install bmp/jpg only if the default value for HKEY_CLASSES_ROOT\.bmp is bmpfile or voidImageViewer.bmpfile -don't replace non default ones. default hard to determine for each version of Windows -avoiding for now.
// rename option?
// rotate option?
// string table for localization.
// right click -> open with ...open with, or rather get a proper context menu. CDefFolderMenu_Create2
// *fix stream for gifs, don't load the entire image before loading the gif. viv_istream_t hurts loading performance, will leave CreateStreamOnHGlobal for now.
// *allow WM_CLOSE from admin/nonadmin. 
// *sysmenu when fullscreen -needed to show size cursors
// *fixed a bug with move to/copy to
// *fixed a bug with save as filters.
// *fix ico association. -whats wrong with ico association?
// *startmenu shortcuts
// *dont use current directory so we can delete the folder of the current shown image .
// *select background window color
// *Important features I needed while testing:
// *I encountered sometimes Errors when I did Ctrl+Z for Undo. I think the image file indeed was moved back but for some reason it says error. Anyway, I did not notice exactly when it occurs. -no undo api, can be undone from Windows Explorer
// *Win + Arrows - shouldn't navigate, but just resize the window. -no longer navigate, not sure about resizing, need a keyboard with windows keys..
// *Ctrl+C Ctrl+X = Copy/Move the current displayed image file (for copying/moving to other location in Explorer, for cases you need to do it just once)
// *Ctrl+V = Add images to ImagePool (Instead of dropping, you can do Crtl+C in Explorer and then Ctrl+V in PhotoSIft)
// *Open Location of file (Show in Explorer).
// *Clear – This command needs a Hotkey (Maybe Crtl+W like in Word), and Dialog Box to approve. -added as close ctrl + W
// *Write the Dimensions (weight height) of each pic and the speed of animation.
// *Voidimageviewer has a nice feature of controlling the animation rate (speed), to make it fast/slower.
// *open clipboard to view image in clipboard? -no must be file based for now
// *Mousewheel up/down – Most of image viewers use these keys for one of two actions: 1. Next image / Previous image 2. Zoom in / Zoom out -keyboard shortcut to toggle between the two.
// *print -using preview to print ..
// *play gifs atleast once.
// *separate setting for filter quality when fullscreen/windowed. -why?
// *set as desktop background
// *SetThreadExecutionState (prevent sleep) - SC_MONITORPOWER and SC_SCREENSAVE work.
// *mipmaps for shrink resizing.. could generate mipmaps on load or as needed. -too slow. maybe do this on resize only.
// *custom slideshow rate
// *status bar to show rate changes and other information.
// *animation menu
// *portable with ini options.
// *copy to / move to menu
// *open location AND select current filename.
// *use a nice stretch when displaying the image for the first time? -always use a nice stretch.
// *alt + 1, alt + 2, alt + 3, alt + 4 window sizes
// *pressing left / right should stop slideshow? -nope
// *f11 shouldn't be a toggle.
// *when fill window is selected and then 1:1 is selected, selecting 1:1 again, should go back to fill window. 1:1 to is more of a action, and the tick is to indicate that it is currently 1:1, it should not be treated as a switch
// *animation control pause/advance frame
// *draw alpha blending images correctly (snail_test.gif)
// *mouse back forward button support
// *check current item in playlist
// *tick 1:1 in view menu when 1:1
// *COLORONCOLOR or HALFTONE mode.
// *file associations
// *multiple instances / single instance.
// *open folders (from drag drop)
// *right click -> Open file location
// *rotate? -no, this is not a photo viewer and we dont modify images.
// *file -> Properties
// *show hand when image is movable? -need custom cursor.
// *middle mouse button move.
// *gamma control -we assume monitor is set correctly and that images do not need modifying.
// *file -> print / page setup
// *really goto the next image after deleting an image.
// *use new image (IStream) instead of a file, that way we can close the file, no longer will we keep an open handle to the file.
// *nav -> list of files
// *hide cursor on fullscreen

#define VIV_SHRINK_BLIT_MODE_COLORONCOLOR	0
#define VIV_SHRINK_BLIT_MODE_HALFTONE		1
#define VIV_MAG_FILTER_COLORONCOLOR			0
#define VIV_MAG_FILTER_HALFTONE				1

#define VIV_WM_TIMER						(WM_USER)
#define VIV_WM_REPLY						(WM_USER+1)

#define VIV_KEYFLAG_CTRL					0x0100
#define VIV_KEYFLAG_SHIFT					0x0200
#define VIV_KEYFLAG_ALT						0x0400
#define VIV_KEYFLAG_VK_MASK					0x00ff
#define VIV_KEYFLAG_MOD_MASK				0xff00

#define VIV_ASSOCIATION_BMP					0x00000001
#define VIV_ASSOCIATION_GIF					0x00000002
#define VIV_ASSOCIATION_ICO					0x00000004
#define VIV_ASSOCIATION_JPEG				0x00000008
#define VIV_ASSOCIATION_JPG					0x00000010
#define VIV_ASSOCIATION_PNG					0x00000020
#define VIV_ASSOCIATION_TIF					0x00000040
#define VIV_ASSOCIATION_TIFF				0x00000080

#define VIV_ZOOM_MAX 16

#define BCM_SETSHIELD	0x0000160C

#ifdef VERSION_X64
#define VERSION_TARGET_MACHINE "(x64)"
#else
#define VERSION_TARGET_MACHINE "(x86)"
#endif

#include "viv.h"

enum
{
	VIV_REPLY_LOAD_IMAGE_COMPLETE = 0,
	VIV_REPLY_LOAD_IMAGE_FAILED,
	VIV_REPLY_LOAD_IMAGE_FIRST_FRAME,
	VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME,
};

enum
{
	VIV_NAV_SORT_NAME=0,
	VIV_NAV_SORT_SIZE,
	VIV_NAV_SORT_DATE_MODIFIED,
	VIV_NAV_SORT_DATE_CREATED,
	VIV_NAV_SORT_FULL_PATH_AND_FILENAME,
};

enum
{
	VIV_MENU_ROOT=0,
	VIV_MENU_FILE,
	VIV_MENU_EDIT,
	VIV_MENU_VIEW,
	VIV_MENU_VIEW_PRESET,
	VIV_MENU_VIEW_WINDOW_SIZE,
	VIV_MENU_VIEW_PANSCAN,
	VIV_MENU_VIEW_ZOOM,
	VIV_MENU_VIEW_ONTOP,
	VIV_MENU_SLIDESHOW,
	VIV_MENU_SLIDESHOW_RATE,
	VIV_MENU_ANIMATION,
	VIV_MENU_NAVIGATE,
	VIV_MENU_NAVIGATE_SORT,
	VIV_MENU_NAVIGATE_PLAYLIST,
	VIV_MENU_HELP,
	VIV_MENU_COUNT,
};

enum
{
	VIV_ID_EDIT_CUT = 1000,
	VIV_ID_EDIT_COPY,
	VIV_ID_EDIT_PASTE,
	VIV_ID_FILE_DELETE,
	VIV_ID_FILE_EDIT,
	VIV_ID_FILE_PREVIEW,
	VIV_ID_FILE_PRINT,
	VIV_ID_FILE_SET_DESKTOP_WALLPAPER,
	VIV_ID_FILE_CLOSE,
	VIV_ID_FILE_OPEN_FILE_LOCATION,
	VIV_ID_FILE_PROPERTIES,
	VIV_ID_EDIT_COPY_TO,
	VIV_ID_EDIT_MOVE_TO,
	
	VIV_ID_VIEW_MENU,
	VIV_ID_VIEW_STATUS,
	VIV_ID_VIEW_CONTROLS,
	VIV_ID_VIEW_PRESET_1,
	VIV_ID_VIEW_PRESET_2,
	VIV_ID_VIEW_PRESET_3,
	VIV_ID_VIEW_ALLOW_SHRINKING,
	VIV_ID_VIEW_KEEP_ASPECT_RATIO,
	VIV_ID_VIEW_FILL_WINDOW,
	VIV_ID_VIEW_1TO1,
	VIV_ID_VIEW_BESTFIT,
	VIV_ID_VIEW_FULLSCREEN,
	VIV_ID_VIEW_SLIDESHOW,
	VIV_ID_VIEW_ONTOP_ALWAYS,
	VIV_ID_VIEW_ONTOP_WHILE_PLAYING_OR_ANIMATING,
	VIV_ID_VIEW_ONTOP_NEVER,
	VIV_ID_VIEW_OPTIONS,

	VIV_ID_VIEW_WINDOW_SIZE_50,
	VIV_ID_VIEW_WINDOW_SIZE_100,
	VIV_ID_VIEW_WINDOW_SIZE_200,
	VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT,

	VIV_ID_VIEW_PANSCAN_INCREASE_SIZE,
	VIV_ID_VIEW_PANSCAN_DECREASE_SIZE,
	VIV_ID_VIEW_PANSCAN_INCREASE_WIDTH,
	VIV_ID_VIEW_PANSCAN_DECREASE_WIDTH,
	VIV_ID_VIEW_PANSCAN_INCREASE_HEIGHT,
	VIV_ID_VIEW_PANSCAN_DECREASE_HEIGHT,
	VIV_ID_VIEW_PANSCAN_MOVE_UP,
	VIV_ID_VIEW_PANSCAN_MOVE_DOWN,
	VIV_ID_VIEW_PANSCAN_MOVE_LEFT,
	VIV_ID_VIEW_PANSCAN_MOVE_RIGHT,
	VIV_ID_VIEW_PANSCAN_MOVE_UP_LEFT,
	VIV_ID_VIEW_PANSCAN_MOVE_UP_RIGHT,
	VIV_ID_VIEW_PANSCAN_MOVE_DOWN_LEFT,
	VIV_ID_VIEW_PANSCAN_MOVE_DOWN_RIGHT,
	VIV_ID_VIEW_PANSCAN_MOVE_CENTER,
	VIV_ID_VIEW_PANSCAN_RESET,

	VIV_ID_VIEW_ZOOM_IN,
	VIV_ID_VIEW_ZOOM_OUT,
	VIV_ID_VIEW_ZOOM_RESET,

	VIV_ID_SLIDESHOW_PAUSE,
	VIV_ID_SLIDESHOW_PLAY_ONLY,
	VIV_ID_SLIDESHOW_PAUSE_ONLY,
	VIV_ID_SLIDESHOW_STOP,
	VIV_ID_SLIDESHOW_RATE_DEC,
	VIV_ID_SLIDESHOW_RATE_INC,
	VIV_ID_SLIDESHOW_RATE_250,
	VIV_ID_SLIDESHOW_RATE_500,
	VIV_ID_SLIDESHOW_RATE_1000,
	VIV_ID_SLIDESHOW_RATE_2000,
	VIV_ID_SLIDESHOW_RATE_3000,
	VIV_ID_SLIDESHOW_RATE_4000,
	VIV_ID_SLIDESHOW_RATE_5000,
	VIV_ID_SLIDESHOW_RATE_6000,
	VIV_ID_SLIDESHOW_RATE_7000,
	VIV_ID_SLIDESHOW_RATE_8000,
	VIV_ID_SLIDESHOW_RATE_9000,
	VIV_ID_SLIDESHOW_RATE_10000,
	VIV_ID_SLIDESHOW_RATE_20000,
	VIV_ID_SLIDESHOW_RATE_30000,
	VIV_ID_SLIDESHOW_RATE_40000,
	VIV_ID_SLIDESHOW_RATE_50000,
	VIV_ID_SLIDESHOW_RATE_60000,
	VIV_ID_SLIDESHOW_RATE_CUSTOM,

	VIV_ID_ANIMATION_PLAY_PAUSE,
	VIV_ID_ANIMATION_JUMP_FORWARD_MEDIUM,
	VIV_ID_ANIMATION_JUMP_BACKWARD_MEDIUM,
	VIV_ID_ANIMATION_JUMP_FORWARD_SHORT,
	VIV_ID_ANIMATION_JUMP_BACKWARD_SHORT,
	VIV_ID_ANIMATION_JUMP_FORWARD_LONG,
	VIV_ID_ANIMATION_JUMP_BACKWARD_LONG,
	VIV_ID_ANIMATION_FRAME_HOME,
	VIV_ID_ANIMATION_FRAME_END,
	VIV_ID_ANIMATION_FRAME_STEP,
	VIV_ID_ANIMATION_FRAME_PREV,
	VIV_ID_ANIMATION_RATE_DEC,
	VIV_ID_ANIMATION_RATE_INC,
	VIV_ID_ANIMATION_RATE_RESET,

	VIV_ID_NAV_PREV,
	VIV_ID_NAV_NEXT,
	VIV_ID_NAV_HOME,
	VIV_ID_NAV_END,
	VIV_ID_NAV_SORT_NAME,
	VIV_ID_NAV_SORT_SIZE,
	VIV_ID_NAV_SORT_DATE_MODIFIED,
	VIV_ID_NAV_SORT_DATE_CREATED,
	VIV_ID_NAV_SORT_FULL_PATH_AND_FILENAME,
	VIV_ID_NAV_SORT_ASCENDING,
	VIV_ID_NAV_SORT_DESCENDING,
	
	VIV_ID_HELP_HELP,
	VIV_ID_HELP_COMMAND_LINE_OPTIONS,
	VIV_ID_HELP_WEBSITE,
	VIV_ID_HELP_DONATE,
	VIV_ID_HELP_ABOUT,
	
	VIV_ID_FILE_EXIT,
	
	VIV_ID_SLIDESHOW_TIMER,
	VIV_ID_HIDE_CURSOR_TIMER,
	VIV_ID_STATUS_TEMP_TEXT_TIMER,
	VIV_ID_ANIMATION_TIMER,

	VIV_ID_STATUS,
	VIV_ID_REBAR,
	VIV_ID_TOOLBAR,
	
	VIV_ID_OPTIONS_GENERAL,
	VIV_ID_OPTIONS_VIEW,
	VIV_ID_OPTIONS_CONTROLS,

	VIV_ID_FILE_OPEN_FILE,
	VIV_ID_FILE_OPEN_FOLDER,
	VIV_ID_FILE_ADD_FILE,
	VIV_ID_FILE_ADD_FOLDER,
};

#define VIV_ID_NAV_PLAYLIST_START	10000
#define VIV_ID_NAV_PLAYLIST_END		20000 // exclusive.

#define VIV_HIDE_CURSOR_DELAY		2000

// a reply from the image load thread

typedef struct viv_reply_s
{
	struct viv_reply_s *next;
	DWORD type;
	DWORD size;
	// data follows
	
}viv_reply_t;

// a keyboard shortcut

typedef struct viv_key_s
{
	struct viv_key_s *next;
	WORD key;
	
}viv_key_t;

// a frame in the image.
// there might be more than one.

typedef struct viv_frame_s
{
	HBITMAP hbitmap;
	
}viv_frame_t;

typedef struct viv_reply_load_image_first_frame_s
{
	UINT wide;
	UINT high;
	UINT frame_count;
	viv_frame_t frame;
	DWORD *frame_delays;
	
}viv_reply_load_image_first_frame_t;

typedef struct viv_nav_item_s
{
	WIN32_FIND_DATA fd;
	struct viv_nav_item_s *next;
	
}viv_nav_item_t;

typedef struct viv_dragdrop_s
{
	struct viv_dragdrop_s *next;
	struct viv_dragdrop_s *prev;
	WIN32_FIND_DATA fd;
	
}viv_dragdrop_t;

typedef struct viv_command_s
{
	const char *name;
	WORD flags;
	BYTE menu_id;
	WORD command_id;
}viv_command_t;

typedef struct viv_default_key_s
{
	WORD command_id;
	WORD key_flags;
	
}viv_default_key_t;

static void viv_update_title(void);
static void viv_on_size(void);
static LRESULT CALLBACK viv_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static LRESULT CALLBACK viv_fullscreen_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void viv_command(int command);
static void viv_load_settings(void);
static int viv_is_ws(wchar_t c);
static wchar_t *viv_skip_ws(wchar_t *p);
static wchar_t *viv_get_word(wchar_t *p,wchar_t *buf);
static int viv_process_install_command_line_options(wchar_t *cl);
static void viv_process_command_line(wchar_t *cl);
static void viv_save_settings_by_location(const wchar_t *path,int is_root);
static void viv_save_settings(int appdata);
static int viv_init(void);
static void viv_kill(void);
static void viv_exit(void);
static void viv_next(int prev,int reset_slideshow_timer);
static void viv_home(int end);
static int viv_is_valid_filename(wchar_t *s);
static int viv_is_msg(MSG *msg);
static void viv_view_set(int view_x,int view_y,int invalidate);
static void viv_slideshow(void);
static void viv_toggle_fullscreen(void);
static void viv_get_render_size(int *prw,int *prh);
static void viv_set_custom_rate(void);
static void viv_set_rate(int rate);
static void viv_delete(void);
static void viv_copy(int cut);
static int viv_is_key_state(int control,int shift,int alt);
static CLIPFORMAT _viv_get_CF_PREFERREDDROPEFFECT(void);
static void viv_pause(void);
static void viv_write_int(HANDLE h,const char *ascii_key,int value);
static void viv_write_string(HANDLE h,const char *ascii_key,const wchar_t *s);
static void viv_write_utf8(HANDLE h,const utf8_t *s);
static int viv_compare(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b);
static void viv_open(WIN32_FIND_DATA *fd);
static void viv_open_from_filename(const wchar_t *filename);
static void viv_increase_rate(int dec);
static void viv_file_preview(void);
static void viv_file_print(void);
static void viv_file_set_desktop_wallpaper(void);
static void viv_file_edit(void);
static void viv_open_file_location(void);
static void viv_properties(void);
static void viv_doing_cancel(void);
static const char *_viv_get_copydata_string(const char *p,const char *e,wchar_t *buf,int bufsize);
static void viv_nav_item_free_all(void);
static int viv_nav_playlist_compare(const void *a,const void *b);
static void viv_blank(void);
static void viv_options(void);
static INT_PTR CALLBACK viv_options_proc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK viv_options_general_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK viv_options_view_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK viv_options_controls_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void viv_install_association_by_extension(const char *association,const char *description,const char *icon_location);
static void viv_uninstall_association_by_extension(const char *association);
static int viv_is_association(const char *association);
static int viv_get_registry_string(HKEY hkey,const utf8_t *value,wchar_t *wbuf,int size_in_wchars);
static int viv_set_registry_string(HKEY hkey,const utf8_t *value,const wchar_t *wbuf);
static void viv_check_menus(HMENU hmenu);
static void viv_increase_animation_rate(int dec);
static void viv_reset_animation_rate(void);
static void viv_timer_start(void);
static void viv_show_cursor(void);
static void viv_animation_pause(void);
static void viv_frame_step(void);
static void viv_frame_prev(void);
static void viv_view_1to1(void);
static void viv_dragdrop_clearall(void);
static viv_dragdrop_t *viv_dragdrop_add(const WIN32_FIND_DATA *fd);
static void viv_dragdrop_add_path(const wchar_t *full_path_and_filename);
static void viv_dragdrop_add_filename(const wchar_t *filename);
static void viv_dragdrop_delete(const wchar_t *filename);
static void viv_timer_stop(void);
static int viv_is_window_maximized(HWND hwnd);
static void viv_load_settings_by_location(const wchar_t *path,int is_root);
static INT_PTR CALLBACK viv_custom_rate_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK viv_about_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void viv_update_frame(void);
static void viv_update_ontop(void);
static void viv_dst_pos_set(int x,int y);
static void viv_dst_zoom_set(int x,int y);
static void viv_frame_skip(int size);
static DWORD WINAPI viv_load_image_proc(void *param);
static viv_reply_t *viv_reply_add(DWORD type,DWORD size,void *data);
static void viv_reply_clear_all(void);
static void viv_reply_free(viv_reply_t *e);
static void viv_controls_show(int show);
static void viv_status_show(int show);
static void viv_status_update(void);
static void viv_status_set(int part,const wchar_t *text);
static int viv_get_status_high(void);
static int viv_get_controls_high(void);
static LRESULT CALLBACK viv_rebar_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static int viv_toolbar_get_wide(void);
static void viv_toolbar_update_buttons(void);
static void viv_status_set_temp_text(wchar_t *text);
static void viv_status_update_temp_pos_zoom(void);
static void viv_status_update_temp_animation_rate(void);
static void viv_zoom_in(int out,int have_xy,int x,int y);
static void viv_status_update_slideshow_rate(void);
static void viv_options_treeview_changed(HWND hwnd);
static void viv_command_line_options(void);
static void viv_install_association(DWORD flags);
static void viv_uninstall_association(DWORD flags);
static void viv_view_scroll(int mx,int my);
static void viv_options_update_sheild(HWND hwnd);
static void viv_update_color_button_bitmap(HWND hwnd);
static void viv_delete_color_button_bitmap(HWND hwnd);
static int viv_convert_menu_ini_name_ch(int ch);
static utf8_t *viv_cat_command_menu_ini_name_path(utf8_t *buf,int menu_index);
static void viv_menu_name_to_ini_name(utf8_t *buf,int command_index);
static void viv_get_menu_display_name(wchar_t *buf,const utf8_t *menu_name);
static void viv_get_command_name(wchar_t *wbuf,int command_index);
static int viv_command_index_from_command_id(int command_id);
static void viv_cat_command_menu_path(wchar_t *wbuf,int menu_index);
static int viv_vk_to_text(wchar_t *wbuf,int vk);
static void viv_cat_key_mod(wchar_t *wbuf,int vk,const utf8_t *default_keytext);
static void viv_get_key_text(wchar_t *wbuf,DWORD keyflags);
static HMENU viv_create_menu(void);
static void viv_key_add(struct viv_key_list_s *key_list,int command_index,DWORD keyflags);
static void viv_key_clear_all(struct viv_key_list_s *key_list,int command_index);
static void viv_key_list_copy(struct viv_key_list_s *dst,const struct viv_key_list_s *src);
static void viv_key_list_clear(struct viv_key_list_s *list);
static void viv_key_list_init(struct viv_key_list_s *list);
static void viv_options_key_list_sel_change(HWND hwnd,int previous_key_index);
static void viv_options_remove_key(HWND hwnd);
static void viv_options_edit_key(HWND hwnd,int key_index);
static INT_PTR CALLBACK viv_edit_key_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static LRESULT CALLBACK _viv_edit_key_edit_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static int viv_get_current_key_mod_flags(void);
static void viv_options_edit_key_changed(HWND hwnd);
static void viv_edit_key_set_key(HWND hwnd,DWORD key_flags);
static void viv_key_remove(struct viv_key_list_s *keylist,int command_index,DWORD keyflags);
static void viv_edit_key_remove_currently_used_by(struct viv_key_list_s *keylist,DWORD keyflags);
static void viv_close_existing_process(void);
static int viv_get_appdata_voidimageviewer_path(wchar_t *wbuf);
static void viv_uninstall_delete_file(const wchar_t *path,const utf8_t *filename);
static int viv_is_start_menu_shortcuts(void);
static void viv_install_start_menu_shortcuts(void);
static void viv_uninstall_start_menu_shortcuts(void);
static void viv_append_admin_param(wchar_t *wbuf,const utf8_t *param);
static void viv_install_copy_file(const wchar_t *install_path,const wchar_t *temp_path,const utf8_t *filename,int critical);

static HMODULE _viv_stobject_hmodule = 0; // module for setdesktopwallpaper verb.
static int viv_keep_centered = 1; // when zooming out, don't recenter the image. (keep cursor under the same pixel)
static viv_dragdrop_t *viv_dragdrop_start = 0; // a list of files that were dropped on us.
static viv_dragdrop_t *viv_dragdrop_last = 0; // a list of files that were dropped on us.
static HWND viv_hwnd = 0; // the main hwnd
static HWND viv_status_hwnd = 0; // the status bar hwnd
static HWND viv_toolbar_hwnd = 0; // the toolbar hwnd
static HWND viv_rebar_hwnd = 0; // our rebar window
static HIMAGELIST viv_toolbar_image_list = 0; // the toolbar imagelist containing the button graphics.
static HANDLE viv_mutex = 0; // the MUTEX to allow on a single instance, if this mutex exists, we will exit immediately after passing our command line arguments to the existing instance.
static int viv_x = 0; // window position in screen coords
static int viv_y = 0; // window position in screen coords
static int viv_wide = 640; // window width (not client width)
static int viv_high = 480; // window height (not client width)
static int viv_ontop = 0; // 0 = never, 1 = always, 2 = while slideshow or animating.
static int viv_slideshow_rate = 5000; // rate of change in slideshow mode in milliseconds.
static int viv_slideshow_custom_rate = 3; // custom slideshow rate (see type below)
static int viv_slideshow_custom_rate_type = 1; // 0 = milliseconds, 1 = seconds, 2 = minutes
//static float viv_animation_rates[] = {0.085899f,0.107374f,0.134218f,0.167772f,0.209715f,0.262144f,0.327680f,0.409600f,0.512000f,0.640000f,0.800000f,1.000000f,1.250000f,1.562500f,1.953125f,2.441406f,3.051758f,3.814697f,4.768372f,5.960464f,7.450581f,9.313226f}; // natural curve.
static float viv_animation_rates[] = {0.125000f,0.142857f,0.166667f,0.200000f,0.250000f,0.333333f,0.500000f,0.571429f,0.666667f,0.800000f,1.000000f,1.250000f,1.500000f,1.750000f,2.000000f,3.000000f,4.000000f,5.000000f,6.000000f,7.000000f,8.000000f}; // fixed animation rates
#define VIV_ANIMATION_RATE_MAX	(sizeof(viv_animation_rates) / sizeof(float))
#define VIV_ANIMATION_RATE_ONE	10
static int viv_animation_rate_pos = VIV_ANIMATION_RATE_ONE;
static int viv_animation_play = 1; // play or pause animations
static int viv_allow_shrinking = 1; // prevent resizing an image below 100%
static int viv_shrink_blit_mode = VIV_SHRINK_BLIT_MODE_HALFTONE; // shrink filter
static int viv_mag_filter = VIV_MAG_FILTER_COLORONCOLOR; // magnify filter
static int viv_nav_sort = VIV_NAV_SORT_DATE_MODIFIED; // current navigation sort.
static int viv_nav_sort_ascending = 0; // sort navigation ascending or descending.
static int viv_keep_aspect_ratio = 1; // stretch images with the original aspect ratio.
static int viv_fill_window = 0; // stretch the image to fill the window
static int viv_fullscreen_fill_window = 1; // same as fill_window, except this setting is used when we are fullscreen
static int viv_1to1 = 0; // temporarily show the image with 100% scaling
static int viv_have_old_zoom = 0; // restore this zoom level after leaving 1:1 mode.
static int viv_old_zoom_pos = 0; // restore this zoom level after leaving 1:1 mode.
static int viv_auto_zoom = 0; // automatically resize the window to fit the newly loaded image
static int viv_auto_zoom_type = 1; // 0 = 50%, 1 = 100%, 2 = 200%
static int viv_frame_minus = 0; // show frame counter or remaining frames in status bar
static int viv_multiple_instances = 0; // all multiple instances or use a single instance.
static int viv_appdata = 0; // store settings in %APPDATA%\voidimageviewer or in the same location as voidimageviewer.exe
WIN32_FIND_DATA *viv_current_fd; // the current image find data including the full path and filename.
static int viv_view_x = 0; // the current image offset in pixels
static int viv_view_y = 0; // the current image offset in pixels
static double viv_view_ix = 0.0; // the current image offset in percent, used when resizing the window
static double viv_view_iy = 0.0; // the current image offset in percent, used when resizing the window
static int viv_zoom_pos = 0; // the current zoom level
static float viv_zoom_presets[VIV_ZOOM_MAX] = {0.004815f,0.019215f,0.043060f,0.076120f,0.118079f,0.168530f,0.226989f,0.292893f,0.365607f,0.444430f,0.528603f,0.617316f,0.709715f,0.804909f,0.901983f,1.000000f}; // (1 - cos(((float)(x+1) * 1.570796f) / VIV_ZOOM_MAX)) // this is missing cos((1 * 1.570796f) / VIV_ZOOM_MAX), which is too small
static ULONG_PTR os_GdiplusToken; // gdiplus handle
static int viv_image_wide = 0; // current image width
static int viv_image_high = 0; // current image width
static int viv_frame_count = 0; // current image frame count, 1 for static image, > 1 for animation
static int viv_frame_loaded_count = 0; // number of loaded frames, can be less than viv_frame_count
static DWORD *viv_frame_delays = 0; // the delay in milliseconds before showing the next frame.
static int viv_frame_position = 0; // the current frame position
static int viv_frame_looped = 0; // all frames have been displayed for this animation
static int viv_is_slideshow_timeup = 0; // the slideshow timer has expired, but we are still showing an animation at least once.
static viv_frame_t *viv_frames = 0; // the frames that make up an image, could be more than one for animations.
static DWORD viv_timer_tick = 0; // the current tick for the current frame.
static int viv_is_animation_timer = 0; // animation timer started?
static DWORD viv_animation_timer_tick_start = 0; // the current start tick
static int viv_doing = 0; // mouse action, such as drag to scroll image
static int viv_doing_x;
static int viv_doing_y;
static int viv_mdoing_x;
static int viv_mdoing_y;
static int viv_fullscreen_is_maxed = 0;
static int viv_is_fullscreen = 0;
static RECT viv_fullscreen_rect;
static int viv_is_show_menu = 1;
static int viv_is_show_status = 1;
static int viv_is_show_controls = 1;
static int viv_is_show_caption = 1;
static int viv_is_show_thickframe = 1;
static int viv_is_slideshow = 0;
static int viv_allow_sleep = 1;
static int viv_is_hide_cursor_timer = 0;
static CLIPFORMAT _viv_CF_PREFERREDDROPEFFECT = 0; // copy or move? clipboard operation
static int viv_predefined_rates[] = {250,500,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000,20000,30000,40000,50000,60000};
#define VIV_PREDEFINED_RATE_COUNT (sizeof(viv_predefined_rates) / sizeof(int))
static HMENU viv_nav_playlist_menu;
static viv_nav_item_t **viv_nav_items = 0;
static int viv_nav_item_count = 0;
static int viv_cursor_shown = 1;
static int viv_hide_cursor_x = -1;
static int viv_hide_cursor_y = -1;
static int viv_in_popup_menu = 0;
static GUID viv_FrameDimensionTime = {0x6aedbd6d,0x3fb5,0x418a,{0x83,0xa6,0x7f,0x45,0x22,0x9d,0xc8,0x72}};
static int viv_loop_animations_once = 1;
static HMENU viv_hmenu = 0;
static int viv_dst_pos_x = 500;
static int viv_dst_pos_y = 500;
static float viv_dst_zoom_values[] = {0.197145f,0.20108f,0.20511f,0.20921f,0.21339f,0.21766f,0.22201f,0.22645f,0.23098f,0.23560f,0.24031f,0.24512f,0.25002f,0.25502f,0.26012f,0.26533f,0.27063f,0.27605f,0.28157f,0.28720f,0.29294f,0.29880f,0.30478f,0.31087f,0.31709f,0.32343f,0.32990f,0.33650f,0.34323f,0.35009f,0.35710f,0.36424f,0.37152f,0.37895f,0.38653f,0.39426f,0.40215f,0.41019f,0.41840f,0.42676f,0.43530f,0.44401f,0.45289f,0.46194f,0.47118f,0.48061f,0.49022f,0.50002f,0.51002f,0.52022f,0.53063f,0.54124f,0.55207f,0.56311f,0.57437f,0.58586f,0.59758f,0.60953f,0.62172f,0.63415f,0.64683f,0.65977f,0.67297f,0.68643f,0.70016f,0.71416f,0.72844f,0.74301f,0.75787f,0.77303f,0.78849f,0.80426f,0.82034f,0.83675f,0.85349f,0.87056f,0.88797f,0.90573f,0.92384f,0.94232f,0.96116f,0.98039f,1.00000f,1.02000f,1.04040f,1.06120f,1.08243f,1.10408f,1.12616f,1.14868f,1.17165f,1.19509f,1.21899f,1.24337f,1.26824f,1.29360f,1.31947f,1.34586f,1.37278f,1.40024f,1.42824f,1.45681f,1.48594f,1.51566f,1.54597f,1.57689f,1.60843f,1.64060f,1.67341f,1.70688f,1.74102f,1.77584f,1.81136f,1.84758f,1.88453f,1.92223f,1.96067f,1.99988f,2.03988f,2.08068f,2.12229f,2.16474f,2.20803f,2.25219f,2.29724f,2.34318f,2.39005f,2.43785f,2.48660f,2.53634f,2.58706f,2.63880f,2.69158f,2.74541f,2.80032f,2.85633f,2.91345f,2.97172f,3.03116f};
#define VIV_DST_ZOOM_ONE	82
#define VIV_DST_ZOOM_MAX	(sizeof(viv_dst_zoom_values) / sizeof(float))
static int viv_dst_zoom_x_pos = VIV_DST_ZOOM_ONE;
static int viv_dst_zoom_y_pos = VIV_DST_ZOOM_ONE;
CRITICAL_SECTION viv_cs;
HANDLE viv_load_image_thread = 0;
static wchar_t *viv_load_image_filename = 0;
static WIN32_FIND_DATA *viv_load_image_next_fd = 0;
static volatile int viv_load_image_terminate = 0;
static viv_reply_t *viv_reply_start = 0;
static viv_reply_t *viv_reply_last = 0;
static wchar_t *viv_status_temp_text = 0;
static int viv_mouse_wheel_action = 0; // 0 = zoom, 1 = next/prev, 2=prev/next
static int viv_left_click_action = 0; // 0 = scroll, 1 = play/pause slideshow, 2 = play/pause animation, 3=zoom in, 4=next 
static int viv_right_click_action = 0; // 0 = context menu, 1=zoom out, 2=prev, 
static const utf8_t *viv_options_page_names[] = {"General","View","Controls"};
static int viv_options_tab_ids[] = {IDC_TAB1,IDC_TAB2,IDC_TAB3};
static int viv_options_dialog_ids[] = {IDD_GENERAL,IDD_VIEW,IDD_CONTROLS};
#define VIV_OPTIONS_PAGE_COUNT	(sizeof(viv_options_dialog_ids) / sizeof(int))
static int viv_options_page_ids[] = {VIV_ID_OPTIONS_GENERAL,VIV_ID_OPTIONS_VIEW,VIV_ID_OPTIONS_CONTROLS};
static DLGPROC viv_options_page_procs[] = {viv_options_general_proc,viv_options_view_proc,viv_options_controls_proc};
static HFONT viv_about_hfont = 0;
static int viv_windowed_background_color_r = 255;
static int viv_windowed_background_color_g = 255;
static int viv_windowed_background_color_b = 255;
static int viv_fullscreen_background_color_r = 0;
static int viv_fullscreen_background_color_g = 0;
static int viv_fullscreen_background_color_b = 0;
static int viv_options_last_page = 0;
static int viv_short_jump = 500;
static int viv_medium_jump = 1000;
static int viv_long_jump = 2000;
static wchar_t *viv_last_open_file = 0;
static wchar_t *viv_last_open_folder = 0;

static viv_command_t viv_commands[] = 
{
	{"&File",MF_POPUP,VIV_MENU_ROOT,VIV_MENU_FILE},

	{"&Open File...",MF_STRING,VIV_MENU_FILE,VIV_ID_FILE_OPEN_FILE},
	{"&Open Folder...",MF_STRING,VIV_MENU_FILE,VIV_ID_FILE_OPEN_FOLDER},
	{"&Add File...",MF_STRING|MF_OWNERDRAW,VIV_MENU_FILE,VIV_ID_FILE_ADD_FILE},
	{"&Add Folder...",MF_STRING|MF_OWNERDRAW,VIV_MENU_FILE,VIV_ID_FILE_ADD_FOLDER},
	{0,MF_SEPARATOR,VIV_MENU_FILE,0},
	{"Open F&ile Location...",MF_STRING,VIV_MENU_FILE,VIV_ID_FILE_OPEN_FILE_LOCATION},
	{"&Edit...",MF_STRING,VIV_MENU_FILE,VIV_ID_FILE_EDIT},
	{"Pre&view...",MF_STRING,VIV_MENU_FILE,VIV_ID_FILE_PREVIEW},
	{"&Print...",MF_STRING,VIV_MENU_FILE,VIV_ID_FILE_PRINT},
	{"Set &Desktop Wallpaper",MF_STRING,VIV_MENU_FILE,VIV_ID_FILE_SET_DESKTOP_WALLPAPER},
	{"&Close",MF_STRING,VIV_MENU_FILE,VIV_ID_FILE_CLOSE},
	{0,MF_SEPARATOR,VIV_MENU_FILE,0},
	{"&Delete",MF_STRING,VIV_MENU_FILE,VIV_ID_FILE_DELETE},
	{"P&roperties",MF_STRING,VIV_MENU_FILE,VIV_ID_FILE_PROPERTIES},
	{0,MF_SEPARATOR,VIV_MENU_FILE,0},
	{"E&xit",MF_STRING,VIV_MENU_FILE,VIV_ID_FILE_EXIT},
	
	{"&Edit",MF_POPUP,VIV_MENU_ROOT,VIV_MENU_EDIT},

	{"Cu&t",MF_STRING,VIV_MENU_EDIT,VIV_ID_EDIT_CUT},
	{"&Copy",MF_STRING,VIV_MENU_EDIT,VIV_ID_EDIT_COPY},
	{"&Paste",MF_STRING|MF_OWNERDRAW,VIV_MENU_EDIT,VIV_ID_EDIT_PASTE},
	{0,MF_SEPARATOR,VIV_MENU_EDIT,0},
	{"Copy to &folder...",MF_STRING,VIV_MENU_EDIT,VIV_ID_EDIT_COPY_TO},
	{"Mo&ve to folder...",MF_STRING,VIV_MENU_EDIT,VIV_ID_EDIT_MOVE_TO},

	{"&View",MF_POPUP,VIV_MENU_ROOT,VIV_MENU_VIEW},
	
	{"&Menu",MF_STRING,VIV_MENU_VIEW,VIV_ID_VIEW_MENU},
	{"Status &Bar",MF_STRING,VIV_MENU_VIEW,VIV_ID_VIEW_STATUS},
	{"&Controls",MF_STRING,VIV_MENU_VIEW,VIV_ID_VIEW_CONTROLS},
	{0,MF_SEPARATOR,VIV_MENU_VIEW,0},
	{"&Preset",MF_POPUP,VIV_MENU_VIEW,VIV_MENU_VIEW_PRESET},
	{"&Minimal",MF_STRING,VIV_MENU_VIEW_PRESET,VIV_ID_VIEW_PRESET_1},
	{"&Compact",MF_STRING,VIV_MENU_VIEW_PRESET,VIV_ID_VIEW_PRESET_2},
	{"&Normal",MF_STRING,VIV_MENU_VIEW_PRESET,VIV_ID_VIEW_PRESET_3},
	{0,MF_SEPARATOR,VIV_MENU_VIEW,0},
	{"&Allow shrinking",MF_STRING,VIV_MENU_VIEW,VIV_ID_VIEW_ALLOW_SHRINKING},
	{"&Keep aspect ratio",MF_STRING,VIV_MENU_VIEW,VIV_ID_VIEW_KEEP_ASPECT_RATIO},
	{"&Fill Window",MF_STRING,VIV_MENU_VIEW,VIV_ID_VIEW_FILL_WINDOW},
	{"1:1",MF_STRING,VIV_MENU_VIEW,VIV_ID_VIEW_1TO1},
	{"&Best fit",MF_STRING|MF_OWNERDRAW,VIV_MENU_VIEW,VIV_ID_VIEW_BESTFIT},
	{"F&ullscreen",MF_STRING,VIV_MENU_VIEW,VIV_ID_VIEW_FULLSCREEN},
	{"&Slideshow",MF_STRING,VIV_MENU_VIEW,VIV_ID_VIEW_SLIDESHOW},
	{0,MF_SEPARATOR,VIV_MENU_VIEW,0},
	{"&Window Size",MF_POPUP,VIV_MENU_VIEW,VIV_MENU_VIEW_WINDOW_SIZE},
	{"50%",MF_STRING,VIV_MENU_VIEW_WINDOW_SIZE,VIV_ID_VIEW_WINDOW_SIZE_50},
	{"100%",MF_STRING,VIV_MENU_VIEW_WINDOW_SIZE,VIV_ID_VIEW_WINDOW_SIZE_100},
	{"200%",MF_STRING,VIV_MENU_VIEW_WINDOW_SIZE,VIV_ID_VIEW_WINDOW_SIZE_200},
	{"&Auto Fit",MF_STRING,VIV_MENU_VIEW_WINDOW_SIZE,VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT},
	{0,MF_SEPARATOR,VIV_MENU_VIEW,0},
	{"Pa&n && Scan",MF_POPUP,VIV_MENU_VIEW,VIV_MENU_VIEW_PANSCAN},
	{"&Increase Size",MF_STRING,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_INCREASE_SIZE},
	{"&Decrease Size",MF_STRING,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_DECREASE_SIZE},
	{"I&ncrease Width",MF_STRING,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_INCREASE_WIDTH},
	{"D&ecrease Width",MF_STRING,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_DECREASE_WIDTH},
	{"In&crease Height",MF_STRING,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_INCREASE_HEIGHT},
	{"De&cre&ase Height",MF_STRING,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_DECREASE_HEIGHT},
	{0,MF_SEPARATOR,VIV_MENU_VIEW_PANSCAN,0},
	{"Move &Up",MF_STRING,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_UP},
	{"Move &Down",MF_STRING,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_DOWN},
	{"Move &Left",MF_STRING,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_LEFT},
	{"Move &Right",MF_STRING,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_RIGHT},
	{"Move Up Left",MF_STRING|MF_OWNERDRAW,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_UP_LEFT},
	{"Move Up Right",MF_STRING|MF_OWNERDRAW,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_UP_RIGHT},
	{"Move Down Left",MF_STRING|MF_OWNERDRAW,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_DOWN_LEFT},
	{"Move Down Right",MF_STRING|MF_OWNERDRAW,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_DOWN_RIGHT},
	{"Move Cen&ter",MF_STRING,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_CENTER},
	{0,MF_SEPARATOR,VIV_MENU_VIEW_PANSCAN,0},
	{"Re&set",MF_STRING,VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_RESET},
	{0,MF_SEPARATOR,VIV_MENU_VIEW,0},
	{"&Zoom",MF_POPUP,VIV_MENU_VIEW,VIV_MENU_VIEW_ZOOM},
	{"Zoom &In",MF_STRING,VIV_MENU_VIEW_ZOOM,VIV_ID_VIEW_ZOOM_IN},
	{"Zoom &Out",MF_STRING,VIV_MENU_VIEW_ZOOM,VIV_ID_VIEW_ZOOM_OUT},
	{"&Reset",MF_STRING,VIV_MENU_VIEW_ZOOM,VIV_ID_VIEW_ZOOM_RESET},
	{0,MF_SEPARATOR,VIV_MENU_VIEW,0},
	{"On &Top",MF_POPUP,VIV_MENU_VIEW,VIV_MENU_VIEW_ONTOP},
	{"&Always",MF_STRING|MFT_RADIOCHECK,VIV_MENU_VIEW_ONTOP,VIV_ID_VIEW_ONTOP_ALWAYS},
	{"&While playing slideshow or animating",MF_STRING|MFT_RADIOCHECK,VIV_MENU_VIEW_ONTOP,VIV_ID_VIEW_ONTOP_WHILE_PLAYING_OR_ANIMATING},
	{"&Never",MF_STRING|MFT_RADIOCHECK,VIV_MENU_VIEW_ONTOP,VIV_ID_VIEW_ONTOP_NEVER},
	{"&Options",MF_STRING,VIV_MENU_VIEW,VIV_ID_VIEW_OPTIONS},
	{"&Slideshow",MF_POPUP,VIV_MENU_ROOT,VIV_MENU_SLIDESHOW},
	{"&Play/Pause",MF_STRING,VIV_MENU_SLIDESHOW,VIV_ID_SLIDESHOW_PAUSE},
	{0,MF_SEPARATOR,VIV_MENU_SLIDESHOW,0},
	{"&Rate",MF_POPUP,VIV_MENU_SLIDESHOW,VIV_MENU_SLIDESHOW_RATE},
	{"&Decrease Rate",MF_STRING,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_DEC},
	{"&Increase Rate",MF_STRING,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_INC},
	{0,MF_SEPARATOR,VIV_MENU_SLIDESHOW_RATE,0},
	{"250 Milliseconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_250},
	{"500 Milliseconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_500},
	{"&1 Second",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_1000},
	{"&2 Seconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_2000},
	{"&3 Seconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_3000},
	{"&4 Seconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_4000},
	{"&5 Seconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_5000},
	{"&6 Seconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_6000},
	{"&7 Seconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_7000},
	{"&8 Seconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_8000},
	{"&9 Seconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_9000},
	{"1&0 Seconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_10000},
	{"20 Seconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_20000},
	{"30 Seconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_30000},
	{"40 Seconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_40000},
	{"50 Seconds",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_50000},
	{"1 Minute",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_60000},
	{"Custom...",MF_STRING|MFT_RADIOCHECK,VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_CUSTOM},

	{"&Animation",MF_POPUP,VIV_MENU_ROOT,VIV_MENU_ANIMATION},
	{"&Play/Pause",MF_STRING,VIV_MENU_ANIMATION,VIV_ID_ANIMATION_PLAY_PAUSE},
	{0,MF_SEPARATOR,VIV_MENU_ANIMATION,0},
	{"Jump &Forward",MF_STRING,VIV_MENU_ANIMATION,VIV_ID_ANIMATION_JUMP_FORWARD_MEDIUM},
	{"Jump &Backward",MF_STRING,VIV_MENU_ANIMATION,VIV_ID_ANIMATION_JUMP_BACKWARD_MEDIUM},
	{"Short Jump &Forward",MF_STRING|MF_OWNERDRAW,VIV_MENU_ANIMATION,VIV_ID_ANIMATION_JUMP_FORWARD_SHORT},
	{"Short Jump &Backward",MF_STRING|MF_OWNERDRAW,VIV_MENU_ANIMATION,VIV_ID_ANIMATION_JUMP_BACKWARD_SHORT},
	{"Long Jump &Forward",MF_STRING|MF_OWNERDRAW,VIV_MENU_ANIMATION,VIV_ID_ANIMATION_JUMP_FORWARD_LONG},
	{"Long Jump &Backward",MF_STRING|MF_OWNERDRAW,VIV_MENU_ANIMATION,VIV_ID_ANIMATION_JUMP_BACKWARD_LONG},
	{0,MF_SEPARATOR,VIV_MENU_ANIMATION,0},
	{"F&rame Step",MF_STRING,VIV_MENU_ANIMATION,VIV_ID_ANIMATION_FRAME_STEP},
	{"Pre&vious Frame",MF_STRING,VIV_MENU_ANIMATION,VIV_ID_ANIMATION_FRAME_PREV},
	{"F&irst Frame",MF_STRING,VIV_MENU_ANIMATION,VIV_ID_ANIMATION_FRAME_HOME},
	{"&Last Frame",MF_STRING,VIV_MENU_ANIMATION,VIV_ID_ANIMATION_FRAME_END},
	{0,MF_SEPARATOR,VIV_MENU_ANIMATION,0},
	{"&Decrease Rate",MF_STRING,VIV_MENU_ANIMATION,VIV_ID_ANIMATION_RATE_DEC},
	{"&Increase Rate",MF_STRING,VIV_MENU_ANIMATION,VIV_ID_ANIMATION_RATE_INC},
	{"R&eset Rate",MF_STRING,VIV_MENU_ANIMATION,VIV_ID_ANIMATION_RATE_RESET},
	
	{"&Navigate",MF_POPUP,VIV_MENU_ROOT,VIV_MENU_NAVIGATE},
	{"&Next",MF_STRING,VIV_MENU_NAVIGATE,VIV_ID_NAV_NEXT},
	{"P&revious",MF_STRING,VIV_MENU_NAVIGATE,VIV_ID_NAV_PREV},
	{"&Home",MF_STRING,VIV_MENU_NAVIGATE,VIV_ID_NAV_HOME},
	{"&End",MF_STRING,VIV_MENU_NAVIGATE,VIV_ID_NAV_END},
	{0,MF_SEPARATOR,VIV_MENU_NAVIGATE,0},
	{"S&ort",MF_POPUP,VIV_MENU_NAVIGATE,VIV_MENU_NAVIGATE_SORT},
	{"&Name",MF_STRING|MFT_RADIOCHECK,VIV_MENU_NAVIGATE_SORT,VIV_ID_NAV_SORT_NAME},
	{"Full &Path and Filename",MF_STRING|MFT_RADIOCHECK,VIV_MENU_NAVIGATE_SORT,VIV_ID_NAV_SORT_FULL_PATH_AND_FILENAME},
	{"&Size",MF_STRING|MFT_RADIOCHECK,VIV_MENU_NAVIGATE_SORT,VIV_ID_NAV_SORT_SIZE},
	{"Date &Modified",MF_STRING|MFT_RADIOCHECK,VIV_MENU_NAVIGATE_SORT,VIV_ID_NAV_SORT_DATE_MODIFIED},
	{"Date &Created",MF_STRING|MFT_RADIOCHECK,VIV_MENU_NAVIGATE_SORT,VIV_ID_NAV_SORT_DATE_CREATED},
	{0,MF_SEPARATOR,VIV_MENU_NAVIGATE_SORT,0},
	{"&Ascending",MF_STRING|MFT_RADIOCHECK,VIV_MENU_NAVIGATE_SORT,VIV_ID_NAV_SORT_ASCENDING},
	{"&Descending",MF_STRING|MFT_RADIOCHECK,VIV_MENU_NAVIGATE_SORT,VIV_ID_NAV_SORT_DESCENDING},
	{0,MF_SEPARATOR,VIV_MENU_NAVIGATE,0},
	{"&Playlist",MF_POPUP,VIV_MENU_NAVIGATE,VIV_MENU_NAVIGATE_PLAYLIST},

	{"&Help",MF_POPUP,VIV_MENU_ROOT,VIV_MENU_HELP},
	{"&Help",MF_STRING,VIV_MENU_HELP,VIV_ID_HELP_HELP},
	{"&Command Line Options",MF_STRING,VIV_MENU_HELP,VIV_ID_HELP_COMMAND_LINE_OPTIONS},
	{0,MF_SEPARATOR,VIV_MENU_HELP,0},
	{"Home &Page",MF_STRING,VIV_MENU_HELP,VIV_ID_HELP_WEBSITE},
	{"&Donate",MF_STRING,VIV_MENU_HELP,VIV_ID_HELP_DONATE},
	{0,MF_SEPARATOR,VIV_MENU_HELP,0},
	{"&About",MF_STRING,VIV_MENU_HELP,VIV_ID_HELP_ABOUT},
};

#define VIV_COMMAND_COUNT	(sizeof(viv_commands) / sizeof(viv_command_t))

viv_default_key_t viv_default_keys[] =
{
	{VIV_ID_FILE_OPEN_FILE,VIV_KEYFLAG_CTRL | 'O'},
	{VIV_ID_FILE_ADD_FILE,VIV_KEYFLAG_CTRL | VIV_KEYFLAG_SHIFT | 'O'},
	{VIV_ID_FILE_OPEN_FOLDER,VIV_KEYFLAG_CTRL | 'B'},
	{VIV_ID_FILE_ADD_FOLDER,VIV_KEYFLAG_CTRL | VIV_KEYFLAG_SHIFT | 'B'},
	{VIV_ID_FILE_OPEN_FILE_LOCATION,VIV_KEYFLAG_CTRL | VK_RETURN},
	{VIV_ID_FILE_EDIT,VIV_KEYFLAG_CTRL | 'E'},
	{VIV_ID_FILE_PRINT,VIV_KEYFLAG_CTRL | 'P'},
//	{VIV_ID_FILE_SET_DESKTOP_WALLPAPER,VIV_KEYFLAG_CTRL | 'D'}, // this needs a confirmation dialog
	{VIV_ID_FILE_CLOSE,VIV_KEYFLAG_CTRL | 'W'},
	{VIV_ID_FILE_DELETE,VK_DELETE},
	{VIV_ID_FILE_EXIT,VIV_KEYFLAG_CTRL | 'Q'},
	{VIV_ID_EDIT_CUT,VIV_KEYFLAG_CTRL | 'X'},
	{VIV_ID_EDIT_COPY,VIV_KEYFLAG_CTRL | 'C'},
	{VIV_ID_EDIT_PASTE,VIV_KEYFLAG_CTRL | 'V'},
	{VIV_ID_VIEW_PRESET_1,'1'},
	{VIV_ID_VIEW_PRESET_2,'2'},
	{VIV_ID_VIEW_PRESET_3,'3'},
	{VIV_ID_VIEW_1TO1,VIV_KEYFLAG_CTRL | VIV_KEYFLAG_ALT | '0'},
	{VIV_ID_VIEW_FULLSCREEN,VIV_KEYFLAG_ALT | VK_RETURN},
	{VIV_ID_VIEW_SLIDESHOW,VK_F11},
	{VIV_ID_VIEW_WINDOW_SIZE_50,VIV_KEYFLAG_ALT | '1'},
	{VIV_ID_VIEW_WINDOW_SIZE_100,VIV_KEYFLAG_ALT | '2'},
	{VIV_ID_VIEW_WINDOW_SIZE_200,VIV_KEYFLAG_ALT | '3'},
	{VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT,VIV_KEYFLAG_ALT | '4'},
	{VIV_ID_VIEW_PANSCAN_INCREASE_SIZE,VK_NUMPAD9},
	{VIV_ID_VIEW_PANSCAN_DECREASE_SIZE,VK_NUMPAD1},
	{VIV_ID_VIEW_PANSCAN_INCREASE_WIDTH,VK_NUMPAD6},
	{VIV_ID_VIEW_PANSCAN_DECREASE_WIDTH,VK_NUMPAD4},
	{VIV_ID_VIEW_PANSCAN_INCREASE_HEIGHT,VK_NUMPAD8},
	{VIV_ID_VIEW_PANSCAN_DECREASE_HEIGHT,VK_NUMPAD2},
	{VIV_ID_VIEW_PANSCAN_MOVE_UP,VIV_KEYFLAG_CTRL | VK_NUMPAD8},
	{VIV_ID_VIEW_PANSCAN_MOVE_DOWN,VIV_KEYFLAG_CTRL | VK_NUMPAD2},
	{VIV_ID_VIEW_PANSCAN_MOVE_LEFT,VIV_KEYFLAG_CTRL | VK_NUMPAD4},
	{VIV_ID_VIEW_PANSCAN_MOVE_RIGHT,VIV_KEYFLAG_CTRL | VK_NUMPAD6},
	{VIV_ID_VIEW_PANSCAN_MOVE_UP_LEFT,VIV_KEYFLAG_CTRL | VK_NUMPAD7},
	{VIV_ID_VIEW_PANSCAN_MOVE_UP_RIGHT,VIV_KEYFLAG_CTRL | VK_NUMPAD9},
	{VIV_ID_VIEW_PANSCAN_MOVE_DOWN_LEFT,VIV_KEYFLAG_CTRL | VK_NUMPAD1},
	{VIV_ID_VIEW_PANSCAN_MOVE_DOWN_RIGHT,VIV_KEYFLAG_CTRL | VK_NUMPAD3},
	{VIV_ID_VIEW_PANSCAN_MOVE_CENTER,VIV_KEYFLAG_CTRL | VK_NUMPAD5},
	{VIV_ID_VIEW_PANSCAN_RESET,VK_NUMPAD5},
	{VIV_ID_VIEW_ZOOM_IN,VK_OEM_PLUS},
	{VIV_ID_VIEW_ZOOM_IN,VK_ADD},
	{VIV_ID_VIEW_ZOOM_OUT,VK_OEM_MINUS},
	{VIV_ID_VIEW_ZOOM_OUT,VK_SUBTRACT},
	{VIV_ID_VIEW_ZOOM_RESET,VIV_KEYFLAG_CTRL | '0'},
	{VIV_ID_VIEW_ONTOP_ALWAYS,VIV_KEYFLAG_CTRL | 'T'},
	{VIV_ID_VIEW_OPTIONS,'O'},
	{VIV_ID_SLIDESHOW_PAUSE,VK_SPACE},
	{VIV_ID_SLIDESHOW_RATE_DEC,VK_DOWN},
	{VIV_ID_SLIDESHOW_RATE_INC,VK_UP},
	{VIV_ID_ANIMATION_PLAY_PAUSE,VIV_KEYFLAG_CTRL | VK_SPACE},
	{VIV_ID_ANIMATION_JUMP_FORWARD_MEDIUM,VIV_KEYFLAG_SHIFT | VK_RIGHT},
	{VIV_ID_ANIMATION_JUMP_BACKWARD_MEDIUM,VIV_KEYFLAG_SHIFT | VK_LEFT},
	{VIV_ID_ANIMATION_FRAME_STEP,VIV_KEYFLAG_CTRL | VK_RIGHT},
	{VIV_ID_ANIMATION_FRAME_PREV,VIV_KEYFLAG_CTRL | VK_LEFT},
	{VIV_ID_ANIMATION_FRAME_HOME,VIV_KEYFLAG_CTRL | VK_HOME},
	{VIV_ID_ANIMATION_FRAME_END,VIV_KEYFLAG_CTRL | VK_END},
	{VIV_ID_ANIMATION_RATE_DEC,VIV_KEYFLAG_CTRL | VK_DOWN},
	{VIV_ID_ANIMATION_RATE_INC,VIV_KEYFLAG_CTRL | VK_UP},
	{VIV_ID_ANIMATION_RATE_RESET,VIV_KEYFLAG_CTRL | 'R'},
	{VIV_ID_NAV_NEXT,VK_RIGHT},
	{VIV_ID_NAV_NEXT,VK_NEXT},
	{VIV_ID_NAV_PREV,VK_LEFT},
	{VIV_ID_NAV_PREV,VK_PRIOR},
	{VIV_ID_NAV_HOME,VK_HOME},
	{VIV_ID_NAV_END,VK_END},
	{VIV_ID_HELP_HELP,VK_F1},
	{VIV_ID_HELP_ABOUT,VIV_KEYFLAG_CTRL | VK_F1},
};

#define VIV_DEFAULT_KEY_COUNT (sizeof(viv_default_keys) / sizeof(viv_default_key_t))

WORD viv_context_menu_items[] = 
{
	VIV_ID_NAV_NEXT,
	VIV_ID_NAV_PREV,
	0,
	VIV_ID_SLIDESHOW_PAUSE,
	VIV_MENU_SLIDESHOW_RATE,
	VIV_ID_SLIDESHOW_RATE_DEC,
	VIV_ID_SLIDESHOW_RATE_INC,
	0,
	VIV_ID_SLIDESHOW_RATE_250,
	VIV_ID_SLIDESHOW_RATE_500,
	VIV_ID_SLIDESHOW_RATE_1000,
	VIV_ID_SLIDESHOW_RATE_2000,
	VIV_ID_SLIDESHOW_RATE_3000,
	VIV_ID_SLIDESHOW_RATE_4000,
	VIV_ID_SLIDESHOW_RATE_5000,
	VIV_ID_SLIDESHOW_RATE_6000,
	VIV_ID_SLIDESHOW_RATE_7000,
	VIV_ID_SLIDESHOW_RATE_8000,
	VIV_ID_SLIDESHOW_RATE_9000,
	VIV_ID_SLIDESHOW_RATE_10000,
	VIV_ID_SLIDESHOW_RATE_20000,
	VIV_ID_SLIDESHOW_RATE_30000,
	VIV_ID_SLIDESHOW_RATE_40000,
	VIV_ID_SLIDESHOW_RATE_50000,
	VIV_ID_SLIDESHOW_RATE_60000,
	VIV_ID_SLIDESHOW_RATE_CUSTOM,
	VIV_MENU_SLIDESHOW_RATE,
	0,
	VIV_ID_VIEW_MENU,
	0,
	VIV_ID_VIEW_ALLOW_SHRINKING,
	VIV_ID_VIEW_KEEP_ASPECT_RATIO,
	VIV_ID_VIEW_FILL_WINDOW,
	VIV_ID_VIEW_1TO1,
	VIV_ID_VIEW_FULLSCREEN,
	VIV_ID_VIEW_SLIDESHOW,
	0,
	VIV_MENU_NAVIGATE_SORT,
	VIV_ID_NAV_SORT_NAME,
	VIV_ID_NAV_SORT_FULL_PATH_AND_FILENAME,
	VIV_ID_NAV_SORT_SIZE,
	VIV_ID_NAV_SORT_DATE_MODIFIED,
	VIV_ID_NAV_SORT_DATE_CREATED,
	0,
	VIV_ID_NAV_SORT_ASCENDING,
	VIV_ID_NAV_SORT_DESCENDING,
	VIV_MENU_NAVIGATE_SORT,
	0,
	VIV_ID_FILE_OPEN_FILE_LOCATION,
	VIV_ID_FILE_EDIT,
	VIV_ID_FILE_PREVIEW,
	VIV_ID_FILE_PRINT,
	VIV_ID_FILE_SET_DESKTOP_WALLPAPER,
	0,
	VIV_ID_EDIT_CUT,
	VIV_ID_EDIT_COPY,
	VIV_ID_FILE_DELETE,
	0,
	VIV_ID_FILE_PROPERTIES,
};

#define VIV_CONEXT_MENU_ITEM_COUNT	(sizeof(viv_context_menu_items) / sizeof(WORD))

typedef struct viv_key_list_s
{
	viv_key_t *start[VIV_COMMAND_COUNT];
	viv_key_t *last[VIV_COMMAND_COUNT];
	
}viv_key_list_t;

viv_key_list_t *viv_key_list = 0;

const char *viv_association_extensions[] = 
{
	"bmp",
	"gif",
	"ico",
	"jpeg",
	"jpg",
	"png",
	"tif",
	"tiff",
};

const char *viv_association_descriptions[] = 
{
	"Bitmap Image",
	"Animated GIF Image",
	"Icon File",
	"JPEG Image",
	"JPEG Image",
	"PNG Image",
	"TIFF Image",
	"TIFF Image",
};

const char *viv_association_icon_locations[] = 
{
	0,
	0,
	"%1",
	0,
	0,
	0,
	0,
	0,
};

const WORD viv_association_dlg_item_id[] = 
{
	IDC_BMP,
	IDC_GIF,
	IDC_ICO,
	IDC_JPEG,
	IDC_JPG,
	IDC_PNG,
	IDC_TIF,
	IDC_TIFF,
};

#define VIV_ASSOCIATION_COUNT	(sizeof(viv_association_extensions) / sizeof(const wchar_t *))

// unicode on Windows 9x.
HMODULE LoadUnicowsProc(void);
extern FARPROC _PfnLoadUnicows = (FARPROC) &LoadUnicowsProc;

static void viv_update_title(void)
{
	wchar_t window_title[STRING_SIZE+STRING_SIZE];
	const wchar_t *filename;

	window_title[0] = 0;

	filename = string_get_filename_part(viv_current_fd->cFileName);
	
	if (*filename)
	{
		string_cat(window_title,filename);
		string_cat_utf8(window_title," - ");
	}
		
	string_cat_utf8(window_title,"void Image Viewer");
	
	SetWindowTextW(viv_hwnd,window_title);
}

static void viv_clear(void)
{
	if (viv_frame_delays)
	{
		mem_free(viv_frame_delays);
		
		viv_frame_delays = 0;
	}
	
	if (viv_frames)
	{
		int i;
		
		for(i=0;i<viv_frame_loaded_count;i++)		
		{
			if (viv_frames[i].hbitmap)
			{
				DeleteObject(viv_frames[i].hbitmap);
			}
		}
		
		mem_free(viv_frames);
		
		viv_frames = 0;
	}

	viv_timer_stop();	

	viv_frame_position = 0;
	viv_frame_looped = 0;
	viv_is_slideshow_timeup = 0;
	viv_frame_count = 0;
	viv_zoom_pos = 0;
	viv_view_x = 0;
	viv_view_y = 0;
	viv_view_ix = 0.0;
	viv_view_iy = 0.0;
	viv_1to1 = 0;
	viv_have_old_zoom = 0;
	viv_image_wide = 0;
	viv_image_high = 0;
	viv_animation_play = 1;
}

static void viv_open_from_filename(const wchar_t *filename)
{
	WIN32_FIND_DATA fd;
	HANDLE h;
	wchar_t full_path_and_filename[STRING_SIZE];
	wchar_t cwd[STRING_SIZE];

	GetCurrentDirectory(STRING_SIZE,cwd);
	string_path_combine(full_path_and_filename,cwd,filename);

	debug_printf("open filename: %S\n",full_path_and_filename);
	
	if (GetFileAttributesEx(full_path_and_filename,GetFileExInfoStandard,&fd))
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// add subfolders and subsubfolders...
			viv_dragdrop_add_path(full_path_and_filename);

			viv_home(0);
		}
		else
		{
			string_copy(fd.cFileName,full_path_and_filename);
			
			viv_open(&fd);
		}
	}
	else
	{
		h = FindFirstFile(full_path_and_filename,&fd);
		if (h != INVALID_HANDLE_VALUE)
		{
			wchar_t path[STRING_SIZE];
			
			string_get_path_part(path,full_path_and_filename);
		
			for(;;)
			{
				string_path_combine(full_path_and_filename,path,fd.cFileName);
					
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					viv_dragdrop_add_path(full_path_and_filename);
				}
				else
				{
					string_copy(fd.cFileName,full_path_and_filename);
					viv_dragdrop_add(&fd);
				}
					
				if (!FindNextFile(h,&fd))
				{
					break;
				}
			}

			viv_home(0);
			
			FindClose(h);
		}
	}
}

static void viv_open(WIN32_FIND_DATA *fd)
{
	if (viv_load_image_thread)
	{
		// already loading a different image.
		// add it to the queue and cancel this one.
		viv_load_image_terminate = 1;
		
		if (viv_load_image_next_fd)
		{
			mem_free(viv_load_image_next_fd);
		}
		
		viv_load_image_next_fd = (WIN32_FIND_DATA *)mem_alloc(sizeof(WIN32_FIND_DATA));
		os_copy_memory(viv_load_image_next_fd,fd,sizeof(WIN32_FIND_DATA));
	}
	else
	{
		DWORD thread_id;
		
		viv_load_image_terminate = 0;
		
		viv_load_image_filename = string_alloc(fd->cFileName);
		
		viv_load_image_thread = CreateThread(NULL,0,viv_load_image_proc,0,0,&thread_id);

		viv_status_update();
	}
	
	os_copy_memory(viv_current_fd,fd,sizeof(WIN32_FIND_DATA));
	
	viv_update_title();
}

static void viv_on_size(void)
{
	RECT rect;
	int wide;
	int high;
	
	GetClientRect(viv_hwnd,&rect);
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top;

	if (!IsIconic(viv_hwnd))
	{
		if (!viv_is_window_maximized(viv_hwnd))
		{
			if (!viv_is_fullscreen)
			{
				RECT window_rect;
				
				GetWindowRect(viv_hwnd,&window_rect);
				
				viv_wide = window_rect.right - window_rect.left;
				viv_high = window_rect.bottom - window_rect.top;
			}
		}
	}
	
	{
		int rw;
		int rh;
		
		viv_get_render_size(&rw,&rh);
		
		viv_view_set((int)(viv_view_ix * (double)rw),(int)(viv_view_iy * (double)rh),1);
	}

	if (viv_status_hwnd)
	{
		viv_status_update();
		
		SendMessage(viv_status_hwnd,WM_SIZE,0,0);
		
		high -= viv_get_status_high();
	}

	if (viv_toolbar_hwnd)
	{
		int toolbar_wide;
		
		toolbar_wide = viv_toolbar_get_wide();
		
		SetWindowPos(viv_rebar_hwnd,0,0,high - viv_get_controls_high(),wide,viv_get_controls_high(),SWP_NOZORDER|SWP_NOACTIVATE);
		SetWindowPos(viv_toolbar_hwnd,0,(wide / 2) - (toolbar_wide /2),6,toolbar_wide,viv_get_controls_high() - 6,SWP_NOZORDER|SWP_NOACTIVATE);
	}
	
	viv_toolbar_update_buttons();
}

static void viv_command(int command_id)
{
	// Parse the menu selections:
	switch (command_id)
	{
		case VIV_ID_VIEW_ZOOM_IN:
			viv_zoom_in(0,0,0,0);
			break;

		case VIV_ID_VIEW_ZOOM_OUT:
			viv_zoom_in(1,0,0,0);
			break;

		case VIV_ID_VIEW_ZOOM_RESET:
			viv_1to1 = 0;
			viv_zoom_pos = 0;
			viv_view_set(viv_view_x,viv_view_y,1);
			InvalidateRect(viv_hwnd,0,FALSE);
			break;
	
		case VIV_ID_HELP_HELP:
			ShellExecuteA(viv_hwnd,NULL,"http://www.voidtools.com/support/voidimageviewer/",NULL,NULL,SW_SHOWNORMAL);
			break;
			
		case VIV_ID_HELP_COMMAND_LINE_OPTIONS:
			viv_command_line_options();
			break;
			
		case VIV_ID_HELP_DONATE:
			ShellExecuteA(viv_hwnd,NULL,"http://www.voidtools.com/donate/",NULL,NULL,SW_SHOWNORMAL);
			break;
			
		case VIV_ID_HELP_ABOUT:
			DialogBox(os_hinstance,MAKEINTRESOURCE(IDD_ABOUT),viv_hwnd,viv_about_proc);
			break;
			
		case VIV_ID_HELP_WEBSITE:
			ShellExecuteA(viv_hwnd,NULL,"http://www.voidtools.com/",NULL,NULL,SW_SHOWNORMAL);
			break;
			
		case VIV_ID_FILE_EXIT:
			viv_exit();
			break;
		
		case VIV_ID_NAV_PREV:
			viv_next(1,1);
			break;
		
		case VIV_ID_NAV_NEXT:
			viv_next(0,1);
			break;
		
		case VIV_ID_NAV_HOME:
			viv_home(0);
			break;
		
		case VIV_ID_NAV_END:
			viv_home(1);
			break;
		
		case VIV_ID_NAV_SORT_NAME:
		case VIV_ID_NAV_SORT_SIZE:
		case VIV_ID_NAV_SORT_DATE_MODIFIED:
		case VIV_ID_NAV_SORT_DATE_CREATED:
		case VIV_ID_NAV_SORT_FULL_PATH_AND_FILENAME:

			if (viv_nav_sort == command_id - VIV_ID_NAV_SORT_NAME)
			{
				viv_nav_sort_ascending = !viv_nav_sort_ascending;
			}
			else
			{
				viv_nav_sort = command_id - VIV_ID_NAV_SORT_NAME;
				switch(viv_nav_sort)
				{
					case VIV_NAV_SORT_NAME:
					case VIV_NAV_SORT_FULL_PATH_AND_FILENAME:
						viv_nav_sort_ascending = 1;
						break;
						
					default:
						viv_nav_sort_ascending = 0;
						break;
				}
			}
			
			break;
		
		case VIV_ID_NAV_SORT_ASCENDING:
			viv_nav_sort_ascending = 1;
			break;
			
		case VIV_ID_NAV_SORT_DESCENDING:
			viv_nav_sort_ascending = 0;
			break;
		
		case VIV_ID_SLIDESHOW_PLAY_ONLY:
			if (!viv_is_slideshow)
			{
				viv_pause();
			}
			break;

		case VIV_ID_SLIDESHOW_PAUSE_ONLY:
			if (viv_is_slideshow)
			{
				viv_pause();
			}
			break;
		
		case VIV_ID_SLIDESHOW_STOP:
			if (viv_is_slideshow)
			{
				viv_pause();
			}
			break;

		case VIV_ID_SLIDESHOW_PAUSE:
			viv_pause();
			break;
			
		case VIV_ID_SLIDESHOW_RATE_DEC:
			viv_increase_rate(1);
			break;
			
		case VIV_ID_SLIDESHOW_RATE_INC:
			viv_increase_rate(0);
			break;

		case VIV_ID_ANIMATION_PLAY_PAUSE:
			viv_animation_pause();
			break;

		case VIV_ID_ANIMATION_JUMP_FORWARD_MEDIUM:
			viv_frame_skip(viv_medium_jump);
			break;

		case VIV_ID_ANIMATION_JUMP_BACKWARD_MEDIUM:
			viv_frame_skip(-viv_medium_jump);
			break;

		case VIV_ID_ANIMATION_JUMP_FORWARD_SHORT:
			viv_frame_skip(viv_short_jump);
			break;

		case VIV_ID_ANIMATION_JUMP_BACKWARD_SHORT:
			viv_frame_skip(-viv_short_jump);
			break;

		case VIV_ID_ANIMATION_JUMP_FORWARD_LONG:
			viv_frame_skip(viv_long_jump);
			break;

		case VIV_ID_ANIMATION_JUMP_BACKWARD_LONG:
			viv_frame_skip(-viv_long_jump);
			break;

		case VIV_ID_ANIMATION_FRAME_STEP:
			viv_frame_step();
			break;

		case VIV_ID_ANIMATION_FRAME_PREV:
			viv_frame_prev();
			break;

		case VIV_ID_ANIMATION_FRAME_HOME:
			viv_frame_looped = 0;
			
			if (viv_animation_play)
			{
				viv_animation_play = 0;
			}

			if (viv_frame_count > 1)			
			{
				viv_frame_position = 0;

				viv_status_update();
					
				InvalidateRect(viv_hwnd,NULL,FALSE);
				UpdateWindow(viv_hwnd);
			}
			
			break;

		case VIV_ID_ANIMATION_FRAME_END:

			viv_frame_looped = 0;
			
			if (viv_animation_play)
			{
				viv_animation_play = 0;
			}

			if (viv_frame_count > 1)
			{
				viv_frame_position = viv_frame_loaded_count - 1;

				viv_status_update();
					
				InvalidateRect(viv_hwnd,NULL,FALSE);
				UpdateWindow(viv_hwnd);
			}
			
			break;

		case VIV_ID_ANIMATION_RATE_DEC:
			viv_increase_animation_rate(1);
			break;
			
		case VIV_ID_ANIMATION_RATE_INC:
			viv_increase_animation_rate(0);
			break;

		case VIV_ID_ANIMATION_RATE_RESET:
			viv_reset_animation_rate();
			break;

		case VIV_ID_SLIDESHOW_RATE_250: viv_set_rate(250); break;
		case VIV_ID_SLIDESHOW_RATE_500: viv_set_rate(500); break;
		case VIV_ID_SLIDESHOW_RATE_1000: viv_set_rate(1000); break;
		case VIV_ID_SLIDESHOW_RATE_2000: viv_set_rate(2000); break;
		case VIV_ID_SLIDESHOW_RATE_3000: viv_set_rate(3000); break;
		case VIV_ID_SLIDESHOW_RATE_4000: viv_set_rate(4000); break;
		case VIV_ID_SLIDESHOW_RATE_5000: viv_set_rate(5000); break;
		case VIV_ID_SLIDESHOW_RATE_6000: viv_set_rate(6000); break;
		case VIV_ID_SLIDESHOW_RATE_7000: viv_set_rate(7000); break;
		case VIV_ID_SLIDESHOW_RATE_8000: viv_set_rate(8000); break;
		case VIV_ID_SLIDESHOW_RATE_9000: viv_set_rate(9000); break;
		case VIV_ID_SLIDESHOW_RATE_10000: viv_set_rate(10000); break;
		case VIV_ID_SLIDESHOW_RATE_20000: viv_set_rate(20000); break;
		case VIV_ID_SLIDESHOW_RATE_30000: viv_set_rate(30000); break;
		case VIV_ID_SLIDESHOW_RATE_40000: viv_set_rate(40000); break;
		case VIV_ID_SLIDESHOW_RATE_50000: viv_set_rate(50000); break;
		case VIV_ID_SLIDESHOW_RATE_60000: viv_set_rate(60000); break;
		case VIV_ID_SLIDESHOW_RATE_CUSTOM: viv_set_custom_rate(); break;
			
		case VIV_ID_VIEW_MENU:
			viv_is_show_menu = !viv_is_show_menu;
			viv_update_frame();
			break;
			
		case VIV_ID_VIEW_STATUS:
			viv_is_show_status = !viv_is_show_status;
			viv_update_frame();
			break;
			
		case VIV_ID_VIEW_CONTROLS:
			viv_is_show_controls = !viv_is_show_controls;
			viv_update_frame();
			break;
			
		case VIV_ID_VIEW_PRESET_1:
			viv_is_show_menu = 0;
			viv_is_show_status = 0;
			viv_is_show_controls = 0;
			viv_is_show_caption = 0;
			viv_is_show_thickframe = 0;
			viv_update_frame();
			break;

		case VIV_ID_VIEW_PRESET_2:
			viv_is_show_menu = 0;
			viv_is_show_status = 0;
			viv_is_show_controls = 0;
			viv_is_show_caption = 0;
			viv_is_show_thickframe = 1;
			viv_update_frame();
			break;

		case VIV_ID_VIEW_PRESET_3:
			viv_is_show_menu = 1;
			viv_is_show_status = 1;
			viv_is_show_controls = 1;
			viv_is_show_caption = 1;
			viv_is_show_thickframe = 1;
			viv_update_frame();
			break;
			
		case VIV_ID_VIEW_ALLOW_SHRINKING:
			viv_1to1 = 0;
			viv_allow_shrinking = !viv_allow_shrinking;
			
			viv_on_size();
			InvalidateRect(viv_hwnd,0,FALSE);

			break;
			
		case VIV_ID_VIEW_KEEP_ASPECT_RATIO:
			viv_1to1 = 0;
			viv_keep_aspect_ratio = !viv_keep_aspect_ratio;
			
			viv_on_size();
			InvalidateRect(viv_hwnd,0,FALSE);
			break;
			
		case VIV_ID_VIEW_FILL_WINDOW:
		
			viv_1to1 = 0;
			if (viv_is_fullscreen)
			{
				viv_fullscreen_fill_window = !viv_fullscreen_fill_window;
			}
			else
			{
				viv_fill_window = !viv_fill_window;
			}
			
			viv_on_size();
			InvalidateRect(viv_hwnd,0,FALSE);
			break;
		
		case VIV_ID_VIEW_FULLSCREEN:

			viv_toggle_fullscreen();
			break;		
			
		case VIV_ID_VIEW_1TO1:
			viv_view_1to1();
			break;		
			
		case VIV_ID_VIEW_BESTFIT:
			viv_zoom_pos = 0;
			viv_1to1 = 0;
			viv_view_set(0,0,1);
			InvalidateRect(viv_hwnd,0,FALSE);
			break;
			
		case VIV_ID_VIEW_SLIDESHOW:

			viv_slideshow();
						
			break;
			
		case VIV_ID_VIEW_WINDOW_SIZE_50:
		case VIV_ID_VIEW_WINDOW_SIZE_100:
		case VIV_ID_VIEW_WINDOW_SIZE_200:
		case VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT:
			if (viv_is_fullscreen)
			{
				viv_toggle_fullscreen();
			}
		
			// get out of fullscreen mode.
			if (viv_is_window_maximized(viv_hwnd))
			{
				ShowWindow(viv_hwnd,SW_RESTORE);
			}
		
			if ((viv_image_wide) && (viv_image_high))
			{
				HMONITOR hmonitor;
				MONITORINFO mi;
				RECT rect;
				int wide;
				int high;
				int midx;
				int midy;
				
				GetWindowRect(viv_hwnd,&rect);
				midx = (rect.left + rect.right) / 2;
				midy = (rect.top + rect.bottom) / 2;
				
				rect.left = 0;
				rect.top = 0;
				
				switch(command_id)
				{
					case VIV_ID_VIEW_WINDOW_SIZE_50:
						rect.right = viv_image_wide / 2;
						rect.bottom = viv_image_high / 2;
						break;
						
					case VIV_ID_VIEW_WINDOW_SIZE_100:
						rect.right = viv_image_wide;
						rect.bottom = viv_image_high;
						break;
						
					case VIV_ID_VIEW_WINDOW_SIZE_200:
						rect.right = viv_image_wide * 2;
						rect.bottom = viv_image_high * 2;
						break;
						
					case VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT:
					{
						HMONITOR hmonitor;
						MONITORINFO mi;
						
						hmonitor = MonitorFromWindow(viv_hwnd,MONITOR_DEFAULTTOPRIMARY);
						
						mi.cbSize = sizeof(MONITORINFO);
						GetMonitorInfo(hmonitor,&mi);										
						
						rect.right = ((mi.rcMonitor.right - mi.rcMonitor.left) * 3) / 5;
						rect.bottom = ((mi.rcMonitor.bottom - mi.rcMonitor.top) * 3) / 5;
						break;
					}
				}
				
				AdjustWindowRect(&rect,GetWindowStyle(viv_hwnd),GetMenu(viv_hwnd) ? TRUE : FALSE);

//debug_printf("%d %d | %d %d %d %d\n",midx,midy,rect.left,rect.top,rect.right,rect.bottom)								;
				
				wide = rect.right - rect.left;
				high = rect.bottom - rect.top + viv_get_status_high() + viv_get_controls_high();
				
				hmonitor = MonitorFromWindow(viv_hwnd,MONITOR_DEFAULTTOPRIMARY);
				
				mi.cbSize = sizeof(MONITORINFO);
				GetMonitorInfo(hmonitor,&mi);
				
				if (wide > mi.rcWork.right - mi.rcWork.left)
				{
					wide = mi.rcWork.right - mi.rcWork.left;
				}
				
				if (high > mi.rcWork.bottom - mi.rcWork.top)
				{
					high = mi.rcWork.bottom - mi.rcWork.top;
				}
				
				rect.left = midx - wide / 2;
				rect.top = midy - high / 2;
				rect.right = midx - (wide / 2) + wide;
				rect.bottom = midy - (high / 2) + high;
				
				os_make_rect_completely_visible(&rect);
				
				SetWindowPos(viv_hwnd,0,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOCOPYBITS);
			}			
			break;
			
		case VIV_ID_VIEW_PANSCAN_INCREASE_SIZE:
			viv_dst_zoom_set(viv_dst_zoom_x_pos + 1,viv_dst_zoom_y_pos + 1);
			break;
			
		case VIV_ID_VIEW_PANSCAN_DECREASE_SIZE:
			viv_dst_zoom_set(viv_dst_zoom_x_pos - 1,viv_dst_zoom_y_pos - 1);
			break;
			
		case VIV_ID_VIEW_PANSCAN_INCREASE_WIDTH:
			viv_dst_zoom_set(viv_dst_zoom_x_pos + 1,viv_dst_zoom_y_pos);
			break;
			
		case VIV_ID_VIEW_PANSCAN_DECREASE_WIDTH:
			viv_dst_zoom_set(viv_dst_zoom_x_pos - 1,viv_dst_zoom_y_pos);
			break;
			
		case VIV_ID_VIEW_PANSCAN_INCREASE_HEIGHT:
			viv_dst_zoom_set(viv_dst_zoom_x_pos,viv_dst_zoom_y_pos + 1);
			break;
			
		case VIV_ID_VIEW_PANSCAN_DECREASE_HEIGHT:
			viv_dst_zoom_set(viv_dst_zoom_x_pos,viv_dst_zoom_y_pos - 1);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_RIGHT:
			viv_dst_pos_set(viv_dst_pos_x + 5,viv_dst_pos_y);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_LEFT:
			viv_dst_pos_set(viv_dst_pos_x - 5,viv_dst_pos_y);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_UP:
			viv_dst_pos_set(viv_dst_pos_x,viv_dst_pos_y - 5);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_DOWN:
			viv_dst_pos_set(viv_dst_pos_x,viv_dst_pos_y + 5);
			break;

		case VIV_ID_VIEW_PANSCAN_MOVE_UP_LEFT:
			viv_dst_pos_set(viv_dst_pos_x - 5,viv_dst_pos_y - 5);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_UP_RIGHT:
			viv_dst_pos_set(viv_dst_pos_x + 5,viv_dst_pos_y - 5);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_DOWN_LEFT:
			viv_dst_pos_set(viv_dst_pos_x - 5,viv_dst_pos_y + 5);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_DOWN_RIGHT:
			viv_dst_pos_set(viv_dst_pos_x + 5,viv_dst_pos_y + 5);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_CENTER:
			viv_dst_pos_x = 500;
			viv_dst_pos_y = 500;
			InvalidateRect(viv_hwnd,NULL,FALSE);
			viv_status_update_temp_pos_zoom();	
			break;
			
		case VIV_ID_VIEW_PANSCAN_RESET:
			viv_dst_pos_x = 500;
			viv_dst_pos_y = 500;
			InvalidateRect(viv_hwnd,NULL,FALSE);
			viv_dst_zoom_set(VIV_DST_ZOOM_ONE,VIV_DST_ZOOM_ONE);		
			break;
			
		case VIV_ID_VIEW_ONTOP_ALWAYS:
			viv_ontop = !viv_ontop;
			viv_update_ontop();
			break;
						
		case VIV_ID_VIEW_ONTOP_WHILE_PLAYING_OR_ANIMATING:
			viv_ontop = 2;
			viv_update_ontop();
			break;
						
		case VIV_ID_VIEW_ONTOP_NEVER:
			viv_ontop = 0;
			viv_update_ontop();
			break;
						
		case VIV_ID_VIEW_OPTIONS:
			viv_options();
			break;
			
		case VIV_ID_EDIT_COPY:
			viv_copy(0);
			break;

		case VIV_ID_EDIT_PASTE:
			SendMessage(viv_hwnd,WM_PASTE,0,0);
			break;

		case VIV_ID_EDIT_CUT:
			viv_copy(1);
			break;
			
		case VIV_ID_FILE_OPEN_FILE:
		case VIV_ID_FILE_ADD_FILE:
			{
				OPENFILENAME ofn;
				wchar_t tobuf[STRING_SIZE+1];
				wchar_t filter_wbuf[STRING_SIZE];
				wchar_t title_wbuf[STRING_SIZE];
				
				os_zero_memory(&ofn,sizeof(OPENFILENAME));

				string_copy(tobuf,viv_last_open_file ? viv_last_open_file : L"");

				string_copy_utf8_double_null(filter_wbuf,(const utf8_t *)
					"All Image Files\0*.bmp;*.gif;*.ico;*.jpeg;*.jpg;*.png;*.tif;*.tiff\0"
					"*.* (All Files)\0*.*\0");

				string_copy_utf8(title_wbuf,"Open Image");
				
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = viv_hwnd;
				ofn.hInstance = os_hinstance;
				ofn.lpstrFilter = filter_wbuf;
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = tobuf;
				ofn.nMaxFile = STRING_SIZE;
				ofn.lpstrTitle = title_wbuf;
				ofn.Flags = OFN_ENABLESIZING | OFN_NOCHANGEDIR;
				
				if (GetOpenFileName(&ofn))
				{
					if (command_id == VIV_ID_FILE_OPEN_FILE)
					{
						viv_dragdrop_clearall();
	
						viv_open_from_filename(ofn.lpstrFile);
					}
					else
					{
						viv_dragdrop_add_filename(ofn.lpstrFile);
					}
					
					if (viv_last_open_file)
					{
						mem_free(viv_last_open_file);
					}
					
					viv_last_open_file = string_alloc(ofn.lpstrFile);
				}
			}
			break;
			
		case VIV_ID_FILE_OPEN_FOLDER:
		case VIV_ID_FILE_ADD_FOLDER:
			
			{
				wchar_t filename[STRING_SIZE];
				
				string_copy(filename,viv_last_open_folder ? viv_last_open_folder : L"");
				
				if (command_id == VIV_ID_FILE_OPEN_FOLDER)
				{
					viv_dragdrop_clearall();
				}
				
				if (os_browse_for_folder(viv_hwnd,filename))
				{
					viv_dragdrop_add_path(filename);

					if (viv_last_open_folder)
					{
						mem_free(viv_last_open_folder);
					}
					
					viv_last_open_folder = string_alloc(filename);

					if (command_id == VIV_ID_FILE_OPEN_FOLDER)
					{
						viv_home(0);
					}
				}
			}
			
			break;
			
		case VIV_ID_FILE_DELETE:
			viv_delete();
			break;

		case VIV_ID_FILE_PREVIEW:
			viv_file_preview();
			break;

		case VIV_ID_FILE_PRINT:
			viv_file_print();
			break;

		case VIV_ID_FILE_SET_DESKTOP_WALLPAPER:
			viv_file_set_desktop_wallpaper();
			break;
			
		case VIV_ID_FILE_CLOSE:
			viv_blank();
			break;
	
		case VIV_ID_FILE_EDIT:
			viv_file_edit();
			break;

		case VIV_ID_FILE_OPEN_FILE_LOCATION:
			viv_open_file_location();
			break;
			
		case VIV_ID_FILE_PROPERTIES:
			viv_properties();
			break;
			
		case VIV_ID_EDIT_COPY_TO:
		case VIV_ID_EDIT_MOVE_TO:
		{
			if (*viv_current_fd->cFileName)
			{
				OPENFILENAME ofn;
				wchar_t tobuf[STRING_SIZE+1];
				wchar_t filter_wbuf[STRING_SIZE];
				wchar_t title_wbuf[STRING_SIZE];
				
				os_zero_memory(&ofn,sizeof(OPENFILENAME));
				
				string_copy(tobuf,viv_current_fd->cFileName);
				string_copy_utf8_double_null(filter_wbuf,(const utf8_t *)"*.* (All Files)\0*.*\0");
				string_copy_utf8(title_wbuf,command_id == VIV_ID_EDIT_COPY_TO ? (const utf8_t *)"Copy To" : (const utf8_t *)"Move To");
				
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = viv_hwnd;
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
					
					tobuf[string_length(tobuf) + 1] = 0;
					
					os_zero_memory(&shfileop,sizeof(SHFILEOPSTRUCT));
					
					string_copy(frombuf,viv_current_fd->cFileName);
					frombuf[string_length(frombuf) + 1] = 0;
					
					shfileop.hwnd = viv_hwnd;
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
	
	if ((command_id >= VIV_ID_NAV_PLAYLIST_START) && (command_id < VIV_ID_NAV_PLAYLIST_END))
	{
		if (command_id - VIV_ID_NAV_PLAYLIST_START < viv_nav_item_count)
		{
			viv_open(&viv_nav_items[command_id - VIV_ID_NAV_PLAYLIST_START]->fd);
		}
	}
}

static void viv_exit(void)
{
	viv_save_settings(viv_appdata);
	PostQuitMessage(0);
}

static LRESULT CALLBACK viv_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) 
	{	
		case WM_NCHITTEST:

			if (!viv_is_fullscreen)
			{
				if (DefWindowProc(hwnd,msg,wParam,lParam) == HTCLIENT)
				{
					if (!viv_is_show_thickframe)
					{
						int x;
						int y;
						RECT rect;
						
						GetWindowRect(hwnd,&rect);

						x = GET_X_LPARAM(lParam); 
						y = GET_Y_LPARAM(lParam); 				
						
						if (x < rect.left + GetSystemMetrics(SM_CXSIZEFRAME))
						{
							if (y < rect.top + GetSystemMetrics(SM_CYSIZEFRAME))
							{
								return HTTOPLEFT;
							}
							if (y > rect.bottom - GetSystemMetrics(SM_CYSIZEFRAME))
							{
								return HTBOTTOMLEFT;
							}

							return HTLEFT;
						}
						else
						if (x > rect.right - GetSystemMetrics(SM_CXSIZEFRAME))
						{
							if (y < rect.top + GetSystemMetrics(SM_CYSIZEFRAME))
							{
								return HTTOPRIGHT;
							}
							if (y > rect.bottom - GetSystemMetrics(SM_CYSIZEFRAME))
							{
								return HTBOTTOMRIGHT;
							}

							return HTRIGHT;
						}
						else
						if (y < rect.top + GetSystemMetrics(SM_CYSIZEFRAME))
						{
							if (x < rect.left + GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXSIZEFRAME))
							{
								return HTTOPLEFT;
							}
							if (x > rect.right - GetSystemMetrics(SM_CXSIZEFRAME) - GetSystemMetrics(SM_CXSIZEFRAME))
							{
								return HTTOPRIGHT;
							}
							
							return HTTOP;
						}
						if (y > rect.bottom - GetSystemMetrics(SM_CYSIZEFRAME))
						{
							if (x < rect.left + GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXSIZEFRAME))
							{
								return HTBOTTOMLEFT;
							}
							if (x > rect.right - GetSystemMetrics(SM_CXSIZEFRAME) - GetSystemMetrics(SM_CXSIZEFRAME))
							{
								return HTBOTTOMRIGHT;
							}
							
							return HTBOTTOM;
						}
					}
				}
			}
			
			break;
			
		case WM_NCLBUTTONDOWN:

			if (!viv_is_fullscreen)
			{
				if (!viv_is_show_thickframe)
				{
					DWORD flag;
					
					flag = 0;
					
					switch(wParam)
					{
						case HTTOP:
							flag = WMSZ_TOP;
							break;
						case HTTOPLEFT:
							flag = WMSZ_TOPLEFT;
							break;
						case HTTOPRIGHT:
							flag = WMSZ_TOPRIGHT;
							break;
						case HTLEFT:
							flag = WMSZ_LEFT;
							break;
						case HTRIGHT:
							flag = WMSZ_RIGHT;
							break;
						case HTBOTTOM:
							flag = WMSZ_BOTTOM;
							break;
						case HTBOTTOMLEFT:
							flag = WMSZ_BOTTOMLEFT;
							break;
						case HTBOTTOMRIGHT:
							flag = WMSZ_BOTTOMRIGHT;
							break;							
					}

					if (flag)
					{					
			            PostMessage(hwnd,WM_SYSCOMMAND,(SC_SIZE | flag),lParam);
			            return 0;
					}
				}
			}
			
			break;
		
		case WM_DESTROY:
			// don't free the menu again.
			viv_hmenu = 0;
			break;
			
		case WM_QUERYENDSESSION:
			return TRUE;

		case WM_ENDSESSION:
			if (wParam)
			{
				// save settings on logout.
				viv_save_settings(viv_appdata);
			}
			return 0;
			
		case VIV_WM_REPLY:
		{
			viv_reply_t *e;
			
			EnterCriticalSection(&viv_cs);
			e = viv_reply_start;
			viv_reply_start = 0;
			viv_reply_last = 0;
			LeaveCriticalSection(&viv_cs);
			
			while(e)
			{
				viv_reply_t *next_e;
				
				next_e = e->next;
				
				switch(e->type)
				{
					case VIV_REPLY_LOAD_IMAGE_COMPLETE:
					case VIV_REPLY_LOAD_IMAGE_FAILED:
					
						if (e->type == VIV_REPLY_LOAD_IMAGE_FAILED)
						{
							viv_clear();

							InvalidateRect(viv_hwnd,NULL,FALSE);
						}
						
						if (viv_load_image_thread)
						{
							CloseHandle(viv_load_image_thread);
							
							viv_load_image_thread = 0;
						}
					
						if (viv_load_image_filename)
						{
							mem_free(viv_load_image_filename);

							viv_load_image_filename = 0;
						}
						
						if (viv_load_image_next_fd)
						{
							WIN32_FIND_DATA *fd;
							
							fd = viv_load_image_next_fd;
							viv_load_image_next_fd = 0;
							
							viv_open(fd);
							
							mem_free(fd);
						}
						
						viv_status_update();
						viv_toolbar_update_buttons();

						break;
										
					case VIV_REPLY_LOAD_IMAGE_FIRST_FRAME:
					{
						viv_reply_load_image_first_frame_t *first_frame;

						viv_clear();

						first_frame = (viv_reply_load_image_first_frame_t *)(e + 1);
						
						viv_image_wide = first_frame->wide;
						viv_image_high = first_frame->high;
						viv_frame_count = first_frame->frame_count;
						viv_frame_delays = first_frame->frame_delays;

						first_frame->frame_delays = 0;
						
						if (viv_frames)
						{
							int i;
							
							for(i=0;i<viv_frame_loaded_count;i++)
							{
								if (viv_frames[i].hbitmap)
								{
									DeleteObject(viv_frames[i].hbitmap);
								}
							}

							mem_free(viv_frames);
						}

						// allocate hbitmaps.
						viv_frames = (viv_frame_t *)mem_alloc(sizeof(viv_frame_t) * viv_frame_count);
						viv_frames[0].hbitmap = first_frame->frame.hbitmap;

						first_frame->frame.hbitmap = 0;
						viv_frame_loaded_count = 1;

						// draw first frame and start timer.
						viv_frame_position = 0;
						viv_frame_looped = 0;
						
						if (viv_frame_count > 1)
						{
							viv_timer_start();
						}

						viv_status_update();
						
						if (!viv_is_fullscreen)
						{
							if (viv_auto_zoom)
							{
								switch(viv_auto_zoom_type)
								{
									case 0:
									case 1:
									case 2:
										viv_command(VIV_ID_VIEW_WINDOW_SIZE_50 + viv_auto_zoom_type);
										break;
								}
							}
						}
						
						InvalidateRect(viv_hwnd,NULL,FALSE);
						
						break;
					}

					case VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME:
					
						// we could have been cleared.
						if (viv_frames)
						{
							viv_frames[viv_frame_loaded_count].hbitmap = ((viv_frame_t *)(e + 1))->hbitmap;
							
							viv_frame_loaded_count++;
							
							((viv_frame_t *)(e + 1))->hbitmap = 0;
						}
					
						break;
				}

				viv_reply_free(e);
				
				e = next_e;
			}
			
			break;
		}
			
		case WM_INITMENU:
		{	
			HMENU hmenu;

			hmenu = GetMenu(hwnd);

			viv_check_menus(hmenu);
			
			break;
		}
					
		case WM_INITMENUPOPUP:
		{	
			if ((HMENU)wParam == viv_nav_playlist_menu)
			{
				WIN32_FIND_DATA fd;
				viv_nav_item_t *navitemstart;
				viv_nav_item_t *navitemlast;
				viv_nav_item_t *navitem;
				
				navitemstart = 0;
				navitemlast = 0;
				
				debug_printf("menu playlist\n");
				
				os_remove_all_menu_items(viv_nav_playlist_menu);
				viv_nav_item_free_all();
				
				if (viv_dragdrop_start)
				{
					viv_dragdrop_t *d;
					
					d = viv_dragdrop_start;
					while(d)
					{
						navitem = (viv_nav_item_t *)mem_alloc(sizeof(viv_nav_item_t));
						
						os_copy_memory(&navitem->fd,&d->fd,sizeof(WIN32_FIND_DATA));

						if (navitemstart)
						{
							navitemlast->next = navitem;
						}
						else
						{
							navitemstart = navitem;
						}
						
						navitem->next = 0;
						navitemlast = navitem;
						viv_nav_item_count++;				
					
						d = d->next;
					}
				}
				else
				if (*viv_current_fd->cFileName)
				{		
					HANDLE h;
					
					wchar_t search_wbuf[STRING_SIZE];
					wchar_t path_wbuf[STRING_SIZE];

					string_get_path_part(path_wbuf,viv_current_fd->cFileName);
					
					string_copy(search_wbuf,path_wbuf);
					string_cat_utf8(search_wbuf,(const utf8_t *)"\\*.*");

					h = FindFirstFile(L"*.*",&fd);
					
					if (h != INVALID_HANDLE_VALUE)
					{
						for(;;)
						{
							if (viv_is_valid_filename(fd.cFileName))
							{
								navitem = (viv_nav_item_t *)mem_alloc(sizeof(viv_nav_item_t));

								string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
								string_copy(fd.cFileName,search_wbuf);
								os_copy_memory(&navitem->fd,&fd,sizeof(WIN32_FIND_DATA));
								
								if (navitemstart)
								{
									navitemlast->next = navitem;
								}
								else
								{
									navitemstart = navitem;
								}
								
								navitem->next = 0;
								navitemlast = navitem;
								viv_nav_item_count++;
							}
							
							if (!FindNextFile(h,&fd)) break;
						}

						FindClose(h);
					}
				}
				
				if (viv_nav_item_count)
				{
					viv_nav_item_t **d;
					
					viv_nav_items = (viv_nav_item_t **)mem_alloc(viv_nav_item_count * sizeof(viv_nav_item_t *));
					d = viv_nav_items;
					
					navitem = navitemstart;
					while(navitem)
					{
						*d++ = navitem;
						
						navitem = navitem->next;
					}
				}
				
				os_qsort(viv_nav_items,viv_nav_item_count,viv_nav_playlist_compare);
				
				{
					int i;
					int checked;
					int run;
					
					run = viv_nav_item_count;
					if (run > VIV_ID_NAV_PLAYLIST_END-VIV_ID_NAV_PLAYLIST_START)
					{
						run = VIV_ID_NAV_PLAYLIST_END-VIV_ID_NAV_PLAYLIST_START;
					}
					
					for(i=0;i<run;i++)
					{
						wchar_t menutext[STRING_SIZE+STRING_SIZE];
						wchar_t *d;
						const wchar_t *p;
						
						checked = (string_compare(viv_nav_items[i]->fd.cFileName,viv_current_fd->cFileName) == 0);
						
						if (viv_nav_sort == VIV_NAV_SORT_FULL_PATH_AND_FILENAME)
						{
							p = viv_nav_items[i]->fd.cFileName;
						}
						else
						{
							p = string_get_filename_part(viv_nav_items[i]->fd.cFileName);
						}
						
						d = menutext;
						
						while(*p)
						{
							if (*p == '&')
							{
								*d++ = '&';
								*d++ = '&';
							}
							else
							{
								*d++ = *p;
							}
							
							p++;
						}
						*d = 0;
	
						AppendMenu(viv_nav_playlist_menu,MF_STRING|(checked?(MF_CHECKED|MFT_RADIOCHECK):(MF_UNCHECKED|MFT_RADIOCHECK)),VIV_ID_NAV_PLAYLIST_START + i,menutext);
					}
				}
			}
			
			break;
		}

		case WM_DROPFILES:
		{
			wchar_t filename[STRING_SIZE];
			DWORD count;
			int is_shift;
			
			is_shift = (GetKeyState(VK_SHIFT) < 0);
			if (!is_shift)
			{
				viv_dragdrop_clearall();
			}
			
			count = DragQueryFile((HDROP)wParam,0xFFFFFFFF,0,0);
			
			if ((count >= 2) || (is_shift))
			{
				DWORD i;
				
				for(i=0;i<count;i++)
				{
					DragQueryFile((HDROP)wParam,i,filename,STRING_SIZE);
				
					viv_dragdrop_add_filename(filename);
				}
			
				if (!is_shift)
				{
					viv_home(0);
				}
			}
			else
			if (count == 1)
			{
				DragQueryFile((HDROP)wParam,0,filename,STRING_SIZE);
			
				viv_open_from_filename(filename);
			}
			
			SetForegroundWindow(hwnd);
			
			break;
		}
			
		case WM_TIMER:
			
			switch(wParam)
			{
				case VIV_ID_STATUS_TEMP_TEXT_TIMER:
					viv_status_set_temp_text(0);
					break;

				case VIV_ID_SLIDESHOW_TIMER:

					if (viv_is_slideshow)
					{
						if (viv_loop_animations_once)
						{
							if (viv_frame_delays)
							{
								if (!viv_frame_looped)
								{
									viv_is_slideshow_timeup = 1;
								
									break;
								}
							}
						}
						
						viv_next(0,0);
					}
					
					break;
					
				case VIV_ID_HIDE_CURSOR_TIMER:
					if (viv_is_hide_cursor_timer)
					{
						if (!viv_in_popup_menu)
						{
							if (viv_cursor_shown)
							{
								if (GetForegroundWindow() == viv_hwnd)
								{
									KillTimer(hwnd,VIV_ID_HIDE_CURSOR_TIMER);
									viv_is_hide_cursor_timer = 0;
									
									ShowCursor(FALSE);
									
									viv_cursor_shown = 0;
								}
							}
						}
					}
					break;
					
				case VIV_ID_ANIMATION_TIMER:
				{
					if (viv_is_animation_timer)
					{
						if (viv_animation_play)
						{
							DWORD elapsed;
							DWORD tick;
							
							tick = GetTickCount();
							
							elapsed = tick - viv_animation_timer_tick_start;
							viv_animation_timer_tick_start = tick;
							
							viv_timer_tick += elapsed;
							
							for(;;)
							{
								DWORD delay;

								delay = viv_frame_delays[viv_frame_position] * (1.0f/viv_animation_rates[viv_animation_rate_pos]);
								
								if (!delay)
								{
									delay = 1;
								}
				
								if (viv_timer_tick >= delay)
								{
									if (viv_frame_loaded_count != viv_frame_count)
									{
										if (viv_frame_position + 1 >= viv_frame_loaded_count)
										{	
											// ignore this tick
											viv_timer_tick -= elapsed;
											break;
										}
									}
								
									viv_frame_position++;
									if (viv_frame_position == viv_frame_count)
									{
										viv_frame_looped = 1;
										viv_frame_position = 0;

										if (viv_loop_animations_once)
										{
											if (viv_is_slideshow_timeup)
											{
												viv_next(0,1);
												
												break;
											}
										}
									}
									
									viv_status_update();
									InvalidateRect(viv_hwnd,0,FALSE);

									viv_timer_tick -= delay;
								}
								else
								{
									break;
								}
							}
						}					
					}
					
					break;
				}
			}
			
			break;
			
		case WM_LBUTTONDBLCLK:

			// // 0 = scroll, 1 = play/pause slideshow, 2 = play/pause animation, 3=zoom in, 4=zoom out, 5=next, 6=prev, 
			switch(viv_left_click_action)
			{
				case 0:
				case 1:
				case 2:
					viv_toggle_fullscreen();
					break;

				default:
					SendMessage(hwnd,WM_LBUTTONDOWN,wParam,lParam);
					break;
			}
			break;
			
		case WM_LBUTTONDOWN:
		
			// // 0 = scroll, 1 = play/pause slideshow, 2 = play/pause animation, 3=zoom in, 4=zoom out, 5=next, 6=prev, 
			switch(viv_left_click_action)
			{
				case 0:
				
					if (!viv_doing)
					{
						if (!viv_is_fullscreen)
						{
							if (!viv_is_show_caption)
							{
								SendMessage(hwnd,WM_NCLBUTTONDOWN,(WPARAM)HTCAPTION,lParam);
								return 0;
							}				
						}

						viv_doing = 1;
						viv_doing_x = GET_X_LPARAM(lParam);
						viv_doing_y = GET_Y_LPARAM(lParam);
						SetCapture(hwnd);
					}
					
					break;

				case 1:
					viv_pause();				
					break;

				case 2:
					viv_animation_pause();
					break;

				case 3:
					viv_zoom_in(0,1,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
					break;

				case 4:
					viv_next(0,1);
					break;
			}
			
			break;
			
		case WM_XBUTTONDOWN:

			switch(HIWORD(wParam))
			{
				case XBUTTON1:
					viv_next(1,1);
					break;

				case XBUTTON2:
					viv_next(0,1);
					break;
			}
			
			break;
			
		case WM_MBUTTONDOWN:
			if (!viv_doing)
			{
				SetCapture(hwnd);
				viv_doing = 2;
				viv_doing_x = GET_X_LPARAM(lParam);
				viv_doing_y = GET_Y_LPARAM(lParam);
				{
					POINT pt;
					GetCursorPos(&pt);
					
					viv_mdoing_x = pt.x;
					viv_mdoing_y = pt.y;
				}
				ShowCursor(FALSE);
			}
			break;
			
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:

			switch(viv_right_click_action)
			{
				case 0:
					break;

				case 1:
					viv_zoom_in(1,1,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
					return 0;

				case 2:
					viv_next(1,1);
					return 0;
			}

			break;		
			
		case WM_RBUTTONUP:

			switch(viv_right_click_action)
			{
				case 1:
					return 0;

				case 2:
					return 0;
			}

			break;
			
		case WM_CONTEXTMENU:
		{
			HMENU hmenu;
			POINT pt;
			DWORD tpm_flags;
			
			tpm_flags = 0;
			
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			
			if ((pt.x == -1) && (pt.y == -1))
			{
				RECT rect;
				
				GetWindowRect(hwnd,&rect);
				
				pt.x = (rect.right + rect.left) / 2;
				pt.y = (rect.bottom + rect.top) / 2;
				
				tpm_flags |= TPM_VCENTERALIGN | TPM_CENTERALIGN;
			}
			
			hmenu = CreatePopupMenu();
			
			{
				int i;
				HMENU submenu;
				HMENU curmenu;
				WORD submenuid;
				int was_seperator;
				
				submenuid = 0;
				curmenu = hmenu;
				was_seperator = 1;
				
				for(i=0;i<VIV_CONEXT_MENU_ITEM_COUNT;i++)
				{
					if (viv_context_menu_items[i] > VIV_MENU_COUNT)
					{
						if ((viv_context_menu_items[i] != VIV_ID_VIEW_MENU) || (!viv_is_show_menu))
						{
							int command_index;
							
							command_index = viv_command_index_from_command_id(viv_context_menu_items[i]);
							
							// its a command
							if (command_index != -1)
							{
								wchar_t text_wbuf[STRING_SIZE];
								wchar_t key_text[STRING_SIZE];
								
								string_copy_utf8(text_wbuf,viv_commands[command_index].name);

								if (viv_key_list->start[command_index])
								{
									viv_get_key_text(key_text,viv_key_list->start[command_index]->key);
									
									string_cat_utf8(text_wbuf,(const utf8_t *)"\t");
									string_cat(text_wbuf,key_text);
								}
									
								AppendMenu(curmenu,viv_commands[command_index].flags,viv_commands[command_index].command_id,text_wbuf);
								was_seperator = 0;
							}
						}
					}
					else
					if (viv_context_menu_items[i])
					{
						if (submenuid == viv_context_menu_items[i])
						{
							// pop submenu
							curmenu = hmenu;
							submenuid = 0;
						}
						else
						{
							int command_index;
							
							command_index = viv_command_index_from_command_id(viv_context_menu_items[i]);
							if (command_index != -1)
							{
								wchar_t text_wbuf[STRING_SIZE];
								
								// push submenu
								string_copy_utf8(text_wbuf,viv_commands[command_index].name);
								
								submenuid = viv_context_menu_items[i];
								submenu = CreatePopupMenu();
								AppendMenu(curmenu,MF_POPUP,(UINT_PTR)submenu,text_wbuf);
								curmenu = submenu;
								was_seperator = 0;
							}
						}
					}
					else
					{
						// seperator
						if (!was_seperator)
						{
							AppendMenu(curmenu,MF_SEPARATOR,0,0);

							was_seperator = 1;
						}
					}
				}
			}
	
			viv_check_menus(hmenu);
			
			if (!viv_cursor_shown)
			{
				ShowCursor(TRUE);
				
				viv_cursor_shown = 1;
			}

			viv_in_popup_menu = 1;
			
			TrackPopupMenu(hmenu,tpm_flags,pt.x,pt.y,0,hwnd,0);

			// start the hide cursor timer again.
			viv_in_popup_menu = 0;
			viv_show_cursor();
			
			DestroyMenu(hmenu);
			
			break;
		}
			
		case WM_ACTIVATE:
			if (!viv_cursor_shown)
			{
				ShowCursor(TRUE);
				
				viv_cursor_shown = 1;
			}
			break;
			
		case WM_MOUSEMOVE:
				
			viv_show_cursor();

			if (viv_doing)
			{
				int mx;
				int my;
				
				if (viv_doing == 2)
				{
					POINT pt;
					GetCursorPos(&pt);

					mx = viv_mdoing_x - pt.x;
					my = viv_mdoing_y - pt.y;
					
					if ((mx) || (my))
					{
						SetCursorPos(viv_mdoing_x,viv_mdoing_y);
					}
				}
				else
				{
					int x;
					int y;
					
					x = GET_X_LPARAM(lParam); 
					y = GET_Y_LPARAM(lParam); 

					mx = x - viv_doing_x;
					my = y - viv_doing_y;
					
					viv_doing_x = x;
					viv_doing_y = y;
				}
				
				if ((mx) || (my))
				{
					viv_view_scroll(mx,my);
//					UpdateWindow(viv_hwnd);
				}				
			}
			
			break;
			
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		
			viv_doing_cancel();
			
			break;
			
		case WM_MOUSEWHEEL:
		{
			if (viv_mouse_wheel_action == 0)
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
				
				GetClientRect(hwnd,&rect);
				wide = rect.right - rect.left;
				high = rect.bottom - rect.top - viv_get_status_high() - viv_get_controls_high();
				
				pt.x = GET_X_LPARAM(lParam); 
				pt.y = GET_Y_LPARAM(lParam);
				
				ScreenToClient(hwnd,&pt);

				cursor_x = pt.x; 
				cursor_y = pt.y;
			
				viv_get_render_size(&rw,&rh);
				
	/*
				if (viv_zoom_pos == 1)
				{
					if ((rw < viv_image_wide) || (rw < viv_image_wide))
					{
						rw = viv_image_wide;
						rh = viv_image_high;
					}
				}
				*/
				rx = (wide / 2) - (rw / 2) - viv_view_x;
				ry = (high / 2) - (rh / 2) - viv_view_y;
				
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
				if (viv_1to1)
				{
					viv_1to1 = 0;
					
					if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
					{
						for(viv_zoom_pos = 0;viv_zoom_pos<VIV_ZOOM_MAX;viv_zoom_pos++)
						{
							viv_get_render_size(&rw,&rh);
							
							if (rw > old_rw)
							{
								break;
							}
						}
					}
					else
					{
						for(viv_zoom_pos = VIV_ZOOM_MAX-1;viv_zoom_pos>=0;viv_zoom_pos--)
						{
							viv_get_render_size(&rw,&rh);
							
							if (rw < old_rw)
							{
								break;
							}
						}
					}
				}
				else
				{
					if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
					{
						viv_zoom_pos++;
					}
					else
					{
						viv_zoom_pos--;
					}
				}
				
				if (viv_zoom_pos < 0) viv_zoom_pos = 0;
				if (viv_zoom_pos > VIV_ZOOM_MAX-1) viv_zoom_pos = VIV_ZOOM_MAX-1;
				
				viv_get_render_size(&rw,&rh);
	/*
				if (viv_zoom_pos == 1)
				{
					if ((rw < viv_image_wide) || (rw < viv_image_wide))
					{
						rw = viv_image_wide;
						rh = viv_image_high;
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

				viv_view_set((wide / 2) - (rw / 2) - cursor_x + new_cursor_x,(high / 2) - (rh / 2) - cursor_y + new_cursor_y,1);
				
				InvalidateRect(hwnd,0,FALSE);
			}
			else
			if (viv_mouse_wheel_action == 1)
			{
				if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
				{
					viv_next(1,1);
				}
				else
				if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
				{
					viv_next(0,1);
				}
			}
			else
			if (viv_mouse_wheel_action == 2)
			{
				if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
				{
					viv_next(0,1);
				}
				else
				if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
				{
					viv_next(1,1);
				}
			}
			
			break;
		}
		
		case WM_COPYDATA:
		{
			COPYDATASTRUCT *cds;
				
			cds = (COPYDATASTRUCT *)lParam;

			switch(cds->dwData)
			{
				case 0:
				{
					DWORD showcmd;
					const char *p;
					const char *e;
					wchar_t cl[STRING_SIZE];

					SetForegroundWindow(hwnd);
					
					p = (char *)cds->lpData;
					e = p + cds->cbData;
					
					// is there a show command ?
					if (e-p >= sizeof(DWORD))
					{
						wchar_t cwd[STRING_SIZE];
						
						showcmd = *(DWORD *)p;
						p += sizeof(DWORD);
						
						p = _viv_get_copydata_string(p,e,cl,STRING_SIZE);
						p = _viv_get_copydata_string(p,e,cwd,STRING_SIZE);
						
						viv_dragdrop_clearall();
						
						debug_printf("set cwd %S\n",cwd);
						SetCurrentDirectory(cwd);
						
						viv_process_command_line(cl);
						
						ShowWindow(viv_hwnd,showcmd);
					}
										
					return 1;
				}
			}
			
			break;
		}
		
		case WM_CLOSE:
			viv_exit();
			return 0;
		
		case WM_SYSCOMMAND:	
			
			switch(wParam)
			{
				case SC_MONITORPOWER:
				case SC_SCREENSAVE:

					// prevent sleep / monitor power off.

					if (viv_allow_sleep)
					{
						if (viv_is_slideshow)
						{
							return 0;
						}
						
						if (viv_is_animation_timer)
						{
							if (viv_animation_play)
							{
								return 0;
							}
						}
					}
					
					break;
			}
			break;
		
		case WM_SIZE:
		
			viv_on_size();

			break;
			
		case WM_MOVE:
		{
			if (!IsIconic(hwnd))
			{
				if (!IsMaximized(hwnd))
				{
					if (!viv_is_fullscreen)
					{
						RECT rect;
						
						GetWindowRect(hwnd,&rect);
			
						viv_x = rect.left;
						viv_y = rect.top;
					}
				}
			}
			
			break;
		}
			
		case WM_NOTIFY:

			switch(((NMHDR *)lParam)->idFrom)
			{
				case VIV_ID_STATUS:
				
					if (viv_status_hwnd)
					{

						switch(((NMHDR *)lParam)->code)
						{
							case NM_CLICK:
							{
								int item;
								
								item = ((NMMOUSE *)lParam)->dwItemSpec;

								// if we hit nothing use the last part.
								if (item < 0) 
								{
									item = (int)SendMessage(viv_status_hwnd,SB_GETPARTS,0,0) - 1;
								}
								
								switch(item)
								{
									case 1:
										viv_frame_minus = !viv_frame_minus;
										viv_status_update();
										break;
									
									default:
										break;
								}
															
								break;
							}
						}
					}
	
					break;
			}
			
			break;
			
		case WM_COMMAND:
		{
			viv_command(LOWORD(wParam));
		
			break;
		}
		
		case WM_PASTE:
		
		debug_printf("paste\n");
		
			if (OpenClipboard(hwnd))
			{
				HGLOBAL hglobal;
				
				// try a hdrop
				hglobal = GetClipboardData(CF_HDROP);
				
				if (hglobal)
				{
					HDROP hdrop;
					
					hdrop = (HDROP)GlobalLock(hglobal);
					if (hdrop)
					{
						SendMessage(hwnd,WM_DROPFILES,(WPARAM)hdrop,0);

						GlobalUnlock(hglobal);
					}
				}

				CloseClipboard();
			}

			break;
		
		case WM_PAINT:
		{
			RECT rect;
			int wide;
			int high;
			PAINTSTRUCT ps;
			
			GetClientRect(hwnd,&rect);
			wide = rect.right - rect.left;
			high = rect.bottom - rect.top - viv_get_status_high() - viv_get_controls_high();

			BeginPaint(hwnd,&ps);

			if ((wide) && (high))
			{
				// controls.
				/*
				if (viv_get_controls_high())
				{
					rect.left = 0;
					rect.top = high;
					rect.right = wide;
					rect.bottom = rect.top + viv_get_controls_high();
					
					FillRect(ps.hdc,&rect,(HBRUSH)(COLOR_WINDOW+1));
					ExcludeClipRect(ps.hdc,rect.left,rect.top,rect.right,rect.bottom);
				}
			*/
				if (viv_frame_count)
				{
					int rx;
					int ry;
					int rw;
					int rh;
					HDC mem_hdc;
					
					viv_get_render_size(&rw,&rh);

					rw = (int)(rw * viv_dst_zoom_values[viv_dst_zoom_x_pos]);
					rh = (int)(rh * viv_dst_zoom_values[viv_dst_zoom_y_pos]);
					
		#if 0
					if (viv_zoom_pos == 1)
					{
						if ((rw < viv_image_wide) || (rw < viv_image_wide))
						{
							rw = viv_image_wide;
							rh = viv_image_high;
						}
					}
		#endif
					
					rx = ((viv_dst_pos_x * wide) / 1000) - (rw / 2) - viv_view_x;
					ry = ((viv_dst_pos_y * high) / 1000) - (rh / 2) - viv_view_y;
					
		//			rh += (-dst_left_offset + dst_right_offset) * wide / 1000;
		//			rh += (-dst_top_offset + dst_bottom_offset) * high / 1000;

					mem_hdc = CreateCompatibleDC(ps.hdc);
					if (mem_hdc)
					{
						if (viv_frames[viv_frame_position].hbitmap)
						{
							HGDIOBJ last_hbitmap;
							
							last_hbitmap = SelectObject(mem_hdc,viv_frames[viv_frame_position].hbitmap);
							
							
							if ((rw == viv_image_wide) && (rh == viv_image_high))
							{
								BitBlt(ps.hdc,rx,ry,rw,rh,mem_hdc,0,0,SRCCOPY);
							}
							else
							{
								int last_stretch_mode;
								int did_set_stretch_blt_mode;
								
								did_set_stretch_blt_mode = 0;
								
								if ((rw < viv_image_wide) || (rh < viv_image_high))
								{
									if (viv_shrink_blit_mode == VIV_SHRINK_BLIT_MODE_HALFTONE)
									{
										last_stretch_mode = SetStretchBltMode(ps.hdc,HALFTONE);
										SetBrushOrgEx(ps.hdc,-rx,-ry,NULL);
									}
									else
									{
										last_stretch_mode = SetStretchBltMode(ps.hdc,COLORONCOLOR);
									}
									
									did_set_stretch_blt_mode = 1;
								}
								else
								if ((rw > viv_image_wide) || (rh > viv_image_high))
								{
									if (viv_mag_filter == VIV_MAG_FILTER_HALFTONE)
									{
										last_stretch_mode = SetStretchBltMode(ps.hdc,HALFTONE);
									}
									else
									{
										last_stretch_mode = SetStretchBltMode(ps.hdc,COLORONCOLOR);
									}
									
									did_set_stretch_blt_mode = 1;
								}
								
								StretchBlt(ps.hdc,rx,ry,rw,rh,mem_hdc,0,0,viv_image_wide,viv_image_high,SRCCOPY);

								if (did_set_stretch_blt_mode)
								{
									SetStretchBltMode(ps.hdc,last_stretch_mode);
								}
							}
								
							ExcludeClipRect(ps.hdc,rx,ry,rx+rw,ry+rh);

							SelectObject(mem_hdc,last_hbitmap);
						}
						
						DeleteDC(mem_hdc);
					}
				}

				{
					HBRUSH hbrush;
					
					hbrush = CreateSolidBrush(viv_is_fullscreen ? RGB(viv_fullscreen_background_color_r,viv_fullscreen_background_color_g,viv_fullscreen_background_color_b) : RGB(viv_windowed_background_color_r,viv_windowed_background_color_g,viv_windowed_background_color_b));
					
					FillRect(ps.hdc,&rect,hbrush);
					
					DeleteObject(hbrush);
				}
			}
			
			EndPaint(hwnd,&ps);
			
			return 0;
		}
			
		case WM_ERASEBKGND:
			return 1;
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static void viv_load_settings_by_location(const wchar_t *path,int is_root)
{
	ini_t *ini;
	wchar_t filename[STRING_SIZE];
	
	string_path_combine_utf8(filename,path,(const utf8_t *)"voidImageViewer.ini");
	
	ini = ini_open(filename,"voidImageViewer");
	if (ini)
	{
		ini_get_int(ini,(const utf8_t *)"x",&viv_x);
		ini_get_int(ini,(const utf8_t *)"y",&viv_y);
		ini_get_int(ini,(const utf8_t *)"wide",&viv_wide);
		ini_get_int(ini,(const utf8_t *)"high",&viv_high);
		ini_get_int(ini,(const utf8_t *)"slideshow_rate",&viv_slideshow_rate);
		ini_get_int(ini,(const utf8_t *)"allow_shrinking",&viv_allow_shrinking);
		ini_get_int(ini,(const utf8_t *)"shrink_blit_mode",&viv_shrink_blit_mode);
		ini_get_int(ini,(const utf8_t *)"mag_filter",&viv_mag_filter);
		ini_get_int(ini,(const utf8_t *)"keep_aspect_ratio",&viv_keep_aspect_ratio);
		ini_get_int(ini,(const utf8_t *)"fill_window",&viv_fill_window);
		ini_get_int(ini,(const utf8_t *)"fullscreen_fill_window",&viv_fullscreen_fill_window);
		ini_get_int(ini,(const utf8_t *)"sort",&viv_nav_sort);
		ini_get_int(ini,(const utf8_t *)"sort_ascending",&viv_nav_sort_ascending);
		ini_get_int(ini,(const utf8_t *)"multiple_instances",&viv_multiple_instances);
		ini_get_int(ini,(const utf8_t *)"show_status",&viv_is_show_status);
		ini_get_int(ini,(const utf8_t *)"show_controls",&viv_is_show_controls);
		ini_get_int(ini,(const utf8_t *)"auto_zoom",&viv_auto_zoom);
		ini_get_int(ini,(const utf8_t *)"auto_zoom_type",&viv_auto_zoom_type);
		ini_get_int(ini,(const utf8_t *)"frame_minus",&viv_frame_minus);
		ini_get_int(ini,(const utf8_t *)"mouse_wheel_action",&viv_mouse_wheel_action);
		ini_get_int(ini,(const utf8_t *)"left_click_action",&viv_left_click_action);
		ini_get_int(ini,(const utf8_t *)"right_click_action",&viv_right_click_action);
		ini_get_int(ini,(const utf8_t *)"keep_centered",&viv_keep_centered);
		ini_get_int(ini,(const utf8_t *)"windowed_background_color_r",&viv_windowed_background_color_r);
		ini_get_int(ini,(const utf8_t *)"windowed_background_color_g",&viv_windowed_background_color_g);
		ini_get_int(ini,(const utf8_t *)"windowed_background_color_b",&viv_windowed_background_color_b);
		ini_get_int(ini,(const utf8_t *)"fullscreen_background_color_r",&viv_fullscreen_background_color_r);
		ini_get_int(ini,(const utf8_t *)"fullscreen_background_color_g",&viv_fullscreen_background_color_g);
		ini_get_int(ini,(const utf8_t *)"fullscreen_background_color_b",&viv_fullscreen_background_color_b);
		ini_get_int(ini,(const utf8_t *)"options_last_page",&viv_options_last_page);
		ini_get_int(ini,(const utf8_t *)"short_jump",&viv_short_jump);
		ini_get_int(ini,(const utf8_t *)"medium_jump",&viv_medium_jump);
		ini_get_int(ini,(const utf8_t *)"long_jump",&viv_long_jump);
		ini_get_int(ini,(const utf8_t *)"loop_animations_once",&viv_loop_animations_once);
		ini_get_int(ini,(const utf8_t *)"allow_sleep",&viv_allow_sleep);
		
		if (is_root)
		{
			ini_get_int(ini,(const utf8_t *)(const utf8_t *)"appdata",&viv_appdata);
		}
		
		{
			int i;
			
			for(i=0;i<VIV_COMMAND_COUNT;i++)
			{
				if (!(viv_commands[i].flags & MF_POPUP))
				{
					if (!(viv_commands[i].flags & MF_SEPARATOR))
					{
						utf8_t key_buf[STRING_SIZE];
						const utf8_t *key_list;
						const utf8_t *p;
						
						// get ini key name.
						
						viv_menu_name_to_ini_name(key_buf,i);
						
						key_list = ini_get_string(ini,key_buf);
						
						if (key_list)
						{
							p = key_list;
							
							// clear this key.
							viv_key_clear_all(viv_key_list,i);
							
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

								viv_key_add(viv_key_list,i,utf8_to_int(start));
							}
						}
					}
				}
			}
		}
		
		ini_close(ini);
	}
}

static void viv_load_settings(void)
{
	wchar_t path[STRING_SIZE];
	
	string_get_exe_path(path);
	
	viv_load_settings_by_location(path,1);
		
	if (viv_appdata)
	{
		if (viv_get_appdata_voidimageviewer_path(path))
		{
			viv_load_settings_by_location(path,0);
		}
	}
}

static int viv_is_ws(wchar_t c)
{
	switch(c)
	{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			return 1;
	}
	
	return 0;
}

static wchar_t *viv_skip_ws(wchar_t *p)
{
	while(*p)
	{
		if (!viv_is_ws(*p))
		{
			break;
		}
		
		p++;
	}
	
	return p;
}

static wchar_t *viv_get_word(wchar_t *p,wchar_t *buf)
{
	wchar_t *d;
	int is_quote;
	
	d = buf;
	is_quote = 0;

	while(*p)
	{
		if ((*p == '"') && (p[1] == '"'))
		{
			p += 2;
			*d++ = '"';
		}
		else
		if (*p == '"')
		{
			is_quote = !is_quote;
			p++;
		}
		else
		if ((!is_quote) && (viv_is_ws(*p)))
		{
			break;
		}
		else
		{
			*d++ = *p;
			p++;
		}
	}
	
	*d = 0;
	
	return p;
}

static int viv_process_install_command_line_options(wchar_t *cl)
{
	int ret;
	wchar_t *p;
	wchar_t buf[STRING_SIZE];
	DWORD install_flags;
	DWORD uninstall_flags;
	int appdata;
	int is_runas;
	wchar_t install_path[STRING_SIZE];
	wchar_t install_options[STRING_SIZE];
	wchar_t uninstall_path[STRING_SIZE];
	int startmenu;
	wchar_t *cl_start;
	
	startmenu = 0;
	install_flags = 0;
	uninstall_flags = 0;
	appdata = 0;
	is_runas = 0;
	ret = 0;
	install_path[0] = 0;
	install_options[0] = 0;
	uninstall_path[0] = 0;

	p = viv_skip_ws(cl);
	
	// skip filename
	p = viv_get_word(p,buf);
	p = viv_skip_ws(p);

	cl_start = p;

	// skip first parameter.
	for(;;)
	{
		wchar_t *bufstart;
		
		// no more text?
		if (!*p)
		{
			break;
		}
		
		p = viv_get_word(p,buf);
		p = viv_skip_ws(p);
		
		bufstart = buf;
		
		if (*bufstart == '/')
		{
			bufstart++;
			
			if (string_icompare_lowercase_ascii(bufstart,"install") == 0)
			{
				p = viv_get_word(p,install_path);
				p = viv_skip_ws(p);				

				uninstall_path[0] = 0;

				ret = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"install-options") == 0)
			{
				p = viv_get_word(p,install_options);
				p = viv_skip_ws(p);				

				ret = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"uninstall") == 0)
			{
				p = viv_get_word(p,uninstall_path);
				p = viv_skip_ws(p);			
				
				// no uninstall path?
				if (!uninstall_path[0])	
				{
					// caller will need to manually delete voidimageviewer.exe
					string_get_exe_path(uninstall_path);
				}

				// uninstall all
				uninstall_flags = 0xffffffff;
				install_flags = 0;
				startmenu = -1;

				install_path[0] = 0;
				
				ret = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"appdata") == 0)
			{
				appdata = 1;
				ret = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"noappdata") == 0)
			{
				appdata = -1;
				ret = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"startmenu") == 0)
			{
				startmenu = 1;
				ret = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"nostartmenu") == 0)
			{
				startmenu = -1;
				ret = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"isrunas") == 0)
			{
				is_runas = 1;
			}
			else
			{
				int exti;
				int is_no;
				
				is_no = 0;
			
				if (string_istartwith_lowercase_ascii(bufstart,"no"))
				{
					bufstart += 2;
					is_no = 1;
				}
				
				for(exti=0;exti<VIV_ASSOCIATION_COUNT;exti++)
				{
					if (string_icompare_lowercase_ascii(bufstart,viv_association_extensions[exti]) == 0)
					{
						if (is_no)
						{
							uninstall_flags |= 1 << exti;
						}
						else
						{
							install_flags |= 1 << exti;
						}
						
						ret = 1;
						break;
					}
				}
			}
		}
	}

	if (ret)
	{
		if (!os_is_admin())
		{
			if (!is_runas)
			{
				wchar_t exe_filename[STRING_SIZE];
				wchar_t params[STRING_SIZE];
				wchar_t cwd[STRING_SIZE];
				
				GetModuleFileName(0,exe_filename,STRING_SIZE);
				GetCurrentDirectory(STRING_SIZE,cwd);
				
				string_copy_utf8(params,(const utf8_t *)"/isrunas ");
				string_cat(params,cl_start);
				
				os_shell_execute(0,exe_filename,1,"runas",params);
				
				return 1;
			}
		}
	}
	
	if (install_flags)
	{
		viv_install_association(install_flags);
	}
	
	if (uninstall_flags)
	{
		viv_uninstall_association(uninstall_flags);
	}
		
	if (appdata > 0)
	{	
		viv_appdata = 1;
		
		viv_save_settings(viv_appdata);

		// appdata enabled.
		viv_save_settings(0);
	}
	else
	if (appdata < 0)
	{
		viv_appdata = 0;
		
		// appdata disabled
		viv_save_settings(0);
	}
	
	if (startmenu > 0)
	{
		viv_install_start_menu_shortcuts();
	}
	else
	if (startmenu < 0)
	{
		viv_uninstall_start_menu_shortcuts();
	}
	
	if (install_path[0])
	{
		wchar_t temp_path[STRING_SIZE];
		
		// make sure no other process is running.
		viv_close_existing_process();
		
		string_get_exe_path(temp_path);
		
		os_make_sure_path_exists(install_path);
		
		viv_install_copy_file(install_path,temp_path,(const utf8_t *)"voidImageViewer.exe",1);
		viv_install_copy_file(install_path,temp_path,(const utf8_t *)"Uninstall.exe",0);
		viv_install_copy_file(install_path,temp_path,(const utf8_t *)"Changes.txt",0);
		
		if (install_options[0])
		{
			wchar_t new_exe_filename_wbuf[STRING_SIZE];
			
			string_path_combine_utf8(new_exe_filename_wbuf,install_path,(const utf8_t *)"voidImageViewer.exe");
			
			os_shell_execute(0,new_exe_filename_wbuf,1,NULL,install_options);
		}
	}
	
	if (uninstall_path[0])
	{
		wchar_t path[STRING_SIZE];
		
		// make sure no other process is running.
		viv_close_existing_process();
		
		// remove %APPDATA%\voidimageviewer
		if (viv_get_appdata_voidimageviewer_path(path))
		{
			viv_uninstall_delete_file(path,(const utf8_t *)"voidImageViewer.ini");

			RemoveDirectory(path);
		}
					
		viv_uninstall_delete_file(uninstall_path,(const utf8_t *)"Uninstall.exe");
		viv_uninstall_delete_file(uninstall_path,(const utf8_t *)"Changes.txt");
		viv_uninstall_delete_file(uninstall_path,(const utf8_t *)"voidImageViewer.ini");
		viv_uninstall_delete_file(uninstall_path,(const utf8_t *)"voidImageViewer.exe");
		
		RemoveDirectory(uninstall_path);
	}
	
	return ret;
}

static void viv_process_command_line(wchar_t *cl)
{
	wchar_t *p;
	wchar_t buf[STRING_SIZE];
	wchar_t single[STRING_SIZE];
	int file_count;
	int start_slideshow;
	int start_fullscreen;
	int start_window;
	
	debug_printf("cl: %S\n",cl);
	
	start_slideshow = 0;
	start_fullscreen = 0;
	start_window = 0;
	p = cl;
	file_count = 0;
	single[0] = 0;
	
	p = viv_skip_ws(p);
	
	// skip filename
	p = viv_get_word(p,buf);
	p = viv_skip_ws(p);

	// skip first parameter.
	for(;;)
	{
		wchar_t *bufstart;
		// no more text?
		if (!*p)
		{
			break;
		}
		
		p = viv_get_word(p,buf);
		p = viv_skip_ws(p);
		
		bufstart = buf;
		
		if (*bufstart == '/')
		{
			bufstart++;
			
			if (string_icompare_lowercase_ascii(bufstart,"slideshow") == 0)
			{
				start_slideshow = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"fullscreen") == 0)
			{
				start_fullscreen = 1;	
				start_window = 0;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"window") == 0)
			{
				start_fullscreen = 0;	
				start_window = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"x") == 0)
			{
				p = viv_get_word(p,buf);
				p = viv_skip_ws(p);

				viv_x = string_to_int(buf);
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"y") == 0)
			{
				p = viv_get_word(p,buf);
				p = viv_skip_ws(p);

				viv_y = string_to_int(buf);
			}		
			else
			if (string_icompare_lowercase_ascii(bufstart,"rate") == 0)
			{
				p = viv_get_word(p,buf);
				p = viv_skip_ws(p);

				viv_slideshow_rate = string_to_int(buf);
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"name") == 0)
			{
				viv_nav_sort = VIV_NAV_SORT_NAME;
				viv_nav_sort_ascending = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"dm") == 0)
			{
				viv_nav_sort = VIV_NAV_SORT_DATE_MODIFIED;
				viv_nav_sort_ascending = 0;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"dc") == 0)
			{
				viv_nav_sort = VIV_NAV_SORT_DATE_MODIFIED;
				viv_nav_sort_ascending = 0;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"path") == 0)
			{
				viv_nav_sort = VIV_NAV_SORT_FULL_PATH_AND_FILENAME;
				viv_nav_sort_ascending = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"size") == 0)
			{
				viv_nav_sort = VIV_NAV_SORT_SIZE;
				viv_nav_sort_ascending = 0;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"ascending") == 0)
			{
				viv_nav_sort_ascending = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"descending") == 0)
			{
				viv_nav_sort_ascending = 0;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"isrunas") == 0)
			{
				// ignore this for non-install commands.
			}
			else
			{
				viv_command_line_options();
			}
		}
		else
		{
			if (*buf)
			{
				if (file_count == 1)
				{
					// add the last single
					viv_dragdrop_add_filename(single);
				}

				if (file_count >= 1)
				{
					// add this file
					viv_dragdrop_add_filename(buf);
				}
			
				if (file_count == 0)
				{
					string_copy(single,buf);
				}
				
				file_count++;
			}
		}
	}
	
	debug_printf("file count %d\n",file_count);

	if (file_count >= 1)
	{
		viv_dragdrop_clearall();
		
		// open the first image specified.
		viv_open_from_filename(single);
	}
	
	if (start_slideshow)
	{
		viv_slideshow();
	}
	
	if (start_fullscreen)
	{
		if (!viv_is_fullscreen)
		{
			viv_toggle_fullscreen();
		}
	}
	
	if (start_window)
	{
		if (viv_is_fullscreen)
		{
			viv_toggle_fullscreen();
		}
	}
}

// unicode on Windows 9x.
HMODULE LoadUnicowsProc(void)
{
    return(LoadLibraryA("unicows.dll"));
}

static int viv_init(void)
{
	RECT rect;
	STARTUPINFO si;
	
	os_init();
	
	viv_key_list = mem_alloc(sizeof(viv_key_list_t));
	
	viv_key_list_init(viv_key_list);
	
	// setup keys.
	{
		int i;
		
		for(i=0;i<VIV_DEFAULT_KEY_COUNT;i++)
		{
			viv_key_add(viv_key_list,viv_command_index_from_command_id(viv_default_keys[i].command_id),viv_default_keys[i].key_flags);
		}
	}
	
	debug_printf("viv %d.%d.%d.%d%s %s\n",VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION,VERSION_BUILD,VERSION_TYPE,VERSION_TARGET_MACHINE);
	
	os_hinstance = GetModuleHandle(0);
	
	viv_current_fd = (WIN32_FIND_DATA *)mem_alloc(sizeof(WIN32_FIND_DATA));
	
	debug_printf("CoInitializeEx\n");
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);
	
    // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
    DefWindowProc(NULL,0,0,0);
    
	// init common controls
	
	debug_printf("InitCommonControlsEx\n");
	{
		INITCOMMONCONTROLSEX icex;

		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_WIN95_CLASSES;
		InitCommonControlsEx(&icex);
	}

	InitializeCriticalSection(&viv_cs);
	
	// Initialize GDI+.
	{
		os_GdiplusStartupInput_t gdiplusStartupInput;
		int gdiplus_ret;
		
		gdiplusStartupInput.GdiplusVersion = 1;
		gdiplusStartupInput.DebugEventCallback = NULL;
		gdiplusStartupInput.SuppressBackgroundThread = FALSE;
		gdiplusStartupInput.SuppressExternalCodecs = FALSE;
	
		gdiplus_ret = os_GdiplusStartup(&os_GdiplusToken,&gdiplusStartupInput,NULL);
	}

	// load settings
	viv_load_settings();
	
	// process install command line options
	{
		if (viv_process_install_command_line_options(GetCommandLineW()))
		{
			viv_kill();
			
			return 0;
		}
	}

	// mutex
	if (!viv_multiple_instances)
	{
		SetLastError(0);

		viv_mutex = CreateMutexA(NULL,0,"VOIDIMAGEVIEWER");
		
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			HWND hwnd;
			
			hwnd = FindWindowA("VOIDIMAGEVIEWER",0);
			
			if (hwnd)
			{
				COPYDATASTRUCT cds;
				wchar_t *command_line;
				wchar_t cwd[STRING_SIZE];
				int size;
				char *buf;
				char *d;

				// allow this process to set focus
				SetForegroundWindow(hwnd);
				
				command_line = GetCommandLineW();
				GetCurrentDirectory(STRING_SIZE,cwd);
				
				si.cb = sizeof(STARTUPINFO);
				GetStartupInfo(&si);
				
				// calc size
				size = sizeof(DWORD) + (((int)string_length(command_line) + 1) * sizeof(wchar_t)) + (((int)string_length(cwd) + 1) * sizeof(wchar_t));
				buf = (char *)mem_alloc(size);
				
				// fill in
				d = buf;
				*(DWORD *)d = si.wShowWindow;
				d += sizeof(DWORD);
				string_copy((wchar_t *)d,command_line);
				d += ((string_length(command_line) + 1) * sizeof(wchar_t));
				string_copy((wchar_t *)d,cwd);
				d += ((string_length(cwd) + 1) * sizeof(wchar_t));

				// setup copydata struct
				cds.lpData = buf;
				cds.cbData = size;
				cds.dwData = 0;

				SendMessage(hwnd,WM_COPYDATA,(WPARAM)0,(LPARAM)&cds);
				
				mem_free(buf);
			}
			
			viv_kill();
			
			return 0;
		}
	}
	
	os_RegisterClassEx(
		CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW,
		viv_proc,
		(HICON)LoadImage(os_hinstance,MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON,GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CXICON),0),
		LoadCursor(NULL,IDC_ARROW),
		(HBRUSH)(COLOR_BTNFACE+1),
		"VOIDIMAGEVIEWER",
		(HICON)LoadImage(os_hinstance,MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0));
	
	viv_hmenu = viv_create_menu();
	
	rect.left = viv_x;
	rect.top = viv_y;
	rect.right = viv_x + viv_wide;
	rect.bottom = viv_y + viv_high;
	
	os_make_rect_completely_visible(&rect);
	
	viv_hwnd = os_CreateWindowEx(
		0,
		"VOIDIMAGEVIEWER",
		"void Image Viewer",
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,
		0,viv_hmenu,os_hinstance,NULL);
		
	// allow non-admin/admins to close this window.
	if (os_ChangeWindowMessageFilterEx)
	{
		// MSGFLT_ALLOW = 1
		os_ChangeWindowMessageFilterEx(viv_hwnd,WM_CLOSE,1,0);
	}
		
	viv_status_show(viv_is_show_status);
	viv_controls_show(viv_is_show_controls);
	
	DragAcceptFiles(viv_hwnd,TRUE);

	viv_update_title();

	viv_process_command_line(GetCommandLineW());

	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	
	ShowWindow(viv_hwnd,si.wShowWindow);
	UpdateWindow(viv_hwnd);
	
	return 1;
}

static void viv_kill(void)
{
	// don't load another image..
	if (viv_load_image_next_fd)
	{
		mem_free(viv_load_image_next_fd);
		
		viv_load_image_next_fd = 0;
	}
	
	// stop load_image immediately...
	if (viv_load_image_thread)
	{
		viv_load_image_terminate = 1;
		
		// it's critical we wait for load image to finish before we kill the main window.
		WaitForSingleObject(viv_load_image_thread,INFINITE);
		
		CloseHandle(viv_load_image_thread);
	}
	
	if (viv_load_image_filename)
	{
		mem_free(viv_load_image_filename);
	}
	
	viv_clear();

	viv_status_show(0);
	viv_controls_show(0);

	if (viv_hwnd)
	{
		DestroyWindow(viv_hwnd);
	}
	
	if (viv_status_temp_text)
	{
		mem_free(viv_status_temp_text);
	}
	
	viv_nav_item_free_all();
	
	if (viv_hmenu)
	{
		DestroyMenu(viv_hmenu);
	}

	if (viv_about_hfont)
	{
		DeleteObject(viv_about_hfont);
	}

	if (viv_mutex)
	{
		CloseHandle(viv_mutex);
	}

	viv_dragdrop_clearall();
	viv_reply_clear_all();

	os_GdiplusShutdown(os_GdiplusToken);

	if (_viv_stobject_hmodule)
	{
		FreeLibrary(_viv_stobject_hmodule);
	}
	
	CoUninitialize();
	
	DeleteCriticalSection(&viv_cs);

	if (viv_last_open_file)
	{
		mem_free(viv_last_open_file);
	}

	if (viv_last_open_folder)
	{
		mem_free(viv_last_open_folder);
	}

	mem_free(viv_current_fd);
	
	viv_key_list_clear(viv_key_list);
	mem_free(viv_key_list);
	
	os_kill();

#ifdef _DEBUG
	mem_debug();
#endif	
}

static int viv_main(void)
{
	if (viv_init())
	{

		for(;;)
		{
			// Main message loop:
			for(;;)
			{
				MSG msg;
				
				if (!PeekMessage(&msg,0,0,0,PM_REMOVE)) break;
				
				if (msg.message == WM_QUIT) goto exit;
				
				if (!viv_is_msg(&msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			
			WaitMessage();
		}
		
	exit:
		
		viv_kill();
	}

	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
	return viv_main();
}

int __cdecl main(int argc,char **argv)
{
	return viv_main();
}

static void viv_write_int(HANDLE h,const char *ascii_key,int value)
{
	wchar_t wbuf[STRING_SIZE];
	
	string_format_number(wbuf,value);
	
	viv_write_string(h,ascii_key,wbuf);
}

static void viv_write_string(HANDLE h,const char *ascii_key,const wchar_t *s)
{
	utf8_t buf[STRING_SIZE*3];
	
	WideCharToMultiByte(CP_UTF8,0,s,-1,(char *)buf,STRING_SIZE*3,0,0);
	
	viv_write_utf8(h,(const utf8_t *)ascii_key);
	viv_write_utf8(h,(const utf8_t *)"=");
	viv_write_utf8(h,buf);
	viv_write_utf8(h,(const utf8_t *)"\r\n");
}

static void viv_write_utf8(HANDLE h,const utf8_t *s)
{
	DWORD num_written;
	
	WriteFile(h,s,utf8_length(s),&num_written,0);
}

static void viv_save_settings_by_location(const wchar_t *path,int is_root)
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
		viv_write_utf8(h,(const utf8_t *)"[voidImageViewer]\r\n");
		
		if ((is_root) && (viv_appdata))
		{
			viv_write_int(h,"appdata",viv_appdata);
		}
		else
		{
			viv_write_int(h,"x",viv_x);
			viv_write_int(h,"y",viv_y);
			viv_write_int(h,"wide",viv_wide);
			viv_write_int(h,"high",viv_high);
			viv_write_int(h,"slideshow_rate",viv_slideshow_rate);
			viv_write_int(h,"allow_shrinking",viv_allow_shrinking);
			viv_write_int(h,"shrink_blit_mode",viv_shrink_blit_mode);
			viv_write_int(h,"mag_filter",viv_mag_filter);
			viv_write_int(h,"keep_aspect_ratio",viv_keep_aspect_ratio);
			viv_write_int(h,"fill_window",viv_fill_window);
			viv_write_int(h,"fullscreen_fill_window",viv_fullscreen_fill_window);
			viv_write_int(h,"sort",viv_nav_sort);
			viv_write_int(h,"sort_ascending",viv_nav_sort_ascending);
			viv_write_int(h,"multiple_instances",viv_multiple_instances);
			viv_write_int(h,"show_status",viv_is_show_status);
			viv_write_int(h,"show_controls",viv_is_show_controls);
			viv_write_int(h,"auto_zoom",viv_auto_zoom);
			viv_write_int(h,"auto_zoom_type",viv_auto_zoom_type);
			viv_write_int(h,"frame_minus",viv_frame_minus);
			viv_write_int(h,"mouse_wheel_action",viv_mouse_wheel_action);
			viv_write_int(h,"left_click_action",viv_left_click_action);
			viv_write_int(h,"right_click_action",viv_right_click_action);
			viv_write_int(h,"keep_centered",viv_keep_centered);
			viv_write_int(h,"windowed_background_color_r",viv_windowed_background_color_r);
			viv_write_int(h,"windowed_background_color_g",viv_windowed_background_color_g);
			viv_write_int(h,"windowed_background_color_b",viv_windowed_background_color_b);
			viv_write_int(h,"fullscreen_background_color_r",viv_fullscreen_background_color_r);
			viv_write_int(h,"fullscreen_background_color_g",viv_fullscreen_background_color_g);
			viv_write_int(h,"fullscreen_background_color_b",viv_fullscreen_background_color_b);
			viv_write_int(h,"options_last_page",viv_options_last_page);
			viv_write_int(h,"short_jump",viv_short_jump);
			viv_write_int(h,"medium_jump",viv_medium_jump);
			viv_write_int(h,"long_jump",viv_long_jump);
			viv_write_int(h,"loop_animations_once",viv_loop_animations_once);
			viv_write_int(h,"allow_sleep",viv_allow_sleep);
		
			// save keys
			{
				int i;
				
				for(i=0;i<VIV_COMMAND_COUNT;i++)
				{
					if (!(viv_commands[i].flags & MF_POPUP))
					{
						if (!(viv_commands[i].flags & MF_SEPARATOR))
						{
							utf8_t key_buf[STRING_SIZE];
							wchar_t keylist[STRING_SIZE];
							viv_key_t *key;
							
							// get ini key name.
							
							viv_menu_name_to_ini_name(key_buf,i);
							
							key = viv_key_list->start[i];
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

							viv_write_string(h,key_buf,keylist);
						}
					}
				}
			}	
			
			// save context menu items
//			viv_write_int(h,"context_menu_items",);
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

static void viv_save_settings(int appdata)
{
	if (appdata)
	{
		wchar_t path[STRING_SIZE];

		if (string_get_appdata_path(path))
		{
			wchar_t appdata_wbuf[STRING_SIZE];
			
			string_path_combine_utf8(appdata_wbuf,path,(const utf8_t *)"voidImageViewer");
			
			CreateDirectory(appdata_wbuf,NULL);

			viv_save_settings_by_location(appdata_wbuf,0);
		}
	}
	else
	{
		wchar_t path[STRING_SIZE];
		
		string_get_exe_path(path);
		
		viv_save_settings_by_location(path,1);
	}
}

static int viv_compare_filenames(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b)
{
	int ret;
	const wchar_t *afilename;
	const wchar_t *bfilename;

	afilename = string_get_filename_part(a->cFileName);
	bfilename = string_get_filename_part(b->cFileName);

	ret = CompareString(LOCALE_USER_DEFAULT,SORT_STRINGSORT|NORM_IGNORECASE,afilename,string_length(afilename),bfilename,string_length(bfilename));
	
	switch(ret)
	{
		case CSTR_LESS_THAN:	
			return -1;
			
		case CSTR_GREATER_THAN:
			return 1;
	}
	
	return 0;
}

static int viv_compare_full_path_and_filenames(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b)
{
	int ret;

	ret = CompareString(LOCALE_USER_DEFAULT,SORT_STRINGSORT|NORM_IGNORECASE,a->cFileName,string_length(a->cFileName),b->cFileName,string_length(b->cFileName));
	
	switch(ret)
	{
		case CSTR_LESS_THAN:	
			return -1;
			
		case CSTR_GREATER_THAN:
			return 1;
	}
	
	return 0;
}

static int viv_compare(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b)
{
	int ret;
	
	ret = 0;
	
	switch(viv_nav_sort)
	{
		case VIV_NAV_SORT_NAME:
			ret = viv_compare_filenames(a,b);
			break;

		case VIV_NAV_SORT_FULL_PATH_AND_FILENAME:
			ret = viv_compare_full_path_and_filenames(a,b);
			break;

		case VIV_NAV_SORT_SIZE:
		{
			LARGE_INTEGER sizea;
			LARGE_INTEGER sizeb;
			
			sizea.HighPart = a->nFileSizeHigh;
			sizea.LowPart = a->nFileSizeLow;

			sizeb.HighPart = b->nFileSizeHigh;
			sizeb.LowPart = b->nFileSizeLow;

			if (sizea.QuadPart < sizeb.QuadPart)
			{
				ret = -1;
			}
			else
			if (sizea.QuadPart > sizeb.QuadPart)
			{
				ret = 1;
			}
			else
			{
				// we want name ascending when we are size descending.
				ret = -viv_compare_filenames(a,b);
			}
			
			break;
		}

		case VIV_NAV_SORT_DATE_MODIFIED:
		{
			LARGE_INTEGER datea;
			LARGE_INTEGER dateb;
			
			datea.HighPart = a->ftLastWriteTime.dwHighDateTime;
			datea.LowPart = a->ftLastWriteTime.dwLowDateTime;

			dateb.HighPart = b->ftLastWriteTime.dwHighDateTime;
			dateb.LowPart = b->ftLastWriteTime.dwLowDateTime;

			if (datea.QuadPart < dateb.QuadPart)
			{
				ret = -1;
			}
			else
			if (datea.QuadPart > dateb.QuadPart)
			{
				ret = 1;
			}
			else
			{
				ret = -viv_compare_filenames(a,b);
			}
			
			break;
		}

		case VIV_NAV_SORT_DATE_CREATED:
		{
			LARGE_INTEGER datea;
			LARGE_INTEGER dateb;
			
			datea.HighPart = a->ftCreationTime.dwHighDateTime;
			datea.LowPart = a->ftCreationTime.dwLowDateTime;

			dateb.HighPart = b->ftCreationTime.dwHighDateTime;
			dateb.LowPart = b->ftCreationTime.dwLowDateTime;

			if (datea.QuadPart < dateb.QuadPart)
			{
				ret = -1;
			}
			else
			if (datea.QuadPart > dateb.QuadPart)
			{
				ret = 1;
			}
			else
			{
				ret = -viv_compare_filenames(a,b);
			}
			
			break;
		}
	}
	
	if (!viv_nav_sort_ascending)
	{
		ret *= -1;
	}
	
	return ret;
}

static void viv_next(int prev,int reset_slideshow_timer)
{
	if (*viv_current_fd->cFileName)	
	{
		WIN32_FIND_DATA start_fd;
		WIN32_FIND_DATA best_fd;
		int got_start;
		int got_best;

		got_best = 0;
		got_start = 0;
		
		if (viv_dragdrop_start)
		{
			viv_dragdrop_t *d;
			
			d = viv_dragdrop_start;
			while(d)
			{
				int compare_ret;
				
				compare_ret = viv_compare(&d->fd,viv_current_fd);

				if (compare_ret != 0)
				{
					if (prev)
					{
						if (compare_ret < 0)
						{
							if ((!got_best) || (viv_compare(&d->fd,&best_fd) > 0))
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
							if ((!got_best) || (viv_compare(&d->fd,&best_fd) < 0))
							{
								os_copy_memory(&best_fd,&d->fd,sizeof(WIN32_FIND_DATA));
								got_best = 1;
							}
						}		
					}
				}
				
				// compare with start..
				if (prev)
				{
					if ((!got_start) || (viv_compare(&d->fd,&start_fd) > 0))
					{
						os_copy_memory(&start_fd,&d->fd,sizeof(WIN32_FIND_DATA));
						
						got_start = 1;
					}
				}
				else
				{
					if ((!got_start) || (viv_compare(&d->fd,&start_fd) < 0))
					{
						os_copy_memory(&start_fd,&d->fd,sizeof(WIN32_FIND_DATA));
						got_start = 1;
					}
				}					
			
				d = d->next;
			}
		}
		else
		{
			WIN32_FIND_DATA fd;
			HANDLE h;
			wchar_t search_wbuf[STRING_SIZE];
			wchar_t path_wbuf[STRING_SIZE];
			
			string_get_path_part(path_wbuf,viv_current_fd->cFileName);
			
			string_copy(search_wbuf,path_wbuf);
			string_cat_utf8(search_wbuf,(const utf8_t *)"\\*.*");
			
			h = FindFirstFile(search_wbuf,&fd);
			if (h != INVALID_HANDLE_VALUE)
			{
				for(;;)
				{
					if (viv_is_valid_filename(fd.cFileName))
					{
						int compare_ret;
						
						compare_ret = viv_compare(&fd,viv_current_fd);

						if (compare_ret != 0)
						{
							if (prev)
							{
								if (compare_ret < 0)
								{
									if ((!got_best) || (viv_compare(&fd,&best_fd) > 0))
									{
										string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
										string_copy(fd.cFileName,search_wbuf);
										os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
										got_best = 1;
									}
								}
							}
							else
							{
								if (compare_ret > 0)
								{
									if ((!got_best) || (viv_compare(&fd,&best_fd) < 0))
									{
										string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
										string_copy(fd.cFileName,search_wbuf);
										os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
										got_best = 1;
									}
								}		
							}
						}
						
						// compare with start..
						if (prev)
						{
							if ((!got_start) || (viv_compare(&fd,&start_fd) > 0))
							{
								string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
								string_copy(fd.cFileName,search_wbuf);
								os_copy_memory(&start_fd,&fd,sizeof(WIN32_FIND_DATA));
								
								got_start = 1;
							}
						}
						else
						{
							if ((!got_start) || (viv_compare(&fd,&start_fd) < 0))
							{
								string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
								string_copy(fd.cFileName,search_wbuf);
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

		if (got_best)
		{
			viv_open(&best_fd);
		}
		else
		if (got_start)
		{
			// don't open the same image again.
			debug_printf("%S %S\n",start_fd.cFileName,viv_current_fd->cFileName);
			if (string_compare(start_fd.cFileName,viv_current_fd->cFileName) != 0)
			{
				viv_open(&start_fd);
			}
		}
		else
		{
			viv_blank();
		}
	}
	else
	{
		viv_home(0);
	}

	// reset slideshow timer.
	if (reset_slideshow_timer)
	{
		if (viv_is_slideshow)
		{
			KillTimer(viv_hwnd,VIV_ID_SLIDESHOW_TIMER);
			SetTimer(viv_hwnd,VIV_ID_SLIDESHOW_TIMER,viv_slideshow_rate,0);
		}
	}
}

static void viv_home(int end)
{
	WIN32_FIND_DATA best_fd;
	int got_best;
	
	got_best = 0;
	
	if (viv_dragdrop_start)
	{
		viv_dragdrop_t *d;
		
		d = viv_dragdrop_start;
		while(d)
		{
//debug_printf("%S\n",d->filename);

			if (end)
			{
				if ((!got_best) || (viv_compare(&d->fd,&best_fd) > 0))
				{
					os_copy_memory(&best_fd,&d->fd,sizeof(WIN32_FIND_DATA));
					got_best = 1;
				}
			}
			else
			{
				if ((!got_best) || (viv_compare(&d->fd,&best_fd) < 0))
				{
					os_copy_memory(&best_fd,&d->fd,sizeof(WIN32_FIND_DATA));
					got_best = 1;
				}		
			}					
		
			d = d->next;
		}
	}
	else
	{
		HANDLE h;
		WIN32_FIND_DATA fd;
		wchar_t search_wbuf[STRING_SIZE];
		wchar_t path_wbuf[STRING_SIZE];
	
		if (*viv_current_fd->cFileName)
		{
			string_get_path_part(path_wbuf,viv_current_fd->cFileName);
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
				if (viv_is_valid_filename(fd.cFileName))
				{
					if (end)
					{
						if ((!got_best) || (viv_compare(&fd,&best_fd) > 0))
						{
							string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
							string_copy(fd.cFileName,search_wbuf);
							os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
							got_best = 1;
						}
					}
					else
					{
						if ((!got_best) || (viv_compare(&fd,&best_fd) < 0))
						{
							string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
							string_copy(fd.cFileName,search_wbuf);
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
		viv_open(&best_fd);
	}
	else
	{
		viv_blank();
	}

	// reset slideshow timer.
	if (viv_is_slideshow)
	{
		KillTimer(viv_hwnd,VIV_ID_SLIDESHOW_TIMER);
		SetTimer(viv_hwnd,VIV_ID_SLIDESHOW_TIMER,viv_slideshow_rate,0);
	}
}

static int viv_is_valid_filename(wchar_t *s)
{
	wchar_t *e;
	
	e = 0;
	while(*s)
	{
		if (*s == '.')
		{
			e = s + 1;
		}
		
		s++;
	}

	if (e)
	{
		int exti;
		
		for(exti=0;exti<VIV_ASSOCIATION_COUNT;exti++)
		{
			if (string_icompare_lowercase_ascii(e,viv_association_extensions[exti]) == 0) 
			{
				return 1;
			}
		}
	}
	
	return 0;
}

static int viv_is_msg(MSG *msg)
{
	switch (msg->message)
	{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			int key_flags;
		
			key_flags = viv_get_current_key_mod_flags();
			
			if (msg->hwnd == viv_hwnd)
			{
				int key_index;
				
				// find the key.
				for(key_index=0;key_index<VIV_COMMAND_COUNT;key_index++)
				{
					viv_key_t *k;
					
					k = viv_key_list->start[key_index];
					while(k)
					{
						if ((k->key & VIV_KEYFLAG_MOD_MASK) == key_flags)
						{
							if ((k->key & VIV_KEYFLAG_VK_MASK) == msg->wParam)
							{
								viv_command(viv_commands[key_index].command_id);
								
								return 1;
							}
						}
						
						k = k->next;
					}
				}
				
				break;
			}
		}
	}

	return 0;
}

static void viv_view_set(int view_x,int view_y,int invalidate)
{
	RECT rect;
	int wide;
	int high;
	int rx;
	int ry;
	int rw;
	int rh;
	
	GetClientRect(viv_hwnd,&rect);
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top - viv_get_status_high() - viv_get_controls_high();

	viv_get_render_size(&rw,&rh);
/*		
		if (viv_zoom_pos == 1)
		{
			if ((rw < viv_image_wide) || (rw < viv_image_wide))
			{
				rw = viv_image_wide;
				rh = viv_image_high;
			}
		}
*/
		
	rx = (wide / 2) - (rw / 2) - view_x;
	ry = (high / 2) - (rh / 2) - view_y;

	if (viv_keep_centered)
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
	
	if ((viv_view_x != view_x) || (viv_view_y != view_y))
	{
		viv_view_x = view_x;
		viv_view_y = view_y;
		viv_view_ix = (double)view_x / (double)rw;
		viv_view_iy = (double)view_y / (double)rh;

		if (invalidate)
		{
			InvalidateRect(viv_hwnd,0,FALSE);
		}
	}

	viv_toolbar_update_buttons();
}

static void viv_toggle_fullscreen(void)
{
	DWORD style;
	
	style = GetWindowLong(viv_hwnd,GWL_STYLE);
	
	viv_1to1 = 0;
	
	if (viv_is_fullscreen)
	{
		if (viv_is_show_caption)	
		{
			style |= WS_CAPTION | WS_SYSMENU;
		}
		else
		{
			style &= ~(WS_CAPTION | WS_SYSMENU);
		}
		
		if (viv_is_show_thickframe)	
		{
			style |= WS_THICKFRAME;
		}
		else
		{
			style &= ~WS_THICKFRAME;
		}

		if (viv_is_show_menu)	
		{
			SetMenu(viv_hwnd,viv_hmenu);
		}
		else
		{
			SetMenu(viv_hwnd,0);
		}
		
		viv_status_show(viv_is_show_status);
		viv_controls_show(viv_is_show_controls);
		
		SetWindowLong(viv_hwnd,GWL_STYLE,style);

		SetWindowPos(viv_hwnd,HWND_TOP,viv_fullscreen_rect.left,viv_fullscreen_rect.top,viv_fullscreen_rect.right - viv_fullscreen_rect.left,viv_fullscreen_rect.bottom - viv_fullscreen_rect.top,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOCOPYBITS);
	
		if (viv_fullscreen_is_maxed)
		{	
			ShowWindow(viv_hwnd,SW_MAXIMIZE);
		}
		
		KillTimer(viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER);
		viv_is_hide_cursor_timer = 0;
		
		if (!viv_cursor_shown)
		{
			ShowCursor(TRUE);
			
			viv_cursor_shown = 1;
		}
		
		viv_is_fullscreen = 0;
	}
	else
	{
		HMONITOR hmonitor;
		MONITORINFO mi;
		
		// set fullscreen before we resize.
		viv_is_fullscreen = 1;
		
		hmonitor = MonitorFromWindow(viv_hwnd,MONITOR_DEFAULTTOPRIMARY);
		
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hmonitor,&mi);
		
		// clear zooming if the current render size is smaller than the full screen size.
		/*
		if (viv_zoom_pos)
		{
			int rw;
			int rh;
			
			viv_get_render_size(&rw,&rh);
			
			if ((rw <= mi.rcMonitor.right - mi.rcMonitor.left) &&  (rh <= mi.rcMonitor.bottom - mi.rcMonitor.top))
			{
				viv_zoom_pos = 0;
			}
		}*/
		
		SetWindowLong(viv_hwnd,GWL_STYLE,style & ~(WS_CAPTION|WS_THICKFRAME|WS_SYSMENU));
		
		viv_fullscreen_is_maxed = IsZoomed(viv_hwnd);
		if (viv_fullscreen_is_maxed)
		{
			ShowWindow(viv_hwnd,SW_SHOWNORMAL);
		}
		
		GetWindowRect(viv_hwnd,&viv_fullscreen_rect);
		
		SetMenu(viv_hwnd,0);
		viv_status_show(0);
		viv_controls_show(0);

		SetWindowPos(viv_hwnd,HWND_TOP,mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right - mi.rcMonitor.left,mi.rcMonitor.bottom - mi.rcMonitor.top,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOCOPYBITS);

		// create a dummy fullscreen window and destroy it immediately
		// this window is created fullscreen correctly, and when our normal window gains focus it will keep the fullscreen setting (no taskbar)
		// without this dummy window, sometimes the taskbar will not disappear.
		{
			HWND fullscreen_hwnd;
			
			// Initialize global strings
			os_RegisterClassEx(
				0,
				viv_fullscreen_proc,
				0,
				LoadCursor(NULL,IDC_ARROW),
				(HBRUSH)(COLOR_BTNFACE+1),
				"VIV_FULLSCREEN",
				0);

			fullscreen_hwnd = os_CreateWindowEx(
				0,
				"VIV_FULLSCREEN",
				"void Image Viewer",
				WS_POPUP | WS_VISIBLE,
				mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right - mi.rcMonitor.left,mi.rcMonitor.bottom - mi.rcMonitor.top,
				0,0,os_hinstance,NULL);
				
			SetForegroundWindow(fullscreen_hwnd);

			DestroyWindow(fullscreen_hwnd);
		}		

		SetTimer(viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER,VIV_HIDE_CURSOR_DELAY,0);
		viv_is_hide_cursor_timer = 1;
		
		{
			POINT pt;
			
			GetCursorPos(&pt);
			
			viv_hide_cursor_x = pt.x;
			viv_hide_cursor_y = pt.y;
		}
	}
	
//	viv_on_size();
//	InvalidateRect(viv_hwnd,0,FALSE);
}

static void viv_slideshow(void)
{
	if (!viv_is_fullscreen)
	{
		viv_toggle_fullscreen();
	}
	
	if (!viv_is_slideshow)
	{
		SetTimer(viv_hwnd,VIV_ID_SLIDESHOW_TIMER,viv_slideshow_rate,0);
		
		viv_is_slideshow = 1;
		viv_status_update();
		viv_toolbar_update_buttons();
		viv_update_ontop();
	}
}

static void viv_get_render_size(int *prw,int *prh)
{
	RECT rect;
	int wide;
	int high;
	int rw;
	int rh;
	int fill_window;
	int max_zoom_wide;
	int max_zoom_high;
	
	if (!((viv_image_wide) && (viv_image_high)))
	{
		*prw = 0;
		*prh = 0;
		
		return;
	}
	
	GetClientRect(viv_hwnd,&rect);
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top - viv_get_status_high() - viv_get_controls_high();
	
	if (!((wide) && (high)))
	{
		*prw = 0;
		*prh = 0;
		
		return;
	}
		
	if (viv_is_fullscreen)
	{
		fill_window = viv_fullscreen_fill_window;
	}
	else
	{
		fill_window = viv_fill_window;
	}
	
	if (viv_keep_aspect_ratio)
	{
		if ((high * viv_image_wide) / viv_image_high < wide)
		{
			// tall image.
			rh = high;
			rw = (high * viv_image_wide) / viv_image_high;
		}
		else
		{
			// long image.
			rw = wide;
			rh = (wide * viv_image_high) / viv_image_wide;
		}

		if (!fill_window)
		{
			if ((rw > viv_image_wide) || (rh > viv_image_high))
			{
				rw = viv_image_wide;
				rh = viv_image_high;
			}
		}
	}
	else
	{
		rw = wide;
		rh = high;

		if (!fill_window)
		{
			if (rw > viv_image_wide)
			{
				rw = viv_image_wide;
			}			
	
			if (rh > viv_image_high)
			{
				rh = viv_image_high;
			}
		}
	}

	if (!viv_allow_shrinking)
	{
		if (viv_keep_aspect_ratio)
		{
			if ((rw < viv_image_wide) || (rh < viv_image_high))
			{
				rw = viv_image_wide;
				rh = viv_image_high;
			}
		}
		else
		{
			if (rw < viv_image_wide)
			{
				rw = viv_image_wide;
			}

			if (rh < viv_image_high)
			{
				rh = viv_image_high;
			}
		}
	}		
	
	max_zoom_wide = rw * 20;
	max_zoom_high = rh * 20;
	
	if (max_zoom_wide < viv_image_wide)
	{
		max_zoom_wide = viv_image_wide;
	}
	
	if (max_zoom_high < viv_image_high)
	{
		max_zoom_high = viv_image_high;
	}
	
	if (viv_zoom_pos)
	{
		rw = rw + (max_zoom_wide - rw) * viv_zoom_presets[viv_zoom_pos];
		rh = rh + (max_zoom_high - rh) * viv_zoom_presets[viv_zoom_pos];
	}
	
	if (viv_1to1)
	{
		rw = viv_image_wide;
		rh = viv_image_high;
	}
	
	*prw = rw;
	*prh = rh;
}

static void viv_set_custom_rate(void)
{
	if (DialogBox(os_hinstance,MAKEINTRESOURCE(IDD_CUSTOM_RATE),viv_hwnd,viv_custom_rate_proc))
	{
		switch(viv_slideshow_custom_rate_type)
		{
			case 0:
				viv_slideshow_rate = viv_slideshow_custom_rate;
				break;
		
			case 1: 
				viv_slideshow_rate = viv_slideshow_custom_rate * 1000;
				break;
				
			case 2: 
				viv_slideshow_rate = viv_slideshow_custom_rate * 1000 * 60;
				break;
		}
		
		if (viv_slideshow_rate < 1)
		{
			viv_slideshow_rate = 1;
		}
		
		if (viv_is_slideshow)
		{
			KillTimer(viv_hwnd,VIV_ID_SLIDESHOW_TIMER);

			SetTimer(viv_hwnd,VIV_ID_SLIDESHOW_TIMER,viv_slideshow_rate,0);
		}			

		viv_status_update_slideshow_rate();
	}
}

static void viv_set_rate(int rate)
{
	viv_slideshow_rate = rate;
	
	if (viv_is_slideshow)
	{
		KillTimer(viv_hwnd,VIV_ID_SLIDESHOW_TIMER);

		SetTimer(viv_hwnd,VIV_ID_SLIDESHOW_TIMER,viv_slideshow_rate,0);
	}			

	viv_status_update_slideshow_rate();
}

static void viv_check_menus(HMENU hmenu)
{
	int is_slideshow;
	int fill_window;
	int rw;
	int rh;
	int slideshow_rate_id;
	UINT is_image_enabled;
	
	is_slideshow = 0;

	if (viv_is_fullscreen)
	{
		if (viv_is_slideshow)
		{	
			is_slideshow = 1;
			viv_status_update();
			viv_toolbar_update_buttons();
			viv_update_ontop();
		}
		
		fill_window = viv_fullscreen_fill_window;
	}
	else
	{
		fill_window = viv_fill_window;
	}
				
	viv_get_render_size(&rw,&rh);
	
	is_image_enabled = *viv_current_fd->cFileName ? MF_ENABLED : MF_DISABLED;

	EnableMenuItem(hmenu,VIV_ID_FILE_CLOSE,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_EDIT_COPY,is_image_enabled);
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

	CheckMenuItem(hmenu,VIV_ID_VIEW_MENU,viv_is_show_menu ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_CONTROLS,viv_is_show_controls ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_STATUS,viv_is_show_status ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_ALLOW_SHRINKING,viv_allow_shrinking ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_KEEP_ASPECT_RATIO,viv_keep_aspect_ratio ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_FILL_WINDOW,fill_window ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_1TO1,((rw == viv_image_wide) && (rh == viv_image_high)) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_FULLSCREEN,viv_is_fullscreen ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_SLIDESHOW,is_slideshow ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_ONTOP_ALWAYS,viv_ontop == 1 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_VIEW_ONTOP_WHILE_PLAYING_OR_ANIMATING,viv_ontop == 2 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_VIEW_ONTOP_NEVER,viv_ontop == 0 ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	
	CheckMenuItem(hmenu,VIV_ID_SLIDESHOW_PAUSE,viv_is_slideshow ? MF_CHECKED : MF_UNCHECKED);

	switch(viv_slideshow_rate)
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

	CheckMenuItem(hmenu,VIV_ID_ANIMATION_PLAY_PAUSE,viv_animation_play ? MF_CHECKED : MF_UNCHECKED);
	
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_NAME,viv_nav_sort == VIV_NAV_SORT_NAME ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_FULL_PATH_AND_FILENAME,viv_nav_sort == VIV_NAV_SORT_FULL_PATH_AND_FILENAME ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_SIZE,viv_nav_sort == VIV_NAV_SORT_SIZE ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_DATE_MODIFIED,viv_nav_sort == VIV_NAV_SORT_DATE_MODIFIED ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_DATE_CREATED,viv_nav_sort == VIV_NAV_SORT_DATE_CREATED ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));

	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_ASCENDING,viv_nav_sort_ascending ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	CheckMenuItem(hmenu,VIV_ID_NAV_SORT_DESCENDING,(!viv_nav_sort_ascending) ? (MF_CHECKED|MFT_RADIOCHECK) : (MF_UNCHECKED|MFT_RADIOCHECK));
	
	CheckMenuItem(hmenu,VIV_ID_VIEW_1TO1,((rw == viv_image_wide) && (rh == viv_image_high)) ? MF_CHECKED : MF_UNCHECKED);
}

static void viv_delete(void)
{
	if (*viv_current_fd->cFileName)
	{
		SHFILEOPSTRUCT fo;
		wchar_t filename_list[STRING_SIZE+1];
		wchar_t *d;
		wchar_t *s;
		
		d = filename_list;
		s = viv_current_fd->cFileName;
		
		while(*s)
		{
			*d++ = *s++;
		}

		*d++ = 0;
		*d = 0;
		
		ZeroMemory(&fo,sizeof(SHFILEOPSTRUCT));
		fo.hwnd = viv_hwnd;
		fo.wFunc = FO_DELETE;
		fo.pFrom = filename_list;
		fo.fFlags = (GetKeyState(VK_SHIFT) < 0) ? 0 : FOF_ALLOWUNDO;

		if (SHFileOperation(&fo) == 0)
		{
			if (!fo.fAnyOperationsAborted)
			{
				viv_dragdrop_delete(viv_current_fd->cFileName);
			
				viv_next(0,1);
			}
		}
	}
}

static void viv_copy(int cut)
{
	if (*viv_current_fd->cFileName)
	{
		if (OpenClipboard(viv_hwnd))
		{
			EmptyClipboard();
	
			if (viv_frame_count)
			{
				if (viv_frames[viv_frame_position].hbitmap)
				{
					HDC screen_hdc;
					
					screen_hdc = GetDC(0);
					if (screen_hdc)
					{
						HDC mem1_hdc;
						
						mem1_hdc = CreateCompatibleDC(screen_hdc);
						if (mem1_hdc)
						{
							HDC mem2_hdc;
							
							mem2_hdc = CreateCompatibleDC(screen_hdc);
							if (mem2_hdc)
							{
								HBITMAP mem1_hbitmap;

								mem1_hbitmap = CreateCompatibleBitmap(screen_hdc,viv_image_wide,viv_image_high);
								if (mem1_hbitmap)
								{
									HGDIOBJ last_mem1_hbitmap;
									HGDIOBJ last_mem2_hbitmap;
									
									last_mem1_hbitmap = SelectObject(mem1_hdc,mem1_hbitmap);
									last_mem2_hbitmap = SelectObject(mem2_hdc,viv_frames[viv_frame_position].hbitmap);
									
									BitBlt(mem1_hdc,0,0,viv_image_wide,viv_image_high,mem2_hdc,0,0,SRCCOPY);

									SelectObject(mem2_hdc,last_mem2_hbitmap);
									SelectObject(mem1_hdc,last_mem1_hbitmap);
									
									SetClipboardData(CF_BITMAP,mem1_hbitmap);
								}

								DeleteDC(mem2_hdc);
							}

							DeleteDC(mem1_hdc);
						}
						
						ReleaseDC(0,screen_hdc);
					}
				}
			}

			{
				HGLOBAL hmem;
				int wlen;
				
				wlen = string_length(viv_current_fd->cFileName);
				
				hmem = GlobalAlloc(GMEM_MOVEABLE,(wlen + 1 + 1) * sizeof(wchar_t) + sizeof(DROPFILES));
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
						
						os_copy_memory(df+1,viv_current_fd->cFileName,wlen * sizeof(wchar_t));
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

static int viv_is_key_state(int control,int shift,int alt)
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

static CLIPFORMAT _viv_get_CF_PREFERREDDROPEFFECT(void)
{
	if (!_viv_CF_PREFERREDDROPEFFECT)
	{
		_viv_CF_PREFERREDDROPEFFECT = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
	}
	
	return _viv_CF_PREFERREDDROPEFFECT;
}

static void viv_pause(void)
{
	if (viv_is_slideshow)
	{
		KillTimer(viv_hwnd,VIV_ID_SLIDESHOW_TIMER);
		
		viv_is_slideshow = 0;
		viv_status_update();
		viv_toolbar_update_buttons();
		viv_update_ontop();
	}
	else
	{
		SetTimer(viv_hwnd,VIV_ID_SLIDESHOW_TIMER,viv_slideshow_rate,0);
		
		viv_is_slideshow = 1;
		viv_status_update();
		viv_toolbar_update_buttons();
		viv_update_ontop();
	}
}

static void viv_increase_rate(int dec)
{
	if (dec)	
	{
		int i;
		
		for(i=0;i<VIV_PREDEFINED_RATE_COUNT;i++)
		{
			if (viv_slideshow_rate < viv_predefined_rates[i])
			{
				viv_set_rate(viv_predefined_rates[i]);
				
				break;
			}
		}
	}
	else
	{
		int i;
		
		for(i=0;i<VIV_PREDEFINED_RATE_COUNT;i++)
		{
			if (viv_slideshow_rate > viv_predefined_rates[VIV_PREDEFINED_RATE_COUNT-i-1])
			{
				viv_set_rate(viv_predefined_rates[VIV_PREDEFINED_RATE_COUNT-i-1]);
				
				break;
			}
		}
	}

	viv_status_update_slideshow_rate();
}

static void viv_increase_animation_rate(int dec)
{
	if (dec)	
	{
		if (viv_animation_rate_pos)
		{
			viv_animation_rate_pos--;
		}
	}
	else
	{
		if (viv_animation_rate_pos < VIV_ANIMATION_RATE_MAX-1)
		{
			viv_animation_rate_pos++;
		}
	}
	
	viv_status_update_temp_animation_rate();
}

static void viv_reset_animation_rate(void)
{
	viv_animation_rate_pos = VIV_ANIMATION_RATE_ONE;

	viv_status_update_temp_animation_rate();
}

static void viv_file_preview(void)
{
	if (*viv_current_fd->cFileName)
	{
		os_shell_execute(viv_hwnd,viv_current_fd->cFileName,0,"preview",0);
	}
}

static void viv_file_print(void)
{
	if (*viv_current_fd->cFileName)
	{
		os_shell_execute(viv_hwnd,viv_current_fd->cFileName,0,"print",0);
	}
}

static void viv_file_set_desktop_wallpaper(void)
{
	if (*viv_current_fd->cFileName)
	{
		if (!_viv_stobject_hmodule)
		{
			_viv_stobject_hmodule = LoadLibraryA("stobject.dll");
		}
		
		if (_viv_stobject_hmodule)
		{
			os_shell_execute(viv_hwnd,viv_current_fd->cFileName,0,"setdesktopwallpaper",0);
		}
	}
}

static void viv_file_edit(void)
{
	if (*viv_current_fd->cFileName)
	{
		os_shell_execute(viv_hwnd,viv_current_fd->cFileName,0,"edit",0);
	}
}

static void viv_open_file_location(void)
{
	if (*viv_current_fd->cFileName)
	{
		int openpathok;
		
		openpathok = 0;
		
		if (os_SHOpenFolderAndSelectItems)
		{
			wchar_t path_part[STRING_SIZE];
			ITEMIDLIST *folder_idlist;
			
			string_get_path_part(path_part,viv_current_fd->cFileName);
		
			// if path_part_buf.buf is an empty string, os_ILCreateFromPath will
			// correctly return the desktop pidl (an empty pidl).
			folder_idlist = os_ILCreateFromPath(path_part);

		debug_printf("folder_idlist %p\n",folder_idlist);
			if (folder_idlist)
			{
				ITEMIDLIST *idlist;
				
				idlist = os_ILCreateFromPath(viv_current_fd->cFileName);
		debug_printf("idlist %S %p\n",viv_current_fd->cFileName,idlist);
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
			
			string_get_path_part(path,viv_current_fd->cFileName);
			
			os_shell_execute(viv_hwnd,path,0,NULL,NULL);
		}
	}
}


static void viv_properties(void)
{
	if (*viv_current_fd->cFileName)
	{
		os_shell_execute(viv_hwnd,viv_current_fd->cFileName,0,"properties",0);
	}
}

static void viv_doing_cancel(void)
{
	if (viv_doing)
	{
		if (viv_doing == 2)
		{
			ShowCursor(TRUE);
		}
		
		ReleaseCapture();
		
		viv_doing = 0;
	}
}

static const char *_viv_get_copydata_string(const char *p,const char *e,wchar_t *buf,int bufsize)
{
	wchar_t *d;
	
	d = buf;
	
	while(p + sizeof(wchar_t) <= e)
	{
		if (!*(wchar_t *)p)
		{
			p += sizeof(wchar_t);
			break;
		}
		
		if (bufsize > 1)
		{
			*d++ = *(wchar_t *)p;
			bufsize--;
		}
		
		p+=sizeof(wchar_t);
	}
	
	if (bufsize > 0)
	{
		*d = 0;
	}
	
	return p;
}


static void os_remove_all_menu_items(HMENU hmenu)
{	
	// remove all items
	// we loop until all the items are removed
	// we remove the first item in the menu each time we delete an item
	// use deletemenu to recursively destroy submenus.
	// removemenu does not destroy sub menus.
	for(;;)
	{
		if (!DeleteMenu(hmenu,0,MF_BYPOSITION)) 
		{
			break;
		}
	}
}

static void viv_nav_item_free_all(void)
{
	if (viv_nav_items)
	{
		int i;
		
		for(i=0;i<viv_nav_item_count;i++)
		{
			mem_free(viv_nav_items[i]);
		}
		
		mem_free(viv_nav_items);
		viv_nav_items = 0;
		viv_nav_item_count = 0;
	}
}

static int viv_nav_playlist_compare(const viv_nav_item_t *a,const viv_nav_item_t *b)
{
	return viv_compare(&a->fd,&b->fd);
}

static void viv_blank(void)
{
	viv_clear();

	viv_current_fd->cFileName[0] = 0;

	viv_status_update();
	viv_update_title();

	// free all dropfiles
	viv_dragdrop_clearall();

	InvalidateRect(viv_hwnd,0,FALSE);
}

static INT_PTR CALLBACK viv_options_general_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			int exti;

			if (viv_appdata) 
			{
				CheckDlgButton(hwnd,IDC_APPDATA,BST_CHECKED);
			}
			
			if (viv_multiple_instances) 
			{
				CheckDlgButton(hwnd,IDC_MULTIPLE_INSTANCES,BST_CHECKED);
			}
			
			for(exti=0;exti<VIV_ASSOCIATION_COUNT;exti++)
			{
				if (viv_is_association(viv_association_extensions[exti])) 
				{
					CheckDlgButton(hwnd,viv_association_dlg_item_id[exti],BST_CHECKED);
				}
			}
			
			if (viv_is_start_menu_shortcuts()) 
			{
				CheckDlgButton(hwnd,IDC_STARTMENU,BST_CHECKED);
			}
			
			return FALSE;
		}
			
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDC_STARTMENU:
				case IDC_APPDATA:
				case IDC_BMP:
				case IDC_GIF:
				case IDC_ICO:
				case IDC_JPEG:
				case IDC_JPG:
				case IDC_PNG:
				case IDC_TIF:
				case IDC_TIFF:
					viv_options_update_sheild(GetParent(hwnd));
					break;
					
				case IDC_CHECKALL:
				case IDC_CHECKNONE:
				{
					UINT check;
					
					check = LOWORD(wParam) == IDC_CHECKALL ? BST_CHECKED : BST_UNCHECKED;
					
					CheckDlgButton(hwnd,IDC_BMP,check);
					CheckDlgButton(hwnd,IDC_GIF,check);
					CheckDlgButton(hwnd,IDC_ICO,check);
					CheckDlgButton(hwnd,IDC_JPEG,check);
					CheckDlgButton(hwnd,IDC_JPG,check);
					CheckDlgButton(hwnd,IDC_PNG,check);
					CheckDlgButton(hwnd,IDC_TIF,check);
					CheckDlgButton(hwnd,IDC_TIFF,check);
					
					viv_options_update_sheild(GetParent(hwnd));

					break;
				}
			}
					
			break;

	}
	
	return FALSE;
}

static void viv_options_key_list_sel_change(HWND hwnd,int previous_key_index)
{
	int index;
	viv_key_list_t *keylist;
	int key_count;

	keylist = (viv_key_list_t *)GetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA);
	
	index = ListBox_GetCurSel(GetDlgItem(hwnd,IDC_COMMANDS_LIST));
	
	ListBox_ResetContent(GetDlgItem(hwnd,IDC_KEYS_LIST));
	SetWindowRedraw(GetDlgItem(hwnd,IDC_KEYS_LIST),FALSE);
	
	key_count = 0;
	
	if (index != LB_ERR)
	{
		int command_index;
		viv_key_t *key;
		
		command_index = ListBox_GetItemData(GetDlgItem(hwnd,IDC_COMMANDS_LIST),index);
		
		key = keylist->start[command_index];
		
		while(key)
		{
			wchar_t key_text[STRING_SIZE];
			int listbox_index;
			
			viv_get_key_text(key_text,key->key);
			
			listbox_index = ListBox_AddString(GetDlgItem(hwnd,IDC_KEYS_LIST),key_text);
			
			if (listbox_index != LB_ERR)
			{
				ListBox_SetItemData(GetDlgItem(hwnd,IDC_KEYS_LIST),listbox_index,key->key);
			}

			key_count++;
			key = key->next;
		}
		
		EnableWindow(GetDlgItem(hwnd,IDC_ADD_KEY),TRUE);
		
		if (key_count)
		{
			EnableWindow(GetDlgItem(hwnd,IDC_EDIT_KEY),TRUE);
			EnableWindow(GetDlgItem(hwnd,IDC_REMOVE_KEY),TRUE);
			
			if ((previous_key_index == LB_ERR) || (previous_key_index > key_count - 1))
			{
				previous_key_index = key_count - 1;
			}
			
			ListBox_SetCurSel(GetDlgItem(hwnd,IDC_KEYS_LIST),previous_key_index);
		}
		else
		{
			EnableWindow(GetDlgItem(hwnd,IDC_EDIT_KEY),FALSE);
			EnableWindow(GetDlgItem(hwnd,IDC_REMOVE_KEY),FALSE);
		}
	}
	else
	{
		EnableWindow(GetDlgItem(hwnd,IDC_ADD_KEY),FALSE);
		EnableWindow(GetDlgItem(hwnd,IDC_EDIT_KEY),FALSE);
		EnableWindow(GetDlgItem(hwnd,IDC_REMOVE_KEY),FALSE);
	}

	SetWindowRedraw(GetDlgItem(hwnd,IDC_KEYS_LIST),TRUE);
}

static void viv_options_remove_key(HWND hwnd)
{
	int index;
	viv_key_list_t *keylist;

	keylist = (viv_key_list_t *)GetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA);
	
	index = ListBox_GetCurSel(GetDlgItem(hwnd,IDC_COMMANDS_LIST));
	
	if (index != LB_ERR)
	{
		int key_index;
		
		key_index = ListBox_GetCurSel(GetDlgItem(hwnd,IDC_KEYS_LIST));
		
		if (key_index != LB_ERR)
		{
			int command_index;
			WORD keyflags;
			
			command_index = ListBox_GetItemData(GetDlgItem(hwnd,IDC_COMMANDS_LIST),index);
			keyflags = ListBox_GetItemData(GetDlgItem(hwnd,IDC_KEYS_LIST),key_index);
			
			viv_key_remove(keylist,command_index,keyflags);
			
			viv_options_key_list_sel_change(hwnd,key_index);
		}
	}
}

static INT_PTR CALLBACK viv_edit_key_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			WNDPROC last_proc;
			wchar_t caption_wbuf[STRING_SIZE];
			
			os_center_dialog(hwnd);

			last_proc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwnd,IDC_EDIT_KEY_EDIT),GWLP_WNDPROC,(LONG_PTR)_viv_edit_key_edit_proc);
			SetWindowLongPtr(GetDlgItem(hwnd,IDC_EDIT_KEY_EDIT),GWLP_USERDATA,(LONG_PTR)last_proc);
			
			viv_edit_key_set_key(GetDlgItem(hwnd,IDC_EDIT_KEY_EDIT),lParam);
			viv_options_edit_key_changed(hwnd);
			
			string_printf(caption_wbuf,lParam ? "Edit Keyboard Shortcut" : "Add Keyboard Shortcut");
			SetWindowText(hwnd,caption_wbuf);
			
			return FALSE;
		}
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hwnd,GetWindowLongPtr(hwnd,GWLP_USERDATA));
					break;
					
				case IDCANCEL:
					EndDialog(hwnd,0);
					break;
			}

			break;
	}
	
	return FALSE;
}

static void viv_options_edit_key(HWND hwnd,int key_index)
{
	int index;
	viv_key_list_t *keylist;

	keylist = (viv_key_list_t *)GetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA);
	
	index = ListBox_GetCurSel(GetDlgItem(hwnd,IDC_COMMANDS_LIST));
	
	if (index != LB_ERR)
	{
		WORD keyflags;
		
		if (key_index == LB_ERR)
		{
			keyflags = 0;
		}
		else
		{
			keyflags = ListBox_GetItemData(GetDlgItem(hwnd,IDC_KEYS_LIST),key_index);
		}
		
		keyflags = DialogBoxParam(os_hinstance,MAKEINTRESOURCE(IDD_EDIT_KEY),hwnd,viv_edit_key_proc,keyflags);

		viv_edit_key_remove_currently_used_by(keylist,keyflags);

		if (keyflags)
		{
			int command_index;

			command_index = ListBox_GetItemData(GetDlgItem(hwnd,IDC_COMMANDS_LIST),index);
			
			if (key_index == LB_ERR)
			{
				// new
				viv_key_add(keylist,command_index,keyflags);
			}
			else
			{
				viv_key_t *key;
				int keyi;
				
				// edit
				key = keylist->start[command_index];
				keyi = 0;
				
				while(key)
				{
					if (keyi == key_index)
					{
						key->key = keyflags;
						break;
					}
				
					keyi++;
					key = key->next;
				}
			}
			
			viv_options_key_list_sel_change(hwnd,key_index);			
		}
	}
}

static INT_PTR CALLBACK viv_options_controls_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			os_ComboBox_AddString(hwnd,IDC_LEFTCLICKACTION,(const utf8_t *)"Scroll");
			os_ComboBox_AddString(hwnd,IDC_LEFTCLICKACTION,(const utf8_t *)"Play/Pause Slideshow");
			os_ComboBox_AddString(hwnd,IDC_LEFTCLICKACTION,(const utf8_t *)"Play/Pause Animation");
			os_ComboBox_AddString(hwnd,IDC_LEFTCLICKACTION,(const utf8_t *)"Zoom In");
			os_ComboBox_AddString(hwnd,IDC_LEFTCLICKACTION,(const utf8_t *)"Next Image");
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_LEFTCLICKACTION),viv_left_click_action);
			
			os_ComboBox_AddString(hwnd,IDC_RIGHTCLICKACTION,(const utf8_t *)"Context Menu");
			os_ComboBox_AddString(hwnd,IDC_RIGHTCLICKACTION,(const utf8_t *)"Zoom Out");
			os_ComboBox_AddString(hwnd,IDC_RIGHTCLICKACTION,(const utf8_t *)"Previous Image");
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_RIGHTCLICKACTION),viv_right_click_action);
			
			os_ComboBox_AddString(hwnd,IDC_MOUSEWHEELACTION,(const utf8_t *)"Zoom");
			os_ComboBox_AddString(hwnd,IDC_MOUSEWHEELACTION,(const utf8_t *)"Next/Prev");
			os_ComboBox_AddString(hwnd,IDC_MOUSEWHEELACTION,(const utf8_t *)"Prev/Next");
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_MOUSEWHEELACTION),viv_mouse_wheel_action);
			
			{
				int i;
				viv_key_list_t *keylist;
				
				keylist = mem_alloc(sizeof(viv_key_list_t));

				viv_key_list_init(keylist);
				
				viv_key_list_copy(keylist,viv_key_list);
				
				SetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA,(LONG_PTR)keylist);
				
				for(i=0;i<VIV_COMMAND_COUNT;i++)
				{
					if (!(viv_commands[i].flags & MF_POPUP))
					{
						if (!(viv_commands[i].flags & MF_SEPARATOR))
						{
							wchar_t command_name_wbuf[STRING_SIZE];
							int index;

							viv_get_command_name(command_name_wbuf,i);
							
							index = ListBox_AddString(GetDlgItem(hwnd,IDC_COMMANDS_LIST),command_name_wbuf);
							if (index != LB_ERR)
							{
								ListBox_SetItemData(GetDlgItem(hwnd,IDC_COMMANDS_LIST),index,i);
							}
						}
					}
				}
			}
			
			ListBox_SetCurSel(GetDlgItem(hwnd,IDC_COMMANDS_LIST),0);
			viv_options_key_list_sel_change(hwnd,0);
			
			return FALSE;
		}
		
		case WM_COMMAND:

			switch(LOWORD(wParam))
			{
				case IDC_COMMANDS_LIST:
					if (HIWORD(wParam) == LBN_SELCHANGE) 
					{	
						viv_options_key_list_sel_change(hwnd,0);
					}
					break;
					
				case IDC_REMOVE_KEY:
					viv_options_remove_key(hwnd);
					break;
					
				case IDC_EDIT_KEY:
					viv_options_edit_key(hwnd,ListBox_GetCurSel(GetDlgItem(hwnd,IDC_KEYS_LIST)));
					break;
					
				case IDC_ADD_KEY:
					viv_options_edit_key(hwnd,LB_ERR);
					break;
			}
			break;
		
		case WM_DESTROY:
		{
			viv_key_list_t *keylist;
			
			keylist = (viv_key_list_t *)GetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA);
			
			viv_key_list_clear(keylist);
			mem_free(keylist);
			
			break;
		}
	}
	
	return FALSE;
}
static INT_PTR CALLBACK viv_options_view_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			
			os_ComboBox_AddString(hwnd,IDC_COMBO1,(const utf8_t *)"COLORONCOLOR (Performance)");
			os_ComboBox_AddString(hwnd,IDC_COMBO1,(const utf8_t *)"HALFTONE (Quality)");
			
			if (viv_shrink_blit_mode == VIV_SHRINK_BLIT_MODE_HALFTONE)
			{
				ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_COMBO1),1);
			}
			else
			{
				ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_COMBO1),0);
			}

			os_ComboBox_AddString(hwnd,IDC_COMBO2,(const utf8_t *)"COLORONCOLOR (Performance)");
			os_ComboBox_AddString(hwnd,IDC_COMBO2,(const utf8_t *)"HALFTONE (Quality)");
			
			if (viv_mag_filter == VIV_MAG_FILTER_HALFTONE)
			{
				ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_COMBO2),1);
			}
			else
			{
				ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_COMBO2),0);
			}
					
			CheckDlgButton(hwnd,IDC_LOOP_ANIMATIONS_ONCE,viv_loop_animations_once ? BST_CHECKED : BST_UNCHECKED);
			
			SetWindowLongPtr(GetDlgItem(hwnd,IDC_WINDOWEDBACKGROUNDCOLOR),GWLP_USERDATA,RGB(viv_windowed_background_color_r,viv_windowed_background_color_g,viv_windowed_background_color_b));
			viv_update_color_button_bitmap(GetDlgItem(hwnd,IDC_WINDOWEDBACKGROUNDCOLOR));

			SetWindowLongPtr(GetDlgItem(hwnd,IDC_FULLSCREENBACKGROUNDCOLOR),GWLP_USERDATA,RGB(viv_fullscreen_background_color_r,viv_fullscreen_background_color_g,viv_fullscreen_background_color_b));
			viv_update_color_button_bitmap(GetDlgItem(hwnd,IDC_FULLSCREENBACKGROUNDCOLOR));
					
			return FALSE;
			
		case WM_DESTROY:	
			viv_delete_color_button_bitmap(GetDlgItem(hwnd,IDC_WINDOWEDBACKGROUNDCOLOR));
			viv_delete_color_button_bitmap(GetDlgItem(hwnd,IDC_FULLSCREENBACKGROUNDCOLOR));
			break;
		
		case WM_COMMAND:

			switch(LOWORD(wParam))
			{
				case IDC_WINDOWEDBACKGROUNDCOLOR:
				case IDC_FULLSCREENBACKGROUNDCOLOR:
				{
					COLORREF color;
					
					color = GetWindowLongPtr(GetDlgItem(hwnd,LOWORD(wParam)),GWLP_USERDATA);
					
					if (os_choose_color(hwnd,&color))
					{
						SetWindowLongPtr(GetDlgItem(hwnd,LOWORD(wParam)),GWLP_USERDATA,color);
						viv_update_color_button_bitmap(GetDlgItem(hwnd,LOWORD(wParam)));
					}
					
					break;
				}
			}
			
			break;
	}
	
	return FALSE;
}

static void viv_options_treeview_changed(HWND hwnd)
{
	TV_ITEM tvi;

	// get the current index
	tvi.hItem = TreeView_GetSelection(GetDlgItem(hwnd,IDC_TREE1));
	if (tvi.hItem)
	{
		int i;

		tvi.mask = TVIF_PARAM;
	    
		TreeView_GetItem(GetDlgItem(hwnd,IDC_TREE1),&tvi);
		
		ShowWindow(GetDlgItem(hwnd,viv_options_tab_ids[tvi.lParam]),SW_SHOW);
		ShowWindow(GetDlgItem(hwnd,viv_options_page_ids[tvi.lParam]),SW_SHOW);
		
		for(i=0;i<VIV_OPTIONS_PAGE_COUNT;i++)
		{
			if (i != tvi.lParam)
			{
				ShowWindow(GetDlgItem(hwnd,viv_options_tab_ids[i]),SW_HIDE);
				ShowWindow(GetDlgItem(hwnd,viv_options_page_ids[i]),SW_HIDE);
			}
		}

		viv_options_last_page = tvi.lParam;
	}
}

static void viv_options_update_sheild(HWND hwnd)
{
	int exti;
	int need_admin;
	HWND general_page;
	
	general_page = GetDlgItem(hwnd,VIV_ID_OPTIONS_GENERAL);
	
	need_admin = 0;
	
	for(exti=0;exti<VIV_ASSOCIATION_COUNT;exti++)
	{
		if (IsDlgButtonChecked(general_page,viv_association_dlg_item_id[exti]) == BST_CHECKED) 
		{
			if (!viv_is_association(viv_association_extensions[exti]))
			{
				need_admin = 1;
			}
		}
		else 
		{
			if (viv_is_association(viv_association_extensions[exti]))
			{
				need_admin = 1;
			}
		}
	}

	if ((!!viv_appdata) != (IsDlgButtonChecked(general_page,IDC_APPDATA) == BST_CHECKED)) 
	{
		need_admin = 1;
	}
	
	if ((IsDlgButtonChecked(general_page,IDC_STARTMENU) == BST_CHECKED) != !!viv_is_start_menu_shortcuts()) 
	{
		need_admin = 1;
	}
	
	if ((need_admin) && (!os_is_admin()))
	{
		SendMessage(GetDlgItem(hwnd,IDOK),BCM_SETSHIELD,0,TRUE);
	}
	else
	{
		SendMessage(GetDlgItem(hwnd,IDOK),BCM_SETSHIELD,0,FALSE);
	}
}

static INT_PTR CALLBACK viv_options_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_NOTIFY:

			switch(((NMHDR *)lParam)->idFrom)
			{
				case IDC_TREE1:

					switch(((NMHDR *)lParam)->code)
					{
						case TVN_SELCHANGEDA:
						case TVN_SELCHANGEDW:
						
							if (IsWindowVisible(hwnd))
							{
								viv_options_treeview_changed(hwnd);
							}
							
							break;
					}
					
					break;
			}
			
			return 0;
			
		case WM_INITDIALOG:
		{
			os_center_dialog(hwnd);

			{
				int i;
				
				for(i=0;i<VIV_OPTIONS_PAGE_COUNT;i++)
				{
					TV_INSERTSTRUCT tvitem;
					TCITEM tcitem;
					RECT rect;
					HWND page_hwnd;
					HTREEITEM hitem;
					wchar_t text_wbuf[STRING_SIZE];
					
					string_copy_utf8(text_wbuf,viv_options_page_names[i]);
					
					// treeview
					tvitem.hInsertAfter = TVI_LAST;
					tvitem.hParent = TVI_ROOT;
					tvitem.item.mask = TVIF_TEXT | TVIF_PARAM;
					tvitem.item.lParam = i;
					tvitem.item.pszText = text_wbuf;
					
					hitem = TreeView_InsertItem(GetDlgItem(hwnd,IDC_TREE1),&tvitem);

					if (i == viv_options_last_page) 
					{
						TreeView_Select(GetDlgItem(hwnd,IDC_TREE1),hitem,TVGN_CARET);
					}
					
					// tab
					string_copy_utf8(text_wbuf,viv_options_page_names[i]);
					
					tcitem.mask = TCIF_TEXT;
					tcitem.pszText = text_wbuf;
					TabCtrl_InsertItem(GetDlgItem(hwnd,viv_options_tab_ids[i]),0,&tcitem);

					// page
					GetClientRect(GetDlgItem(hwnd,IDC_PAGEPLACEHOLDER),&rect);
					MapWindowPoints(GetDlgItem(hwnd,IDC_PAGEPLACEHOLDER),hwnd,(LPPOINT)&rect,2);

					page_hwnd = CreateDialog(os_hinstance,MAKEINTRESOURCE(viv_options_dialog_ids[i]),hwnd,viv_options_page_procs[i]);
					SetWindowLong(page_hwnd,GWL_ID,viv_options_page_ids[i]);
					
					if (os_EnableThemeDialogTexture)
					{
						os_EnableThemeDialogTexture(page_hwnd,ETDT_ENABLETAB);
					}

					SetWindowPos(page_hwnd,HWND_TOP,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOSIZE|SWP_NOACTIVATE);
				}
			}

			viv_options_treeview_changed(hwnd);
			viv_options_update_sheild(hwnd);

			return TRUE;
		}
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
				
					if (LOWORD(wParam) == IDOK)
					{
						HWND general_page;
						HWND view_page;
						HWND controls_page;
						int old_shrink_blit_mode;
						int exti;
						COLORREF colorref;
						wchar_t params[STRING_SIZE];
						
						params[0] = 0;
						
						general_page = GetDlgItem(hwnd,VIV_ID_OPTIONS_GENERAL);
						view_page = GetDlgItem(hwnd,VIV_ID_OPTIONS_VIEW);
						controls_page = GetDlgItem(hwnd,VIV_ID_OPTIONS_CONTROLS);
						viv_multiple_instances = 0;
						
						if (IsDlgButtonChecked(general_page,IDC_APPDATA) == BST_CHECKED) 
						{
							if (!viv_appdata)
							{
								viv_append_admin_param(params,(const utf8_t *)"appdata");
								viv_appdata = 1;
							}
						}
						else
						{
							if (viv_appdata)
							{
								viv_append_admin_param(params,(const utf8_t *)"noappdata");

								viv_appdata = 0;
							}
						}

						if (IsDlgButtonChecked(general_page,IDC_MULTIPLE_INSTANCES) == BST_CHECKED) 
						{
							viv_multiple_instances = 1;
						}
						
						if (IsDlgButtonChecked(general_page,IDC_STARTMENU) == BST_CHECKED) 
						{
							if (!viv_is_start_menu_shortcuts())
							{
								viv_append_admin_param(params,(const utf8_t *)"startmenu");
							}
						}
						else
						{
							if (viv_is_start_menu_shortcuts())
							{
								viv_append_admin_param(params,(const utf8_t *)"nostartmenu");
							}
						}
						
						
						for(exti=0;exti<VIV_ASSOCIATION_COUNT;exti++)
						{
							if (IsDlgButtonChecked(general_page,viv_association_dlg_item_id[exti]) == BST_CHECKED) 
							{
								if (!viv_is_association(viv_association_extensions[exti]))
								{
									viv_append_admin_param(params,viv_association_extensions[exti]);
								}
							}
							else 
							{
								if (viv_is_association(viv_association_extensions[exti]))
								{
									string_cat_utf8(params," /no");
									string_cat_utf8(params,viv_association_extensions[exti]);
								}
							}
						}
						
						old_shrink_blit_mode = viv_shrink_blit_mode;
						
						viv_left_click_action = ComboBox_GetCurSel(GetDlgItem(controls_page,IDC_LEFTCLICKACTION));
						viv_right_click_action = ComboBox_GetCurSel(GetDlgItem(controls_page,IDC_RIGHTCLICKACTION));
						viv_mouse_wheel_action = ComboBox_GetCurSel(GetDlgItem(controls_page,IDC_MOUSEWHEELACTION));
						
						if (ComboBox_GetCurSel(GetDlgItem(view_page,IDC_COMBO1)) == 1)
						{
							viv_shrink_blit_mode = VIV_SHRINK_BLIT_MODE_HALFTONE;
						}
						else
						{
							viv_shrink_blit_mode = VIV_SHRINK_BLIT_MODE_COLORONCOLOR;
						}
						
						colorref = (COLORREF)GetWindowLongPtr(GetDlgItem(view_page,IDC_WINDOWEDBACKGROUNDCOLOR),GWLP_USERDATA);

						if ((GetRValue(colorref) != viv_windowed_background_color_r) || (GetGValue(colorref) != viv_windowed_background_color_g) || (GetBValue(colorref) != viv_windowed_background_color_b))
						{
							viv_windowed_background_color_r = GetRValue(colorref);
							viv_windowed_background_color_g = GetGValue(colorref);
							viv_windowed_background_color_b = GetBValue(colorref);

							InvalidateRect(viv_hwnd,0,FALSE);
						}
						
						colorref = (COLORREF)GetWindowLongPtr(GetDlgItem(view_page,IDC_FULLSCREENBACKGROUNDCOLOR),GWLP_USERDATA);
						
						if ((GetRValue(colorref) != viv_fullscreen_background_color_r) || (GetGValue(colorref) != viv_fullscreen_background_color_g) || (GetBValue(colorref) != viv_fullscreen_background_color_b))
						{
							viv_fullscreen_background_color_r = GetRValue(colorref);
							viv_fullscreen_background_color_g = GetGValue(colorref);
							viv_fullscreen_background_color_b = GetBValue(colorref);

							InvalidateRect(viv_hwnd,0,FALSE);
						}
						
						if (old_shrink_blit_mode != viv_shrink_blit_mode)
						{
							InvalidateRect(viv_hwnd,0,FALSE);
						}
						
						old_shrink_blit_mode = viv_mag_filter;
						
						if (ComboBox_GetCurSel(GetDlgItem(view_page,IDC_COMBO2)) == 1)
						{
							viv_mag_filter = VIV_SHRINK_BLIT_MODE_HALFTONE;
						}
						else
						{
							viv_mag_filter = VIV_SHRINK_BLIT_MODE_COLORONCOLOR;
						}
						
						if (old_shrink_blit_mode != viv_mag_filter)
						{
							InvalidateRect(viv_hwnd,0,FALSE);
						}

						viv_loop_animations_once = IsDlgButtonChecked(view_page,IDC_LOOP_ANIMATIONS_ONCE) == BST_CHECKED ? 1 : 0;
						
						// copy keys.
						viv_key_list_copy(viv_key_list,(viv_key_list_t *)GetWindowLongPtr(GetDlgItem(controls_page,IDC_COMMANDS_LIST),GWLP_USERDATA));
						
						// reinit menu.
						{
							HMENU new_hmenu;
							
							new_hmenu = viv_create_menu();
							
							if (GetMenu(viv_hwnd))
							{
								SetMenu(viv_hwnd,new_hmenu);
							}
							
							if (viv_hmenu)
							{
								DestroyMenu(viv_hmenu);
							}
							
							viv_hmenu = new_hmenu;
						}
						
						// do admin commands.
						if (*params)
						{	
							wchar_t exe_filename[STRING_SIZE];
							
							GetModuleFileName(0,exe_filename,STRING_SIZE);
							
							os_shell_execute(0,exe_filename,1,NULL,params);
						}

						// save settings to disk						
						viv_save_settings(viv_appdata);
					}
				
					EndDialog(hwnd,0);
					break;
			}

			break;
	}
	
	return FALSE;
}

static void viv_options(void)
{	
	DialogBox(os_hinstance,MAKEINTRESOURCE(IDD_OPTIONS),viv_hwnd,viv_options_proc);
}


// use the default class description, ie: TXT File
static void viv_install_association_by_extension(const char *association,const char *description,const char *icon_location)
{
	HKEY hkey;
	wchar_t class_name[STRING_SIZE];
	wchar_t key[STRING_SIZE];
	wchar_t default_icon[STRING_SIZE];
	wchar_t dot_association[STRING_SIZE];
	
	string_copy_utf8(dot_association,(const utf8_t *)".");
	string_cat_utf8(dot_association,association);
	
	// make sure we uninstall old associations first.
	viv_uninstall_association_by_extension(association);

	string_copy_utf8(class_name,"voidImageViewer");
	string_cat(class_name,dot_association);

	string_copy_utf8(default_icon,"voidImageViewer");
	string_cat(default_icon,dot_association);
	string_cat_utf8(default_icon,"\\DefaultIcon");

	string_copy(key,class_name);
	string_cat_utf8(key,"\\shell\\open\\command");
	
	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,default_icon,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		if (icon_location)
		{
			wchar_t icon_location_wbuf[STRING_SIZE];
			
			string_copy_utf8(icon_location_wbuf,icon_location);

			viv_set_registry_string(hkey,0,icon_location_wbuf);
		}
		else
		{
			wchar_t filename[STRING_SIZE];
			wchar_t command[STRING_SIZE];

			GetModuleFileName(0,filename,STRING_SIZE);
			
			string_copy(command,filename);
			string_cat_utf8(command,(const utf8_t *)", 1");
			
			viv_set_registry_string(hkey,0,command);
		}
			
		RegCloseKey(hkey);
	}	
		
	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,class_name,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t description_wbuf[STRING_SIZE];
		
		string_copy_utf8(description_wbuf,description);
		
		viv_set_registry_string(hkey,0,description_wbuf);
		
		RegCloseKey(hkey);
	}		
	
	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,key,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t filename[STRING_SIZE];
		wchar_t command[STRING_SIZE];

		GetModuleFileName(0,filename,STRING_SIZE);
		
		string_copy_utf8(command,(const utf8_t *)"\"");
		string_cat(command,filename);
		string_cat_utf8(command,(const utf8_t *)"\" \"%1\"");
		
		viv_set_registry_string(hkey,0,command);
		
		RegCloseKey(hkey);
	}

	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,dot_association,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t wbuf[STRING_SIZE];
		
		if (!viv_get_registry_string(hkey,(const utf8_t *)"voidImageViewer.Backup",wbuf,STRING_SIZE))
		{
			if (!viv_get_registry_string(hkey,0,wbuf,STRING_SIZE))
			{
				*wbuf = 0;
			}
			
			viv_set_registry_string(hkey,(const utf8_t *)"voidImageViewer.Backup",wbuf);
		}

		viv_set_registry_string(hkey,0,class_name);
		
		RegCloseKey(hkey);
	}
}

static void viv_uninstall_association_by_extension(const char *association)
{
	HKEY hkey;
	wchar_t class_name[STRING_SIZE];
	wchar_t dot_association[STRING_SIZE];
	
	string_copy_utf8(dot_association,(const utf8_t *)".");
	string_cat_utf8(dot_association,association);
	
	string_copy_utf8(class_name,(const utf8_t *)"voidImageViewer");
	string_cat(class_name,dot_association);
	
	if (RegCreateKeyExW(HKEY_CLASSES_ROOT,dot_association,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t wbuf[STRING_SIZE];
		
		if (viv_get_registry_string(hkey,(const utf8_t *)"voidImageViewer.Backup",wbuf,STRING_SIZE))
		{
			viv_set_registry_string(hkey,0,wbuf);

			RegDeleteValueA(hkey,"voidImageViewer.Backup");
		}

		RegCloseKey(hkey);
	}
	
	RegDeleteKey(HKEY_CLASSES_ROOT,class_name);
}

static int viv_is_association(const char *association)
{
	int ret;
	HKEY hkey;
	wchar_t class_name[STRING_SIZE];
	wchar_t key[STRING_SIZE];
	wchar_t dot_association[STRING_SIZE];
	
	string_copy_utf8(dot_association,(const utf8_t *)".");
	string_cat_utf8(dot_association,association);

	string_copy_utf8(class_name,(const utf8_t *)"voidImageViewer");
	string_cat(class_name,dot_association);
	
	string_copy(key,class_name);
	string_cat_utf8(key,(const utf8_t *)"\\shell\\open\\command");
	
	ret = 0;

	if (RegOpenKeyExW(HKEY_CLASSES_ROOT,dot_association,0,KEY_QUERY_VALUE,&hkey) == ERROR_SUCCESS)
	{
		wchar_t wbuf[STRING_SIZE];

		if (viv_get_registry_string(hkey,0,wbuf,STRING_SIZE))
		{
			if (string_compare(wbuf,class_name) == 0)
			{
				ret++;
			}
		}

		RegCloseKey(hkey);
	}

	if (RegOpenKeyExW(HKEY_CLASSES_ROOT,key,0,KEY_QUERY_VALUE,&hkey) == ERROR_SUCCESS)
	{
		wchar_t wbuf[STRING_SIZE];

		if (viv_get_registry_string(hkey,0,wbuf,STRING_SIZE))
		{
			wchar_t filename[STRING_SIZE];
			wchar_t command[STRING_SIZE];

			GetModuleFileName(0,filename,STRING_SIZE);
			
			string_copy_utf8(command,(const utf8_t *)"\"");
			string_cat(command,filename);
			string_cat_utf8(command,(const utf8_t *)"\" \"%1\"");
			
			if (string_compare(wbuf,command) == 0)
			{
				ret++;
			}
		}

		RegCloseKey(hkey);
	}

	return ret == 2;
}

static int viv_get_registry_string(HKEY hkey,const utf8_t *value,wchar_t *wbuf,int size_in_wchars)
{
	wchar_t value_wbuf[STRING_SIZE];
	wchar_t *value_wp;
	DWORD cbData;
	DWORD type;
	
	if (value)
	{
		string_copy_utf8(value_wbuf,value);
		value_wp = value_wbuf;
	}
	else
	{
		value_wp = 0;
	}
	
	cbData = size_in_wchars * sizeof(wchar_t);
	
	if (RegQueryValueExW(hkey,value_wp,0,&type,(BYTE *)wbuf,&cbData) == ERROR_SUCCESS)
	{
		if ((type == REG_SZ) || (type == REG_EXPAND_SZ))
		{
			return 1;
		}
	}
	
	return 0;
}

static int viv_set_registry_string(HKEY hkey,const utf8_t *value,const wchar_t *wbuf)
{
	wchar_t value_wbuf[STRING_SIZE];
	wchar_t *value_wp;

	if (value)
	{
		string_copy_utf8(value_wbuf,value);
		value_wp = value_wbuf;
	}
	else
	{
		value_wp = 0;
	}	

	if (RegSetValueExW(hkey,value_wp,0,REG_SZ,(BYTE *)wbuf,(string_length(wbuf) + 1) * sizeof(wchar_t)) == ERROR_SUCCESS)
	{
		return 1;
	}
	
	return 0;
}

static void viv_timer_start(void)
{
	if (!viv_is_animation_timer)
	{
		viv_is_animation_timer = 1;
		viv_animation_timer_tick_start = GetTickCount();
		
		SetTimer(viv_hwnd,VIV_ID_ANIMATION_TIMER,USER_TIMER_MINIMUM,0);
	}
}

static void viv_show_cursor(void)
{
	if ((viv_is_fullscreen && (!viv_in_popup_menu)))
	{
		POINT pt;
		
		GetCursorPos(&pt);
		
		debug_printf("%d %d, last %d %d\n",pt.x,pt.y,viv_hide_cursor_x,viv_hide_cursor_y);
		
		if ((pt.x != viv_hide_cursor_x) || (pt.y != viv_hide_cursor_y))
		{
			if (!viv_cursor_shown)
			{
				ShowCursor(TRUE);
				
				viv_cursor_shown = 1;
			}

			if (viv_is_hide_cursor_timer)
			{
				KillTimer(viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER);
				SetTimer(viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER,VIV_HIDE_CURSOR_DELAY,0);
			}
			else
			{
				SetTimer(viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER,VIV_HIDE_CURSOR_DELAY,0);
				
				viv_is_hide_cursor_timer = 1;
			}
			
			viv_hide_cursor_x = pt.x;
			viv_hide_cursor_y = pt.y;
		}
	}
}

static void viv_animation_pause(void)
{
	viv_animation_play = !viv_animation_play;
}

static void viv_frame_step(void)
{
	viv_frame_looped = 0;
	
	if (viv_animation_play)
	{
		viv_animation_play = 0;
	}
	else
	{
		if (viv_frame_count > 1)			
		{
			if ((viv_frame_loaded_count == viv_frame_count) || (viv_frame_position + 1 < viv_frame_loaded_count))
			{
				viv_frame_position++;
				if (viv_frame_position == viv_frame_count)
				{
					viv_frame_position = 0;
				}
			
				viv_status_update();
				
				InvalidateRect(viv_hwnd,NULL,FALSE);
				UpdateWindow(viv_hwnd);
			}
		}
	}
}

static void viv_frame_prev(void)
{
	viv_frame_looped = 0;

	if (viv_animation_play)
	{
		viv_animation_play = 0;
	}
	else
	{
		if (viv_frame_count > 1)
		{
			if (viv_frame_position > 0)
			{
				viv_frame_position--;
			}
			else
			{
				viv_frame_position = viv_frame_loaded_count - 1;
			}
		
			viv_status_update();
			
			InvalidateRect(viv_hwnd,NULL,FALSE);
			UpdateWindow(viv_hwnd);
		}
	}
}

static void viv_view_1to1(void)
{
	if (viv_1to1)	
	{
		if (viv_have_old_zoom)
		{
			viv_1to1 = 0;
			viv_zoom_pos = viv_old_zoom_pos;
			viv_view_set(viv_view_x,viv_view_y,1);
			InvalidateRect(viv_hwnd,0,FALSE);
			
			return;
		}
	}
	
	viv_have_old_zoom = 1;
	viv_old_zoom_pos = viv_zoom_pos;
	
	viv_1to1 = 1;
	viv_zoom_pos = 0;
	viv_view_set(viv_view_x,viv_view_y,1);
	InvalidateRect(viv_hwnd,0,FALSE);
}

static void viv_dragdrop_clearall(void)
{	
	viv_dragdrop_t *d;
	
	d = viv_dragdrop_start;
	while(d)
	{
		viv_dragdrop_t *next_d;
		
		next_d = d->next;
		
		mem_free(d);
		
		d = next_d;
	}
	
	viv_dragdrop_start = 0;
	viv_dragdrop_last = 0;
}

static void viv_dragdrop_delete(const wchar_t *filename)
{
	viv_dragdrop_t *d;
	
	d = viv_dragdrop_start;
	while(d)
	{
		if (string_compare(d->fd.cFileName,filename) == 0)
		{
			if (viv_dragdrop_start == d)
			{
				viv_dragdrop_start = d->next;
			}
			else
			{
				d->prev->next = d->next;
			}
		
			if (viv_dragdrop_last == d)
			{
				viv_dragdrop_last = d->prev;
			}
			else
			{
				d->next->prev = d->prev;
			}
		
			mem_free(d);
		
			break;
		}
		
		d = d->next;
	}
}

static viv_dragdrop_t *viv_dragdrop_add(const WIN32_FIND_DATA *fd)
{
	viv_dragdrop_t *d;
	
	d = (viv_dragdrop_t *)mem_alloc(sizeof(viv_dragdrop_t));
	
	os_copy_memory(&d->fd,fd,sizeof(WIN32_FIND_DATA));
	
	if (viv_dragdrop_start)
	{
		viv_dragdrop_last->next = d;
		d->prev = viv_dragdrop_last;
	}
	else
	{
		viv_dragdrop_start = d;
		d->prev = 0;
	}
	
	viv_dragdrop_last = d;
	d->next = 0;
	
	return d;
}

static void viv_dragdrop_add_path(const wchar_t *full_path_and_filename)
{
	WIN32_FIND_DATA fd;
	wchar_t buf[STRING_SIZE];
	HANDLE h;

	string_copy(buf,full_path_and_filename);
	string_cat_utf8(buf,(const utf8_t *)"\\*.*");

	h = FindFirstFile(buf,&fd);
	if (h != INVALID_HANDLE_VALUE)
	{
		for(;;)
		{
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!((string_compare_ascii(fd.cFileName,".") == 0) || (string_compare_ascii(fd.cFileName,"..") == 0)))
				{
					string_copy(buf,full_path_and_filename);
					string_cat_utf8(buf,(const utf8_t *)"\\");
					string_cat(buf,fd.cFileName);
					
					viv_dragdrop_add_path(buf);
				}
			}
			else
			{
				if (viv_is_valid_filename(fd.cFileName))
				{
					string_copy(buf,full_path_and_filename);
					string_cat_utf8(buf,(const utf8_t *)"\\");
					string_cat(buf,fd.cFileName);
					
					string_copy(fd.cFileName,buf);
					
					viv_dragdrop_add(&fd);
				}
			}

			if (!FindNextFile(h,&fd)) break;
		}
		
		FindClose(h);
	}
}

static void viv_dragdrop_add_filename(const wchar_t *filename)
{
	WIN32_FIND_DATA fd;
	wchar_t full_path_and_filename[STRING_SIZE];
	wchar_t cwd[STRING_SIZE];
	
	GetCurrentDirectory(STRING_SIZE,cwd);

	string_path_combine(full_path_and_filename,cwd,filename);
	
	if (GetFileAttributesEx(full_path_and_filename,GetFileExInfoStandard,&fd))
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			viv_dragdrop_add_path(full_path_and_filename);
		}
		else
		{
			string_copy(fd.cFileName,full_path_and_filename);
			viv_dragdrop_add(&fd);
		}
	}
}

static void viv_timer_stop(void)
{
	if (viv_is_animation_timer)
	{
		viv_is_animation_timer = 0;
		
		KillTimer(viv_hwnd,VIV_ID_ANIMATION_TIMER);
	}
}

static int viv_is_window_maximized(HWND hwnd)
{
	// are we minimized with the WPF_RESTORETOMAXIMIZED flag set?
	if (IsIconic(hwnd))
	{
		WINDOWPLACEMENT wp;

		// IsZoomed does not work when the window is minimized.
		// its ok to use GetWindowPlacement here as we do not touch the normal rect.
		wp.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(hwnd,&wp);

		if (wp.flags & WPF_RESTORETOMAXIMIZED)
		{
			return 1;
		}
	}

	if (IsZoomed(hwnd))
	{
		return 1;
	}

	return 0;
}

static INT_PTR CALLBACK viv_custom_rate_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			os_center_dialog(hwnd);

			SetDlgItemInt(hwnd,IDC_CUSTOM_RATE_EDIT,viv_slideshow_custom_rate,FALSE);

			os_ComboBox_AddString(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO,(const utf8_t *)"milliseconds");
			os_ComboBox_AddString(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO,(const utf8_t *)"seconds");
			os_ComboBox_AddString(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO,(const utf8_t *)"minutes");
			
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO),viv_slideshow_custom_rate_type);

			return TRUE;
		}
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
					viv_slideshow_custom_rate_type = ComboBox_GetCurSel(GetDlgItem(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO));
					viv_slideshow_custom_rate = GetDlgItemInt(hwnd,IDC_CUSTOM_RATE_EDIT,NULL,FALSE);
					EndDialog(hwnd,1);
					break;

				case IDCANCEL:
					EndDialog(hwnd,0);
					break;
			}

			break;
	}
	
	return FALSE;
}

static INT_PTR CALLBACK viv_about_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_CTLCOLOREDIT:	
		case WM_CTLCOLORSTATIC:	
			if (((HWND)lParam == GetDlgItem(hwnd,IDC_ABOUTBACK)) || ((HWND)lParam == GetDlgItem(hwnd,IDC_ABOUTTITLE)))
			{
				SetTextColor((HDC)wParam,RGB(255,255,255));
				SetBkColor((HDC)wParam,RGB(0,0,0));
				return (LRESULT)GetStockObject(BLACK_BRUSH);
			}
			else
			{
				SetTextColor((HDC)wParam,RGB(0,0,0));
				SetBkColor((HDC)wParam,RGB(255,255,255));
				return (LRESULT)GetStockObject(WHITE_BRUSH);
			}
			break;
			
		case WM_PAINT:
		{
			RECT rect;
			PAINTSTRUCT ps;	
			GetClientRect(hwnd,&rect);
			BeginPaint(hwnd,&ps);
			rect.bottom -= (48 * os_logical_high) / 96;
			FillRect(ps.hdc,&rect,(HBRUSH)GetStockObject(WHITE_BRUSH));
			rect.top = rect.bottom;
			rect.bottom++;
			FillRect(ps.hdc,&rect,(HBRUSH)(COLOR_BTNSHADOW + 1));
			rect.top = rect.bottom;
			rect.bottom++;
			FillRect(ps.hdc,&rect,(HBRUSH)(COLOR_BTNHIGHLIGHT + 1));
			rect.top = rect.bottom;
			rect.bottom+=(46 * os_logical_high) / 96;
			FillRect(ps.hdc,&rect,(HBRUSH)(COLOR_BTNFACE + 1));
			EndPaint(hwnd,&ps);
			break;
		}
			
		case WM_INITDIALOG:
		{
			HFONT hfont;
			LOGFONT lf;
			wchar_t version_wbuf[STRING_SIZE];

			os_center_dialog(hwnd);

			os_SetDlgItemText(hwnd,IDC_ABOUTTITLE,(const utf8_t *)"void Image Viewer");
			os_SetDlgItemText(hwnd,IDC_ABOUTVOIDIMAGEVIEWER,(const utf8_t *)"void Image Viewer");
			string_printf(version_wbuf,"%d.%d.%d.%d%s %s",VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION,VERSION_BUILD,VERSION_TYPE,VERSION_TARGET_MACHINE);
			SetDlgItemText(hwnd,IDC_ABOUTVERSION,version_wbuf);
			os_SetDlgItemText(hwnd,IDC_ABOUTCOPYRIGHT,(const utf8_t *)"Copyright © 2017 David Carpenter");
			os_SetDlgItemText(hwnd,IDC_ABOUTEMAIL,(const utf8_t *)"david@voidtools.com");
			os_SetDlgItemText(hwnd,IDC_ABOUTWEBSITE,(const utf8_t *)"www.voidtools.com");
			
			if (!viv_about_hfont)
			{
				hfont = (HFONT)SendMessage(GetDlgItem(hwnd,IDC_ABOUTTITLE),WM_GETFONT,0,0);
				GetObject(hfont,sizeof(LOGFONT),&lf);
				
				lf.lfHeight = 32;
				viv_about_hfont = CreateFontIndirect(&lf);
			}
			
			SendMessage(GetDlgItem(hwnd,IDC_ABOUTTITLE),WM_SETFONT,(WPARAM)viv_about_hfont,0);
			
			return TRUE;
		}
		
		case WM_DESTROY:
			break;
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					EndDialog(hwnd,0);
					break;
			}

			break;
	}
	
	return FALSE;
}

static void viv_update_frame(void)
{
	if (!viv_is_fullscreen)
	{
		DWORD oldstyle;
		DWORD newstyle;
		RECT windowrect;
		RECT clientrect;
		RECT newrect;
		RECT oldrect;
		int was_maximized;
		
		was_maximized = 0;

		// get out of maximized state.
		if (viv_is_window_maximized(viv_hwnd))
		{
			ShowWindow(viv_hwnd,SW_RESTORE);
			was_maximized = 1;
		}
		
		oldstyle = GetWindowLong(viv_hwnd,GWL_STYLE);
		newstyle = oldstyle;
		
		GetClientRect(viv_hwnd,&clientrect);
		debug_printf("clientrect %d %d %d %d\n",clientrect.left,clientrect.top,clientrect.right,clientrect.bottom);
		
		CopyRect(&oldrect,&clientrect);
		AdjustWindowRect(&oldrect,oldstyle,GetMenu(viv_hwnd) ? TRUE : FALSE);
		
		oldrect.bottom += viv_get_status_high() + viv_get_controls_high();
		
		debug_printf("oldrect %d %d %d %d %d\n",oldrect.left,oldrect.top,oldrect.right,oldrect.bottom,GetMenu(viv_hwnd) ? TRUE : FALSE);
	
		if (viv_is_show_caption)	
		{
			newstyle |= WS_CAPTION | WS_SYSMENU;
		}
		else
		{
			newstyle &= ~(WS_CAPTION | WS_SYSMENU);
		}
		
		if (viv_is_show_thickframe)	
		{
			newstyle |= WS_THICKFRAME;
		}
		else
		{
			newstyle &= ~WS_THICKFRAME;
		}

		if (viv_is_show_menu)	
		{
			if (GetMenu(viv_hwnd) != viv_hmenu)
			{
				SetMenu(viv_hwnd,viv_hmenu);
			}
		}
		else
		{
			if (GetMenu(viv_hwnd) != 0)
			{
				SetMenu(viv_hwnd,0);
			}
		}
		
		viv_status_show(viv_is_show_status);
		viv_controls_show(viv_is_show_controls);

		CopyRect(&newrect,&clientrect);
		AdjustWindowRect(&newrect,newstyle,viv_is_show_menu ? TRUE : FALSE);

		newrect.bottom += viv_get_status_high() + viv_get_controls_high();

		debug_printf("newrect %d %d %d %d %d\n",newrect.left,newrect.top,newrect.right,newrect.bottom,viv_is_show_menu ? TRUE : FALSE);
		
		GetWindowRect(viv_hwnd,&windowrect);
		
		windowrect.left += newrect.left - oldrect.left;
		windowrect.top += newrect.top - oldrect.top;
		windowrect.right += newrect.right - oldrect.right;
		windowrect.bottom += newrect.bottom - oldrect.bottom;
	
		SetWindowLong(viv_hwnd,GWL_STYLE,newstyle);
		
		SetWindowPos(viv_hwnd,HWND_TOP,windowrect.left,windowrect.top,windowrect.right - windowrect.left,windowrect.bottom - windowrect.top,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOCOPYBITS);

		// if there is no catpion or thick frame we should not allow maximize
		// avoid our resize borders when maximized.
		if ((was_maximized) && (viv_is_show_caption) && (viv_is_show_thickframe))
		{
			ShowWindow(viv_hwnd,SW_MAXIMIZE);
		}
	}
}

static void viv_update_ontop(void)
{
	int is_top_most;
	
	is_top_most = 0;
	
	switch(viv_ontop)
	{
		case 1:
			is_top_most = 1;
			break;
			
		case 2:
			is_top_most = (viv_is_slideshow) || ((viv_frame_count > 1) && (viv_animation_play));
			break;
	}

	SetWindowPos(viv_hwnd,is_top_most ? HWND_TOPMOST : HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
}

static void viv_dst_pos_set(int x,int y)
{
	if (x < 0)
	{
		x = 0;
	}

	if (y < 0)
	{
		y = 0;
	}

	if (x > 1000)
	{
		x = 1000;
	}

	if (y > 1000)
	{
		y = 1000;
	}

	if ((x != viv_dst_pos_x) || (y != viv_dst_pos_y))
	{
		viv_dst_pos_x = x;
		viv_dst_pos_y = y;
		
		InvalidateRect(viv_hwnd,NULL,FALSE);
	}

	viv_status_update_temp_pos_zoom();	
}

static void viv_dst_zoom_set(int x,int y)
{
	if (x < 0)
	{
		x = 0;
	}
	else
	if (x > VIV_DST_ZOOM_MAX - 1)
	{
		x = VIV_DST_ZOOM_MAX - 1;
	}
	
	if (y < 0)
	{
		y = 0;
	}
	else
	if (y > VIV_DST_ZOOM_MAX - 1)
	{
		y = VIV_DST_ZOOM_MAX - 1;
	}
	
	if ((x != viv_dst_zoom_x_pos) || (y != viv_dst_zoom_y_pos))
	{
		viv_dst_zoom_x_pos = x;
		viv_dst_zoom_y_pos = y;
		
		InvalidateRect(viv_hwnd,NULL,FALSE);
	}

	viv_status_update_temp_pos_zoom();	
}

static void viv_frame_skip(int size)
{
	if (viv_frame_count > 1)			
	{
		if (size > 0)
		{
			while(size > 0)
			{
				if ((viv_frame_loaded_count == viv_frame_count) || (viv_frame_position + 1 < viv_frame_loaded_count))
				{
					viv_frame_position++;
					if (viv_frame_position == viv_frame_count)
					{
						viv_frame_position = 0;
					}
				}

				size -= viv_frame_delays[viv_frame_position];
			}
		}
		else
		if (size < 0)
		{
			while(size < 0)
			{
				viv_frame_position--;
				if (viv_frame_position < 0)
				{
					viv_frame_position = viv_frame_loaded_count - 1;
				}
				
				size += viv_frame_delays[viv_frame_position];
			}
		}
			
		viv_status_update();
		
		InvalidateRect(viv_hwnd,NULL,FALSE);
		UpdateWindow(viv_hwnd);
	}
}

// 14.447 - CreateStreamOnHGlobal - this is too slow over slow networks -which doesn't matter because the gif wont show the first frame until the entire gif is loaded anyway.
// 14.520 - CreateStreamOnHGlobal
// 15.834 - SHCreateStreamOnFile
// 15.850 - SHCreateStreamOnFile
// 14.914 - viv stream - 1MB file buffer
// 14.914 - viv stream
static DWORD WINAPI viv_load_image_proc(void *param)
{
	viv_reply_load_image_first_frame_t first_frame;
	IStream *stream;
	int ret;
	DWORD tickstart;

	tickstart = GetTickCount();
	
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);

	first_frame.wide = 0;
	first_frame.high = 0;
	first_frame.frame_count = 0;
	first_frame.frame_delays = 0;
	first_frame.frame.hbitmap = 0;

	ret = 0;
	stream = 0;

	debug_printf("load %S...\n",viv_load_image_filename);
	
	{
		HANDLE h;
		
		h = CreateFile(viv_load_image_filename,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
		if (h != INVALID_HANDLE_VALUE)
		{
			DWORD size;
			HANDLE global_handle;

			size = GetFileSize(h,0);
			
			global_handle = GlobalAlloc(GMEM_MOVEABLE,size);
			if (global_handle)
			{
				char *buf;
				
				buf = (char *)GlobalLock(global_handle);
				
				if (buf)
				{
					DWORD numread;
					DWORD totreadsize;
					char *d;
					
					d = buf;
					totreadsize = size;
					
					while(totreadsize)
					{
						DWORD readsize;

						if (viv_load_image_terminate)
						{
							break;
						}
						
						readsize = 1 * 1024 * 1024;
						
						if (readsize > totreadsize)
						{
							readsize = totreadsize;
						}
						
						if (ReadFile(h,d,readsize,&numread,0))
						{
							if (!numread)
							{
								break;
							}

							d += numread;
							totreadsize -= numread;
						}
						else
						{
							break;
						}
					}

					GlobalUnlock(global_handle);
					
					if (totreadsize == 0)
					{
						HRESULT hresult;
						
						hresult = CreateStreamOnHGlobal(global_handle,TRUE,&stream);
						
						if (SUCCEEDED(hresult))
						{
							// the stream owns this handle now.
							global_handle = 0;
						}
						else
						{
							debug_printf("CreateStreamOnHGlobal %x\n",hresult);
							stream = 0;
						}
					}
				}
				
				if (global_handle)
				{
					GlobalFree(global_handle);
				}
			}

			CloseHandle(h);
		}
		else
		{
			debug_printf("CreateFile %x\n",GetLastError());
		}
	}

	if (stream)
	{
		debug_printf("stream %p\n",stream);
		
		if (!viv_load_image_terminate)
		{
			void *image;
			int load_ret;
			
			load_ret = os_GdipLoadImageFromStream(stream,&image);
			
			debug_printf("image %p\n",image);

			if (load_ret == 0)
			{
				if (!viv_load_image_terminate)
				{
					if (os_GdipGetImageWidth(image,&first_frame.wide) == 0)
					{
						if (os_GdipGetImageHeight(image,&first_frame.high) == 0)
						{
							UINT count;

							//First of all we should get the number of frame dimensions
							//Images considered by GDI+ as:
							//frames[animation_frame_index][how_many_animation];
							if (os_GdipImageGetFrameDimensionsCount(image,&count) == 0)
							{
								//Now we should get the identifiers for the frame dimensions 
								{
									GUID *DimensionIDs;
									
									DimensionIDs = mem_alloc(sizeof(GUID) * count);
									
									if (os_GdipImageGetFrameDimensionsList(image,DimensionIDs,count) == 0)
									{
										//For gif image , we only care about animation set#0
										WCHAR strGuid[39];
										
										StringFromGUID2(&DimensionIDs[0], strGuid, 39);
										
										os_GdipImageGetFrameCount(image,&DimensionIDs[0],&first_frame.frame_count);

									}

									mem_free(DimensionIDs);
								}

								// get frame delays.
								if (first_frame.frame_count > 1)
								{
									os_PropertyItem_t *frame_delay;
									UINT size;
									DWORD i;

									// PropertyTagFrameDelay 0x5100
									os_GdipGetPropertyItemSize(image,0x5100,&size);

									debug_printf("frame delay size %d\n",size);

									frame_delay = (os_PropertyItem_t *)mem_alloc(size);
									
									// PropertyTagFrameDelay 0x5100
									os_GdipGetPropertyItem(image,0x5100,size,frame_delay);
									
									first_frame.frame_delays = (DWORD *)mem_alloc(first_frame.frame_count * sizeof(int));

									for(i=0;i<first_frame.frame_count;i++)
									{
										first_frame.frame_delays[i] = (((UINT *)frame_delay[0].value)[i]) * 10;
										
										if (!first_frame.frame_delays[i])
										{
											first_frame.frame_delays[i] = 100;
										}
									}
									
									mem_free(frame_delay);
								}

								// draw frames.

								{
									HDC screen_hdc;
								
									screen_hdc = GetDC(0);
									
									if (screen_hdc)
									{
										HDC mem_hdc;
									
										mem_hdc = CreateCompatibleDC(screen_hdc);
										if (mem_hdc)
										{
											DWORD i;
											
											for(i=0;i<first_frame.frame_count;i++)
											{
												HBITMAP hbitmap;
												HGDIOBJ last_hbitmap;
												
												if (viv_load_image_terminate)
												{
													break;
												}
												
												hbitmap = CreateCompatibleBitmap(screen_hdc,first_frame.wide,first_frame.high);
												if (hbitmap)
												{
													UINT image_flags;
													
													last_hbitmap = SelectObject(mem_hdc,hbitmap);
													
													os_GdipImageSelectActiveFrame(image,&viv_FrameDimensionTime,i);

													os_GdipGetImageFlags(image,&image_flags);
													
													{
														void *g;
														int draw_ret;

														if (os_GdipCreateFromHDC(mem_hdc,&g) == 0)
														{
															if (image_flags & 2)
															{
																{
																	RECT rect;
																	HBRUSH hbrush;
																	
																	hbrush = CreateSolidBrush(RGB(viv_windowed_background_color_r,viv_windowed_background_color_g,viv_windowed_background_color_b));
																	
																	rect.left = 0;
																	rect.top = 0;
																	rect.right = first_frame.wide;
																	rect.bottom = first_frame.high;

																	FillRect(mem_hdc,&rect,hbrush);
																	
																	DeleteObject(hbrush);
																}
														
																os_GdipSetCompositingMode(g,0);
															}
															else
															{
																os_GdipSetCompositingMode(g,1);
															}

															os_GdipSetCompositingQuality(g,1);
															
															// Gdiplus::InterpolationModeNearestNeighbor
															os_GdipSetInterpolationMode(g,5);
															
															// Gdiplus::PixelOffsetModeNone
															os_GdipSetPixelOffsetMode(g,3);
															
															// Gdiplus::SmoothingModeNone
															os_GdipSetSmoothingMode(g,3);

															draw_ret = os_GdipDrawImageRectI(g,image,0,0,first_frame.wide,first_frame.high);
															if (draw_ret != 0)
															{
																debug_printf("DrawImage failed %d\n",draw_ret);
															}
															
															os_GdipDeleteGraphics(g);
														}
													}
													
													SelectObject(mem_hdc,last_hbitmap);
												
													if (i)
													{
														viv_frame_t frame;
														
														frame.hbitmap = hbitmap;
														
														viv_reply_add(VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME,sizeof(viv_frame_t),&frame);
													}
													else
													{
														first_frame.frame.hbitmap = hbitmap;
														
														viv_reply_add(VIV_REPLY_LOAD_IMAGE_FIRST_FRAME,sizeof(viv_reply_load_image_first_frame_t),&first_frame);
													}
													
													ret = 1;
												}
											}
											
											DeleteDC(mem_hdc);
										}
										
										ReleaseDC(0,screen_hdc);
									}
								}

								debug_printf("image loaded\n");
							}
						}
					}
				}

				os_GdipDisposeImage(image);
			}
			else
			{
				debug_printf("Failed to load image %S %d\n",viv_load_image_filename,load_ret);
			}
		}

		stream->lpVtbl->Release(stream);				
	}
	else
	{
		debug_printf("Failed to create stream from %S\n",viv_load_image_filename);
	}

	viv_reply_add(ret ? VIV_REPLY_LOAD_IMAGE_COMPLETE : VIV_REPLY_LOAD_IMAGE_FAILED,0,0);
					
	CoUninitialize();
	
	debug_printf("loaded in %f seconds\n",(double)(GetTickCount()-tickstart) * 0.001);

    return 0;
}

static void viv_reply_free(viv_reply_t *e)
{
	switch (e->type)
	{
		case VIV_REPLY_LOAD_IMAGE_FIRST_FRAME:
			
			if (((viv_reply_load_image_first_frame_t *)(e+1))->frame_delays)
			{
				mem_free(((viv_reply_load_image_first_frame_t *)(e+1))->frame_delays);
			}
		
			if (((viv_reply_load_image_first_frame_t *)(e+1))->frame.hbitmap)
			{
				DeleteObject(((viv_reply_load_image_first_frame_t *)(e+1))->frame.hbitmap);
			}
		
			break;

		case VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME:
		
			if (((viv_frame_t *)(e + 1))->hbitmap)
			{	
				DeleteObject(((viv_frame_t *)(e + 1))->hbitmap);
			}
		
			break;
	}
	
	mem_free(e);
}

static viv_reply_t *viv_reply_add(DWORD type,DWORD size,void *data)
{
	viv_reply_t *e;
	int is_first;
	
	is_first = 0;
	
	e = (viv_reply_t *)mem_alloc(sizeof(viv_reply_t) + size);
	
	e->type = type;
	e->size = size;
	
	os_copy_memory(e + 1,data,size);
	
	EnterCriticalSection(&viv_cs);
	
	if (viv_reply_start)
	{
		viv_reply_last->next = e;
	}
	else
	{
		viv_reply_start = e;
		is_first = 1;
	}
	
	viv_reply_last = e;
	e->next = 0;
	
	LeaveCriticalSection(&viv_cs);
	
	if (is_first)
	{
		PostMessage(viv_hwnd,VIV_WM_REPLY,0,0);
	}
	
	return e;
}

static void viv_reply_clear_all(void)
{
	viv_reply_t *e;
	
	EnterCriticalSection(&viv_cs);
	
	e = viv_reply_start;
	viv_reply_start = 0;
	viv_reply_last = 0;
	
	LeaveCriticalSection(&viv_cs);
	
	while(e)
	{
		viv_reply_t *next_e;
		
		next_e = e->next;
		
		viv_reply_free(e);
		
		e = next_e;
	}
}

static void viv_status_show(int show)
{
	if (show)
	{
		if (!viv_status_hwnd)
		{
			viv_status_hwnd = os_CreateWindowEx(
				0,
				STATUSCLASSNAMEA,
				"",
				WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | SBARS_SIZEGRIP | WS_VISIBLE,
				0,0,0,0,
				viv_hwnd,(HMENU)VIV_ID_STATUS,os_hinstance,NULL);
		}
	}
	else
	{
		if (viv_status_hwnd)
		{
			DestroyWindow(viv_status_hwnd);

			viv_status_hwnd = 0;
		}	
	}
	
	viv_on_size();
}

static void viv_controls_show(int show)
{
	if (show)
	{
		if (!viv_toolbar_hwnd)
		{
			os_RegisterClassEx(
				CS_DBLCLKS,
				viv_rebar_proc,
				0,
				LoadCursor(NULL,IDC_ARROW),
				(HBRUSH)(COLOR_WINDOW+1),
				"VIV_REBAR",
				0);

			viv_rebar_hwnd = os_CreateWindowEx(
				0,
				"VIV_REBAR",
				"",
				WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE,
				0,0,0,0,
				viv_hwnd,(HMENU)VIV_ID_TOOLBAR,os_hinstance,NULL);

			viv_toolbar_hwnd = os_CreateWindowEx(
				0,
				TOOLBARCLASSNAMEA,
				"",
				WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NODIVIDER | CCS_NORESIZE | CCS_TOP,
				0,0,0,0,
				viv_rebar_hwnd,(HMENU)VIV_ID_TOOLBAR,os_hinstance,NULL);
			
			SendMessage(viv_toolbar_hwnd,TB_SETEXTENDEDSTYLE,0,TBSTYLE_EX_MIXEDBUTTONS|TBSTYLE_EX_HIDECLIPPEDBUTTONS|TBSTYLE_EX_DOUBLEBUFFER);
			SendMessage(viv_toolbar_hwnd,TB_BUTTONSTRUCTSIZE,sizeof(TBBUTTON),0);
			
			viv_toolbar_image_list = ImageList_Create((16 * os_logical_wide) / 96,(16 * os_logical_high) / 96,ILC_COLOR24|ILC_MASK,0,0);
			
			ImageList_AddIcon(viv_toolbar_image_list,LoadIcon(os_hinstance,(LPCTSTR)IDI_PREV));
			ImageList_AddIcon(viv_toolbar_image_list,LoadIcon(os_hinstance,(LPCTSTR)IDI_PLAY));
			ImageList_AddIcon(viv_toolbar_image_list,LoadIcon(os_hinstance,(LPCTSTR)IDI_PAUSE));
			ImageList_AddIcon(viv_toolbar_image_list,LoadIcon(os_hinstance,(LPCTSTR)IDI_NEXT));
			ImageList_AddIcon(viv_toolbar_image_list,LoadIcon(os_hinstance,(LPCTSTR)IDI_BESTFIT));
			ImageList_AddIcon(viv_toolbar_image_list,LoadIcon(os_hinstance,(LPCTSTR)IDI_1TO1));

			SendMessage(viv_toolbar_hwnd,TB_SETIMAGELIST,0,(LPARAM)viv_toolbar_image_list);

			{
				TBBUTTON buttons[8];
				int buttoni;
				
				buttoni = 0;
	
				buttons[buttoni].iBitmap = 0;
				buttons[buttoni].idCommand = VIV_ID_NAV_PREV;
				buttons[buttoni].fsState = TBSTATE_ENABLED;
				buttons[buttoni].fsStyle = TBSTYLE_BUTTON;
				buttons[buttoni].iString = (INT_PTR)L"Previous Image";
				buttoni++;
				
				buttons[buttoni].iBitmap = 3;
				buttons[buttoni].idCommand = VIV_ID_NAV_NEXT;
				buttons[buttoni].fsState = TBSTATE_ENABLED;
				buttons[buttoni].fsStyle = TBSTYLE_BUTTON;
				buttons[buttoni].iString = (INT_PTR)L"Next Image";
				buttoni++;
	
				buttons[buttoni].iBitmap = 0;
				buttons[buttoni].idCommand = 0;
				buttons[buttoni].fsState = 0;
				buttons[buttoni].fsStyle = TBSTYLE_SEP;
				buttons[buttoni].iString = 0;
				buttoni++;
				
				buttons[buttoni].iBitmap = 1;
				buttons[buttoni].idCommand = VIV_ID_SLIDESHOW_PLAY_ONLY;
				buttons[buttoni].fsState = TBSTATE_ENABLED;
				buttons[buttoni].fsStyle = TBSTYLE_BUTTON|TBSTYLE_CHECK|TBSTYLE_GROUP;
				buttons[buttoni].iString = (INT_PTR)L"Play Slideshow";
				buttoni++;
	
				buttons[buttoni].iBitmap = 2;
				buttons[buttoni].idCommand = VIV_ID_SLIDESHOW_PAUSE_ONLY;
				buttons[buttoni].fsState = TBSTATE_ENABLED;
				buttons[buttoni].fsStyle = TBSTYLE_BUTTON|TBSTYLE_CHECK|TBSTYLE_GROUP;
				buttons[buttoni].iString = (INT_PTR)L"Pause Slideshow";
				buttoni++;
	
				buttons[buttoni].iBitmap = 0;
				buttons[buttoni].idCommand = 0;
				buttons[buttoni].fsState = 0;
				buttons[buttoni].fsStyle = TBSTYLE_SEP;
				buttons[buttoni].iString = 0;
				buttoni++;
					
				buttons[buttoni].iBitmap = 4;
				buttons[buttoni].idCommand = VIV_ID_VIEW_BESTFIT;
				buttons[buttoni].fsState = TBSTATE_ENABLED;
				buttons[buttoni].fsStyle = TBSTYLE_BUTTON;
				buttons[buttoni].iString = (INT_PTR)L"Best Fit";
				buttoni++;				
					
				buttons[buttoni].iBitmap = 5;
				buttons[buttoni].idCommand = VIV_ID_VIEW_1TO1;
				buttons[buttoni].fsState = TBSTATE_ENABLED;
				buttons[buttoni].fsStyle = TBSTYLE_BUTTON;
				buttons[buttoni].iString = (INT_PTR)L"Actual Size";
				buttoni++;
					
				SendMessage(viv_toolbar_hwnd,TB_ADDBUTTONS,8,(LPARAM)buttons);
			}

			viv_toolbar_update_buttons();
			ShowWindow(viv_toolbar_hwnd,SW_SHOW);
					
//			Toolba				
//			TB_ADDBUTTONS();
		}
	}
	else
	{
		if (viv_toolbar_hwnd)
		{
			DestroyWindow(viv_toolbar_hwnd);
			DestroyWindow(viv_rebar_hwnd);

			viv_toolbar_hwnd = 0;
			viv_rebar_hwnd = 0;
		}	
		
		if (viv_toolbar_image_list)
		{
			ImageList_Destroy(viv_toolbar_image_list);
			
			viv_toolbar_image_list = 0;
		}
	}
	
	viv_on_size();
}

static void viv_status_update(void)
{
	if (viv_status_hwnd)
	{
		int part_array[3];
		RECT rect;
		wchar_t widebuf[STRING_SIZE];
		wchar_t highbuf[STRING_SIZE];
		wchar_t dimension_buf[STRING_SIZE];
		wchar_t frame_buf[STRING_SIZE];
		HDC hdc;
		int dimension_wide;
		int frame_wide;
		int minwide;
		
		GetClientRect(viv_hwnd,&rect);
		
		if ((viv_image_wide) && (viv_image_high))
		{
			string_format_number(widebuf,viv_image_wide);
			string_format_number(highbuf,viv_image_high);
		
			string_copy(dimension_buf,widebuf);
			string_cat_utf8(dimension_buf,(const utf8_t *)" x ");
			string_cat(dimension_buf,highbuf);
		}
		else
		{
			string_copy_utf8(dimension_buf,(const utf8_t *)"");
		}
		
		if (viv_frame_count > 1)
		{
			int frame_pos;
			string_format_number(highbuf,viv_frame_count);

			string_copy_utf8(frame_buf,(const utf8_t *)"");
			
			if (viv_frame_minus)
			{
				frame_pos = viv_frame_count - (viv_frame_position);
				string_cat_utf8(frame_buf,(const utf8_t *)"- ");
			}
			else
			{
				frame_pos = viv_frame_position + 1;
			}

			string_format_number(widebuf,frame_pos);
			
			string_cat(frame_buf,widebuf);
			string_cat_utf8(frame_buf,(const utf8_t *)" / ");
			string_cat(frame_buf,highbuf);
		}
		else
		{
			string_copy_utf8(frame_buf,(const utf8_t *)"");
		}
		
		dimension_wide = 0;
		frame_wide = 0;
		minwide = (72 * os_logical_wide) / 96;

		hdc = GetDC(viv_status_hwnd);
		if (hdc)
		{
			HFONT hfont;
			
			hfont = (HFONT)SendMessage(viv_status_hwnd,WM_GETFONT,0,0);
			if (hfont)
			{
				SIZE size;
				HGDIOBJ last_font;
				
				last_font = SelectObject(hdc,hfont);
				
				if (GetTextExtentPoint32(hdc,dimension_buf,string_length(dimension_buf),&size))
				{
					dimension_wide = size.cx + GetSystemMetrics(SM_CXEDGE) * 5;
					
					if (dimension_wide < minwide)
					{
						dimension_wide = minwide;
					}
				}

				if (*frame_buf)
				{
					if (GetTextExtentPoint32(hdc,frame_buf,string_length(frame_buf),&size))
					{
						frame_wide = size.cx + GetSystemMetrics(SM_CXEDGE) * 5;
						
						if (frame_wide < minwide)
						{
							frame_wide = minwide;
						}
					}
				}

				SelectObject(hdc,last_font);
			}
			
			ReleaseDC(viv_status_hwnd,hdc);
		}
		
		// add size box
		dimension_wide += GetSystemMetrics(SM_CXVSCROLL) + GetSystemMetrics(SM_CXBORDER);
		
		part_array[0] = (rect.right - rect.left) - dimension_wide - frame_wide;
		if (part_array[0] < 0)
		{
			part_array[0] = 0;
		}
		
		part_array[1] = (rect.right - rect.left) - dimension_wide;
		part_array[2] = -1;
		
		SendMessage(viv_status_hwnd,SB_SETPARTS,3,(LPARAM)part_array);

		{
			wchar_t *text;
			
			text =  L"";
			
			if (viv_status_temp_text)
			{
				text = viv_status_temp_text;
			}
			else
			if (viv_load_image_thread)
			{	
				text =  L"Loading...";
			}
			else
			if (viv_is_slideshow)
			{
				text =  L"Slideshow playing";
			}
			
			viv_status_set(0,text);
		}
		
		viv_status_set(1,frame_buf);
		viv_status_set(2,dimension_buf);
	}
}

static void viv_status_set(int part,const wchar_t *text)
{
	wchar_t oldtext[STRING_SIZE];
	
	SendMessage(viv_status_hwnd,SB_GETTEXTW,(WPARAM)part,(LPARAM)oldtext);
	
	if (string_compare(oldtext,text) != 0)
	{	
		SendMessage(viv_status_hwnd,SB_SETTEXTW,(WPARAM)part,(LPARAM)text);
	}
}

static int viv_get_status_high(void)
{
	if (viv_status_hwnd)
	{
		RECT rect;
		
		GetWindowRect(viv_status_hwnd,&rect);
		
		return rect.bottom - rect.top;
	}
	
	return 0;
}

static int viv_get_controls_high(void)
{
	if (viv_toolbar_hwnd)
	{
		return (32 * os_logical_high) / 96;
	}
	
	return 0;
}

static LRESULT CALLBACK viv_rebar_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) 
	{	
		case WM_LBUTTONDOWN:

			SendMessage(viv_hwnd,WM_NCLBUTTONDOWN,(WPARAM)HTCAPTION,lParam);
			return 0;
			
			break;
			
		case WM_NOTIFY:

			switch(((NMHDR *)lParam)->idFrom)
			{
				case VIV_ID_TOOLBAR:

					if (viv_toolbar_hwnd)
					{
						switch(((NMHDR *)lParam)->code)
						{
							case NM_CUSTOMDRAW:
							{
								switch(((NMTBCUSTOMDRAW *)lParam)->nmcd.dwDrawStage)
								{
									case CDDS_PREPAINT:	
									{
										RECT rect;
										GetClientRect(viv_toolbar_hwnd,&rect);
										FillRect(((NMTBCUSTOMDRAW *)lParam)->nmcd.hdc,&rect,(HBRUSH)(COLOR_BTNFACE+1));
										return CDRF_NOTIFYITEMDRAW;
									}
								}
								
								break;
							}
						}
					}
	
					break;
			}
			
			break;

			
		case WM_COMMAND:
			return SendMessage(viv_hwnd,WM_COMMAND,wParam,lParam);
			
		case WM_PAINT:
		{
			RECT rect;
			int wide;
			int high;
			PAINTSTRUCT ps;
			
			GetClientRect(hwnd,&rect);
			wide = rect.right - rect.left;
			high = rect.bottom - rect.top;

			BeginPaint(hwnd,&ps);
			
			rect.left = 0;
			rect.top = 0;
			rect.right = wide;
			rect.bottom = 1;
			
//			FillRect(ps.hdc,&rect,(HBRUSH)(COLOR_WINDOW + 1));
			FillRect(ps.hdc,&rect,(HBRUSH)(COLOR_3DSHADOW + 1));
			
			rect.left = 0;
			rect.top = 1;
			rect.right = wide;
			rect.bottom = 2;
			
//			FillRect(ps.hdc,&rect,(HBRUSH)(COLOR_WINDOW + 1));
			FillRect(ps.hdc,&rect,(HBRUSH)(COLOR_3DHIGHLIGHT + 1));
			
			rect.left = 0;
			rect.top = 2;
			rect.right = wide;
			rect.bottom = high;
			
//			FillRect(ps.hdc,&rect,(HBRUSH)(COLOR_WINDOW + 1));
			FillRect(ps.hdc,&rect,(HBRUSH)(COLOR_BTNFACE + 1));
			
			EndPaint(hwnd,&ps);
			
			return 0;
		}
			
		case WM_ERASEBKGND:
			return 1;
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static int viv_toolbar_get_wide(void)
{
	if (viv_toolbar_hwnd)
	{	
		SIZE size;
		
		if (SendMessage(viv_toolbar_hwnd,TB_GETMAXSIZE,0,(LPARAM)&size))
		{
			return size.cx;
		}
	}
	
	return 0;
}

static void viv_toolbar_update_buttons(void)
{
	if (viv_toolbar_hwnd)
	{
		TBBUTTONINFO tbbinfo;
		int rw;
		int rh;
		
		tbbinfo.cbSize = sizeof(TBBUTTONINFO);
		tbbinfo.dwMask = TBIF_STATE;
		tbbinfo.fsState = viv_is_slideshow ? (TBSTATE_ENABLED | TBSTATE_CHECKED) : (TBSTATE_ENABLED);
		
		SendMessage(viv_toolbar_hwnd,TB_SETBUTTONINFO,VIV_ID_SLIDESHOW_PLAY_ONLY,(LPARAM)&tbbinfo);
		
		tbbinfo.cbSize = sizeof(TBBUTTONINFO);
		tbbinfo.dwMask = TBIF_STATE;
		tbbinfo.fsState = viv_is_slideshow ? (TBSTATE_ENABLED) : (TBSTATE_ENABLED | TBSTATE_CHECKED);
		
		SendMessage(viv_toolbar_hwnd,TB_SETBUTTONINFO,VIV_ID_SLIDESHOW_PAUSE_ONLY,(LPARAM)&tbbinfo);
		
		viv_get_render_size(&rw,&rh);
		
		tbbinfo.cbSize = sizeof(TBBUTTONINFO);
		tbbinfo.dwMask = TBIF_STATE;
		tbbinfo.fsState = ((rw == viv_image_wide) && (rh == viv_image_high)) ? (0) : (TBSTATE_ENABLED);
		
		SendMessage(viv_toolbar_hwnd,TB_SETBUTTONINFO,VIV_ID_VIEW_1TO1,(LPARAM)&tbbinfo);
		
		tbbinfo.cbSize = sizeof(TBBUTTONINFO);
		tbbinfo.dwMask = TBIF_STATE;
		tbbinfo.fsState = ((viv_zoom_pos == 0) && (!viv_1to1)) ? (0) : (TBSTATE_ENABLED);
		
		SendMessage(viv_toolbar_hwnd,TB_SETBUTTONINFO,VIV_ID_VIEW_BESTFIT,(LPARAM)&tbbinfo);
	}
}

static LRESULT CALLBACK viv_fullscreen_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			EndPaint(hwnd,&ps);
			break;
		}
			
		case WM_ERASEBKGND:
			return 1;
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static void viv_status_set_temp_text(wchar_t *text)
{
	if (viv_status_temp_text)
	{
		KillTimer(viv_hwnd,VIV_ID_STATUS_TEMP_TEXT_TIMER);
		
		mem_free(viv_status_temp_text);
		
		viv_status_temp_text = 0;
	}
	
	if (text)
	{
		viv_status_temp_text = string_alloc(text);
	}
	
	viv_status_update();
	
	if (viv_status_temp_text)
	{
		SetTimer(viv_hwnd,VIV_ID_STATUS_TEMP_TEXT_TIMER,3000,0);
	}
}

static void viv_status_update_temp_pos_zoom(void)
{
	wchar_t wbuf[STRING_SIZE];

	string_printf(wbuf,"Pos %0.3f %0.3f, Zoom %0.3f %0.3f, Aspect Ratio %0.3f",(float)viv_dst_pos_x / 1000.0f,(float)viv_dst_pos_y / 1000.0f,(float)viv_dst_zoom_values[viv_dst_zoom_x_pos],(float)viv_dst_zoom_values[viv_dst_zoom_y_pos],(viv_dst_zoom_values[viv_dst_zoom_x_pos] * (float)viv_image_wide) / (viv_dst_zoom_values[viv_dst_zoom_y_pos] * (float)viv_image_high));
			
	viv_status_set_temp_text(wbuf);
}

static void viv_status_update_temp_animation_rate(void)
{
	wchar_t wbuf[STRING_SIZE];

	string_printf(wbuf,"Animation rate %0.3f",viv_animation_rates[viv_animation_rate_pos]);
			
	viv_status_set_temp_text(wbuf);
}

static void viv_zoom_in(int out,int have_xy,int x,int y)
{
	POINT pt;
	
	if (have_xy)
	{
		pt.x = x;
		pt.y = y;
	}
	else
	{
		RECT rect;
		GetClientRect(viv_hwnd,&rect);
		pt.x = (rect.right - rect.left) / 2;
		pt.y = (rect.bottom - rect.top - viv_get_status_high() - viv_get_controls_high()) / 2;
	}

	ClientToScreen(viv_hwnd,&pt);
	
	SendMessage(viv_hwnd,WM_MOUSEWHEEL,MAKEWPARAM(0,out ? -120 : 120),MAKELPARAM(pt.x,pt.y));
}

static void viv_status_update_slideshow_rate(void)
{
	wchar_t wbuf[STRING_SIZE];
	const wchar_t *res;
	int r;
	
	if ((viv_slideshow_rate / 3600000) && ((viv_slideshow_rate % 3600000) == 0))
	{
		r = viv_slideshow_rate / 3600000;
		res = L"hours";
	}
	else
	if ((viv_slideshow_rate / 60000) && ((viv_slideshow_rate % 60000) == 0))
	{
		r = viv_slideshow_rate / 60000;
		res = L"minutes";
	}
	else
	if ((viv_slideshow_rate / 1000) && ((viv_slideshow_rate % 1000) == 0))
	{
		r = viv_slideshow_rate / 1000;
		res = L"seconds";
	}
	else
	{
		r = viv_slideshow_rate;
		res = L"milliseconds";
	}
	
	string_printf(wbuf,"Slideshow rate %d %S",r,res);
	
	viv_status_set_temp_text(wbuf);
}

static void viv_command_line_options(void)
{
	wchar_t *text_wbuf;
	wchar_t caption_wbuf[STRING_SIZE];
	
	text_wbuf = string_alloc_utf8("Usage:\nvoidImageViewer.exe [/switches] [filename(s)]\n"
		"\n"
		"Switches:\n"
		"/slideshow\tStart a slideshow.\n"
		"/fullscreen\tStart fullscreen.\n"
		"/window\t\tStart windowed.\n"
		"/rate <rate>\tSet the slideshow rate in milliseconds.\n"
		"/name\t\tSort by name.\n"
		"/path\t\tSort by full path and filename.\n"
		"/size\t\tSort by size.\n"
		"/dm\t\tSort by date modified.\n"
		"/dc\t\tSort by date created.\n"
		"/ascending\tSort in ascending order.\n"
		"/descending\tSort in descending order.\n"
		"/<bmp|gif|ico|jpeg|jpg|png|tif|tiff>\n"
		"\t\tInstall association.\n"
		"/no<bmp|gif|ico|jpeg|jpg|png|tif|tiff>\n"
		"\t\tUninstall association.\n"
		"/appdata\t\tSave settings in appdata.\n"
		"/noappdata\tSave settings in exe path.\n"
		"/startmenu\tAdd Start menu shortcuts.\n"
		"/nostartmenu\tRemove Start menu shortcuts.\n"
		"/install <path>\tInstall to the specified path.\n"
		"/install-options <...>\tRun with the specified options after installation.\n"
		"/uninstall <path>\tUninstall from the specified path.\n");
		
	string_copy_utf8(caption_wbuf,"voidImageViewer");

	MessageBox(0,
		text_wbuf,
		caption_wbuf,MB_OK|MB_ICONINFORMATION);
		
	mem_free(text_wbuf);
}

static void viv_install_association(DWORD flags)
{
	int i;
	
	for(i=0;i<VIV_ASSOCIATION_COUNT;i++)
	{
		if (flags & 1 << i)
		{
			viv_install_association_by_extension(viv_association_extensions[i],viv_association_descriptions[i],viv_association_icon_locations[i]);
		}
	}
}

static void viv_uninstall_association(DWORD flags)
{
	int i;
	
	for(i=0;i<VIV_ASSOCIATION_COUNT;i++)
	{
		if (flags & 1 << i)
		{
			viv_uninstall_association_by_extension(viv_association_extensions[i]);
		}
	}
}

static void viv_view_scroll(int mx,int my)
{
	int old_view_x;
	int old_view_y;

	old_view_x = viv_view_x;
	old_view_y = viv_view_y;

	viv_view_set(viv_view_x - mx,viv_view_y - my,0);
	ScrollWindowEx(viv_hwnd,old_view_x - viv_view_x,old_view_y - viv_view_y,0,0,0,0,SW_INVALIDATE);
}

static void viv_update_color_button_bitmap(HWND hwnd)
{	
	HBITMAP hbitmap;
	HDC screen_hdc;
	HDC mem_hdc;
	HGDIOBJ last_hbitmap;
	RECT rect;
	HBRUSH hbrush;
	COLORREF colorref;
	int wide;
	int high;
	
	wide = (64 * os_logical_wide) / 96;
	high = (13 * os_logical_high) / 96;
	
	colorref = (COLORREF)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	
	screen_hdc = GetDC(0);
	mem_hdc = CreateCompatibleDC(screen_hdc);
	
	hbitmap = CreateCompatibleBitmap(screen_hdc,wide,high);

	last_hbitmap = SelectObject(mem_hdc,hbitmap);
	
	hbrush = CreateSolidBrush(colorref);

	rect.left = 1;
	rect.top = 1;
	rect.right = wide-1;
	rect.bottom = high-1;
	FillRect(mem_hdc,&rect,hbrush);
	ExcludeClipRect(mem_hdc,rect.left,rect.top,rect.right,rect.bottom);
	
	rect.left = 0;
	rect.top = 0;
	rect.right = wide;
	rect.bottom = high;
	FillRect(mem_hdc,&rect,(HBRUSH)GetStockObject(BLACK_BRUSH));

	DeleteObject(hbrush);
	
	SelectObject(mem_hdc,last_hbitmap);
	
	DeleteDC(mem_hdc);
	ReleaseDC(0,screen_hdc);
	
	last_hbitmap = (HBITMAP)SendMessage(hwnd,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)hbitmap);
	if (last_hbitmap)
	{
		DeleteObject(last_hbitmap);
	}
}

static void viv_delete_color_button_bitmap(HWND hwnd)
{	
	HGDIOBJ last_hbitmap;
	
	last_hbitmap = (HBITMAP)SendMessage(hwnd,BM_SETIMAGE,IMAGE_BITMAP,0);
	
	if (last_hbitmap)
	{
		DeleteObject(last_hbitmap);
	}
}

static void viv_get_command_name(wchar_t *wbuf,int command_index)
{
	wchar_t menu_wbuf[STRING_SIZE];
	
	*wbuf = 0;
	
	viv_cat_command_menu_path(wbuf,viv_commands[command_index].menu_id);

	viv_get_menu_display_name(menu_wbuf,viv_commands[command_index].name);
	
	string_cat(wbuf,menu_wbuf);
}

static void viv_cat_command_menu_path(wchar_t *wbuf,int menu_index)
{
	int command_index;
	
	if (menu_index == VIV_MENU_ROOT)
	{
		return;
	}
	
	command_index = viv_command_index_from_command_id(menu_index);
	
	if (command_index != -1)
	{
		wchar_t menu_wbuf[STRING_SIZE];
		
		viv_get_menu_display_name(menu_wbuf,viv_commands[command_index].name);
		
		viv_cat_command_menu_path(wbuf,viv_commands[command_index].menu_id);
		
		string_cat(wbuf,menu_wbuf);
		string_cat_utf8(wbuf," | ");
	}
}

static int viv_convert_menu_ini_name_ch(int ch)
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

static utf8_t *viv_cat_command_menu_ini_name_path(utf8_t *buf,int menu_index)
{
	int command_index;
	utf8_t *d;
	
	if (menu_index == VIV_MENU_ROOT)
	{
		return buf;
	}
	
	d = buf;
	
	command_index = viv_command_index_from_command_id(menu_index);
	
	if (command_index != -1)
	{
		const utf8_t *p;
		
		d = viv_cat_command_menu_ini_name_path(d,viv_commands[command_index].menu_id);
		
		p = viv_commands[command_index].name;
		
		while(*p)
		{
			int ch;
			
			ch = viv_convert_menu_ini_name_ch(*p);
			
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
static void viv_menu_name_to_ini_name(utf8_t *buf,int command_index)
{
	utf8_t *d;
	const utf8_t *p;
	
	d = buf;
	p = viv_commands[command_index].name;
	
	d = viv_cat_command_menu_ini_name_path(d,viv_commands[command_index].menu_id);
	
	while(*p)
	{
		int ch;
		
		ch = viv_convert_menu_ini_name_ch(*p);
		
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
}

static void viv_get_menu_display_name(wchar_t *buf,const utf8_t *menu_name)
{
	wchar_t *d;
	const utf8_t  *p;
	
	d = buf;
	p = menu_name;
	
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

static int viv_command_index_from_command_id(int command_id)
{
	int i;
	
	for(i=0;i<VIV_COMMAND_COUNT;i++)
	{
		if (viv_commands[i].command_id == command_id)
		{
			return i;
		}
	}
	
	return -1;
}

static int viv_vk_to_text(wchar_t *wbuf,int vk)
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

static void viv_cat_key_mod(wchar_t *wbuf,int vk,const utf8_t *default_keytext)
{
	wchar_t keytext[STRING_SIZE];
	
	if (!viv_vk_to_text(keytext,vk))
	{
		string_copy_utf8(keytext,default_keytext);
	}
	
	string_cat(wbuf,keytext);
	string_cat_utf8(wbuf,(const utf8_t *)"+");
}

static void viv_get_key_text(wchar_t *wbuf,DWORD keyflags)
{
	wchar_t keytext[STRING_SIZE];
	
	*wbuf = 0;
	
	if (keyflags & VIV_KEYFLAG_CTRL)
	{
		viv_cat_key_mod(wbuf,VK_CONTROL,"Ctrl");
	}
	
	if (keyflags & VIV_KEYFLAG_ALT)
	{
		viv_cat_key_mod(wbuf,VK_MENU,"Alt");
	}
	
	if (keyflags & VIV_KEYFLAG_SHIFT)
	{
		viv_cat_key_mod(wbuf,VK_SHIFT,"Shift");
	}
	
	if (viv_vk_to_text(keytext,keyflags & VIV_KEYFLAG_VK_MASK))
	{
		string_cat(wbuf,keytext);
	}
}

static HMENU viv_create_menu(void)
{
	HMENU hmenu;
	
	hmenu = CreateMenu();
	
	{
		int i;
		HMENU menus[VIV_MENU_COUNT];
		
		for(i=1;i<VIV_MENU_COUNT;i++)
		{
			menus[i] = 0;
		}
		menus[0] = hmenu;
		
		for(i=0;i<VIV_COMMAND_COUNT;i++)
		{
			if (!(viv_commands[i].flags & MF_OWNERDRAW))
			{
				wchar_t text_wbuf[STRING_SIZE];
				
				string_copy_utf8(text_wbuf,viv_commands[i].name);

				if (viv_commands[i].flags & MF_POPUP)
				{
					if (!menus[viv_commands[i].command_id])
					{
						menus[viv_commands[i].command_id] = CreatePopupMenu();
						
						if (viv_commands[i].command_id == VIV_MENU_NAVIGATE_PLAYLIST)
						{
							viv_nav_playlist_menu = menus[viv_commands[i].command_id];
						}
					}
					
					AppendMenu(menus[viv_commands[i].menu_id],viv_commands[i].flags,(UINT_PTR)menus[viv_commands[i].command_id],text_wbuf);
				}
				else
				{
					wchar_t key_text[STRING_SIZE];
					
					if (viv_key_list->start[i])
					{
						viv_get_key_text(key_text,viv_key_list->start[i]->key);
						
						string_cat_utf8(text_wbuf,(const utf8_t *)"\t");
						string_cat(text_wbuf,key_text);
					}
					
					AppendMenu(menus[viv_commands[i].menu_id],viv_commands[i].flags,viv_commands[i].command_id,text_wbuf);
				}
			}
		}
	}
	
	return hmenu;
}

static void viv_key_add(viv_key_list_t *key_list,int command_index,DWORD keyflags)
{	
	viv_key_t *key;
	
	key = mem_alloc(sizeof(viv_key_t));
	
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

static void viv_key_clear_all(viv_key_list_t *key_list,int command_index)
{	
	viv_key_t *key;
	
	key = key_list->start[command_index];
	
	while(key)
	{
		viv_key_t *next_key;
		
		next_key = key->next;
		
		mem_free(key);
		
		key = next_key;
	}								

	key_list->start[command_index] = 0;
	key_list->last[command_index] = 0;
}

static void viv_key_list_copy(viv_key_list_t *dst,const viv_key_list_t *src)
{
	int i;

	viv_key_list_clear(dst);
	
	for(i=0;i<VIV_COMMAND_COUNT;i++)
	{
		viv_key_t *key;

		key = src->start[i];
		
		while(key)
		{
			viv_key_add(dst,i,key->key);
			
			key = key->next;
		}
	}
}

static void viv_key_list_clear(viv_key_list_t *list)
{
	// free keys
	int i;
	
	for(i=0;i<VIV_COMMAND_COUNT;i++)
	{
		viv_key_clear_all(list,i);
	}
}

static void viv_key_list_init(viv_key_list_t *list)
{
	// free keys
	int i;
	
	for(i=0;i<VIV_COMMAND_COUNT;i++)
	{
		list->start[i] = 0;
		list->last[i] = 0;
	}
}

static LRESULT CALLBACK _viv_edit_key_edit_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	WNDPROC last_proc;
	
	last_proc = (WNDPROC)GetWindowLongPtr(hwnd,GWLP_USERDATA);
		
	switch(msg)
	{
		case WM_GETDLGCODE:
			return DLGC_WANTALLKEYS;
			
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			int key_flags;
			int vk;
			
			key_flags = viv_get_current_key_mod_flags();
			vk = wParam;
			
			switch(vk)
			{
				case VK_CONTROL:
				case VK_SHIFT:
				case VK_MENU:
				case VK_LWIN:
				case VK_RWIN:
					return 0;

	//FIXME:						
	//					case VK_PROCESSKEY:
	//						// restore IME hacked key.
	//						vk = ImmGetVirtualKey(msg->hwnd);
	//						break;
			}

			viv_edit_key_set_key(hwnd,key_flags | vk);
			viv_options_edit_key_changed(GetParent(hwnd));

			return 0;
		}
	}
		
	if ((msg == WM_LBUTTONDOWN) || (msg == WM_RBUTTONDOWN))
	{
		SetFocus(hwnd);
		
		return 0;
	}
	
	// ignore mouse messages.
	if ((msg >= WM_MOUSEFIRST) && (msg <= WM_MOUSELAST))
	{
		return 0;
	}

	// ignore keyboard messages.
	if ((msg >= WM_KEYFIRST) && (msg <= WM_KEYLAST))
	{
		return 0;
	}
			
	return CallWindowProc(last_proc,hwnd,msg,wParam,lParam);
}

static int viv_get_current_key_mod_flags(void)
{
	int key_flags;
	
	key_flags = 0;
	
	if (GetKeyState(VK_CONTROL) < 0)
	{
		key_flags |= VIV_KEYFLAG_CTRL;
	}
	
	if (GetKeyState(VK_SHIFT) < 0)
	{
		key_flags |= VIV_KEYFLAG_SHIFT;
	}
	
	if (GetKeyState(VK_MENU) < 0)
	{
		key_flags |= VIV_KEYFLAG_ALT;
	}
	
	return key_flags;
}	

static void viv_options_edit_key_changed(HWND hwnd)
{
	int keyflags;
	int command_index;
	
	keyflags = GetWindowLongPtr(hwnd,GWLP_USERDATA);
	
	ListBox_ResetContent(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST));
	SetWindowRedraw(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST),FALSE);
	
	for(command_index=0;command_index<VIV_COMMAND_COUNT;command_index++)
	{
		viv_key_t *key;
		
		key = viv_key_list->start[command_index];
		
		while(key)
		{
			if (key->key == keyflags)
			{
				wchar_t command_wbuf[STRING_SIZE];
				
				viv_get_command_name(command_wbuf,command_index);
				
				ListBox_AddString(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST),command_wbuf);
			}
		
			key = key->next;
		}
	}

	ListBox_SetCurSel(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST),0);

	SetWindowRedraw(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST),TRUE);
}

static void viv_edit_key_set_key(HWND hwnd,DWORD key_flags)
{
	wchar_t keytext_wbuf[STRING_SIZE];

	if (key_flags)
	{
		viv_get_key_text(keytext_wbuf,key_flags);
	}
	else
	{
		keytext_wbuf[0] = 0;					
		SetWindowLongPtr(GetParent(hwnd),GWLP_USERDATA,0);
	}

	SetWindowLongPtr(GetParent(hwnd),GWLP_USERDATA,key_flags);

	SetWindowText(hwnd,keytext_wbuf);

	os_edit_move_caret_to_end(hwnd);
}

static void viv_key_remove(viv_key_list_t *keylist,int command_index,DWORD keyflags)
{
	viv_key_t *key;
	
	key = keylist->start[command_index];
	
	keylist->start[command_index] = 0;
	keylist->last[command_index] = 0;
	
	while(key)
	{
		viv_key_t *next_key;
		
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

static void viv_edit_key_remove_currently_used_by(viv_key_list_t *keylist,DWORD keyflags)
{
	int command_index;
	
	for(command_index=0;command_index<VIV_COMMAND_COUNT;command_index++)
	{
		viv_key_remove(keylist,command_index,keyflags);
	}
}

//FIXME: we should timeout 
//FIXME: we should check for the process name voidImageViewer.exe rather than the window class name. -be careful when uninstalling as the non-admin process will be waiting for the admin process to exit.
static void viv_close_existing_process(void)
{
	for(;;)
	{
		HWND hwnd;
		
		hwnd = FindWindowA("VOIDIMAGEVIEWER",0);
		if (hwnd)
		{
			DWORD process_id;
			HANDLE process_handle;
			
			SendMessage(hwnd,WM_CLOSE,0,0);
			
			GetWindowThreadProcessId(hwnd,&process_id);
			
			process_handle = OpenProcess(SYNCHRONIZE,FALSE,process_id);
			if (process_handle)
			{
				WaitForSingleObject(process_handle,INFINITE);
			
				CloseHandle(process_handle);
			}
		}
		else
		{
			// no window open
			break;
		}
	}
}

static int viv_get_appdata_voidimageviewer_path(wchar_t *wbuf)
{
	wchar_t appdata_wbuf[STRING_SIZE];
	
	if (string_get_appdata_path(appdata_wbuf))
	{
		string_path_combine_utf8(wbuf,appdata_wbuf,(const utf8_t *)"voidImageViewer");

		return 1;
	}
	
	return 0;
}

static void viv_uninstall_delete_file(const wchar_t *path,const utf8_t *filename)
{
	wchar_t full_path_and_filename[STRING_SIZE];
	
	string_path_combine_utf8(full_path_and_filename,path,filename);
	
	DeleteFile(full_path_and_filename);
}

static int viv_is_start_menu_shortcuts(void)
{
	wchar_t special_folder_path_wbuf[STRING_SIZE];
	wchar_t path_wbuf[STRING_SIZE];
	int ret;
	
	ret = 0;

	// delete old english shortcuts
	// delete shortcuts
	if (os_get_special_folder_path(special_folder_path_wbuf,CSIDL_COMMON_PROGRAMS))
	{
		string_path_combine_utf8(path_wbuf,special_folder_path_wbuf,(const utf8_t *)"void Image Viewer");

		// make sure this directory exists!		
		if (GetFileAttributesW(path_wbuf) != INVALID_FILE_ATTRIBUTES)
		{
			// at least one item exists, ret = 2
			ret = 1;
		}
	}

	return ret;
}

static void viv_install_start_menu_shortcuts(void)
{
	wchar_t special_folder_path_wbuf[STRING_SIZE];
	
	// always uninstall and reinstall
	// this allows us to switch between english and another language.
	viv_uninstall_start_menu_shortcuts();

	// create shortcuts
	if (os_get_special_folder_path(special_folder_path_wbuf,CSIDL_COMMON_PROGRAMS))
	{
		wchar_t path_wbuf[STRING_SIZE];
		wchar_t exe_filename_wbuf[STRING_SIZE];
		wchar_t exe_path_wbuf[STRING_SIZE];
		wchar_t uninstall_filename_wbuf[STRING_SIZE];
		wchar_t lnk_wbuf[STRING_SIZE];
		
		string_path_combine_utf8(path_wbuf,special_folder_path_wbuf,(const utf8_t *)"void Image Viewer");

		// make sure this directory exists!		
		os_make_sure_path_exists(path_wbuf);

		// void image viewer.lnk
		GetModuleFileName(NULL,exe_filename_wbuf,STRING_SIZE);
		string_path_combine_utf8(lnk_wbuf,path_wbuf,"void Image Viewer.lnk");
		os_create_shell_link(exe_filename_wbuf,lnk_wbuf);
	
		// uninstall
		string_get_path_part(exe_path_wbuf,exe_filename_wbuf);
		string_path_combine_utf8(uninstall_filename_wbuf,exe_path_wbuf,(const utf8_t *)"Uninstall.exe");

		string_path_combine_utf8(lnk_wbuf,path_wbuf,"Uninstall.lnk");
		os_create_shell_link(uninstall_filename_wbuf,lnk_wbuf);
	}
}

static void viv_uninstall_start_menu_shortcuts(void)
{
	wchar_t special_folder_path_wbuf[STRING_SIZE];
	wchar_t path_wbuf[STRING_SIZE];

	// delete old english shortcuts
	// delete shortcuts
	if (os_get_special_folder_path(special_folder_path_wbuf,CSIDL_COMMON_PROGRAMS))
	{
		wchar_t lnk_wbuf[STRING_SIZE];

		string_path_combine_utf8(path_wbuf,special_folder_path_wbuf,(const utf8_t *)"void Image Viewer");
		
		// localized.
		string_path_combine_utf8(lnk_wbuf,path_wbuf,"void Image Viewer.lnk");
		DeleteFile(lnk_wbuf);
		
		string_path_combine_utf8(lnk_wbuf,path_wbuf,"Uninstall.lnk");
		DeleteFile(lnk_wbuf);
		
		RemoveDirectory(path_wbuf);
	}
}

static void viv_append_admin_param(wchar_t *wbuf,const utf8_t *param)
{
	string_cat_utf8(wbuf,(const utf8_t *)" /");
	string_cat_utf8(wbuf,param);
}

static void viv_install_copy_file(const wchar_t *install_path,const wchar_t *temp_path,const utf8_t *filename,int critical)
{
	wchar_t dst_wbuf[STRING_SIZE];
	wchar_t src_wbuf[STRING_SIZE];
	
	string_path_combine_utf8(dst_wbuf,install_path,filename);
	string_path_combine_utf8(src_wbuf,temp_path,filename);
	
	if (!CopyFile(src_wbuf,dst_wbuf,FALSE))
	{
		if (critical)
		{
			debug_fatal("Error %d: unable to copy %S to %S",GetLastError(),src_wbuf,dst_wbuf);
		}
	}	
}

