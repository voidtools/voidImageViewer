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
void os_copy_memory(void *d,const void *s,int size);
void os_move_memory(void *d,const void *s,int size);
void os_qsort(void **base,uintptr_t count,int (*compare)(const void *a,const void *b));
void os_make_rect_completely_visible(HWND hwnd,RECT *prect);
void os_MonitorRectFromWindow(HWND hwnd,int is_fullscreen,RECT *out_monitor_rect);
void os_MonitorRectFromRect(RECT *window_rect,int is_fullscreen,RECT *out_monitor_rect);
void os_MonitorRectFromCursor(int is_fullscreen,RECT *out_monitor_rect);
int os_ComboBox_AddString(HWND hwnd,int id,const utf8_t *s);
void os_SetDlgItemText(HWND hwnd,int id,const utf8_t *s);
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
int os_statusbar_index_from_x(HWND statusbar_hwnd,int x);
int os_is_windows_7_or_later(void);
int os_is_windows_8_or_later(void);
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
extern BOOL (WINAPI *os_CreateTimerQueueTimer)(PHANDLE phNewTimer,HANDLE TimerQueue,WAITORTIMERCALLBACK Callback,PVOID Parameter,DWORD DueTime,DWORD Period,ULONG Flags);
extern BOOL (WINAPI *os_DeleteTimerQueueTimer)(HANDLE TimerQueue,HANDLE Timer,HANDLE CompletionEvent);
extern BOOL (WINAPI *os_GetFileAttributesExW)(LPCWSTR lpFileName,GET_FILEEX_INFO_LEVELS fInfoLevelId,LPVOID lpFileInformation);
extern BOOL (STDAPICALLTYPE *os_IsUserAnAdmin)(void);
extern HRESULT (__stdcall *os_EnableThemeDialogTexture)(HWND hwnd, DWORD dwFlags);
extern BOOL (WINAPI *os_ChangeWindowMessageFilterEx)(HWND hWnd,UINT message,DWORD action,void *pChangeFilterStruct);
extern DWORD (WINAPI *os_GetLayout)(HDC hdc);
extern EXECUTION_STATE (WINAPI *os_SetThreadExecutionState)(  EXECUTION_STATE esFlags);
extern LANGID (WINAPI *os_GetUserDefaultUILanguage)(void);
extern int os_logical_wide;
extern int os_logical_high;

#ifdef __cplusplus
}
#endif
