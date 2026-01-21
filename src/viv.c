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

// TODO:
// .rc localization
// fix controls in view options page being 1 pixel too long
// fix jump-to up/down focus
// SVG support -Librsvg?
// GDI/GDI+ is limited to 2GB (wide*high*4), work out a workaround for large images.
// add UI option for config_title_bar_format.
// compile on mingw
// option to show full path like MPC
// Undo option, after delete, undo the delete and re-add the image to the playlist.
// delete crashes on win9x, might indicate a deeper issue..
// fix horrible screen buffer mangling by Windows when resizing the window or auto fitting the window.
// msi installer
// ARM/ARM64 installer
// install for current user only option, install to %LocalAppData%\Programs
// dark mode (nothing in viv has Microsoft dark theme support -I will have to render ALL controls myself)
// Use Direct3D to render images when shrinking.
// use sort order from Windows Explorer folder.
// Copy the zoomed part of the image to another buffer and stretch that to avoid gdi driver issues when zooming in really close with large images.
// - get image width/length via IPC
// - get/set viv display area width/length via IPC
// Support piping of image data for ImageMagic support
// - set/get zoom level 
// - change zoom in/out level in 1/10/100 percent steps
// - auto zoom levels: always fit to width, always fit to height, zoom inside (fit to width or height so that still the whole image is shown), zoom outside (fit to width or height so that the window is fully filled)
// - option to keep the custom zoom level while image displayed changed
// - by holding left mouse button on displayed image and moving the mouse, move viv window when using "zoom inside" mode, else move image inside viv window
// - keyboard shortcut Ctrl+C to copy viv display area to clipboard and Ctrl+V to paste the image from clipboard and display it.
// - I use ImageMagick's convert.exe and GraphicsMagick's gm.exe tools for color correction an image sharpening. I need to pipe out the image showing in viv to those apps and pipe in the output of those apps to viv and display the processed image without writing to disk, i.e. viv write to STOUT and read from STDIN.
// - may be there could be an option in viv that the user just provide the executable names and the command line parameters that optionally would be executed whenever the file displayed changes. - some sort of multiple instances setting, eg: viv.exe -no-new-instance -other command line arguments... -could also support named instances
// - set/get file name of displayed image. Considering when clipboard/STDIN is displayed, it would be nice to still be able to get the name of the file that was displayed before showing the clipboard/STDIN.
// - auto update the image displayed when the image on disk (or the clipboard) has changed
// - next/previous image with option to show files in subfolders. Considering when clipboard/STDIN is displayed, the base image would be the file that was displayed before showing the clipboard/STDIN.
// - border less window with retractable title bar
// - dark skin (use system theme)
// - snap viv window to other windows and the monitor borders
// - no minimum viv window size restriction
// - open/edit image with another app
// - color correction, white balance, sharpening
// - == mehdi
// create a playlist file format (aka an album of images)
// Ken Burns Effect Slideshows with FFMPeg -stamimail -https://el-tramo.be/blog/ken-burns-ffmpeg/
// open a file with the filename clipboard: to open the clipboard
// open a file with the filename stdin: to open stdin
// Check we are using ICC
// show main window on monitor that the cursor is currently on, like MPC-HC.
// remove GetFileAttributesEx or replace with GetFileAttributes..
// Ctrl + V to paste image from the clipboard into voidImageViewer??
// a touch window from inside option
// add support for APNG
// A Play All Instances option that plays/pause all instances
// keyboard shortcut to toggle Everything randomize.
// middle mouse action, scroll and control slideshow speed
// make VIV more aware of other VIV windows for improved tile support.. cascade etc..
// OpenGL renderer
// Direct3D renderer
// graphics::GetHalftonePalette for 256 color mode.
// high dpi icons
// control toolbar customization
// install bmp/jpg only if the default value for HKEY_CLASSES_ROOT\.bmp is bmpfile or voidImageViewer.bmpfile -don't replace non default ones. default hard to determine for each version of Windows -avoiding for now.
// string table for localization.
// right click -> open with ...open with, or rather get a proper context menu. CDefFolderMenu_Create2
// keep window aspect size option
// generate a shuffle list of indexes for the Everything randomize option.
// image playlists. m3u? efu? -command line option to load a list of filenames from a txt/efu file lists.
// shift + Ctrl + Numpad arrow keys for faster/slower movement
// when panning the image, clamp to the image edge, instead of the image center.
// paste dib from clipboard CF_DIB
// therube: Just to note...  Something like: voidImageViewer.exe "\my documents" or voidImageViewer.exe "\my documents\"  , will load "images" found in the \my documents\ directory.  Though somethig like: voidImageViewer.exe "\my documents\*" or voidImageViewer.exe "\my documents\*.*"  will load (I suppose it is) ALL images on your computer. voidImageViewer.exe "\my documents\*.jpg" works as expected. 
// maintain correct image aspect ratio when window is clipped on auto size.
// if we have a small image 64x64 and zoom right in, the image still fits inside our window -if we then go fullscreen the image is massive, we should check if Fill Window is triggered and disable the zoom in fullscreen mode.
//
// DONE:
// *deleting the last image in a playlist does not clear the image.
// *added xbutton action
// *dont invalidate when zooming out when we are already zoomed out the most.
// *when keep aspect ratio is off, zooming in should use the bad aspect otherwise aspect gradually changes to same aspect.
// *added ctrl mousewheel action
// *fixed an issue when only one image was in the playlist.
// *fixed a leak when loading an image.
// *fixed a crash when there was only one shuffled image.
// *improved randomize seeding
// *fixed a leak when loading an image while already loading an image.
// *rotate option -using verbs
// *added /x /y /width /height /minimal /compact command line options.
// *fixed /dc command line option
// *j - jump to list - replaces navigate menu.
// *everything search
// *added f2 rename.
// *current image is added to playlist when adding an image to the playlist and the playlist is empty.
// *fix stream for gifs, don't load the entire image before loading the gif. _viv_istream_t hurts loading performance, will leave CreateStreamOnHGlobal for now.
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
// *Clear ΓÇô This command needs a Hotkey (Maybe Crtl+W like in Word), and Dialog Box to approve. -added as close ctrl + W
// *Write the Dimensions (weight height) of each pic and the speed of animation.
// *Voidimageviewer has a nice feature of controlling the animation rate (speed), to make it fast/slower.
// *open clipboard to view image in clipboard? -no must be file based for now
// *Mousewheel up/down ΓÇô Most of image viewers use these keys for one of two actions: 1. Next image / Previous image 2. Zoom in / Zoom out -keyboard shortcut to toggle between the two.
// *print -using preview to print ..
// *play gifs atleast once.
// *separate setting for filter quality when fullscreen/windowed. -why?
// *set as desktop background
// *SetThreadExecutionState (prevent sleep) - SC_MONITORPOWER and SC_SCREENSAVE work.
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
// *why does webp use soo much RAM when gif doesn't use any? -because we used a memory bitmap instead of a compatible bitmap -images now stored in video RAM.
// *mipmaps for shrink resizing.. could generate mipmaps on load or as needed. -too slow. maybe do this on resize only. -find a way to do this efficiently
// *add right click Exit
// *Ctrl + Shift + C = Copy filename / *add an option to copy image path ctrl + shift +c
// *Copy "Image" context menu item. -users don't want to copy the file, they want the image. -they don't want to copy meta data. -we were copying the image too with copy, but we now have a separate option to copy just the image.
// 1.0.0.11
// *rotate now rotates mipmaps correctly.
// *view -> Menu setting not remembered. -added caption and thickframe too.
// *exit in right click menu
// *rate up/down was upside down
// *option to keep last viewed image in cache.
// *when going back (left arrow key), start preloading the previous item instead.
// *F5 = reload current image.
// *fixed a crash when interrupting the loading of a multi-frame image.
// *dragging the status bar should move the window.
// *fix going fullscreen and using the window zoom, if zoomed in and we go fullscreen the zoom is too much (try a small image to see)
// *opening a non existing filename should show an error message: "File Not Found". eg: viv.exe "c:\non-existing-folder\foo.jpg" -also added "failed to load image" message for bad images.
// *hide mouse on hover.
// *using wrong registry key in _viv_install_association_by_extension.
// *vs2005 and vs2019 solutions in separate dirs.
// *remove crt.c and other unused c files.
// 1.0.0.12
// *fixed a gdi leak when freeing mipmaps
// *fixed a gdi leak when refreshing.
// *fixed changing sort loading the wrong next image.
// *added natural sort
// 1.0.0.13
// *add left click action: move window
// *large images lose center on resize. -typo in (high - rw)
// *I will add an option to auto resize the window to show the full image (50%/100%/200%) when a new image is shown. -already had option -added GUI option and improve autofit
// *break the redraw region down into rects and only stretch what we need to. -SelectClipRgn works well when using halftone.
// *back/forward mouse buttons dont work on toolbars
// *does viv update correct when we drop a folder on viv and then delete an image from that folder? does undelete show the file? -removed preload check so we always load the next image from disk, in case preload is stale. -dropping a folder creates a playlist, we will not monitor folders and we already delete from the playlist when deleting from viv.
// *don't bother preloading if the next image is the last image.
// 1.0.0.14
// *break POS and RGB into separate parts
// *allow xbuttons to repeat -noone does this.
// *move window action: I will trial scrolling if zoomed, then fallback to move window.
// *orientation metadata for rotation.
// *pixel info doesn't work if shrinking. -we incorrectly get the pixel from the mipmap.
// *webp alpha using wrong background color 
// *re-enabled WEBP_MSC_SSE41 
// *-switches -don't just use /switch, allow -switch command line options.
// *fixed rotating the wrong way.
// *correct tracking of center pixel when zooming/resizing.
// *when going full screen, check if the image is currently smaller than it would be full screen, if it is, set a flag to restore the zoom, save the zoom and set the zoom to 0.
// *D:\Misc\Game Data\Aladdin\Aladdin-RugRide.png doesn't render correctly on some zooms 85129x224 -converted int to __int64
// *check the viv icon is being used for webp. -it does, but you have to open a webp file first and allow voidimageviewer to open.
// *prevent screen saver on slideshow
// 1.0.0.15
// *fixed _viv_dst_pos_x/_viv_dst_pos_y (doubled offsets to match mpchc)
// *zooming in AND numpad+6 zooming breaks for largish images. (black bars on scrolling) -pan and zoom was overflowing, set stitch size to 512 to fix.
// In "1:1" mode, use shortcut keys to move a big picture(larger than the screen resolution), it will stop halfway. Perhaps it has something to do with the movement of the small images? -mpc does the whole image.

#define _VIV_WM_REPLY							(WM_USER+1)
#define _VIV_WM_RETRY_RANDOM_EVERYTHING_SEARCH	(WM_USER+2)

#define _VIV_ASSOCIATION_BMP				0x00000001
#define _VIV_ASSOCIATION_GIF				0x00000002
#define _VIV_ASSOCIATION_ICO				0x00000004
#define _VIV_ASSOCIATION_JPEG				0x00000008
#define _VIV_ASSOCIATION_JPG				0x00000010
#define _VIV_ASSOCIATION_PNG				0x00000020
#define _VIV_ASSOCIATION_TIF				0x00000040
#define _VIV_ASSOCIATION_TIFF				0x00000080
#define _VIV_ASSOCIATION_WEBP				0x00000100

#define _VIV_ZOOM_MAX 16

#define BCM_SETSHIELD	0x0000160C

#ifdef VERSION_X64
	#define VERSION_TARGET_MACHINE "(x64)"
#else
	#ifdef VERSION_ARM
		#define VERSION_TARGET_MACHINE "(ARM)"
	#else
		#ifdef VERSION_ARM64
			#define VERSION_TARGET_MACHINE "(ARM64)"
		#else
			#define VERSION_TARGET_MACHINE "(x86)"
		#endif
	#endif
#endif

#define _VIV_DEFAULT_SHUFFLE_ALLOCATED		(65536 / sizeof(_viv_playlist_t *))

#define VIV_YEAR_STRING2(x)	#x
#define VIV_YEAR_STRING(x)	VIV_YEAR_STRING2(x)	

// _VIV_STRETCH_BLT_STITCH_SIZE * 3.1 (pan+zoom) * 16 (zoom) MUST BE < 32768
#define _VIV_STRETCH_BLT_STITCH_SIZE		512

#include "viv.h"

enum
{
	_VIV_COPYDATA_COMMAND_LINE,
	_VIV_COPYDATA_OPEN_EVERYTHING_SEARCH,
	_VIV_COPYDATA_ADD_EVERYTHING_SEARCH,
	_VIV_COPYDATA_RANDOM_EVERYTHING_SEARCH,
};

enum
{
	_VIV_DOING_NOTHING,
	_VIV_DOING_SCROLL,
	_VIV_DOING_MSCROLL,
	_VIV_DOING_1TO1SCROLL,
};

enum
{
	_VIV_REPLY_LOAD_IMAGE_COMPLETE = 0,
	_VIV_REPLY_LOAD_IMAGE_FAILED,
	_VIV_REPLY_LOAD_IMAGE_FIRST_FRAME,
	_VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME,
};

enum
{
	_VIV_MENU_ROOT=0,
	_VIV_MENU_FILE,
	_VIV_MENU_EDIT,
	_VIV_MENU_VIEW,
	_VIV_MENU_VIEW_PRESET,
	_VIV_MENU_VIEW_WINDOW_SIZE,
	_VIV_MENU_VIEW_PANSCAN,
	_VIV_MENU_VIEW_ZOOM,
	_VIV_MENU_VIEW_ONTOP,
	_VIV_MENU_SLIDESHOW,
	_VIV_MENU_SLIDESHOW_RATE,
	_VIV_MENU_ANIMATION,
	_VIV_MENU_NAVIGATE,
	_VIV_MENU_NAVIGATE_SORT,
	_VIV_MENU_NAVIGATE_PLAYLIST,
	_VIV_MENU_HELP,
	_VIV_MENU_COUNT,
};

#define _VIV_HIDE_CURSOR_DELAY		2000

// a file descriptor or find data
// to describe the image.
/*
typedef struct _viv_fd_s
{
	unsigned __int64 id; // unique playlist id.
	unsigned __int64 date_modified; 
	unsigned __int64 size;
	unsigned __int64 date_created;
	
	// filename follows.
	// utf8_t filename[...];
	
}_viv_fd_t;
*/
// a reply from the image load thread

typedef struct _viv_reply_s
{
	struct _viv_reply_s *next;
	DWORD type;
	DWORD size;
	// data follows
	
}_viv_reply_t;

// a mipmap level
typedef struct _viv_mip_s
{
//	int wide;
//	int high;
	HBITMAP hbitmap;
	struct _viv_mip_s *mipmap;
	
}_viv_mipmap_t;

// a frame in the image.
// there might be more than one.

typedef struct _viv_frame_s
{
	HBITMAP hbitmap;
	
	// next mipmap level.
	// NULL if not computed.
	_viv_mipmap_t *mipmap;
	
	// frame delay in milliseconds.
	DWORD delay;
	
}_viv_frame_t;

typedef struct _viv_reply_load_image_first_frame_s
{
	UINT wide;
	UINT high;
	UINT frame_count;
	_viv_frame_t frame;
	
}_viv_reply_load_image_first_frame_t;

typedef struct _viv_playlist_s
{
	struct _viv_playlist_s *next;
	struct _viv_playlist_s *prev;
	WIN32_FIND_DATA fd;
	
}_viv_playlist_t;

typedef struct _viv_command_s
{
	localization_id_t localization_id;
	WORD flags;
	BYTE menu_id;
	WORD command_id;
}_viv_command_t;

typedef struct _viv_default_key_s
{
	WORD command_id;
	WORD key_flags;
	
}_viv_default_key_t;

typedef struct _viv_nav_item_s
{
	WIN32_FIND_DATA fd;
	struct _viv_nav_item_s *next;
	
}_viv_nav_item_t;

typedef struct _viv_webp_s
{
	DWORD wide;
	DWORD high;
	HDC screen_hdc;
	HDC mem_hdc;
	int has_alpha;
	int frame_index;
	DWORD frame_count;
	DWORD last_delay;
	int orientation;
	
}_viv_webp_t;

static void _viv_update_title(void);
static void _viv_on_size(void);
static LRESULT CALLBACK _viv_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static LRESULT CALLBACK _viv_fullscreen_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void _viv_command_with_is_key_repeat(int command,int is_key_repeat);
static void _viv_command(int command);
static int _viv_process_install_command_line_options(wchar_t *cl);
static void _viv_process_command_line(wchar_t *cl);
static int _viv_init(int nCmdShow);
static void _viv_kill(void);
static void _viv_exit(void);
static int _viv_next(int prev,int reset_slideshow_timer,int is_preload,int wait_for_current_load);
static void _viv_home(int end,int is_preload);
static int _viv_is_valid_filename(WIN32_FIND_DATA *fd);
static int _viv_is_msg(MSG *msg);
static void _viv_view_set(int view_x,int view_y,int invalidate);
static void _viv_slideshow(void);
static void _viv_toggle_fullscreen(void);
static void _viv_get_render_size(int *prw,int *prh);
static void _viv_set_custom_rate(void);
static void _viv_set_rate(int rate);
static void _viv_rename(void);
static void _viv_delete(int permanently);
static void _viv_copy(int cut);
static void _viv_copy_filename(void);
static void _viv_set_clipboard_image(void);
static void _viv_copy_image(void);
static int _viv_is_key_state(int control,int shift,int alt);
static CLIPFORMAT _viv_get_CF_PREFERREDDROPEFFECT(void);
static void _viv_pause(void);
static int _viv_fd_compare(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b);
static void _viv_open(WIN32_FIND_DATA *fd,int is_preload);
static BOOL _viv_open_from_filename(const wchar_t *filename);
static void _viv_increase_rate(int dec);
static void _viv_file_preview(void);
static void _viv_file_print(void);
static void _viv_file_set_desktop_wallpaper(void);
static void _viv_edit_rotate(int counterclockwise);
static void _viv_file_edit(void);
static void _viv_open_file_location(void);
static void _viv_properties(void);
static void _viv_doing_cancel(void);
static const char *_viv_get_copydata_string(const char *p,const char *e,wchar_t *buf,int bufsize);
static void _viv_blank(void);
static void _viv_options(void);
static INT_PTR CALLBACK _viv_options_proc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK _viv_options_general_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK _viv_options_view_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK _viv_options_controls_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void _viv_install_association_by_extension(const char *association,const char *description,const char *icon_location);
static void _viv_uninstall_association_by_extension(const char *association);
static int _viv_is_association(const char *association);
static int _viv_get_registry_string(HKEY hkey,const utf8_t *value,wchar_t *wbuf,int size_in_wchars);
static int _viv_set_registry_string(HKEY hkey,const utf8_t *value,const wchar_t *wbuf);
static void _viv_check_menus(HMENU hmenu);
static void _viv_increase_animation_rate(int dec);
static void _viv_reset_animation_rate(void);
static void _viv_timer_start(void);
static void _viv_mousemove(void);
static void _viv_update_src_pixel(int force,int update_statusbar);
static void _viv_animation_pause(void);
static void _viv_frame_step(void);
static void _viv_frame_prev(void);
static void _viv_view_1to1(void);
static void _viv_playlist_clearall(void);
static void _viv_playlist_add_current_if_empty(void);
static _viv_playlist_t *_viv_playlist_add(const WIN32_FIND_DATA *fd);
static void _viv_playlist_add_path(const wchar_t *full_path_and_filename);
static void _viv_playlist_add_filename(const wchar_t *filename);
static void _viv_playlist_delete(const WIN32_FIND_DATA *fd);
static void _viv_playlist_rename(const wchar_t *old_filename,const wchar_t *new_filename);
static void _viv_timer_stop(void);
static int _viv_is_window_maximized(HWND hwnd);
static INT_PTR CALLBACK _viv_custom_rate_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK _viv_about_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void _viv_update_frame(void);
static void _viv_update_ontop(void);
static void _viv_update_prevent_sleep(void);
static void _viv_dst_pos_set(int x,int y);
static void _viv_dst_zoom_set(int x,int y);
static void _viv_frame_skip(int size);
static DWORD WINAPI _viv_load_image_thread_proc(void *param);
static _viv_reply_t *_viv_reply_add(DWORD type,DWORD size,void *data);
static void _viv_reply_clear_all(void);
static void _viv_reply_free(_viv_reply_t *e);
static void _viv_controls_show(int show);
static void _viv_status_show(int show);
static void _viv_status_update(void);
static void _viv_status_set(int part,const wchar_t *text);
static int _viv_get_status_high(void);
static int _viv_get_controls_high(void);
static LRESULT CALLBACK _viv_rebar_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static LRESULT CALLBACK _viv_status_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static int _viv_toolbar_get_wide(void);
static void _viv_toolbar_update_buttons(void);
static void _viv_status_set_temp_text(wchar_t *text);
static void _viv_status_update_temp_pos_zoom(void);
static void _viv_status_update_temp_animation_rate(void);
static void _viv_zoom_in(int out,int have_xy,int x,int y);
static void _viv_status_update_slideshow_rate(void);
static void _viv_options_treeview_changed(HWND hwnd);
static void _viv_command_line_options(void);
static void _viv_install_association(DWORD flags);
static void _viv_uninstall_association(DWORD flags);
static void _viv_view_scroll(int mx,int my);
static void _viv_options_update_sheild(HWND hwnd);
static void _viv_update_color_button_bitmap(HWND hwnd);
static void _viv_delete_color_button_bitmap(HWND hwnd);
static int _viv_convert_menu_ini_name_ch(int ch);
static utf8_t *_viv_cat_command_menu_ini_name_path(utf8_t *buf,int menu_index);
static void _viv_get_menu_display_name(wchar_t *buf,const utf8_t *menu_name);
static void _viv_get_command_name(wchar_t *wbuf,int command_index);
static int _viv_command_index_from_command_id(int command_id);
static void _viv_cat_command_menu_path(wchar_t *wbuf,int menu_index);
static int _viv_vk_to_text(wchar_t *wbuf,int vk);
static void _viv_cat_key_mod(wchar_t *wbuf,int vk,const utf8_t *default_keytext);
static void _viv_get_key_text(wchar_t *wbuf,DWORD keyflags);
static HMENU _viv_create_menu(void);
static void _viv_key_add(struct _viv_key_list_s *key_list,int command_index,DWORD keyflags);
static void _viv_key_clear(struct _viv_key_list_s *key_list,int command_index);
static void _viv_key_list_copy(struct _viv_key_list_s *dst,const struct _viv_key_list_s *src);
static void _viv_key_clear_all(struct _viv_key_list_s *list);
static void _viv_key_list_init(struct _viv_key_list_s *list);
static void _viv_options_key_list_sel_change(HWND hwnd,int previous_key_index);
static void _viv_options_remove_key(HWND hwnd);
static void _viv_options_edit_key(HWND hwnd,int key_index);
static INT_PTR CALLBACK _viv_edit_key_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static LRESULT CALLBACK _viv_edit_key_edit_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static int _viv_get_current_key_mod_flags(void);
static void _viv_options_edit_key_changed(HWND hwnd);
static void _viv_edit_key_set_key(HWND hwnd,DWORD key_flags);
static void _viv_key_remove(struct _viv_key_list_s *keylist,int command_index,DWORD keyflags);
static void _viv_edit_key_remove_currently_used_by(struct _viv_key_list_s *keylist,DWORD keyflags);
static void _viv_close_existing_process(void);
static void _viv_uninstall_delete_file(const wchar_t *path,const utf8_t *filename);
static int _viv_is_start_menu_shortcuts(void);
static void _viv_install_start_menu_shortcuts(void);
static void _viv_uninstall_start_menu_shortcuts(void);
static void _viv_append_admin_param(wchar_t *wbuf,const utf8_t *param);
static void _viv_install_copy_file(const wchar_t *install_path,const wchar_t *temp_path,const utf8_t *filename,int critical);
static INT_PTR CALLBACK _viv_rename_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void _viv_shuffle_playlist(void);
static void _viv_show_jumpto(void);
static LRESULT CALLBACK _viv_jumpto_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void _viv_jumpto_on_size(HWND hwnd);
static void _viv_jumpto_on_search(HWND hwnd);
static void _viv_jumpto_open_sel(HWND hwnd);
static void _viv_nav_item_free_all(void);
static void _viv_nav_item_add(WIN32_FIND_DATA *fd);
static int _viv_nav_compare(const _viv_nav_item_t *a,const _viv_nav_item_t *b);
static void _viv_add_current_path_to_playlist(void);
static void _viv_search_everything(int add);
static int _viv_send_everything_search(HWND parent,int add,int randomize,const wchar_t *search);
static INT_PTR CALLBACK _viv_search_everything_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static int _viv_playlist_shuffle_index_from_fd(const WIN32_FIND_DATA *fd);
static void _viv_do_initial_shuffle(void);
static _viv_playlist_t *_viv_playlist_from_fd(const WIN32_FIND_DATA *fd);
static int _viv_compare_id(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b);
static int _viv_fd_compare_name(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b);
static int _viv_fd_compare_path_and_name(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b);
static void _viv_update_1to1_scroll(int x,int y);
static HBITMAP _viv_orientate_hbitmap(HBITMAP hbitmap,int counterclockwise);
static void _viv_send_random_everything_search(void);
static void _viv_do_mousewheel_action(int action,int delta,int x,int y);
static void _viv_mipmap_free(_viv_mipmap_t *mipmap);
static void _viv_queue_clear(void);
static void _viv_clear(void);
static void _viv_process_pending_clear(void);
static void _viv_clear_loading_preload(void);
static void _viv_clear_preload(void);
static void _viv_clear_preload_frames(void);
static HBITMAP _viv_get_mipmap(HBITMAP hbitmap,int image_wide,int image_high,int render_wide,int render_high,int *pmip_wide,int *pmip_high,_viv_mipmap_t **out_mip);
static void _viv_preload_next(void);
static void _viv_start_first_frame(void);
static void _viv_activate_preload(void);
static int _viv_webp_info_proc(_viv_webp_t *viv_webp,DWORD frame_count,DWORD wide,DWORD high,int has_alpha);
static int _viv_webp_frame_proc(_viv_webp_t *viv_webp,BYTE *pixels,int delay);
static void _viv_clear_frames(_viv_frame_t *frames,int loaded_count);
static void viv_copy_current_image_to_last_image(void);
static void _viv_activate_last(void);
static void _viv_clear_last(void);
static void _viv_refresh(void);
static void _viv_show_cursor(void);
static void _viv_hide_cursor(void);
static int _viv_should_show_cursor(void);
static void _viv_update_show_cursor(void);
static void _viv_start_hide_cursor_timer(void);
static int _viv_main(int nCmdShow);
static void _viv_get_exe_filename(wchar_t filename[STRING_SIZE]);
static void _viv_do_left_click_action(int action);
static void _viv_start_move_window(void);
static int _viv_ceil(double x);
static void _viv_center_listbox_item(HWND listbox_hwnd,int item_index);
static void _viv_stretch_blt(HDC dst_hdc,int dst_x,int dst_y,int dst_wide,int dst_high,HDC src_hdc,int src_wide,int src_high,int clip_x,int clip_y,int clip_wide,int clip_high);
static BOOL _viv_StretchBltStitch(HDC hdcDest,int xDest,int yDest,int wDest,int hDest,HDC hdcSrc,int xSrc,int ySrc,int wSrc,int hSrc,DWORD rop,int clip_x,int clip_y,int clip_wide,int clip_high);
static BOOL _viv_get_src_pixel_pos(int client_x,int client_y,POINT *out_src_pt);
static void _viv_get_src_pixel_rgb(int src_x,int src_y,COLORREF *out_colorref);
static int _viv_clamp_zoom_pos(int zoom_pos);
static void _viv_open_preload(void);
//static void _viv_get_tooltip(void);
//static void _viv_tooltip_hide(void);
//static void _viv_tooltip_update(void);
//static void _viv_tooltip_update_track_position(void);

static HMODULE _viv_stobject_hmodule = 0;
static _viv_playlist_t *_viv_playlist_start = 0;
static _viv_playlist_t *_viv_playlist_last = 0;
static int _viv_playlist_count = 0;
static _viv_playlist_t **_viv_playlist_shuffle_indexes = 0;
static int _viv_playlist_shuffle_allocated = 0;
static LARGE_INTEGER _viv_playlist_id = {0};
static HWND _viv_hwnd = 0;
static HWND _viv_status_hwnd = 0;
static HWND _viv_toolbar_hwnd = 0;
static HWND _viv_rebar_hwnd = 0;
//static HWND _viv_tooltip_hwnd = 0;
static HIMAGELIST _viv_toolbar_image_list = 0;
static HANDLE _viv_mutex = 0;
//static float _viv_animation_rates[] = {0.085899f,0.107374f,0.134218f,0.167772f,0.209715f,0.262144f,0.327680f,0.409600f,0.512000f,0.640000f,0.800000f,1.000000f,1.250000f,1.562500f,1.953125f,2.441406f,3.051758f,3.814697f,4.768372f,5.960464f,7.450581f,9.313226f}; // natural curve.
static float _viv_animation_rates[] = {0.125000f,0.142857f,0.166667f,0.200000f,0.250000f,0.333333f,0.500000f,0.571429f,0.666667f,0.800000f,1.000000f,1.250000f,1.500000f,1.750000f,2.000000f,3.000000f,4.000000f,5.000000f,6.000000f,7.000000f,8.000000f}; // fixed animation rates
#define _VIV_ANIMATION_RATE_MAX	(sizeof(_viv_animation_rates) / sizeof(float))
#define _VIV_ANIMATION_RATE_ONE	10
static int _viv_animation_rate_pos = _VIV_ANIMATION_RATE_ONE;
static BYTE _viv_animation_play = 1; // play or pause animations
static BYTE _viv_1to1 = 0; // temporarily show the image with 100% scaling
static BYTE _viv_have_old_zoom = 0; // restore this zoom level after leaving 1:1 mode.
static int _viv_old_zoom_pos = 0; // restore this zoom level after leaving 1:1 mode.
WIN32_FIND_DATA *_viv_current_fd = 0; // the current image find data including the full path and filename.
WIN32_FIND_DATA *_viv_preload_fd = 0;
static int _viv_view_x = 0; // the current image offset in pixels
static int _viv_view_y = 0; // the current image offset in pixels
static double _viv_view_ix = 0.0; // the current image offset in percent, used when resizing the window
static double _viv_view_iy = 0.0; // the current image offset in percent, used when resizing the window
static int _viv_zoom_pos = 0; // the current zoom level
static float _viv_zoom_presets[_VIV_ZOOM_MAX] = {0.004815f,0.019215f,0.043060f,0.076120f,0.118079f,0.168530f,0.226989f,0.292893f,0.365607f,0.444430f,0.528603f,0.617316f,0.709715f,0.804909f,0.901983f,1.000000f}; // (1 - cos(((float)(x+1) * 1.570796f) / _VIV_ZOOM_MAX)) // this is missing cos((1 * 1.570796f) / _VIV_ZOOM_MAX), which is too small
static ULONG_PTR os_GdiplusToken; // gdiplus handle
static int _viv_image_wide = 0; // current image width
static int _viv_image_high = 0; // current image width
static int _viv_frame_count = 0; // current image frame count, 1 for static image, > 1 for animation
static int _viv_frame_loaded_count = 0; // number of loaded frames, can be less than _viv_frame_count
static int _viv_frame_position = 0; // the current frame position
static BYTE _viv_frame_looped = 0; // all frames have been displayed for this animation
static BYTE _viv_is_slideshow_timeup = 0; // the slideshow timer has expired, but we are still showing an animation at least once.
static _viv_frame_t *_viv_frames = 0; // the frames that make up an image, could be more than one for animations.
static VIV_UINT64 _viv_timer_tick = 0; // the current tick for the current frame.
static BYTE _viv_is_animation_timer = 0; // animation timer started?
static VIV_UINT64 _viv_animation_timer_tick_start = 0; // the current start tick
static BYTE _viv_doing = _VIV_DOING_NOTHING; // current mouse action, such as drag to scroll image
static int _viv_doing_x;
static int _viv_doing_y;
static int _viv_mdoing_x;
static int _viv_mdoing_y;
static BYTE _viv_fullscreen_is_maxed = 0;
static BYTE _viv_is_fullscreen = 0;
static RECT _viv_fullscreen_rect;
static int _viv_fullscreen_zoom_offset = 0;
static BYTE _viv_is_slideshow = 0;
static BYTE _viv_is_hide_cursor_timer = 0;
static CLIPFORMAT _viv_CF_PREFERREDDROPEFFECT = 0; // copy or move? clipboard operation
static WORD _viv_slideshow_rate_presets[] = {250,500,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000,20000,30000,40000,50000,60000};
#define _VIV_SLIDESHOW_RATE_PRESET_COUNT (sizeof(_viv_slideshow_rate_presets) / sizeof(WORD))
static BYTE _viv_is_cursor_shown = 1;
static int _viv_mousemove_x = -1;
static int _viv_mousemove_y = -1;
static BYTE _viv_in_popup_menu = 0;
static GUID _viv_FrameDimensionTime = {0x6aedbd6d,0x3fb5,0x418a,{0x83,0xa6,0x7f,0x45,0x22,0x9d,0xc8,0x72}};
static HMENU _viv_hmenu = 0;
static int _viv_dst_pos_x = 500;
static int _viv_dst_pos_y = 500;
#define _VIV_DST_ZOOM_MAX	139
static float *_viv_dst_zoom_values;
#define _VIV_DST_ZOOM_ONE	82
static int _viv_dst_zoom_x_pos = _VIV_DST_ZOOM_ONE;
static int _viv_dst_zoom_y_pos = _VIV_DST_ZOOM_ONE;
static CRITICAL_SECTION _viv_cs;
static HANDLE _viv_load_image_thread = 0;
static BYTE _viv_load_is_preload = 0;
static wchar_t *_viv_load_image_filename = 0;
static WIN32_FIND_DATA *_viv_load_image_next_fd = NULL;
static BYTE _viv_load_image_next_is_preload = 0;
static volatile int _viv_load_image_terminate = 0;
static _viv_reply_t *_viv_reply_start = 0;
static _viv_reply_t *_viv_reply_last = 0;
static wchar_t *_viv_status_temp_text = 0;
static localization_id_t _viv_options_page_localization_id_array[] = {LOCALIZATION_OPTIONS_GENERAL_PAGE,LOCALIZATION_OPTIONS_VIEW_PAGE,LOCALIZATION_OPTIONS_CONTROLS_PAGE};
static int _viv_options_tab_ids[] = {IDC_TAB1,IDC_TAB2,IDC_TAB3};
static int _viv_options_dialog_ids[] = {IDD_GENERAL,IDD_VIEW,IDD_CONTROLS};
#define _VIV_OPTIONS_PAGE_COUNT	(sizeof(_viv_options_dialog_ids) / sizeof(int))
static int _viv_options_page_ids[] = {VIV_ID_OPTIONS_GENERAL,VIV_ID_OPTIONS_VIEW,VIV_ID_OPTIONS_CONTROLS};
static DLGPROC _viv_options_page_procs[] = {_viv_options_general_proc,_viv_options_view_proc,_viv_options_controls_proc};
static HFONT _viv_about_hfont = 0;
static wchar_t *_viv_last_open_file = 0;
static wchar_t *_viv_last_open_folder = 0;
static _viv_nav_item_t **_viv_nav_items = 0;
static _viv_nav_item_t *__viv_nav_item_start = 0;
static _viv_nav_item_t *_viv_nav_item_last = 0;
static int _viv_nav_item_count = 0;
static wchar_t *_viv_random = 0; // temp shuffle.
static DWORD _viv_random_tot_results = 0xffffffff;
static DWORD _viv_everything_request_flags = 0;
static BYTE _viv_is_timer_queue_timer = 0;
static HANDLE _viv_timer_queue_timer_handle;
static BYTE _viv_is_animation_timer_event = 0;
static BYTE _viv_is_animation_paint = 0;
static BYTE _viv_preload_state = 0; // 0 = loading, 1=complete, 2=failed
static int _viv_preload_image_wide = 0; // current image width
static int _viv_preload_image_high = 0; // current image height
static int _viv_preload_frame_count = 0; // current image frame count, 1 for static image, > 1 for animation
static int _viv_preload_frame_loaded_count = 0; // number of loaded frames, can be less than _viv_frame_count
static _viv_frame_t *_viv_preload_frames = 0; // the frames that make up an image, could be more than one for animations.
static BYTE _viv_last_is_prev = 0; // preload next or previous?
static BYTE _viv_preload_is_prev = 0; // preload next or previous?
static BYTE _viv_should_activate_preload_on_load = 0;
static int _viv_load_render_wide = 0;
static int _viv_load_render_high = 0;
static _viv_frame_t *_viv_pending_clear_frames = NULL;
static int _viv_pending_clear_frame_loaded_count = 0;
static WIN32_FIND_DATA *_viv_last_fd = 0; // the last find data including the full path and filename.
static WIN32_FIND_DATA *_viv_frame_fd = 0; // the frame fd, may differ to the current fd because we change the title before the frames are loaded.
static WIN32_FIND_DATA *_viv_load_fd = 0; // the load fd
static int _viv_last_image_wide = 0; // last image width
static int _viv_last_image_high = 0; // last image height
static int _viv_last_frame_count = 0; // last image frame count, 1 for static image, > 1 for animation (all frames are loaded)
static _viv_frame_t *_viv_last_frames = 0; // the last frames that make up an image, could be more than one for animations.
static BYTE _viv_load_image_allow_draw = 0; // allow the image to show if it loaded successfully after the load was terminated.
static LRESULT (CALLBACK *_viv_old_status_proc)(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) = NULL; // old status bar proc
static BYTE _viv_prevent_on_size = 0; // don't process WM_SIZE changes.
static BYTE _viv_file_not_found = 0; // the current file was not found. -filename is shown in the window caption.
static BYTE _viv_load_failed = 0; // the current file failed to load. -filename is shown in the window caption.
static BYTE _viv_is_tracking_mouse = 0; // currently tracking mouse movement over our window.
static BYTE _viv_is_mouseover = 0; // mouse is currently over our window.
static BYTE _viv_prevent_on_deactivate = 0; // prevent handling of deactivate in WM_ACTIVATE.
static BYTE _viv_jump_ret = 0;
static int _viv_load_frame_count = 0; // the number of frames loaded, while loading.
static int _viv_src_pixel_x = -1;
static int _viv_src_pixel_y = -1;
static BYTE _viv_src_pixel_r = 0;
static BYTE _viv_src_pixel_g = 0;
static BYTE _viv_src_pixel_b = 0;
static BYTE _viv_is_prevent_sleep = 0;
//static BYTE _viv_is_alt = 0;

// MF_OWNERDRAW = don't show in menu.
static _viv_command_t _viv_commands[] = 
{
	{LOCALIZATION_ID_FILE,MF_POPUP,_VIV_MENU_ROOT,_VIV_MENU_FILE},

	{LOCALIZATION_ID_OPEN_FILE,MF_STRING,_VIV_MENU_FILE,VIV_ID_FILE_OPEN_FILE},
	{LOCALIZATION_ID_OPEN_FOLDER,MF_STRING,_VIV_MENU_FILE,VIV_ID_FILE_OPEN_FOLDER},
	{LOCALIZATION_ID_OPEN_EVERYTHING_SEARCH,MF_STRING|MF_OWNERDRAW,_VIV_MENU_FILE,VIV_ID_FILE_OPEN_EVERYTHING_SEARCH},
	{LOCALIZATION_ID_ADD_FILE,MF_STRING|MF_OWNERDRAW,_VIV_MENU_FILE,VIV_ID_FILE_ADD_FILE},
	{LOCALIZATION_ID_ADD_FOLDER,MF_STRING|MF_OWNERDRAW,_VIV_MENU_FILE,VIV_ID_FILE_ADD_FOLDER},
	{LOCALIZATION_ID_ADD_EVERYTHING_SEARCH,MF_STRING|MF_OWNERDRAW,_VIV_MENU_FILE,VIV_ID_FILE_ADD_EVERYTHING_SEARCH},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_FILE,0},
	{LOCALIZATION_ID_OPEN_FILE_LOCATION,MF_STRING,_VIV_MENU_FILE,VIV_ID_FILE_OPEN_FILE_LOCATION},
	{LOCALIZATION_ID_EDIT,MF_STRING,_VIV_MENU_FILE,VIV_ID_FILE_EDIT},
	{LOCALIZATION_ID_PREVIEW,MF_STRING,_VIV_MENU_FILE,VIV_ID_FILE_PREVIEW},
	{LOCALIZATION_ID_PRINT,MF_STRING,_VIV_MENU_FILE,VIV_ID_FILE_PRINT},
	{LOCALIZATION_ID_SET_DESKTOP_WALLPAPER,MF_STRING,_VIV_MENU_FILE,VIV_ID_FILE_SET_DESKTOP_WALLPAPER},
	{LOCALIZATION_ID_CLOSE,MF_STRING|MF_OWNERDRAW,_VIV_MENU_FILE,VIV_ID_FILE_CLOSE}, // this just clears the image.
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_FILE,0},
	{LOCALIZATION_ID_DELETE,MF_STRING|MF_DELETE,_VIV_MENU_FILE,VIV_ID_FILE_DELETE},
	{LOCALIZATION_ID_DELETE_RECYCLE,MF_STRING|MF_OWNERDRAW,_VIV_MENU_FILE,VIV_ID_FILE_DELETE_RECYCLE},
	{LOCALIZATION_ID_DELETE_PERMANENTLY,MF_STRING|MF_OWNERDRAW,_VIV_MENU_FILE,VIV_ID_FILE_DELETE_PERMANENTLY},
	{LOCALIZATION_ID_RENAME,MF_STRING,_VIV_MENU_FILE,VIV_ID_FILE_RENAME},
	{LOCALIZATION_ID_PROPERTIES,MF_STRING,_VIV_MENU_FILE,VIV_ID_FILE_PROPERTIES},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_FILE,0},
	{LOCALIZATION_ID_EXIT,MF_STRING,_VIV_MENU_FILE,VIV_ID_FILE_EXIT},
	
	{LOCALIZATION_ID_EDIT_MENU,MF_POPUP,_VIV_MENU_ROOT,_VIV_MENU_EDIT},

	{LOCALIZATION_ID_CUT,MF_STRING,_VIV_MENU_EDIT,VIV_ID_EDIT_CUT},
	{LOCALIZATION_ID_COPY,MF_STRING,_VIV_MENU_EDIT,VIV_ID_EDIT_COPY},
	{LOCALIZATION_ID_COPY_FILENAME,MF_STRING|MF_OWNERDRAW,_VIV_MENU_EDIT,VIV_ID_EDIT_COPY_FILENAME},
	{LOCALIZATION_ID_COPY_IMAGE,MF_STRING,_VIV_MENU_EDIT,VIV_ID_EDIT_COPY_IMAGE},
	{LOCALIZATION_ID_PASTE,MF_STRING|MF_OWNERDRAW,_VIV_MENU_EDIT,VIV_ID_EDIT_PASTE},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_EDIT,0},
	{LOCALIZATION_ID_ROTATE_CLOCKWISE,MF_STRING,_VIV_MENU_EDIT,VIV_ID_EDIT_ROTATE_90},
	{LOCALIZATION_ID_ROTATE_COUNTERCLOCKWISE,MF_STRING,_VIV_MENU_EDIT,VIV_ID_EDIT_ROTATE_270},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_EDIT,0},
	{LOCALIZATION_ID_COPY_TO,MF_STRING,_VIV_MENU_EDIT,VIV_ID_EDIT_COPY_TO},
	{LOCALIZATION_ID_MOVE_TO,MF_STRING,_VIV_MENU_EDIT,VIV_ID_EDIT_MOVE_TO},

	{LOCALIZATION_ID_VIEW,MF_POPUP,_VIV_MENU_ROOT,_VIV_MENU_VIEW},
	
	{LOCALIZATION_ID_CAPTION,MF_STRING|MF_OWNERDRAW,_VIV_MENU_VIEW,VIV_ID_VIEW_CAPTION},
	{LOCALIZATION_ID_FRAME,MF_STRING|MF_OWNERDRAW,_VIV_MENU_VIEW,VIV_ID_VIEW_THICKFRAME},
	{LOCALIZATION_ID_MENU,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_MENU},
	{LOCALIZATION_ID_STATUS_BAR,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_STATUS},
	{LOCALIZATION_ID_CONTROLS,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_CONTROLS},
	{LOCALIZATION_ID_PRESET,MF_POPUP,_VIV_MENU_VIEW,_VIV_MENU_VIEW_PRESET},
	{LOCALIZATION_ID_MINIMAL,MF_STRING,_VIV_MENU_VIEW_PRESET,VIV_ID_VIEW_PRESET_1},
	{LOCALIZATION_ID_COMPACT,MF_STRING,_VIV_MENU_VIEW_PRESET,VIV_ID_VIEW_PRESET_2},
	{LOCALIZATION_ID_NORMAL,MF_STRING,_VIV_MENU_VIEW_PRESET,VIV_ID_VIEW_PRESET_3},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_VIEW,0},
	{LOCALIZATION_ID_FULLSCREEN,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_FULLSCREEN},
	{LOCALIZATION_ID_SLIDESHOW,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_SLIDESHOW},
	{LOCALIZATION_ID_WINDOW_SIZE,MF_POPUP,_VIV_MENU_VIEW,_VIV_MENU_VIEW_WINDOW_SIZE},
	{LOCALIZATION_ID_50_PERCENT,MF_STRING,_VIV_MENU_VIEW_WINDOW_SIZE,VIV_ID_VIEW_WINDOW_SIZE_50},
	{LOCALIZATION_ID_100_PERCENT,MF_STRING,_VIV_MENU_VIEW_WINDOW_SIZE,VIV_ID_VIEW_WINDOW_SIZE_100},
	{LOCALIZATION_ID_200_PERCENT,MF_STRING,_VIV_MENU_VIEW_WINDOW_SIZE,VIV_ID_VIEW_WINDOW_SIZE_200},
	{LOCALIZATION_ID_AUTO_FIT,MF_STRING,_VIV_MENU_VIEW_WINDOW_SIZE,VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT},
	{LOCALIZATION_ID_REFRESH,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_REFRESH},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_VIEW,0},
	{LOCALIZATION_ID_ALLOW_SHRINKING,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_ALLOW_SHRINKING},
	{LOCALIZATION_ID_KEEP_ASPECT_RATIO,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_KEEP_ASPECT_RATIO},
	{LOCALIZATION_ID_FILL_WINDOW,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_FILL_WINDOW},
	{LOCALIZATION_ID_ONE_TO_ONE,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_1TO1},
	{LOCALIZATION_ID_BEST_FIT,MF_STRING|MF_OWNERDRAW,_VIV_MENU_VIEW,VIV_ID_VIEW_BESTFIT},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_VIEW,0},
	{LOCALIZATION_ID_PAN_SCAN,MF_POPUP,_VIV_MENU_VIEW,_VIV_MENU_VIEW_PANSCAN},
	{LOCALIZATION_ID_INCREASE_SIZE,MF_STRING,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_INCREASE_SIZE},
	{LOCALIZATION_ID_DECREASE_SIZE,MF_STRING,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_DECREASE_SIZE},
	{LOCALIZATION_ID_INCREASE_WIDTH,MF_STRING,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_INCREASE_WIDTH},
	{LOCALIZATION_ID_DECREASE_WIDTH,MF_STRING,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_DECREASE_WIDTH},
	{LOCALIZATION_ID_INCREASE_HEIGHT,MF_STRING,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_INCREASE_HEIGHT},
	{LOCALIZATION_ID_DECREASE_HEIGHT,MF_STRING,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_DECREASE_HEIGHT},
	{LOCALIZATION_ID_ZOOM,MF_POPUP,_VIV_MENU_VIEW,_VIV_MENU_VIEW_ZOOM},
	{LOCALIZATION_ID_ZOOM_IN,MF_STRING,_VIV_MENU_VIEW_ZOOM,VIV_ID_VIEW_ZOOM_IN},
	{LOCALIZATION_ID_ZOOM_OUT,MF_STRING,_VIV_MENU_VIEW_ZOOM,VIV_ID_VIEW_ZOOM_OUT},
	{LOCALIZATION_ID_RESET,MF_STRING,_VIV_MENU_VIEW_ZOOM,VIV_ID_VIEW_ZOOM_RESET},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_VIEW_PANSCAN,0},
	{LOCALIZATION_ID_MOVE_UP,MF_STRING,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_UP},
	{LOCALIZATION_ID_MOVE_DOWN,MF_STRING,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_DOWN},
	{LOCALIZATION_ID_MOVE_LEFT,MF_STRING,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_LEFT},
	{LOCALIZATION_ID_MOVE_RIGHT,MF_STRING,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_RIGHT},
	{LOCALIZATION_ID_MOVE_UP_LEFT,MF_STRING|MF_OWNERDRAW,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_UP_LEFT},
	{LOCALIZATION_ID_MOVE_UP_RIGHT,MF_STRING|MF_OWNERDRAW,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_UP_RIGHT},
	{LOCALIZATION_ID_MOVE_DOWN_LEFT,MF_STRING|MF_OWNERDRAW,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_DOWN_LEFT},
	{LOCALIZATION_ID_MOVE_DOWN_RIGHT,MF_STRING|MF_OWNERDRAW,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_DOWN_RIGHT},
	{LOCALIZATION_ID_MOVE_CENTER,MF_STRING,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_MOVE_CENTER},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_VIEW_PANSCAN,0},
	{LOCALIZATION_ID_PANSCAN_RESET,MF_STRING,_VIV_MENU_VIEW_PANSCAN,VIV_ID_VIEW_PANSCAN_RESET},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_VIEW,0},
	{LOCALIZATION_ID_ON_TOP,MF_POPUP,_VIV_MENU_VIEW,_VIV_MENU_VIEW_ONTOP},
	{LOCALIZATION_ID_ALWAYS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_VIEW_ONTOP,VIV_ID_VIEW_ONTOP_ALWAYS},
	{LOCALIZATION_ID_WHILE_PLAYING_OR_ANIMATING,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_VIEW_ONTOP,VIV_ID_VIEW_ONTOP_WHILE_PLAYING_OR_ANIMATING},
	{LOCALIZATION_ID_NEVER,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_VIEW_ONTOP,VIV_ID_VIEW_ONTOP_NEVER},
	{LOCALIZATION_ID_OPTIONS,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_OPTIONS},
	{LOCALIZATION_ID_SLIDESHOW_MENU,MF_POPUP,_VIV_MENU_ROOT,_VIV_MENU_SLIDESHOW},
	{LOCALIZATION_ID_PLAY_PAUSE,MF_STRING,_VIV_MENU_SLIDESHOW,VIV_ID_SLIDESHOW_PAUSE},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_SLIDESHOW,0},
	{LOCALIZATION_ID_RATE,MF_POPUP,_VIV_MENU_SLIDESHOW,_VIV_MENU_SLIDESHOW_RATE},
	{LOCALIZATION_ID_DECREASE_RATE,MF_STRING,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_DEC},
	{LOCALIZATION_ID_INCREASE_RATE,MF_STRING,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_INC},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_SLIDESHOW_RATE,0},
	{LOCALIZATION_ID_RATE_250_MILLISECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_250},
	{LOCALIZATION_ID_RATE_500_MILLISECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_500},
	{LOCALIZATION_ID_RATE_1_SECOND,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_1000},
	{LOCALIZATION_ID_RATE_2_SECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_2000},
	{LOCALIZATION_ID_RATE_3_SECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_3000},
	{LOCALIZATION_ID_RATE_4_SECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_4000},
	{LOCALIZATION_ID_RATE_5_SECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_5000},
	{LOCALIZATION_ID_RATE_6_SECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_6000},
	{LOCALIZATION_ID_RATE_7_SECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_7000},
	{LOCALIZATION_ID_RATE_8_SECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_8000},
	{LOCALIZATION_ID_RATE_9_SECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_9000},
	{LOCALIZATION_ID_RATE_10_SECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_10000},
	{LOCALIZATION_ID_RATE_20_SECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_20000},
	{LOCALIZATION_ID_RATE_30_SECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_30000},
	{LOCALIZATION_ID_RATE_40_SECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_40000},
	{LOCALIZATION_ID_RATE_50_SECONDS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_50000},
	{LOCALIZATION_ID_RATE_1_MINUTE,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_60000},
	{LOCALIZATION_ID_CUSTOM,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_SLIDESHOW_RATE,VIV_ID_SLIDESHOW_RATE_CUSTOM},

	{LOCALIZATION_ID_ANIMATION,MF_POPUP,_VIV_MENU_ROOT,_VIV_MENU_ANIMATION},
	{LOCALIZATION_ID_ANIMATION_PLAY_PAUSE,MF_STRING,_VIV_MENU_ANIMATION,VIV_ID_ANIMATION_PLAY_PAUSE},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_ANIMATION,0},
	{LOCALIZATION_ID_ANIMATION_JUMP_FORWARD,MF_STRING,_VIV_MENU_ANIMATION,VIV_ID_ANIMATION_JUMP_FORWARD_MEDIUM},
	{LOCALIZATION_ID_ANIMATION_JUMP_BACKWARD,MF_STRING,_VIV_MENU_ANIMATION,VIV_ID_ANIMATION_JUMP_BACKWARD_MEDIUM},
	{LOCALIZATION_ID_ANIMATION_SHORT_JUMP_FORWARD,MF_STRING|MF_OWNERDRAW,_VIV_MENU_ANIMATION,VIV_ID_ANIMATION_JUMP_FORWARD_SHORT},
	{LOCALIZATION_ID_ANIMATION_SHORT_JUMP_BACKWARD,MF_STRING|MF_OWNERDRAW,_VIV_MENU_ANIMATION,VIV_ID_ANIMATION_JUMP_BACKWARD_SHORT},
	{LOCALIZATION_ID_ANIMATION_LONG_JUMP_FORWARD,MF_STRING|MF_OWNERDRAW,_VIV_MENU_ANIMATION,VIV_ID_ANIMATION_JUMP_FORWARD_LONG},
	{LOCALIZATION_ID_ANIMATION_LONG_JUMP_BACKWARD,MF_STRING|MF_OWNERDRAW,_VIV_MENU_ANIMATION,VIV_ID_ANIMATION_JUMP_BACKWARD_LONG},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_ANIMATION,0},
	{LOCALIZATION_ID_ANIMATION_FRAME_STEP,MF_STRING,_VIV_MENU_ANIMATION,VIV_ID_ANIMATION_FRAME_STEP},
	{LOCALIZATION_ID_ANIMATION_PREVIOUS_FRAME,MF_STRING,_VIV_MENU_ANIMATION,VIV_ID_ANIMATION_FRAME_PREV},
	{LOCALIZATION_ID_ANIMATION_FIRST_FRAME,MF_STRING,_VIV_MENU_ANIMATION,VIV_ID_ANIMATION_FRAME_HOME},
	{LOCALIZATION_ID_ANIMATION_LAST_FRAME,MF_STRING,_VIV_MENU_ANIMATION,VIV_ID_ANIMATION_FRAME_END},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_ANIMATION,0},
	{LOCALIZATION_ID_ANIMATION_DECREASE_RATE,MF_STRING,_VIV_MENU_ANIMATION,VIV_ID_ANIMATION_RATE_DEC},
	{LOCALIZATION_ID_ANIMATION_INCREASE_RATE,MF_STRING,_VIV_MENU_ANIMATION,VIV_ID_ANIMATION_RATE_INC},
	{LOCALIZATION_ID_ANIMATION_RESET_RATE,MF_STRING,_VIV_MENU_ANIMATION,VIV_ID_ANIMATION_RATE_RESET},
	
	{LOCALIZATION_ID_NAVIGATE,MF_POPUP,_VIV_MENU_ROOT,_VIV_MENU_NAVIGATE},
	{LOCALIZATION_ID_NEXT,MF_STRING,_VIV_MENU_NAVIGATE,VIV_ID_NAV_NEXT},
	{LOCALIZATION_ID_PREVIOUS,MF_STRING,_VIV_MENU_NAVIGATE,VIV_ID_NAV_PREV},
	{LOCALIZATION_ID_HOME,MF_STRING,_VIV_MENU_NAVIGATE,VIV_ID_NAV_HOME},
	{LOCALIZATION_ID_END,MF_STRING,_VIV_MENU_NAVIGATE,VIV_ID_NAV_END},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_NAVIGATE,0},
	{LOCALIZATION_ID_SORT,MF_POPUP,_VIV_MENU_NAVIGATE,_VIV_MENU_NAVIGATE_SORT},
	{LOCALIZATION_ID_SORT_NAME,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_NAVIGATE_SORT,VIV_ID_NAV_SORT_NAME},
	{LOCALIZATION_ID_SORT_FULL_PATH,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_NAVIGATE_SORT,VIV_ID_NAV_SORT_FULL_PATH},
	{LOCALIZATION_ID_SORT_SIZE,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_NAVIGATE_SORT,VIV_ID_NAV_SORT_SIZE},
	{LOCALIZATION_ID_SORT_DATE_MODIFIED,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_NAVIGATE_SORT,VIV_ID_NAV_SORT_DATE_MODIFIED},
	{LOCALIZATION_ID_SORT_DATE_CREATED,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_NAVIGATE_SORT,VIV_ID_NAV_SORT_DATE_CREATED},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_NAVIGATE_SORT,0},
	{LOCALIZATION_ID_SORT_ASCENDING,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_NAVIGATE_SORT,VIV_ID_NAV_SORT_ASCENDING},
	{LOCALIZATION_ID_SORT_DESCENDING,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_NAVIGATE_SORT,VIV_ID_NAV_SORT_DESCENDING},
	{LOCALIZATION_ID_SHUFFLE,MF_STRING,_VIV_MENU_NAVIGATE,VIV_ID_NAV_SHUFFLE},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_NAVIGATE,0},
	{LOCALIZATION_ID_JUMP_TO,MF_STRING,_VIV_MENU_NAVIGATE,VIV_ID_NAV_JUMPTO},

	{LOCALIZATION_ID_HELP,MF_POPUP,_VIV_MENU_ROOT,_VIV_MENU_HELP},
	{LOCALIZATION_ID_HELP_MENU,MF_STRING,_VIV_MENU_HELP,VIV_ID_HELP_HELP},
	{LOCALIZATION_ID_COMMAND_LINE_OPTIONS,MF_STRING,_VIV_MENU_HELP,VIV_ID_HELP_COMMAND_LINE_OPTIONS},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_HELP,0},
	{LOCALIZATION_ID_HOME_PAGE,MF_STRING,_VIV_MENU_HELP,VIV_ID_HELP_WEBSITE},
	{LOCALIZATION_ID_DONATE,MF_STRING,_VIV_MENU_HELP,VIV_ID_HELP_DONATE},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_HELP,0},
	{LOCALIZATION_ID_ABOUT,MF_STRING,_VIV_MENU_HELP,VIV_ID_HELP_ABOUT},
};

#define _VIV_COMMAND_COUNT	(sizeof(_viv_commands) / sizeof(_viv_command_t))

_viv_default_key_t _viv_default_keys[] =
{
	{VIV_ID_FILE_OPEN_FILE,CONFIG_KEYFLAG_CTRL | 'O'},
	{VIV_ID_FILE_OPEN_FOLDER,CONFIG_KEYFLAG_CTRL | 'B'},
	{VIV_ID_FILE_OPEN_EVERYTHING_SEARCH,CONFIG_KEYFLAG_CTRL | 'E'},
	{VIV_ID_FILE_ADD_FILE,CONFIG_KEYFLAG_CTRL | CONFIG_KEYFLAG_SHIFT | 'O'},
	{VIV_ID_FILE_ADD_FOLDER,CONFIG_KEYFLAG_CTRL | CONFIG_KEYFLAG_SHIFT | 'B'},
	{VIV_ID_FILE_ADD_EVERYTHING_SEARCH,CONFIG_KEYFLAG_CTRL | CONFIG_KEYFLAG_SHIFT | 'E'},
	{VIV_ID_FILE_OPEN_FILE_LOCATION,CONFIG_KEYFLAG_CTRL | VK_RETURN},
//	{VIV_ID_FILE_EDIT,CONFIG_KEYFLAG_CTRL | 'E'},
	{VIV_ID_FILE_PRINT,CONFIG_KEYFLAG_CTRL | 'P'},
//	{VIV_ID_FILE_SET_DESKTOP_WALLPAPER,CONFIG_KEYFLAG_CTRL | 'D'}, // this needs a confirmation dialog
	{VIV_ID_FILE_CLOSE,CONFIG_KEYFLAG_CTRL | 'W'},
	{VIV_ID_FILE_DELETE_RECYCLE,VK_DELETE},
	{VIV_ID_FILE_DELETE_PERMANENTLY,CONFIG_KEYFLAG_SHIFT | VK_DELETE},
	{VIV_ID_FILE_RENAME,VK_F2},
	{VIV_ID_FILE_EXIT,CONFIG_KEYFLAG_CTRL | 'Q'},
	{VIV_ID_EDIT_CUT,CONFIG_KEYFLAG_CTRL | 'X'},
	{VIV_ID_EDIT_COPY,CONFIG_KEYFLAG_CTRL | 'C'},
	{VIV_ID_EDIT_COPY_FILENAME,CONFIG_KEYFLAG_CTRL | CONFIG_KEYFLAG_SHIFT | 'C'},
	{VIV_ID_EDIT_PASTE,CONFIG_KEYFLAG_CTRL | 'V'},
	{VIV_ID_VIEW_PRESET_1,'1'},
	{VIV_ID_VIEW_PRESET_2,'2'},
	{VIV_ID_VIEW_PRESET_3,'3'},
	{VIV_ID_VIEW_1TO1,CONFIG_KEYFLAG_CTRL | CONFIG_KEYFLAG_ALT | '0'},
	{VIV_ID_VIEW_FULLSCREEN,CONFIG_KEYFLAG_ALT | VK_RETURN},
	{VIV_ID_VIEW_SLIDESHOW,VK_F11},
	{VIV_ID_VIEW_WINDOW_SIZE_50,CONFIG_KEYFLAG_ALT | '1'},
	{VIV_ID_VIEW_WINDOW_SIZE_100,CONFIG_KEYFLAG_ALT | '2'},
	{VIV_ID_VIEW_WINDOW_SIZE_200,CONFIG_KEYFLAG_ALT | '3'},
	{VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT,CONFIG_KEYFLAG_ALT | '4'},
	{VIV_ID_VIEW_PANSCAN_INCREASE_SIZE,VK_NUMPAD9},
	{VIV_ID_VIEW_PANSCAN_DECREASE_SIZE,VK_NUMPAD1},
	{VIV_ID_VIEW_PANSCAN_INCREASE_WIDTH,VK_NUMPAD6},
	{VIV_ID_VIEW_PANSCAN_DECREASE_WIDTH,VK_NUMPAD4},
	{VIV_ID_VIEW_PANSCAN_INCREASE_HEIGHT,VK_NUMPAD8},
	{VIV_ID_VIEW_PANSCAN_DECREASE_HEIGHT,VK_NUMPAD2},
	{VIV_ID_VIEW_PANSCAN_MOVE_UP,CONFIG_KEYFLAG_CTRL | VK_NUMPAD8},
	{VIV_ID_VIEW_PANSCAN_MOVE_DOWN,CONFIG_KEYFLAG_CTRL | VK_NUMPAD2},
	{VIV_ID_VIEW_PANSCAN_MOVE_LEFT,CONFIG_KEYFLAG_CTRL | VK_NUMPAD4},
	{VIV_ID_VIEW_PANSCAN_MOVE_RIGHT,CONFIG_KEYFLAG_CTRL | VK_NUMPAD6},
	{VIV_ID_VIEW_PANSCAN_MOVE_UP_LEFT,CONFIG_KEYFLAG_CTRL | VK_NUMPAD7},
	{VIV_ID_VIEW_PANSCAN_MOVE_UP_RIGHT,CONFIG_KEYFLAG_CTRL | VK_NUMPAD9},
	{VIV_ID_VIEW_PANSCAN_MOVE_DOWN_LEFT,CONFIG_KEYFLAG_CTRL | VK_NUMPAD1},
	{VIV_ID_VIEW_PANSCAN_MOVE_DOWN_RIGHT,CONFIG_KEYFLAG_CTRL | VK_NUMPAD3},
	{VIV_ID_VIEW_PANSCAN_MOVE_CENTER,CONFIG_KEYFLAG_CTRL | VK_NUMPAD5},
	{VIV_ID_VIEW_PANSCAN_RESET,VK_NUMPAD5},
	{VIV_ID_VIEW_ZOOM_IN,VK_OEM_PLUS},
	{VIV_ID_VIEW_ZOOM_IN,VK_ADD},
	{VIV_ID_VIEW_ZOOM_IN,CONFIG_KEYFLAG_CTRL | VK_ADD},
	{VIV_ID_VIEW_ZOOM_OUT,VK_OEM_MINUS},
	{VIV_ID_VIEW_ZOOM_OUT,VK_SUBTRACT},
	{VIV_ID_VIEW_ZOOM_OUT,CONFIG_KEYFLAG_CTRL | VK_SUBTRACT},
	{VIV_ID_VIEW_ZOOM_RESET,CONFIG_KEYFLAG_CTRL | '0'},
	{VIV_ID_VIEW_ONTOP_ALWAYS,CONFIG_KEYFLAG_CTRL | 'T'},
	{VIV_ID_VIEW_OPTIONS,'O'},
	{VIV_ID_VIEW_REFRESH,VK_F5},
	{VIV_ID_SLIDESHOW_PAUSE,VK_SPACE},
	{VIV_ID_SLIDESHOW_RATE_DEC,VK_DOWN},
	{VIV_ID_SLIDESHOW_RATE_INC,VK_UP},
	{VIV_ID_ANIMATION_PLAY_PAUSE,CONFIG_KEYFLAG_CTRL | VK_SPACE},
	{VIV_ID_ANIMATION_JUMP_FORWARD_MEDIUM,CONFIG_KEYFLAG_CTRL | VK_NEXT},
	{VIV_ID_ANIMATION_JUMP_BACKWARD_MEDIUM,CONFIG_KEYFLAG_CTRL | VK_PRIOR},
	{VIV_ID_ANIMATION_FRAME_STEP,CONFIG_KEYFLAG_CTRL | VK_RIGHT},
	{VIV_ID_ANIMATION_FRAME_PREV,CONFIG_KEYFLAG_CTRL | VK_LEFT},
	{VIV_ID_ANIMATION_FRAME_HOME,CONFIG_KEYFLAG_CTRL | VK_HOME},
	{VIV_ID_ANIMATION_FRAME_END,CONFIG_KEYFLAG_CTRL | VK_END},
	{VIV_ID_ANIMATION_RATE_DEC,CONFIG_KEYFLAG_CTRL | VK_DOWN},
	{VIV_ID_ANIMATION_RATE_INC,CONFIG_KEYFLAG_CTRL | VK_UP},
	{VIV_ID_ANIMATION_RATE_RESET,CONFIG_KEYFLAG_CTRL | 'R'},
	{VIV_ID_NAV_NEXT,VK_RIGHT},
	{VIV_ID_NAV_NEXT,VK_NEXT},
	{VIV_ID_NAV_PREV,VK_LEFT},
	{VIV_ID_NAV_PREV,VK_PRIOR},
	{VIV_ID_NAV_HOME,VK_HOME},
	{VIV_ID_NAV_END,VK_END},
	{VIV_ID_NAV_JUMPTO,'J'},
	{VIV_ID_HELP_HELP,VK_F1},
	{VIV_ID_HELP_ABOUT,CONFIG_KEYFLAG_CTRL | VK_F1},
};

#define _VIV_DEFAULT_KEY_COUNT (sizeof(_viv_default_keys) / sizeof(_viv_default_key_t))

WORD _viv_context_menu_items[] = 
{
	VIV_ID_NAV_NEXT,
	VIV_ID_NAV_PREV,
	0,
	VIV_ID_VIEW_FULLSCREEN,
	VIV_ID_SLIDESHOW_PAUSE,
	_VIV_MENU_SLIDESHOW_RATE,
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
	_VIV_MENU_SLIDESHOW_RATE,
	0,
	VIV_ID_VIEW_MENU,
	0,
	VIV_ID_VIEW_ALLOW_SHRINKING,
	VIV_ID_VIEW_KEEP_ASPECT_RATIO,
	VIV_ID_VIEW_FILL_WINDOW,
	VIV_ID_VIEW_1TO1,
//	VIV_ID_VIEW_SLIDESHOW,
	0,
	_VIV_MENU_NAVIGATE_SORT,
	VIV_ID_NAV_SORT_NAME,
	VIV_ID_NAV_SORT_FULL_PATH,
	VIV_ID_NAV_SORT_SIZE,
	VIV_ID_NAV_SORT_DATE_MODIFIED,
	VIV_ID_NAV_SORT_DATE_CREATED,
	0,
	VIV_ID_NAV_SORT_ASCENDING,
	VIV_ID_NAV_SORT_DESCENDING,
	_VIV_MENU_NAVIGATE_SORT,
	0,
	VIV_ID_FILE_OPEN_FILE_LOCATION,
	VIV_ID_FILE_SET_DESKTOP_WALLPAPER,
	VIV_ID_FILE_EDIT,
	VIV_ID_FILE_PRINT,
	VIV_ID_FILE_PREVIEW,
	0,
	VIV_ID_EDIT_ROTATE_90,
	VIV_ID_EDIT_ROTATE_270,
	0,
	VIV_ID_EDIT_CUT,
	VIV_ID_EDIT_COPY,
	VIV_ID_EDIT_COPY_IMAGE,
	VIV_ID_FILE_DELETE,
	0,
	VIV_ID_FILE_PROPERTIES,
	VIV_ID_VIEW_OPTIONS,
	0,
	VIV_ID_FILE_EXIT,
};

#define _VIV_CONEXT_MENU_ITEM_COUNT	(sizeof(_viv_context_menu_items) / sizeof(WORD))

typedef struct _viv_key_list_s
{
	config_key_t *start[_VIV_COMMAND_COUNT];
	config_key_t *last[_VIV_COMMAND_COUNT];
	
}_viv_key_list_t;

_viv_key_list_t *_viv_key_list = 0;

const char *_viv_association_extensions[] = 
{
	"bmp",
	"gif",
	"ico",
	"jpeg",
	"jpg",
	"png",
	"tif",
	"tiff",
	"webp",
};

const char *_viv_association_descriptions[] = 
{
	"Bitmap Image",
	"Animated GIF Image",
	"Icon File",
	"JPEG Image",
	"JPEG Image",
	"PNG Image",
	"TIFF Image",
	"TIFF Image",
	"WebP Image",
};

const char *_viv_association_icon_locations[] = 
{
	NULL,
	NULL,
	"%1",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

const WORD _viv_association_dlg_item_id[] = 
{
	IDC_BMP,
	IDC_GIF,
	IDC_ICO,
	IDC_JPEG,
	IDC_JPG,
	IDC_PNG,
	IDC_TIF,
	IDC_TIFF,
	IDC_WEBP,
};

#define _VIV_ASSOCIATION_COUNT	(sizeof(_viv_association_extensions) / sizeof(const wchar_t *))

// load unicode for windows 95/98
// TODO: do this for x86 only
HMODULE LoadUnicowsProc(void);

extern FARPROC _PfnLoadUnicows = (FARPROC) &LoadUnicowsProc;

HMODULE LoadUnicowsProc(void)
{
	OSVERSIONINFOA osvi;
	
	// make sure we are win9x.
	// to prevent loading unicows.dll on NT as a securiry precausion.
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	if (GetVersionExA(&osvi))
	{
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			return LoadLibraryA("unicows.dll");
		}
	}
	
	return NULL;
}

static void _viv_update_title(void)
{
	wchar_t window_title[STRING_SIZE+STRING_SIZE];
	const wchar_t *filename;

	window_title[0] = 0;
	filename = L"";

	switch(config_title_bar_format)
	{
		case 0: // full path
			filename = _viv_current_fd->cFileName;
			break;

		case 1: // filename only
		default:
			filename = string_get_filename_part(_viv_current_fd->cFileName);
			break;
			
		case 2: // none
			break;
	}
	
	if (*filename)
	{
		string_cat(window_title,filename);
		string_cat_utf8(window_title," - ");
	}
		
	string_cat_utf8(window_title,localization_get_string(LOCALIZATION_ID_APP_NAME));
	
	SetWindowTextW(_viv_hwnd,window_title);
}

static void _viv_mipmap_free(_viv_mipmap_t *mipmap)
{
	DeleteObject(mipmap->hbitmap);
	
	if (mipmap->mipmap)
	{
		_viv_mipmap_free(mipmap->mipmap);
	}
	
	mem_free(mipmap);
}

// clearing is really slow.
// delay this until after the new image is shown.
// we add the clear to a queue which is cleared with _viv_process_pending_clear.
// _viv_process_pending_clear should be called after the new frame is shown.
static void _viv_clear(void)
{
	_viv_pending_clear_frames = _viv_frames;
	_viv_pending_clear_frame_loaded_count = _viv_frame_loaded_count;
	_viv_frames = NULL;
	_viv_frame_fd->cFileName[0] = 0;

	_viv_timer_stop();	

	_viv_frame_position = 0;
	_viv_frame_looped = 0;
	_viv_is_slideshow_timeup = 0;
	_viv_frame_count = 0;
	_viv_frame_loaded_count = 0;
	_viv_zoom_pos = 0;
	_viv_view_x = 0;
	_viv_view_y = 0;
	_viv_view_ix = 0.0;
	_viv_view_iy = 0.0;
	_viv_1to1 = 0;
	_viv_have_old_zoom = 0;
	_viv_image_wide = 0;
	_viv_image_high = 0;
	_viv_animation_play = 1;
}

static void _viv_clear_frames(_viv_frame_t *frames,int loaded_count)
{
	int i;
	
	for(i=0;i<loaded_count;i++)		
	{
		if (frames[i].hbitmap)
		{
			DeleteObject(frames[i].hbitmap);
		}
		
		if (frames[i].mipmap)
		{
			_viv_mipmap_free(frames[i].mipmap);
		}
	}
	
	mem_free(frames);
}

static void _viv_process_pending_clear(void)
{
	if (_viv_pending_clear_frames)
	{
		_viv_clear_frames(_viv_pending_clear_frames,_viv_pending_clear_frame_loaded_count);
		
		_viv_pending_clear_frames = NULL;
	}

	_viv_pending_clear_frame_loaded_count = 0;
}

static void _viv_clear_loading_preload(void)
{
	if (_viv_load_image_thread)
	{
		if (_viv_load_is_preload)
		{
			_viv_load_image_terminate = 1;
		}
	}
}

static void _viv_clear_preload_frames(void)
{
	if (_viv_preload_frames)
	{
		_viv_clear_frames(_viv_preload_frames,_viv_preload_frame_loaded_count);
		
		_viv_preload_frames = NULL;
	}

	_viv_preload_frame_count = 0;
	_viv_preload_frame_loaded_count = 0;
	_viv_preload_image_wide = 0;
	_viv_preload_image_high = 0;
}

static void _viv_clear_preload(void)
{
	_viv_clear_preload_frames();
	
	_viv_preload_fd->cFileName[0] = 0;
}

static BOOL _viv_open_from_filename(const wchar_t *filename)
{
	BOOL ret;
	WIN32_FIND_DATA fd;
	HANDLE h;
	wchar_t full_path_and_filename[STRING_SIZE];
	wchar_t cwd[STRING_SIZE];

	ret = FALSE;
	GetCurrentDirectory(STRING_SIZE,cwd);
	string_path_combine(full_path_and_filename,cwd,filename);

debug_printf("open filename: %S\n",full_path_and_filename);
	
	if ((os_GetFileAttributesExW) && (os_GetFileAttributesExW(full_path_and_filename,GetFileExInfoStandard,&fd)))
	{
		fd.dwReserved0 = 0;
		fd.dwReserved1 = 0;
		
		ret = TRUE;

		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// add subfolders and subsubfolders...
			_viv_playlist_add_path(full_path_and_filename);

			_viv_home(0,0);
		}
		else
		{
			string_copy(fd.cFileName,full_path_and_filename);
			
			_viv_open(&fd,0);
		}
	}
	else
	{
		h = FindFirstFile(full_path_and_filename,&fd);
		if (h != INVALID_HANDLE_VALUE)
		{
			wchar_t path[STRING_SIZE];
			
			ret = TRUE;

			string_get_path_part(path,full_path_and_filename);
		
			for(;;)
			{
				string_path_combine(full_path_and_filename,path,fd.cFileName);
					
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					_viv_playlist_add_path(full_path_and_filename);
				}
				else
				{
					string_copy(fd.cFileName,full_path_and_filename);
					_viv_playlist_add(&fd);
				}
					
				if (!FindNextFile(h,&fd))
				{
					break;
				}
			}

			_viv_home(0,0);
			
			FindClose(h);
		}
	}
	
	return ret;
}

// _viv_load_image_thread will be NULL if is_preload is 1.
static void _viv_open(WIN32_FIND_DATA *fd,int is_preload)
{
debug_printf("open: %S last %S frame %S is_preload %d\n",fd->cFileName,_viv_last_fd->cFileName,_viv_frame_fd->cFileName,is_preload);

if ((_viv_load_image_thread) && (_viv_load_image_filename))
{
debug_printf("CURRENTLY LOADING %S preload %d\n",_viv_load_image_filename,_viv_load_is_preload);
	
}

	if (!is_preload)
	{
		if (_viv_file_not_found)
		{
			_viv_file_not_found = 0;
			
			_viv_status_update();
		}

		if (_viv_load_failed)
		{
			_viv_load_failed = 0;

			_viv_status_update();
		}
	}

	if ((is_preload) && (_viv_last_frames) && (string_compare(_viv_last_fd->cFileName,fd->cFileName) == 0))
	{
		// don't preload if its the same as last cache.
		// this can occur if you have a playlist or folder with only 2 items.
		return;
	}
	else
	if ((!is_preload) && (_viv_last_frames) && (string_compare(_viv_last_fd->cFileName,fd->cFileName) == 0))
	{
		// activate last cache
		// don't activate preload.
		_viv_should_activate_preload_on_load = 0;
		
		// stop loading
		debug_printf("SET TERMINATE (LAST)\n");
		_viv_load_image_allow_draw = 0;
		_viv_load_image_terminate = 1;
		
		if (_viv_load_image_next_fd)
		{
			mem_free(_viv_load_image_next_fd);
			
			_viv_load_image_next_fd = NULL;
		}

		_viv_preload_fd->cFileName[0] = 0;
		
		_viv_status_update();

		_viv_activate_last();

		_viv_preload_next();

		return;
	}
	else
	if ((!is_preload) && (_viv_load_is_preload) && (*_viv_preload_fd->cFileName) && (string_compare(_viv_preload_fd->cFileName,fd->cFileName) == 0))
	{
		_viv_open_preload();

		return;
	}
	else
	if ((!is_preload) && (_viv_load_image_thread) && (_viv_load_image_filename) && (string_compare(_viv_load_image_filename,fd->cFileName) == 0))
	{
		debug_printf("already loading...\n");
		// already loading this one..
		return;
	}
	
	// clear any existing preload and start a fresh one.
	_viv_clear_preload();
	
	if (_viv_load_image_thread)
	{
		// already loading a different image.
		// add it to the queue and cancel this one.
		debug_printf("SET TERMINATE (next)\n");
		_viv_load_image_terminate = 1;
		
		if (_viv_load_image_next_fd)
		{
			mem_free(_viv_load_image_next_fd);
		}
		
		_viv_load_image_next_fd = (WIN32_FIND_DATA *)mem_alloc(sizeof(WIN32_FIND_DATA));
		os_copy_memory(_viv_load_image_next_fd,fd,sizeof(WIN32_FIND_DATA));
		_viv_load_image_next_is_preload = is_preload;
		
		_viv_status_update();
	}
	else
	{
		DWORD thread_id;
//		int rw;
//		int rh;
		RECT rect;

		GetClientRect(_viv_hwnd,&rect);

//		_viv_get_render_size(&rw,&rh);

		_viv_load_image_allow_draw = 1;
		_viv_load_image_terminate = 0;
		
		if (_viv_load_image_filename)
		{
			mem_free(_viv_load_image_filename);
		}
		
		_viv_load_image_filename = string_alloc(fd->cFileName);
		_viv_load_is_preload = is_preload;
		_viv_preload_is_prev = _viv_last_is_prev;
		_viv_preload_state = 0;
		_viv_should_activate_preload_on_load = 0;
		_viv_load_render_wide = rect.right - rect.left;
		_viv_load_render_high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_controls_high();
		_viv_load_frame_count = 0;
		os_copy_memory(_viv_load_fd,fd,sizeof(WIN32_FIND_DATA));
		
		if (is_preload)
		{
			os_copy_memory(_viv_preload_fd,fd,sizeof(WIN32_FIND_DATA));
		}
		
debug_printf("LOAD %S is_preload %d\n",_viv_load_image_filename,is_preload);

		_viv_load_image_thread = CreateThread(NULL,0,_viv_load_image_thread_proc,0,0,&thread_id);

		_viv_status_update();
	}
	
	if (!is_preload)
	{
		os_copy_memory(_viv_current_fd,fd,sizeof(WIN32_FIND_DATA));
		
		_viv_update_title();
	}
}

static void _viv_on_size(void)
{
	if (!_viv_prevent_on_size)
	{
		RECT rect;
		int wide;
		int high;
		
		GetClientRect(_viv_hwnd,&rect);
		wide = rect.right - rect.left;
		high = rect.bottom - rect.top;

		if (!IsIconic(_viv_hwnd))
		{
			if (!_viv_is_fullscreen)
			{
				int is_maximized;
				
				is_maximized = _viv_is_window_maximized(_viv_hwnd);
				
				config_maximized = is_maximized;
				
				if (!is_maximized)
				{
					RECT window_rect;
					
					GetWindowRect(_viv_hwnd,&window_rect);
					
					config_wide = window_rect.right - window_rect.left;
					config_high = window_rect.bottom - window_rect.top;
				}
			}
		}
		
		if (_viv_status_hwnd)
		{
			_viv_status_update();
			
			SendMessage(_viv_status_hwnd,WM_SIZE,0,0);
			
			high -= _viv_get_status_high();
		}

		if (_viv_toolbar_hwnd)
		{
			int toolbar_wide;
			
			toolbar_wide = _viv_toolbar_get_wide();
			
			SetWindowPos(_viv_rebar_hwnd,0,0,high - _viv_get_controls_high(),wide,_viv_get_controls_high(),SWP_NOZORDER|SWP_NOACTIVATE);
			SetWindowPos(_viv_toolbar_hwnd,0,(wide / 2) - (toolbar_wide /2),6,toolbar_wide,_viv_get_controls_high() - 6,SWP_NOZORDER|SWP_NOACTIVATE);
			
			high -= _viv_get_controls_high();
		}
		
		{
		
			int rw;
			int rh;
			double new_view_x;
			double new_view_y;

			_viv_get_render_size(&rw,&rh);
		
			new_view_x = (int)(((_viv_view_ix * rw) / _viv_image_wide) + 0.5) + (((_viv_dst_pos_x - 250) * (wide*2)) / 1000) - (wide / 2) - (rw / 2);
			new_view_y = (int)(((_viv_view_iy * rh) / _viv_image_high) + 0.5) + (((_viv_dst_pos_y - 250) * (high*2)) / 1000) - (high / 2) - (rh / 2);
	
	debug_printf("RESTORE VIEW x %d y %d ix %d iy %d rw %d rh %d wide %d high %d\n",(int)(new_view_x),(int)(new_view_y),(int)_viv_view_ix,(int)_viv_view_iy,rw,rh,wide,high)		;
	//		_viv_view_set((int)(_viv_view_ix * (double)rw),(int)(_viv_view_iy * (double)rh),1);
			_viv_view_set((int)new_view_x,(int)new_view_y,1);
		}

		_viv_toolbar_update_buttons();
	}
}

static void _viv_command(int command_id)
{
	_viv_command_with_is_key_repeat(command_id,0);
}

static void _viv_command_with_is_key_repeat(int command_id,int is_key_repeat)
{
	// Parse the menu selections:
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
			_viv_increase_rate(1);
			break;
			
		case VIV_ID_SLIDESHOW_RATE_INC:
			_viv_increase_rate(0);
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
				client_high = rect.bottom - rect.top + _viv_get_status_high() + _viv_get_controls_high();
				
				AdjustWindowRect(&rect,GetWindowStyle(_viv_hwnd),GetMenu(_viv_hwnd) ? TRUE : FALSE);

debug_printf("%d %d | %d %d %d %d\n",midx,midy,rect.left,rect.top,rect.right,rect.bottom);
				
				wide = rect.right - rect.left;
				high = rect.bottom - rect.top + _viv_get_status_high() + _viv_get_controls_high();
				
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
			
		case VIV_ID_VIEW_PANSCAN_INCREASE_SIZE:
			_viv_dst_zoom_set(_viv_dst_zoom_x_pos + 1,_viv_dst_zoom_y_pos + 1);
			break;
			
		case VIV_ID_VIEW_PANSCAN_DECREASE_SIZE:
			_viv_dst_zoom_set(_viv_dst_zoom_x_pos - 1,_viv_dst_zoom_y_pos - 1);
			break;
			
		case VIV_ID_VIEW_PANSCAN_INCREASE_WIDTH:
			_viv_dst_zoom_set(_viv_dst_zoom_x_pos + 1,_viv_dst_zoom_y_pos);
			break;
			
		case VIV_ID_VIEW_PANSCAN_DECREASE_WIDTH:
			_viv_dst_zoom_set(_viv_dst_zoom_x_pos - 1,_viv_dst_zoom_y_pos);
			break;
			
		case VIV_ID_VIEW_PANSCAN_INCREASE_HEIGHT:
			_viv_dst_zoom_set(_viv_dst_zoom_x_pos,_viv_dst_zoom_y_pos + 1);
			break;
			
		case VIV_ID_VIEW_PANSCAN_DECREASE_HEIGHT:
			_viv_dst_zoom_set(_viv_dst_zoom_x_pos,_viv_dst_zoom_y_pos - 1);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_RIGHT:
			_viv_dst_pos_set(_viv_dst_pos_x + 5,_viv_dst_pos_y);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_LEFT:
			_viv_dst_pos_set(_viv_dst_pos_x - 5,_viv_dst_pos_y);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_UP:
			_viv_dst_pos_set(_viv_dst_pos_x,_viv_dst_pos_y - 5);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_DOWN:
			_viv_dst_pos_set(_viv_dst_pos_x,_viv_dst_pos_y + 5);
			break;

		case VIV_ID_VIEW_PANSCAN_MOVE_UP_LEFT:
			_viv_dst_pos_set(_viv_dst_pos_x - 5,_viv_dst_pos_y - 5);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_UP_RIGHT:
			_viv_dst_pos_set(_viv_dst_pos_x + 5,_viv_dst_pos_y - 5);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_DOWN_LEFT:
			_viv_dst_pos_set(_viv_dst_pos_x - 5,_viv_dst_pos_y + 5);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_DOWN_RIGHT:
			_viv_dst_pos_set(_viv_dst_pos_x + 5,_viv_dst_pos_y + 5);
			break;
			
		case VIV_ID_VIEW_PANSCAN_MOVE_CENTER:
			_viv_dst_pos_x = 500;
			_viv_dst_pos_y = 500;
			InvalidateRect(_viv_hwnd,NULL,FALSE);
			_viv_status_update_temp_pos_zoom();	
			break;
			
		case VIV_ID_VIEW_PANSCAN_RESET:
			_viv_dst_pos_x = 500;
			_viv_dst_pos_y = 500;
			InvalidateRect(_viv_hwnd,NULL,FALSE);
			_viv_dst_zoom_set(_VIV_DST_ZOOM_ONE,_VIV_DST_ZOOM_ONE);		
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
			_viv_options();
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
				
				string_printf(filter_wbuf,"%s (*.bmp;*.gif;*.ico;*.jpeg;*.jpg;*.png;*.tif;*.tiff;*.webp)%c*.bmp;*.gif;*.ico;*.jpeg;*.jpg;*.png;*.tif;*.tiff;*.webp%c%s (*.*)%c*.*%c",localization_get_string(LOCALIZATION_OPEN_ALL_IMAGE_FILES),0,0,localization_get_string(LOCALIZATION_OPEN_ALL_FILES),0,0);

				string_copy_utf8(title_wbuf,localization_get_string(LOCALIZATION_OPEN_IMAGE_CAPTION));
				
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

				string_printf(filter_wbuf,"%s (*.*)%c*.*%c",localization_get_string(LOCALIZATION_OPEN_ALL_FILES),0,0);

				string_copy_utf8(title_wbuf,localization_get_string(command_id == VIV_ID_EDIT_COPY_TO ? LOCALIZATION_COPY_TO_CAPTION : LOCALIZATION_MOVE_TO_CAPTION));
				
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
					
					tobuf[string_length(tobuf) + 1] = 0;
					
					os_zero_memory(&shfileop,sizeof(SHFILEOPSTRUCT));
					
					string_copy(frombuf,_viv_current_fd->cFileName);
					frombuf[string_length(frombuf) + 1] = 0;
					
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

static void _viv_exit(void)
{
	_viv_load_image_terminate = 1;
	config_save_settings(config_appdata);
	PostQuitMessage(0);
}

static LRESULT CALLBACK _viv_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) 
	{	
		case WM_NCHITTEST:

			if (!_viv_is_fullscreen)
			{
				if (DefWindowProc(hwnd,msg,wParam,lParam) == HTCLIENT)
				{
					if (!config_show_thickframe)
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
			
			if (config_toolbar_move_window)
			{
				if (wParam == HTMENU)
				{
					HMENU hmenu;
					
					hmenu = GetMenu(hwnd);
					
					if (hmenu)
					{
						POINT pt;
						
						pt.x = GET_X_LPARAM(lParam);
						pt.y = GET_Y_LPARAM(lParam);
						
						if (MenuItemFromPoint(hwnd,hmenu,pt) == -1)
						{
							_viv_start_move_window();
							
							return 0;
						}
					}
				}
			}

			if (!_viv_is_fullscreen)
			{
				if (!config_show_thickframe)
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
			_viv_hmenu = 0;
			break;
			
		case WM_QUERYENDSESSION:
			return TRUE;

		case WM_ENDSESSION:
			if (wParam)
			{
				// save settings on logout.
				config_save_settings(config_appdata);
			}
			return 0;
			
		case _VIV_WM_RETRY_RANDOM_EVERYTHING_SEARCH:
			_viv_send_random_everything_search();
			break;
			
		case _VIV_WM_REPLY:
		{
			_viv_reply_t *e;
			
			EnterCriticalSection(&_viv_cs);
			e = _viv_reply_start;
			_viv_reply_start = 0;
			_viv_reply_last = 0;
			LeaveCriticalSection(&_viv_cs);
			
			while(e)
			{
				_viv_reply_t *next_e;
				
				next_e = e->next;
				
				switch(e->type)
				{
					case _VIV_REPLY_LOAD_IMAGE_COMPLETE:
					case _VIV_REPLY_LOAD_IMAGE_FAILED:
					
						{
							int allow_preload_next;
							
							allow_preload_next = 0;

							debug_printf((e->type == _VIV_REPLY_LOAD_IMAGE_FAILED) ? "_VIV_REPLY_LOAD_IMAGE_FAILED\n" : "_VIV_REPLY_LOAD_IMAGE_COMPLETE\n");
							
							if (_viv_load_image_terminate)
							{
		debug_printf("LOADED/FAILED TERMINATE\n");
								// do nothing.
								// let terminator handle it.
							}
							else
							{
								if (_viv_load_is_preload)
								{
									if (e->type == _VIV_REPLY_LOAD_IMAGE_FAILED)
									{
										if (_viv_should_activate_preload_on_load)
										{
											// preload next below..
											allow_preload_next = 1;

											_viv_load_failed = 1;
											_viv_clear();
											_viv_start_first_frame();
											_viv_process_pending_clear();

											// we update status below.
										}

										_viv_preload_state = 2;
									}
									else
									{
										if (_viv_should_activate_preload_on_load)
										{
											// preload next below..
											allow_preload_next = 1;
											
											_viv_activate_preload();
										}

										_viv_preload_state = 1;

										// we update status below.
									}
								}
								else
								{
									allow_preload_next = 1;
									
									if (e->type == _VIV_REPLY_LOAD_IMAGE_FAILED)
									{
										_viv_load_failed = 1;
										_viv_clear();
										_viv_start_first_frame();
										_viv_process_pending_clear();

										// we update status below.
									}
								}
							}
							
							if (_viv_load_image_thread)
							{
								CloseHandle(_viv_load_image_thread);
								
								_viv_load_image_thread = 0;
							}
						
							if (_viv_load_image_filename)
							{
								mem_free(_viv_load_image_filename);

								_viv_load_image_filename = 0;
							}
							
							if (_viv_load_image_next_fd)
							{
								WIN32_FIND_DATA *fd;
								
								fd = _viv_load_image_next_fd;
								_viv_load_image_next_fd = NULL;

debug_printf("NEXT AFTER LOAD %S\n",fd->cFileName);
								_viv_open(fd,_viv_load_image_next_is_preload);
								
								mem_free(fd);
							}
							
							_viv_status_update();
							_viv_toolbar_update_buttons();
							
							// preload next
							if (allow_preload_next)
							{
								_viv_preload_next();
							}
						}
							
						break;
										
					case _VIV_REPLY_LOAD_IMAGE_FIRST_FRAME:
						
						{
							_viv_reply_load_image_first_frame_t *first_frame;

							debug_printf("_VIV_REPLY_LOAD_IMAGE_FIRST_FRAME is preload %d activate %d\n",_viv_load_is_preload,_viv_should_activate_preload_on_load);
							
							first_frame = (_viv_reply_load_image_first_frame_t *)(e + 1);
							
							// always show the first frame.
							// if we check for the terminate flag and hold down right, we might never see an image.
							// 
							// make sure we terminate the preload, otherwise it might get shown unexpectedly.
							
							if ((_viv_load_image_terminate) && (!_viv_load_image_allow_draw) && (!((_viv_load_is_preload) && (_viv_should_activate_preload_on_load))))
							{
		debug_printf("FIRST FRAME TERMINATE\n");
								if (first_frame->frame.hbitmap)
								{
									DeleteObject(first_frame->frame.hbitmap);
									
									first_frame->frame.hbitmap = NULL;
								}
								
								if (first_frame->frame.mipmap)
								{
									_viv_mipmap_free(first_frame->frame.mipmap);
									
									first_frame->frame.mipmap = NULL;
								}
							}
							else
							{
								if ((_viv_load_is_preload) && (_viv_should_activate_preload_on_load))
								{
									// _viv_should_activate_preload_on_load was set to 1 AFTER we started loading and before the first frame loaded.
									// just load as normal..
									_viv_load_is_preload = 0;
									_viv_status_update();
								}
								
								_viv_load_frame_count = 1;
								
								if (_viv_load_is_preload)
								{
									_viv_clear_preload_frames();
									
									_viv_preload_image_wide = first_frame->wide;
									_viv_preload_image_high = first_frame->high;
									_viv_preload_frame_count = first_frame->frame_count;

									// allocate hbitmaps.
									_viv_preload_frames = (_viv_frame_t *)mem_alloc(sizeof(_viv_frame_t) * _viv_preload_frame_count);
									_viv_preload_frames[0].hbitmap = first_frame->frame.hbitmap;
									_viv_preload_frames[0].mipmap = first_frame->frame.mipmap;
									_viv_preload_frames[0].delay = first_frame->frame.delay;

									first_frame->frame.hbitmap = 0;
									first_frame->frame.mipmap = NULL;
									_viv_preload_frame_loaded_count = 1;
									
									_viv_status_update();
								}
								else
								{
									// copy the image, not the filename.
									viv_copy_current_image_to_last_image();

									_viv_clear();

									_viv_image_wide = first_frame->wide;
									_viv_image_high = first_frame->high;
									_viv_frame_count = first_frame->frame_count;

									// allocate hbitmaps.
									_viv_frames = mem_alloc(sizeof(_viv_frame_t) * _viv_frame_count);
									_viv_frames[0].hbitmap = first_frame->frame.hbitmap;
									_viv_frames[0].mipmap = first_frame->frame.mipmap;
									_viv_frames[0].delay = first_frame->frame.delay;
									os_copy_memory(_viv_frame_fd,_viv_load_fd,sizeof(WIN32_FIND_DATA));

									first_frame->frame.hbitmap = 0;
									first_frame->frame.mipmap = NULL;
									_viv_frame_loaded_count = 1;
									
									_viv_start_first_frame();
									
									_viv_process_pending_clear();
								}
							}
						}
						
						break;

					case _VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME:
						
						{
							_viv_frame_t *additional_frame;
							
							additional_frame = (_viv_frame_t *)(e + 1);
							
							debug_printf("_VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME %d preload %d activate %d\n",_viv_load_is_preload ? _viv_preload_frame_loaded_count : _viv_frame_loaded_count,_viv_load_is_preload,_viv_should_activate_preload_on_load);
							
							if ((_viv_load_image_terminate) && (!_viv_load_image_allow_draw) && (!((_viv_load_is_preload) && (_viv_should_activate_preload_on_load))))
							{
		debug_printf("ADDITIONAL FRAME TERMINATE\n");
								if (additional_frame->hbitmap)
								{
									DeleteObject(additional_frame->hbitmap);
									
									additional_frame->hbitmap = NULL;
								}
								
								if (additional_frame->mipmap)
								{
									_viv_mipmap_free(additional_frame->mipmap);
									
									additional_frame->mipmap = NULL;
								}
							}
							else
							{
								if ((_viv_load_is_preload) && (_viv_should_activate_preload_on_load))
								{
									// _viv_should_activate_preload_on_load was set to 1 AFTER we started loading and before the first frame loaded.
									// just load as normal..
									_viv_load_is_preload = 0;
									_viv_activate_preload();
									_viv_status_update();
								}
								
								if (_viv_load_is_preload)
								{
									// we could have been cleared.
									if (_viv_preload_frames)
									{
										_viv_preload_frames[_viv_preload_frame_loaded_count].hbitmap = additional_frame->hbitmap;
										_viv_preload_frames[_viv_preload_frame_loaded_count].mipmap = additional_frame->mipmap;
										_viv_preload_frames[_viv_preload_frame_loaded_count].delay = additional_frame->delay;
										
										_viv_preload_frame_loaded_count++;
										
										additional_frame->hbitmap = 0;
										additional_frame->mipmap = 0;
									}
								}
								else
								{
									// we could have been cleared.
									if (_viv_frames)
									{
										// make sure we check the frame count too
										// incase we get an event from an old load.
										if (_viv_frame_loaded_count < _viv_frame_count)
										{
											_viv_frames[_viv_frame_loaded_count].hbitmap = additional_frame->hbitmap;
											_viv_frames[_viv_frame_loaded_count].mipmap = additional_frame->mipmap;
											_viv_frames[_viv_frame_loaded_count].delay = additional_frame->delay;
											
											_viv_frame_loaded_count++;
											
											((_viv_frame_t *)(e + 1))->hbitmap = 0;
											((_viv_frame_t *)(e + 1))->mipmap = 0;
										}
									}
								}
								
								_viv_load_frame_count++;
							}
						}
						
						break;
				}

				_viv_reply_free(e);
				
				e = next_e;
			}
			
			break;
		}
			
		case WM_INITMENU:
		{	
			HMENU hmenu;

			hmenu = GetMenu(hwnd);

			_viv_check_menus(hmenu);
			
			break;
		}

		case WM_DROPFILES:
		{
			wchar_t filename[STRING_SIZE];
			DWORD count;
			int is_shift;
			
			if (_viv_random)
			{
				mem_free(_viv_random);
				
				_viv_random = 0;
			}
			
			is_shift = (GetKeyState(VK_SHIFT) < 0);
			if (is_shift)
			{
				// add current?
				_viv_playlist_add_current_if_empty();
			}
			else
			{
				_viv_playlist_clearall();
			}
			
			count = DragQueryFile((HDROP)wParam,0xFFFFFFFF,0,0);
			
			if ((count >= 2) || (is_shift))
			{
				DWORD i;
				
				for(i=0;i<count;i++)
				{
					DragQueryFile((HDROP)wParam,i,filename,STRING_SIZE);
				
					_viv_playlist_add_filename(filename);
				}
			
				if (!is_shift)
				{
					_viv_home(0,0);
				}
			}
			else
			if (count == 1)
			{
				DragQueryFile((HDROP)wParam,0,filename,STRING_SIZE);
			
				_viv_open_from_filename(filename);
			}
			
			SetForegroundWindow(hwnd);
			
			break;
		}
			
		case WM_TIMER:
			
			switch(wParam)
			{
				case VIV_ID_STATUS_TEMP_TEXT_TIMER:
					_viv_status_set_temp_text(0);
					break;

				case VIV_ID_SLIDESHOW_TIMER:

					if (_viv_is_slideshow)
					{
						if (config_loop_animations_once)
						{
							if (_viv_frame_count > 1)
							{
								if (!_viv_frame_looped)
								{
									_viv_is_slideshow_timeup = 1;
								
									break;
								}
							}
						}
						
						_viv_next(0,0,0,0);
					}
					
					break;
					
				case VIV_ID_HIDE_CURSOR_TIMER:
					if (_viv_is_hide_cursor_timer)
					{
						if (!_viv_should_show_cursor())
						{
							_viv_hide_cursor();
						}
					}
					break;
					
				case VIV_ID_ANIMATION_TIMER:
				{
					if ((_viv_is_animation_timer) && (_viv_frame_count))
					{
						VIV_UINT64 elapsed;
						VIV_UINT64 tick;
						int invalidate;
						VIV_UINT64 freq;
						
						invalidate = 0;
						
						tick = os_get_tick_count();
						freq = os_get_tick_freq();
						
						elapsed = tick - _viv_animation_timer_tick_start;
						_viv_animation_timer_tick_start = tick;
						
						// what happened?
						// don't elapse more than one second at a time.
						if (elapsed > freq)
						{
							elapsed = freq;
						}
						
						if (_viv_animation_play)
						{
							DWORD frames_skipped;
//debug_printf("%p %p\n",_viv_timer_tick,elapsed)							;
							
							_viv_timer_tick += elapsed;
							
							frames_skipped = 0;
							
							for(;;)
							{
								DWORD delay;
								VIV_UINT64 performance_counter_delay;

								delay = _viv_frames[_viv_frame_position].delay * (1.0f/_viv_animation_rates[_viv_animation_rate_pos]);
								
								if (!delay)
								{
									delay = 1;
								}
								
								performance_counter_delay = (delay * freq) / 1000;

								// debug_printf("delay %d\n",delay);
				
								if (_viv_timer_tick >= performance_counter_delay)
								{
									//debug_printf("%d %d error %d %d\n",(DWORD)_viv_timer_tick,(DWORD)freq,(DWORD)(_viv_timer_tick - performance_counter_delay),(DWORD)(((_viv_timer_tick - performance_counter_delay) * 1000) / freq));
									
									if (_viv_frame_loaded_count != _viv_frame_count)
									{
										if (_viv_frame_position + 1 >= _viv_frame_loaded_count)
										{	
											// ignore this tick
											frames_skipped++;
											_viv_timer_tick = 0;
											break;
										}
									}
								
									_viv_frame_position++;
									if (_viv_frame_position == _viv_frame_count)
									{
										_viv_frame_looped = 1;
										_viv_frame_position = 0;

										if (config_loop_animations_once)
										{
											if (_viv_is_slideshow_timeup)
											{
												_viv_next(0,1,0,0);
												
												break;
											}
										}
									}

									if (invalidate)
									{
										frames_skipped++;
									}
									
									invalidate = 1;

									_viv_timer_tick -= performance_counter_delay;
								}
								else
								{
									break;
								}
							}
							
							if (frames_skipped)
							{
								debug_printf("frames skipped %d\n",frames_skipped);
							}
						}					
						
//debug_printf("_viv_frame_position %u\n",_viv_frame_position);
						if (invalidate)
						{
							_viv_update_src_pixel(1,0);
							_viv_status_update();
							InvalidateRect(hwnd,0,FALSE);
							
							if (_viv_is_animation_paint)
							{
								debug_printf("paint frame dropped\n");
							}
							
							_viv_is_animation_paint = 1;
							//UpdateWindow(hwnd);
						}
					}
					
					_viv_is_animation_timer_event = 0;

					break;
				}
			}
			
			break;
			
		case WM_LBUTTONDBLCLK:

			_viv_show_cursor();
			_viv_update_show_cursor();

			// 0 = scroll, 1 = play/pause slideshow, 2 = play/pause animation, 3=zoom in, 4=next, 5=1:1 scroll
			switch(config_left_click_action)
			{
				case 0:
				case 1:
				case 2:
				case 5:
				case 6:
					_viv_toggle_fullscreen();
					break;

				default:
					_viv_do_left_click_action(config_left_click_action);
					break;
			}
			break;
			
		case WM_LBUTTONDOWN:
		
			_viv_show_cursor();
			_viv_update_show_cursor();
			
			_viv_do_left_click_action(config_left_click_action);
		
			break;
			
		case WM_MBUTTONDOWN:
			if (_viv_doing == _VIV_DOING_NOTHING)
			{
				SetCapture(hwnd);
				_viv_doing = _VIV_DOING_MSCROLL;
				_viv_doing_x = GET_X_LPARAM(lParam);
				_viv_doing_y = GET_Y_LPARAM(lParam);
				{
					POINT pt;
					GetCursorPos(&pt);
					
					_viv_mdoing_x = pt.x;
					_viv_mdoing_y = pt.y;
				}
				ShowCursor(FALSE);
			}
			break;
			
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:

			switch(config_right_click_action)
			{
				case 1:
					_viv_zoom_in(1,1,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
					return 0;

				case 2:
					_viv_next(1,1,0,0);
					return 0;
			}

			break;		
			
		case WM_RBUTTONUP:

			switch(config_right_click_action)
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
				
				for(i=0;i<_VIV_CONEXT_MENU_ITEM_COUNT;i++)
				{
					if (_viv_context_menu_items[i] > _VIV_MENU_COUNT)
					{
						switch (_viv_context_menu_items[i])
						{
							case VIV_ID_FILE_PREVIEW:
								// this doesn't exist on Windows 8 or later.
								if (os_is_windows_8_or_later())
								{
									continue;
								}
								break;
						}
						
						if ((_viv_context_menu_items[i] != VIV_ID_VIEW_MENU) || (!config_show_menu))
						{
							int command_index;
							
							command_index = _viv_command_index_from_command_id(_viv_context_menu_items[i]);
							
							// its a command
							if (command_index != -1)
							{
								int key_command_index;
								wchar_t text_wbuf[STRING_SIZE];
								wchar_t key_text[STRING_SIZE];
								
								key_command_index = command_index;
								
								switch(_viv_commands[command_index].command_id)
								{
									case VIV_ID_SLIDESHOW_PAUSE:
										string_copy_utf8(text_wbuf,localization_get_string(LOCALIZATION_ID_PLAY_PAUSE));
										break;
										
									default:
										string_copy_utf8(text_wbuf,localization_get_string(_viv_commands[command_index].localization_id));
										break;
										
								}
								
								switch (_viv_commands[command_index].command_id)
								{
									
									case VIV_ID_FILE_DELETE:
										key_command_index = _viv_command_index_from_command_id(VIV_ID_FILE_DELETE_RECYCLE);
										break;
										
								}

								if (key_command_index >= 0)
								{
									if (_viv_key_list->start[key_command_index])
									{
										_viv_get_key_text(key_text,_viv_key_list->start[key_command_index]->key);
										
										string_cat_utf8(text_wbuf,(const utf8_t *)"\t");
										string_cat(text_wbuf,key_text);
									}
								}
									
								AppendMenu(curmenu,_viv_commands[command_index].flags & (~(MF_DELETE|MF_OWNERDRAW)),_viv_commands[command_index].command_id,text_wbuf);
								was_seperator = 0;
							}
						}
					}
					else
					if (_viv_context_menu_items[i])
					{
						if (submenuid == _viv_context_menu_items[i])
						{
							// pop submenu
							curmenu = hmenu;
							submenuid = 0;
						}
						else
						{
							int command_index;
							
							command_index = _viv_command_index_from_command_id(_viv_context_menu_items[i]);
							if (command_index != -1)
							{
								wchar_t text_wbuf[STRING_SIZE];
								
								// push submenu
								switch(_viv_context_menu_items[i])
								{
									case _VIV_MENU_SLIDESHOW_RATE:
										string_copy_utf8(text_wbuf,localization_get_string(LOCALIZATION_ID_RATE));
										break;
								
									default:
										string_copy_utf8(text_wbuf,localization_get_string(_viv_commands[command_index].localization_id));
										break;
								}
								
								submenuid = _viv_context_menu_items[i];
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
	
			_viv_check_menus(hmenu);
			
			_viv_show_cursor();
			
			_viv_in_popup_menu = 1;
			
			TrackPopupMenu(hmenu,tpm_flags,pt.x,pt.y,0,hwnd,0);

			// start the hide cursor timer again.
			_viv_in_popup_menu = 0;
			_viv_update_show_cursor();
			
			DestroyMenu(hmenu);
			
			break;
		}
			
		case WM_ACTIVATE:
			
			if (wParam == WA_INACTIVE)
			{
				if (!_viv_prevent_on_deactivate)
				{
					_viv_show_cursor();
				}
			}
			else
			{
				_viv_update_show_cursor();
			}
			break;
			
		case WM_MOUSELEAVE:
		
			_viv_is_tracking_mouse = 0;
			_viv_is_mouseover = 0;
			
			_viv_mousemove_x = -1;
			_viv_mousemove_y = -1;

			if ((-1 != _viv_src_pixel_x) || (-1 != _viv_src_pixel_y))
			{
				_viv_src_pixel_x = -1;
				_viv_src_pixel_y = -1;

				if (config_pixel_info)
				{
					_viv_status_update();
				}
			}
			
			_viv_show_cursor();
//			_viv_tooltip_hide();
			_viv_update_src_pixel(0,1);

			break;
			
		case WM_MOUSEMOVE:

			if (!_viv_is_tracking_mouse)
			{
				TRACKMOUSEEVENT tme;
				
				// ui must come first, as TrackMouseEvent can SEND WM_MOUSELEAVE
				_viv_is_tracking_mouse = 1;
				
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.dwHoverTime = 0;
				tme.hwndTrack = hwnd;
				
				_TrackMouseEvent(&tme);
			}
			
			_viv_is_mouseover = 1;
							
			_viv_mousemove();
			
			switch(_viv_doing)
			{
				case _VIV_DOING_MSCROLL:

					{
						int mx;
						int my;
						POINT pt;
						GetCursorPos(&pt);

						mx = _viv_mdoing_x - pt.x;
						my = _viv_mdoing_y - pt.y;
						
						if ((mx) || (my))
						{
							SetCursorPos(_viv_mdoing_x,_viv_mdoing_y);
						}
					}
					
					break;
			
				case _VIV_DOING_SCROLL:

					{
						int mx;
						int my;
						int x;
						int y;
						
						x = GET_X_LPARAM(lParam); 
						y = GET_Y_LPARAM(lParam); 

						mx = x - _viv_doing_x;
						my = y - _viv_doing_y;
						
						_viv_doing_x = x;
						_viv_doing_y = y;
					
						if ((mx) || (my))
						{
							_viv_view_scroll(mx,my);
		//					UpdateWindow(hwnd);
						}				
					}
					
					break;

				case _VIV_DOING_1TO1SCROLL:

					_viv_update_1to1_scroll(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
					
					break;					
			}
			
			_viv_update_src_pixel(0,1);
			
			break;
			
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		
			_viv_doing_cancel();
			
			break;
			
		case WM_MOUSEWHEEL:
		{
			_viv_do_mousewheel_action(_viv_get_current_key_mod_flags() == CONFIG_KEYFLAG_CTRL ? config_ctrl_mouse_wheel_action : config_mouse_wheel_action,GET_WHEEL_DELTA_WPARAM(wParam),GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
			
			break;
		}
		
		case WM_COPYDATA:
		{
			COPYDATASTRUCT *cds;
				
			cds = (COPYDATASTRUCT *)lParam;

			switch(cds->dwData)
			{
				// execute command line options
				case _VIV_COPYDATA_COMMAND_LINE:
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
						
						debug_printf("set cwd %S\n",cwd);
						SetCurrentDirectory(cwd);
						
						_viv_process_command_line(cl);
						
						ShowWindow(hwnd,showcmd);
					}
										
					return 1;
				}
				
				// Everything reply
				case _VIV_COPYDATA_RANDOM_EVERYTHING_SEARCH:
				{
					EVERYTHING_IPC_ITEM2 *items;
					
					debug_printf("%d / %d results\n",((EVERYTHING_IPC_LIST2 *)cds->lpData)->numitems,((EVERYTHING_IPC_LIST2 *)cds->lpData)->totitems);
					
					items = (EVERYTHING_IPC_ITEM2 *)(((EVERYTHING_IPC_LIST2 *)cds->lpData) + 1);
					
					if (((EVERYTHING_IPC_LIST2 *)cds->lpData)->numitems)
					{
						if (items[0].flags & EVERYTHING_IPC_FOLDER)
						{
							// add this folder ?
//							_viv_playlist_add_path(full_path_and_filename);
						}
						else
						{
							DWORD filename_len;
							char *p;

							// EVERYTHING_IPC_QUERY2_REQUEST_FULL_PATH_AND_NAME
							p = ((char *)cds->lpData) + items[0].data_offset;
							
							filename_len = *(DWORD *)p;
							if (filename_len < MAX_PATH)
							{
								WIN32_FIND_DATA fd;

								p += sizeof(DWORD);
								
								os_zero_memory(&fd,sizeof(WIN32_FIND_DATA));
								
								os_copy_memory(fd.cFileName,(wchar_t *)p,filename_len * sizeof(wchar_t));
								fd.cFileName[filename_len] = 0;
								
								if (_viv_is_valid_filename(&fd))
								{
									p += (filename_len + 1) * sizeof(wchar_t);
									
									// EVERYTHING_IPC_QUERY2_REQUEST_SIZE	
									if (_viv_everything_request_flags & EVERYTHING_IPC_QUERY2_REQUEST_SIZE)
									{
										fd.nFileSizeLow = *(DWORD *)p;
										p += sizeof(DWORD);
										fd.nFileSizeHigh = *(DWORD *)p;
										p += sizeof(DWORD);
									}

									// EVERYTHING_IPC_QUERY2_REQUEST_DATE_CREATED
									if (_viv_everything_request_flags & EVERYTHING_IPC_QUERY2_REQUEST_DATE_CREATED)
									{
										fd.ftCreationTime.dwLowDateTime = *(DWORD *)p;
										p += sizeof(DWORD);
										fd.ftCreationTime.dwHighDateTime = *(DWORD *)p;
										p += sizeof(DWORD);
									}
									
									// EVERYTHING_IPC_QUERY2_REQUEST_DATE_MODIFIED
									if (_viv_everything_request_flags & EVERYTHING_IPC_QUERY2_REQUEST_DATE_MODIFIED)
									{
										fd.ftLastWriteTime.dwLowDateTime = *(DWORD *)p;
										p += sizeof(DWORD);
										fd.ftLastWriteTime.dwHighDateTime = *(DWORD *)p;
										p += sizeof(DWORD);
									}
									
									_viv_open(&fd,0);
								}
							}
						}
					}
					else
					if (((EVERYTHING_IPC_LIST2 *)cds->lpData)->totitems)
					{
						// our random index was too high, try again.
						_viv_random_tot_results = ((EVERYTHING_IPC_LIST2 *)cds->lpData)->totitems;

						PostMessage(hwnd,_VIV_WM_RETRY_RANDOM_EVERYTHING_SEARCH,0,0);
					}

					break;
				}

				// Everything reply
				case _VIV_COPYDATA_OPEN_EVERYTHING_SEARCH:
				case _VIV_COPYDATA_ADD_EVERYTHING_SEARCH:
				{
					DWORD i;
					EVERYTHING_IPC_ITEM2 *items;
					
					if (_viv_random)
					{
						mem_free(_viv_random);
						
						_viv_random = 0;
					}
					
					if (cds->dwData == _VIV_COPYDATA_OPEN_EVERYTHING_SEARCH)
					{
						_viv_playlist_clearall();
					}
					else
					{
						_viv_playlist_add_current_if_empty();
					}
					
					debug_printf("%d / %d results\n",((EVERYTHING_IPC_LIST2 *)cds->lpData)->numitems,((EVERYTHING_IPC_LIST2 *)cds->lpData)->totitems);
					
					items = (EVERYTHING_IPC_ITEM2 *)(((EVERYTHING_IPC_LIST2 *)cds->lpData) + 1);
					
					for(i=0;i<((EVERYTHING_IPC_LIST2 *)cds->lpData)->numitems;i++)
					{
						if (items[i].flags & EVERYTHING_IPC_FOLDER)
						{
							// add this folder ?
//							_viv_playlist_add_path(full_path_and_filename);
						}
						else
						{
							DWORD filename_len;
							char *p;

							// EVERYTHING_IPC_QUERY2_REQUEST_FULL_PATH_AND_NAME
							p = ((char *)cds->lpData) + items[i].data_offset;
							
							filename_len = *(DWORD *)p;
							if (filename_len < MAX_PATH)
							{
								WIN32_FIND_DATA fd;

								p += sizeof(DWORD);
								
								os_zero_memory(&fd,sizeof(WIN32_FIND_DATA));
								
								os_copy_memory(fd.cFileName,(wchar_t *)p,filename_len * sizeof(wchar_t));
								fd.cFileName[filename_len] = 0;
								
								if (_viv_is_valid_filename(&fd))
								{
									p += (filename_len + 1) * sizeof(wchar_t);
									
									// EVERYTHING_IPC_QUERY2_REQUEST_SIZE	
									if (_viv_everything_request_flags & EVERYTHING_IPC_QUERY2_REQUEST_SIZE)
									{
										fd.nFileSizeLow = *(DWORD *)p;
										p += sizeof(DWORD);
										fd.nFileSizeHigh = *(DWORD *)p;
										p += sizeof(DWORD);
									}

									// EVERYTHING_IPC_QUERY2_REQUEST_DATE_CREATED
									if (_viv_everything_request_flags & EVERYTHING_IPC_QUERY2_REQUEST_DATE_CREATED)
									{
										fd.ftCreationTime.dwLowDateTime = *(DWORD *)p;
										p += sizeof(DWORD);
										fd.ftCreationTime.dwHighDateTime = *(DWORD *)p;
										p += sizeof(DWORD);
									}
									
									// EVERYTHING_IPC_QUERY2_REQUEST_DATE_MODIFIED
									if (_viv_everything_request_flags & EVERYTHING_IPC_QUERY2_REQUEST_DATE_MODIFIED)
									{
										fd.ftLastWriteTime.dwLowDateTime = *(DWORD *)p;
										p += sizeof(DWORD);
										fd.ftLastWriteTime.dwHighDateTime = *(DWORD *)p;
										p += sizeof(DWORD);
									}
									
									_viv_playlist_add(&fd);
								}
							}
						}
					}

					if (cds->dwData == _VIV_COPYDATA_OPEN_EVERYTHING_SEARCH)
					{
						_viv_home(0,0);
					}
					
					break;
				}
			}
			
			break;
		}
		
		case WM_CLOSE:
			_viv_exit();
			return 0;
		
		case WM_SYSCOMMAND:	
			
			switch(wParam)
			{
				case SC_MONITORPOWER:
				case SC_SCREENSAVE:

					// prevent sleep / monitor power off.

					if (config_prevent_sleep)
					{
						if (_viv_is_slideshow)
						{
							return 0;
						}
						
						if (_viv_is_animation_timer)
						{
							if (_viv_animation_play)
							{
								return 0;
							}
						}
					}
					
					break;
			}
			break;
		
		case WM_SIZE:
		
			_viv_on_size();

			break;
			 
		case WM_MOVE:
		{
			if (!IsIconic(hwnd))
			{
				if (!IsMaximized(hwnd))
				{
					if (!_viv_is_fullscreen)
					{
						RECT rect;
						
						GetWindowRect(hwnd,&rect);
			
						config_x = rect.left;
						config_y = rect.top;
					}
				}
			}
			
			break;
		}
			
		case WM_NOTIFY:

			switch(((NMHDR *)lParam)->idFrom)
			{
				case VIV_ID_STATUS:
				
					if (_viv_status_hwnd)
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
									item = (int)SendMessage(_viv_status_hwnd,SB_GETPARTS,0,0) - 1;
								}
								
								switch(item)
								{
									case 1:
										config_frame_minus = !config_frame_minus;
										_viv_status_update();
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
			_viv_command(LOWORD(wParam));
		
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

			// paint the image.
			//
			// we get a list of RECTs that describe the update region.
			// we only want to render inside these RECTs as they are likely to be small (scrolling)
			// avoid stretching the entire image on each paint.
			// 
			// for shrink-stretching we set a HDC user clipping rect.
			// StretchBlt appears to honor this clipping RECT, the whole image is not stretched and is much faster.
			//
			// for magnify-stretching we use our own stretch function that avoids 
			// stretching the entire image.
			// StretchBlt DOES NOT honor the clipping RECT when magnifying.
			
			GetClientRect(hwnd,&rect);
			wide = rect.right - rect.left;
			high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_controls_high();

			if (BeginPaint(hwnd,&ps))
			{
				HRGN update_hrgn;

				update_hrgn = os_CreateRectRgn(0,0,0,0);

				// get visible region
				// this MUST be done between begin and end paint.
				GetRandomRgn(ps.hdc,update_hrgn,SYSRGN);
				
				if (os_is_nt)
				{
					POINT pt;

					GetDCOrgEx(ps.hdc,&pt);
					OffsetRgn(update_hrgn,-pt.x,-pt.y);
				}
				
				// mirror
				if ((os_GetLayout) && (os_GetLayout(ps.hdc) & LAYOUT_RTL))
				{
					HRGN mirror_hrgn;
					RECT rect;
					
					GetClientRect(hwnd,&rect);
					
					mirror_hrgn = os_mirror_region(update_hrgn,rect.right - rect.left);
					
					DeleteObject(update_hrgn);
					
					update_hrgn = mirror_hrgn;
				}

//debug_printf("WM_PAINT\n")			;
				if ((wide) && (high))
				{
					int rx;
					int ry;
					int rw;
					int rh;

					rx = 0;
					ry = 0;
					rw = 0;
					rh = 0;

					// controls.
					/*
					if (_viv_get_controls_high())
					{
						rect.left = 0;
						rect.top = high;
						rect.right = wide;
						rect.bottom = rect.top + _viv_get_controls_high();
						
						FillRect(ps.hdc,&rect,(HBRUSH)(COLOR_WINDOW+1));
						ExcludeClipRect(ps.hdc,rect.left,rect.top,rect.right,rect.bottom);
					}
				*/
					if (_viv_frame_count)
					{
						HDC mem_hdc;
						
						_viv_get_render_size(&rw,&rh);

						rw = (int)(rw * _viv_dst_zoom_values[_viv_dst_zoom_x_pos]);
						rh = (int)(rh * _viv_dst_zoom_values[_viv_dst_zoom_y_pos]);
						
			#if 0
						if (_viv_zoom_pos == 1)
						{
							if ((rw < _viv_image_wide) || (rw < _viv_image_wide))
							{
								rw = _viv_image_wide;
								rh = _viv_image_high;
							}
						}
			#endif
						
						rx = (((_viv_dst_pos_x - 250) * (wide*2)) / 1000) - (rw / 2) - _viv_view_x;
						ry = (((_viv_dst_pos_y - 250) * (high*2)) / 1000) - (rh / 2) - _viv_view_y;
						
			//			rh += (-dst_left_offset + dst_right_offset) * wide / 1000;
			//			rh += (-dst_top_offset + dst_bottom_offset) * high / 1000;

						mem_hdc = CreateCompatibleDC(ps.hdc);
						if (mem_hdc)
						{
							HBITMAP mip_hbitmap;
							int mip_wide;
							int mip_high;
							
debug_printf("PAINT %d %d %d\n",_viv_frame_position,rw,rh);
							mip_hbitmap = _viv_get_mipmap(_viv_frames[_viv_frame_position].hbitmap,_viv_image_wide,_viv_image_high,rw,rh,&mip_wide,&mip_high,&_viv_frames[_viv_frame_position].mipmap);
							
							if (mip_hbitmap)
							{
								HGDIOBJ last_hbitmap;
								
								last_hbitmap = SelectObject(mem_hdc,mip_hbitmap);
								
								if (last_hbitmap)
								{
									if ((rw == mip_wide) && (rh == mip_high))
									{
										if (BitBlt(ps.hdc,rx,ry,rw,rh,mem_hdc,0,0,SRCCOPY))
										{
										}
										else
										{
											debug_printf("BitBlt failed %d\n",GetLastError());
										}
									}
									else
									{
										int last_stretch_mode;
										int did_set_stretch_blt_mode;
										int is_halftone;
										int is_mag;

										did_set_stretch_blt_mode = 0;
										is_halftone = 0;
										is_mag = 0;
										
										if ((rw >= mip_wide) && (rh >= mip_high))
										{
											is_mag = 1;
										}
										
										if ((rw < mip_wide) || (rh < mip_high))
										{
											if (config_shrink_blit_mode == CONFIG_SHRINK_BLIT_MODE_HALFTONE)
											{
												is_halftone = 1;
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
										if ((rw > mip_wide) || (rh > mip_high))
										{
											if (config_mag_filter == CONFIG_MAG_FILTER_HALFTONE)
											{
												is_halftone = 1;
												last_stretch_mode = SetStretchBltMode(ps.hdc,HALFTONE);
											}
											else
											{
												last_stretch_mode = SetStretchBltMode(ps.hdc,COLORONCOLOR);
											}
											
											did_set_stretch_blt_mode = 1;
										}									
								
										{
											DWORD region_size;
									
											region_size = GetRegionData(update_hrgn,0,NULL);
											if (region_size)
											{
												small_pool_t region_data_small_pool;
												RGNDATA *region_data;

												small_pool_init(&region_data_small_pool);
												
												region_data = small_pool_alloc(&region_data_small_pool,region_size);
												
												if (GetRegionData(update_hrgn,region_size,region_data))
												{
													RECT *rect_p;
													DWORD rect_run;

													
													// StretchBlt is REALLY SLOW when there is a complex clipping region.
													// only call StretchBlt for a simple rect clipping region.
													//
													// this only works for halftone
													// don't bother breaking down into simple rects if we are magnifying.

													rect_p = (RECT *)region_data->Buffer;
													rect_run = region_data->rdh.nCount;
													
													while(rect_run)
													{
														if (is_halftone)
														{
															HRGN clip_hrgn;
														
															clip_hrgn = CreateRectRgn(rect_p->left,rect_p->top,rect_p->right,rect_p->bottom);
															if (clip_hrgn)
															{
																if (SelectClipRgn(ps.hdc,clip_hrgn) != ERROR)
																{
																	if (StretchBlt(ps.hdc,rx,ry,rw,rh,mem_hdc,0,0,mip_wide,mip_high,SRCCOPY))
																	{
																	}
																	else
																	{
																		debug_printf("StretchBlt failed %d\n",GetLastError());
																	}
																}
																
																DeleteObject(clip_hrgn);
															}
														}
														else
														{
															if (is_mag)
															{
																int paint_left;
																int paint_right;
																int paint_wide;
																int paint_top;
																int paint_bottom;
																int paint_high;
																
																paint_left = rx;
																paint_right = rx + rw;
																paint_top = ry;
																paint_bottom = ry + rh;
																
																if (paint_left < 0)
																{
																	paint_left = 0;
																}
																
																if (paint_right > wide)
																{
																	paint_right = wide;
																}
																
																if (paint_top < 0)
																{
																	paint_top = 0;
																}
																
																if (paint_bottom > high)
																{
																	paint_bottom = high;
																}
																
																paint_wide = paint_right - paint_left;
																paint_high = paint_bottom - paint_top;
																
																// are we drawing the whole thing?
																if ((rect_p->right - rect_p->left >= paint_wide) && (rect_p->bottom - rect_p->top >= paint_high))
																{
																	// if the clipping region is the full area, just use stretchblt, which is faster.
																	if (_viv_StretchBltStitch(ps.hdc,rx,ry,rw,rh,mem_hdc,0,0,mip_wide,mip_high,SRCCOPY,rect_p->left,rect_p->top,rect_p->right - rect_p->left,rect_p->bottom - rect_p->top))
																	{
																	}
																	else
																	{
																		debug_printf("StretchBlt failed %d\n",GetLastError());
																	}
																}
																else
																{
																	// use our own stretch that only renders the clipping rect region.
																	// where as StretchBlt ignores the clipping region and renders the entire dst region.
																	_viv_stretch_blt(ps.hdc,rx,ry,rw,rh,mem_hdc,mip_wide,mip_high,rect_p->left,rect_p->top,rect_p->right - rect_p->left,rect_p->bottom - rect_p->top);
																}
															}
															else
															{
																if (StretchBlt(ps.hdc,rx,ry,rw,rh,mem_hdc,0,0,mip_wide,mip_high,SRCCOPY))
																{
																}
																else
																{
																	debug_printf("StretchBlt failed %d\n",GetLastError());
																}
															}
														}
															
														rect_p++;
														rect_run--;
													}

													if (is_halftone)
													{
														SelectClipRgn(ps.hdc,NULL);
													}
												}

												small_pool_kill(&region_data_small_pool);
											}
										}

										if (did_set_stretch_blt_mode)
										{
											SetStretchBltMode(ps.hdc,last_stretch_mode);
										}
									}

									SelectObject(mem_hdc,last_hbitmap);								
								}
								else
								{
									debug_printf("SelectObject failed %d\n",GetLastError());
								}
									
								//ExcludeClipRect(ps.hdc,rx,ry,rx+rw,ry+rh);
							}
							
							DeleteDC(mem_hdc);
						}
						else
						{
							debug_printf("CreateCompatibleDC failed %d\n",GetLastError());
						}
					}

					{
						HBRUSH hbrush;
						
						hbrush = CreateSolidBrush(_viv_is_fullscreen ? RGB(config_fullscreen_background_color_r,config_fullscreen_background_color_g,config_fullscreen_background_color_b) : RGB(config_windowed_background_color_r,config_windowed_background_color_g,config_windowed_background_color_b));
						
						if (hbrush)
						{
							os_fill_clipped_rect(ps.hdc,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,rx,ry,rw,rh,hbrush);
						
							DeleteObject(hbrush);
						}
					}
				}
				
				_viv_is_animation_paint = 0;
				
				DeleteObject(update_hrgn);
				
				EndPaint(hwnd,&ps);
			}
			else
			{
				debug_printf("BeginPaint failed %d\n",GetLastError());
			}
			
			return 0;
		}
			
		case WM_ERASEBKGND:
			return 1;
			
		case WM_GETMINMAXINFO:

			{
				int wide;
				int high;
				RECT rect;
				BOOL is_menu;
				
				wide = _viv_toolbar_get_wide();
				high = _viv_get_status_high() + _viv_get_controls_high();
				
				is_menu = GetMenu(_viv_hwnd) ? TRUE : FALSE;
			
				rect.left = 0;
				rect.top = 0;
				rect.right = wide;
				rect.bottom = high;
				
				AdjustWindowRectEx(&rect,os_get_window_style(hwnd),is_menu,os_get_window_ex_style(hwnd));

				((MINMAXINFO *)lParam)->ptMinTrackSize.x = rect.right - rect.left; 
				((MINMAXINFO *)lParam)->ptMinTrackSize.y = rect.bottom - rect.top;
			}

			break;
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static int _viv_process_install_command_line_options(wchar_t *cl)
{
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
	int is_admin_install;
	int is_standard_user_install;
	
	startmenu = 0;
	install_flags = 0;
	uninstall_flags = 0;
	appdata = 0;
	is_runas = 0;
	is_admin_install = 0;
	is_standard_user_install = 0;
	install_path[0] = 0;
	install_options[0] = 0;
	uninstall_path[0] = 0;

	p = string_skip_ws(cl);
	
	// skip exe filename
	p = string_get_word(p,buf);
	p = string_skip_ws(p);

	cl_start = p;

	// skip first parameter.
	for(;;)
	{
		wchar_t *bufstart;
		BOOL was_quote;
		
		// no more text?
		if (!*p)
		{
			break;
		}
		
		was_quote = FALSE;
		if (*p == '"')
		{
			was_quote = TRUE;
		}
		
		p = string_get_word(p,buf);
		p = string_skip_ws(p);
		
		bufstart = buf;
		
		// treat switches with a '.' as a filename.
		// eg: voidimageviewer.exe -my-image.png
		if ((!was_quote) && ((*bufstart == '/') || (*bufstart == '-')) && (!string_is_dot(buf)))
		{
			bufstart++;
			
			if (string_icompare_lowercase_ascii(bufstart,"install") == 0)
			{
				p = string_get_word(p,install_path);
				p = string_skip_ws(p);				

				uninstall_path[0] = 0;

				is_admin_install = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"install-options") == 0)
			{
				p = string_get_word(p,install_options);
				p = string_skip_ws(p);				

				is_admin_install = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"uninstall") == 0)
			{
				p = string_get_word(p,uninstall_path);
				p = string_skip_ws(p);			
				
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
				
				is_admin_install = 1;
				is_standard_user_install = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"appdata") == 0)
			{
				appdata = 1;
				is_admin_install = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"noappdata") == 0)
			{
				appdata = -1;
				is_admin_install = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"startmenu") == 0)
			{
				startmenu = 1;
				is_admin_install = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"nostartmenu") == 0)
			{
				startmenu = -1;
				is_admin_install = 1;
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
				
				for(exti=0;exti<_VIV_ASSOCIATION_COUNT;exti++)
				{
					if (string_icompare_lowercase_ascii(bufstart,_viv_association_extensions[exti]) == 0)
					{
						if (is_no)
						{
							uninstall_flags |= 1 << exti;
						}
						else
						{
							install_flags |= 1 << exti;
						}
						
						is_standard_user_install = 1;
						break;
					}
				}
			}
		}
	}
	
	// debug_printf("isrunas %d install_flags %u uninstall_flags %u\n",is_runas,install_flags,uninstall_flags);

	// do associationas as standard user
	if (!is_runas)
	{
		if (install_flags)
		{
			_viv_install_association(install_flags);
		}
	
		if (uninstall_flags)
		{
			_viv_uninstall_association(uninstall_flags);
		}
	}
		
	if (is_admin_install)
	{
		if (!os_is_admin())
		{
			if (!is_runas)
			{
				wchar_t exe_filename[STRING_SIZE];
				wchar_t params[STRING_SIZE];
				wchar_t cwd[STRING_SIZE];
				
				_viv_get_exe_filename(exe_filename);
				GetCurrentDirectory(STRING_SIZE,cwd);
				
				string_copy_utf8(params,(const utf8_t *)"/isrunas ");
				string_cat(params,cl_start);
				
				os_shell_execute(0,exe_filename,1,"runas",params);
				
				return 1;
			}
		}
	}
	
	if (appdata > 0)
	{	
		config_appdata = 1;
		
		config_save_settings(config_appdata);

		// appdata enabled.
		config_save_settings(0);
	}
	else
	if (appdata < 0)
	{
		config_appdata = 0;
		
		// appdata disabled
		config_save_settings(0);
	}
	
	if (startmenu > 0)
	{
		_viv_install_start_menu_shortcuts();
	}
	else
	if (startmenu < 0)
	{
		_viv_uninstall_start_menu_shortcuts();
	}
	
	if (install_path[0])
	{
		wchar_t temp_path[STRING_SIZE];
		
		// make sure no other process is running.
		_viv_close_existing_process();
		
		string_get_exe_path(temp_path);
		
		os_make_sure_path_exists(install_path);
		
		_viv_install_copy_file(install_path,temp_path,(const utf8_t *)"voidImageViewer.exe",1);
		_viv_install_copy_file(install_path,temp_path,(const utf8_t *)"Uninstall.exe",0);
		_viv_install_copy_file(install_path,temp_path,(const utf8_t *)"Changes.txt",0);
		
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
		_viv_close_existing_process();
		
		// remove %APPDATA%\voidimageviewer
		if (string_get_appdata_voidimageviewer_path(path))
		{
			_viv_uninstall_delete_file(path,(const utf8_t *)"voidImageViewer.ini");

			RemoveDirectory(path);
		}
					
		_viv_uninstall_delete_file(uninstall_path,(const utf8_t *)"Uninstall.exe");
		_viv_uninstall_delete_file(uninstall_path,(const utf8_t *)"Changes.txt");
		_viv_uninstall_delete_file(uninstall_path,(const utf8_t *)"voidImageViewer.ini");
		_viv_uninstall_delete_file(uninstall_path,(const utf8_t *)"voidImageViewer.exe");
		
		RemoveDirectory(uninstall_path);
	}
	
	if ((is_admin_install) || (is_standard_user_install))
	{
		return 1;
	}
	
	return 0;
}

static void _viv_process_command_line(wchar_t *cl)
{
	wchar_t *p;
	wchar_t buf[STRING_SIZE];
	wchar_t single[STRING_SIZE];
	int file_count;
	int start_slideshow;
	int start_fullscreen;
	int start_window;
	int window_x;
	int window_y;
	int window_wide;
	int window_high;
	int set_window_rect;
	RECT rect;
	int start_maximized;
	
	debug_printf("cl: %S\n",cl);
	
	start_slideshow = 0;
	start_fullscreen = 0;
	start_window = 0;
	start_maximized = 0;
	p = cl;
	file_count = 0;
	single[0] = 0;
	set_window_rect = 0;
	GetWindowRect(_viv_hwnd,&rect);

	window_x = rect.left;
	window_y = rect.top;
	window_wide = rect.right - rect.left;
	window_high = rect.bottom - rect.top;
	
	p = string_skip_ws(p);
	
	// skip exe filename
	p = string_get_word(p,buf);
	p = string_skip_ws(p);

	// skip first parameter.
	for(;;)
	{
		wchar_t *bufstart;
		BOOL was_quote;

		// no more text?
		if (!*p)
		{
			break;
		}
		
		was_quote = FALSE;
		if (*p == '"')
		{
			was_quote = TRUE;
		}

		p = string_get_word(p,buf);
		p = string_skip_ws(p);
		
		bufstart = buf;
		
		// treat switches with a '.' as a filename.
		if ((!was_quote) && ((*bufstart == '/') || (*bufstart == '-')) && (!string_is_dot(buf)))
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
			if (string_icompare_lowercase_ascii(bufstart,"maximized") == 0)
			{
				start_fullscreen = 0;	
				start_window = 1;
				start_maximized = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"ontop") == 0)
			{
				_viv_command(VIV_ID_VIEW_ONTOP_ALWAYS);
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"shuffle") == 0)
			{
				config_shuffle = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"everything") == 0)
			{
				p = string_get_word(p,buf);
				p = string_skip_ws(p);
				
				_viv_send_everything_search(0,0,0,buf);
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"random") == 0)
			{
				p = string_get_word(p,buf);
				p = string_skip_ws(p);
				
				_viv_send_everything_search(0,0,1,buf);
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"minimal") == 0)
			{
				_viv_command(VIV_ID_VIEW_PRESET_1);
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"compact") == 0)
			{
				_viv_command(VIV_ID_VIEW_PRESET_2);
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"x") == 0)
			{
				p = string_get_word(p,buf);
				p = string_skip_ws(p);
				
				window_x = string_to_int(buf);
				set_window_rect = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"y") == 0)
			{
				p = string_get_word(p,buf);
				p = string_skip_ws(p);
				
				window_y = string_to_int(buf);
				set_window_rect = 1;
			}		
			else
			if (string_icompare_lowercase_ascii(bufstart,"width") == 0)
			{
				p = string_get_word(p,buf);
				p = string_skip_ws(p);
				
				window_wide = string_to_int(buf);
				set_window_rect = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"height") == 0)
			{
				p = string_get_word(p,buf);
				p = string_skip_ws(p);
				
				window_high = string_to_int(buf);
				set_window_rect = 1;
			}		
			else
			if (string_icompare_lowercase_ascii(bufstart,"rate") == 0)
			{
				p = string_get_word(p,buf);
				p = string_skip_ws(p);

				config_slideshow_rate = string_to_int(buf);
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"name") == 0)
			{
				config_nav_sort = CONFIG_NAV_SORT_NAME;
				config_nav_sort_ascending = 1;
			}
			else			
			if (string_icompare_lowercase_ascii(bufstart,"dm") == 0)
			{
				config_nav_sort = CONFIG_NAV_SORT_DATE_MODIFIED;
				config_nav_sort_ascending = 0;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"dc") == 0)
			{
				config_nav_sort = CONFIG_NAV_SORT_DATE_CREATED;
				config_nav_sort_ascending = 0;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"path") == 0)
			{
				config_nav_sort = CONFIG_NAV_SORT_FULL_PATH_AND_FILENAME;
				config_nav_sort_ascending = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"size") == 0)
			{
				config_nav_sort = CONFIG_NAV_SORT_SIZE;
				config_nav_sort_ascending = 0;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"ascending") == 0)
			{
				config_nav_sort_ascending = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"descending") == 0)
			{
				config_nav_sort_ascending = 0;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"isrunas") == 0)
			{
				// ignore this for non-install commands.
			}
			else
			{
				_viv_command_line_options();
			}
		}
		else
		{
			if (*buf)
			{
				if (file_count == 0)
				{
					// new playlist
					if (_viv_random)
					{
						mem_free(_viv_random);
						
						_viv_random = 0;
					}
					
					_viv_playlist_clearall();
				}
				
				if (file_count == 1)
				{
					// add the last single
					_viv_playlist_add_filename(single);
				}

				if (file_count >= 1)
				{
					// add this file
					_viv_playlist_add_filename(buf);
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
		const wchar_t *open_filename;
	
		if (_viv_random)
		{
			mem_free(_viv_random);
			
			_viv_random = 0;
		}
		
		open_filename = NULL;
		
		if (file_count > 1)
		{
			// treat as a playlist.
			if (_viv_playlist_start)
			{
				open_filename = _viv_playlist_start->fd.cFileName;
			}
		}
		else
		if (file_count == 1)
		{
			open_filename = single;
		}
		
		// open the first image found (if multiple images passed).
		// if we only specified a single image, use the single image filename.
		if ((open_filename) && (_viv_open_from_filename(open_filename)))
		{
			// all good.
		}
		else
		{
			_viv_file_not_found = 1;
			_viv_status_update();
		}
	}
	
	if (start_slideshow)
	{
		_viv_slideshow();
	}
	
	if (start_fullscreen)
	{
		if (!_viv_is_fullscreen)
		{
			_viv_toggle_fullscreen();
			
			if (start_maximized)
			{
				_viv_fullscreen_is_maxed = 1;
			}
		}
	}
	else
	{
		if (start_window)
		{
			if (_viv_is_fullscreen)
			{
				_viv_toggle_fullscreen();
			}
		}
		else
		{
			if (set_window_rect)
			{
				SetWindowPos(_viv_hwnd,0,window_x,window_y,window_wide,window_high,SWP_NOZORDER|SWP_NOACTIVATE);
			}
		}

		if (start_maximized)
		{
			if (!_viv_is_window_maximized(_viv_hwnd))
			{
				ShowWindow(_viv_hwnd,SW_MAXIMIZE);
			}
		}
	}
}

static int _viv_init(int nCmdShow)
{
	RECT rect;
	STARTUPINFO si;
	int show_maximized;
	DWORD window_style;
	
	os_init();
	localization_init(); // Initialize language system
	show_maximized = 0;
	
	debug_printf("%u\n",sizeof(_viv_key_list_t));
	
	_viv_key_list = mem_alloc(sizeof(_viv_key_list_t));
	
	_viv_key_list_init(_viv_key_list);
	
	// setup keys.
	{
		int i;
		
		for(i=0;i<_VIV_DEFAULT_KEY_COUNT;i++)
		{
			viv_key_add(_viv_command_index_from_command_id(_viv_default_keys[i].command_id),_viv_default_keys[i].key_flags);
		}
	}
	
	// init _viv_dst_zoom_values
	
	_viv_dst_zoom_values = mem_alloc(sizeof(float) * _VIV_DST_ZOOM_MAX);
	
	{
		int i;
		float f;
		
		f = 1.0;
		
		i = _VIV_DST_ZOOM_ONE;
		while(i >= 0)
		{
			_viv_dst_zoom_values[i] = f;
			f /= 1.02;
			
			i--;
		}
		
		f = 1.0;
		
		i = _VIV_DST_ZOOM_ONE;
		while(i < _VIV_DST_ZOOM_MAX)
		{
			_viv_dst_zoom_values[i] = f;
			f *= 1.02;
			
			i++;
		}
	}
	
	debug_printf("viv %d.%d.%d.%d%s %s\n",VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION,VERSION_BUILD,VERSION_TYPE,VERSION_TARGET_MACHINE);
	
	os_hinstance = GetModuleHandle(0);
	
	_viv_current_fd = (WIN32_FIND_DATA *)mem_alloc(sizeof(WIN32_FIND_DATA));
	os_zero_memory(_viv_current_fd,sizeof(WIN32_FIND_DATA));

	_viv_preload_fd = (WIN32_FIND_DATA *)mem_alloc(sizeof(WIN32_FIND_DATA));
	os_zero_memory(_viv_preload_fd,sizeof(WIN32_FIND_DATA));
	
	_viv_last_fd = (WIN32_FIND_DATA *)mem_alloc(sizeof(WIN32_FIND_DATA));
	os_zero_memory(_viv_last_fd,sizeof(WIN32_FIND_DATA));

	_viv_frame_fd = (WIN32_FIND_DATA *)mem_alloc(sizeof(WIN32_FIND_DATA));
	os_zero_memory(_viv_frame_fd,sizeof(WIN32_FIND_DATA));
	
	_viv_load_fd = (WIN32_FIND_DATA *)mem_alloc(sizeof(WIN32_FIND_DATA));
	os_zero_memory(_viv_load_fd,sizeof(WIN32_FIND_DATA));
	
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

	InitializeCriticalSection(&_viv_cs);
	
	// Initialize GDI+.
	if (os_GdiplusStartup)
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
	config_load_settings();
	
	// config_maximized will be overwritten when we show are normal window
	// so save it now and apply it later.
	show_maximized = config_maximized;
	
	// process install command line options
	{
		if (_viv_process_install_command_line_options(GetCommandLineW()))
		{
			_viv_kill();
			
			return 0;
		}
	}

	// mutex
	if (!config_multiple_instances)
	{
		SetLastError(0);

		_viv_mutex = CreateMutexA(NULL,0,"VOIDIMAGEVIEWER");
		
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
				
				if (!(si.dwFlags & STARTF_USESHOWWINDOW))
				{
					si.wShowWindow = nCmdShow;
				}
				
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
				cds.dwData = _VIV_COPYDATA_COMMAND_LINE;

				SendMessage(hwnd,WM_COPYDATA,(WPARAM)0,(LPARAM)&cds);
				
				mem_free(buf);
			}
			
			_viv_kill();
			
			return 0;
		}
	}
	
	os_RegisterClassEx(
		CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW,
		_viv_proc,
		(HICON)LoadImage(os_hinstance,MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON,GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CXICON),0),
		LoadCursor(NULL,IDC_ARROW),
		(HBRUSH)(COLOR_BTNFACE+1),
		"VOIDIMAGEVIEWER",
		(HICON)LoadImage(os_hinstance,MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0));
	
	_viv_hmenu = _viv_create_menu();
	
	rect.left = config_x;
	rect.top = config_y;
	rect.right = config_x + config_wide;
	rect.bottom = config_y + config_high;
	
	// position the window nicely on first use
	// auto-fit and center on the monitor from the mouse cursor.
	if ((!config_wide) || (!config_high))
	{
		RECT monitor_rect;
		int default_wide;
		int default_high;
		
		default_wide = 640;
		default_high = 480;
		
		// use full screen to calculate auto-fit size.
		os_MonitorRectFromCursor(1,&monitor_rect);
		
		if (config_auto_fit_wide_div)
		{
			default_wide = ((monitor_rect.right - monitor_rect.left) * config_auto_fit_wide_mul) / config_auto_fit_wide_div;
		}

		if (config_auto_fit_high_div)
		{
			default_high = ((monitor_rect.bottom - monitor_rect.top) * config_auto_fit_high_mul) / config_auto_fit_high_div;
		}
		
		rect.left = ((monitor_rect.right - monitor_rect.left) / 2) - (default_wide / 2);
		rect.top = ((monitor_rect.bottom - monitor_rect.top) / 2) - (default_high / 2);
		rect.right = rect.left + default_wide;
		rect.bottom = rect.top + default_high;
	}
	
	window_style = WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	_viv_hwnd = os_CreateWindowEx(
		0,
		"VOIDIMAGEVIEWER",
		localization_get_string(LOCALIZATION_ID_APP_NAME),
		window_style,
		rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,
		0,config_show_menu ? _viv_hmenu : NULL,os_hinstance,NULL);
	
	if ((!config_show_caption) || (!config_show_thickframe))
	{
		_viv_update_frame();
	}
		
	os_make_rect_completely_visible(_viv_hwnd,&rect);
		
	SetWindowPos(_viv_hwnd,0,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOZORDER|SWP_NOACTIVATE);

	// allow non-admin/admins to close this window.
	if (os_ChangeWindowMessageFilterEx)
	{
		// MSGFLT_ALLOW = 1
		os_ChangeWindowMessageFilterEx(_viv_hwnd,WM_CLOSE,1,0);
	}
		
	_viv_status_show(config_show_status);
	_viv_controls_show(config_show_controls);
	
	DragAcceptFiles(_viv_hwnd,TRUE);

	_viv_update_title();

	_viv_update_ontop();
	
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);

	if (!(si.dwFlags & STARTF_USESHOWWINDOW))
	{
		si.wShowWindow = nCmdShow;
	}
	
	// default is to shownormal.
	// if its anything else, like minimize/maximize to that before we apply the command line.
	if (si.wShowWindow != SW_SHOWNORMAL)
	{
		ShowWindow(_viv_hwnd,si.wShowWindow);
		UpdateWindow(_viv_hwnd);
	}
	
	if (show_maximized)
	{
		ShowWindow(_viv_hwnd,SW_MAXIMIZE);
	}
	
	_viv_process_command_line(GetCommandLineW());

	// if we didn't show the window above, make sure it 
	// is shown now.
	if (si.wShowWindow == SW_SHOWNORMAL)
	{
		ShowWindow(_viv_hwnd,SW_SHOW);
		UpdateWindow(_viv_hwnd);
	}

	return 1;
}

static void _viv_kill(void)
{
	_viv_show_cursor();

	// don't load another image..
	if (_viv_load_image_next_fd)
	{
		mem_free(_viv_load_image_next_fd);
		
		_viv_load_image_next_fd = 0;
	}
	
	// stop load_image immediately...
	if (_viv_load_image_thread)
	{
		_viv_load_image_terminate = 1;
		
		// it's critical we wait for load image to finish before we kill the main window.
		WaitForSingleObject(_viv_load_image_thread,INFINITE);
		
		CloseHandle(_viv_load_image_thread);
	}
	
	if (_viv_load_image_filename)
	{
		mem_free(_viv_load_image_filename);
	}
	
	if (_viv_last_frames)
	{
		_viv_clear_frames(_viv_last_frames,_viv_last_frame_count);
		
		_viv_last_frames = NULL;
	}
	
	_viv_clear_preload_frames();
	_viv_clear();
	_viv_process_pending_clear();

	_viv_status_show(0);
	_viv_controls_show(0);
//	_viv_tooltip_hide();

	if (_viv_hwnd)
	{
		DestroyWindow(_viv_hwnd);
	}
	
	if (_viv_status_temp_text)
	{
		mem_free(_viv_status_temp_text);
	}
	
	_viv_nav_item_free_all();
	
	if (_viv_random)
	{
		mem_free(_viv_random);
	}
	
	if (_viv_hmenu)
	{
		DestroyMenu(_viv_hmenu);
	}

	if (_viv_about_hfont)
	{
		DeleteObject(_viv_about_hfont);
	}

	if (_viv_mutex)
	{
		CloseHandle(_viv_mutex);
	}

	_viv_playlist_clearall();
	_viv_reply_clear_all();

	if (os_GdiplusShutdown)
	{
		os_GdiplusShutdown(os_GdiplusToken);
	}

	if (_viv_stobject_hmodule)
	{
		FreeLibrary(_viv_stobject_hmodule);
	}
	
	CoUninitialize();
	
	DeleteCriticalSection(&_viv_cs);

	if (_viv_last_open_file)
	{
		mem_free(_viv_last_open_file);
	}

	if (_viv_last_open_folder)
	{
		mem_free(_viv_last_open_folder);
	}

	mem_free(_viv_load_fd);
	mem_free(_viv_frame_fd);
	mem_free(_viv_last_fd);
	mem_free(_viv_preload_fd);
	mem_free(_viv_current_fd);
	
	mem_free(_viv_dst_zoom_values);
	
	_viv_key_clear_all(_viv_key_list);
	mem_free(_viv_key_list);
	
	os_kill();

#ifdef _DEBUG
	mem_debug();
#endif	
}

static int _viv_main(int nCmdShow)
{
	if (_viv_init(nCmdShow))
	{
		for(;;)
		{
			// Main message loop:
			for(;;)
			{
				MSG msg;
				
				if (!PeekMessage(&msg,0,0,0,PM_REMOVE)) break;
				
//				debug_printf("MSG %u %u %u\n",msg.message,msg.wParam,msg.lParam);
				
				if (msg.message == WM_QUIT) goto exit;
				
				if (!_viv_is_msg(&msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			
			WaitMessage();
		}
		
	exit:
		
		_viv_kill();
	}

	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
	return _viv_main(nShowCmd);
}

int __cdecl main(int argc,char **argv)
{
	return _viv_main(SW_SHOW);
}

static int _viv_compare_id(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b)
{
	if (a->dwReserved0 < b->dwReserved0)
	{
		return -1;
	}
	
	if (a->dwReserved0 > b->dwReserved0)
	{
		return 1;
	}
	
	if (a->dwReserved1 < b->dwReserved1)
	{
		return -1;
	}
	
	if (a->dwReserved1 > b->dwReserved1)
	{
		return 1;
	}
	
	return 0;	
}

static int _viv_fd_compare_name(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b)
{
	int ret;
	const wchar_t *afilename;
	const wchar_t *bfilename;
	DWORD dwCmpFlags;
	
	dwCmpFlags = SORT_STRINGSORT|NORM_IGNORECASE;
	
	if (os_is_windows_7_or_later())
	{
		// #define SORT_DIGITSASNUMBERS      0x00000008  // use digits as numbers sort method
		dwCmpFlags |= 0x00000008;
	}

	afilename = string_get_filename_part(a->cFileName);
	bfilename = string_get_filename_part(b->cFileName);

	ret = CompareString(LOCALE_USER_DEFAULT,dwCmpFlags,afilename,string_length(afilename),bfilename,string_length(bfilename));
	
	switch(ret)
	{
		case CSTR_LESS_THAN:	
			return -1;
			
		case CSTR_GREATER_THAN:
			return 1;
	}

	return _viv_compare_id(a,b);
}

static int _viv_fd_compare_path_and_name(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b)
{
	int ret;
	DWORD dwCmpFlags;
	
	dwCmpFlags = SORT_STRINGSORT|NORM_IGNORECASE;
	
	if (os_is_windows_7_or_later())
	{
		// #define SORT_DIGITSASNUMBERS      0x00000008  // use digits as numbers sort method
		dwCmpFlags |= 0x00000008;
	}

	ret = CompareString(LOCALE_USER_DEFAULT,dwCmpFlags,a->cFileName,string_length(a->cFileName),b->cFileName,string_length(b->cFileName));
	
	switch(ret)
	{
		case CSTR_LESS_THAN:	
			return -1;
			
		case CSTR_GREATER_THAN:
			return 1;
	}

	return _viv_compare_id(a,b);
}

static int _viv_fd_compare(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b)
{
	int ret;
	
	ret = 0;
	
	switch(config_nav_sort)
	{
		case CONFIG_NAV_SORT_NAME:
			ret = _viv_fd_compare_name(a,b);
			break;

		case CONFIG_NAV_SORT_FULL_PATH_AND_FILENAME:
			ret = _viv_fd_compare_path_and_name(a,b);
			break;

		case CONFIG_NAV_SORT_SIZE:
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
				ret = -_viv_fd_compare_name(a,b);
			}
			
			break;
		}

		case CONFIG_NAV_SORT_DATE_MODIFIED:
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
				ret = -_viv_fd_compare_name(a,b);
			}
			
			break;
		}

		case CONFIG_NAV_SORT_DATE_CREATED:
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
				ret = -_viv_fd_compare_name(a,b);
			}
			
			break;
		}
	}
	
	if (!config_nav_sort_ascending)
	{
		ret *= -1;
	}
	
	return ret;
}

static int _viv_next(int prev,int reset_slideshow_timer,int is_preload,int wait_for_current_load)
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
							
							// compare with start..
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
										if ((!got_best) || (_viv_fd_compare(&fd,&best_fd) < 0))
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
								if ((!got_start) || (_viv_fd_compare(&fd,&start_fd) > 0))
								{
									string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
									string_copy(fd.cFileName,search_wbuf);
									os_copy_memory(&start_fd,&fd,sizeof(WIN32_FIND_DATA));
									
									got_start = 1;
								}
							}
							else
							{
								if ((!got_start) || (_viv_fd_compare(&fd,&start_fd) < 0))
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

static void _viv_home(int end,int is_preload)
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
								string_copy(fd.cFileName,search_wbuf);
								os_copy_memory(&best_fd,&fd,sizeof(WIN32_FIND_DATA));
								got_best = 1;
							}
						}
						else
						{
							if ((!got_best) || (_viv_fd_compare(&fd,&best_fd) < 0))
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

static int _viv_is_valid_filename(WIN32_FIND_DATA *fd)
{
	if (!(fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		const wchar_t *s;
		const wchar_t *e;
		
		e = 0;
		s = fd->cFileName;
		
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
			
			for(exti=0;exti<_VIV_ASSOCIATION_COUNT;exti++)
			{
				if (string_icompare_lowercase_ascii(e,_viv_association_extensions[exti]) == 0) 
				{
					return 1;
				}
			}
		}
	}
	
	return 0;		
}

static int _viv_is_msg(MSG *msg)
{
  	switch (msg->message)
	{
		case WM_XBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
		case WM_NCXBUTTONDOWN:
		case WM_NCXBUTTONDBLCLK:

			switch(config_xbutton_action)
			{
				case 1:
					
					switch(HIWORD(msg->wParam))
					{
						case XBUTTON1:
							_viv_zoom_in(1,1,GET_X_LPARAM(msg->lParam),GET_Y_LPARAM(msg->lParam));
							break;

						case XBUTTON2:
							_viv_zoom_in(0,1,GET_X_LPARAM(msg->lParam),GET_Y_LPARAM(msg->lParam));
							break;
					}
					
					break;					

				case 2:
					
					switch(HIWORD(msg->wParam))
					{
						case XBUTTON1:
							_viv_next(1,1,0,0);
							break;

						case XBUTTON2:
							_viv_next(0,1,0,0);
							break;
					}
					
					break;
					
			}
			
			break;
				
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			
			{
				int key_flags;
			
				key_flags = _viv_get_current_key_mod_flags();
				
				if (msg->hwnd == _viv_hwnd)
				{
					int key_index;

/*
					if (msg->wParam == VK_MENU)
					{
						_viv_is_alt = 1;
						_viv_update_show_cursor();
						_viv_update_src_pixel(0,1);
					}
*/					
					// cancel action
					if ((key_flags == 0) && (msg->wParam == VK_ESCAPE))
					{
						if (_viv_doing)
						{
							_viv_doing_cancel();
						
							return 1;
						}

						if (_viv_is_fullscreen)
						{
							_viv_toggle_fullscreen();
							
							// also pause slideshow
							if (_viv_is_slideshow)
							{
								_viv_pause();
							}
						
							return 1;
						}
					}
					
					// find the key.
					for(key_index=0;key_index<_VIV_COMMAND_COUNT;key_index++)
					{
						config_key_t *k;
						
						k = _viv_key_list->start[key_index];
						while(k)
						{
							if ((k->key & CONFIG_KEYFLAG_MOD_MASK) == key_flags)
							{
								if ((k->key & CONFIG_KEYFLAG_VK_MASK) == msg->wParam)
								{
									_viv_command_with_is_key_repeat(_viv_commands[key_index].command_id,(msg->lParam & 0x40000000) ? 1 : 0);
									
									return 1;
								}
							}
							
							k = k->next;
						}
					}
				}
			}
			
			break;

/*			
		case WM_KEYUP:
		case WM_SYSKEYUP:
		
			if (msg->wParam == VK_MENU)
			{
				_viv_is_alt = 0;
				_viv_update_show_cursor();
				_viv_update_src_pixel(0,1);
			}
			break;
			*/
	}

	return 0;
}

static void _viv_view_set(int view_x,int view_y,int invalidate)
{
	RECT rect;
	int wide;
	int high;
	int rw;
	int rh;
	
	GetClientRect(_viv_hwnd,&rect);
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_controls_high();

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

static void _viv_toggle_fullscreen(void)
{
	DWORD style;
	int old_rw;
	int old_rh;
	int zoom_wide_array[_VIV_ZOOM_MAX];
	int zoom_high_array[_VIV_ZOOM_MAX];
	
	_viv_get_render_size(&old_rw,&old_rh);

	// precalculate all zoom levels for comparison later to find the zoom offset.
	{
		int backup_zoom;

		backup_zoom = _viv_zoom_pos;
		
		for(_viv_zoom_pos=0;_viv_zoom_pos<_VIV_ZOOM_MAX;_viv_zoom_pos++)
		{
			int rw;
			int rh;
			
			_viv_get_render_size(&rw,&rh);
			
			zoom_wide_array[_viv_zoom_pos] = rw;
			zoom_high_array[_viv_zoom_pos] = rh;
		}
		
		_viv_zoom_pos = backup_zoom;
	}
				
	_viv_prevent_on_size = 1;

debug_printf("toggle fullscreen %d\n",!_viv_is_fullscreen);

	style = GetWindowLong(_viv_hwnd,GWL_STYLE);
	
	_viv_1to1 = 0;
	
	if (_viv_is_fullscreen)
	{
		// set before restore
		// otherwise get_render_size will return the wrong size.
		_viv_is_fullscreen = 0;

		if (config_show_caption)	
		{
			style |= WS_CAPTION;
		}
		else
		{
			style &= ~(WS_CAPTION);
		}
		
		if (config_show_thickframe)	
		{
			style |= WS_THICKFRAME;
		}
		else
		{
			style &= ~WS_THICKFRAME;
		}

		if (config_show_menu)	
		{
			SetMenu(_viv_hwnd,_viv_hmenu);
		}
		else
		{
			SetMenu(_viv_hwnd,0);
		}
		
		_viv_status_show(config_show_status);
		_viv_controls_show(config_show_controls);
		
		SetWindowLong(_viv_hwnd,GWL_STYLE,style);

		SetWindowPos(_viv_hwnd,HWND_TOP,_viv_fullscreen_rect.left,_viv_fullscreen_rect.top,_viv_fullscreen_rect.right - _viv_fullscreen_rect.left,_viv_fullscreen_rect.bottom - _viv_fullscreen_rect.top,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOCOPYBITS);
	
		if (_viv_fullscreen_is_maxed)
		{	
			ShowWindow(_viv_hwnd,SW_MAXIMIZE);
		}
		
		_viv_zoom_pos += _viv_fullscreen_zoom_offset;
		_viv_zoom_pos = _viv_clamp_zoom_pos(_viv_zoom_pos);
	}
	else
	{
		RECT monitor_rect;
			
		// set fullscreen before we resize.
		_viv_is_fullscreen = 1;
			
		os_MonitorRectFromWindow(_viv_hwnd,1,&monitor_rect);
		
		SetWindowLong(_viv_hwnd,GWL_STYLE,style & ~(WS_CAPTION|WS_THICKFRAME));
		
		_viv_fullscreen_is_maxed = IsZoomed(_viv_hwnd);
		if (_viv_fullscreen_is_maxed)
		{
			ShowWindow(_viv_hwnd,SW_SHOWNORMAL);
		}
		
		GetWindowRect(_viv_hwnd,&_viv_fullscreen_rect);
		
		SetMenu(_viv_hwnd,0);
		_viv_status_show(0);
		_viv_controls_show(0);

		SetWindowPos(_viv_hwnd,HWND_TOP,monitor_rect.left,monitor_rect.top,monitor_rect.right - monitor_rect.left,monitor_rect.bottom - monitor_rect.top,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOCOPYBITS);

		_viv_prevent_on_deactivate = 1;

		// create a dummy fullscreen window and destroy it immediately
		// this window is created fullscreen correctly, and when our normal window gains focus it will keep the fullscreen setting (no taskbar)
		// without this dummy window, sometimes the taskbar will not disappear.
		
		{
			HWND fullscreen_hwnd;
			
			// Initialize global strings
			os_RegisterClassEx(
				0,
				_viv_fullscreen_proc,
				0,
				LoadCursor(NULL,IDC_ARROW),
				(HBRUSH)(COLOR_BTNFACE+1),
				"_VIV_FULLSCREEN",
				0);

			fullscreen_hwnd = os_CreateWindowEx(
				0,
				"_VIV_FULLSCREEN",
				localization_get_string(LOCALIZATION_ID_APP_NAME),
				WS_POPUP|WS_VISIBLE,
				monitor_rect.left,monitor_rect.top,monitor_rect.right - monitor_rect.left,monitor_rect.bottom - monitor_rect.top,
				0,0,os_hinstance,NULL);

			SetForegroundWindow(fullscreen_hwnd);

			DestroyWindow(fullscreen_hwnd);
		}

		// find zoom offset.
		{
			int zoom_index;
			
			_viv_fullscreen_zoom_offset = 0;
			
			if (config_fullscreen_fill_window)
			{
				for(zoom_index=0;zoom_index<_VIV_ZOOM_MAX;zoom_index++)
				{
					// debug_printf("%d %d\n",zoom_wide_array[zoom_index] , monitor_rect.right - monitor_rect.left);
				
					if ((zoom_wide_array[zoom_index] > monitor_rect.right - monitor_rect.left) || (zoom_high_array[zoom_index] > monitor_rect.bottom - monitor_rect.top))
					{
						break;
					}
							
					_viv_fullscreen_zoom_offset = zoom_index;
				}	
				
				if (_viv_fullscreen_zoom_offset > _viv_zoom_pos)
				{
					_viv_fullscreen_zoom_offset = _viv_zoom_pos;
				}
			}
			else
			if (config_fill_window)
			{
				int backup_zoom_pos;
				
				backup_zoom_pos = _viv_zoom_pos;
				
				for(_viv_zoom_pos=0;_viv_zoom_pos<_VIV_ZOOM_MAX;_viv_zoom_pos++)
				{
					int rw;
					int rh;
					
					_viv_get_render_size(&rw,&rh);
					
					// debug_printf("%d %d\n",zoom_wide_array[zoom_index] , monitor_rect.right - monitor_rect.left);
				
					if ((rw > old_rw) || (rh > old_rh))
					{
						break;
					}
							
					_viv_fullscreen_zoom_offset = -_viv_zoom_pos;
				}

				_viv_zoom_pos = backup_zoom_pos;
				
				if (_viv_fullscreen_zoom_offset < -(_VIV_ZOOM_MAX-1-_viv_zoom_pos))
				{
					_viv_fullscreen_zoom_offset = -(_VIV_ZOOM_MAX-1-_viv_zoom_pos);
				}
			}
			
//			debug_printf("ZOOM OFFSET %d\n",_viv_fullscreen_zoom_offset);
		}
	
		_viv_zoom_pos -= _viv_fullscreen_zoom_offset;
		_viv_zoom_pos = _viv_clamp_zoom_pos(_viv_zoom_pos);
		
		_viv_prevent_on_deactivate = 0;
	}
	
	_viv_prevent_on_size = 0;

	_viv_on_size();
	
//	InvalidateRect(_viv_hwnd,0,FALSE);

	// update mouseover
	// so we show the cursor corectly in
	// _viv_update_show_cursor
	{
		POINT cursor_pt;
		HWND hwnd_mouseover;
		int is_mouseover;
		
		GetCursorPos(&cursor_pt);
		is_mouseover = 0;
		
		hwnd_mouseover = WindowFromPoint(cursor_pt);
		if (hwnd_mouseover == _viv_hwnd)
		{
			RECT client_rect;
			
			ScreenToClient(_viv_hwnd,&cursor_pt);
			
			GetClientRect(_viv_hwnd,&client_rect);
			
			if (PtInRect(&client_rect,cursor_pt))
			{
				is_mouseover = 1;
			}
		}
		
		_viv_is_mouseover = is_mouseover;
	}

	_viv_update_show_cursor();
}

static void _viv_slideshow(void)
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

static void _viv_get_render_size(int *prw,int *prh)
{
	RECT rect;
	int wide;
	int high;
	int rw;
	int rh;
	int fill_window;
	int max_zoom_wide;
	int max_zoom_high;
	
	if (!((_viv_image_wide) && (_viv_image_high)))
	{
		*prw = 0;
		*prh = 0;
		
		return;
	}
	
	GetClientRect(_viv_hwnd,&rect);
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_controls_high();
	
	if (!((wide) && (high)))
	{
		*prw = 0;
		*prh = 0;
		
		return;
	}

	if ((_viv_1to1) || (_viv_doing == _VIV_DOING_1TO1SCROLL))
	{
		*prw = _viv_image_wide;
		*prh = _viv_image_high;
		
		return;
	}
			
	if (_viv_is_fullscreen)
	{
		fill_window = config_fullscreen_fill_window;
	}
	else
	{
		fill_window = config_fill_window;
	}
	
	if (config_keep_aspect_ratio)
	{
		if ((high * _viv_image_wide) / _viv_image_high < wide)
		{
			// tall image.
			// add  _viv_image_high - 1 so when we resize the window to 50% it stretches to the screen edges correctly.
			rh = high;
			rw = ((high * (__int64)_viv_image_wide) + _viv_image_high - 1) / _viv_image_high;

			// make sure we have some width.			
			if (rw <= 0)
			{
				rw = 1;
			}
		}
		else
		{
			// long image.
			// add _viv_image_wide - 1 so when we resize the window to 50% it stretches to the screen edges correctly.
			rw = wide;
			rh = ((wide * (__int64)_viv_image_high) + _viv_image_wide - 1) / _viv_image_wide;

			// make sure we have some height	
			if (rh <= 0)
			{
				rh = 1;
			}
		}

		if (!fill_window)
		{
			if ((rw > _viv_image_wide) || (rh > _viv_image_high))
			{
				rw = _viv_image_wide;
				rh = _viv_image_high;
			}
		}
	}
	else
	{
		rw = wide;
		rh = high;

		if (!fill_window)
		{
			if (rw > _viv_image_wide)
			{
				rw = _viv_image_wide;
			}			
	
			if (rh > _viv_image_high)
			{
				rh = _viv_image_high;
			}
		}
	}

//	debug_printf("GET_RENDER_SIZE wide %d high %d rw %d rh %d fill_window %d\n",wide,high,rw,rh,fill_window);

	if (!config_allow_shrinking)
	{
		if (config_keep_aspect_ratio)
		{
			if ((rw < _viv_image_wide) || (rh < _viv_image_high))
			{
				rw = _viv_image_wide;
				rh = _viv_image_high;
			}
		}
		else
		{
			if (rw < _viv_image_wide)
			{
				rw = _viv_image_wide;
			}

			if (rh < _viv_image_high)
			{
				rh = _viv_image_high;
			}
		}
	}		
	
	if (config_keep_aspect_ratio)
	{
		if (rw > _viv_image_wide)
		{
			max_zoom_wide = rw * 16;
		}
		else
		{
			max_zoom_wide = _viv_image_wide * 16;
		}
		
		if (rh > _viv_image_high)
		{
			max_zoom_high = rh * 16;
		}
		else
		{
			max_zoom_high = _viv_image_high * 16;
		}
	}
	else
	{
		max_zoom_wide = rw * 16;
		max_zoom_high = rh * 16;
	}
/*
	if (max_zoom_wide < _viv_image_wide)
	{
		max_zoom_wide = _viv_image_wide;
	}
	
	if (max_zoom_high < _viv_image_high)
	{
		max_zoom_high = _viv_image_high;
	}
	*/
	if (_viv_zoom_pos)
	{
		rw = rw + (int)((max_zoom_wide - rw) * _viv_zoom_presets[_viv_zoom_pos]);
		rh = rh + (int)((max_zoom_high - rh) * _viv_zoom_presets[_viv_zoom_pos]);
	}
	
	*prw = rw;
	*prh = rh;
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

static void _viv_check_menus(HMENU hmenu)
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
	EnableMenuItem(hmenu,VIV_ID_FILE_RENAME,is_image_enabled);

	EnableMenuItem(hmenu,VIV_ID_EDIT_ROTATE_270,is_image_enabled);
	EnableMenuItem(hmenu,VIV_ID_EDIT_ROTATE_90,is_image_enabled);
//	EnableMenuItem(hmenu,VIV_ID_SLIDESHOW_PAUSE,is_image_enabled);

	CheckMenuItem(hmenu,VIV_ID_VIEW_CAPTION,config_show_caption ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_THICKFRAME,config_show_thickframe ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_MENU,config_show_menu ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu,VIV_ID_VIEW_CONTROLS,config_show_controls ? MF_CHECKED : MF_UNCHECKED);
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

static INT_PTR CALLBACK _viv_rename_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			wchar_t name[STRING_SIZE];
			
			os_center_dialog(hwnd);
			
			string_copy(name,string_get_filename_part((wchar_t *)lParam));
			string_remove_extension(name);

			SetDlgItemText(hwnd,IDC_RENAME_OLD_EDIT,(wchar_t *)lParam);
			SetDlgItemText(hwnd,IDC_RENAME_EDIT,name);

			return TRUE;
		}
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					wchar_t old_filename[STRING_SIZE];
					wchar_t new_filename[STRING_SIZE];
					wchar_t path[STRING_SIZE];
					wchar_t new_full_path_and_filename[STRING_SIZE];
					wchar_t *ext;
					
					GetDlgItemText(hwnd,IDC_RENAME_OLD_EDIT,old_filename,STRING_SIZE);
					GetDlgItemText(hwnd,IDC_RENAME_EDIT,new_filename,STRING_SIZE);
					
					string_get_path_part(path,old_filename);
					ext = string_get_extension(old_filename);
					
					string_path_combine(new_full_path_and_filename,path,new_filename);
					if (*ext)
					{
						string_cat_utf8(new_full_path_and_filename,(const utf8_t *)".");
						string_cat(new_full_path_and_filename,ext);
					}
					
					if (string_compare(old_filename,new_full_path_and_filename) != 0)
					{
						SHFILEOPSTRUCT fo;
						
						wchar_t old_filename_list[STRING_SIZE+1];
						wchar_t new_filename_list[STRING_SIZE+1];
						
						string_copy_double_null(old_filename_list,old_filename);
						string_copy_double_null(new_filename_list,new_full_path_and_filename);
						
						ZeroMemory(&fo,sizeof(SHFILEOPSTRUCT));
						fo.hwnd = _viv_hwnd;
						fo.wFunc = FO_RENAME;
						fo.pFrom = old_filename_list;
						fo.pTo = new_filename_list;
						fo.fFlags = FOF_ALLOWUNDO;

						if (SHFileOperation(&fo) == 0)
						{
							if (!fo.fAnyOperationsAborted)
							{	
								// has the filename changed? slideshow could make this a different filename
								if (string_compare(old_filename,_viv_current_fd->cFileName) == 0)
								{
									// rename
									string_copy(_viv_current_fd->cFileName,new_full_path_and_filename);
									
									_viv_playlist_rename(old_filename,new_full_path_and_filename);

									_viv_update_title();
								}
							}
						}
					}

					EndDialog(hwnd,1);
					break;
				}

				case IDCANCEL:
					EndDialog(hwnd,0);
					break;
			}

			break;
	}
	
	return FALSE;
}

static void _viv_rename(void)
{
	if (*_viv_current_fd->cFileName)
	{
		DialogBoxParam(os_hinstance,MAKEINTRESOURCE(IDD_RENAME),_viv_hwnd,_viv_rename_proc,(LPARAM)_viv_current_fd->cFileName);
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
				
				wlen = string_length(_viv_current_fd->cFileName);
				
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
				
				wlen = string_length(_viv_current_fd->cFileName);
				
				hmem = GlobalAlloc(GMEM_MOVEABLE,(wlen + 1) * sizeof(wchar_t));
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

static void _viv_set_clipboard_image(void)
{
	if (_viv_frame_count)
	{
		if (_viv_frames[_viv_frame_position].hbitmap)
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

						mem1_hbitmap = CreateCompatibleBitmap(screen_hdc,_viv_image_wide,_viv_image_high);
						if (mem1_hbitmap)
						{
							HGDIOBJ last_mem1_hbitmap;
							HGDIOBJ last_mem2_hbitmap;
							
							last_mem1_hbitmap = SelectObject(mem1_hdc,mem1_hbitmap);
							last_mem2_hbitmap = SelectObject(mem2_hdc,_viv_frames[_viv_frame_position].hbitmap);
							
							BitBlt(mem1_hdc,0,0,_viv_image_wide,_viv_image_high,mem2_hdc,0,0,SRCCOPY);

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

static CLIPFORMAT _viv_get_CF_PREFERREDDROPEFFECT(void)
{
	if (!_viv_CF_PREFERREDDROPEFFECT)
	{
		_viv_CF_PREFERREDDROPEFFECT = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
	}
	
	return _viv_CF_PREFERREDDROPEFFECT;
}

static void _viv_pause(void)
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

static void _viv_increase_animation_rate(int dec)
{
	if (dec)	
	{
		if (_viv_animation_rate_pos)
		{
			_viv_animation_rate_pos--;
		}
	}
	else
	{
		if (_viv_animation_rate_pos < _VIV_ANIMATION_RATE_MAX-1)
		{
			_viv_animation_rate_pos++;
		}
	}
	
	_viv_status_update_temp_animation_rate();
}

static void _viv_reset_animation_rate(void)
{
	_viv_animation_rate_pos = _VIV_ANIMATION_RATE_ONE;

	_viv_status_update_temp_animation_rate();
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

static void _viv_doing_cancel(void)
{
	if (_viv_doing)
	{
		int was_doing;
		
		was_doing = _viv_doing;
		
		_viv_doing = _VIV_DOING_NOTHING;
		
		if (was_doing == _VIV_DOING_MSCROLL)
		{
			ShowCursor(TRUE);
		}

		if (was_doing == _VIV_DOING_1TO1SCROLL)
		{
//			ShowCursor(TRUE);
			_viv_view_set(_viv_doing_x,_viv_doing_y,1);
			InvalidateRect(_viv_hwnd,0,FALSE);
		}
		
		ReleaseCapture();
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


static void _viv_blank(void)
{
	_viv_clear();

	if (_viv_random)
	{
		mem_free(_viv_random);
		
		_viv_random = 0;
	}
	
	_viv_current_fd->cFileName[0] = 0;

	_viv_update_src_pixel(1,0);
	_viv_status_update();
	_viv_update_title();

	// free all dropfiles
	_viv_playlist_clearall();

	_viv_start_first_frame();
	_viv_process_pending_clear();
}

static INT_PTR CALLBACK _viv_options_general_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			int exti;

			if (config_appdata) 
			{
				CheckDlgButton(hwnd,IDC_APPDATA,BST_CHECKED);
			}
			
			if (config_multiple_instances) 
			{
				CheckDlgButton(hwnd,IDC_MULTIPLE_INSTANCES,BST_CHECKED);
			}
			
			for(exti=0;exti<_VIV_ASSOCIATION_COUNT;exti++)
			{
				if (_viv_is_association(_viv_association_extensions[exti])) 
				{
					CheckDlgButton(hwnd,_viv_association_dlg_item_id[exti],BST_CHECKED);
				}
			}
			
			if (_viv_is_start_menu_shortcuts()) 
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
					_viv_options_update_sheild(GetParent(hwnd));
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
					CheckDlgButton(hwnd,IDC_WEBP,check);
					
					break;
				}
			}
					
			break;

	}
	
	return FALSE;
}

static void _viv_options_key_list_sel_change(HWND hwnd,int previous_key_index)
{
	int index;
	_viv_key_list_t *keylist;
	int key_count;

	keylist = (_viv_key_list_t *)GetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA);
	
	index = ListBox_GetCurSel(GetDlgItem(hwnd,IDC_COMMANDS_LIST));
	
	ListBox_ResetContent(GetDlgItem(hwnd,IDC_KEYS_LIST));
	SetWindowRedraw(GetDlgItem(hwnd,IDC_KEYS_LIST),FALSE);
	
	key_count = 0;
	
	if (index != LB_ERR)
	{
		int command_index;
		config_key_t *key;
		
		command_index = ListBox_GetItemData(GetDlgItem(hwnd,IDC_COMMANDS_LIST),index);
		
		key = keylist->start[command_index];
		
		while(key)
		{
			wchar_t key_text[STRING_SIZE];
			int listbox_index;
			
			_viv_get_key_text(key_text,key->key);
			
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

static void _viv_options_remove_key(HWND hwnd)
{
	int index;
	_viv_key_list_t *keylist;

	keylist = (_viv_key_list_t *)GetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA);
	
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
			
			_viv_key_remove(keylist,command_index,keyflags);
			
			_viv_options_key_list_sel_change(hwnd,key_index);
		}
	}
}

static INT_PTR CALLBACK _viv_edit_key_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
			
			_viv_edit_key_set_key(GetDlgItem(hwnd,IDC_EDIT_KEY_EDIT),lParam);
			_viv_options_edit_key_changed(hwnd);
			
			string_printf(caption_wbuf,localization_get_string(lParam ? LOCALIZATION_OPTIONS_EDIT_KEYBOARD_SHORTCUT_CAPTION : LOCALIZATION_OPTIONS_ADD_KEYBOARD_SHORTCUT_CAPTION));
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

static void _viv_options_edit_key(HWND hwnd,int key_index)
{
	int index;
	_viv_key_list_t *keylist;

	keylist = (_viv_key_list_t *)GetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA);
	
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
		
		keyflags = DialogBoxParam(os_hinstance,MAKEINTRESOURCE(IDD_EDIT_KEY),hwnd,_viv_edit_key_proc,keyflags);

		_viv_edit_key_remove_currently_used_by(keylist,keyflags);

		if (keyflags)
		{
			int command_index;

			command_index = ListBox_GetItemData(GetDlgItem(hwnd,IDC_COMMANDS_LIST),index);
			
			if (key_index == LB_ERR)
			{
				// new
				_viv_key_add(keylist,command_index,keyflags);
			}
			else
			{
				config_key_t *key;
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
			
			_viv_options_key_list_sel_change(hwnd,key_index);			
		}
	}
}

static INT_PTR CALLBACK _viv_options_controls_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			os_ComboBox_AddString(hwnd,IDC_LEFTCLICKACTION,localization_get_string(LOCALIZATION_OPTIONS_ACTION_SCROLL_COMBOBOX));
			os_ComboBox_AddString(hwnd,IDC_LEFTCLICKACTION,localization_get_string(LOCALIZATION_OPTIONS_ACTION_PLAY_PAUSE_SLIDESHOW_COMBOBOX));
			os_ComboBox_AddString(hwnd,IDC_LEFTCLICKACTION,localization_get_string(LOCALIZATION_OPTIONS_ACTION_PLAY_PAUSE_ANIMATION_COMBOBOX));
			os_ComboBox_AddString(hwnd,IDC_LEFTCLICKACTION,localization_get_string(LOCALIZATION_OPTIONS_ACTION_ZOOM_IN_COMBOBOX));
			os_ComboBox_AddString(hwnd,IDC_LEFTCLICKACTION,localization_get_string(LOCALIZATION_OPTIONS_ACTION_NEXT_IMAGE_COMBOBOX));
			os_ComboBox_AddString(hwnd,IDC_LEFTCLICKACTION,localization_get_string(LOCALIZATION_OPTIONS_ACTION_ONE_TO_ONE_SCROLL_COMBOBOX));
			os_ComboBox_AddString(hwnd,IDC_LEFTCLICKACTION,localization_get_string(LOCALIZATION_OPTIONS_ACTION_SCROLL_MOVE_WINDOW_COMBOBOX));
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_LEFTCLICKACTION),config_left_click_action);
			
			os_ComboBox_AddString(hwnd,IDC_RIGHTCLICKACTION,localization_get_string(LOCALIZATION_OPTIONS_ACTION_CONTEXT_MENU_COMBOBOX));
			os_ComboBox_AddString(hwnd,IDC_RIGHTCLICKACTION,localization_get_string(LOCALIZATION_OPTIONS_ACTION_ZOOM_OUT_COMBOBOX));
			os_ComboBox_AddString(hwnd,IDC_RIGHTCLICKACTION,localization_get_string(LOCALIZATION_OPTIONS_ACTION_PREVIOUS_IMAGE_COMBOBOX));
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_RIGHTCLICKACTION),config_right_click_action);
			
			os_ComboBox_AddString(hwnd,IDC_MOUSEWHEELACTION,localization_get_string(LOCALIZATION_OPTIONS_ACTION_ZOOM_COMBOBOX));
			os_ComboBox_AddString(hwnd,IDC_MOUSEWHEELACTION,localization_get_string(LOCALIZATION_OPTIONS_ACTION_NEXT_PREV_COMBOBOX));
			os_ComboBox_AddString(hwnd,IDC_MOUSEWHEELACTION,localization_get_string(LOCALIZATION_OPTIONS_ACTION_PREV_NEXT_COMBOBOX));
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_MOUSEWHEELACTION),config_mouse_wheel_action);
			
			{
				int i;
				_viv_key_list_t *keylist;
				
				keylist = mem_alloc(sizeof(_viv_key_list_t));

				_viv_key_list_init(keylist);
				
				_viv_key_list_copy(keylist,_viv_key_list);
				
				SetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA,(LONG_PTR)keylist);
				
				for(i=0;i<_VIV_COMMAND_COUNT;i++)
				{
					if (!(_viv_commands[i].flags & MF_POPUP))
					{
						if (!(_viv_commands[i].flags & MF_SEPARATOR))
						{
							if (!(_viv_commands[i].flags & MF_DELETE))
							{
								wchar_t command_name_wbuf[STRING_SIZE];
								int index;

								_viv_get_command_name(command_name_wbuf,i);
								
								index = ListBox_AddString(GetDlgItem(hwnd,IDC_COMMANDS_LIST),command_name_wbuf);
								if (index != LB_ERR)
								{
									ListBox_SetItemData(GetDlgItem(hwnd,IDC_COMMANDS_LIST),index,i);
								}
							}
						}
					}
				}
			}
			
			ListBox_SetCurSel(GetDlgItem(hwnd,IDC_COMMANDS_LIST),0);
			_viv_options_key_list_sel_change(hwnd,0);
			
			return FALSE;
		}
		
		case WM_COMMAND:

			switch(LOWORD(wParam))
			{
				case IDC_COMMANDS_LIST:
					if (HIWORD(wParam) == LBN_SELCHANGE) 
					{	
						_viv_options_key_list_sel_change(hwnd,0);
					}
					break;
					
				case IDC_REMOVE_KEY:
					_viv_options_remove_key(hwnd);
					break;
					
				case IDC_EDIT_KEY:
					_viv_options_edit_key(hwnd,ListBox_GetCurSel(GetDlgItem(hwnd,IDC_KEYS_LIST)));
					break;
					
				case IDC_ADD_KEY:
					_viv_options_edit_key(hwnd,LB_ERR);
					break;
			}
			break;
		
		case WM_DESTROY:
		{
			_viv_key_list_t *keylist;
			
			keylist = (_viv_key_list_t *)GetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA);
			
			_viv_key_clear_all(keylist);
			mem_free(keylist);
			
			break;
		}
	}
	
	return FALSE;
}
static INT_PTR CALLBACK _viv_options_view_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			
			os_ComboBox_AddString(hwnd,IDC_COMBO1,(const utf8_t *)"COLORONCOLOR (Performance)");
			os_ComboBox_AddString(hwnd,IDC_COMBO1,(const utf8_t *)"HALFTONE (Quality)");
			
			if (config_shrink_blit_mode == CONFIG_SHRINK_BLIT_MODE_HALFTONE)
			{
				ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_COMBO1),1);
			}
			else
			{
				ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_COMBO1),0);
			}

			os_ComboBox_AddString(hwnd,IDC_COMBO2,(const utf8_t *)"COLORONCOLOR (Performance)");
			os_ComboBox_AddString(hwnd,IDC_COMBO2,(const utf8_t *)"HALFTONE (Quality)");
			
			if (config_mag_filter == CONFIG_MAG_FILTER_HALFTONE)
			{
				ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_COMBO2),1);
			}
			else
			{
				ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_COMBO2),0);
			}
					
			CheckDlgButton(hwnd,IDC_AUTO_ZOOM,config_auto_zoom ? BST_CHECKED : BST_UNCHECKED);
			
			os_ComboBox_AddString(hwnd,IDC_COMBO4,(const utf8_t *)"50%");
			os_ComboBox_AddString(hwnd,IDC_COMBO4,(const utf8_t *)"100%");
			os_ComboBox_AddString(hwnd,IDC_COMBO4,(const utf8_t *)"200%");
			os_ComboBox_AddString(hwnd,IDC_COMBO4,(const utf8_t *)"Auto Fit");
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_COMBO4),config_auto_zoom_type);
			EnableWindow(GetDlgItem(hwnd,IDC_COMBO4),IsDlgButtonChecked(hwnd,IDC_AUTO_ZOOM) == BST_CHECKED);

			CheckDlgButton(hwnd,IDC_LOOP_ANIMATIONS_ONCE,config_loop_animations_once ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwnd,IDC_PRELOAD_NEXT_IMAGE,config_preload_next ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwnd,IDC_CACHE_LAST_IMAGE,config_cache_last ? BST_CHECKED : BST_UNCHECKED);
			
			SetWindowLongPtr(GetDlgItem(hwnd,IDC_WINDOWEDBACKGROUNDCOLOR),GWLP_USERDATA,RGB(config_windowed_background_color_r,config_windowed_background_color_g,config_windowed_background_color_b));
			_viv_update_color_button_bitmap(GetDlgItem(hwnd,IDC_WINDOWEDBACKGROUNDCOLOR));

			SetWindowLongPtr(GetDlgItem(hwnd,IDC_FULLSCREENBACKGROUNDCOLOR),GWLP_USERDATA,RGB(config_fullscreen_background_color_r,config_fullscreen_background_color_g,config_fullscreen_background_color_b));
			_viv_update_color_button_bitmap(GetDlgItem(hwnd,IDC_FULLSCREENBACKGROUNDCOLOR));
					
			return FALSE;
			
		case WM_DESTROY:	
			_viv_delete_color_button_bitmap(GetDlgItem(hwnd,IDC_WINDOWEDBACKGROUNDCOLOR));
			_viv_delete_color_button_bitmap(GetDlgItem(hwnd,IDC_FULLSCREENBACKGROUNDCOLOR));
			break;
		
		case WM_COMMAND:

			switch(LOWORD(wParam))
			{
				case IDC_AUTO_ZOOM:
				
					EnableWindow(GetDlgItem(hwnd,IDC_COMBO4),IsDlgButtonChecked(hwnd,IDC_AUTO_ZOOM) == BST_CHECKED);
				
					break; 
				
				case IDC_WINDOWEDBACKGROUNDCOLOR:
				case IDC_FULLSCREENBACKGROUNDCOLOR:
					
					{
						COLORREF color;
						
						color = GetWindowLongPtr(GetDlgItem(hwnd,LOWORD(wParam)),GWLP_USERDATA);
						
						if (os_choose_color(hwnd,&color))
						{
							SetWindowLongPtr(GetDlgItem(hwnd,LOWORD(wParam)),GWLP_USERDATA,color);
							_viv_update_color_button_bitmap(GetDlgItem(hwnd,LOWORD(wParam)));
						}
					}
					
					break;
			}
			
			break;
	}
	
	return FALSE;
}

static void _viv_options_treeview_changed(HWND hwnd)
{
	TV_ITEM tvi;

	// get the current index
	tvi.hItem = TreeView_GetSelection(GetDlgItem(hwnd,IDC_TREE1));
	if (tvi.hItem)
	{
		int i;

		tvi.mask = TVIF_PARAM;
	    
		TreeView_GetItem(GetDlgItem(hwnd,IDC_TREE1),&tvi);
		
		ShowWindow(GetDlgItem(hwnd,_viv_options_tab_ids[tvi.lParam]),SW_SHOW);
		ShowWindow(GetDlgItem(hwnd,_viv_options_page_ids[tvi.lParam]),SW_SHOW);
		
		for(i=0;i<_VIV_OPTIONS_PAGE_COUNT;i++)
		{
			if (i != tvi.lParam)
			{
				ShowWindow(GetDlgItem(hwnd,_viv_options_tab_ids[i]),SW_HIDE);
				ShowWindow(GetDlgItem(hwnd,_viv_options_page_ids[i]),SW_HIDE);
			}
		}

		config_options_last_page = tvi.lParam;
	}
}

static void _viv_options_update_sheild(HWND hwnd)
{
//	int exti;
	int need_admin;
	HWND general_page;
	
	general_page = GetDlgItem(hwnd,VIV_ID_OPTIONS_GENERAL);
	
	need_admin = 0;
	/*
	for(exti=0;exti<_VIV_ASSOCIATION_COUNT;exti++)
	{
		if (IsDlgButtonChecked(general_page,_viv_association_dlg_item_id[exti]) == BST_CHECKED) 
		{
			if (!_viv_is_association(_viv_association_extensions[exti]))
			{
				need_admin = 1;
			}
		}
		else 
		{
			if (_viv_is_association(_viv_association_extensions[exti]))
			{
				need_admin = 1;
			}
		}
	}
*/
	if ((!!config_appdata) != (IsDlgButtonChecked(general_page,IDC_APPDATA) == BST_CHECKED)) 
	{
		need_admin = 1;
	}
	
	if ((IsDlgButtonChecked(general_page,IDC_STARTMENU) == BST_CHECKED) != !!_viv_is_start_menu_shortcuts()) 
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

static INT_PTR CALLBACK _viv_options_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
								_viv_options_treeview_changed(hwnd);
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
				
				for(i=0;i<_VIV_OPTIONS_PAGE_COUNT;i++)
				{
					TV_INSERTSTRUCT tvitem;
					TCITEM tcitem;
					RECT rect;
					HWND page_hwnd;
					HTREEITEM hitem;
					wchar_t text_wbuf[STRING_SIZE];
					
					string_copy_utf8(text_wbuf,localization_get_string(_viv_options_page_localization_id_array[i]));
					
					// treeview
					tvitem.hInsertAfter = TVI_LAST;
					tvitem.hParent = TVI_ROOT;
					tvitem.item.mask = TVIF_TEXT | TVIF_PARAM;
					tvitem.item.lParam = i;
					tvitem.item.pszText = text_wbuf;
					
					hitem = TreeView_InsertItem(GetDlgItem(hwnd,IDC_TREE1),&tvitem);

					if (i == config_options_last_page) 
					{
						TreeView_Select(GetDlgItem(hwnd,IDC_TREE1),hitem,TVGN_CARET);
					}
					
					// tab
					string_copy_utf8(text_wbuf,localization_get_string(_viv_options_page_localization_id_array[i]));
					
					tcitem.mask = TCIF_TEXT;
					tcitem.pszText = text_wbuf;
					TabCtrl_InsertItem(GetDlgItem(hwnd,_viv_options_tab_ids[i]),0,&tcitem);

					// page
					GetClientRect(GetDlgItem(hwnd,IDC_PAGEPLACEHOLDER),&rect);
					MapWindowPoints(GetDlgItem(hwnd,IDC_PAGEPLACEHOLDER),hwnd,(LPPOINT)&rect,2);

					page_hwnd = CreateDialog(os_hinstance,MAKEINTRESOURCE(_viv_options_dialog_ids[i]),hwnd,_viv_options_page_procs[i]);
					SetWindowLong(page_hwnd,GWL_ID,_viv_options_page_ids[i]);
					
					if (os_EnableThemeDialogTexture)
					{
						os_EnableThemeDialogTexture(page_hwnd,ETDT_ENABLETAB);
					}

					SetWindowPos(page_hwnd,HWND_TOP,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOSIZE|SWP_NOACTIVATE);
				}
			}

			_viv_options_treeview_changed(hwnd);
			_viv_options_update_sheild(hwnd);

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
						config_multiple_instances = 0;
						
						if (IsDlgButtonChecked(general_page,IDC_APPDATA) == BST_CHECKED) 
						{
							if (!config_appdata)
							{
								_viv_append_admin_param(params,(const utf8_t *)"appdata");
								config_appdata = 1;
							}
						}
						else
						{
							if (config_appdata)
							{
								_viv_append_admin_param(params,(const utf8_t *)"noappdata");

								config_appdata = 0;
							}
						}

						if (IsDlgButtonChecked(general_page,IDC_MULTIPLE_INSTANCES) == BST_CHECKED) 
						{
							config_multiple_instances = 1;
						}
						
						if (IsDlgButtonChecked(general_page,IDC_STARTMENU) == BST_CHECKED) 
						{
							if (!_viv_is_start_menu_shortcuts())
							{
								_viv_append_admin_param(params,(const utf8_t *)"startmenu");
							}
						}
						else
						{
							if (_viv_is_start_menu_shortcuts())
							{
								_viv_append_admin_param(params,(const utf8_t *)"nostartmenu");
							}
						}
						
						
						for(exti=0;exti<_VIV_ASSOCIATION_COUNT;exti++)
						{
							if (IsDlgButtonChecked(general_page,_viv_association_dlg_item_id[exti]) == BST_CHECKED) 
							{
								if (!_viv_is_association(_viv_association_extensions[exti]))
								{
									_viv_install_association_by_extension(_viv_association_extensions[exti],_viv_association_descriptions[exti],_viv_association_icon_locations[exti]);
								}
							}
							else 
							{
								if (_viv_is_association(_viv_association_extensions[exti]))
								{
									_viv_uninstall_association_by_extension(_viv_association_extensions[exti]);
								}
							}
						}
						
						old_shrink_blit_mode = config_shrink_blit_mode;
						
						config_left_click_action = ComboBox_GetCurSel(GetDlgItem(controls_page,IDC_LEFTCLICKACTION));
						config_right_click_action = ComboBox_GetCurSel(GetDlgItem(controls_page,IDC_RIGHTCLICKACTION));
						config_mouse_wheel_action = ComboBox_GetCurSel(GetDlgItem(controls_page,IDC_MOUSEWHEELACTION));
						
						if (ComboBox_GetCurSel(GetDlgItem(view_page,IDC_COMBO1)) == 1)
						{
							config_shrink_blit_mode = CONFIG_SHRINK_BLIT_MODE_HALFTONE;
						}
						else
						{
							config_shrink_blit_mode = CONFIG_SHRINK_BLIT_MODE_COLORONCOLOR;
						}
						
						colorref = (COLORREF)GetWindowLongPtr(GetDlgItem(view_page,IDC_WINDOWEDBACKGROUNDCOLOR),GWLP_USERDATA);

						if ((GetRValue(colorref) != config_windowed_background_color_r) || (GetGValue(colorref) != config_windowed_background_color_g) || (GetBValue(colorref) != config_windowed_background_color_b))
						{
							config_windowed_background_color_r = GetRValue(colorref);
							config_windowed_background_color_g = GetGValue(colorref);
							config_windowed_background_color_b = GetBValue(colorref);

							InvalidateRect(_viv_hwnd,0,FALSE);
						}
						
						colorref = (COLORREF)GetWindowLongPtr(GetDlgItem(view_page,IDC_FULLSCREENBACKGROUNDCOLOR),GWLP_USERDATA);
						
						if ((GetRValue(colorref) != config_fullscreen_background_color_r) || (GetGValue(colorref) != config_fullscreen_background_color_g) || (GetBValue(colorref) != config_fullscreen_background_color_b))
						{
							config_fullscreen_background_color_r = GetRValue(colorref);
							config_fullscreen_background_color_g = GetGValue(colorref);
							config_fullscreen_background_color_b = GetBValue(colorref);

							InvalidateRect(_viv_hwnd,0,FALSE);
						}
						
						if (old_shrink_blit_mode != config_shrink_blit_mode)
						{
							InvalidateRect(_viv_hwnd,0,FALSE);
						}
						
						old_shrink_blit_mode = config_mag_filter;
						
						if (ComboBox_GetCurSel(GetDlgItem(view_page,IDC_COMBO2)) == 1)
						{
							config_mag_filter = CONFIG_SHRINK_BLIT_MODE_HALFTONE;
						}
						else
						{
							config_mag_filter = CONFIG_SHRINK_BLIT_MODE_COLORONCOLOR;
						}
						
						if (old_shrink_blit_mode != config_mag_filter)
						{
							InvalidateRect(_viv_hwnd,0,FALSE);
						}

						config_auto_zoom = IsDlgButtonChecked(view_page,IDC_AUTO_ZOOM) == BST_CHECKED ? 1 : 0;
						config_auto_zoom_type = ComboBox_GetCurSel(GetDlgItem(view_page,IDC_COMBO4));
						config_loop_animations_once = IsDlgButtonChecked(view_page,IDC_LOOP_ANIMATIONS_ONCE) == BST_CHECKED ? 1 : 0;
						config_preload_next = IsDlgButtonChecked(view_page,IDC_PRELOAD_NEXT_IMAGE) == BST_CHECKED ? 1 : 0;
						config_cache_last = IsDlgButtonChecked(view_page,IDC_CACHE_LAST_IMAGE) == BST_CHECKED ? 1 : 0;
						
						// copy keys.
						_viv_key_list_copy(_viv_key_list,(_viv_key_list_t *)GetWindowLongPtr(GetDlgItem(controls_page,IDC_COMMANDS_LIST),GWLP_USERDATA));
						
						// reinit menu.
						{
							HMENU new_hmenu;
							
							new_hmenu = _viv_create_menu();
							
							if (GetMenu(_viv_hwnd))
							{
								SetMenu(_viv_hwnd,new_hmenu);
							}
							
							if (_viv_hmenu)
							{
								DestroyMenu(_viv_hmenu);
							}
							
							_viv_hmenu = new_hmenu;
						}
						
						// do admin commands.
						if (*params)
						{	
							wchar_t exe_filename[STRING_SIZE];
							
							_viv_get_exe_filename(exe_filename);
							
							os_shell_execute(0,exe_filename,1,NULL,params);
						}

						// save settings to disk						
						config_save_settings(config_appdata);
					}
				
					EndDialog(hwnd,0);
					break;
			}

			break;
	}
	
	return FALSE;
}

static void _viv_options(void)
{	
	DialogBox(os_hinstance,MAKEINTRESOURCE(IDD_OPTIONS),_viv_hwnd,_viv_options_proc);
}

// use the default class description, ie: TXT File
static void _viv_install_association_by_extension(const char *association,const char *description,const char *icon_location)
{
	HKEY hkey;
	wchar_t class_name[STRING_SIZE];
	wchar_t key[STRING_SIZE];
	wchar_t default_icon[STRING_SIZE];
	wchar_t dot_association[STRING_SIZE];
	LONG reg_ret;
	
	string_copy_utf8(dot_association,(const utf8_t *)".");
	string_cat_utf8(dot_association,association);
	
	// make sure we uninstall old associations first.
	_viv_uninstall_association_by_extension(association);

	string_copy_utf8(class_name,"voidImageViewer");
	string_cat(class_name,dot_association);

	string_copy_utf8(default_icon,"SOFTWARE\\Classes\\voidImageViewer");
	string_cat(default_icon,dot_association);
	string_cat_utf8(default_icon,"\\DefaultIcon");

	if (RegCreateKeyExW(HKEY_CURRENT_USER,default_icon,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		if (icon_location)
		{
			wchar_t icon_location_wbuf[STRING_SIZE];
			
			string_copy_utf8(icon_location_wbuf,icon_location);

			_viv_set_registry_string(hkey,0,icon_location_wbuf);
		}
		else
		{
			wchar_t filename[STRING_SIZE];
			wchar_t command[STRING_SIZE];

			_viv_get_exe_filename(filename);
			
			string_copy(command,filename);

			string_cat_utf8(command,(const utf8_t *)",0");
			
			_viv_set_registry_string(hkey,0,command);
		}
			
		RegCloseKey(hkey);
	}	
		
	string_copy_utf8(key,"SOFTWARE\\Classes\\");
	string_cat(key,class_name);
	
	if (RegCreateKeyExW(HKEY_CURRENT_USER,key,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t description_wbuf[STRING_SIZE];
		
		string_copy_utf8(description_wbuf,description);
		
		_viv_set_registry_string(hkey,0,description_wbuf);
		
		RegCloseKey(hkey);
	}		
	
	string_copy_utf8(key,"SOFTWARE\\Classes\\");
	string_cat(key,class_name);
	string_cat_utf8(key,"\\shell\\open\\command");
	
	if (RegCreateKeyExW(HKEY_CURRENT_USER,key,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t filename[STRING_SIZE];
		wchar_t command[STRING_SIZE];

		_viv_get_exe_filename(filename);
		
		string_copy_utf8(command,(const utf8_t *)"\"");
		string_cat(command,filename);
		string_cat_utf8(command,(const utf8_t *)"\" \"%1\"");
		
		_viv_set_registry_string(hkey,0,command);
		
		RegCloseKey(hkey);
	}

	string_copy_utf8(key,"SOFTWARE\\Classes\\");
	string_cat(key,dot_association);
	
	reg_ret = RegCreateKeyExW(HKEY_CURRENT_USER,key,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0);
	if (reg_ret == ERROR_SUCCESS)
	{
		wchar_t wbuf[STRING_SIZE];
		
		if (!_viv_get_registry_string(hkey,(const utf8_t *)"voidImageViewer.Backup",wbuf,STRING_SIZE))
		{
			if (!_viv_get_registry_string(hkey,0,wbuf,STRING_SIZE))
			{
				*wbuf = 0;
			}
			
			_viv_set_registry_string(hkey,(const utf8_t *)"voidImageViewer.Backup",wbuf);
		}

		_viv_set_registry_string(hkey,0,class_name);
		
		RegCloseKey(hkey);
	}
	else
	{
		debug_printf("RegCreateKeyExW failed %u\n",reg_ret);
	}
}

static void _viv_uninstall_association_by_extension(const char *association)
{
	long reg_ret;
	HKEY hkey;
	wchar_t key[STRING_SIZE];
	
	string_copy_utf8(key,(const utf8_t *)"SOFTWARE\\Classes\\.");
	string_cat_utf8(key,association);
	
	// debug_printf("query %S\n",key);
	
	if (RegCreateKeyExW(HKEY_CURRENT_USER,key,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t wbuf[STRING_SIZE];
		
		if (_viv_get_registry_string(hkey,(const utf8_t *)"voidImageViewer.Backup",wbuf,STRING_SIZE))
		{
			_viv_set_registry_string(hkey,0,wbuf);

			// debug_printf("Delete voidImageViewer.Backup\n");

			reg_ret = RegDeleteValueA(hkey,"voidImageViewer.Backup");
			if (reg_ret == ERROR_SUCCESS)
			{
				// debug_printf("Delete voidImageViewer.Backup OK\n");
			}
			else
			{
				debug_printf("RegDeleteValueA failed %u\n",reg_ret);
			}
		}

		RegCloseKey(hkey);
	}
	
	string_copy_utf8(key,(const utf8_t *)"SOFTWARE\\Classes\\voidImageViewer.");
	string_cat_utf8(key,association);
	
	RegDeleteKey(HKEY_CURRENT_USER,key);
}

static int _viv_is_association(const char *association)
{
	int ret;
	HKEY hkey;
	wchar_t class_name[STRING_SIZE];
	wchar_t key[STRING_SIZE];
	
	string_copy_utf8(class_name,(const utf8_t *)"voidImageViewer.");
	string_cat_utf8(class_name,association);
	
	ret = 0;

	string_copy_utf8(key,"SOFTWARE\\Classes\\.");
	string_cat_utf8(key,association);
	
//debug_printf("key %S\n",key);
	
	if (RegOpenKeyExW(HKEY_CURRENT_USER,key,0,KEY_QUERY_VALUE,&hkey) == ERROR_SUCCESS)
	{
		wchar_t wbuf[STRING_SIZE];

//debug_printf("key OK\n",key);

		if (_viv_get_registry_string(hkey,0,wbuf,STRING_SIZE))
		{
//debug_printf("value cmp %S %S\n",wbuf,class_name);
			if (string_compare(wbuf,class_name) == 0)
			{
				ret++;
			}
		}

		RegCloseKey(hkey);
	}

	string_copy_utf8(key,"SOFTWARE\\Classes\\");
	string_cat(key,class_name);
	string_cat_utf8(key,(const utf8_t *)"\\shell\\open\\command");
	
//debug_printf("key %S\n",key);
	
	if (RegOpenKeyExW(HKEY_CURRENT_USER,key,0,KEY_QUERY_VALUE,&hkey) == ERROR_SUCCESS)
	{
		wchar_t wbuf[STRING_SIZE];

		if (_viv_get_registry_string(hkey,0,wbuf,STRING_SIZE))
		{
			wchar_t filename[STRING_SIZE];
			wchar_t command[STRING_SIZE];

//debug_printf("key OK\n",key);

			_viv_get_exe_filename(filename);
			
			string_copy_utf8(command,(const utf8_t *)"\"");
			string_cat(command,filename);
			string_cat_utf8(command,(const utf8_t *)"\" \"%1\"");

//debug_printf("value cmp %S %S\n",wbuf,command);
			
			if (string_compare(wbuf,command) == 0)
			{
				ret++;
			}
		}

		RegCloseKey(hkey);
	}

	return ret == 2;
}

static int _viv_get_registry_string(HKEY hkey,const utf8_t *value,wchar_t *wbuf,int size_in_wchars)
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

static int _viv_set_registry_string(HKEY hkey,const utf8_t *value,const wchar_t *wbuf)
{
	wchar_t value_wbuf[STRING_SIZE];
	wchar_t *value_wp;
	LONG reg_ret;

	if (value)
	{
		string_copy_utf8(value_wbuf,value);
		value_wp = value_wbuf;
	}
	else
	{
		value_wp = 0;
	}	

	reg_ret = RegSetValueExW(hkey,value_wp,0,REG_SZ,(BYTE *)wbuf,(string_length(wbuf) + 1) * sizeof(wchar_t));
	if (reg_ret == ERROR_SUCCESS)
	{
		return 1;
	}
	else
	{
		debug_printf("failed to set reg value %u %s %S",reg_ret,value,wbuf);
	}
	
	return 0;
}

VOID NTAPI _viv_timer_queue_timer_callback(PVOID param,BOOLEAN TimerOrWaitFired)
{
	if (!_viv_is_animation_timer_event)
	{
		_viv_is_animation_timer_event = 1;
		
		PostMessage(_viv_hwnd,WM_TIMER,VIV_ID_ANIMATION_TIMER,0);
	}
}

static void _viv_timer_start(void)
{
	if (!_viv_is_animation_timer)
	{
		_viv_is_animation_timer = 1;
		
		_viv_animation_timer_tick_start = os_get_tick_count();
		_viv_timer_tick = 0;
		
		if ((os_CreateTimerQueueTimer) && (os_DeleteTimerQueueTimer))
		{
			_viv_is_timer_queue_timer = 0;
			
			if (os_CreateTimerQueueTimer(&_viv_timer_queue_timer_handle,NULL,_viv_timer_queue_timer_callback,NULL,1,1,0))
			{
				_viv_is_timer_queue_timer = 1;
			}
		}
		else
		{
			SetTimer(_viv_hwnd,VIV_ID_ANIMATION_TIMER,USER_TIMER_MINIMUM,0);
		}
		
		_viv_update_prevent_sleep();
	}
}

static void _viv_mousemove(void)
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

static void _viv_update_src_pixel(int force,int update_statusbar)
{
//	if ((config_pixel_info) || ((_viv_is_alt) && (_viv_is_tracking_mouse)))
	if (config_pixel_info)
	{
		POINT src_pixel_pt;
		POINT client_pt;
		RECT client_rect;
		POINT pt;
		
		GetCursorPos(&pt);
		
		client_pt.x = pt.x;
		client_pt.y = pt.y;
		ScreenToClient(_viv_hwnd,&client_pt);
		GetClientRect(_viv_hwnd,&client_rect);
		
		if (_viv_get_src_pixel_pos(client_pt.x,client_pt.y,&src_pixel_pt))
		{
			
		}
		else
		{
			src_pixel_pt.x = -1;
			src_pixel_pt.y = -1;
		}

		if ((force) || (src_pixel_pt.x != _viv_src_pixel_x) || (src_pixel_pt.y != _viv_src_pixel_y))
		{
			COLORREF src_pixel_rgb;
			
			_viv_src_pixel_x = src_pixel_pt.x;
			_viv_src_pixel_y = src_pixel_pt.y;
			
			if ((_viv_src_pixel_x >= 0) && (_viv_src_pixel_y >= 0))
			{
				_viv_get_src_pixel_rgb(_viv_src_pixel_x,_viv_src_pixel_y,&src_pixel_rgb);
			}
			
			_viv_src_pixel_r = GetRValue(src_pixel_rgb);
			_viv_src_pixel_g = GetGValue(src_pixel_rgb);
			_viv_src_pixel_b = GetBValue(src_pixel_rgb);

			if (update_statusbar)
			{
				_viv_status_update();
			}
			
/*
			if ((src_pixel_pt.x != -1) && (src_pixel_pt.y != -1))
			{
				_viv_tooltip_update();
			}
			else
			{
				_viv_tooltip_hide();
			}*/
		}
	}
	else
	{
		_viv_src_pixel_x = -1;
		_viv_src_pixel_y = -1;
	}

/*	
	if ((_viv_is_alt) && (_viv_is_tracking_mouse))
	{
		_viv_tooltip_update_track_position();
	}
	else
	{
		_viv_tooltip_hide();
	}
	*/
}

static void _viv_animation_pause(void)
{
	_viv_animation_play = !_viv_animation_play;
}

static void _viv_frame_step(void)
{
	_viv_frame_looped = 0;
	
	if (_viv_animation_play)
	{
		_viv_animation_play = 0;
	}

	if (_viv_frame_count > 1)			
	{
		if ((_viv_frame_loaded_count == _viv_frame_count) || (_viv_frame_position + 1 < _viv_frame_loaded_count))
		{
			_viv_frame_position++;
			if (_viv_frame_position == _viv_frame_count)
			{
				_viv_frame_position = 0;
			}

			_viv_animation_timer_tick_start = os_get_tick_count();
			_viv_timer_tick = 0;
		
			_viv_update_src_pixel(1,0);
			_viv_status_update();
			
			InvalidateRect(_viv_hwnd,NULL,FALSE);
			UpdateWindow(_viv_hwnd);
		}
	}
}

static void _viv_frame_prev(void)
{
	_viv_frame_looped = 0;

	if (_viv_animation_play)
	{
		_viv_animation_play = 0;
	}

	if (_viv_frame_count > 1)
	{
		if (_viv_frame_position > 0)
		{
			_viv_frame_position--;
		}
		else
		{
			_viv_frame_position = _viv_frame_loaded_count - 1;
		}
	
		_viv_animation_timer_tick_start = os_get_tick_count();
		_viv_timer_tick = 0;
		
		_viv_update_src_pixel(1,0);
		_viv_status_update();
		
		InvalidateRect(_viv_hwnd,NULL,FALSE);
		UpdateWindow(_viv_hwnd);
	}
}

static void _viv_view_1to1(void)
{
	if (_viv_1to1)	
	{
		if (_viv_have_old_zoom)
		{
			_viv_1to1 = 0;
			_viv_zoom_pos = _viv_old_zoom_pos;
			_viv_view_set(_viv_view_x,_viv_view_y,1);
			InvalidateRect(_viv_hwnd,0,FALSE);
			
			return;
		}
	}
	
	_viv_have_old_zoom = 1;
	_viv_old_zoom_pos = _viv_zoom_pos;
	
	_viv_1to1 = 1;
	_viv_zoom_pos = 0;
	_viv_view_set(_viv_view_x,_viv_view_y,1);
	InvalidateRect(_viv_hwnd,0,FALSE);
}

static void _viv_playlist_add_current_if_empty(void)
{	
	// add current?
	if (!_viv_playlist_start)
	{
		if (*_viv_current_fd->cFileName)
		{
			_viv_playlist_add(_viv_current_fd);
		}
	}
}

static void _viv_playlist_clearall(void)
{	
	_viv_playlist_t *d;
	
	d = _viv_playlist_start;
	while(d)
	{
		_viv_playlist_t *next_d;
		
		next_d = d->next;
		
		mem_free(d);
		
		d = next_d;
	}
	
	if (_viv_playlist_shuffle_indexes)
	{
		mem_free(_viv_playlist_shuffle_indexes);

		_viv_playlist_shuffle_indexes = 0;
		_viv_playlist_shuffle_allocated = 0;
	}
	
	_viv_playlist_start = 0;
	_viv_playlist_last = 0;
	_viv_playlist_count = 0;
	_viv_playlist_id.QuadPart = 0;
}

static void _viv_playlist_delete(const WIN32_FIND_DATA *fd)
{
	if (_viv_playlist_shuffle_indexes)
	{
		int index;
		
		index = _viv_playlist_shuffle_index_from_fd(fd);
		if (index != -1)
		{
			_viv_playlist_t *d;
	
			d = _viv_playlist_shuffle_indexes[index];
			
			os_move_memory(_viv_playlist_shuffle_indexes + index,_viv_playlist_shuffle_indexes + index + 1,_viv_playlist_count - (index + 1));
			
			if (_viv_playlist_start == d)
			{
				_viv_playlist_start = d->next;
			}
			else
			{
				d->prev->next = d->next;
			}
		
			if (_viv_playlist_last == d)
			{
				_viv_playlist_last = d->prev;
			}
			else
			{
				d->next->prev = d->prev;
			}
		
			mem_free(d);

			_viv_playlist_count--;		
		}
	}
	else
	{
		_viv_playlist_t *d;
	
		d = _viv_playlist_from_fd(fd);
		if (d)
		{
			if (_viv_playlist_start == d)
			{
				_viv_playlist_start = d->next;
			}
			else
			{
				d->prev->next = d->next;
			}
		
			if (_viv_playlist_last == d)
			{
				_viv_playlist_last = d->prev;
			}
			else
			{
				d->next->prev = d->prev;
			}
		
			mem_free(d);

			_viv_playlist_count--;
		}
	}
}

static void _viv_playlist_rename(const wchar_t *old_filename,const wchar_t *new_filename)
{
	_viv_playlist_t *d;
	
	d = _viv_playlist_start;
	while(d)
	{
		if (string_compare(d->fd.cFileName,old_filename) == 0)
		{
			string_copy(d->fd.cFileName,new_filename);
		
			break;
		}
		
		d = d->next;
	}
}

static _viv_playlist_t *_viv_playlist_add(const WIN32_FIND_DATA *fd)
{
	_viv_playlist_t *d;
	
	d = (_viv_playlist_t *)mem_alloc(sizeof(_viv_playlist_t));
	
	os_copy_memory(&d->fd,fd,sizeof(WIN32_FIND_DATA));
	d->fd.dwReserved0 = _viv_playlist_id.HighPart;
	d->fd.dwReserved1 = _viv_playlist_id.LowPart;
	_viv_playlist_id.QuadPart = _viv_playlist_id.QuadPart + 1;
	
	if (_viv_playlist_start)
	{
		_viv_playlist_last->next = d;
		d->prev = _viv_playlist_last;
	}
	else
	{
		_viv_playlist_start = d;
		d->prev = 0;
	}
	
	_viv_playlist_last = d;
	d->next = 0;
	
	if (_viv_playlist_shuffle_indexes)
	{
		// make sure shuffle list has enough room...
		if (_viv_playlist_count + 1 > _viv_playlist_shuffle_allocated)
		{
			_viv_playlist_t **new_indexes;
			
			_viv_playlist_shuffle_allocated	*= 2;
			
			if (!_viv_playlist_shuffle_allocated)
			{
				_viv_playlist_shuffle_allocated = _VIV_DEFAULT_SHUFFLE_ALLOCATED;
			}
			
			new_indexes = mem_alloc(_viv_playlist_shuffle_allocated * sizeof(_viv_playlist_t *));
			
			os_copy_memory(new_indexes,_viv_playlist_shuffle_indexes,_viv_playlist_count * sizeof(_viv_playlist_t *));
			
			mem_free(_viv_playlist_shuffle_indexes);
			
			_viv_playlist_shuffle_indexes = new_indexes;
		}
		
		// place in shuffle list in a random position.
		{
			int index;
			
			index = ((rand() * RAND_MAX) + rand()) % (_viv_playlist_count + 1);

			if (_viv_playlist_count != index)
			{
				_viv_playlist_shuffle_indexes[_viv_playlist_count] = _viv_playlist_shuffle_indexes[index];
			}
			
			_viv_playlist_shuffle_indexes[index] = d;
		}
	}
	
	_viv_playlist_count++;
	
	return d;
}

static void _viv_playlist_add_path(const wchar_t *full_path_and_filename)
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
					string_cat_path_separator(buf);
					string_cat(buf,fd.cFileName);
					
					_viv_playlist_add_path(buf);
				}
			}
			else
			{
				if (_viv_is_valid_filename(&fd))
				{
					string_copy(buf,full_path_and_filename);
					string_cat_path_separator(buf);
					string_cat(buf,fd.cFileName);
					
					string_copy(fd.cFileName,buf);
					
					_viv_playlist_add(&fd);
				}
			}

			if (!FindNextFile(h,&fd)) break;
		}
		
		FindClose(h);
	}
}

static void _viv_playlist_add_filename(const wchar_t *filename)
{
	WIN32_FIND_DATA fd;
	wchar_t full_path_and_filename[STRING_SIZE];
	wchar_t cwd[STRING_SIZE];
	
	GetCurrentDirectory(STRING_SIZE,cwd);

	string_path_combine(full_path_and_filename,cwd,filename);
	
	if ((os_GetFileAttributesExW) && (os_GetFileAttributesExW(full_path_and_filename,GetFileExInfoStandard,&fd)))
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			_viv_playlist_add_path(full_path_and_filename);
		}
		else
		{
			string_copy(fd.cFileName,full_path_and_filename);

			if (_viv_is_valid_filename(&fd))
			{
				_viv_playlist_add(&fd);
			}
		}
	}
}

static void _viv_timer_stop(void)
{
	if (_viv_is_animation_timer)
	{
		_viv_is_animation_timer = 0;
		
		if ((os_CreateTimerQueueTimer) && (os_DeleteTimerQueueTimer))
		{
			if (_viv_is_timer_queue_timer)
			{
				os_DeleteTimerQueueTimer(NULL,_viv_timer_queue_timer_handle,NULL);
				
				_viv_is_timer_queue_timer = 0;
			}
		}
		else
		{
			KillTimer(_viv_hwnd,VIV_ID_ANIMATION_TIMER);
		}

		_viv_update_prevent_sleep();
	}
}

static int _viv_is_window_maximized(HWND hwnd)
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

static INT_PTR CALLBACK _viv_custom_rate_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			os_center_dialog(hwnd);

			SetDlgItemInt(hwnd,IDC_CUSTOM_RATE_EDIT,config_slideshow_custom_rate,FALSE);

			os_ComboBox_AddString(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO,(const utf8_t *)"milliseconds");
			os_ComboBox_AddString(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO,(const utf8_t *)"seconds");
			os_ComboBox_AddString(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO,(const utf8_t *)"minutes");
			
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO),config_slideshow_custom_rate_type);

			return TRUE;
		}
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
					config_slideshow_custom_rate_type = ComboBox_GetCurSel(GetDlgItem(hwnd,IDC_CUSTOM_RATE_TYPE_COMBO));
					config_slideshow_custom_rate = GetDlgItemInt(hwnd,IDC_CUSTOM_RATE_EDIT,NULL,FALSE);
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

static INT_PTR CALLBACK _viv_about_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
			os_SetDlgItemText(hwnd,IDC_ABOUTCOPYRIGHT,(const utf8_t *)"Copyright © " VIV_YEAR_STRING(VERSION_YEAR) " David Carpenter");
			os_SetDlgItemText(hwnd,IDC_ABOUTEMAIL,(const utf8_t *)"david.carpenter@voidtools.com");
			os_SetDlgItemText(hwnd,IDC_ABOUTWEBSITE,(const utf8_t *)"www.voidtools.com");
			
			if (!_viv_about_hfont)
			{
				hfont = (HFONT)SendMessage(GetDlgItem(hwnd,IDC_ABOUTTITLE),WM_GETFONT,0,0);
				GetObject(hfont,sizeof(LOGFONT),&lf);
				
				lf.lfHeight = 32;
				_viv_about_hfont = CreateFontIndirect(&lf);
			}
			
			SendMessage(GetDlgItem(hwnd,IDC_ABOUTTITLE),WM_SETFONT,(WPARAM)_viv_about_hfont,0);
			
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

static void _viv_update_frame(void)
{
	if (!_viv_is_fullscreen)
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
		if (_viv_is_window_maximized(_viv_hwnd))
		{
			ShowWindow(_viv_hwnd,SW_RESTORE);
			was_maximized = 1;
		}
		
		oldstyle = GetWindowLong(_viv_hwnd,GWL_STYLE);
		newstyle = oldstyle;
		
		GetClientRect(_viv_hwnd,&clientrect);
		debug_printf("clientrect %d %d %d %d\n",clientrect.left,clientrect.top,clientrect.right,clientrect.bottom);
		
		CopyRect(&oldrect,&clientrect);
		AdjustWindowRect(&oldrect,oldstyle,GetMenu(_viv_hwnd) ? TRUE : FALSE);
		
		oldrect.bottom += _viv_get_status_high() + _viv_get_controls_high();
		
		debug_printf("oldrect %d %d %d %d %d\n",oldrect.left,oldrect.top,oldrect.right,oldrect.bottom,GetMenu(_viv_hwnd) ? TRUE : FALSE);
	
		if (config_show_caption)	
		{
			newstyle |= WS_CAPTION | WS_SYSMENU;
		}
		else
		{
			newstyle &= ~(WS_CAPTION | WS_SYSMENU);
		}
		
		if (config_show_thickframe)	
		{
			newstyle |= WS_THICKFRAME;
		}
		else
		{
			newstyle &= ~WS_THICKFRAME;
		}

		if (config_show_menu)	
		{
			if (GetMenu(_viv_hwnd) != _viv_hmenu)
			{
				SetMenu(_viv_hwnd,_viv_hmenu);
			}
		}
		else
		{
			if (GetMenu(_viv_hwnd) != 0)
			{
				SetMenu(_viv_hwnd,0);
			}
		}
		
		_viv_status_show(config_show_status);
		_viv_controls_show(config_show_controls);

		CopyRect(&newrect,&clientrect);
		AdjustWindowRect(&newrect,newstyle,config_show_menu ? TRUE : FALSE);

		newrect.bottom += _viv_get_status_high() + _viv_get_controls_high();

		debug_printf("newrect %d %d %d %d %d\n",newrect.left,newrect.top,newrect.right,newrect.bottom,config_show_menu ? TRUE : FALSE);
		
		GetWindowRect(_viv_hwnd,&windowrect);
		
		windowrect.left += newrect.left - oldrect.left;
		windowrect.top += newrect.top - oldrect.top;
		windowrect.right += newrect.right - oldrect.right;
		windowrect.bottom += newrect.bottom - oldrect.bottom;
	
		SetWindowLong(_viv_hwnd,GWL_STYLE,newstyle);
		
		SetWindowPos(_viv_hwnd,HWND_TOP,windowrect.left,windowrect.top,windowrect.right - windowrect.left,windowrect.bottom - windowrect.top,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOCOPYBITS);

		// if there is no catpion or thick frame we should not allow maximize
		// avoid our resize borders when maximized.
		if ((was_maximized) && (config_show_caption) && (config_show_thickframe))
		{
			ShowWindow(_viv_hwnd,SW_MAXIMIZE);
		}
	}
}

static void _viv_update_ontop(void)
{
	int is_top_most;
	
	is_top_most = 0;
	
	switch(config_ontop)
	{
		case 1:
			is_top_most = 1;
			break;
			
		case 2:
			is_top_most = (_viv_is_slideshow) || ((_viv_frame_count > 1) && (_viv_animation_play));
			break;
	}

	SetWindowPos(_viv_hwnd,is_top_most ? HWND_TOPMOST : HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
}

static void _viv_update_prevent_sleep(void)
{
	BYTE _is_prevent_sleep;
	
	_is_prevent_sleep = 0;
	
	if (config_prevent_sleep)
	{
		if ((_viv_is_slideshow) || (_viv_is_animation_timer))
		{
			_is_prevent_sleep = 1;
		}
	}
	
	// _is_prevent_sleep changed?
	if (!!_viv_is_prevent_sleep != !!_is_prevent_sleep)
	{
		if (_os_SetThreadExecutionState)
		{	
			_os_SetThreadExecutionState(_is_prevent_sleep ? (ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED) : ES_CONTINUOUS);
		}
		
		_viv_is_prevent_sleep = _is_prevent_sleep;
	}
}

static void _viv_dst_pos_set(int x,int y)
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

	if ((x != _viv_dst_pos_x) || (y != _viv_dst_pos_y))
	{
		_viv_dst_pos_x = x;
		_viv_dst_pos_y = y;
		
		InvalidateRect(_viv_hwnd,NULL,FALSE);
	}

	_viv_status_update_temp_pos_zoom();	
}

static void _viv_dst_zoom_set(int x,int y)
{
	if (x < 0)
	{
		x = 0;
	}
	else
	if (x > _VIV_DST_ZOOM_MAX - 1)
	{
		x = _VIV_DST_ZOOM_MAX - 1;
	}
	
	if (y < 0)
	{
		y = 0;
	}
	else
	if (y > _VIV_DST_ZOOM_MAX - 1)
	{
		y = _VIV_DST_ZOOM_MAX - 1;
	}
	
	if ((x != _viv_dst_zoom_x_pos) || (y != _viv_dst_zoom_y_pos))
	{
		_viv_dst_zoom_x_pos = x;
		_viv_dst_zoom_y_pos = y;
		
		InvalidateRect(_viv_hwnd,NULL,FALSE);
	}

	_viv_status_update_temp_pos_zoom();	
	/*
	{
		RECT rect;
		int wide;
		int high;
		int rw;
		int rh;
		double new_view_x;
		double new_view_y;

		
		GetClientRect(_viv_hwnd,&rect);
		wide = rect.right - rect.left;
		high = rect.bottom - rect.top;	
		_viv_get_render_size(&rw,&rh);
	
		new_view_x = (int)(((_viv_view_ix * rw) / _viv_image_wide) + 0.5) + (((_viv_dst_pos_x - 250) * (wide*2)) / 1000) - (wide / 2) - (rw / 2);
		new_view_y = (int)(((_viv_view_iy * rh) / _viv_image_high) + 0.5) + (((_viv_dst_pos_y - 250) * (high*2)) / 1000) - (high / 2) - (rh / 2);

debug_printf("RESTORE VIEW x %d y %d ix %d iy %d rw %d rh %d wide %d high %d\n",(int)(new_view_x),(int)(new_view_y),(int)_viv_view_ix,(int)_viv_view_iy,rw,rh,wide,high)		;
//		_viv_view_set((int)(_viv_view_ix * (double)rw),(int)(_viv_view_iy * (double)rh),1);
		_viv_view_set((int)new_view_x,(int)new_view_y,1);
	}
	*/
}

static void _viv_frame_skip(int size)
{
	if (_viv_frame_count > 1)			
	{
		if (size > 0)
		{
			while(size > 0)
			{
				if ((_viv_frame_loaded_count == _viv_frame_count) || (_viv_frame_position + 1 < _viv_frame_loaded_count))
				{
					_viv_frame_position++;
					if (_viv_frame_position == _viv_frame_count)
					{
						_viv_frame_position = 0;
					}

					_viv_animation_timer_tick_start = os_get_tick_count();
					_viv_timer_tick = 0;
				}

				size -= _viv_frames[_viv_frame_position].delay;
			}
		}
		else
		if (size < 0)
		{
			while(size < 0)
			{
				_viv_frame_position--;
				if (_viv_frame_position < 0)
				{
					_viv_frame_position = _viv_frame_loaded_count - 1;
				}
				
				_viv_animation_timer_tick_start = os_get_tick_count();
				_viv_timer_tick = 0;
				
				size += _viv_frames[_viv_frame_position].delay;
			}
		}
			
		_viv_update_src_pixel(1,0);
		_viv_status_update();
		
		InvalidateRect(_viv_hwnd,NULL,FALSE);
		UpdateWindow(_viv_hwnd);
	}
}

static int _viv_webp_info_proc(_viv_webp_t *viv_webp,DWORD frame_count,DWORD wide,DWORD high,int has_alpha)
{
	int ret;
	
	ret = 0;
	
	viv_webp->wide = wide;
	viv_webp->high = high;
	viv_webp->has_alpha = has_alpha;
	viv_webp->frame_count = frame_count;

	viv_webp->screen_hdc = GetDC(NULL);
	if (viv_webp->screen_hdc)
	{
		viv_webp->mem_hdc = CreateCompatibleDC(viv_webp->screen_hdc);
		
		if (viv_webp->mem_hdc)
		{
			ret = 1;
		}
	}
	
	return ret;
}

static int _viv_webp_frame_proc(_viv_webp_t *viv_webp,BYTE *pixels,int delay)
{
	// always load first frame..
	if ((viv_webp->frame_index) && (_viv_load_image_terminate))
	{
		return 0;
	}
	
	if (viv_webp->has_alpha)
	{
		BYTE *p;
		DWORD run;
		
		run = viv_webp->wide * viv_webp->high;
		p = pixels;
		
		while(run)
		{
			int a;

			a = p[3];
			
			if (a != 255)
			{
				while(run)
				{
					int r;
					int g;
					int b;
					int a;

					r = p[0];
					g = p[1];
					b = p[2];
					a = p[3];
					
					b = config_windowed_background_color_b + ((b - config_windowed_background_color_b) * a) / 255;
					g = config_windowed_background_color_g + ((g - config_windowed_background_color_g) * a) / 255;
					r = config_windowed_background_color_r + ((r - config_windowed_background_color_r) * a) / 255;
					
					p[0] = r;
					p[1] = g;
					p[2] = b;
					p[3] = 255;
					
					p += 4;
					run--;
				}
				
				break;
			}
			
			p += 4;
			run--;
		}
	}
	
	/*
	{
		int x;
		int y;
		
		for(y=0;y<height;y++)
		{
			for(x=0;x<width;x++)
			{
				SetPixel(screen_hdc,x,y,RGB(pixels[(x + (y * width)) * 4],pixels[(x + (y * width)) * 4],pixels[(x + (y * width)) * 4]));
			}
		}
	}
	*/

	// RGBA => BGR
	
	{
		const BYTE *p;
		BYTE *d;
		DWORD high_run;
		
		high_run = viv_webp->high;
		p = pixels;
		d = pixels;

		while(high_run)
		{
			BYTE *wd;
			DWORD wide_run;
			
			wide_run = viv_webp->wide;
			wd = d;
		
			while(wide_run)
			{
				int a;
				int r;
				int g;
				int b;
				
				r = p[0];
				g = p[1];
				b = p[2];
				a = p[3];
				
				p += 4;
				
				wd[0] = b;
				wd[1] = g;
				wd[2] = r;
				wd += 3;
				
				wide_run--;
			}
			
			d += (((viv_webp->wide * 3) + 3) / 4) * 4;
			
			high_run--;
		}
	}
	

	{
		BITMAPINFO bmi;
		HBITMAP hbitmap;

		// Create compatible bitmap
		ZeroMemory(&bmi, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = viv_webp->wide;
		bmi.bmiHeader.biHeight = -(int)viv_webp->high; // Negative height to indicate top-down bitmap
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 24;
		bmi.bmiHeader.biCompression = BI_RGB;
		
		hbitmap = CreateCompatibleBitmap(viv_webp->screen_hdc,viv_webp->wide,viv_webp->high);

		// Set RGB data to the bitmap
		if (hbitmap) 
		{
			int mip_wide;
			int mip_high;

			SetDIBits(viv_webp->mem_hdc,hbitmap,0,viv_webp->high,pixels,&bmi,DIB_RGB_COLORS);

			if (viv_webp->orientation > 1)
			{
				HBITMAP new_hbitmap;
				
				new_hbitmap = _viv_orientate_hbitmap(hbitmap,viv_webp->orientation);
				if (new_hbitmap)
				{
					DeleteObject(hbitmap);
					hbitmap = new_hbitmap;
				}
			}
			
			if (viv_webp->frame_index == 0)
			{
				_viv_reply_load_image_first_frame_t first_frame;

				first_frame.wide = viv_webp->wide;
				first_frame.high = viv_webp->high;
				first_frame.frame.hbitmap = hbitmap;
				first_frame.frame.mipmap = NULL;
				first_frame.frame.delay = 0;
				first_frame.frame_count = viv_webp->frame_count;
				
				if (viv_webp->frame_count > 1)
				{
					first_frame.frame.delay = delay;
				}

				// preload first mipmap
				_viv_get_mipmap(hbitmap,first_frame.wide,first_frame.high,_viv_load_render_wide/2,_viv_load_render_high/2,&mip_wide,&mip_high,&first_frame.frame.mipmap);

				_viv_reply_add(_VIV_REPLY_LOAD_IMAGE_FIRST_FRAME,sizeof(_viv_reply_load_image_first_frame_t),&first_frame);
			}
			else
			{
				_viv_frame_t frame;
				
				frame.hbitmap = hbitmap;
				frame.mipmap = NULL;
				frame.delay = delay;
				
//printf("DELAY %d\n",delay)				;

				_viv_get_mipmap(hbitmap,viv_webp->wide,viv_webp->high,_viv_load_render_wide/2,_viv_load_render_high/2,&mip_wide,&mip_high,&frame.mipmap);
				
				_viv_reply_add(_VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME,sizeof(_viv_frame_t),&frame);
			}
			
			viv_webp->frame_index++;
		}
	}
	
	return 1;
}

// 14.447 - CreateStreamOnHGlobal - this is too slow over slow networks -which doesn't matter because the gif wont show the first frame until the entire gif is loaded anyway.
// 14.520 - CreateStreamOnHGlobal
// 15.834 - SHCreateStreamOnFile
// 15.850 - SHCreateStreamOnFile
// 14.914 - _viv stream - 1MB file buffer
// 14.914 - _viv stream
static DWORD WINAPI _viv_load_image_thread_proc(void *param)
{
	_viv_reply_load_image_first_frame_t first_frame;
	IStream *stream;
	int ret;
	DWORD tickstart;

	tickstart = GetTickCount();
	
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);

	first_frame.wide = 0;
	first_frame.high = 0;
	first_frame.frame_count = 0;
	first_frame.frame.hbitmap = 0;
	first_frame.frame.mipmap = 0;
	first_frame.frame.delay = 0;

	ret = 0;
	stream = NULL;
	
	debug_printf("%s %S...\n",_viv_load_is_preload ? "PRELOAD" : "LOAD",_viv_load_image_filename);
	
	{
		HANDLE h;
		
		h = CreateFile(_viv_load_image_filename,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|((os_major_version >= 5) ? FILE_SHARE_DELETE : 0),0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
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

//						if (_viv_load_image_terminate)
//						{
//							totreadsize = 0;
//							break;
//						}

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
							debug_printf("got stream in %f seconds\n",(double)(GetTickCount()-tickstart) * 0.001);
							
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


/*
	// webp uses hglobals
	{
		IStream *my_stream;
		
		if (SUCCEEDED(SHCreateStreamOnFileEx(_viv_load_image_filename,STGM_READ,FILE_ATTRIBUTE_NORMAL,FALSE,NULL,&my_stream)))
		{
			// stream owns my_stream now.
			stream = my_stream;
		}
	}
*/

	if (stream)
	{
		int orientation;
		
		debug_printf("stream %d\n",stream);
		
		if (config_orientation)
		{
			orientation = os_get_orientation(_viv_load_image_filename);

			debug_printf("orientation %d\n",orientation);
		}
		else
		{
			orientation = 0;
		}

//		if (!_viv_load_image_terminate)
		{
			void *image;
			int load_ret;
			
			// not implemented.
			load_ret = 1; 
			
			if ((os_GdipLoadImageFromStream) && (os_GdipLoadImageFromStreamICM) && (os_GdipGetImageWidth) && (os_GdipGetImageHeight) && (os_GdipImageGetFrameDimensionsCount) && (os_GdipImageGetFrameDimensionsList) && (os_GdipImageGetFrameCount) && (os_GdipGetPropertyItemSize) && (os_GdipGetPropertyItem) && (os_GdipImageSelectActiveFrame) && (os_GdipGetImageFlags) && (os_GdipDisposeImage) && (os_GdipCreateFromHDC) && (os_GdipSetCompositingMode) && (os_GdipSetCompositingQuality) && (os_GdipSetInterpolationMode) && (os_GdipSetPixelOffsetMode) && (os_GdipSetSmoothingMode) && (os_GdipDrawImageRectI) && (os_GdipDeleteGraphics))
			{
				if (config_icm)
				{
					load_ret = os_GdipLoadImageFromStreamICM(stream,&image);
				}
				else
				{
					load_ret = os_GdipLoadImageFromStream(stream,&image);
				}
			}
			
			debug_printf("image %p\n",image);

			if (load_ret == 0)
			{
//				if (!_viv_load_image_terminate)
				{
					if (os_GdipGetImageWidth(image,&first_frame.wide) == 0)
					{
						if (os_GdipGetImageHeight(image,&first_frame.high) == 0)
						{
							UINT count;
							int load_wide;
							int load_high;
							
							load_wide = first_frame.wide;
							load_high = first_frame.high;
							
							// apply orientation.
							switch (orientation)
							{
								case 2: // #define PHOTO_ORIENTATION_FLIPHORIZONTAL    2u
									break;
									
								case 5: // #define PHOTO_ORIENTATION_TRANSPOSE         5u
								case 6: // #define PHOTO_ORIENTATION_ROTATE270         6u
								case 7: // #define PHOTO_ORIENTATION_TRANSVERSE        7u
								case 8: // #define PHOTO_ORIENTATION_ROTATE90          8u
									{
										int temp;
										temp = first_frame.wide;
										first_frame.wide = first_frame.high;
										first_frame.high = temp;
									}
									break;
							}

							//First of all we should get the number of frame dimensions
							//Images considered by GDI+ as:
							//frames[animation_frame_index][how_many_animation];
							if (os_GdipImageGetFrameDimensionsCount(image,&count) == 0)
							{
								os_PropertyItem_t *frame_delay;
								
								frame_delay = NULL;
								
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
									UINT size;

									// PropertyTagFrameDelay 0x5100
									os_GdipGetPropertyItemSize(image,0x5100,&size);

									debug_printf("frame delay size %d\n",size);

									frame_delay = (os_PropertyItem_t *)mem_alloc(size);
									
									// PropertyTagFrameDelay 0x5100
									os_GdipGetPropertyItem(image,0x5100,size,frame_delay);
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
												
												if ((i) && (_viv_load_image_terminate))
												{
													break;
												}
												
												hbitmap = CreateCompatibleBitmap(screen_hdc,load_wide,load_high);
												if (hbitmap)
												{
													UINT image_flags;
													
													last_hbitmap = SelectObject(mem_hdc,hbitmap);
													
													os_GdipImageSelectActiveFrame(image,&_viv_FrameDimensionTime,i);

													os_GdipGetImageFlags(image,&image_flags);
													
													{
														void *g;
														int draw_ret;

														if (os_GdipCreateFromHDC(mem_hdc,&g) == 0)
														{
															// ImageFlagsHasAlpha = 0x0002,
															if (image_flags & 2)
															{
																{
																	RECT rect;
																	HBRUSH hbrush;
																	
																	hbrush = CreateSolidBrush(RGB(config_windowed_background_color_r,config_windowed_background_color_g,config_windowed_background_color_b));
																	
																	rect.left = 0;
																	rect.top = 0;
																	rect.right = load_wide;
																	rect.bottom = load_high;

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

															draw_ret = os_GdipDrawImageRectI(g,image,0,0,load_wide,load_high);
															if (draw_ret != 0)
															{
																debug_printf("DrawImage failed %d\n",draw_ret);
															}
															
															os_GdipDeleteGraphics(g);
														}
													}

													if (orientation > 1)
													{
														HBITMAP new_hbitmap;
														
														new_hbitmap = _viv_orientate_hbitmap(hbitmap,orientation);
														if (new_hbitmap)
														{
															DeleteObject(hbitmap);
															hbitmap = new_hbitmap;
														}
													}
													
													SelectObject(mem_hdc,last_hbitmap);
												
													if (i)
													{
														_viv_frame_t frame;
														int mip_wide;
														int mip_high;

														frame.hbitmap = hbitmap;
														frame.mipmap = NULL;
														frame.delay = (((UINT *)frame_delay[0].value)[i]) * 10;
															
														if (!frame.delay)
														{
															frame.delay = 100;
														}
														
														_viv_get_mipmap(hbitmap,first_frame.wide,first_frame.high,_viv_load_render_wide/2,_viv_load_render_high/2,&mip_wide,&mip_high,&frame.mipmap);

														_viv_reply_add(_VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME,sizeof(_viv_frame_t),&frame);
													}
													else
													{
														int mip_wide;
														int mip_high;

														first_frame.frame.hbitmap = hbitmap;
														first_frame.frame.mipmap = NULL;
														first_frame.frame.delay = 0;
														
														if (first_frame.frame_count > 1)
														{
															first_frame.frame.delay = (((UINT *)frame_delay[0].value)[i]) * 10;
															
															if (!first_frame.frame.delay)
															{
																first_frame.frame.delay = 100;
															}
														}
															
														// preload first mipmap
														_viv_get_mipmap(hbitmap,first_frame.wide,first_frame.high,_viv_load_render_wide/2,_viv_load_render_high/2,&mip_wide,&mip_high,&first_frame.frame.mipmap);
							
														_viv_reply_add(_VIV_REPLY_LOAD_IMAGE_FIRST_FRAME,sizeof(_viv_reply_load_image_first_frame_t),&first_frame);

														ret = 1;
													}
												}
											}
											
											DeleteDC(mem_hdc);
										}
										
										ReleaseDC(0,screen_hdc);
									}
								}

								if (frame_delay)
								{
									mem_free(frame_delay);
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
				_viv_webp_t viv_webp;
				
				debug_printf("GDIPlus failed to load image %S %d\n",_viv_load_image_filename,load_ret);
				
				viv_webp.screen_hdc = NULL;
				viv_webp.mem_hdc = NULL;
				viv_webp.frame_index = 0;
				viv_webp.last_delay = 0;
				viv_webp.orientation = orientation;
				
				if (webp_load(stream,&viv_webp,_viv_webp_info_proc,_viv_webp_frame_proc))
				{
					ret = 1;
				}
				else
				{
					debug_printf("libwebp failed to load image %S\n",_viv_load_image_filename);

					// we loaded at least one frame, so treat it as complete.
					if (viv_webp.frame_index > 0)
					{
						ret = 1;
					}
				}

				if (viv_webp.mem_hdc)
				{
					DeleteDC(viv_webp.mem_hdc);
				}

				if (viv_webp.screen_hdc)
				{
					ReleaseDC(NULL,viv_webp.screen_hdc);
				}
			}
		}

		stream->lpVtbl->Release(stream);				
	}
	else
	{
		debug_printf("Failed to create stream from %S\n",_viv_load_image_filename);
	}

	_viv_reply_add(ret ? _VIV_REPLY_LOAD_IMAGE_COMPLETE : _VIV_REPLY_LOAD_IMAGE_FAILED,0,0);
	
	CoUninitialize();
	
	debug_printf("loaded in %f seconds\n",(double)(GetTickCount()-tickstart) * 0.001);

    return 0;
}

static void _viv_reply_free(_viv_reply_t *e)
{
	switch (e->type)
	{
		case _VIV_REPLY_LOAD_IMAGE_FIRST_FRAME:
			
			if (((_viv_reply_load_image_first_frame_t *)(e+1))->frame.hbitmap)
			{
				DeleteObject(((_viv_reply_load_image_first_frame_t *)(e+1))->frame.hbitmap);
			}
		
			if (((_viv_reply_load_image_first_frame_t *)(e+1))->frame.mipmap)
			{
				_viv_mipmap_free(((_viv_reply_load_image_first_frame_t *)(e+1))->frame.mipmap);
			}

			break;

		case _VIV_REPLY_LOAD_IMAGE_ADDITIONAL_FRAME:
		
			if (((_viv_frame_t *)(e + 1))->hbitmap)
			{	
				DeleteObject(((_viv_frame_t *)(e + 1))->hbitmap);
			}

			if (((_viv_frame_t *)(e + 1))->mipmap)
			{	
				_viv_mipmap_free(((_viv_frame_t *)(e + 1))->mipmap);
			}
		
			break;
	}
	
	mem_free(e);
}

static _viv_reply_t *_viv_reply_add(DWORD type,DWORD size,void *data)
{
	_viv_reply_t *e;
	int is_first;
	
	is_first = 0;
	
	e = (_viv_reply_t *)mem_alloc(sizeof(_viv_reply_t) + size);
	
	e->type = type;
	e->size = size;
	
	os_copy_memory(e + 1,data,size);
	
	EnterCriticalSection(&_viv_cs);
	
	if (_viv_reply_start)
	{
		_viv_reply_last->next = e;
	}
	else
	{
		_viv_reply_start = e;
		is_first = 1;
	}
	
	_viv_reply_last = e;
	e->next = 0;
	
	LeaveCriticalSection(&_viv_cs);
	
	if (is_first)
	{
		PostMessage(_viv_hwnd,_VIV_WM_REPLY,0,0);
	}
	
	return e;
}

static void _viv_reply_clear_all(void)
{
	_viv_reply_t *e;
	
	EnterCriticalSection(&_viv_cs);
	
	e = _viv_reply_start;
	_viv_reply_start = 0;
	_viv_reply_last = 0;
	
	LeaveCriticalSection(&_viv_cs);
	
	while(e)
	{
		_viv_reply_t *next_e;
		
		next_e = e->next;
		
		_viv_reply_free(e);
		
		e = next_e;
	}
}

static void _viv_status_show(int show)
{
	if (show)
	{
		if (!_viv_status_hwnd)
		{
			// WS_EX_COMPOSITED stops flickering issues
			_viv_status_hwnd = os_CreateWindowEx(
				WS_EX_COMPOSITED,
				STATUSCLASSNAMEA,
				"",
				WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | SBARS_SIZEGRIP | WS_VISIBLE,
				0,0,0,0,
				_viv_hwnd,(HMENU)VIV_ID_STATUS,os_hinstance,NULL);
				
			_viv_old_status_proc = os_set_window_proc(_viv_status_hwnd,_viv_status_proc);
		}
	}
	else
	{
		if (_viv_status_hwnd)
		{
			DestroyWindow(_viv_status_hwnd);

			_viv_status_hwnd = 0;
		}	
	}
	
	_viv_on_size();
}

static void _viv_controls_show(int show)
{
	if (show)
	{
		if (!_viv_toolbar_hwnd)
		{
			os_RegisterClassEx(
				CS_DBLCLKS,
				_viv_rebar_proc,
				0,
				LoadCursor(NULL,IDC_ARROW),
				(HBRUSH)(COLOR_WINDOW+1),
				"_VIV_REBAR",
				0);

			_viv_rebar_hwnd = os_CreateWindowEx(
				0,
				"_VIV_REBAR",
				"",
				WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE,
				0,0,0,0,
				_viv_hwnd,(HMENU)VIV_ID_TOOLBAR,os_hinstance,NULL);

			_viv_toolbar_hwnd = os_CreateWindowEx(
				0,
				TOOLBARCLASSNAMEA,
				"",
				WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NODIVIDER | CCS_NORESIZE | CCS_TOP,
				0,0,0,0,
				_viv_rebar_hwnd,(HMENU)VIV_ID_TOOLBAR,os_hinstance,NULL);
			
			SendMessage(_viv_toolbar_hwnd,TB_SETEXTENDEDSTYLE,0,TBSTYLE_EX_MIXEDBUTTONS|TBSTYLE_EX_HIDECLIPPEDBUTTONS|TBSTYLE_EX_DOUBLEBUFFER);
			SendMessage(_viv_toolbar_hwnd,TB_BUTTONSTRUCTSIZE,sizeof(TBBUTTON),0);
			
			_viv_toolbar_image_list = ImageList_Create((16 * os_logical_wide) / 96,(16 * os_logical_high) / 96,ILC_COLOR24|ILC_MASK,0,0);
			
			ImageList_AddIcon(_viv_toolbar_image_list,LoadIcon(os_hinstance,(LPCTSTR)IDI_PREV));
			ImageList_AddIcon(_viv_toolbar_image_list,LoadIcon(os_hinstance,(LPCTSTR)IDI_PLAY));
			ImageList_AddIcon(_viv_toolbar_image_list,LoadIcon(os_hinstance,(LPCTSTR)IDI_PAUSE));
			ImageList_AddIcon(_viv_toolbar_image_list,LoadIcon(os_hinstance,(LPCTSTR)IDI_NEXT));
			ImageList_AddIcon(_viv_toolbar_image_list,LoadIcon(os_hinstance,(LPCTSTR)IDI_BESTFIT));
			ImageList_AddIcon(_viv_toolbar_image_list,LoadIcon(os_hinstance,(LPCTSTR)IDI_1TO1));

			SendMessage(_viv_toolbar_hwnd,TB_SETIMAGELIST,0,(LPARAM)_viv_toolbar_image_list);

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
					
				SendMessage(_viv_toolbar_hwnd,TB_ADDBUTTONS,8,(LPARAM)buttons);
			}

			_viv_toolbar_update_buttons();
			ShowWindow(_viv_toolbar_hwnd,SW_SHOW);
					
//			Toolba				
//			TB_ADDBUTTONS();
		}
	}
	else
	{
		if (_viv_toolbar_hwnd)
		{
			DestroyWindow(_viv_toolbar_hwnd);
			DestroyWindow(_viv_rebar_hwnd);

			_viv_toolbar_hwnd = 0;
			_viv_rebar_hwnd = 0;
		}	
		
		if (_viv_toolbar_image_list)
		{
			ImageList_Destroy(_viv_toolbar_image_list);
			
			_viv_toolbar_image_list = 0;
		}
	}
	
	_viv_on_size();
}

static void _viv_status_update(void)
{
	if (_viv_status_hwnd)
	{
		int part_array[6];
		RECT rect;
		wchar_t widebuf[STRING_SIZE];
		wchar_t highbuf[STRING_SIZE];
		wchar_t dimension_buf[STRING_SIZE];
		wchar_t frame_buf[STRING_SIZE];
		wchar_t pixel_pos_buf[STRING_SIZE];
		wchar_t pixel_rgb_buf[STRING_SIZE];
		const wchar_t *preload_buf;
		HDC hdc;
		int dimension_wide;
		int frame_wide;
		int preload_wide;
		int pixel_pos_wide;
		int pixel_rgb_wide;
		int minwide;
		
		GetClientRect(_viv_hwnd,&rect);
		
		preload_buf = 0;
		*pixel_pos_buf = 0;
		*pixel_rgb_buf = 0;
		
		if ((_viv_image_wide) && (_viv_image_high))
		{
			string_format_number(widebuf,_viv_image_wide);
			string_format_number(highbuf,_viv_image_high);
		
			string_copy(dimension_buf,widebuf);
			string_cat_utf8(dimension_buf,(const utf8_t *)" x ");
			string_cat(dimension_buf,highbuf);
		}
		else
		{
			string_copy_utf8(dimension_buf,(const utf8_t *)"");
		}

		if (*_viv_frame_fd->cFileName)
		{
			LARGE_INTEGER size;
			
			size.HighPart = _viv_frame_fd->nFileSizeHigh;
			size.LowPart = _viv_frame_fd->nFileSizeLow;
			
			if (size.QuadPart)
			{
				NUMBERFMT numberfmt;
				
				if (*dimension_buf)
				{
					string_cat_utf8(dimension_buf," ");
				}
				
				string_cat_utf8(dimension_buf,"(");
				string_format_number(widebuf,(size.QuadPart + 1023) / 1024);
				
				numberfmt.NumDigits = 0;
				numberfmt.LeadingZero = 0;
				numberfmt.Grouping = 3;
				numberfmt.lpThousandSep = L",";
				numberfmt.lpDecimalSep = L".";
				numberfmt.NegativeOrder = 0;
	    
    			GetNumberFormat(LOCALE_USER_DEFAULT,0,widebuf,&numberfmt,highbuf,STRING_SIZE);
				
				string_cat(dimension_buf,highbuf);
				string_cat_utf8(dimension_buf," KB)");
			}
		}
		
		if (_viv_frame_count > 1)
		{
			int frame_pos;
			string_format_number(highbuf,_viv_frame_count);

			string_copy_utf8(frame_buf,(const utf8_t *)"");
			
			if (config_frame_minus)
			{
				frame_pos = _viv_frame_count - (_viv_frame_position);
				string_cat_utf8(frame_buf,(const utf8_t *)"- ");
			}
			else
			{
				frame_pos = _viv_frame_position + 1;
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
		
		// this is just noise..
		
		if ((_viv_load_is_preload) && (_viv_preload_state == 0) && (!_viv_should_activate_preload_on_load) && (!_viv_load_image_terminate) && (!_viv_preload_frame_loaded_count))
		{
			preload_buf = L"PRELOAD";
		}

		if (config_pixel_info)
		{
			if ((_viv_src_pixel_x >= 0) && (_viv_src_pixel_y >= 0))
			{		
				string_printf(pixel_pos_buf,"POS: %d,%d",_viv_src_pixel_x,_viv_src_pixel_y);
				string_printf(pixel_rgb_buf,"RGB: %d,%d,%d",_viv_src_pixel_r,_viv_src_pixel_g,_viv_src_pixel_b);
			}
		}
		
		dimension_wide = 0;
		frame_wide = 0;
		preload_wide = 0;
		pixel_pos_wide = 0;
		pixel_rgb_wide = 0;
		minwide = (72 * os_logical_wide) / 96;

		hdc = GetDC(_viv_status_hwnd);
		if (hdc)
		{
			HFONT hfont;
			
			hfont = (HFONT)SendMessage(_viv_status_hwnd,WM_GETFONT,0,0);
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

				if (preload_buf)
				{
					if (GetTextExtentPoint32(hdc,preload_buf,string_length(preload_buf),&size))
					{
						preload_wide = size.cx + GetSystemMetrics(SM_CXEDGE) * 5;
					}
				}

				if (*pixel_pos_buf)
				{
					if (GetTextExtentPoint32(hdc,pixel_pos_buf,string_length(pixel_pos_buf),&size))
					{
						pixel_pos_wide = size.cx + GetSystemMetrics(SM_CXEDGE) * 5;
					}
				}

				if (*pixel_rgb_buf)
				{
					if (GetTextExtentPoint32(hdc,pixel_rgb_buf,string_length(pixel_rgb_buf),&size))
					{
						pixel_rgb_wide = size.cx + GetSystemMetrics(SM_CXEDGE) * 5;
					}
				}

				SelectObject(hdc,last_font);
			}
			
			ReleaseDC(_viv_status_hwnd,hdc);
		}
		
		// add size box
		dimension_wide += GetSystemMetrics(SM_CXVSCROLL) + GetSystemMetrics(SM_CXBORDER);
		
		{
			int parti;
			int part_wide;
			
			parti = 0;
			part_wide = (rect.right - rect.left) - dimension_wide - frame_wide - preload_wide - pixel_pos_wide - pixel_rgb_wide;

			part_array[parti] = part_wide;
			if (part_array[parti] < 0)
			{
				part_array[parti] = 0;
			}
			parti++;
		
			if (preload_buf)
			{
				part_wide += preload_wide;
				part_array[parti] = part_wide;
				parti++;
			}
			
			if (pixel_pos_buf)
			{
				part_wide += pixel_pos_wide;
				part_array[parti] = part_wide;
				parti++;
			}
			
			if (pixel_rgb_buf)
			{
				part_wide += pixel_rgb_wide;
				part_array[parti] = part_wide;
				parti++;
			}
			
			part_wide += frame_wide;
			part_array[parti] = part_wide;
			parti++;
			
			part_array[parti] = -1;
			parti++;
			
			SendMessage(_viv_status_hwnd,SB_SETPARTS,parti,(LPARAM)part_array);
		}

		{
			wchar_t *text;
			
			text =  L"";
			
			if (_viv_status_temp_text)
			{
				text = _viv_status_temp_text;
			}
			else
			if ((_viv_load_image_thread) && ((!_viv_load_is_preload) || (_viv_should_activate_preload_on_load)))
			{	
				text =  L"Loading...";
			}
			else
			if (_viv_file_not_found)
			{
				text = L"File not found.";
			}
			else
			if (_viv_load_failed)
			{
				text = L"Failed to load image.";
			}
			else
			if (_viv_is_slideshow)
			{
				text =  L"Slideshow playing";
			}
		
			_viv_status_set(0,text);
		}
		
		{
			int parti;
			
			parti = 1;
			
			if (preload_buf)
			{
				_viv_status_set(parti,preload_buf);
				parti++;
			}
			
			if (pixel_pos_buf)
			{
				_viv_status_set(parti,pixel_pos_buf);
				parti++;
			}
			
			if (pixel_rgb_buf)
			{
				_viv_status_set(parti,pixel_rgb_buf);
				parti++;
			}
			
			_viv_status_set(parti,frame_buf);
			parti++;
			
			_viv_status_set(parti,dimension_buf);
			parti++;
		}
	}
}

static void _viv_status_set(int part,const wchar_t *text)
{
	wchar_t oldtext[STRING_SIZE];
	
	SendMessage(_viv_status_hwnd,SB_GETTEXTW,(WPARAM)part,(LPARAM)oldtext);
	
	if (string_compare(oldtext,text) != 0)
	{	
		SendMessage(_viv_status_hwnd,SB_SETTEXTW,(WPARAM)part,(LPARAM)text);
	}
}

static int _viv_get_status_high(void)
{
	if (_viv_status_hwnd)
	{
		RECT rect;
		
		GetWindowRect(_viv_status_hwnd,&rect);
		
		return rect.bottom - rect.top;
	}
	
	return 0;
}

static int _viv_get_controls_high(void)
{
	if (_viv_toolbar_hwnd)
	{
		return (32 * os_logical_high) / 96;
	}
	
	return 0;
}

static LRESULT CALLBACK _viv_rebar_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) 
	{	
		case WM_LBUTTONDOWN:
			
			if (config_toolbar_move_window)
			{
				_viv_start_move_window();
				
				return 0;
			}
			
			break;
			
		case WM_NOTIFY:

			switch(((NMHDR *)lParam)->idFrom)
			{
				case VIV_ID_TOOLBAR:

					if (_viv_toolbar_hwnd)
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
										GetClientRect(_viv_toolbar_hwnd,&rect);
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
			return SendMessage(_viv_hwnd,WM_COMMAND,wParam,lParam);
			
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

static LRESULT CALLBACK _viv_status_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) 
	{	
		case WM_LBUTTONDOWN:
		
			if (config_toolbar_move_window)
			{
				if (os_statusbar_index_from_x(hwnd,GET_X_LPARAM(lParam)) == 0)
				{
					_viv_start_move_window();

					return 0;
				}
			}
			
			break;
	}
	
	return CallWindowProc(_viv_old_status_proc,hwnd,msg,wParam,lParam);
}

static int _viv_toolbar_get_wide(void)
{
	if (_viv_toolbar_hwnd)
	{
		DWORD count;
		DWORD button_index;
		int min_x;
		int max_x;
		int got_x;
		
		count = (DWORD)SendMessage(_viv_toolbar_hwnd,TB_BUTTONCOUNT,0,0);
		got_x = 0;
		
		for(button_index=0;button_index<count;button_index++)
		{
		    RECT button_rect;
		    
		    if (SendMessage(_viv_toolbar_hwnd,TB_GETITEMRECT,button_index,(LPARAM)&button_rect))
		    {
				if (got_x)
				{
					if (button_rect.left < min_x)
					{
						min_x = button_rect.left;
					}

					if (button_rect.right > max_x)
					{
						max_x = button_rect.right;
					}
				}
				else
				{
					min_x = button_rect.left;
					max_x = button_rect.right;
					
					got_x = 1;
				}
		    }
		}
		
		if (got_x)
		{
			return max_x - min_x;
		}

/*		
{
	TBMETRICS tbmetrics;
	DWORD button_size;
	
	os_zero_memory(&tbmetrics,sizeof(TBMETRICS));
	
	tbmetrics.cbSize = sizeof(TBMETRICS);
	tbmetrics.dwMask = TBMF_PAD | TBMF_BARPAD | TBMF_BUTTONSPACING;
	
	SendMessage(_viv_toolbar_hwnd,TB_GETMETRICS,0,(LPARAM)&tbmetrics);

	debug_printf("TB_GETMETRICS cbSize %u %u %p\n",tbmetrics.cbSize,sizeof(TB_GETMETRICS),&tbmetrics);
	debug_printf("TB_GETMETRICS dwMask %u\n",tbmetrics.dwMask);
	debug_printf("TB_GETMETRICS cxPad %d\n",tbmetrics.cxPad);
	debug_printf("TB_GETMETRICS cyPad %d\n",tbmetrics.cyPad);
	debug_printf("TB_GETMETRICS cxBarPad %d\n",tbmetrics.cxBarPad);
	debug_printf("TB_GETMETRICS cyBarPad %d\n",tbmetrics.cyBarPad);
	debug_printf("TB_GETMETRICS cxButtonSpacing %d\n",tbmetrics.cxButtonSpacing);
	debug_printf("TB_GETMETRICS cyButtonSpacing %d\n",tbmetrics.cyButtonSpacing);
	
	button_size = SendMessage(_viv_toolbar_hwnd,TB_GETBUTTONSIZE,0,0);
	debug_printf("TB_GETBUTTONSIZE wide %u\n",LOWORD(button_size));
	debug_printf("TB_GETBUTTONSIZE high %u\n",HIWORD(button_size));
	
	if (SendMessage(_viv_toolbar_hwnd,TB_GETMAXSIZE,0,(LPARAM)&size))
	{
		debug_printf("TB_GETMAXSIZE wide %u\n",size.cx);
		debug_printf("TB_GETMAXSIZE high %u\n",size.cy);
	}	
}*/

/*
		SIZE size;
		
		// doesn't work at all on win9x / older than common controls v6 
		if (SendMessage(_viv_toolbar_hwnd,TB_GETMAXSIZE,0,(LPARAM)&size))
		{
			return size.cx;
		}*/
	}
	
	return 0;
}

static void _viv_toolbar_update_buttons(void)
{
	if (_viv_toolbar_hwnd)
	{
		TBBUTTONINFO tbbinfo;
		int rw;
		int rh;
		
		tbbinfo.cbSize = sizeof(TBBUTTONINFO);
		tbbinfo.dwMask = TBIF_STATE;
		tbbinfo.fsState = _viv_is_slideshow ? (TBSTATE_ENABLED | TBSTATE_CHECKED) : (TBSTATE_ENABLED);
		
		SendMessage(_viv_toolbar_hwnd,TB_SETBUTTONINFO,VIV_ID_SLIDESHOW_PLAY_ONLY,(LPARAM)&tbbinfo);
		
		tbbinfo.cbSize = sizeof(TBBUTTONINFO);
		tbbinfo.dwMask = TBIF_STATE;
		tbbinfo.fsState = _viv_is_slideshow ? (TBSTATE_ENABLED) : (TBSTATE_ENABLED | TBSTATE_CHECKED);
		
		SendMessage(_viv_toolbar_hwnd,TB_SETBUTTONINFO,VIV_ID_SLIDESHOW_PAUSE_ONLY,(LPARAM)&tbbinfo);
		
		_viv_get_render_size(&rw,&rh);
		
		tbbinfo.cbSize = sizeof(TBBUTTONINFO);
		tbbinfo.dwMask = TBIF_STATE;
		tbbinfo.fsState = ((rw == _viv_image_wide) && (rh == _viv_image_high)) ? (0) : (TBSTATE_ENABLED);
		
		SendMessage(_viv_toolbar_hwnd,TB_SETBUTTONINFO,VIV_ID_VIEW_1TO1,(LPARAM)&tbbinfo);
		
		tbbinfo.cbSize = sizeof(TBBUTTONINFO);
		tbbinfo.dwMask = TBIF_STATE;
		tbbinfo.fsState = ((_viv_zoom_pos == 0) && (!_viv_1to1)) ? (0) : (TBSTATE_ENABLED);
		
		SendMessage(_viv_toolbar_hwnd,TB_SETBUTTONINFO,VIV_ID_VIEW_BESTFIT,(LPARAM)&tbbinfo);
	}
}

static LRESULT CALLBACK _viv_fullscreen_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_PAINT:
		{
			RECT rect;
			PAINTSTRUCT ps;
			
			GetClientRect(hwnd,&rect);
			BeginPaint(hwnd,&ps);
			/*
			{
				HBRUSH hbrush;
				
				hbrush = CreateSolidBrush(RGB(config_fullscreen_background_color_r,config_fullscreen_background_color_g,config_fullscreen_background_color_b));
				
				if (hbrush)
				{
					FillRect(ps.hdc,&rect,hbrush);
				
					DeleteObject(hbrush);
				}
			}
			*/
			EndPaint(hwnd,&ps);
			break;
		}
			
		case WM_ERASEBKGND:
			return 1;
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static void _viv_status_set_temp_text(wchar_t *text)
{
	if (_viv_status_temp_text)
	{
		KillTimer(_viv_hwnd,VIV_ID_STATUS_TEMP_TEXT_TIMER);
		
		mem_free(_viv_status_temp_text);
		
		_viv_status_temp_text = 0;
	}
	
	if (text)
	{
		_viv_status_temp_text = string_alloc(text);
	}
	
	_viv_status_update();
	
	if (_viv_status_temp_text)
	{
		SetTimer(_viv_hwnd,VIV_ID_STATUS_TEMP_TEXT_TIMER,3000,0);
	}
}

static void _viv_status_update_temp_pos_zoom(void)
{
	wchar_t wbuf[STRING_SIZE];
	double x;
	double y;
	
	x = (double)((_viv_dst_pos_x - 500) / 500.0f);
	
	if (x < 0)
	{
		x -= 0.0005;
	}
	else
	{	
		x += 0.0005;
	}

	y = (float)((_viv_dst_pos_y - 500) / 500.0f);
	
	if (y < 0)
	{
		y -= 0.0005;
	}
	else
	{	
		y += 0.0005;
	}

	string_printf(wbuf,"Pos %0.3f %0.3f, Zoom %0.3f %0.3f, Aspect Ratio %0.3f",x,y,(float)_viv_dst_zoom_values[_viv_dst_zoom_x_pos],(float)_viv_dst_zoom_values[_viv_dst_zoom_y_pos],(_viv_dst_zoom_values[_viv_dst_zoom_x_pos] * (float)_viv_image_wide) / (_viv_dst_zoom_values[_viv_dst_zoom_y_pos] * (float)_viv_image_high));
			
	_viv_status_set_temp_text(wbuf);
}

static void _viv_status_update_temp_animation_rate(void)
{
	wchar_t wbuf[STRING_SIZE];

	string_printf(wbuf,"Animation rate %0.3f",_viv_animation_rates[_viv_animation_rate_pos]);
			
	_viv_status_set_temp_text(wbuf);
}

static void _viv_zoom_in(int out,int have_xy,int x,int y)
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
		GetClientRect(_viv_hwnd,&rect);
		pt.x = (rect.right - rect.left) / 2;
		pt.y = (rect.bottom - rect.top - _viv_get_status_high() - _viv_get_controls_high()) / 2;
	}

	ClientToScreen(_viv_hwnd,&pt);
	
	_viv_do_mousewheel_action(0,MAKEWPARAM(0,out ? -120 : 120),pt.x,pt.y);
}

static void _viv_status_update_slideshow_rate(void)
{
	wchar_t wbuf[STRING_SIZE];
	const wchar_t *res;
	int r;
	
	if ((config_slideshow_rate / 3600000) && ((config_slideshow_rate % 3600000) == 0))
	{
		r = config_slideshow_rate / 3600000;
		res = L"hours";
	}
	else
	if ((config_slideshow_rate / 60000) && ((config_slideshow_rate % 60000) == 0))
	{
		r = config_slideshow_rate / 60000;
		res = L"minutes";
	}
	else
	if ((config_slideshow_rate / 1000) && ((config_slideshow_rate % 1000) == 0))
	{
		r = config_slideshow_rate / 1000;
		res = L"seconds";
	}
	else
	{
		r = config_slideshow_rate;
		res = L"milliseconds";
	}
	
	string_printf(wbuf,"Slideshow rate %d %S",r,res);
	
	_viv_status_set_temp_text(wbuf);
}

static void _viv_command_line_options(void)
{
	wchar_t *text_wbuf;
	wchar_t caption_wbuf[STRING_SIZE];
	
	text_wbuf = string_alloc_utf8("Usage:\nvoidImageViewer.exe [/switches] [filename(s)]\n"
		"\n"
		"Switches:\n"
		"/slideshow\tStart a slideshow.\n"
		"/fullscreen\tStart fullscreen.\n"
		"/maximized\tStart maximized.\n"
		"/window\t\tStart windowed.\n"
		"/ontop\t\tShow on top of other windows.\n"
		"/minimal\t\tBorderless window.\n"
		"/compact\t\tBordered window.\n"
		"/x <x> /y <y> /width <width> /height <height>\n"
		"\t\tSet the Window position and size.\n"
		"/rate <rate>\tSet the slideshow rate in milliseconds.\n"
		"/name\t\tSort by name.\n"
		"/path\t\tSort by full path.\n"
		"/size\t\tSort by size.\n"
		"/dm\t\tSort by date modified.\n"
		"/dc\t\tSort by date created.\n"
		"/ascending\tSort in ascending order.\n"
		"/descending\tSort in descending order.\n"
//		"/everything <search> Open files from an Everything search.\n"
//		"/random <search>\tOpen random files from an Everything search.\n"
		"/shuffle\t\tShuffle playlist.\n"
		"/<bmp|gif|ico|jpeg|jpg|png|tif|tiff|webp>\n"
		"\t\tInstall association.\n"
		"/no<bmp|gif|ico|jpeg|jpg|png|tif|tiff|webp>\n"
		"\t\tUninstall association.\n"
		"/appdata\t\tSave settings in appdata.\n"
		"/noappdata\tSave settings in exe path.\n"
		"/startmenu\tAdd Start menu shortcuts.\n"
		"/nostartmenu\tRemove Start menu shortcuts.\n"
		"/install <path>\tInstall to the specified path.\n"
		"/install-options <...> Run with the specified options after installation.\n"
		"/uninstall <path>\tUninstall from the specified path.\n");
		
	string_copy_utf8(caption_wbuf,"void Image Viewer");

	// MB_ICONQUESTION avoids the messagebeep.
	MessageBox(_viv_hwnd,
		text_wbuf,
		caption_wbuf,MB_OK|MB_ICONQUESTION);
		
	mem_free(text_wbuf);
}

static void _viv_install_association(DWORD flags)
{
	int i;
	
	for(i=0;i<_VIV_ASSOCIATION_COUNT;i++)
	{
		if (flags & (1 << i))
		{
			_viv_install_association_by_extension(_viv_association_extensions[i],_viv_association_descriptions[i],_viv_association_icon_locations[i]);
		}
	}
}

static void _viv_uninstall_association(DWORD flags)
{
	int i;
	
	for(i=0;i<_VIV_ASSOCIATION_COUNT;i++)
	{
		if (flags & 1 << i)
		{
			_viv_uninstall_association_by_extension(_viv_association_extensions[i]);
		}
	}
}

static void _viv_view_scroll(int mx,int my)
{
	int old_view_x;
	int old_view_y;

	old_view_x = _viv_view_x;
	old_view_y = _viv_view_y;

	_viv_view_set(_viv_view_x - mx,_viv_view_y - my,0);
	
	if (config_scroll_window)
	{
		// this is not working for stamimail?!?
		// probably a RTL issue.
		if (ScrollWindowEx(_viv_hwnd,old_view_x - _viv_view_x,old_view_y - _viv_view_y,0,0,0,0,SW_INVALIDATE) == ERROR)
		{
			debug_printf("scroll error %d\n",GetLastError());
		}
	}
	else
	{
		// this fixes the scroll bug for stamimail
		InvalidateRect(_viv_hwnd,0,FALSE);
	}
	
//	UpdateWindow(_viv_hwnd);
}

static void _viv_update_color_button_bitmap(HWND hwnd)
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

static void _viv_delete_color_button_bitmap(HWND hwnd)
{	
	HGDIOBJ last_hbitmap;
	
	last_hbitmap = (HBITMAP)SendMessage(hwnd,BM_SETIMAGE,IMAGE_BITMAP,0);
	
	if (last_hbitmap)
	{
		DeleteObject(last_hbitmap);
	}
}

static void _viv_get_command_name(wchar_t *wbuf,int command_index)
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

	string_copy_utf8(buf,menu_name);
	
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

static int _viv_command_index_from_command_id(int command_id)
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
		string_copy_utf8(keytext,default_keytext);
	}
	
	string_cat(wbuf,keytext);
	string_cat_utf8(wbuf,(const utf8_t *)"+");
}

static void _viv_get_key_text(wchar_t *wbuf,DWORD keyflags)
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

static HMENU _viv_create_menu(void)
{
	HMENU hmenu;
	
	hmenu = CreateMenu();
	
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
				if (_viv_commands[i].flags & MF_SEPARATOR)
				{
					AppendMenu(menus[_viv_commands[i].menu_id],_viv_commands[i].flags & (~MF_DELETE),_viv_commands[i].command_id,L"");
				}
				else
				{
					wchar_t text_wbuf[STRING_SIZE];
					
					string_copy_utf8(text_wbuf,localization_get_string(_viv_commands[i].localization_id));

					if (_viv_commands[i].flags & MF_POPUP)
					{
						if (!menus[_viv_commands[i].command_id])
						{
							menus[_viv_commands[i].command_id] = CreatePopupMenu();
						}
						
						AppendMenu(menus[_viv_commands[i].menu_id],_viv_commands[i].flags & (~MF_DELETE),(UINT_PTR)menus[_viv_commands[i].command_id],text_wbuf);
					}
					else
					{
						int key_command_index;
						wchar_t key_text[STRING_SIZE];
						
						key_command_index = i;
						
						switch (_viv_commands[i].command_id)
						{
							case VIV_ID_FILE_DELETE:
								key_command_index = _viv_command_index_from_command_id(VIV_ID_FILE_DELETE_RECYCLE);
								break;
						}
						
						if (key_command_index >= 0)
						{
							if (_viv_key_list->start[key_command_index])
							{
								_viv_get_key_text(key_text,_viv_key_list->start[key_command_index]->key);
								
								string_cat_utf8(text_wbuf,(const utf8_t *)"\t");
								string_cat(text_wbuf,key_text);
							}
						}
						
						AppendMenu(menus[_viv_commands[i].menu_id],_viv_commands[i].flags & (~MF_DELETE),_viv_commands[i].command_id,text_wbuf);
					}
				}
			}
		}
	}
	
	return hmenu;
}

static void _viv_key_add(_viv_key_list_t *key_list,int command_index,DWORD keyflags)
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

static void _viv_key_list_copy(_viv_key_list_t *dst,const _viv_key_list_t *src)
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

static void _viv_key_clear_all(_viv_key_list_t *list)
{
	// free keys
	int i;
	
	for(i=0;i<_VIV_COMMAND_COUNT;i++)
	{
		_viv_key_clear(list,i);
	}
}

static void _viv_key_list_init(_viv_key_list_t *list)
{
	// free keys
	int i;
	
	for(i=0;i<_VIV_COMMAND_COUNT;i++)
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
			
			key_flags = _viv_get_current_key_mod_flags();
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

			_viv_edit_key_set_key(hwnd,key_flags | vk);
			_viv_options_edit_key_changed(GetParent(hwnd));

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

static int _viv_get_current_key_mod_flags(void)
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

static void _viv_options_edit_key_changed(HWND hwnd)
{
	int keyflags;
	int command_index;
	
	keyflags = GetWindowLongPtr(hwnd,GWLP_USERDATA);
	
	ListBox_ResetContent(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST));
	SetWindowRedraw(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST),FALSE);
	
	for(command_index=0;command_index<_VIV_COMMAND_COUNT;command_index++)
	{
		config_key_t *key;
		
		key = _viv_key_list->start[command_index];
		
		while(key)
		{
			if (key->key == keyflags)
			{
				wchar_t command_wbuf[STRING_SIZE];
				
				_viv_get_command_name(command_wbuf,command_index);
				
				ListBox_AddString(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST),command_wbuf);
			}
		
			key = key->next;
		}
	}

	ListBox_SetCurSel(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST),0);

	SetWindowRedraw(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST),TRUE);
}

static void _viv_edit_key_set_key(HWND hwnd,DWORD key_flags)
{
	wchar_t keytext_wbuf[STRING_SIZE];

	if (key_flags)
	{
		_viv_get_key_text(keytext_wbuf,key_flags);
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

static void _viv_key_remove(_viv_key_list_t *keylist,int command_index,DWORD keyflags)
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

static void _viv_edit_key_remove_currently_used_by(_viv_key_list_t *keylist,DWORD keyflags)
{
	int command_index;
	
	for(command_index=0;command_index<_VIV_COMMAND_COUNT;command_index++)
	{
		_viv_key_remove(keylist,command_index,keyflags);
	}
}

//FIXME: we should timeout 
//FIXME: we should check for the process name voidImageViewer.exe rather than the window class name. -be careful when uninstalling as the non-admin process will be waiting for the admin process to exit.
static void _viv_close_existing_process(void)
{
	for(;;)
	{
		HWND hwnd;
		
		hwnd = FindWindowA("VOIDIMAGEVIEWER",0);
		if (hwnd)
		{
			DWORD process_id;
			HANDLE process_handle;
			
			GetWindowThreadProcessId(hwnd,&process_id);
			
			process_handle = OpenProcess(SYNCHRONIZE,FALSE,process_id);

			SendMessage(hwnd,WM_CLOSE,0,0);
			
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

static void _viv_uninstall_delete_file(const wchar_t *path,const utf8_t *filename)
{
	wchar_t full_path_and_filename[STRING_SIZE];
	
	string_path_combine_utf8(full_path_and_filename,path,filename);
	
	DeleteFile(full_path_and_filename);
}

static int _viv_is_start_menu_shortcuts(void)
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

static void _viv_install_start_menu_shortcuts(void)
{
	wchar_t special_folder_path_wbuf[STRING_SIZE];
	
	// always uninstall and reinstall
	// this allows us to switch between english and another language.
	_viv_uninstall_start_menu_shortcuts();

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
		_viv_get_exe_filename(exe_filename_wbuf);
		string_path_combine_utf8(lnk_wbuf,path_wbuf,"void Image Viewer.lnk");
		os_create_shell_link(exe_filename_wbuf,lnk_wbuf);
	
		// uninstall
		string_get_path_part(exe_path_wbuf,exe_filename_wbuf);
		string_path_combine_utf8(uninstall_filename_wbuf,exe_path_wbuf,(const utf8_t *)"Uninstall.exe");

		string_path_combine_utf8(lnk_wbuf,path_wbuf,"Uninstall.lnk");
		os_create_shell_link(uninstall_filename_wbuf,lnk_wbuf);
	}
}

static void _viv_uninstall_start_menu_shortcuts(void)
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

static void _viv_append_admin_param(wchar_t *wbuf,const utf8_t *param)
{
	string_cat_utf8(wbuf,(const utf8_t *)" /");
	string_cat_utf8(wbuf,param);
}

static void _viv_install_copy_file(const wchar_t *install_path,const wchar_t *temp_path,const utf8_t *filename,int critical)
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

static void _viv_shuffle_playlist(void)
{
	if (_viv_playlist_count)
	{
		{
			LARGE_INTEGER counter;
			
			QueryPerformanceCounter(&counter);
			
			srand(counter.LowPart);
		}
		
		_viv_playlist_shuffle_allocated = _VIV_DEFAULT_SHUFFLE_ALLOCATED;
		
		while(_viv_playlist_shuffle_allocated < _viv_playlist_count)
		{
			_viv_playlist_shuffle_allocated *= 2;
		}
		
		// allocate indexes.
		_viv_playlist_shuffle_indexes = mem_alloc(_viv_playlist_shuffle_allocated * sizeof(_viv_playlist_t *));
		
		// fill in indexes
		{
			_viv_playlist_t **d;
			_viv_playlist_t *p;
		
			d = _viv_playlist_shuffle_indexes;
			p = _viv_playlist_start;
		
			while(p)
			{
				*d++ = p;
				
				p = p->next;
			}
		}
		
		// shuffle
		{
			int i;
			
			for(i=0;i<_viv_playlist_count-1;i++)
			{
				int j;
				_viv_playlist_t *tmp;
				
				j = i + rand() % (_viv_playlist_count - i);
				
				tmp = _viv_playlist_shuffle_indexes[i];
				_viv_playlist_shuffle_indexes[i] = _viv_playlist_shuffle_indexes[j];
				_viv_playlist_shuffle_indexes[j] = tmp;
			}
		}
	}
}

static void _viv_jumpto_on_size(HWND hwnd)
{
	RECT rect;
	int wide;
	int high;
	int edit_high;
	int button_wide;
	int button_high;
	int y;
	int edge;
	int list_high;

	GetClientRect(hwnd,&rect);
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top;

	GetWindowRect(GetDlgItem(hwnd,IDC_JUMPTO_EDIT),&rect);
	edit_high = rect.bottom - rect.top;

	GetWindowRect(GetDlgItem(hwnd,IDOK),&rect);
	button_wide = rect.right - rect.left;
	button_high = rect.bottom - rect.top;

	edge = (12 * os_logical_high) / 96;
	y = edge;

	SetWindowPos(GetDlgItem(hwnd,IDC_JUMPTO_EDIT),0,edge,y,wide-edge-edge,edit_high,SWP_NOACTIVATE|SWP_NOZORDER);

	y += edit_high + (6 * os_logical_high) / 96;

	high -= button_high + edge;

	list_high = (high - y) - edge;
	if (list_high < 0)
	{
		list_high = 0;
	}

	SetWindowPos(GetDlgItem(hwnd,IDC_JUMPTO_LIST),0,edge,y,wide-edge-edge,list_high,SWP_NOACTIVATE|SWP_NOZORDER);

	y += list_high + edge;

	SetWindowPos(GetDlgItem(hwnd,IDOK),0,wide - edge - button_wide - edge - button_wide,y,button_wide,button_high,SWP_NOACTIVATE|SWP_NOZORDER);
	SetWindowPos(GetDlgItem(hwnd,IDCANCEL),0,wide - edge - button_wide,y,button_wide,button_high,SWP_NOACTIVATE|SWP_NOZORDER);
}

static void _viv_jumpto_on_search(HWND hwnd)
{
	wchar_t search_wbuf[STRING_SIZE];
	int is_path_search;
	
	GetDlgItemText(hwnd,IDC_JUMPTO_EDIT,search_wbuf,STRING_SIZE);
	
	SendDlgItemMessage(hwnd,IDC_JUMPTO_LIST,WM_SETREDRAW,FALSE,0);
	
	ListBox_ResetContent(GetDlgItem(hwnd,IDC_JUMPTO_LIST));
	
	is_path_search = 0;
	
	{
		const wchar_t *p;
		
		p = search_wbuf;

		while(*p)
		{
			if ((*p == '\\') || (*p == '/') || (*p == ':'))
			{
				is_path_search = 1;
				break;
			}
			
			p++;
		}
	}
	
	{
		int i;
		
		for(i=0;i<_viv_nav_item_count;i++)
		{
			const wchar_t *name;
			const wchar_t *search_name;
			
			name = string_get_filename_part(_viv_nav_items[i]->fd.cFileName);
			
			if (is_path_search)
			{
				search_name = _viv_nav_items[i]->fd.cFileName;
			}
			else
			{
				search_name = name;
			}
			
			if ((!(*search_wbuf)) || (StrStrI(search_name,search_wbuf)))
			{
				int lb_index;
				
				lb_index = ListBox_AddString(GetDlgItem(hwnd,IDC_JUMPTO_LIST),string_get_filename_part(name));
				
				ListBox_SetItemData(GetDlgItem(hwnd,IDC_JUMPTO_LIST),lb_index,i);
			}
		}
	}
	
	ListBox_SetCurSel(GetDlgItem(hwnd,IDC_JUMPTO_LIST),0);
	
	SendDlgItemMessage(hwnd,IDC_JUMPTO_LIST,WM_SETREDRAW,TRUE,0);
}

static void _viv_jumpto_open_sel(HWND hwnd)
{
	int lb_index;
	
	lb_index = ListBox_GetCurSel(GetDlgItem(hwnd,IDC_JUMPTO_LIST));

	if (lb_index != LB_ERR)
	{	
		int index;
		
		index = ListBox_GetItemData(GetDlgItem(hwnd,IDC_JUMPTO_LIST),lb_index);
	
		if ((index >= 0) && (index < _viv_nav_item_count))
		{
			_viv_open(&_viv_nav_items[index]->fd,0);
		}
	}
}
			
static LRESULT CALLBACK _viv_jumpto_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			int cur_index;
			
			cur_index = LB_ERR;
				
			os_center_dialog(hwnd);
			
			_viv_nav_item_free_all();
			
			if (_viv_playlist_start)
			{
				_viv_playlist_t *d;
				
				d = _viv_playlist_start;
				while(d)
				{
					_viv_nav_item_add(&d->fd);
				
					d = d->next;
				}
			}
			else
			if (*_viv_current_fd->cFileName)
			{		
				WIN32_FIND_DATA fd;
				HANDLE h;
				wchar_t search_wbuf[STRING_SIZE];
				wchar_t path_wbuf[STRING_SIZE];

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
							string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
							string_copy_with_bufsize(fd.cFileName,MAX_PATH,search_wbuf);
							
							_viv_nav_item_add(&fd);
						}
						
						if (!FindNextFile(h,&fd)) break;
					}

					FindClose(h);
				}
			}
			
			if (_viv_nav_item_count)
			{
				_viv_nav_item_t **d;
				_viv_nav_item_t *navitem;
				
				_viv_nav_items = (_viv_nav_item_t **)mem_alloc(_viv_nav_item_count * sizeof(_viv_nav_item_t *));
				d = _viv_nav_items;
				
				navitem = __viv_nav_item_start;
				while(navitem)
				{
					*d++ = navitem;
					
					navitem = navitem->next;
				}
			}
			
			// sort by name 
			os_qsort(_viv_nav_items,_viv_nav_item_count,_viv_nav_compare);
			
			// find current.
			{
				_viv_nav_item_t **p;
				SIZE_T run;
				int index;
				
				p = _viv_nav_items;
				run = _viv_nav_item_count;
				index = 0;
				
				while(run)
				{
					if (string_compare((*p)->fd.cFileName,_viv_current_fd->cFileName) == 0)
					{
						cur_index = index;
					}
				
					index++;
					p++;
					run--;
				}
			}
			
			_viv_jumpto_on_size(hwnd);
			_viv_jumpto_on_search(hwnd);
			
			if (cur_index != LB_ERR)
			{
				ListBox_SetCurSel(GetDlgItem(hwnd,IDC_JUMPTO_LIST),cur_index);
				
				_viv_center_listbox_item(GetDlgItem(hwnd,IDC_JUMPTO_LIST),cur_index);
			}
			
			return TRUE;
		}
			
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDC_JUMPTO_EDIT:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						_viv_jumpto_on_search(hwnd);
						break;
					}
					break;

				case IDC_JUMPTO_LIST:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						_viv_jumpto_open_sel(hwnd);
						_viv_jump_ret = 1;
						break;
					}
					break;
					
				case IDOK:
					_viv_jumpto_open_sel(hwnd);
					_viv_jump_ret = 1;
					break;

				case IDCANCEL:
					_viv_jump_ret = 1;
					break;
			}
			
			break;
		
		case WM_SIZE:
			_viv_jumpto_on_size(hwnd);
			break;

		case WM_CLOSE:
			_viv_jump_ret = 1;
			return TRUE;
	    
		case WM_DESTROY:
			_viv_nav_item_free_all();
			break;
	}
	
	return FALSE;
}

static void _viv_show_jumpto(void)
{
	HWND dialog_hwnd;
	
	_viv_jump_ret = 0;
	
	dialog_hwnd = CreateDialog(os_hinstance,MAKEINTRESOURCE(IDD_JUMPTO),_viv_hwnd,_viv_jumpto_proc);
	
	if (dialog_hwnd)
	{
		ShowWindow(dialog_hwnd,SW_SHOW);
		EnableWindow(_viv_hwnd,FALSE);

		while(!_viv_jump_ret)
		{
			MSG msg;
			
			if (!GetMessageW(&msg,NULL,0,0))
			{
				break;
			}
			
			if ((msg.hwnd == dialog_hwnd) || (IsChild(dialog_hwnd,msg.hwnd)))
			{
				switch (msg.message)
				{
					case WM_MOUSEWHEEL:
						
						SendMessage(GetDlgItem(dialog_hwnd,IDC_JUMPTO_LIST),WM_MOUSEWHEEL,msg.wParam,msg.lParam);
						continue;
						
					case WM_SYSKEYDOWN:
					case WM_KEYDOWN:
					
						if (msg.hwnd == GetDlgItem(dialog_hwnd,IDC_JUMPTO_EDIT))
						{
							int key_flags;
							int vk;
							
							key_flags = _viv_get_current_key_mod_flags();
							vk = msg.wParam;
							
							switch(vk)
							{
								case VK_UP:
								case VK_DOWN:
								case VK_NEXT:
								case VK_PRIOR:
									
									
									{
										int count;
										int index;

										// msg->hwnd = GetDlgItem(dialog_hwnd,IDC_JUMPTO_LIST);
										
										index = -1;
										count = ListBox_GetCount(GetDlgItem(dialog_hwnd,IDC_JUMPTO_LIST));
										
										if (count)
										{
											switch (msg.wParam)
											{
												case VK_DOWN:
												case VK_NEXT:
													index = 0;
													break;
													
												default:
													index = count - 1;
													break;
											}
										}

										if (index != -1)
										{
											ListBox_SetCurSel(GetDlgItem(dialog_hwnd,IDC_JUMPTO_LIST),index);
										}

										SetFocus(GetDlgItem(dialog_hwnd,IDC_JUMPTO_LIST));					
									}
									
//									SendMessage(GetDlgItem(dialog_hwnd,IDC_JUMPTO_LIST),msg.message,msg.wParam,msg.lParam);

									//SetFocus(GetDlgItem(dialog_hwnd,IDC_JUMPTO_LIST));					
									
									continue;
							}
						}
						
						break;
							
				}
			}
			
			if (!IsDialogMessageW(dialog_hwnd,&msg))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
		
		EnableWindow(_viv_hwnd,TRUE);
		DestroyWindow(dialog_hwnd);
	}
}

static void _viv_nav_item_free_all(void)
{
	if (_viv_nav_items)
	{
		int i;
		
		for(i=0;i<_viv_nav_item_count;i++)
		{
			mem_free(_viv_nav_items[i]);
		}
		
		mem_free(_viv_nav_items);

		__viv_nav_item_start = 0;
		_viv_nav_item_last = 0;
		_viv_nav_items = 0;
		_viv_nav_item_count = 0;
	}
}

static void _viv_nav_item_add(WIN32_FIND_DATA *fd)
{
	_viv_nav_item_t *navitem;
	
	navitem = mem_alloc(sizeof(_viv_nav_item_t));
	
	os_copy_memory(&navitem->fd,fd,sizeof(WIN32_FIND_DATA));

	if (__viv_nav_item_start)
	{
		_viv_nav_item_last->next = navitem;
	}
	else
	{
		__viv_nav_item_start = navitem;
	}
	
	navitem->next = 0;
	_viv_nav_item_last = navitem;
	_viv_nav_item_count++;				
}

static int _viv_nav_compare(const _viv_nav_item_t *a,const _viv_nav_item_t *b)
{
	return _viv_fd_compare_name(&a->fd,&b->fd);
}

static INT_PTR CALLBACK _viv_search_everything_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			wchar_t caption_wbuf[STRING_SIZE];
			
			os_center_dialog(hwnd);
			
			SetWindowLongPtr(hwnd,GWLP_USERDATA,lParam);
			
			string_copy_utf8(caption_wbuf,lParam ? "Add Everything Search" : "Load Everything Search");
			SetWindowText(hwnd,caption_wbuf);
			
			return TRUE;
		}
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					wchar_t search_wbuf[STRING_SIZE];
					
					GetDlgItemText(hwnd,IDC_EVERYTHING_EDIT,search_wbuf,STRING_SIZE);
					
					if (_viv_send_everything_search(hwnd,GetWindowLongPtr(hwnd,GWLP_USERDATA),IsDlgButtonChecked(hwnd,IDC_SEARCH_EVERYTHING_RANDOM) == BST_CHECKED,search_wbuf))
					{
						EndDialog(hwnd,0);
					}
					
					break;
				}
					
				case IDCANCEL:
					EndDialog(hwnd,0);
					break;
			}

			break;
	}
	
	return FALSE;
}

static void _viv_search_everything(int add)
{
	DialogBoxParam(os_hinstance,MAKEINTRESOURCE(IDD_EVERYTHING),_viv_hwnd,_viv_search_everything_proc,add);
}

static int _viv_send_everything_search(HWND hwnd,int add,int randomize,const wchar_t *search)
{
	if (randomize)
	{	
		// add is ignored in this case.
		_viv_random = string_alloc(search);

		_viv_random_tot_results = 0xffffffff;

		_viv_playlist_clearall();

		{
			LARGE_INTEGER counter;
			
			QueryPerformanceCounter(&counter);
			
			srand(counter.LowPart);
		}
									
		_viv_home(0,0);
		
		return 1;
	}
	else
	{
		HWND everything_hwnd;
		
		if (_viv_random)
		{
			mem_free(_viv_random);
			
			_viv_random = 0;
		}
		
		everything_hwnd = FindWindowA(EVERYTHING_IPC_WNDCLASSA,0);
		
		if (everything_hwnd)
		{
			EVERYTHING_IPC_QUERY2 *q;
			COPYDATASTRUCT cds;
			DWORD size;
			wchar_t new_search[STRING_SIZE];
			
			string_copy_utf8(new_search,"ext:bmp;gif;ico;jpeg;jpg;png;tif;tiff;webp <");
			string_cat(new_search,search);
			string_cat_utf8(new_search,">");

			size = (DWORD)(sizeof(EVERYTHING_IPC_QUERY2) + (string_length(new_search) + 1) * sizeof(wchar_t));
			
			_viv_everything_request_flags = EVERYTHING_IPC_QUERY2_REQUEST_FULL_PATH_AND_NAME; 
					
			if (SendMessage(everything_hwnd,EVERYTHING_WM_IPC,EVERYTHING_IPC_IS_FILE_INFO_INDEXED,EVERYTHING_IPC_FILE_INFO_FILE_SIZE))
			{
				_viv_everything_request_flags |= EVERYTHING_IPC_QUERY2_REQUEST_SIZE | EVERYTHING_IPC_QUERY2_REQUEST_DATE_MODIFIED; 
			}
			
			if (SendMessage(everything_hwnd,EVERYTHING_WM_IPC,EVERYTHING_IPC_IS_FILE_INFO_INDEXED,EVERYTHING_IPC_FILE_INFO_DATE_MODIFIED))
			{
				_viv_everything_request_flags |= EVERYTHING_IPC_QUERY2_REQUEST_DATE_MODIFIED; 
			}
			
			if (SendMessage(everything_hwnd,EVERYTHING_WM_IPC,EVERYTHING_IPC_IS_FILE_INFO_INDEXED,EVERYTHING_IPC_FILE_INFO_DATE_CREATED))
			{
				_viv_everything_request_flags |= EVERYTHING_IPC_QUERY2_REQUEST_DATE_CREATED; 
			}
			
			q = mem_alloc(size);
			
			q->reply_hwnd = (DWORD)_viv_hwnd;
			q->reply_copydata_message = add ? _VIV_COPYDATA_ADD_EVERYTHING_SEARCH : _VIV_COPYDATA_OPEN_EVERYTHING_SEARCH;
			q->search_flags = 0;
			q->offset = 0;
			q->max_results = EVERYTHING_IPC_ALLRESULTS;
			q->request_flags = _viv_everything_request_flags;
			
			q->sort_type = EVERYTHING_IPC_SORT_NAME_ASCENDING;
			os_copy_memory(q+1,new_search,(string_length(new_search) + 1) * sizeof(wchar_t));
			
			cds.dwData = EVERYTHING_IPC_COPYDATA_QUERY2;
			cds.cbData = size;
			cds.lpData = q;
			
			SendMessage(everything_hwnd,WM_COPYDATA,(WPARAM)_viv_hwnd,(LPARAM)&cds);
			
			mem_free(q);
			
			return 1;
		}
		else
		{
			wchar_t *text_wbuf;
			wchar_t caption_wbuf[STRING_SIZE];
			
			text_wbuf = string_alloc_utf8(localization_get_string(LOCALIZATION_ID_EVERYTHING_NOT_AVAILABLE_MESSAGE));
				
			string_copy_utf8(caption_wbuf,localization_get_string(LOCALIZATION_ID_APP_NAME));

			// MB_ICONQUESTION avoids the messagebeep.
			MessageBox(hwnd,text_wbuf,caption_wbuf,MB_OK|MB_ICONERROR);
				
			mem_free(text_wbuf);
			
			return 0;
		}
	}
}

static void _viv_add_current_path_to_playlist(void)
{
	WIN32_FIND_DATA fd;
	HANDLE h;
	wchar_t search_wbuf[STRING_SIZE];
	wchar_t path_wbuf[STRING_SIZE];

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
				_viv_playlist_t *d;
				
				string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
				string_copy(fd.cFileName,search_wbuf);
				
				d = _viv_playlist_add(&fd);
				
				if (string_compare(fd.cFileName,_viv_current_fd->cFileName) == 0)
				{
					// copy back the FD ID.
					// so when we get the next image we get the next image from the current image instead of the home image.
					_viv_current_fd->dwReserved0 = d->fd.dwReserved0;
					_viv_current_fd->dwReserved1 = d->fd.dwReserved1;
				}
			}
			
			if (!FindNextFile(h,&fd)) break;
		}

		FindClose(h);
	}
}

static int _viv_playlist_shuffle_index_from_fd(const WIN32_FIND_DATA *fd)
{
	int index;

	if (_viv_playlist_shuffle_indexes)
	{
		for(index=0;index < _viv_playlist_count;index++)
		{
			if (_viv_playlist_shuffle_indexes[index]->fd.dwReserved0 == fd->dwReserved0)
			{
				if (_viv_playlist_shuffle_indexes[index]->fd.dwReserved1 == fd->dwReserved1)
				{
					return index;
				}
			}
		}
	}
	
	return -1;
}

static _viv_playlist_t *_viv_playlist_from_fd(const WIN32_FIND_DATA *fd)
{
	_viv_playlist_t *d;
	
	d = _viv_playlist_start;
	while(d)
	{
		if (d->fd.dwReserved0 == fd->dwReserved0)
		{
			if (d->fd.dwReserved1 == fd->dwReserved1)
			{
				return d;
			}
		}
		
		d = d->next;
	}

	return 0;
}

static void _viv_do_initial_shuffle(void)
{
	// if we don't have a playlist, but we have a current image, add the current path to the playlist and shuffle it.
	if (config_shuffle)
	{
		if ((!_viv_playlist_start) && (*_viv_current_fd->cFileName))
		{
			_viv_add_current_path_to_playlist();
			
			// shuffle_playlist
			_viv_shuffle_playlist();
		}
		else
		if ((_viv_playlist_start) && (!_viv_playlist_shuffle_indexes))
		{
			// shuffle_playlist
			_viv_shuffle_playlist();
		}
	}
}

static void _viv_update_1to1_scroll(int x,int y)
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
	high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_controls_high();
	
	pt.x = x ;
	pt.y = y;
	
	cursor_x = pt.x; 
	cursor_y = pt.y;

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

// #define PHOTO_ORIENTATION_NORMAL            1u
// #define PHOTO_ORIENTATION_FLIPHORIZONTAL    2u
// #define PHOTO_ORIENTATION_ROTATE180         3u
// #define PHOTO_ORIENTATION_FLIPVERTICAL      4u
// #define PHOTO_ORIENTATION_TRANSPOSE         5u
// #define PHOTO_ORIENTATION_ROTATE270         6u
// #define PHOTO_ORIENTATION_TRANSVERSE        7u
// #define PHOTO_ORIENTATION_ROTATE90          8u
static HBITMAP _viv_orientate_hbitmap(HBITMAP hbitmap,int orientation)
{
	BITMAP bitmap;
	HBITMAP ret_hbitmap;
	
	ret_hbitmap = 0;
	
	if (GetObject(hbitmap,sizeof(BITMAP),&bitmap))
	{
		HDC screen_hdc;
		
		screen_hdc = GetDC(0);
		if (screen_hdc)
		{
			HDC mem_hdc;
			
			mem_hdc = CreateCompatibleDC(screen_hdc);
			if (mem_hdc)
			{
				int ret_wide;
				int ret_high;
				
				ret_wide = bitmap.bmWidth;
				ret_high = bitmap.bmHeight;
				
				switch(orientation)
				{
					case 5: // #define PHOTO_ORIENTATION_TRANSPOSE         5u
					case 6: // #define PHOTO_ORIENTATION_ROTATE270         6u
					case 7: // #define PHOTO_ORIENTATION_TRANSVERSE        7u
					case 8: // #define PHOTO_ORIENTATION_ROTATE90          8u
						
						ret_wide = bitmap.bmHeight;
						ret_high = bitmap.bmWidth;
						
						break;
				}
				
				ret_hbitmap = CreateCompatibleBitmap(screen_hdc,ret_wide,ret_high);
				if (ret_hbitmap)
				{
					int ret;
					DWORD *old_pixels;
					DWORD *new_pixels;
					BITMAPINFOHEADER bi;
					
					ret = 0;
					
					os_zero_memory(&bi,sizeof(BITMAPINFOHEADER));
					bi.biSize = sizeof(BITMAPINFOHEADER);
					bi.biWidth = bitmap.bmWidth;
					bi.biHeight = -bitmap.bmHeight;
					bi.biPlanes = 1;
					bi.biBitCount = 32;
					bi.biCompression = BI_RGB;
										
					old_pixels = mem_alloc(bitmap.bmWidth * bitmap.bmHeight * sizeof(DWORD));
					new_pixels = mem_alloc(ret_wide * ret_high * sizeof(DWORD));
				
					if (GetDIBits(mem_hdc,hbitmap,0,bitmap.bmHeight,old_pixels,(BITMAPINFO *)&bi,DIB_RGB_COLORS))
					{
						int y;

						switch(orientation)
						{
							case 2: // #define PHOTO_ORIENTATION_FLIPHORIZONTAL    2u
							
								for(y=0;y<bitmap.bmHeight;y++)
								{
									int x;
									
									for(x=0;x<bitmap.bmWidth;x++)
									{
										new_pixels[x + (y * ret_wide)] = old_pixels[bitmap.bmWidth - x - 1 + (y * bitmap.bmWidth)];
									}
								}
								
								break;
							
							case 3: // #define PHOTO_ORIENTATION_ROTATE180         3u
							
								for(y=0;y<bitmap.bmHeight;y++)
								{
									int x;
									
									for(x=0;x<bitmap.bmWidth;x++)
									{
										new_pixels[x + (y * ret_wide)] = old_pixels[bitmap.bmWidth - x - 1 + ((bitmap.bmHeight - y - 1) * bitmap.bmWidth)];
									}
								}
								
								break;
								
							case 4: // #define PHOTO_ORIENTATION_FLIPVERTICAL      4u
							
								for(y=0;y<bitmap.bmHeight;y++)
								{
									int x;
									
									for(x=0;x<bitmap.bmWidth;x++)
									{
										new_pixels[x + (y * ret_wide)] = old_pixels[x + ((bitmap.bmHeight - y - 1) * bitmap.bmWidth)];
									}
								}
								
								break;
								
							case 5: // #define PHOTO_ORIENTATION_TRANSPOSE         5u
								
								for (y = 0; y < bitmap.bmWidth; y++)
								{
									int x;
									
									for (x = 0; x < bitmap.bmHeight; x++)
									{
										// Transpose: new(x, y) = old(y, x)
										new_pixels[x + (y * ret_wide)] = old_pixels[y + (x * bitmap.bmWidth)];
									}
								}

								break;
								
							case 6: // #define PHOTO_ORIENTATION_ROTATE270         6u
								
								for(y=0;y<bitmap.bmWidth;y++)
								{
									int x;
									
									for(x=0;x<bitmap.bmHeight;x++)
									{
										new_pixels[x + (y * ret_wide)] = old_pixels[y + ((bitmap.bmHeight-x-1) * bitmap.bmWidth)];
									}
								}		
								
								break;
	
							case 7: // #define PHOTO_ORIENTATION_TRANSVERSE        7u
								
								for (y = 0; y < bitmap.bmWidth; y++)
								{
									int x;
									
									for (x = 0; x < bitmap.bmHeight; x++)
									{
										new_pixels[x + (y * ret_wide)] = old_pixels[(bitmap.bmWidth - 1 - y) + ((bitmap.bmHeight - 1 - x) * bitmap.bmWidth)];
									}
								}

								break;
								
							case 8: // #define PHOTO_ORIENTATION_ROTATE90          8u
							
								for(y=0;y<bitmap.bmWidth;y++)
								{
									int x;
									
									for(x=0;x<bitmap.bmHeight;x++)
									{
										new_pixels[x + (y * ret_wide)] = old_pixels[(bitmap.bmWidth-y-1) + (x * bitmap.bmWidth)];
									}
								}

								break;
						}
						
						os_zero_memory(&bi,sizeof(BITMAPINFOHEADER));
						bi.biSize = sizeof(BITMAPINFOHEADER);
						bi.biWidth = ret_wide;
						bi.biHeight = -ret_high;
						bi.biPlanes = 1;
						bi.biBitCount = 32;
						bi.biCompression = BI_RGB;
						
						if (SetDIBits(mem_hdc,ret_hbitmap,0,ret_high,new_pixels,(BITMAPINFO *)&bi,DIB_RGB_COLORS))
						{
							ret = 1;
						}
					}
					
					if (!ret)
					{
						DeleteObject(ret_hbitmap);
						
						ret_hbitmap = 0;	
					}
					
					mem_free(new_pixels);
					mem_free(old_pixels);
				}
				
				DeleteDC(mem_hdc);
			}
			
			ReleaseDC(0,screen_hdc);
		}
	}
	
	return ret_hbitmap;
}

static void _viv_send_random_everything_search(void)
{
	HWND everything_hwnd;
	
	everything_hwnd = FindWindowA(EVERYTHING_IPC_WNDCLASSA,0);
	
	if (everything_hwnd)
	{
		EVERYTHING_IPC_QUERY2 *q;
		COPYDATASTRUCT cds;
		DWORD size;
		wchar_t new_search[STRING_SIZE];
		
		string_copy_utf8(new_search,"ext:bmp;gif;ico;jpeg;jpg;png;tif;tiff;webp <");
		string_cat(new_search,_viv_random);
		string_cat_utf8(new_search,">");

		size = (DWORD)(sizeof(EVERYTHING_IPC_QUERY2) + (string_length(new_search) + 1) * sizeof(wchar_t));
		
		_viv_everything_request_flags = EVERYTHING_IPC_QUERY2_REQUEST_FULL_PATH_AND_NAME; 
				
		if (SendMessage(everything_hwnd,EVERYTHING_WM_IPC,EVERYTHING_IPC_IS_FILE_INFO_INDEXED,EVERYTHING_IPC_FILE_INFO_FILE_SIZE))
		{
			_viv_everything_request_flags |= EVERYTHING_IPC_QUERY2_REQUEST_SIZE | EVERYTHING_IPC_QUERY2_REQUEST_DATE_MODIFIED; 
		}
		
		if (SendMessage(everything_hwnd,EVERYTHING_WM_IPC,EVERYTHING_IPC_IS_FILE_INFO_INDEXED,EVERYTHING_IPC_FILE_INFO_DATE_MODIFIED))
		{
			_viv_everything_request_flags |= EVERYTHING_IPC_QUERY2_REQUEST_DATE_MODIFIED; 
		}
		
		if (SendMessage(everything_hwnd,EVERYTHING_WM_IPC,EVERYTHING_IPC_IS_FILE_INFO_INDEXED,EVERYTHING_IPC_FILE_INFO_DATE_CREATED))
		{
			_viv_everything_request_flags |= EVERYTHING_IPC_QUERY2_REQUEST_DATE_CREATED; 
		}
		
		q = mem_alloc(size);
		
		q->reply_hwnd = (DWORD)_viv_hwnd;
		q->reply_copydata_message = _VIV_COPYDATA_RANDOM_EVERYTHING_SEARCH;
		q->search_flags = 0;
		q->offset = ((rand() * RAND_MAX) + rand()) % _viv_random_tot_results;
		q->max_results = 1;
		q->request_flags = _viv_everything_request_flags;
		
		debug_printf("rand index %d\n",q->offset);
		
		q->sort_type = EVERYTHING_IPC_SORT_NAME_ASCENDING;
		os_copy_memory(q+1,new_search,(string_length(new_search) + 1) * sizeof(wchar_t));
		
		cds.dwData = EVERYTHING_IPC_COPYDATA_QUERY2;
		cds.cbData = size;
		cds.lpData = q;
		
		SendMessage(everything_hwnd,WM_COPYDATA,(WPARAM)_viv_hwnd,(LPARAM)&cds);
		
		mem_free(q);
	}
}

// x,y in screen coords
static void _viv_do_mousewheel_action(int action,int delta,int x,int y)
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
		high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_controls_high();
		
		pt.x = x;
		pt.y = y;
		
		ScreenToClient(_viv_hwnd,&pt);

		cursor_x = pt.x; 
		cursor_y = pt.y;
	
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
				for(_viv_zoom_pos = 0;_viv_zoom_pos<_VIV_ZOOM_MAX;_viv_zoom_pos++)
				{
					_viv_get_render_size(&rw,&rh);
					
					if (rw > old_rw)
					{
						break;
					}
				}
			}
			else
			{
				for(_viv_zoom_pos = _VIV_ZOOM_MAX-1;_viv_zoom_pos>=0;_viv_zoom_pos--)
				{
					_viv_get_render_size(&rw,&rh);
					
					if (rw < old_rw)
					{
						break;
					}
				}
			}
		}
		else
		{
			if (delta > 0)
			{
				_viv_zoom_pos++;
			}
			else
			{
				_viv_zoom_pos--;
			}
		}
		
		if (_viv_zoom_pos < 0) _viv_zoom_pos = 0;
		if (_viv_zoom_pos > _VIV_ZOOM_MAX-1) _viv_zoom_pos = _VIV_ZOOM_MAX-1;
		
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

static HBITMAP _viv_get_mipmap(HBITMAP hbitmap,int image_wide,int image_high,int render_wide,int render_high,int *pmip_wide,int *pmip_high,_viv_mipmap_t **out_mip)
{
	int mip_wide;
	int mip_high;
	_viv_mipmap_t **pmip;
	int depth;
	HBITMAP best_hbitmap;
	int best_wide;
	int best_high;

//	debug_printf("GETMIP %d x %d => %d x %d\n",image_wide,image_high,render_wide,render_high);
		
	mip_wide = (image_wide + 1) / 2;
	mip_high = (image_high + 1) / 2;
		
	if (mip_wide < 1)
	{
		mip_wide = 1;
	}
	
	if (mip_high < 1)
	{
		mip_high = 1;
	}
		
	if ((mip_wide == 1) && (mip_high == 1))
	{
		*pmip_wide = image_wide;
		*pmip_high = image_high;

//			debug_printf("MIP %d: %d %d\n",1,image_wide,image_high);

		return hbitmap;
	}
	
	if ((render_wide >= mip_wide) || (render_high >= mip_wide))
	{
//			debug_printf("MIP 0: %d %d\n",mip_wide,mip_high);
		*pmip_wide = image_wide;
		*pmip_high = image_high;

//			debug_printf("MIP %d: %d %d\n",1,image_wide,image_high);

		return hbitmap;
	}
	
	best_hbitmap = hbitmap;
	best_wide = image_wide;
	best_high = image_high;
	
	pmip = out_mip;
	
	depth = 1;
	
	for(;;)
	{
		if (!*pmip)
		{
			HDC screen_hdc;
			HDC mem_hdc;
			HDC mem2_hdc;
			HGDIOBJ last_hbitmap;
			HGDIOBJ last2_hbitmap;
			int last_stretch_mode;
			
			debug_printf("GETMIPMAP %d: %d %d\n",depth,mip_wide,mip_high);
			
			// create mipmap..
			*pmip = mem_alloc(sizeof(_viv_mipmap_t));
			
			screen_hdc = GetDC(0);
			if (screen_hdc)
			{
				mem_hdc = CreateCompatibleDC(screen_hdc);
				if (mem_hdc)
				{
					mem2_hdc = CreateCompatibleDC(screen_hdc);
					if (mem2_hdc)
					{
						(*pmip)->mipmap = NULL;
						(*pmip)->hbitmap = CreateCompatibleBitmap(screen_hdc,mip_wide,mip_high);
						
						last_hbitmap = SelectObject(mem_hdc,(*pmip)->hbitmap);
						last2_hbitmap = SelectObject(mem2_hdc,best_hbitmap);
						
						last_stretch_mode = SetStretchBltMode(mem_hdc,HALFTONE);
						
						StretchBlt(mem_hdc,0,0,mip_wide,mip_high,mem2_hdc,0,0,best_wide,best_high,SRCCOPY);

						SetStretchBltMode(mem_hdc,last_stretch_mode);
						
						SelectObject(mem_hdc,last_hbitmap);
						SelectObject(mem2_hdc,last2_hbitmap);

						DeleteDC(mem2_hdc);
					}
					
					DeleteDC(mem_hdc);
				}
				
				ReleaseDC(0,screen_hdc);
			}
		}

		best_wide = mip_wide;
		best_high = mip_high;
		best_hbitmap = (*pmip)->hbitmap;
		
		mip_wide = (image_wide + 1) / (2<<depth);
		mip_high = (image_high + 1) / (2<<depth);
		
		if (mip_wide < 1)
		{
			mip_wide = 1;
		}
		
		if (mip_high < 1)
		{
			mip_high = 1;
		}
		
		if ((mip_wide == 1) && (mip_high == 1))
		{
			*pmip_wide = best_wide;
			*pmip_high = best_high;
			
//				debug_printf("MIP %d: %d %d\n",depth,best_wide,best_high);
			
			return best_hbitmap;
		}
		
		if ((render_wide >= mip_wide) || (render_high >= mip_wide))
		{
			*pmip_wide = best_wide;
			*pmip_high = best_high;
			
//				debug_printf("MIP %d: %d %d\n",depth,best_wide,best_high);
			
			return best_hbitmap;
		}
		
		pmip = &(*pmip)->mipmap;
		depth++;
	}
}

static void _viv_preload_next(void)
{
	if (config_preload_next)
	{
		//UpdateWindow(_viv_hwnd);
		
		_viv_next(_viv_last_is_prev,0,1,0);
	}
}

static void _viv_start_first_frame(void)
{
	// draw first frame and start timer.
	_viv_frame_position = 0;
	_viv_frame_looped = 0;
	_viv_animation_timer_tick_start = os_get_tick_count();
	_viv_timer_tick = 0;
	
	if (_viv_frame_count > 1)
	{
		_viv_timer_start();
	}

	_viv_update_src_pixel(1,0);
	_viv_status_update();
	
	if (_viv_doing == _VIV_DOING_1TO1SCROLL)
	{
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(_viv_hwnd,&pt);
		
		_viv_update_1to1_scroll(pt.x,pt.y);
	}

	// show cursor.
	_viv_update_show_cursor();

//debug_printf("---\n",_viv_load_image_next_fd);
//debug_printf("_viv_load_image_next_fd %p\n",_viv_load_image_next_fd);
//debug_printf("---\n",_viv_load_image_next_fd);
/*
	if (_viv_load_image_next_fd)
	{
		// load priority paint..
		// we will paint on the 'next next' image we load..
		_viv_low_priority_paint = 1;
		return;
	}
	
	_viv_low_priority_paint = 0;
	InvalidateRect(_viv_hwnd,NULL,FALSE);

	// building mipmaps as needed hangs the UI.
	// this makes rendering lag while holding down right.
	// avoid painting when user is holding down right..
	//
	// because we build the mipmap for the first image in the load thread this is now instant..
	// ok it's still awful, let the next load refresh..
//	if (_viv_load_image_next_fd)
	{
//		UpdateWindow(_viv_hwnd);
	}*/
	
	if (!_viv_is_fullscreen)
	{
		if (config_auto_zoom)
		{
			switch(config_auto_zoom_type)
			{
				case 0:
				case 1:
				case 2:
				case 3:
					// don't scroll any regions when sizing.
					InvalidateRect(_viv_hwnd,NULL,FALSE);
					_viv_command(VIV_ID_VIEW_WINDOW_SIZE_50 + config_auto_zoom_type);
					break;
			}
		}
	}

	InvalidateRect(_viv_hwnd,NULL,FALSE);
	UpdateWindow(_viv_hwnd);
}			

// the preload has completed and we 
// want to set it as the current image.
static void _viv_activate_preload(void)
{
debug_printf("activate preload\n");

	_viv_clear();
	
	_viv_frames = _viv_preload_frames;
	_viv_preload_frames = NULL;
	
	_viv_frame_loaded_count = _viv_preload_frame_loaded_count;
	_viv_preload_frame_loaded_count = 0;
	
	_viv_frame_count = _viv_preload_frame_count;
	_viv_preload_frame_count = 0;
	
	_viv_image_wide = _viv_preload_image_wide;
	_viv_preload_image_wide = 0;
	
	_viv_image_high = _viv_preload_image_high;
	_viv_preload_image_high = 0;

	_viv_start_first_frame();

	_viv_process_pending_clear();
	
	os_copy_memory(_viv_frame_fd,_viv_preload_fd,sizeof(WIN32_FIND_DATA));

	_viv_clear_preload_frames();
	
	*_viv_preload_fd->cFileName = 0;
}

// copy the current image to the last image.
static void viv_copy_current_image_to_last_image(void)
{
	if (_viv_last_frames)
	{
		_viv_clear_frames(_viv_last_frames,_viv_last_frame_count);
		
		_viv_last_frames = NULL;
		_viv_last_frame_count = 0;
	}

debug_printf("*** Cache LAST : %S\n",_viv_frame_fd->cFileName);

	// only copy if the whole image was loaded.
	// Otherwise we need to reload the whole image again..
	if (config_cache_last)
	{
		if (_viv_frame_count == _viv_frame_loaded_count)
		{
			os_copy_memory(_viv_last_fd,_viv_frame_fd,sizeof(WIN32_FIND_DATA));
			
			_viv_last_image_wide = _viv_image_wide; // last image width
			_viv_last_image_high = _viv_image_high ; // last image width
			_viv_last_frame_count = _viv_frame_count; // last image frame count, 1 for static image, > 1 for animation (all frames are loaded)
			_viv_last_frames = _viv_frames;
			
			_viv_image_wide = 0;
			_viv_image_high = 0;
			_viv_frame_count = 0;
			_viv_frames = NULL;
			_viv_frame_fd->cFileName[0] = 0;
		}
	}
}

static void _viv_activate_last(void)
{
	WIN32_FIND_DATA old_fd;
	int old_image_wide;
	int old_image_high;
	int old_frame_count;
	_viv_frame_t *old_frames;
	
debug_printf("activate last\n");

	old_frames = NULL;

	// save current image so we can store it in last image later.
	// we can't do it now because we are setting the last image to the current image.
	if (config_cache_last)
	{
		if (_viv_frame_count == _viv_frame_loaded_count)
		{
debug_printf("*** Cache LAST2 : %S\n",_viv_frame_fd->cFileName);
		
			os_copy_memory(&old_fd,_viv_frame_fd,sizeof(WIN32_FIND_DATA));
			
			old_image_wide = _viv_image_wide; // last image width
			old_image_high = _viv_image_high ; // last image width
			old_frame_count = _viv_frame_count; // last image frame count, 1 for static image, > 1 for animation (all frames are loaded)
			old_frames = _viv_frames;
			
			_viv_image_wide = 0;
			_viv_image_high = 0;
			_viv_frame_count = 0;
			_viv_frames = NULL;
			_viv_frame_fd->cFileName[0] = 0;
		}
	}
	
	os_copy_memory(_viv_current_fd,_viv_last_fd,sizeof(WIN32_FIND_DATA));
	
	_viv_update_title();
	_viv_status_update();
	
	_viv_clear();
	
	_viv_frames = _viv_last_frames;
	os_copy_memory(_viv_frame_fd,_viv_last_fd,sizeof(WIN32_FIND_DATA));
	_viv_last_frames = NULL;
	
	_viv_frame_loaded_count = _viv_last_frame_count;
	_viv_frame_count = _viv_last_frame_count;
	_viv_last_frame_count = 0;
	
	_viv_image_wide = _viv_last_image_wide;
	_viv_last_image_wide = 0;
	
	_viv_image_high = _viv_last_image_high;
	_viv_last_image_high = 0;

	_viv_start_first_frame();

	_viv_process_pending_clear();
	
	if (old_frames)
	{
		os_copy_memory(_viv_last_fd,&old_fd,sizeof(WIN32_FIND_DATA));
		
		_viv_last_image_wide = old_image_wide;
		_viv_last_image_high = old_image_high;
		_viv_last_frame_count = old_frame_count;
		_viv_last_frames = old_frames;
	}
}

static void _viv_clear_last(void)
{
	if (_viv_last_frames)
	{
		_viv_clear_frames(_viv_last_frames,_viv_last_frame_count);
		
		_viv_last_frames = NULL;
		_viv_last_frame_count = 0;
	}
}

static void _viv_refresh(void)
{
	WIN32_FIND_DATA fd;
	
	os_copy_memory(&fd,_viv_current_fd,sizeof(WIN32_FIND_DATA));

	// clear last cache
	_viv_clear_last();

	// clear preload.
	_viv_clear_loading_preload();
	_viv_clear_preload();

	_viv_clear();
	_viv_start_first_frame();
	_viv_process_pending_clear();

	_viv_open(&fd,0);
}

static void _viv_show_cursor(void)
{
	if (_viv_is_hide_cursor_timer)
	{
		KillTimer(_viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER);
	
		_viv_is_hide_cursor_timer = 0;
	}

	if (!_viv_is_cursor_shown)
	{
		ShowCursor(TRUE);
		
		_viv_is_cursor_shown = 1;
	}
}

static void _viv_hide_cursor(void)
{
	if (_viv_is_hide_cursor_timer)
	{
		KillTimer(_viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER);
	
		_viv_is_hide_cursor_timer = 0;
	}

	if (_viv_is_cursor_shown)
	{
		ShowCursor(FALSE);
		
		_viv_is_cursor_shown = 0;
	}
}

static int _viv_should_show_cursor(void)
{
	if (!_viv_in_popup_menu)
	{
		if ((*_viv_current_fd->cFileName) && (!_viv_file_not_found) && (!_viv_load_failed))
		{
			if (GetForegroundWindow() == _viv_hwnd)
			{
				if (_viv_is_mouseover)
				{
					if (!GetCapture())
					{
						if ((_viv_is_fullscreen) || (config_windowed_hide_cursor))
						{
							// if (!((_viv_is_alt) && (_viv_is_tracking_mouse)))
							{
								return 0;
							}
						}
					}
				}
			}
		}
	}
			
	return 1;
}

static void _viv_update_show_cursor(void)
{
	if (_viv_should_show_cursor())
	{
		_viv_show_cursor();
	}
	else
	{
		_viv_start_hide_cursor_timer();
	}
}

static void _viv_start_hide_cursor_timer(void)
{
	if (!_viv_is_hide_cursor_timer)
	{
		SetTimer(_viv_hwnd,VIV_ID_HIDE_CURSOR_TIMER,_VIV_HIDE_CURSOR_DELAY,0);
		
		_viv_is_hide_cursor_timer = 1;
	}
}

static void _viv_get_exe_filename(wchar_t filename[STRING_SIZE])
{
	filename[0] = 0;
	
	GetModuleFileName(0,filename,STRING_SIZE);
	
	// make sure the drive letter is uppercase.
	// launching from .png == d:
	// launching from .exe == D:
	if ((*filename >= 'a') && (*filename <= 'z'))
	{
		if (filename[1] == ':')
		{
			*filename = *filename - 'a' + 'A';
		}
	}
}

// 0 = scroll, 1 = play/pause slideshow, 2 = play/pause animation, 3=zoom in, 4=next, 5=1:1 scroll, 6=move-window
static void _viv_do_left_click_action(int action)
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
				high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_controls_high();

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

static void _viv_start_move_window(void)
{
	POINT cursor_pt;
					
	GetCursorPos(&cursor_pt);
		
	SendMessage(_viv_hwnd,WM_NCLBUTTONDOWN,(WPARAM)HTCAPTION,MAKELPARAM(cursor_pt.x,cursor_pt.y));
}

static int _viv_ceil(double x) 
{
    int xi = (int)x;
    
    return (x > (double)xi) ? xi + 1 : xi;
}

static void _viv_center_listbox_item(HWND listbox_hwnd,int item_index)
{
    int item_high;
    
    item_high = ListBox_GetItemHeight(listbox_hwnd,item_index);
    
    if (item_high > 0)
    {
		RECT client_rect;
		int visible_count;
		
		GetClientRect(listbox_hwnd,&client_rect);

		visible_count = (client_rect.bottom - client_rect.top) / item_high;
		
		if (visible_count > 0)
		{
			int new_top;
			int item_count;
			
			item_count = ListBox_GetCount(listbox_hwnd);

			new_top = item_index - (visible_count / 2);
			
			if (new_top < 0)
			{
				new_top = 0;
			}

			if (new_top > item_count - visible_count)
			{
				new_top = item_count - visible_count;
			}
			
			if (new_top < 0)
			{
				new_top = 0;
			}

			ListBox_SetTopIndex(listbox_hwnd,new_top);
		}
	}
}											

// stretch a src-HDC with a selected bitmap to a distination-HDC.
// only stretch the specified clipping region (hopefully small from a scroll)
// this is way faster than StretchBlt as StretchBlt stretches the entire image, ignoring the clipping region.
static void _viv_stretch_blt(HDC dst_hdc,int dst_x,int dst_y,int dst_wide,int dst_high,HDC src_hdc,int src_wide,int src_high,int clip_x,int clip_y,int clip_wide,int clip_high)
{
	// clamp clip rectangle inside dst region
	if (clip_x < dst_x) 
	{
		clip_wide -= (dst_x - clip_x);
		clip_x = dst_x;
	}
	
	if (clip_y < dst_y) 
	{
		clip_high -= (dst_y - clip_y);
		clip_y = dst_y;
	}
	
	if (clip_x + clip_wide > dst_x + dst_wide) 
	{
		clip_wide = (dst_x + dst_wide) - clip_x;
	}
	
	if (clip_y + clip_high > dst_y + dst_high) 
	{
		clip_high = (dst_y + dst_high) - clip_y;
	}

	// something to do?
	if ((dst_wide > 0) && (dst_high > 0) && (clip_wide > 0) && (clip_high > 0))
	{
		int src_clip_x;
		int src_clip_y;
		int src_clip_wide;
		int src_clip_high;
		HDC dst_mem_hdc;
		
		src_clip_x = ((clip_x - dst_x) * (__int64)src_wide) / dst_wide;
		src_clip_y = ((clip_y - dst_y) * (__int64)src_high) / dst_high;
		src_clip_wide = (((clip_wide - 1 + clip_x - dst_x) * (__int64)src_wide) / dst_wide) + 1 - src_clip_x;
		src_clip_high = (((clip_high - 1 + clip_y - dst_y) * (__int64)src_high) / dst_high) + 1 - src_clip_y;
		
	//debug_printf("%d %d %d %d - %d %d %d %d\n",src_clip_x,src_clip_y,src_clip_wide,src_clip_high,clip_x,clip_y,clip_wide,clip_high);
			
		dst_mem_hdc = CreateCompatibleDC(dst_hdc);
		if (dst_mem_hdc)
		{
			HBITMAP dst_hbitmap;

			dst_hbitmap = CreateCompatibleBitmap(dst_hdc,clip_wide,clip_high);
			if (dst_hbitmap)
			{
				HGDIOBJ last_dst_hbitmap;

				last_dst_hbitmap = SelectObject(dst_mem_hdc,dst_hbitmap);
				
				// debug_printf("STRETCH %d %d %d %d - %d %d clip %d %d\n",dst_x-clip_x,dst_y-clip_y,dst_wide,dst_high,src_wide,src_high,clip_wide,clip_high);
//						StretchBlt(dst_hdc,dst_x,dst_y,dst_wide,dst_high,src_hdc,0,0,src_wide,src_high,SRCCOPY);
			
				SetStretchBltMode(dst_mem_hdc,GetStretchBltMode(dst_hdc));
			
				if (_viv_StretchBltStitch(dst_mem_hdc,dst_x-clip_x,dst_y-clip_y,dst_wide,dst_high,src_hdc,0,0,src_wide,src_high,SRCCOPY,0,0,clip_wide,clip_high))
				{
				}
				else
				{
					debug_printf("StretchBlt %d %d %d %d %d %d failed %u",dst_x-clip_x,dst_y-clip_y,dst_wide,dst_high,src_wide,src_high,GetLastError());
				}
				
				if (BitBlt(dst_hdc,clip_x,clip_y,clip_wide,clip_high,dst_mem_hdc,0,0,SRCCOPY))
				{
					
				}
				else
				{
					debug_printf("BitBlt %d %d %d %d failed %u",clip_x,clip_y,clip_wide,clip_high,GetLastError());
				}

				SelectObject(dst_mem_hdc,last_dst_hbitmap);
				
				// Cleanup
				DeleteObject(dst_hbitmap);
			}

			DeleteDC(dst_mem_hdc);
		}
	}
}

// stitch multiple 1024x1024 stretches together.
// required to render images over 32768x32768
// only works if using COLORONCOLOR
static BOOL _viv_StretchBltStitch(HDC hdcDest,int xDest,int yDest,int wDest,int hDest,HDC hdcSrc,int xSrc,int ySrc,int wSrc,int hSrc,DWORD rop,int clip_x,int clip_y,int clip_wide,int clip_high)
{
debug_printf("STRETCHBLT1024 %d %d %d %d SRC %d %d %d %d\n",xDest,yDest,wDest,hDest,xSrc,ySrc,wSrc,hSrc);

	if ((wDest < 32768) && (hDest < 32768) && (wSrc < 32768) && (hSrc < 32768))
	{
		return StretchBlt(hdcDest,xDest,yDest,wDest,hDest,hdcSrc,xSrc,ySrc,wSrc,hSrc,rop);
	}
	else
	if ((wDest>0) && (hDest>0) && (wSrc>0) && (hSrc>0))
	{
		int src_y;
		int src_yrun;
		int dst_y;
		int clip_right;
		int clip_bottom;
		
		dst_y = yDest;
		src_y = ySrc;
		src_yrun = hSrc;
		clip_right = clip_x + clip_wide;
		clip_bottom = clip_y + clip_high;
		
		while(src_yrun)
		{
			int src_high;
			int dst_y2;
			
			if (dst_y >= clip_bottom)
			{
				break;
			}
			
			src_high = src_yrun > _VIV_STRETCH_BLT_STITCH_SIZE ? _VIV_STRETCH_BLT_STITCH_SIZE : src_yrun;

			dst_y2 = (((src_y + src_high) * (__int64)hDest) / hSrc) + yDest;

			if (dst_y2 >= clip_y)
			{
				int src_x;
				int src_xrun;
				int dst_x;
				int dst_high;

				dst_x = xDest;
				dst_high = dst_y2 - dst_y;
				
				src_x = xSrc;
				src_xrun = wSrc;
				
				while(src_xrun)
				{
					int dst_x2;
					int dst_wide;
					int src_wide;
					
					src_wide = src_xrun > _VIV_STRETCH_BLT_STITCH_SIZE ? _VIV_STRETCH_BLT_STITCH_SIZE : src_xrun;
					
					dst_x2 = (((src_x + src_wide) * (__int64)wDest) / wSrc) + xDest;
					
					dst_wide = dst_x2 - dst_x;
					
					if (dst_x >= clip_right)
					{
						break;
					}

					if (dst_x2 > clip_x)
					{
	debug_printf("DST %d %d %d %d SRC %d %d %d %d CLIP %d %d\n",dst_x,dst_y,dst_wide,dst_high,src_x,src_y,src_wide,src_high,dst_x2,clip_x);
						if (!StretchBlt(hdcDest,dst_x,dst_y,dst_wide,dst_high,hdcSrc,src_x,src_y,src_wide,src_high,rop))
						{
							return FALSE;
						}
					}
					
					dst_x = dst_x2;
					src_x += src_wide;
					src_xrun -= src_wide;
				}
			}
			
			dst_y = dst_y2;
			src_y += src_high;
			src_yrun -= src_high;
		}
	}
	
	return TRUE;
}

// get the source pixel coordinate.
// doesn't use mipmaps.
static BOOL _viv_get_src_pixel_pos(int client_x,int client_y,POINT *out_pixel_pt)
{
	RECT client_rect;
	int wide;
	int high;
	int rx;
	int ry;
	int rw;
	int rh;
	
	GetClientRect(_viv_hwnd,&client_rect);
	wide = client_rect.right - client_rect.left;
	high = client_rect.bottom - client_rect.top - _viv_get_status_high() - _viv_get_controls_high();

	if (_viv_frame_count)
	{
		_viv_get_render_size(&rw,&rh);

		rw = (int)(rw * _viv_dst_zoom_values[_viv_dst_zoom_x_pos]);
		rh = (int)(rh * _viv_dst_zoom_values[_viv_dst_zoom_y_pos]);

		rx = (((_viv_dst_pos_x - 250) * (wide*2)) / 1000) - (rw / 2) - _viv_view_x;
		ry = (((_viv_dst_pos_y - 250) * (high*2)) / 1000) - (rh / 2) - _viv_view_y;
		
		if ((rw) && (rh))
		{
			if ((client_x >= rx) && (client_y >= ry) && (client_x < rx + rw) && (client_y < ry + rh))
			{
				out_pixel_pt->x = ((client_x - rx) * (__int64)_viv_image_wide) / rw;
				out_pixel_pt->y = ((client_y - ry) * (__int64)_viv_image_high) / rh;
				
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

// get the source pixel color from the specified position.
// doesn't use mipmaps.
static void _viv_get_src_pixel_rgb(int src_x,int src_y,COLORREF *out_colorref)
{
	RECT client_rect;
	int wide;
	int high;
	int rx;
	int ry;
	int rw;
	int rh;
	
	GetClientRect(_viv_hwnd,&client_rect);
	wide = client_rect.right - client_rect.left;
	high = client_rect.bottom - client_rect.top - _viv_get_status_high() - _viv_get_controls_high();

	if (_viv_frame_count)
	{
		HDC screen_hdc;
		
		screen_hdc = GetDC(0);
		
		if (screen_hdc)
		{
			HDC mem_hdc;
			
			_viv_get_render_size(&rw,&rh);

			rw = (int)(rw * _viv_dst_zoom_values[_viv_dst_zoom_x_pos]);
			rh = (int)(rh * _viv_dst_zoom_values[_viv_dst_zoom_y_pos]);

			rx = (((_viv_dst_pos_x - 250) * (wide*2)) / 1000) - (rw / 2) - _viv_view_x;
			ry = (((_viv_dst_pos_y - 250) * (high*2)) / 1000) - (rh / 2) - _viv_view_y;
			
			mem_hdc = CreateCompatibleDC(screen_hdc);
			if (mem_hdc)
			{
				HGDIOBJ last_hbitmap;
				
				last_hbitmap = SelectObject(mem_hdc,_viv_frames[_viv_frame_position].hbitmap);
				
				if (last_hbitmap)
				{
					*out_colorref = GetPixel(mem_hdc,src_x,src_y);
					
					SelectObject(mem_hdc,last_hbitmap);
				}
				
				DeleteDC(mem_hdc);
			}
			
			ReleaseDC(0,screen_hdc);
		}
	}
}

static int _viv_clamp_zoom_pos(int zoom_pos)
{
	if (zoom_pos > _VIV_ZOOM_MAX-1)
	{
		return _VIV_ZOOM_MAX-1;
	}

	if (zoom_pos < 0)
	{
		return 0;
	}
	
	return zoom_pos;
}

static void _viv_open_preload(void)
{
	os_copy_memory(_viv_current_fd,_viv_preload_fd,sizeof(WIN32_FIND_DATA));
		
	_viv_update_title();
	
	if (_viv_preload_state == 0)
	{
		// loading...
		// do we have the first frame?
		if (_viv_preload_frame_loaded_count)
		{
			// save current image to last image.
			viv_copy_current_image_to_last_image();

			// switch now..
			_viv_load_is_preload = 0;
			_viv_activate_preload();
			_viv_status_update();
		}
		else
		{
			// still loading...
			// wait...
			if (!_viv_should_activate_preload_on_load)
			{
				_viv_should_activate_preload_on_load = 1;
			
				// show loading...
				_viv_status_update();
			}
			
			debug_printf("STILL LOADING...\n");
		}
	}
	else
	if (_viv_preload_state == 1)
	{
		// save current image to last image.
		viv_copy_current_image_to_last_image();

		// loaded
		// preload next below..
		_viv_load_is_preload = 0;
		_viv_status_update();
		
		_viv_activate_preload();
		
		_viv_preload_next();
	}
	else
	{
		// save current image to last image.
		viv_copy_current_image_to_last_image();

		// failed.
		// preload next below..
		_viv_load_failed = 1;
		_viv_load_is_preload = 0;
		_viv_status_update();
		
		_viv_clear();
		
		_viv_start_first_frame();
		
		_viv_process_pending_clear();
		
		_viv_preload_next();
	}	
}

/*
static void _viv_get_tooltip(void)
{
	if (_viv_tooltip_hwnd)
	{
		return;
	}
	
	_viv_tooltip_hwnd = CreateWindowExA(
		WS_EX_TOPMOST|WS_EX_NOACTIVATE,
		(const utf8_t *)TOOLTIPS_CLASSA,
		(const utf8_t *)"",
		WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX | TTS_NOANIMATE | WS_GROUP,
		0,0,0,0,
		0,0,os_hinstance,0);
		
	SendMessage(_viv_tooltip_hwnd,TTM_SETDELAYTIME,TTDT_INITIAL,MAKELONG(0,0));
}

static void _viv_tooltip_hide(void)
{
	if (_viv_tooltip_hwnd)
	{
		DestroyWindow(_viv_tooltip_hwnd);
		
		_viv_tooltip_hwnd = 0;
	}
}

static void _viv_tooltip_update(void)
{	
	wchar_t pixel_info_buf[STRING_SIZE];

	_viv_get_tooltip();
	
	string_printf(pixel_info_buf,"%d,%d: %d,%d,%d",_viv_src_pixel_x,_viv_src_pixel_y,_viv_src_pixel_r,_viv_src_pixel_g,_viv_src_pixel_b);

	if (_viv_tooltip_hwnd)
	{
		TOOLINFO ti;
		DWORD message_id;
		
		os_zero_memory(&ti,sizeof(TOOLINFO));
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS | TTF_TRANSPARENT | TTF_IDISHWND;
		ti.hwnd = _viv_hwnd;
		ti.uId = (UINT_PTR)_viv_hwnd;
		
		if (SendMessage(_viv_tooltip_hwnd,TTM_GETTOOLINFO,0,(LPARAM)&ti))
		{
			message_id = TTM_UPDATETIPTEXTW;
		}
		else
		{
			message_id = TTM_ADDTOOLW;
		}

		os_zero_memory(&ti,sizeof(TOOLINFO));
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS | TTF_TRANSPARENT | TTF_IDISHWND;
		ti.hwnd = _viv_hwnd;
		ti.uId = (UINT_PTR)_viv_hwnd;
		ti.lpszText = pixel_info_buf;

		SendMessage(_viv_tooltip_hwnd,message_id,0,(LPARAM)&ti);
		SendMessage(_viv_tooltip_hwnd,TTM_TRACKACTIVATE,TRUE,(LPARAM)&ti);
		
		_viv_tooltip_update_track_position();
	}
}

static void _viv_tooltip_update_track_position(void)
{
	if (_viv_tooltip_hwnd)
	{
		POINT cursor_point;

		GetCursorPos(&cursor_point);
		
		SendMessage(_viv_tooltip_hwnd,TTM_TRACKPOSITION,0,(LPARAM)MAKELONG(cursor_point.x,cursor_point.y));
	}
}
*/