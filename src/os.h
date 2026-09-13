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
// Operating System calls

#ifdef __cplusplus
extern "C" {
#endif

typedef struct os_PropertyItem_s
{
    PROPID  id;                 // ID of this property
    ULONG   length;             // Length of the property value, in bytes
    WORD    type;               // Type of the value, as one of TAG_TYPE_XXX
                                // defined above
    VOID*   value;              // property value
}os_PropertyItem_t;

typedef struct os_GdiplusStartupInput_s
{
    UINT32 GdiplusVersion; // Must be 1
	void (WINAPI *DebugEventCallback)(int level, CHAR *message);
    BOOL SuppressBackgroundThread;
    BOOL SuppressExternalCodecs;
    
}os_GdiplusStartupInput_t;

void os_zero_memory(void *data,int size);
void os_copy_memory(void *d,const void *s,SIZE_T size);
void os_move_memory(void *d,const void *s,SIZE_T size);
void os_qsort(void **base,uintptr_t count,int (*compare)(const void *a,const void *b));
void os_make_rect_completely_visible(HWND hwnd,RECT *prect);
void os_MonitorRectFromWindow(HWND hwnd,int is_fullscreen,RECT *out_monitor_rect);
void os_MonitorRectFromRect(RECT *window_rect,int is_fullscreen,RECT *out_monitor_rect);
void os_MonitorRectFromCursor(int is_fullscreen,RECT *out_monitor_rect);
int os_ComboBox_AddString(HWND hwnd,int id,const utf8_t *s);
int os_ComboBox_AddString_localization_id(HWND hwnd,int id,localization_id_t localization_id);
void os_SetDlgItemText(HWND hwnd,int id,const utf8_t *s);
void os_SetDlgItemText_localization_id(HWND hwnd,int id,localization_id_t localization_id);
void os_SetWindowText(HWND hwnd,const utf8_t *s);
void os_SetWindowText_localization_id(HWND hwnd,localization_id_t localization_id);
HWND os_CreateWindowEx(DWORD dwExStyle,const utf8_t *lpClassName,const utf8_t *lpWindowName,DWORD dwStyle,int X,int Y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam);
void os_RegisterClassEx(UINT style,WNDPROC lpfnWndProc,HICON hIcon,HCURSOR hCursor,HBRUSH hbrBackground,const utf8_t *name,HICON hIconSm);
int os_is_admin(void);
int os_choose_color(HWND parent,COLORREF *colorref);
void os_init(void);
void os_kill(void);
ITEMIDLIST *os_ILCreateFromPath(const wchar_t *path);
void os_edit_move_caret_to_end(HWND hwnd);
void os_make_sure_path_exists(const wchar_t *filename);
int os_get_special_folder_path(wchar_t *wbuf,int nFolder);
void os_create_shell_link(const wchar_t *exe_filename_wbuf,const wchar_t *lnk_wbuf);
void os_center_dialog(HWND hwnd);
int os_shell_execute(HWND hwnd,const wchar_t *filename,int wait,const char *verb,const wchar_t *params);
int os_browse_for_folder(HWND parent,wchar_t *filename);
VIV_UINT64 os_get_tick_count(void);
VIV_UINT64 os_get_tick_freq(void);
void os_get_monitor_rect_from_window(HWND hwnd,RECT *monitor_rect,int fullscreen);
WNDPROC os_set_window_proc(HWND hwnd,WNDPROC proc);
int os_is_windows_7_or_later(void);
int os_is_windows_8_or_later(void);
int os_dark_controls_supported(void);
HRGN os_CreateRectRgn(int left,int top,int right,int bottom);
HRGN os_mirror_region(HRGN hrgn,int wide);
void os_fill_clamped_rect(HDC hdc,int left,int top,int right,int bottom,int clamp_left,int clamp_top,int clamp_right,int clamp_bottom,HBRUSH hbrush);
void os_fill_clipped_rect(HDC hdc,int x,int y,int wide,int high,int clip_x,int clip_y,int clip_wide,int clip_high,HBRUSH hbrush);
void os_fill_rect(HDC hdc,int x,int y,int wide,int high,HBRUSH hbrush);
void os_set_rect(RECT *rect,int x,int y,int wide,int high);
int os_get_orientation(const wchar_t *filename);
void os_adjust_window_rect(HWND hwnd,RECT *window_rect,int window_x,int window_y,int client_wide,int client_high);
DWORD os_get_window_style(HWND hwnd);
DWORD os_get_window_ex_style(HWND hwnd);
int os_get_static_wide(HWND hwnd,int id);
int os_expand_static_wide(HWND hwnd,int id,int static_wide);
void os_set_window_rect(HWND hwnd,int x,int y,int wide,int high);
void os_set_dialog_item_x_wide(HWND hwnd,int id,int x,int wide);

extern HINSTANCE os_hinstance;
extern DWORD os_major_version;
extern DWORD os_minor_version;
extern char os_is_nt;
extern HRESULT (WINAPI *os_SHOpenFolderAndSelectItems)(LPCITEMIDLIST pidlFolder,UINT cidl,LPCITEMIDLIST *apidl,DWORD dwFlags);
extern int (WINAPI *os_GdiplusStartup)( ULONG_PTR *token,const os_GdiplusStartupInput_t *input,void *output);
extern VOID (WINAPI *os_GdiplusShutdown)(ULONG_PTR token);
extern int (__stdcall *os_GdipLoadImageFromStream)(IStream* stream, void **image);
extern int (__stdcall *os_GdipLoadImageFromStreamICM)(IStream* stream, void **image);
extern int (__stdcall *os_GdipGetImageWidth)(void *image, UINT *width);
extern int (__stdcall *os_GdipGetImageHeight)(void *image, UINT *height);
extern int (__stdcall *os_GdipImageGetFrameDimensionsCount)(void *image, UINT* count);
extern int (__stdcall *os_GdipImageGetFrameDimensionsList)(void *image, GUID* dimensionIDs,UINT count);
extern int (__stdcall *os_GdipImageGetFrameCount)(void *image, const GUID* dimensionID,UINT* count);
extern int (__stdcall *os_GdipGetPropertyItemSize)(void *image, PROPID propId, UINT* size);
// note: the real gdiplus flat api is GdipGetImageThumbnail(image,w,h,**out,cb,cb_data):
// the out image is the 4th parameter. loaded optionally (see os.c), older or
// stripped gdiplus builds may not export it at all.
extern int (__stdcall *os_GdipGetImageThumbnail)(void *image,UINT thumb_wide,UINT thumb_high,void **thumb_image,void *callback,void *callback_data);
extern int (__stdcall *os_GdipGetPropertyItem)(void *image, PROPID propId,UINT propSize,void *buffer);
extern int (__stdcall *os_GdipImageSelectActiveFrame)(void *image, const GUID* dimensionID,UINT frameIndex);
extern int (__stdcall *os_GdipGetImageFlags)(void *image, UINT *flags);
extern int (__stdcall *os_GdipDisposeImage)(void *image);
extern int (__stdcall *os_GdipCreateFromHDC)(HDC hdc,void **graphics);
extern int (__stdcall *os_GdipSetCompositingMode)(void *graphics,int compositingMode);
extern int (__stdcall *os_GdipSetCompositingQuality)(void *graphics,int compositingQuality);
extern int (__stdcall *os_GdipSetInterpolationMode)(void *graphics,int interpolationMode);
extern int (__stdcall *os_GdipSetPixelOffsetMode)(void* graphics,int pixelOffsetMode);
extern int (__stdcall *os_GdipSetSmoothingMode)(void *graphics, int smoothingMode);
extern int (__stdcall *os_GdipDrawImageRectI)(void *graphics, void *image, INT x, INT y,INT width, INT height);
extern int (__stdcall *os_GdipDeleteGraphics)(void *graphics);
extern int (__stdcall *os_GdipCreateBitmapFromHBITMAP)(HBITMAP hbitmap,HPALETTE hpalette,void **bitmap);
extern int (__stdcall *os_GdipSaveImageToFile)(void *image,const wchar_t *filename,const GUID *clsidEncoder,const void *encoderParams);
extern int (__stdcall *os_GdipGetImageEncodersSize)(unsigned int *numCodecs,unsigned int *size);
extern int (__stdcall *os_GdipGetImageEncoders)(unsigned int numCodecs,unsigned int size,void *encoders);
int os_save_hbitmap(HBITMAP hbitmap,const wchar_t *filename,int format);
extern BOOL (WINAPI *os_CreateTimerQueueTimer)(PHANDLE phNewTimer,HANDLE TimerQueue,WAITORTIMERCALLBACK Callback,PVOID Parameter,DWORD DueTime,DWORD Period,ULONG Flags);
extern BOOL (WINAPI *os_DeleteTimerQueueTimer)(HANDLE TimerQueue,HANDLE Timer,HANDLE CompletionEvent);
extern BOOL (WINAPI *os_GetFileAttributesExW)(LPCWSTR lpFileName,GET_FILEEX_INFO_LEVELS fInfoLevelId,LPVOID lpFileInformation);
extern BOOL (STDAPICALLTYPE *os_IsUserAnAdmin)(void);
extern HRESULT (__stdcall *os_EnableThemeDialogTexture)(HWND hwnd, DWORD dwFlags);
// windows dark mode support. every function quietly does nothing on windows
// versions without the dark mode apis (older than windows 10 1809).
// mode 0 = light, 1 = dark, 2 = follow the system theme.
extern int os_dark_system_dark(void);
extern void os_dark_set_app_mode(int mode);
extern void os_dark_titlebar(HWND hwnd,int dark);
extern void os_dark_refresh(void);
// drop the cached system dark state so the next os_dark_system_dark()
// call re-reads it (WM_SETTINGCHANGE / WM_THEMECHANGED).

// set the DarkMode_Explorer visual style on a window (dialog controls and
// tabs draw dark with it). returns 1 when the style was applied.
extern int os_dark_window_theme(HWND hwnd);

// combobox dark parts: the common dialog class (windows 10 1903+).

extern int os_dark_combobox_theme(HWND hwnd);
// the light counterpart: back to the standard explorer visual style
// when the ui flips back to light.
extern int os_light_window_theme(HWND hwnd);

// the button theme for the custom drawn glyph faces: one open per
// dialog lifetime, closed on the way out (uxtheme is loaded
// dynamically, so the handle type stays generic).
extern HANDLE os_theme_open_button(HWND hwnd);
extern void os_theme_close(HANDLE theme);
// the size of a button part in pixels: 1 when the theme and the part
// are available (the visual styles off leave the handle null).
extern int os_theme_part_size(HANDLE theme,HDC hdc,int part,int state,int *wide,int *high);
// draw a button part state into a rect: 1 when the theme drew it.
extern int os_theme_draw_part(HANDLE theme,HDC hdc,int part,int state,const RECT *rect);
// the vssym32 button part and state values the glyph drawing uses
// (uxtheme is loaded dynamically, so the constants live here).
#define OS_BP_RADIOBUTTON 2
#define OS_BP_CHECKBOX 3
// the checkbox part states (cbs_*).
#define OS_BS_UNCHECKEDNORMAL 1
#define OS_BS_UNCHECKEDHOT 2
#define OS_BS_UNCHECKEDPRESSED 3
#define OS_BS_UNCHECKEDDISABLED 4
#define OS_BS_CHECKEDNORMAL 5
#define OS_BS_CHECKEDHOT 6
#define OS_BS_CHECKEDPRESSED 7
#define OS_BS_CHECKEDDISABLED 8
#define OS_BS_MIXEDNORMAL 9
#define OS_BS_MIXEDHOT 10
#define OS_BS_MIXEDPRESSED 11
#define OS_BS_MIXEDDISABLED 12
// the radiobutton part states (rbs_*).
#define OS_RBS_UNCHECKEDNORMAL 1
#define OS_RBS_UNCHECKEDHOT 2
#define OS_RBS_UNCHECKEDPRESSED 3
#define OS_RBS_UNCHECKEDDISABLED 4
#define OS_RBS_CHECKEDNORMAL 5
#define OS_RBS_CHECKEDHOT 6
#define OS_RBS_CHECKEDPRESSED 7
#define OS_RBS_CHECKEDDISABLED 8

extern int os_allow_dark_mode_for_window(HWND hwnd,int allow);
extern void os_dark_invalidate(void);
extern BOOL (WINAPI *os_ChangeWindowMessageFilterEx)(HWND hWnd,UINT message,DWORD action,void *pChangeFilterStruct);
extern DWORD (WINAPI *os_GetLayout)(HDC hdc);
extern EXECUTION_STATE (WINAPI *os_SetThreadExecutionState)(  EXECUTION_STATE esFlags);
extern LANGID (WINAPI *os_GetUserDefaultUILanguage)(void);
extern int os_logical_wide;
extern int os_logical_high;

// refresh os_logical_wide / os_logical_high from the window's monitor dpi
// (per monitor v2). returns 1 when the values changed.
int os_window_update_dpi(HWND hwnd);

// the system menu font at the current window dpi (menu bar owner draw).
int os_menu_font(LOGFONTW *lf);
// the dpi of a window: getdpiforwindow when the os provides it, the
// tracked window dpi otherwise.
int os_window_dpi(HWND hwnd);
// fill lf with the message font (the dialog base font) at the window
// own dpi: segoe ui on an english system, the locale native ui face
// (microsoft yahei ui on a chinese one) - no glyph fallback for the
// cjk labels. returns 1 on success.
int os_dialog_font(LOGFONTW *lf,HWND hwnd);

// windows 11 chrome: rounded corners + a caption color matching the canvas.
// returns 0 on windows 10 and older so the caller can fall back to its own
// frame. the first call latches the platform answer.
int os_window_modern_chrome(HWND hwnd,COLORREF caption_color);
int os_menu_modern_chrome(HWND hwnd,COLORREF border_color);

// the latched platform answer from the chrome probes: 1 on windows 11
// (the modern attributes landed), 0 before the first probe or on older
// windows. register-time decisions (the drop shadow class style) read it.
int os_is_win11(void);

// touch / gesture support (Windows 7+)
// self-declared, binary compatible structures.
// (avoids _WIN32_WINNT gating in older SDKs)

typedef struct os_GestureConfig_s
{
	DWORD dwID;
	DWORD dwWant;
	DWORD dwBlock;

}os_GestureConfig_t;

typedef struct os_GestureInfo_s
{
	UINT cbSize;
	DWORD dwFlags;
	DWORD dwID;
	HWND hwndTarget;
	POINTS ptsLocation;
	DWORD dwInstanceID;
	DWORD dwSequenceID;
	VIV_UINT64 ullArguments;
	UINT cbExtraArgs;

}os_GestureInfo_t;

extern BOOL (WINAPI *os_SetGestureConfig)(HWND hwnd,DWORD reserved,UINT cIDs,os_GestureConfig_t *configs,UINT cbSize);
extern BOOL (WINAPI *os_GetGestureInfo)(void *gesture_info_handle,os_GestureInfo_t *gesture_info);
extern BOOL (WINAPI *os_CloseGestureInfoHandle)(void *gesture_info_handle);

int os_is_touch_available(void);

// vista+ regdeletekeyexw, resolved lazily (see os.c). returns 1 when the
// key was deleted, 0 when the api is missing or the delete failed:
// regdeletekeyw cannot reach the alternate registry view.
int os_reg_delete_key_ex(HKEY hkey,const wchar_t *name,REGSAM access);

#ifdef __cplusplus
}
#endif
