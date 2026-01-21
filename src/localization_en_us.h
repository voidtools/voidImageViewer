//
// Copyright 2026 hesphoros
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
// English (US) localization
// Safe as UTF-8 without BOM.

static const utf8_t *_localization_string_array_en_us[LOCALIZATION_ID_COUNT] = 
{
	// Application name
	"English (US)", // LOCALIZATION_ID_LANGUAGE
	
	// Application name
	"void Image Viewer", // LOCALIZATION_ID_APP_NAME

	// File menu
	"&File", // LOCALIZATION_ID_FILE,
	"&Open File...", // LOCALIZATION_ID_OPEN_FILE,
	"Open &Folder...", // LOCALIZATION_ID_OPEN_FOLDER,
	"Open Everything &Search...", // LOCALIZATION_ID_OPEN_EVERYTHING_SEARCH,
	"&Add File...", // LOCALIZATION_ID_ADD_FILE,
	"Add Folder...", // LOCALIZATION_ID_ADD_FOLDER,
	"Add Everything Search...", // LOCALIZATION_ID_ADD_EVERYTHING_SEARCH,
	"Open F&ile Location...", // LOCALIZATION_ID_OPEN_FILE_LOCATION,
	"&Edit...", // LOCALIZATION_ID_EDIT,
	"Pre&view...", // LOCALIZATION_ID_PREVIEW,
	"&Print...", // LOCALIZATION_ID_PRINT,
	"Set &Desktop Wallpaper", // LOCALIZATION_ID_SET_DESKTOP_WALLPAPER,
	"&Close", // LOCALIZATION_ID_CLOSE,
	"&Delete", // LOCALIZATION_ID_DELETE,
	"Delete (Recycle)", // LOCALIZATION_ID_DELETE_RECYCLE,
	"Delete (Permanently)", // LOCALIZATION_ID_DELETE_PERMANENTLY,
	"&Rename", // LOCALIZATION_ID_RENAME,
	"P&roperties", // LOCALIZATION_ID_PROPERTIES,
	"E&xit", // LOCALIZATION_ID_EXIT,
	
	// Edit menu
	"&Edit", // LOCALIZATION_ID_EDIT_MENU,
	"Cu&t", // LOCALIZATION_ID_CUT,
	"&Copy", // LOCALIZATION_ID_COPY,
	"Copy Filename", // LOCALIZATION_ID_COPY_FILENAME,
	"Cop&y Image", // LOCALIZATION_ID_COPY_IMAGE,
	"&Paste", // LOCALIZATION_ID_PASTE,
	"Rotate &Cloc&kwise", // LOCALIZATION_ID_ROTATE_CLOCKWISE,
	"Rotate Cou&nterclockwise", // LOCALIZATION_ID_ROTATE_COUNTERCLOCKWISE,
	"Copy to &Folder...", // LOCALIZATION_ID_COPY_TO,
	"Mo&ve to Folder...", // LOCALIZATION_ID_MOVE_TO,
	
	// View menu
	"&View", // LOCALIZATION_ID_VIEW,
	"Caption", // LOCALIZATION_ID_CAPTION,
	"Frame", // LOCALIZATION_ID_FRAME,
	"&Menu", // LOCALIZATION_ID_MENU,
	"Status &Bar", // LOCALIZATION_ID_STATUS_BAR,
	"&Controls", // LOCALIZATION_ID_CONTROLS,
	"&Preset", // LOCALIZATION_ID_PRESET,
	"&Minimal", // LOCALIZATION_ID_MINIMAL,
	"&Compact", // LOCALIZATION_ID_COMPACT,
	"&Normal", // LOCALIZATION_ID_NORMAL,
	"F&ullscreen", // LOCALIZATION_ID_FULLSCREEN,
	"&Slideshow", // LOCALIZATION_ID_SLIDESHOW,
	"&Window Size", // LOCALIZATION_ID_WINDOW_SIZE,
	"&Auto Fit", // LOCALIZATION_ID_AUTO_FIT,
	"&Refresh", // LOCALIZATION_ID_REFRESH,
	"&Allow Shrinking", // LOCALIZATION_ID_ALLOW_SHRINKING,
	"&Keep Aspect Ratio", // LOCALIZATION_ID_KEEP_ASPECT_RATIO,
	"&Fill Window", // LOCALIZATION_ID_FILL_WINDOW,
	"&Best Fit", // LOCALIZATION_ID_BEST_FIT,
	"Pa&n && Scan", // LOCALIZATION_ID_PAN_SCAN,
	"&Increase Size", // LOCALIZATION_ID_INCREASE_SIZE,
	"&Decrease Size", // LOCALIZATION_ID_DECREASE_SIZE,
	"I&ncrease Width", // LOCALIZATION_ID_INCREASE_WIDTH,
	"D&ecrease Width", // LOCALIZATION_ID_DECREASE_WIDTH,
	"In&crease Height", // LOCALIZATION_ID_INCREASE_HEIGHT,
	"De&cre&ase Height", // LOCALIZATION_ID_DECREASE_HEIGHT,
	"&Zoom", // LOCALIZATION_ID_ZOOM,
	"Zoom &In", // LOCALIZATION_ID_ZOOM_IN,
	"Zoom &Out", // LOCALIZATION_ID_ZOOM_OUT,
	"&Reset", // LOCALIZATION_ID_RESET,
	"Move &Up", // LOCALIZATION_ID_MOVE_UP,
	"Move &Down", // LOCALIZATION_ID_MOVE_DOWN,
	"Move &Left", // LOCALIZATION_ID_MOVE_LEFT,
	"Move &Right", // LOCALIZATION_ID_MOVE_RIGHT,
	"Move Up Left", // LOCALIZATION_ID_MOVE_UP_LEFT,
	"Move Up Right", // LOCALIZATION_ID_MOVE_UP_RIGHT,
	"Move Down Left", // LOCALIZATION_ID_MOVE_DOWN_LEFT,
	"Move Down Right", // LOCALIZATION_ID_MOVE_DOWN_RIGHT,
	"Move Cen&ter", // LOCALIZATION_ID_MOVE_CENTER,
	"On &Top", // LOCALIZATION_ID_ON_TOP,
	"&Always", // LOCALIZATION_ID_ALWAYS,
	"&While Playing Slideshow or Animating", // LOCALIZATION_ID_WHILE_PLAYING_OR_ANIMATING,
	"&Never", // LOCALIZATION_ID_NEVER,
	"&Options...", // LOCALIZATION_ID_OPTIONS,
	
	// Slideshow menu
	"&Slideshow", // LOCALIZATION_ID_SLIDESHOW_MENU,
	"&Play/Pause", // LOCALIZATION_ID_PLAY_PAUSE,
	"&Rate", // LOCALIZATION_ID_RATE,
	"&Decrease Rate", // LOCALIZATION_ID_DECREASE_RATE,
	"&Increase Rate", // LOCALIZATION_ID_INCREASE_RATE,
	"Custom...", // LOCALIZATION_ID_CUSTOM,
	
	// Animation menu
	"&Animation", // LOCALIZATION_ID_ANIMATION,
	"Jump &Forward", // LOCALIZATION_ID_JUMP_FORWARD,
	"Jump &Backward", // LOCALIZATION_ID_JUMP_BACKWARD,
	"Short Jump &Forward", // LOCALIZATION_ID_SHORT_JUMP_FORWARD,
	"Short Jump &Backward", // LOCALIZATION_ID_SHORT_JUMP_BACKWARD,
	"Long Jump &Forward", // LOCALIZATION_ID_LONG_JUMP_FORWARD,
	"Long Jump &Backward", // LOCALIZATION_ID_LONG_JUMP_BACKWARD,
	"F&rame Step", // LOCALIZATION_ID_FRAME_STEP,
	"Pre&vious Frame", // LOCALIZATION_ID_PREVIOUS_FRAME,
	"F&irst Frame", // LOCALIZATION_ID_FIRST_FRAME,
	"&Last Frame", // LOCALIZATION_ID_LAST_FRAME,
	"&Decrease Rate", // LOCALIZATION_ID_DECREASE_RATE_ANIM,
	"&Increase Rate", // LOCALIZATION_ID_INCREASE_RATE_ANIM,
	"R&eset Rate", // LOCALIZATION_ID_RESET_RATE,
	
	// Navigate menu
	"&Navigate", // LOCALIZATION_ID_NAVIGATE,
	"&Next", // LOCALIZATION_ID_NEXT,
	"P&revious", // LOCALIZATION_ID_PREVIOUS,
	"&Home", // LOCALIZATION_ID_HOME,
	"&End", // LOCALIZATION_ID_END,
	"S&ort", // LOCALIZATION_ID_SORT,
	"&Name", // LOCALIZATION_ID_NAME,
	"Full &Path", // LOCALIZATION_ID_FULL_PATH,
	"&Size", // LOCALIZATION_ID_SIZE,
	"Date &Modified", // LOCALIZATION_ID_DATE_MODIFIED,
	"Date &Created", // LOCALIZATION_ID_DATE_CREATED,
	"&Ascending", // LOCALIZATION_ID_ASCENDING,
	"&Descending", // LOCALIZATION_ID_DESCENDING,
	"&Shuffle", // LOCALIZATION_ID_SHUFFLE,
	"&Jump To...", // LOCALIZATION_ID_JUMP_TO,
	
	// Help menu
	"&Help", // LOCALIZATION_ID_HELP,
	"&Help", // LOCALIZATION_ID_HELP_MENU,
	"&Command Line Options", // LOCALIZATION_ID_COMMAND_LINE_OPTIONS,
	"Home &Page", // LOCALIZATION_ID_HOME_PAGE,
	"&Donate", // LOCALIZATION_ID_DONATE,
	"&About", // LOCALIZATION_ID_ABOUT,
	"https://www.voidtools.com/support/voidimageviewer/", // LOCALIZATION_ID_HELP_SUPPORT_URL,
	"https://www.voidtools.com/donate/", // LOCALIZATION_ID_HELP_DONATE_URL,
	"https://www.voidtools.com/", // LOCALIZATION_ID_HELP_WEBSITE_URL,

	// Dialog strings
	"Options - Void Image Viewer", // LOCALIZATION_ID_OPTIONS_TITLE,
	"General", // LOCALIZATION_ID_GENERAL,
	"View", // LOCALIZATION_ID_VIEW_DIALOG,
	"Controls", // LOCALIZATION_ID_CONTROLS_DIALOG,
	"OK", // LOCALIZATION_ID_OK,
	"Cancel", // LOCALIZATION_ID_CANCEL,
	"&Store settings in %APPDATA%\\voidImageViewer", // LOCALIZATION_ID_STORE_SETTINGS_APPDATA,
	"Allow multiple &instances", // LOCALIZATION_ID_ALLOW_MULTIPLE_INSTANCES,
	"Start &menu shortcuts", // LOCALIZATION_ID_STARTMENU_SHORTCUTS,
	"Associations", // LOCALIZATION_ID_ASSOCIATIONS,
	"Check &All", // LOCALIZATION_ID_CHECK_ALL,
	"Check &None", // LOCALIZATION_ID_CHECK_NONE,
	"&Shrink blit mode:", // LOCALIZATION_ID_SHRINK_BLIT_MODE,
	"&Magnify blit mode:", // LOCALIZATION_ID_MAGNIFY_BLIT_MODE,
	"Auto si&ze window:", // LOCALIZATION_ID_AUTO_SIZE_WINDOW,
	"&Play animations at least once in slideshow", // LOCALIZATION_ID_PLAY_ANIMATIONS_ONCE,
	"Preload &next image", // LOCALIZATION_ID_PRELOAD_NEXT_IMAGE,
	"Cache &last image", // LOCALIZATION_ID_CACHE_LAST_IMAGE,
	"&Windowed background color:", // LOCALIZATION_ID_WINDOWED_BACKGROUND_COLOR,
	"&Fullscreen background color:", // LOCALIZATION_ID_FULLSCREEN_BACKGROUND_COLOR,
	"&Left click action:", // LOCALIZATION_ID_LEFT_CLICK_ACTION,
	"&Right click action:", // LOCALIZATION_ID_RIGHT_CLICK_ACTION,
	"&Mouse wheel action:", // LOCALIZATION_ID_MOUSE_WHEEL_ACTION,
	"&Commands:", // LOCALIZATION_ID_COMMANDS,
	"Settings for selected command", // LOCALIZATION_ID_SETTINGS_FOR_SELECTED_COMMAND,
	"&Add...", // LOCALIZATION_ID_ADD,
	"Add Keyboard Shortcut", // LOCALIZATION_ID_ADD_KEYBOARD_SHORTCUT,
	"Edit Keyboard Shortcut", // LOCALIZATION_ID_EDIT_KEYBOARD_SHORTCUT,
	"Shortcut &key:", // LOCALIZATION_ID_SHORTCUT_KEY,
	"Shortcut key currently used by:", // LOCALIZATION_ID_SHORTCUT_KEY_CURRENTLY_USED_BY,
	"Remo&ve", // LOCALIZATION_ID_REMOVE,
	"Set Custom Rate", // LOCALIZATION_ID_SET_CUSTOM_RATE,
	"&Custom rate:", // LOCALIZATION_ID_CUSTOM_RATE,
	"About void Image Viewer", // LOCALIZATION_ID_ABOUT_TITLE,
	"Rename", // LOCALIZATION_ID_RENAME_TITLE,
	"Jump To", // LOCALIZATION_ID_JUMP_TO_TITLE,
	"Search Everything", // LOCALIZATION_ID_SEARCH_EVERYTHING,
	"Randomize", // LOCALIZATION_ID_RANDOMIZE,
	
	// Time units
	" Milliseconds", // LOCALIZATION_ID_MILLISECONDS,
	" Seconds", // LOCALIZATION_ID_SECONDS,
	"1 Minute", // LOCALIZATION_ID_MINUTE,
	" Minutes", // LOCALIZATION_ID_MINUTES,
	
	"Everything not available", // LOCALIZATION_ID_EVERYTHING_NOT_AVAILABLE_MESSAGE,
};
