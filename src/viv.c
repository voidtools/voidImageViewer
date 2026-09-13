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
// right click rename 
// right click open with
// [HIGH] rotate images in memory only
// [HIGH] right click preview on windows xp fails with: the parameter is incorrect.
// [HIGH] right click O conflict options or sort?
// [HIGH] language_id needs to go in translation. we don't want to check for it in code.. has to be a list too, to support multiple language ids
// [HIGH] Help -> About viv -> Credits
// *[HIGH] viv_kill was refreshing the desktop window if no viv_hwnd was created.
// [HIGH] like mpc-hc, allow /add to add to the playlist (instead of clearing the playlist)
// [HIGH] like mpc-hc, allow *.jpg as the pathname.
// *[HIGH] like mpc-hc, if processing the command line again within 500ms, add the images to the playlist.
// [HIGH] playlist pane or tool window
// /close command line option like mpc-hc -how does mpc handle /playnext /close ? -/close needs to work with /slideshow (close after slideshow)
// vs2022 support
// use the sort order from Windows Explorer, from where the images was opened. -do we need another sort option (use default) or (use Windows Explorer sort)
// shrink blit mode=nearest doesn't work.
// option to not reset the zoom when the image changes.?
// .rc localization
// SVG support -Librsvg?
// AVIF
// GDI/GDI+ is limited to 2GB (wide*high*4), work out a workaround for large images.
// compile on mingw
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
//
// DONE:
// 1.0.0.0
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
// *In "1:1" mode, use shortcut keys to move a big picture(larger than the screen resolution), it will stop halfway. Perhaps it has something to do with the movement of the small images? -mpc does the whole image.
// 1.0.0.16
// *add UI option for config_title_bar_format. (by hesphoros)
// *option to show full path like MPC. (by hesphoros)
// *fix controls in view options page being 1 pixel too long
// *fix jump-to up/down focus
// *validate frame delay data
// *use rename collision resolution name
// *rename no to conflict should keep the rename dialog shown so user can continue with a different name.



// _VIV_STRETCH_BLT_STITCH_SIZE * 3.1 (pan+zoom) * 16 (zoom) MUST BE < 32768

#include "viv.h"
#include "viv_state.h"
#include "viv_recent.h"
#include "viv_playlist.h"
#include "viv_load.h"
#include "viv_anim.h"
#include "viv_render.h"
#include "viv_chrome.h"
#include "viv_menubar.h"
#include "viv_dark.h"
#include "viv_dialogs.h"
#include "viv_view.h"
#include "viv_install.h"
#include "viv_menu.h"
#include "viv_wndproc.h"
#include "viv_selfshot.h"

// the recent-files mru command ids run VIV_ID_FILE_RECENT_0 .. +count-1 and
// the menu builder emits ids straight off that base. this compile-time check
// locks the enum block and CONFIG_RECENT_FILE_COUNT together: raising one
// without the other fails the build instead of silently emitting command ids
// owned by unrelated entries.
typedef char _viv_recent_id_block_matches_count[(VIV_ID_FILE_RECENT_9 - VIV_ID_FILE_RECENT_0 + 1 == CONFIG_RECENT_FILE_COUNT) ? 1 : -1];


// theme change message and tooltip color messages. (not defined in older SDKs)

// per monitor dpi change message. (not defined in older SDKs)






// a file descriptor or find data
// to describe the image.







typedef struct _viv_default_key_s
{
	WORD command_id;
	WORD key_flags;
	
}_viv_default_key_t;




static int _viv_init(int nCmdShow);
void _viv_kill(void);
void _viv_exit(void);
static int _viv_is_msg(MSG *msg);
CLIPFORMAT _viv_get_CF_PREFERREDDROPEFFECT(void);

static void _viv_queue_clear(void);
static int _viv_main(int nCmdShow);

HMODULE _viv_stobject_hmodule = 0;
_viv_playlist_t *_viv_playlist_start = 0;
int _viv_playlist_count = 0;
_viv_playlist_t **_viv_playlist_shuffle_indexes = 0;
int _viv_playlist_shuffle_allocated = 0;
HWND _viv_hwnd = 0;
HWND _viv_status_hwnd = 0;

// the status pane texts. the panes are owner drawn (dark ui support):
// the control hands the pane index back in each WM_DRAWITEM item data
// and the text is drawn from this store. declared here: the status bar
// creation (_viv_status_show) flushes it long before _viv_status_set is
// reached in the file.
wchar_t _viv_status_part_text[_VIV_STATUS_PART_MAX][STRING_SIZE];
static HANDLE _viv_mutex = 0;
float _viv_animation_rates[] = {0.125000f,0.142857f,0.166667f,0.200000f,0.250000f,0.333333f,0.500000f,0.571429f,0.666667f,0.800000f,1.000000f,1.250000f,1.500000f,1.750000f,2.000000f,3.000000f,4.000000f,5.000000f,6.000000f,7.000000f,8.000000f}; // fixed animation rates
typedef char _viv_animation_rates_count_assert[(sizeof(_viv_animation_rates) / sizeof(float) == _VIV_ANIMATION_RATE_MAX) ? 1 : -1]; // the count literal pins the table
int _viv_animation_rate_pos = _VIV_ANIMATION_RATE_ONE;
BYTE _viv_animation_play = 1; // play or pause animations
BYTE _viv_1to1 = 0; // temporarily show the image with 100% scaling
BYTE _viv_have_old_zoom = 0; // restore this zoom level after leaving 1:1 mode.
WIN32_FIND_DATA *_viv_current_fd = 0; // the current image find data including the full path and filename.
WIN32_FIND_DATA *_viv_preload_fd = 0;
int _viv_view_x = 0; // the current image offset in pixels
int _viv_view_y = 0; // the current image offset in pixels
double _viv_view_ix = 0.0; // the current image offset in percent, used when resizing the window
double _viv_view_iy = 0.0; // the current image offset in percent, used when resizing the window
int _viv_zoom_pos = 0; // the current zoom level
// the zoom ladder is computed in _viv_init(): each step is a 1% multiplicative
// zoom increase (1.01x) from the best fit size (pos 0). the reachable top is
// measured at runtime (see _viv_zoom_pos_max): the first step that reaches
// the 16x size cap for the current image and window.
float _viv_zoom_scales[_VIV_ZOOM_MAX];

static ULONG_PTR os_GdiplusToken; // gdiplus handle
BYTE _viv_image_is_low_res = 0; // 1 = the displayed image is a progressive preview frame
int _viv_image_wide = 0; // current image width
int _viv_image_high = 0; // current image width
int _viv_frame_count = 0; // current image frame count, 1 for static image, > 1 for animation
int _viv_frame_loaded_count = 0; // number of loaded frames, can be less than _viv_frame_count
int _viv_frame_position = 0; // the current frame position
BYTE _viv_frame_looped = 0; // all frames have been displayed for this animation
BYTE _viv_is_slideshow_timeup = 0; // the slideshow timer has expired, but we are still showing an animation at least once.
_viv_frame_t *_viv_frames = 0; // the frames that make up an image, could be more than one for animations.
VIV_UINT64 _viv_timer_tick = 0; // the current tick for the current frame.
BYTE _viv_is_animation_timer = 0; // animation timer started?
VIV_UINT64 _viv_animation_timer_tick_start = 0; // the current start tick
BYTE _viv_doing = _VIV_DOING_NOTHING; // current mouse action, such as drag to scroll image
int _viv_doing_x;
int _viv_doing_y;
BYTE _viv_fullscreen_is_maxed = 0;
BYTE _viv_is_fullscreen = 0;
BYTE _viv_is_slideshow = 0;
BYTE _viv_is_hide_cursor_timer = 0;
static CLIPFORMAT _viv_CF_PREFERREDDROPEFFECT = 0; // copy or move? clipboard operation
BYTE _viv_is_cursor_shown = 1;
int _viv_mousemove_x = -1;
int _viv_mousemove_y = -1;
BYTE _viv_in_popup_menu = 0;
HMENU _viv_hmenu = 0;
int _viv_dst_pos_x = 500;
int _viv_dst_pos_y = 500;
CRITICAL_SECTION _viv_cs;
HANDLE _viv_load_image_thread = 0;
BYTE _viv_load_is_preload = 0;
wchar_t *_viv_load_image_filename = 0;
WIN32_FIND_DATA *_viv_load_image_next_fd = NULL;
BYTE _viv_load_image_next_is_preload = 0;
volatile int _viv_load_image_terminate = 0;
_viv_reply_t *_viv_reply_start = 0;
_viv_reply_t *_viv_reply_last = 0;
wchar_t *_viv_status_temp_text = 0;
int _viv_options_page_ids[] = {VIV_ID_OPTIONS_GENERAL,VIV_ID_OPTIONS_VIEW,VIV_ID_OPTIONS_CONTROLS};
HFONT _viv_about_hfont = 0;
wchar_t *_viv_last_open_file = 0;
wchar_t *_viv_last_open_folder = 0;
_viv_nav_item_t **_viv_nav_items = 0;
_viv_nav_item_t *__viv_nav_item_start = 0;
int _viv_nav_item_count = 0;
wchar_t *_viv_random = 0; // temp shuffle.
DWORD _viv_random_tot_results = 0xffffffff;
BYTE _viv_is_animation_timer_event = 0;
BYTE _viv_preload_state = 0; // 0 = loading, 1=complete, 2=failed
int _viv_preload_image_wide = 0; // current image width
int _viv_preload_image_high = 0; // current image height
int _viv_preload_frame_count = 0; // current image frame count, 1 for static image, > 1 for animation
int _viv_preload_frame_loaded_count = 0; // number of loaded frames, can be less than _viv_frame_count
_viv_frame_t *_viv_preload_frames = 0; // the frames that make up an image, could be more than one for animations.
BYTE _viv_last_is_prev = 0; // preload next or previous?
BYTE _viv_should_activate_preload_on_load = 0;
int _viv_load_render_wide = 0;
int _viv_load_render_high = 0;
WIN32_FIND_DATA *_viv_last_fd = 0; // the last find data including the full path and filename.
WIN32_FIND_DATA *_viv_frame_fd = 0; // the frame fd, may differ to the current fd because we change the title before the frames are loaded.
WIN32_FIND_DATA *_viv_load_fd = 0; // the load fd
int _viv_last_frame_count = 0; // last image frame count, 1 for static image, > 1 for animation (all frames are loaded)
_viv_frame_t *_viv_last_frames = 0; // the last frames that make up an image, could be more than one for animations.
BYTE _viv_load_image_allow_draw = 0; // allow the image to show if it loaded successfully after the load was terminated.
BYTE _viv_file_not_found = 0; // the current file was not found. -filename is shown in the window caption.
BYTE _viv_load_failed = 0; // the current file failed to load. -filename is shown in the window caption.
BYTE _viv_is_tracking_mouse = 0; // currently tracking mouse movement over our window.
BYTE _viv_is_mouseover = 0; // mouse is currently over our window.
BYTE _viv_prevent_on_deactivate = 0; // prevent handling of deactivate in WM_ACTIVATE.
int _viv_load_frame_count = 0; // the number of frames loaded, while loading.
int _viv_src_pixel_x = -1;
int _viv_src_pixel_y = -1;
BYTE _viv_src_pixel_r = 0;
BYTE _viv_src_pixel_g = 0;
BYTE _viv_src_pixel_b = 0;
static DWORD last_process_command_line_tick;
static BYTE got_last_process_command_line_tick = 0;

//static BYTE _viv_is_alt = 0;


// MF_OWNERDRAW = don't show in menu.
_viv_command_t _viv_commands[] = 
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
	{LOCALIZATION_ID_SAVE_AS,MF_STRING,_VIV_MENU_FILE,VIV_ID_FILE_SAVE_AS},
	{LOCALIZATION_ID_CLOSE,MF_STRING|MF_OWNERDRAW,_VIV_MENU_FILE,VIV_ID_FILE_CLOSE}, // this just clears the image.
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_FILE,0},
	{LOCALIZATION_ID_DELETE,MF_STRING|MF_DELETE,_VIV_MENU_FILE,VIV_ID_FILE_DELETE},
	{LOCALIZATION_ID_DELETE_RECYCLE,MF_STRING|MF_OWNERDRAW,_VIV_MENU_FILE,VIV_ID_FILE_DELETE_RECYCLE},
	{LOCALIZATION_ID_DELETE_PERMANENTLY,MF_STRING|MF_OWNERDRAW,_VIV_MENU_FILE,VIV_ID_FILE_DELETE_PERMANENTLY},
	{LOCALIZATION_ID_RENAME,MF_STRING,_VIV_MENU_FILE,VIV_ID_FILE_RENAME},
	{LOCALIZATION_ID_PROPERTIES,MF_STRING,_VIV_MENU_FILE,VIV_ID_FILE_PROPERTIES},
	{LOCALIZATION_ID_OPTIONS,MF_STRING,_VIV_MENU_FILE,VIV_ID_VIEW_OPTIONS},
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
	
	{LOCALIZATION_ID_LAYOUT,MF_POPUP,_VIV_MENU_VIEW,_VIV_MENU_VIEW_LAYOUT},
	{LOCALIZATION_ID_CAPTION,MF_STRING,_VIV_MENU_VIEW_LAYOUT,VIV_ID_VIEW_CAPTION},
	{LOCALIZATION_ID_FRAME,MF_STRING,_VIV_MENU_VIEW_LAYOUT,VIV_ID_VIEW_THICKFRAME},
	{LOCALIZATION_ID_MENU,MF_STRING,_VIV_MENU_VIEW_LAYOUT,VIV_ID_VIEW_MENU},
	{LOCALIZATION_ID_STATUS_BAR,MF_STRING,_VIV_MENU_VIEW_LAYOUT,VIV_ID_VIEW_STATUS},
	{LOCALIZATION_ID_CONTROLS,MF_STRING,_VIV_MENU_VIEW_LAYOUT,VIV_ID_VIEW_CONTROLS},
	{LOCALIZATION_ID_ZOOM_CONTROLS,MF_STRING,_VIV_MENU_VIEW_LAYOUT,VIV_ID_VIEW_ZOOM_CONTROLS},
	{LOCALIZATION_ID_ZOOM_AUTO_HIDE,MF_STRING,_VIV_MENU_VIEW_LAYOUT,VIV_ID_VIEW_ZOOM_AUTO_HIDE},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_VIEW_LAYOUT,0},
	{LOCALIZATION_ID_PRESET,MF_POPUP,_VIV_MENU_VIEW_LAYOUT,_VIV_MENU_VIEW_PRESET},
	{LOCALIZATION_ID_MINIMAL,MF_STRING,_VIV_MENU_VIEW_PRESET,VIV_ID_VIEW_PRESET_1},
	{LOCALIZATION_ID_COMPACT,MF_STRING,_VIV_MENU_VIEW_PRESET,VIV_ID_VIEW_PRESET_2},
	{LOCALIZATION_ID_NORMAL,MF_STRING,_VIV_MENU_VIEW_PRESET,VIV_ID_VIEW_PRESET_3},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_VIEW,0},
	{LOCALIZATION_ID_FULLSCREEN,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_FULLSCREEN},
	{LOCALIZATION_ID_SLIDESHOW,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_SLIDESHOW},
	{LOCALIZATION_ID_REFRESH,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_REFRESH},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_VIEW,0},
	{LOCALIZATION_ID_VIEW_WINDOW_SIZE,MF_POPUP,_VIV_MENU_VIEW,_VIV_MENU_VIEW_WINDOW_SIZE},
	{LOCALIZATION_ID_VIEW_WINDOW_SIZE_50_PERCENT,MF_STRING,_VIV_MENU_VIEW_WINDOW_SIZE,VIV_ID_VIEW_WINDOW_SIZE_50},
	{LOCALIZATION_ID_VIEW_WINDOW_SIZE_100_PERCENT,MF_STRING,_VIV_MENU_VIEW_WINDOW_SIZE,VIV_ID_VIEW_WINDOW_SIZE_100},
	{LOCALIZATION_ID_VIEW_WINDOW_SIZE_200_PERCENT,MF_STRING,_VIV_MENU_VIEW_WINDOW_SIZE,VIV_ID_VIEW_WINDOW_SIZE_200},
	{LOCALIZATION_ID_VIEW_WINDOW_SIZE_AUTO_FIT,MF_STRING,_VIV_MENU_VIEW_WINDOW_SIZE,VIV_ID_VIEW_WINDOW_SIZE_AUTO_FIT},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_VIEW,0},
	// all zoom related commands live in one submenu.
	{LOCALIZATION_ID_ZOOM,MF_POPUP,_VIV_MENU_VIEW,_VIV_MENU_VIEW_ZOOM},
	{LOCALIZATION_ID_ZOOM_IN,MF_STRING,_VIV_MENU_VIEW_ZOOM,VIV_ID_VIEW_ZOOM_IN},
	{LOCALIZATION_ID_ZOOM_OUT,MF_STRING,_VIV_MENU_VIEW_ZOOM,VIV_ID_VIEW_ZOOM_OUT},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_VIEW_ZOOM,0},
	{LOCALIZATION_ID_ONE_TO_ONE,MF_STRING,_VIV_MENU_VIEW_ZOOM,VIV_ID_VIEW_1TO1},
	{LOCALIZATION_ID_BEST_FIT,MF_STRING,_VIV_MENU_VIEW_ZOOM,VIV_ID_VIEW_BESTFIT},
	{LOCALIZATION_ID_RESET,MF_STRING,_VIV_MENU_VIEW_ZOOM,VIV_ID_VIEW_ZOOM_RESET},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_VIEW,0},
	{LOCALIZATION_ID_ALLOW_SHRINKING,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_ALLOW_SHRINKING},
	{LOCALIZATION_ID_KEEP_ASPECT_RATIO,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_KEEP_ASPECT_RATIO},
	{LOCALIZATION_ID_FILL_WINDOW,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_FILL_WINDOW},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_VIEW,0},
	{LOCALIZATION_ID_ON_TOP,MF_POPUP,_VIV_MENU_VIEW,_VIV_MENU_VIEW_ONTOP},
	{LOCALIZATION_ID_ALWAYS,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_VIEW_ONTOP,VIV_ID_VIEW_ONTOP_ALWAYS},
	{LOCALIZATION_ID_WHILE_PLAYING_OR_ANIMATING,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_VIEW_ONTOP,VIV_ID_VIEW_ONTOP_WHILE_PLAYING_OR_ANIMATING},
	{LOCALIZATION_ID_NEVER,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_VIEW_ONTOP,VIV_ID_VIEW_ONTOP_NEVER},
	{LOCALIZATION_ID_INVALID,MF_SEPARATOR,_VIV_MENU_VIEW,0},
	{LOCALIZATION_ID_WINDOWED_BACKGROUND_COLOR_MENU,MF_STRING,_VIV_MENU_VIEW,VIV_ID_VIEW_WINDOWED_BACKGROUND_COLOR},
	{LOCALIZATION_ID_BACKDROP,MF_POPUP,_VIV_MENU_VIEW,_VIV_MENU_VIEW_BACKDROP},
	{LOCALIZATION_ID_BACKDROP_FOLLOW,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_VIEW_BACKDROP,VIV_ID_VIEW_BACKDROP_FOLLOW},
	{LOCALIZATION_ID_BACKDROP_BLACK,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_VIEW_BACKDROP,VIV_ID_VIEW_BACKDROP_BLACK},
	{LOCALIZATION_ID_BACKDROP_WHITE,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_VIEW_BACKDROP,VIV_ID_VIEW_BACKDROP_WHITE},
	{LOCALIZATION_ID_BACKDROP_CUSTOM,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_VIEW_BACKDROP,VIV_ID_VIEW_BACKDROP_CUSTOM},
	{LOCALIZATION_ID_BACKDROP_CHECKERBOARD,MF_STRING|MFT_RADIOCHECK,_VIV_MENU_VIEW_BACKDROP,VIV_ID_VIEW_BACKDROP_CHECKERBOARD},

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
typedef char _viv_commands_count_assert[(sizeof(_viv_commands) / sizeof(_viv_command_t) == _VIV_COMMAND_COUNT) ? 1 : -1]; // the count literal pins the table


_viv_default_key_t _viv_default_keys[] =
{
	{VIV_ID_FILE_OPEN_FILE,CONFIG_KEYFLAG_CTRL | 'O'},
	{VIV_ID_FILE_OPEN_FOLDER,CONFIG_KEYFLAG_CTRL | 'B'},
	{VIV_ID_FILE_OPEN_EVERYTHING_SEARCH,CONFIG_KEYFLAG_CTRL | 'E'},
	{VIV_ID_FILE_ADD_FILE,CONFIG_KEYFLAG_CTRL | CONFIG_KEYFLAG_SHIFT | 'O'},
	{VIV_ID_FILE_ADD_FOLDER,CONFIG_KEYFLAG_CTRL | CONFIG_KEYFLAG_SHIFT | 'B'},
	{VIV_ID_FILE_ADD_EVERYTHING_SEARCH,CONFIG_KEYFLAG_CTRL | CONFIG_KEYFLAG_ALT | 'E'},
	{VIV_ID_FILE_OPEN_FILE_LOCATION,CONFIG_KEYFLAG_CTRL | VK_RETURN},
	{VIV_ID_FILE_SAVE_AS,CONFIG_KEYFLAG_CTRL | 'S'},
	{VIV_ID_FILE_EDIT,CONFIG_KEYFLAG_CTRL | CONFIG_KEYFLAG_SHIFT | 'E'}, // the everything search keeps plain ctrl+e; everything-add moved to ctrl+alt+e so the editor can take ctrl+shift+e
	{VIV_ID_FILE_PRINT,CONFIG_KEYFLAG_CTRL | 'P'},
	{VIV_ID_FILE_SET_DESKTOP_WALLPAPER,CONFIG_KEYFLAG_CTRL | 'D'}, // safe again: the handler asks before it touches the desktop
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
	{VIV_ID_VIEW_ZOOM_IN,VK_OEM_PLUS},
	{VIV_ID_VIEW_ZOOM_IN,VK_ADD},
	{VIV_ID_VIEW_ZOOM_IN,CONFIG_KEYFLAG_CTRL | VK_ADD},
	{VIV_ID_VIEW_ZOOM_OUT,VK_OEM_MINUS},
	{VIV_ID_VIEW_ZOOM_OUT,VK_SUBTRACT},
	{VIV_ID_VIEW_ZOOM_OUT,CONFIG_KEYFLAG_CTRL | VK_SUBTRACT},
	{VIV_ID_VIEW_ZOOM_RESET,CONFIG_KEYFLAG_CTRL | '0'},
	{VIV_ID_VIEW_ONTOP_ALWAYS,CONFIG_KEYFLAG_CTRL | 'T'},
	{VIV_ID_VIEW_OPTIONS,CONFIG_KEYFLAG_CTRL | VK_OEM_COMMA}, // ctrl+comma, the modern options key: a bare o opened a modal on every stray keypress
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
	"emf",
	"wmf",
};
typedef char _viv_association_extensions_count_assert[(sizeof(_viv_association_extensions) / sizeof(_viv_association_extensions[0]) == _VIV_ASSOCIATION_COUNT) ? 1 : -1]; // the count literal pins the table

// registry description.
const localization_id_t _viv_association_description_localization_id_array[] = 
{
	LOCALIZATION_ID_ASSOCIATION_DESCRIPTION_BMP,
	LOCALIZATION_ID_ASSOCIATION_DESCRIPTION_GIF,
	LOCALIZATION_ID_ASSOCIATION_DESCRIPTION_ICO,
	LOCALIZATION_ID_ASSOCIATION_DESCRIPTION_JPEG,
	LOCALIZATION_ID_ASSOCIATION_DESCRIPTION_JPG,
	LOCALIZATION_ID_ASSOCIATION_DESCRIPTION_PNG,
	LOCALIZATION_ID_ASSOCIATION_DESCRIPTION_TIF,
	LOCALIZATION_ID_ASSOCIATION_DESCRIPTION_TIFF,
	LOCALIZATION_ID_ASSOCIATION_DESCRIPTION_WEBP,
	LOCALIZATION_ID_ASSOCIATION_DESCRIPTION_EMF,
	LOCALIZATION_ID_ASSOCIATION_DESCRIPTION_WMF,
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
	NULL,
	NULL,
};



#ifdef VERSION_X86

// load unicode for windows 95/98
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

#endif










// the deferred recent-files save. every mru mutation used to write the
// whole settings file (create + write + replace, three file system ops)
// and rebuild the entire menu bar on the ui thread right before the image
// load was dispatched - the visible stutter when opening files, especially
// with real-time antivirus scanning the temp file. the write is now
// debounced: the timer coalesces a burst of opens into one save, and the
// exit / endsession saves fold any still-pending write in.
int _viv_recent_save_dirty = 0;











void _viv_exit(void)
{
	_viv_load_image_terminate = 1;
	
	// the deferred recent-files save folds into the exit write below (the
	// debounce timer never gets to fire once the quit is posted).
	_viv_recent_save_fold();
	
	config_save_settings(config_appdata);
	PostQuitMessage(0);
}

// cached backbuffer used for double buffered painting.
HDC _viv_paint_hdc = 0;

HBRUSH _viv_dialog_dark_hbrush = 0; // dark dialog background brush, lazy created
// a cached dark chrome brush for the toolbar strip. which: 0 = the strip
// face (0x252525, one step above the canvas), 1 = the separator shadow
// line (0x454545), 2 = the separator highlight line (0x707070),
// 3 = the menu bar face (0x202020: the canvas and dark system menu
// color). the zoom bar uses the same palette. created lazily, released
// in _viv_kill with the other cached brushes.
HBRUSH _viv_dark_chrome_hbrushes[4];


// a cached light chrome brush for the toolbar strip lines. the light strip
// face is the system menu color (it matches the light menu bar exactly and
// tracks the theme for free), but the flat separator lines are fixed subtle
// grays like the dark palette: 0 = the shadow line, 1 = the highlight line.
// released in _viv_kill with the dark chrome brushes.


HBRUSH _viv_about_light_hbrushes[2];


HBRUSH _viv_backdrop_solid_hbrush = 0; // backdrop solid color brush, cached
COLORREF _viv_backdrop_solid_color = 0; // the color the solid brush was created with
HBRUSH _viv_backdrop_checker_hbrush = 0; // checkerboard pattern brush, cached
HBITMAP _viv_backdrop_checker_hbitmap = 0; // the pattern bitmap (owned while the brush lives)
int _viv_backdrop_checker_cell = 0; // the cell size the pattern was built with


static void _viv_apply_config_language(void)
{
	// apply the language setting from the config file.
	// config_language: 0 = auto (keep the detected system language), 1 = english, 2 = simplified chinese.
	
	if (config_language == 1)
	{
		localization_set_language(LOCALIZATION_LANGUAGE_ENGLISH);
	}
	else
	if (config_language == 2)
	{
		localization_set_language(LOCALIZATION_LANGUAGE_CHINESE_SIMPLIFIED);
	}
}


void _viv_process_command_line(wchar_t *cl)
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
	int is_add;

	debug_printf("cl: %S\n",cl);
	
	start_slideshow = 0;
	start_fullscreen = 0;
	start_window = 0;
	start_maximized = 0;
	is_add = 0;
	p = cl;
	file_count = 0;
	single[0] = 0;
	set_window_rect = 0;

	GetWindowRect(_viv_hwnd,&rect);
	window_x = rect.left;
	window_y = rect.top;
	window_wide = rect.right - rect.left;
	window_high = rect.bottom - rect.top;
	
	if (config_add_command_line_timeout)
	{
		if (got_last_process_command_line_tick)
		{
			if ((GetTickCount() - last_process_command_line_tick) < (DWORD)config_add_command_line_timeout)
			{
				// don't add if nothing is loaded.
				if (*_viv_current_fd->cFileName)
				{
					is_add = 1;
				}
			}
		}
	}
	
	p = string_skip_ws(p);
	
	// skip exe filename
	p = string_get_word(p,buf,STRING_SIZE);
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

		p = string_get_word(p,buf,STRING_SIZE);
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
				p = string_get_word(p,buf,STRING_SIZE);
				p = string_skip_ws(p);
				
				_viv_send_everything_search(0,0,0,buf);
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"random") == 0)
			{
				p = string_get_word(p,buf,STRING_SIZE);
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
				p = string_get_word(p,buf,STRING_SIZE);
				p = string_skip_ws(p);
				
				window_x = string_to_int(buf);
				set_window_rect = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"y") == 0)
			{
				p = string_get_word(p,buf,STRING_SIZE);
				p = string_skip_ws(p);
				
				window_y = string_to_int(buf);
				set_window_rect = 1;
			}		
			else
			if (string_icompare_lowercase_ascii(bufstart,"width") == 0)
			{
				p = string_get_word(p,buf,STRING_SIZE);
				p = string_skip_ws(p);
				
				window_wide = string_to_int(buf);
				set_window_rect = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"height") == 0)
			{
				p = string_get_word(p,buf,STRING_SIZE);
				p = string_skip_ws(p);
				
				window_high = string_to_int(buf);
				set_window_rect = 1;
			}		
			else
			if (string_icompare_lowercase_ascii(bufstart,"rate") == 0)
			{
				p = string_get_word(p,buf,STRING_SIZE);
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
			if (string_icompare_lowercase_ascii(bufstart,"add") == 0)
			{
				// don't add if nothing is loaded.
				if (*_viv_current_fd->cFileName)
				{
					is_add = 1;
				}
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
					
					if (!is_add)
					{
						_viv_playlist_clearall();
					}
				}
				
				if (file_count == 1)
				{
					// add the last single filename.
					// we have two modes:
					// 1) playlist mode. (navigate the playlist)
					// 2) single file mode. (navigate the folder from the single file)
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

	// add the single image filename to the playlist if we are adding.
	if (is_add)
	{
		// if the playlist is empty, add the current item.
		if (!_viv_playlist_start)
		{
			if (*_viv_current_fd->cFileName)
			{
				_viv_playlist_add_filename(_viv_current_fd->cFileName);	
			}
		}
	
		if (file_count == 1)
		{
			_viv_playlist_add_filename(single);
		}
	}
	
	debug_printf("file count %d\n",file_count);

	// show the first image.
	// don't show anything if we are adding.
	// show something if nothing is already shown.
	if (!is_add)
	{
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
	
	// save last processing time
	// if we make another immediate call, add items to the playlist, instead of setting the playlist.
	last_process_command_line_tick = GetTickCount();
	got_last_process_command_line_tick = 1;
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
	
	// init the zoom ladder: a geometric 1.01x per step. the ladder is long
	// enough that the 16x size cap is reachable even for photos much larger
	// than the window.
	{
		int i;
		double f;
		
		f = 1.0;
		
		for(i=0;i<_VIV_ZOOM_MAX;i++)
		{
			_viv_zoom_scales[i] = (float)f;
			
			f *= 1.01;
		}
	}
	
	debug_printf("viv %s%s (build %d) %s\n",VERSION_STRING,VERSION_TYPE,VERSION_BUILD,VERSION_TARGET_MACHINE);
	
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
	
	// apply the language setting (config can override the system language).
	_viv_apply_config_language();
	
	// set the menu theme (light/dark) before any menu or window is created,
	// so the dark mode applies from the very first draw.
	os_dark_set_app_mode(config_dark_mode);
	
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
				size = (int)safe_size_add(safe_size_add(sizeof(DWORD),safe_size_mul_sizeof_wchar(safe_size_add_one(string_get_length(command_line)))),safe_size_mul_sizeof_wchar(safe_size_add_one(string_get_length(cwd))));
				buf = (char *)mem_alloc(size);
				
				// fill in
				d = buf;
				*(DWORD *)d = si.wShowWindow;
				d += sizeof(DWORD);
				string_copy((wchar_t *)d,command_line);
				d += ((string_get_length(command_line) + 1) * sizeof(wchar_t));
				string_copy((wchar_t *)d,cwd);
				d += ((string_get_length(cwd) + 1) * sizeof(wchar_t));

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
		0,NULL,os_hinstance,NULL);
	
	if ((!config_show_caption) || (!config_show_thickframe))
	{
		_viv_update_frame();
	}
		
	os_make_rect_completely_visible(_viv_hwnd,&rect);
		
	SetWindowPos(_viv_hwnd,0,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOZORDER|SWP_NOACTIVATE);
	
	// the top bar is a client side child now: show it before the first
	// layout sweep (the frame menu is gone for good).
	_viv_menubar_show(config_show_menu);

	// allow non-admin/admins to close this window.
	if (os_ChangeWindowMessageFilterEx)
	{
		// MSGFLT_ALLOW = 1
		os_ChangeWindowMessageFilterEx(_viv_hwnd,WM_CLOSE,1,0);
		
		// theme change broadcasts come from unelevated system processes:
		// allow them through the uipi filter so an elevated viewer still
		// follows the windows theme live.
		os_ChangeWindowMessageFilterEx(_viv_hwnd,WM_SETTINGCHANGE,1,0);
		os_ChangeWindowMessageFilterEx(_viv_hwnd,WM_THEMECHANGED,1,0);
	}
		
	_viv_status_show(config_show_status);
	_viv_controls_show(config_show_controls);
	_viv_zoomui_update();
	
	// apply the dark chrome (title bar, status bar, zoom controls) to the
	// freshly created window.
	_viv_apply_dark_mode(0);
	
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

void _viv_kill(void)
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
		// the wait is bounded: a decoder wedged mid-decode must not make exit
		// impossible. (the terminate flag is checked between decode steps, so
		// the timeout below only fires on a truly stuck decoder.)
		if (WaitForSingleObject(_viv_load_image_thread,10000) != WAIT_OBJECT_0)
		{
			// stop the thread where it stands. it is no longer safe for the
			// rest of the teardown to share memory with it. the process is
			// exiting: anything the thread leaked is reclaimed by the OS.
			TerminateThread(_viv_load_image_thread,1);
			
			WaitForSingleObject(_viv_load_image_thread,1000);
		}
		
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

	if (_viv_hwnd)
	{
		_viv_status_show(0);
		_viv_controls_show(0);
	//	_viv_tooltip_hide();

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
	
	
	if (_viv_background_hbrush)
	{
		DeleteObject(_viv_background_hbrush);
		
		_viv_background_hbrush = 0;
	}
	
	if (_viv_dialog_dark_hbrush)
	{
		DeleteObject(_viv_dialog_dark_hbrush);
		
		_viv_dialog_dark_hbrush = 0;
	}
	
	{
		int i;
		
				for(i=0;i<2;i++)
		{
			if (_viv_about_light_hbrushes[i])
			{
				DeleteObject(_viv_about_light_hbrushes[i]);
				
				_viv_about_light_hbrushes[i] = 0;
			}
		}
		
for(i=0;i<4;i++)
		{
			if (_viv_dark_chrome_hbrushes[i])
			{
				DeleteObject(_viv_dark_chrome_hbrushes[i]);
				
				_viv_dark_chrome_hbrushes[i] = 0;
			}
		}
	}
	
	if (_viv_backdrop_solid_hbrush)
	{
		DeleteObject(_viv_backdrop_solid_hbrush);
		
		_viv_backdrop_solid_hbrush = 0;
		_viv_backdrop_solid_color = 0;
	}
	
	if (_viv_backdrop_checker_hbrush)
	{
		DeleteObject(_viv_backdrop_checker_hbrush);
		
		_viv_backdrop_checker_hbrush = 0;
	}
	
	if (_viv_backdrop_checker_hbitmap)
	{
		DeleteObject(_viv_backdrop_checker_hbitmap);
		
		_viv_backdrop_checker_hbitmap = 0;
		_viv_backdrop_checker_cell = 0;
	}
	
	_viv_paint_kill();
	
	_viv_key_clear_all(_viv_key_list);
	mem_free(_viv_key_list);
	
	// the remake domains: the settings window dies with the owner, the
	// menubar drops its cached hover and press brushes.
	_viv_settings_kill();
	_viv_menubar_kill();

	zoomui_kill();

	os_kill();

#ifdef _DEBUG
	mem_debug();
#endif	
}

static int _viv_main(int nCmdShow)
{
	if (_viv_init(nCmdShow))
	{
#ifdef VIVP_SELF_SHOT
		vivp_selfshot_init();
#endif
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

					// key presses are overlay activity (idle fade timer).
					zoomui_activity();

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














// the dialog font: the system message font at the dialog window own
// dpi, one handle per dialog (the template face only sized the dlu
// grid at creation; the message font is the family the menu bar and
// the status bar draw, so the locale text renders with real glyphs
// instead of the fallback the hard coded segoe ui forced). the handle
// belongs to the dialog that adopted it: the options container and
// its create dialog pages coexist and jumpto is a create dialog too,
// so no broadcast (a settings change, a theme flip) may delete a
// face some open dialog is still drawing with - the font dies with
// the dialog, at the ncdestroy after its children are gone, and a
// re-apply replaces the old face only inside the same message.











// the backdrop shown under transparent pixels.





// dark dialog support: the common dialogs (options and its pages, about,
// rename, edit key, custom rate and the everything search) get the dark
// chrome when the dark ui is active. the app mode is already dark app wide
// (the comctl controls draw dark), so what is missing is the dialog title
// bar, the control visual style, the control color replies and the
// background fill.



// the property that marks the controls this module flipped to owner
// drawn (the flip is undone when the ui goes back to light).








// the property that caches the button theme handle on a dark dialog:
// uxtheme handles are not free, and the notify path used to pay an open
// and a close pair on every single paint. the handle opens lazily on the
// first dark paint (a dialog born in the light ui opens it when the theme
// flips), rides in the window data, and closes when the dialog goes away
// or the visual style changes.


















// ascii, case-insensitive wide string compare. returns 0 if equal.
int _viv_icompare_w(const wchar_t *a,const wchar_t *b)
{
	while ((*a) && (*b))
	{
		wchar_t ca;
		wchar_t cb;
		
		ca = *a;
		cb = *b;
		
		if ((ca >= 'A') && (ca <= 'Z'))
		{
			ca = ca + 'a' - 'A';
		}
		
		if ((cb >= 'A') && (cb <= 'Z'))
		{
			cb = cb + 'a' - 'A';
		}
		
		if (ca != cb)
		{
			return 1;
		}
		
		a++;
		b++;
	}
	
	if (*a != *b)
	{
		return 1;
	}
	
	return 0;
}




CLIPFORMAT _viv_get_CF_PREFERREDDROPEFFECT(void)
{
	if (!_viv_CF_PREFERREDDROPEFFECT)
	{
		_viv_CF_PREFERREDDROPEFFECT = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
	}
	
	return _viv_CF_PREFERREDDROPEFFECT;
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
	
	// the pixel probe tooltip joins the theme (the comctl tooltip paints
	// whatever colors it is told here).
	if (_viv_tooltip_hwnd)
	{
		SendMessage(_viv_tooltip_hwnd,TTM_SETTIPBKCOLOR,viv_theme_color(VIV_TK_FACE),0);
		SendMessage(_viv_tooltip_hwnd,TTM_SETTIPTEXTCOLOR,viv_theme_color(VIV_TK_TEXT),0);
	}
	
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





