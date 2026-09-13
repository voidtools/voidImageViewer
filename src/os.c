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

#include "viv.h"

#define _OS_QSORT_CUTOFF	8            /* testing shows that this is good value */
#define _OS_QSORT_STKSIZ	((8*sizeof(void*)) - 2)

#define _OS_DRAWTEXT_MAX_LEN		(4096-1)

typedef struct _os_COMDLG_FILTERSPEC_s 
{
	wchar_t *pszName;
	wchar_t *pszSpec;
	
} _os_COMDLG_FILTERSPEC_t;
	
typedef struct _os_IFileOpenDialogVtbl
{
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(struct _os_IFileOpenDialog_s *This,REFIID riid,void **ppvObject);
    ULONG (STDMETHODCALLTYPE *AddRef)(struct _os_IFileOpenDialog_s *This);
    ULONG (STDMETHODCALLTYPE *Release)(struct _os_IFileOpenDialog_s *This);
    HRESULT (STDMETHODCALLTYPE *Show)(struct _os_IFileOpenDialog_s *This,HWND hwndOwner);
    HRESULT (STDMETHODCALLTYPE *SetFileTypes)(struct _os_IFileOpenDialog_s *This,UINT cFileTypes,const _os_COMDLG_FILTERSPEC_t *rgFilterSpec);
    HRESULT (STDMETHODCALLTYPE *SetFileTypeIndex)(struct _os_IFileOpenDialog_s *This,UINT iFileType);
    HRESULT (STDMETHODCALLTYPE *GetFileTypeIndex)(struct _os_IFileOpenDialog_s *This,UINT *piFileType);
    HRESULT (STDMETHODCALLTYPE *Advise)(struct _os_IFileOpenDialog_s *This,void *pfde,DWORD *pdwCookie);
    HRESULT (STDMETHODCALLTYPE *Unadvise)(struct _os_IFileOpenDialog_s *This,DWORD dwCookie);
    HRESULT (STDMETHODCALLTYPE *SetOptions)(struct _os_IFileOpenDialog_s *This,int fos);
    HRESULT (STDMETHODCALLTYPE *GetOptions)(struct _os_IFileOpenDialog_s *This,int *pfos);
    HRESULT (STDMETHODCALLTYPE *SetDefaultFolder)(struct _os_IFileOpenDialog_s *This,IShellItem *psi);
    HRESULT (STDMETHODCALLTYPE *SetFolder)(struct _os_IFileOpenDialog_s *This,IShellItem *psi);
    HRESULT (STDMETHODCALLTYPE *GetFolder)(struct _os_IFileOpenDialog_s *This,IShellItem **ppsi);
    HRESULT (STDMETHODCALLTYPE *GetCurrentSelection)(struct _os_IFileOpenDialog_s *This,IShellItem **ppsi);
    HRESULT (STDMETHODCALLTYPE *SetFileName)(struct _os_IFileOpenDialog_s *This,LPCWSTR pszName);
    HRESULT (STDMETHODCALLTYPE *GetFileName)(struct _os_IFileOpenDialog_s *This,LPWSTR *pszName);
    HRESULT (STDMETHODCALLTYPE *SetTitle)(struct _os_IFileOpenDialog_s *This,LPCWSTR pszTitle);
    HRESULT (STDMETHODCALLTYPE *SetOkButtonLabel)(struct _os_IFileOpenDialog_s *This,LPCWSTR pszText);
    HRESULT (STDMETHODCALLTYPE *SetFileNameLabel)(struct _os_IFileOpenDialog_s *This,LPCWSTR pszLabel);
    HRESULT (STDMETHODCALLTYPE *GetResult)(struct _os_IFileOpenDialog_s *This,IShellItem **ppsi);
    HRESULT (STDMETHODCALLTYPE *AddPlace)(struct _os_IFileOpenDialog_s *This,IShellItem *psi,int fdap);
    HRESULT (STDMETHODCALLTYPE *SetDefaultExtension)(struct _os_IFileOpenDialog_s *This,LPCWSTR pszDefaultExtension);
    HRESULT (STDMETHODCALLTYPE *Close)(struct _os_IFileOpenDialog_s *This,HRESULT hr);
    HRESULT (STDMETHODCALLTYPE *SetClientGuid)(struct _os_IFileOpenDialog_s *This,REFGUID guid);
    HRESULT (STDMETHODCALLTYPE *ClearClientData)(struct _os_IFileOpenDialog_s *This);
    HRESULT (STDMETHODCALLTYPE *SetFilter)(struct _os_IFileOpenDialog_s *This,void *pFilter);
    HRESULT (STDMETHODCALLTYPE *GetResults)(struct _os_IFileOpenDialog_s *This,void **ppenum);
    HRESULT (STDMETHODCALLTYPE *GetSelectedItems)(struct _os_IFileOpenDialog_s *This,void **ppsai);

}_os_IFileOpenDialogVtbl;

typedef struct _os_IFileOpenDialog_s 
{ 
	struct _os_IFileOpenDialogVtbl *lpVtbl; 

}_os_IFileOpenDialog_t;

typedef struct _os_PROPERTYKEY_s 
{
	GUID fmtid;
	DWORD pid;
	
}_os_PROPERTYKEY_t;

typedef struct _os_IPropertyStoreVtbl_s
{
	HRESULT (STDMETHODCALLTYPE *QueryInterface)(struct _os_IPropertyStore_s *property_store,REFIID riid,void **ppvObject);
	ULONG (STDMETHODCALLTYPE *AddRef)(struct _os_IPropertyStore_s *property_store);
	ULONG (STDMETHODCALLTYPE *Release)(struct _os_IPropertyStore_s *property_store);
	HRESULT (STDMETHODCALLTYPE *GetCount)(struct _os_IPropertyStore_s *property_store,DWORD *cProps);
	HRESULT (STDMETHODCALLTYPE *GetAt)(struct _os_IPropertyStore_s *property_store,DWORD iProp,void *pkey);
	HRESULT (STDMETHODCALLTYPE *GetValue)(struct _os_IPropertyStore_s *property_store,_os_PROPERTYKEY_t *key,PROPVARIANT *pv);
	HRESULT (STDMETHODCALLTYPE *SetValue)(struct _os_IPropertyStore_s *property_store,_os_PROPERTYKEY_t *key,void *propvar);
	HRESULT (STDMETHODCALLTYPE *Commit)(struct _os_IPropertyStore_s *property_store);

}_os_IPropertyStoreVtbl_t;

typedef struct _os_IPropertyStore_s
{
	_os_IPropertyStoreVtbl_t *lpVtbl;

}_os_IPropertyStore_t;

static void _os_qsort_indexes_shortsort(void **lo,void **hi,int (*comp)(const void *,const void *));
static FARPROC __os_get_proc_address(HMODULE hmodule,const char *name);
static int CALLBACK _os_BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData);
static IShellItem *_os_shellitem_from_filename(const wchar_t *filename);
static IShellItem *_os_get_shellitem(const ITEMIDLIST *pidl);

HINSTANCE os_hinstance = 0;
DWORD os_major_version = 0; // 4 = 9x/nt, 5 = xp,2k,server2k3, 6 = vista
DWORD os_minor_version = 0;
DWORD os_build_number = 0; // the real build number (valid with the supportedOS manifest guid list)
char os_is_nt = 0;
BOOL (WINAPI *os_CreateTimerQueueTimer)(PHANDLE phNewTimer,HANDLE TimerQueue,WAITORTIMERCALLBACK Callback,PVOID Parameter,DWORD DueTime,DWORD Period,ULONG Flags) = 0;
BOOL (WINAPI *os_DeleteTimerQueueTimer)(HANDLE TimerQueue,HANDLE Timer,HANDLE CompletionEvent) = 0;
BOOL (WINAPI *_os_SetDllDirectoryW)(LPCTSTR lpPathName) = 0;
BOOL (WINAPI *_os_SetDefaultDllDirectories)(DWORD DirectoryFlags) = 0;
BOOL (WINAPI *os_GetFileAttributesExW)(LPCWSTR lpFileName,GET_FILEEX_INFO_LEVELS fInfoLevelId,LPVOID lpFileInformation) = NULL;
BOOL (WINAPI *_os_IsDebuggerPresent)(void) = 0;
EXECUTION_STATE (WINAPI *os_SetThreadExecutionState)(  EXECUTION_STATE esFlags) = NULL;
LANGID (WINAPI *os_GetUserDefaultUILanguage)(void) = NULL;
BOOL (WINAPI *os_SetGestureConfig)(HWND hwnd,DWORD reserved,UINT cIDs,os_GestureConfig_t *configs,UINT cbSize) = NULL;
BOOL (WINAPI *os_GetGestureInfo)(void *gesture_info_handle,os_GestureInfo_t *gesture_info) = NULL;
BOOL (WINAPI *os_CloseGestureInfoHandle)(void *gesture_info_handle) = NULL;
HRESULT (WINAPI *os_SHOpenFolderAndSelectItems)(LPCITEMIDLIST pidlFolder,UINT cidl,LPCITEMIDLIST *apidl,DWORD dwFlags) = 0;
int (WINAPI *os_GdiplusStartup)(OUT ULONG_PTR *token,const os_GdiplusStartupInput_t *input,void *output) = 0;
VOID (WINAPI *os_GdiplusShutdown)(ULONG_PTR token) = 0;
int (__stdcall *os_GdipLoadImageFromStream)(IStream* stream, void **image) = 0;
int (__stdcall *os_GdipLoadImageFromStreamICM)(IStream* stream, void **image) = 0;
int (__stdcall *os_GdipGetImageWidth)(void *image, UINT *width) = 0;
int (__stdcall *os_GdipGetImageHeight)(void *image, UINT *height) = 0;
int (__stdcall *os_GdipImageGetFrameDimensionsCount)(void *image, UINT* count) = 0;
int (__stdcall *os_GdipImageGetFrameDimensionsList)(void *image, GUID* dimensionIDs,UINT count) = 0;
int (__stdcall *os_GdipImageGetFrameCount)(void *image, const GUID* dimensionID,UINT* count) = 0;
int (__stdcall *os_GdipGetPropertyItemSize)(void *image, PROPID propId, UINT* size) = 0;
int (__stdcall *os_GdipGetImageThumbnail)(void *image,UINT thumb_wide,UINT thumb_high,void **thumb_image,void *callback,void *callback_data) = 0;
int (__stdcall *os_GdipGetPropertyItem)(void *image, PROPID propId,UINT propSize,void *buffer) = 0;
int (__stdcall *os_GdipImageSelectActiveFrame)(void *image, const GUID* dimensionID,UINT frameIndex) = 0;
int (__stdcall *os_GdipGetImageFlags)(void *image, UINT *flags) = 0;
int (__stdcall *os_GdipDisposeImage)(void *image) = 0;
int (__stdcall *os_GdipCreateFromHDC)(HDC hdc,void **graphics) = 0;
int (__stdcall *os_GdipSetCompositingMode)(void *graphics,int compositingMode) = 0;
int (__stdcall *os_GdipSetCompositingQuality)(void *graphics,int compositingQuality) = 0;
int (__stdcall *os_GdipSetInterpolationMode)(void *graphics,int interpolationMode) = 0;
int (__stdcall *os_GdipSetPixelOffsetMode)(void* graphics,int pixelOffsetMode) = 0;
int (__stdcall *os_GdipSetSmoothingMode)(void *graphics, int smoothingMode) = 0;
int (__stdcall *os_GdipDrawImageRectI)(void *graphics, void *image, INT x, INT y,INT width, INT height) = 0;
int (__stdcall *os_GdipDeleteGraphics)(void *graphics) = 0;
int (__stdcall *os_GdipCreateBitmapFromHBITMAP)(HBITMAP hbitmap,HPALETTE hpalette,void **bitmap) = 0;
int (__stdcall *os_GdipSaveImageToFile)(void *image,const wchar_t *filename,const GUID *clsidEncoder,const void *encoderParams) = 0;
int (__stdcall *os_GdipGetImageEncodersSize)(unsigned int *numCodecs,unsigned int *size) = 0;
int (__stdcall *os_GdipGetImageEncoders)(unsigned int numCodecs,unsigned int size,void *encoders) = 0;
BOOL (STDAPICALLTYPE *os_IsUserAnAdmin)(void) = 0;
HRESULT (__stdcall *os_EnableThemeDialogTexture)(HWND hwnd, DWORD dwFlags) = 0;
static unsigned int (__cdecl *_os_controlfp)(unsigned int _NewValue,unsigned int _Mask) = 0;
BOOL (WINAPI *os_ChangeWindowMessageFilterEx)(HWND hWnd,UINT message,DWORD action,void *pChangeFilterStruct) = 0;
DWORD (WINAPI *os_GetLayout)(HDC hdc) = 0;
// windows 10 (1809+) dark mode exports, resolved by ordinal at runtime.
typedef int (__stdcall *OS_SetPreferredAppMode_fn)(int mode);
typedef void (__stdcall *OS_FlushMenuThemes_fn)(void);
typedef void (__stdcall *OS_RefreshImmersiveColorPolicyState_fn)(void);
typedef int (__stdcall *OS_ShouldAppsUseDarkMode_fn)(void);
typedef int (__stdcall *OS_AllowDarkModeForWindow_fn)(HWND hwnd,int allow);
typedef HRESULT (__stdcall *OS_DwmSetWindowAttribute_fn)(HWND hwnd,DWORD attribute,const void *value,DWORD size);
static OS_SetPreferredAppMode_fn _os_SetPreferredAppMode = 0;
static OS_FlushMenuThemes_fn _os_FlushMenuThemes = 0;
static OS_RefreshImmersiveColorPolicyState_fn _os_RefreshImmersiveColorPolicyState = 0;
static OS_ShouldAppsUseDarkMode_fn _os_ShouldAppsUseDarkMode = 0;
static OS_AllowDarkModeForWindow_fn _os_AllowDarkModeForWindow = 0;
static OS_DwmSetWindowAttribute_fn _os_DwmSetWindowAttribute = 0;

typedef HRESULT (__stdcall *OS_SetWindowTheme_fn)(HWND hwnd,const wchar_t *sub_app_name,const wchar_t *sub_id_list);
static OS_SetWindowTheme_fn _os_SetWindowTheme = 0;
// the theme part metrics for the custom drawn glyph labels (the same
// dynamic pattern as the theme class calls above).
typedef HANDLE (__stdcall *OS_OpenThemeData_fn)(HWND hwnd,const wchar_t *class_list);
typedef HRESULT (__stdcall *OS_CloseThemeData_fn)(HANDLE theme);
typedef HRESULT (__stdcall *OS_GetThemePartSize_fn)(HANDLE theme,HDC hdc,int part,int state,const RECT *rect,int size_type,SIZE *size);
static OS_OpenThemeData_fn _os_OpenThemeData = 0;
static OS_CloseThemeData_fn _os_CloseThemeData = 0;
static OS_GetThemePartSize_fn _os_GetThemePartSize = 0;
typedef HRESULT (__stdcall *OS_DrawThemeBackground_fn)(HANDLE theme,HDC hdc,int part,int state,const RECT *rect);
static OS_DrawThemeBackground_fn _os_DrawThemeBackground = 0;

// registry reads for a stable dark mode detection: the undocumented uxtheme
// probe returns wrong values on some windows 10 1903+ builds, the personalize
// registry value is the documented source the shell itself follows.
typedef LONG (__stdcall *OS_RegOpenKeyExW_fn)(HKEY key,const wchar_t *name,DWORD options,DWORD access,HKEY *result);
typedef LONG (__stdcall *OS_RegQueryValueExW_fn)(HKEY key,const wchar_t *name,DWORD *reserved,DWORD *type,unsigned char *data,DWORD *size);
typedef LONG (__stdcall *OS_RegCloseKey_fn)(HKEY key);
static OS_RegOpenKeyExW_fn _os_RegOpenKeyExW = 0;
static OS_RegQueryValueExW_fn _os_RegQueryValueExW = 0;
static OS_RegCloseKey_fn _os_RegCloseKey = 0;
typedef LONG (__stdcall *OS_RegDeleteKeyExW_fn)(HKEY key,const wchar_t *name,REGSAM access,DWORD reserved);
static OS_RegDeleteKeyExW_fn _os_RegDeleteKeyExW = 0;
static HMODULE _os_advapi32_hmodule = 0;

// cached dark state: the ui queries the dark mode in paint paths, so the
// system is probed once and the cache is dropped on setting changes.
static int _os_dark_cache_valid = 0;
static int _os_dark_cache_dark = 0;

static HMONITOR (WINAPI *_os_MonitorFromWindow)(HWND hwnd,DWORD dwFlags) = 0;
static HMONITOR (WINAPI *_os_MonitorFromRect)(LPCRECT lprc,DWORD dwFlags) = 0;
static HMONITOR (WINAPI *_os_MonitorFromPoint)(POINT pt,DWORD dwFlags) = 0;

// windows 10 1607+: per window dpi query, used by os_window_update_dpi().
static UINT (WINAPI *_os_GetDpiForWindow)(HWND hwnd) = 0;
// windows 10 1703+: process wide dpi awareness claim, used when the
// embedded manifest is missing or was stripped.
static BOOL (WINAPI *_os_SetProcessDpiAwarenessContext)(HANDLE context) = 0;
// windows 10 1607+: per thread dpi awareness claim, same purpose.
static HANDLE (WINAPI *_os_SetThreadDpiAwarenessContext)(HANDLE context) = 0;
// windows 8.1: shcore fallback.
static HRESULT (WINAPI *_os_SetProcessDpiAwareness)(int value) = 0;
static HMODULE _os_shcore_hmodule = 0;
// windows vista / 7: the classic system aware claim - the only dpi api
// those systems carry.
static BOOL (WINAPI *_os_SetProcessDPIAware)(void) = 0;

// windows 10 1607+: dpi aware system parameters, used by os_menu_font().
static BOOL (WINAPI *_os_SystemParametersInfoForDpi)(UINT action,UINT param,void *pvparam,UINT winini,UINT dpi) = 0;

int os_logical_wide = 96;
int os_logical_high = 96;
static const GUID _os_IID_IShellItem = {0x43826d1e,0xe718,0x42ee,{0xbc,0x55,0xa1,0xe2,0x61,0xc3,0x7b,0xfe}};
static const GUID _os_IID_IFileOpenDialog = {0xd57c7288,0xd4ad,0x4768,{0xbe,0x02,0x9d,0x96,0x95,0x32,0xd9,0x60}};
static const GUID _os_CLSID_FileOpenDialog = {0xdc1c5a9c,0xe88a,0x4dde,{0xa5,0xa1,0x60,0xf8,0x2a,0x20,0xae,0xf7}};
static const GUID _os_IID_IPropertyStore = {0x886d8eeb,0x8cf2,0x4446,{0x8d,0x02,0xcd,0xba,0x1d,0xbd,0xcf,0x99}};
static const GUID _os_FMTID_ImageProperties = {0x14B81DA1,0x0135,0x4D31,{0x96,0xD9,0x6C,0xBF,0xC9,0x67,0x1A,0x99}};
static HRESULT (WINAPI *_os_SHCreateItemFromIDList)(ITEMIDLIST *pidl,REFIID riid,void **ppv) = 0;
static HRESULT (WINAPI *_os_SHGetPropertyStoreFromIDList)(ITEMIDLIST *pidl,int flags,REFIID riid,void **ppv) = 0;

static COLORREF *_os_custom_colors = 0;
static HMODULE _os_shell32_hmodule = 0;
static HMODULE _os_user32_hmodule = 0;
static HMODULE _os_UxTheme_hmodule = 0;
static HMODULE _os_gdiplus_hmodule = 0;
static HMODULE _os_ucrtbase_hmodule = 0;
static HMODULE _os_gdi32_hmodule = 0;
static HMODULE _os_dwmapi_hmodule = 0;

void os_zero_memory(void *data,int size)
{
	ZeroMemory(data,size);
}

// size_t lengths: the clipboard dib paste copies stride*height bytes,
// which crosses int_max inside the 64-bit pixel budget.
void os_copy_memory(void *d,const void *s,SIZE_T size)
{
	CopyMemory(d,s,size);
}

void os_move_memory(void *d,const void *s,SIZE_T size)
{
	MoveMemory(d,s,size);
}

// make a rect fully visible
// pushes the rect onto the screen,the same as menus.
// does not allow monitor overlapping.
// if you have a window that is fully visible, but half is displayed on one display, and half on another, this function will
// push it onto just one of the displays. (the one with the most area shown)
// make sure you call os_is_rect_completely_visible before calling this.
void os_make_rect_completely_visible(HWND hwnd,RECT *prect)
{
	int wide;
	int high;
	RECT monitor_from_window_rect;
	RECT monitor_from_rect_rect;

	// we hit something so reposition the window on this monitor.
	// note that MonitorFromRect can return null, even when MONITOR_DEFAULTTOPRIMARY is specified.
	os_MonitorRectFromWindow(hwnd,0,&monitor_from_window_rect);
	os_MonitorRectFromRect(prect,0,&monitor_from_rect_rect);
	
	OffsetRect(prect,-monitor_from_rect_rect.left,-monitor_from_rect_rect.top);
	OffsetRect(prect,monitor_from_window_rect.left,monitor_from_window_rect.top);
	
	// get window width and height,
	wide = prect->right - prect->left;
	high = prect->bottom - prect->top;

	if (wide > monitor_from_window_rect.right - monitor_from_window_rect.left)
	{
		wide = monitor_from_window_rect.right - monitor_from_window_rect.left;
	}
	
	if (high > monitor_from_window_rect.bottom - monitor_from_window_rect.top)
	{
		high = monitor_from_window_rect.bottom - monitor_from_window_rect.top;
	}
	
	// push the window 
	if (prect->right > monitor_from_window_rect.right) 
	{
		prect->left = monitor_from_window_rect.right - wide;
		prect->right = monitor_from_window_rect.right;
	}
	
	if (prect->bottom > monitor_from_window_rect.bottom) 
	{
		prect->top = monitor_from_window_rect.bottom - high;
		prect->bottom = monitor_from_window_rect.bottom;
	}

	// push the window 
	if (prect->left < monitor_from_window_rect.left) 
	{
		prect->left = monitor_from_window_rect.left;
		prect->right = monitor_from_window_rect.left + wide;
	}
	
	if (prect->top < monitor_from_window_rect.top) 
	{
		prect->top = monitor_from_window_rect.top;
		prect->bottom = monitor_from_window_rect.top + high;
	}
}

void os_MonitorRectFromWindow(HWND hwnd,int is_fullscreen,RECT *out_monitor_rect)
{
	HMONITOR hmonitor;
	MONITORINFO mi;
	
	hmonitor = NULL;
	
	if (_os_MonitorFromWindow)
	{
		hmonitor = _os_MonitorFromWindow(hwnd,MONITOR_DEFAULTTOPRIMARY);
	}
	
	if (hmonitor)
	{
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hmonitor,&mi);
		
		if (is_fullscreen)
		{
			CopyRect(out_monitor_rect,&mi.rcMonitor);
		}
		else
		{
			CopyRect(out_monitor_rect,&mi.rcWork);
		}
	}
	else
	{
		if (is_fullscreen)
		{
			// full screen (which is the client area...)
			out_monitor_rect->left = 0;
			out_monitor_rect->top = 0;
			out_monitor_rect->right = GetSystemMetrics(SM_CXSCREEN);
			out_monitor_rect->bottom = GetSystemMetrics(SM_CYSCREEN);
		}
		else
		{
			// work area
			SystemParametersInfo(SPI_GETWORKAREA,0,(PVOID)out_monitor_rect,0);
		}
debug_printf("FULLSCREEN %d %d %d\n",is_fullscreen,out_monitor_rect->right,out_monitor_rect->bottom)	;
	}
}

void os_MonitorRectFromRect(RECT *window_rect,int is_fullscreen,RECT *out_monitor_rect)
{
	HMONITOR hmonitor;
	MONITORINFO mi;
	
	hmonitor = NULL;
	
	if (_os_MonitorFromRect)
	{
		hmonitor = _os_MonitorFromRect(window_rect,MONITOR_DEFAULTTOPRIMARY);
	}
	
	if (hmonitor)
	{
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hmonitor,&mi);
		
		if (is_fullscreen)
		{
			CopyRect(out_monitor_rect,&mi.rcMonitor);
		}
		else
		{
			CopyRect(out_monitor_rect,&mi.rcWork);
		}
	}
	else
	{
		if (is_fullscreen)
		{
			// full screen
			// use SM_CXSCREEN for full screen.
			// SM_CXFULLSCREEN is the client area...
			out_monitor_rect->left = 0;
			out_monitor_rect->top = 0;
			out_monitor_rect->right = GetSystemMetrics(SM_CXSCREEN);
			out_monitor_rect->bottom = GetSystemMetrics(SM_CYSCREEN);
		}
		else
		{
			// work area
			SystemParametersInfo(SPI_GETWORKAREA,0,(PVOID)out_monitor_rect,0);
		}
	}
}

void os_MonitorRectFromCursor(int is_fullscreen,RECT *out_monitor_rect)
{
	HMONITOR hmonitor;
	MONITORINFO mi;
	
	hmonitor = NULL;
	
	if (_os_MonitorFromPoint)
	{
		POINT cursor_pt;
		
		GetCursorPos(&cursor_pt);
		
		hmonitor = _os_MonitorFromPoint(cursor_pt,MONITOR_DEFAULTTOPRIMARY);
	}
	
	if (hmonitor)
	{
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hmonitor,&mi);
		
		if (is_fullscreen)
		{
			CopyRect(out_monitor_rect,&mi.rcMonitor);
		}
		else
		{
			CopyRect(out_monitor_rect,&mi.rcWork);
		}
	}
	else
	{
		if (is_fullscreen)
		{
			// full screen
			// use SM_CXSCREEN for full screen.
			// SM_CXFULLSCREEN is the client area...
			out_monitor_rect->left = 0;
			out_monitor_rect->top = 0;
			out_monitor_rect->right = GetSystemMetrics(SM_CXSCREEN);
			out_monitor_rect->bottom = GetSystemMetrics(SM_CYSCREEN);
		}
		else
		{
			// work area
			SystemParametersInfo(SPI_GETWORKAREA,0,(PVOID)out_monitor_rect,0);
		}
	}
}

// copied from MSVC CRT, modified to avoid a deref and use stdcall which is faster on my pc.
void os_qsort(void **base,uintptr_t count,int (*comp)(const void *,const void *))
{
    void **lo;
    void **hi;              /* ends of sub-array currently sorting */
    void **mid;                  /* points to middle of subarray */
    void **loguy;
    void **higuy;        /* traveling pointers for partition step */
    int size;                /* size of the sub-array */
    void **lostk[_OS_QSORT_STKSIZ];
    void **histk[_OS_QSORT_STKSIZ];
    int stkptr;                 /* stack for saving sub-array to be processed */
	void *tmp;

    if (count < 2)
    {
        return;                 /* nothing to do */
	}

    stkptr = 0;                 /* initialize stack */

    lo = base;
    hi = base + (count-1);        /* initialize limits */

    /* this entry point is for pseudo-recursion calling: setting
       lo and hi and jumping to here is like recursion,but stkptr is
       preserved,locals aren't,so we preserve stuff on the stack */
recurse:

    size = (int)((hi - lo) + 1);        /* number of el's to sort */

    /* below a certain size,it is faster to use a O(n^2) sorting method */
    if (size <= _OS_QSORT_CUTOFF) 
    {
        _os_qsort_indexes_shortsort(lo,hi,comp);
    }
    else {
        /* First we pick a partitioning element.  The efficiency of the
           algorithm demands that we find one that is approximately the median
           of the values,but also that we select one fast.  We choose the
           median of the first,middle,and last elements,to avoid bad
           performance in the face of already sorted data,or data that is made
           up of multiple sorted runs appended together.  Testing shows that a
           median-of-three algorithm provides better performance than simply
           picking the middle element for the latter case. */

        mid = lo + (size / 2);      /* find middle element */

        /* Sort the first,middle,last elements into order */
        if (comp(*lo,*mid) > 0) 
        {
			tmp = *lo;
			*lo = *mid;
			*mid = tmp;
        }
        
        if (comp(*lo,*hi) > 0) 
        {
			tmp = *lo;
			*lo = *hi;
			*hi = tmp;
        }
        
        if (comp(*mid,*hi) > 0) 
        {
			tmp = *mid;
			*mid = *hi;
			*hi = tmp;
        }

        /* We now wish to partition the array into three pieces,one consisting
           of elements <= partition element,one of elements equal to the
           partition element,and one of elements > than it.  This is done
           below; comments indicate conditions established at every step. */

        loguy = lo;
        higuy = hi;

        /* Note that higuy decreases and loguy increases on every iteration,
           so loop must terminate. */
        for (;;) 
        {
            /* lo <= loguy < hi,lo < higuy <= hi,
               A[i] <= A[mid] for lo <= i <= loguy,
               A[i] > A[mid] for higuy <= i < hi,
               A[hi] >= A[mid] */

            /* The doubled loop is to avoid calling comp(mid,mid),since some
               existing comparison funcs don't work when passed the same
               value for both pointers. */

            if (mid > loguy) {
                do  {
                    loguy++;
                } while ((loguy < mid) && (comp(*loguy,*mid) <= 0));
            }
            if (mid <= loguy) {
                do  {
                    loguy++;
                } while ((loguy <= hi) && (comp(*loguy,*mid) <= 0));
            }

            /* lo < loguy <= hi+1,A[i] <= A[mid] for lo <= i < loguy,
               either loguy > hi or A[loguy] > A[mid] */

            do  {
                higuy--;
            } while ((higuy > mid) && (comp(*higuy,*mid) > 0));

            /* lo <= higuy < hi,A[i] > A[mid] for higuy < i < hi,
               either higuy == lo or A[higuy] <= A[mid] */

            if (higuy < loguy)
                break;

            /* if loguy > hi or higuy == lo,then we would have exited,so
               A[loguy] > A[mid],A[higuy] <= A[mid],
               loguy <= hi,higuy > lo */

			tmp = *loguy;
			*loguy = *higuy;
			*higuy = tmp;

            /* If the partition element was moved,follow it.  Only need
               to check for mid == higuy,since before the swap,
               A[loguy] > A[mid] implies loguy != mid. */

            if (mid == higuy)
                mid = loguy;

            /* A[loguy] <= A[mid],A[higuy] > A[mid]; so condition at top
               of loop is re-established */
        }

        /*     A[i] <= A[mid] for lo <= i < loguy,
               A[i] > A[mid] for higuy < i < hi,
               A[hi] >= A[mid]
               higuy < loguy
           implying:
               higuy == loguy-1
               or higuy == hi - 1,loguy == hi + 1,A[hi] == A[mid] */

        /* Find adjacent elements equal to the partition element.  The
           doubled loop is to avoid calling comp(mid,mid),since some
           existing comparison funcs don't work when passed the same value
           for both pointers. */

        higuy++;
        if (mid < higuy) {
            do  {
                higuy--;
            } while ((higuy > mid) && (comp(*higuy,*mid)) == 0);
        }
        if (mid >= higuy) {
            do  {
                higuy--;
            } while ((higuy > lo) && (comp(*higuy,*mid)) == 0);
        }

        /* OK,now we have the following:
              higuy < loguy
              lo <= higuy <= hi
              A[i]  <= A[mid] for lo <= i <= higuy
              A[i]  == A[mid] for higuy < i < loguy
              A[i]  >  A[mid] for loguy <= i < hi
              A[hi] >= A[mid] */

        /* We've finished the partition,now we want to sort the subarrays
           [lo,higuy] and [loguy,hi].
           We do the smaller one first to minimize stack usage.
           We only sort arrays of length 2 or more.*/

        if ( higuy - lo >= hi - loguy ) {
            if (lo < higuy) {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy;
                ++stkptr;
            }                           /* save big recursion for later */

            if (loguy < hi) {
                lo = loguy;
                goto recurse;           /* do small recursion */
            }
        }
        else {
            if (loguy < hi) {
                lostk[stkptr] = loguy;
                histk[stkptr] = hi;
                ++stkptr;               /* save big recursion for later */
            }

            if (lo < higuy) {
                hi = higuy;
                goto recurse;           /* do small recursion */
            }
        }
    }

    /* We have sorted the array,except for any pending sorts on the stack.
       Check if there are any,and do them. */

    --stkptr;
    if (stkptr >= 0) {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse;           /* pop subarray from stack */
    }
    else
        return;                 /* all subarrays done */
	
}

static void _os_qsort_indexes_shortsort(void **lo,void **hi,int (*comp)(const void *,const void *))
{
    void **p;
    void *tmp;
    void **maxp;

    /* Note: in assertions below,i and j are alway inside original bound of
       array to sort. */

    while (hi > lo) 
    {
        /* A[i] <= A[j] for i <= j,j > hi */
        maxp = lo;
        
        for (p = lo+1; p <= hi; p++) 
        {
            /* A[i] <= A[maxp] for lo <= i < p */
            
            if (comp(*p,*maxp) > 0) 
            {
                maxp = p;
            }
            
            /* A[i] <= A[maxp] for lo <= i <= p */
        }

        /* A[i] <= A[maxp] for lo <= i <= hi */

		tmp = *maxp;
		*maxp = *hi;
		*hi = tmp;

        /* A[i] <= A[hi] for i <= hi,so A[i] <= A[j] for i <= j,j >= hi */

        hi--;

        /* A[i] <= A[j] for i <= j,j > hi,loop top condition established */
    }
    
    /* A[i] <= A[j] for i <= j,j > lo,which implies A[i] <= A[j] for i < j,
       so array is sorted */
}

int os_ComboBox_AddString(HWND hwnd,int id,const utf8_t *s)
{
	wchar_t wbuf[STRING_SIZE];
	
	string_copy_utf8_string(wbuf,s);
	
	return ComboBox_AddString(GetDlgItem(hwnd,id),wbuf);
}

int os_ComboBox_AddString_localization_id(HWND hwnd,int id,localization_id_t localization_id)
{
	return os_ComboBox_AddString(hwnd,id,localization_get_string(localization_id));
}

void os_SetDlgItemText(HWND hwnd,int id,const utf8_t *s)
{	
	wchar_t wbuf[STRING_SIZE];
	
	string_copy_utf8_string(wbuf,s);
	
	SetDlgItemText(hwnd,id,wbuf);
}

void os_SetWindowText(HWND hwnd,const utf8_t *s)
{	
	wchar_t wbuf[STRING_SIZE];
	
	string_copy_utf8_string(wbuf,s);
	
	SetWindowText(hwnd,wbuf);
}

void os_SetDlgItemText_localization_id(HWND hwnd,int id,localization_id_t localization_id)
{	
	os_SetDlgItemText(hwnd,id,localization_get_string(localization_id));
}

void os_SetWindowText_localization_id(HWND hwnd,localization_id_t localization_id)
{	
	os_SetWindowText(hwnd,localization_get_string(localization_id));
}

HWND os_CreateWindowEx(DWORD dwExStyle,const utf8_t *lpClassName,const utf8_t *lpWindowName,DWORD dwStyle,int X,int Y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam)
{
	wchar_t lpClassName_wbuf[STRING_SIZE];
	wchar_t lpWindowName_wbuf[STRING_SIZE];
	
	string_copy_utf8_string(lpClassName_wbuf,lpClassName);
	string_copy_utf8_string(lpWindowName_wbuf,lpWindowName);
	
	return CreateWindowEx(
		dwExStyle,
		lpClassName_wbuf,
		lpWindowName_wbuf,
		dwStyle,
		X,Y,nWidth,nHeight,
		hWndParent,hMenu,hInstance,lpParam);
}

void os_RegisterClassEx(UINT style,WNDPROC lpfnWndProc,HICON hIcon,HCURSOR hCursor,HBRUSH hbrBackground,const utf8_t *name,HICON hIconSm)
{
	WNDCLASSEXW wcex;
	wchar_t name_wbuf[STRING_SIZE];
	
	string_copy_utf8_string(name_wbuf,name);

	// Initialize global strings
	os_zero_memory(&wcex,sizeof(WNDCLASSEXW));
	wcex.cbSize = sizeof(WNDCLASSEXW); 

	wcex.style = style;
	wcex.lpfnWndProc = lpfnWndProc;
	wcex.hInstance = os_hinstance;
	wcex.hIcon = hIcon;
	// the wrapper honored neither the cursor nor the brush it was given:
	// every class registered with the light window brush (white), so the
	// rebar slabs, the zoomui tray and any bypassing erase flashed white
	// in the dark ui no matter what the callers wrote. the parameters are
	// the contract now - a caller that wants the classic faces passes
	// them, a caller that paints its own face passes no brush at all.
	wcex.hCursor = hCursor;
	wcex.hbrBackground = hbrBackground;
	wcex.lpszClassName = name_wbuf;
	wcex.hIconSm = hIconSm;
	
	RegisterClassExW(&wcex);
}

int os_is_admin(void)
{
	// vista or later?
	if (os_major_version >= 6)
	{
		if (os_IsUserAnAdmin)
		{
			if (os_IsUserAnAdmin())
			{
				return 1;
			}
		}
		
		return 0;
	}
	
	// assume admin
	return 1;
}

int os_choose_color(HWND parent,COLORREF *colorref)
{
	CHOOSECOLOR cc;

	if (!_os_custom_colors)
	{
		int i;

		_os_custom_colors = (COLORREF *)mem_alloc(sizeof(COLORREF) * 16);
		
		for(i=0;i<16;i++)
		{
			_os_custom_colors[i] = RGB(255,255,255);
		}
	}

	os_zero_memory(&cc,sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = parent;
	cc.rgbResult = *colorref;
	cc.lpCustColors = _os_custom_colors;
	cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

	if (ChooseColor(&cc))
	{
		*colorref = cc.rgbResult;

		return 1;
	}
	
	return 0;
}

static FARPROC _os_get_proc_address(HMODULE hmodule,const char *name)
{
	FARPROC proc;
	
	proc = GetProcAddress(hmodule,name);
	if (!proc) 
	{
		debug_fatal("missing proc %s",name);
	}
	
	return proc;
}

void os_init(void)
{
	HMODULE kernel32_hmodule;

	// OS Version
	{
		OSVERSIONINFOA osvi;
		
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
		GetVersionExA(&osvi);

		os_major_version = (char)osvi.dwMajorVersion;
		os_minor_version = (char)osvi.dwMinorVersion;
		os_build_number = osvi.dwBuildNumber;
		os_is_nt = (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);

		debug_printf("os %d\n",os_major_version);
	}
	
	// logical width and height.
	{
		HDC hdc;
		
		hdc = GetDC(0);
		
		os_logical_wide = GetDeviceCaps(hdc,LOGPIXELSX);
		os_logical_high = GetDeviceCaps(hdc,LOGPIXELSY);

		ReleaseDC(0,hdc);
	}	
	
	kernel32_hmodule = GetModuleHandleA("kernel32.dll");
	if (kernel32_hmodule)
	{
		os_CreateTimerQueueTimer  = (void *)GetProcAddress(kernel32_hmodule,"CreateTimerQueueTimer");
		os_DeleteTimerQueueTimer  = (void *)GetProcAddress(kernel32_hmodule,"DeleteTimerQueueTimer");
		_os_SetDllDirectoryW  = (void *)GetProcAddress(kernel32_hmodule,"SetDllDirectoryW");
		_os_SetDefaultDllDirectories = (void *)GetProcAddress(kernel32_hmodule,"SetDefaultDllDirectories");
		os_GetFileAttributesExW = (void *)GetProcAddress(kernel32_hmodule,"GetFileAttributesExW");
		_os_IsDebuggerPresent = (void *)GetProcAddress(kernel32_hmodule,"IsDebuggerPresent");
		os_SetThreadExecutionState = (void *)GetProcAddress(kernel32_hmodule,"SetThreadExecutionState");
		os_GetUserDefaultUILanguage = (void *)GetProcAddress(kernel32_hmodule,"GetUserDefaultUILanguage");
	}

	// system dlls only.
	if (_os_SetDllDirectoryW)
	{
		// prevent dlls from loading other dlls from the current dir.
		_os_SetDllDirectoryW(L"");
	}
	
	// system dlls only.
	if (_os_SetDefaultDllDirectories)
	{
		// #define LOAD_LIBRARY_SEARCH_SYSTEM32 0x00000800
		_os_SetDefaultDllDirectories(0x00000800);
	}

	_os_shell32_hmodule = LoadLibraryA("shell32.dll");
	if (_os_shell32_hmodule)
	{
		os_SHOpenFolderAndSelectItems = (void *)GetProcAddress(_os_shell32_hmodule,"SHOpenFolderAndSelectItems");
		os_IsUserAnAdmin = (void *)GetProcAddress(_os_shell32_hmodule,"IsUserAnAdmin");
		_os_SHCreateItemFromIDList = (void *)GetProcAddress(_os_shell32_hmodule,"SHCreateItemFromIDList");
		_os_SHGetPropertyStoreFromIDList = (void *)GetProcAddress(_os_shell32_hmodule,"SHGetPropertyStoreFromIDList");
	}

	_os_user32_hmodule = LoadLibraryA("user32.dll");
	if (_os_user32_hmodule)
	{
		os_ChangeWindowMessageFilterEx = (void *)GetProcAddress(_os_user32_hmodule,"ChangeWindowMessageFilterEx");
		os_SetGestureConfig = (void *)GetProcAddress(_os_user32_hmodule,"SetGestureConfig");
		os_GetGestureInfo = (void *)GetProcAddress(_os_user32_hmodule,"GetGestureInfo");
		os_CloseGestureInfoHandle = (void *)GetProcAddress(_os_user32_hmodule,"CloseGestureInfoHandle");
		_os_MonitorFromWindow = (void *)GetProcAddress(_os_user32_hmodule,"MonitorFromWindow");
		_os_MonitorFromRect = (void *)GetProcAddress(_os_user32_hmodule,"MonitorFromRect");
		_os_MonitorFromPoint = (void *)GetProcAddress(_os_user32_hmodule,"MonitorFromPoint");
		_os_GetDpiForWindow = (void *)GetProcAddress(_os_user32_hmodule,"GetDpiForWindow");
		_os_SystemParametersInfoForDpi = (void *)GetProcAddress(_os_user32_hmodule,"SystemParametersInfoForDpi");
		_os_SetThreadDpiAwarenessContext = (void *)GetProcAddress(_os_user32_hmodule,"SetThreadDpiAwarenessContext");
		_os_SetProcessDpiAwarenessContext = (void *)GetProcAddress(_os_user32_hmodule,"SetProcessDpiAwarenessContext");
		_os_SetProcessDPIAware = (void *)GetProcAddress(_os_user32_hmodule,"SetProcessDPIAware");
	}

	// dpi awareness: the embedded manifest normally declares per monitor
	// v2 (res/voidImageViewer.Manifest, merged by the vs linker through
	// AdditionalManifestFiles; a viv.exe.manifest next to the exe covers
	// manifest-less builds). the runtime claim below is the safety net
	// for a manifest-less binary (repackaged or stripped exes): without
	// it the process stays dpi unaware, the system bitmap-stretches every
	// window at high dpi and the ui turns blurry and oversized.
	// the claim must happen before the first window exists: os_init is
	// exactly that point, and the GetProcAddress block above must come
	// first -- this used to sit before the pointers were resolved and
	// silently never fired.
	// each tier is a guarded no-op on systems without the api:
	// win10 1703+ process wide, win10 1607+ per thread (covers the ui
	// thread this process creates every window on), win 8.1 shcore
	// process claim, and win 7 / vista close the chain with the classic
	// setprocessdpiaware claim - without that last tier a manifest-less
	// binary on 7 stayed fully unaware and the system bitmap-stretched
	// everything (the manifest carries the same ladder through its dual
	// dpiAware / dpiAwareness declaration).
	if (_os_SetProcessDpiAwarenessContext)
	{
		// DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = ((DPI_CONTEXT_HANDLE)-4)
		_os_SetProcessDpiAwarenessContext((HANDLE)-4);
	}
	else if (_os_SetThreadDpiAwarenessContext)
	{
		_os_SetThreadDpiAwarenessContext((HANDLE)-4);
	}
	else
	{
		int claimed;

		claimed = 0;

		_os_shcore_hmodule = LoadLibraryA("shcore.dll");

		if (_os_shcore_hmodule)
		{
			_os_SetProcessDpiAwareness = (void *)GetProcAddress(_os_shcore_hmodule,"SetProcessDpiAwareness");

			// PROCESS_PER_MONITOR_DPI_AWARE = 2.
			if (_os_SetProcessDpiAwareness)
			{
				_os_SetProcessDpiAwareness(2);

				claimed = 1;
			}
		}

		// win 7 / vista: the classic claim. a manifest-declared process
		// is already aware - the call fails harmlessly there.
		if ((!claimed) && (_os_SetProcessDPIAware))
		{
			_os_SetProcessDPIAware();
		}
	}

	_os_UxTheme_hmodule = LoadLibraryA("UxTheme.dll");
	if (_os_UxTheme_hmodule)
	{
		os_EnableThemeDialogTexture = (void *)GetProcAddress(_os_UxTheme_hmodule,"EnableThemeDialogTexture");
		
		// the dark explorer visual style for dialog controls (named export).
		_os_SetWindowTheme = (void *)GetProcAddress(_os_UxTheme_hmodule,"SetWindowTheme");
		
		// windows 10 dark mode: uxtheme exports these by ordinal only. on
		// windows 7/8 and pre-1809 builds they do not resolve and every dark
		// function below quietly does nothing.
		_os_SetPreferredAppMode = (void *)GetProcAddress(_os_UxTheme_hmodule,MAKEINTRESOURCEA(135));
		_os_FlushMenuThemes = (void *)GetProcAddress(_os_UxTheme_hmodule,MAKEINTRESOURCEA(136));
		_os_RefreshImmersiveColorPolicyState = (void *)GetProcAddress(_os_UxTheme_hmodule,MAKEINTRESOURCEA(104));
		_os_ShouldAppsUseDarkMode = (void *)GetProcAddress(_os_UxTheme_hmodule,MAKEINTRESOURCEA(132));
		_os_AllowDarkModeForWindow = (void *)GetProcAddress(_os_UxTheme_hmodule,MAKEINTRESOURCEA(133));		
		// the button part metrics (the glyph label offset).
		_os_OpenThemeData = (void *)GetProcAddress(_os_UxTheme_hmodule,"OpenThemeData");
		_os_CloseThemeData = (void *)GetProcAddress(_os_UxTheme_hmodule,"CloseThemeData");
		_os_GetThemePartSize = (void *)GetProcAddress(_os_UxTheme_hmodule,"GetThemePartSize");
		_os_DrawThemeBackground = (void *)GetProcAddress(_os_UxTheme_hmodule,"DrawThemeBackground");

	}
	
	_os_dwmapi_hmodule = LoadLibraryA("dwmapi.dll");
	if (_os_dwmapi_hmodule)
	{
		_os_DwmSetWindowAttribute = (void *)GetProcAddress(_os_dwmapi_hmodule,"DwmSetWindowAttribute");
	}
	
	// advapi32: registry reads for the dark mode detection.
	_os_advapi32_hmodule = LoadLibraryA("advapi32.dll");
	if (_os_advapi32_hmodule)
	{
		_os_RegOpenKeyExW = (void *)GetProcAddress(_os_advapi32_hmodule,"RegOpenKeyExW");
		_os_RegQueryValueExW = (void *)GetProcAddress(_os_advapi32_hmodule,"RegQueryValueExW");
		_os_RegCloseKey = (void *)GetProcAddress(_os_advapi32_hmodule,"RegCloseKey");
	}
	
	// set "floating point to int mode" to truncate
	_os_ucrtbase_hmodule = LoadLibraryA("ucrtbase.dll");
	if (_os_ucrtbase_hmodule)
	{
		_os_controlfp = (void *)GetProcAddress(_os_ucrtbase_hmodule,"_controlfp");
		if (_os_controlfp)
		{
			// _RC_CHOP = 0x00000300
			// _MCW_RC = 0x00000300
			_os_controlfp(0x00000300,0x00000300);
		}		
	}

	_os_gdi32_hmodule = LoadLibraryA("gdi32.dll");
	if (_os_gdi32_hmodule)
	{
		os_GetLayout = (void *)GetProcAddress(_os_gdi32_hmodule,"GetLayout");
	}

	_os_gdiplus_hmodule = LoadLibraryA("gdiplus.dll");
	if (_os_gdiplus_hmodule)
	{
		os_GdiplusStartup = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdiplusStartup");
		os_GdiplusShutdown = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdiplusShutdown");
		os_GdipLoadImageFromStream = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipLoadImageFromStream");
		os_GdipLoadImageFromStreamICM = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipLoadImageFromStreamICM");
		os_GdipGetImageWidth = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipGetImageWidth");
		os_GdipGetImageHeight = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipGetImageHeight");
		os_GdipImageGetFrameDimensionsCount = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipImageGetFrameDimensionsCount");
		os_GdipImageGetFrameDimensionsList = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipImageGetFrameDimensionsList");
		os_GdipImageGetFrameCount = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipImageGetFrameCount");
		os_GdipGetPropertyItemSize = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipGetPropertyItemSize");
		// optional: gdi+ serves the embedded exif thumbnail without decoding
		// the full image, but not every gdiplus.dll exports GdipGetImageThumbnail.
		// a missing export must never be fatal at startup (it would kill the
		// /install self-install step), the progressive preview quietly turns off.
		os_GdipGetImageThumbnail = (void *)GetProcAddress(_os_gdiplus_hmodule,"GdipGetImageThumbnail");
		os_GdipGetPropertyItem = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipGetPropertyItem");
		os_GdipImageSelectActiveFrame = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipImageSelectActiveFrame");
		os_GdipGetImageFlags = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipGetImageFlags");
		os_GdipDisposeImage = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipDisposeImage");
		os_GdipCreateFromHDC = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipCreateFromHDC");
		os_GdipSetCompositingMode = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipSetCompositingMode");
		os_GdipSetCompositingQuality = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipSetCompositingQuality");
		os_GdipSetInterpolationMode = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipSetInterpolationMode");
		os_GdipSetPixelOffsetMode = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipSetPixelOffsetMode");
		os_GdipSetSmoothingMode = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipSetSmoothingMode");
		os_GdipDrawImageRectI = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipDrawImageRectI");
		os_GdipDeleteGraphics = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipDeleteGraphics");
		os_GdipCreateBitmapFromHBITMAP = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipCreateBitmapFromHBITMAP");
		os_GdipSaveImageToFile = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipSaveImageToFile");
		os_GdipGetImageEncodersSize = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipGetImageEncodersSize");
		os_GdipGetImageEncoders = (void *)_os_get_proc_address(_os_gdiplus_hmodule,"GdipGetImageEncoders");
	}
}

// is a touch digitizer available?
// (integrated or external touch screen)
int os_is_touch_available(void)
{
	int sm;

	// SM_DIGITIZER = 94
	sm = GetSystemMetrics(94);

	if (!sm)
	{
		return 0;
	}

	// NID_READY = 0x80, NID_INTEGRATED_TOUCH = 0x01, NID_EXTERNAL_TOUCH =
	// 0x02 (and NID_INTEGRATED_PEN = 0x04 - the pen does not belong in the
	// mask: a pen-only digitizer passed the old probe and an external-only
	// touch screen failed it). nid_ready alone is set by any ready
	// digitizer (pens included): ask for an actual touch screen - the
	// integrated or the external one - and only while the digitizer is
	// ready. msdn
	// also warns sm_digitizer has no plug-and-play awareness, so callers
	// treat this as a first-run default, never a permanent configuration.
	return ((sm & 0x80) && (sm & (0x01 | 0x02))) ? 1 : 0;
}

int os_reg_delete_key_ex(HKEY hkey,const wchar_t *name,REGSAM access)
{
	// regdeletekeyw cannot delete from the alternate registry view (msdn:
	// "cannot be used to access an alternate registry view") and
	// regdeletekeyexw is vista+, so it resolves lazily here: the uninstall
	// path runs before os_init populates the runtime table. a missing api
	// (32-bit xp) reports failure and the caller falls back to
	// regdeletekeyw.
	if (!_os_RegDeleteKeyExW)
	{
		HMODULE module;

		module = GetModuleHandleA("advapi32.dll");

		if (module)
		{
			_os_RegDeleteKeyExW = (void *)GetProcAddress(module,"RegDeleteKeyExW");
		}
	}

	if (_os_RegDeleteKeyExW)
	{
		return (_os_RegDeleteKeyExW(hkey,name,access,0) == ERROR_SUCCESS) ? 1 : 0;
	}

	return 0;
}

// GDI+ encoder parameter structures. (locally defined, mirrors the gdiplus ABI)
typedef struct
{
	GUID Guid; // encoder parameter GUID.
	unsigned long NumberOfValues; // number of values pointed to by Value.
	unsigned long Type; // value type. (4 = long)
	void *Value; // pointer to the value.
} os_EncoderParameter_t;

typedef struct
{
	unsigned int Count; // number of parameters.
	os_EncoderParameter_t Parameter[1];
} os_EncoderParameters_t;

// GDI+ image codec info. (locally defined, mirrors the gdiplus ABI)
typedef struct
{
	GUID Clsid;
	GUID FormatID;
	const wchar_t *CodecName;
	const wchar_t *DllName;
	const wchar_t *FormatDescription;
	const wchar_t *FilenameExtension;
	const wchar_t *MimeType;
	unsigned long Flags;
	unsigned long Version;
	unsigned long SigCount;
	unsigned long SigSize;
	const void *SigPattern;
	const void *SigMask;
} os_ImageCodecInfo_t;

// ascii, case-insensitive wide string compare. returns 0 if equal.
static int _os_icompare_w(const wchar_t *a,const wchar_t *b)
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

// save a HBITMAP to a file with the GDI+ built-in encoders.
// format: 0 = png, 1 = jpeg, 2 = bmp.
// returns 1 on success.
int os_save_hbitmap(HBITMAP hbitmap,const wchar_t *filename,int format)
{
	int ret;
	void *image;
	
	ret = 0;
	image = 0;
	
	if (os_GdipCreateBitmapFromHBITMAP)
	{
		if (os_GdipCreateBitmapFromHBITMAP(hbitmap,0,&image) == 0)
		{
			if (image)
			{
				const wchar_t *mime;
				
				// mime type of the built-in encoder for the target format.
				mime = (format == 1) ? L"image/jpeg" : ((format == 2) ? L"image/bmp" : L"image/png");
				
				// find the encoder CLSID for this mime type.
				if ((os_GdipGetImageEncodersSize) && (os_GdipGetImageEncoders) && (os_GdipSaveImageToFile))
				{
					unsigned int num;
					unsigned int size;
					
					num = 0;
					size = 0;
					
					if (os_GdipGetImageEncodersSize(&num,&size) == 0)
					{
						if ((num) && (size))
						{
							os_ImageCodecInfo_t *codecs;
							
							codecs = (os_ImageCodecInfo_t *)mem_alloc(size);
							
							if (codecs)
							{
								os_zero_memory(codecs,size);
								
								if (os_GdipGetImageEncoders(num,size,codecs) == 0)
								{
									unsigned int i;
									
									for(i=0;i<num;i++)
									{
										if (_os_icompare_w(codecs[i].MimeType,mime) == 0)
										{
											int save_ret;
											
											save_ret = 1;
											
											if (format == 1)
											{
												static const GUID encoder_quality = {0x1d5be4b5,0xfa4a,0x452d,{0x9c,0xdd,0x5d,0xb3,0x51,0x05,0xe7,0xeb}};
												os_EncoderParameters_t params;
												unsigned long quality;
												
												// jpeg: request quality 90.
												quality = 90;
												
												os_zero_memory(&params,sizeof(params));
												
												params.Count = 1;
												params.Parameter[0].Guid = encoder_quality;
												params.Parameter[0].Type = 4;
												params.Parameter[0].NumberOfValues = 1;
												params.Parameter[0].Value = &quality;
												
												save_ret = os_GdipSaveImageToFile(image,filename,&codecs[i].Clsid,&params);
												
												// if the quality parameter was rejected, retry with the encoder defaults.
												if (save_ret != 0)
												{
													save_ret = os_GdipSaveImageToFile(image,filename,&codecs[i].Clsid,0);
												}
											}
											else
											{
												save_ret = os_GdipSaveImageToFile(image,filename,&codecs[i].Clsid,0);
											}
											
											ret = (save_ret == 0);
											
											break;
										}
									}
								}
								
								mem_free(codecs);
							}
						}
					}
				}
				
				os_GdipDisposeImage(image);
			}
		}
	}
	
	return ret;
}

// is the system windows theme dark? never in high contrast mode: dark
// overrides would break accessibility color themes. the result is cached:
// the ui queries the dark state in paint paths, so the system is probed
// once and the cache is dropped on WM_SETTINGCHANGE / WM_THEMECHANGED.
int os_dark_system_dark(void)
{
	HIGHCONTRASTW high_contrast;
	
	if (_os_dark_cache_valid)
	{
		return _os_dark_cache_dark;
	}
	
	_os_dark_cache_dark = 0;
	
	high_contrast.cbSize = sizeof(high_contrast);
	
	if (SystemParametersInfoW(SPI_GETHIGHCONTRAST,sizeof(high_contrast),&high_contrast,0))
	{
		if (high_contrast.dwFlags & HCF_HIGHCONTRASTON)
		{
			_os_dark_cache_valid = 1;
			
			return _os_dark_cache_dark;
		}
	}
	
	// primary source: the personalize registry value. 0 means the apps
	// theme is dark. this documented value behaves identically on every
	// windows 10/11 build, unlike the uxtheme ordinal probe.
	if (_os_RegOpenKeyExW && _os_RegQueryValueExW && _os_RegCloseKey)
	{
		HKEY key;
		
		if (_os_RegOpenKeyExW(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",0,KEY_QUERY_VALUE,&key) == 0)
		{
			DWORD type;
			DWORD apps_use_light;
			DWORD size;
			
			size = sizeof(apps_use_light);
			
			if (_os_RegQueryValueExW(key,L"AppsUseLightTheme",0,&type,(unsigned char *)&apps_use_light,&size) == 0)
			{
				if (type == REG_DWORD)
				{
					_os_dark_cache_dark = (apps_use_light == 0) ? 1 : 0;
					_os_dark_cache_valid = 1;
					
					_os_RegCloseKey(key);
					
					return _os_dark_cache_dark;
				}
			}
			
			_os_RegCloseKey(key);
		}
	}
	
	// fallback when the registry value is missing or unreadable: the
	// undocumented uxtheme probe.
	if (_os_ShouldAppsUseDarkMode)
	{
		_os_dark_cache_dark = _os_ShouldAppsUseDarkMode() ? 1 : 0;
	}
	
	_os_dark_cache_valid = 1;
	
	return _os_dark_cache_dark;
}

// drop the cached dark state. call when the system settings may have
// changed (WM_SETTINGCHANGE, WM_THEMECHANGED) so the next query re-reads.
void os_dark_invalidate(void)
{
	_os_dark_cache_valid = 0;
}

// the button theme for the custom drawn glyph faces: one open per
// dialog lifetime (the notify path draws every glyph with the same
// handle - an open and a close pair per notification was pure uxtheme
// bookkeeping on the paint hot path).
HANDLE os_theme_open_button(HWND hwnd)
{
	if (_os_OpenThemeData)
	{
		return _os_OpenThemeData(hwnd,L"Button");
	}
	
	return 0;
}

// close a theme handle this layer opened (a null handle is a no-op, so
// the callers need no gate of their own).
void os_theme_close(HANDLE theme)
{
	if ((theme) && (_os_CloseThemeData))
	{
		_os_CloseThemeData(theme);
	}
}

// the size of a button part in pixels: 1 when the theme and the part
// are available. the visual styles off leave the handle null and the
// caller keeps its native-painting fallback.
int os_theme_part_size(HANDLE theme,HDC hdc,int part,int state,int *wide,int *high)
{
	SIZE size;
	
	if ((!theme) || (!_os_GetThemePartSize))
	{
		return 0;
	}
	
	size.cx = 0;
	
	size.cy = 0;
	
	if (_os_GetThemePartSize(theme,hdc,part,state,0,2 /* TS_DRAW */,&size) != 0)
	{
		return 0;
	}
	
	if (wide)
	{
		*wide = (int)size.cx;
	}
	
	if (high)
	{
		*high = (int)size.cy;
	}
	
	return 1;
}

// draw a button part state into a rect: 1 when the theme drew it.
int os_theme_draw_part(HANDLE theme,HDC hdc,int part,int state,const RECT *rect)
{
	if ((!theme) || (!_os_DrawThemeBackground))
	{
		return 0;
	}
	
	return (_os_DrawThemeBackground(theme,hdc,part,state,rect) == 0) ? 1 : 0;
}

// set the dark explorer visual style on a window (dialog controls and tab
// controls draw dark with it). returns 1 when the style was applied.
int os_dark_window_theme(HWND hwnd)
{
	if (_os_SetWindowTheme)
	{
		if (_os_SetWindowTheme(hwnd,L"DarkMode_Explorer",0) == 0)
		{
			return 1;
		}
	}
	
	return 0;
}

// comboboxes have no parts in the darkmode explorer style (the dark
// dialogs kept light frames and arrows on 1903+ builds): the common
// dialog class carries them. the owner drawn items carry the field
// and the list rows on every build.
int os_dark_combobox_theme(HWND hwnd)
{
	if (_os_SetWindowTheme)
	{
		if (_os_SetWindowTheme(hwnd,L"DarkMode_CFD",0) == 0)
		{
			return 1;
		}
	}
	
	return 0;
}

// the light counterpart of the two dark theme classes above: hands a
// control back to the standard explorer visual style. used when the ui
// flips back to light after a control carried a dark class. returns 1
// when the style was applied.
int os_light_window_theme(HWND hwnd)
{
        if (_os_SetWindowTheme)
        {
                if (_os_SetWindowTheme(hwnd,L"Explorer",0) == 0)
                {
                        return 1;
                }
        }
        
        return 0;
}
// opt a single window (usually a dialog child control) into the dark
// comctl styles. must be called before os_dark_window_theme for the
// control. returns 1 when the dark mode api is available.
int os_allow_dark_mode_for_window(HWND hwnd,int allow)
{
	if (_os_AllowDarkModeForWindow)
	{
		_os_AllowDarkModeForWindow(hwnd,allow);
		
		return 1;
	}
	
	return 0;
}
// set the app menu theme. mode 0 = light, 1 = dark, 2 = follow the system.
void os_dark_set_app_mode(int mode)
{
	if (_os_SetPreferredAppMode)
	{
		// PreferredAppMode: Default = 0, AllowDark = 1, ForceDark = 2, ForceLight = 3.
		switch(mode)
		{
			case 0:
				_os_SetPreferredAppMode(3);
				break;
				
			case 1:
				_os_SetPreferredAppMode(2);
				break;
				
			default:
				_os_SetPreferredAppMode(1);
				break;
		}
	}
	
	os_dark_refresh();
}

// draw a window frame (title bar) in dark or light colors.
void os_dark_titlebar(HWND hwnd,int dark)
{
	BOOL value;
	HRESULT result;
	
	if (!hwnd)
	{
		return;
	}
	
	value = dark ? TRUE : FALSE;
	
	if (_os_AllowDarkModeForWindow)
	{
		_os_AllowDarkModeForWindow(hwnd,dark);
	}
	
	if (!_os_DwmSetWindowAttribute)
	{
		return;
	}
	
	// DWMWA_USE_IMMERSIVE_DARK_MODE is 20 on windows 10 2004 and newer;
	// windows 10 1809-1909 used 19 and fails with E_INVALIDARG for 20.
	result = _os_DwmSetWindowAttribute(hwnd,20,&value,sizeof(value));
	
	if (result == (HRESULT)0x80070057)
	{
		// E_INVALIDARG: retry with the older attribute.
		_os_DwmSetWindowAttribute(hwnd,19,&value,sizeof(value));
	}
}

// refresh os_logical_wide / os_logical_high from the window's current
// monitor dpi (per monitor v2). returns 1 when the values changed.
// when the api is missing (pre windows 10 1607) or reports nothing the
// init time constants stay and the caller keeps the old scaling.
int os_window_update_dpi(HWND hwnd)
{
	UINT dpi;
	
	if ((!hwnd) || (!_os_GetDpiForWindow))
	{
		return 0;
	}
	
	dpi = _os_GetDpiForWindow(hwnd);
	
	if (!dpi)
	{
		return 0;
	}
	
	// never scale below 96 dpi: sub 96 values would shrink every metric.
	if (dpi < 96)
	{
		dpi = 96;
	}
	
	if (((int)dpi == os_logical_wide) && ((int)dpi == os_logical_high))
	{
		return 0;
	}
	
	os_logical_wide = (int)dpi;
	os_logical_high = (int)dpi;
	
	return 1;
}

// fill lf with the menu font the system metrics use for the current
// window dpi (per monitor v2 aware so the menu bar text matches the
// bar the system sized for this dpi). returns 1 on success.
int os_menu_font(LOGFONTW *lf)
{
	NONCLIENTMETRICSW ncm;
	
	if (!lf)
	{
		return 0;
	}
	
	os_zero_memory(&ncm,sizeof(ncm));
	
	ncm.cbSize = sizeof(ncm);
	
	if (_os_SystemParametersInfoForDpi)
	{
		// uiParam must match cbSize for the metric queries.
		if (_os_SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS,sizeof(ncm),&ncm,0,(UINT)os_logical_wide))
		{
			*lf = ncm.lfMenuFont;
			
			return 1;
		}
	}
	
	// pre 1607 systems (and any ForDpi refusal): the plain query.
	if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS,sizeof(ncm),&ncm,0))
	{
		*lf = ncm.lfMenuFont;
		
		return 1;
	}
	
	return 0;
}

// the dpi of a window: getdpiforwindow when the os provides it, the
// tracked window dpi otherwise.
int os_window_dpi(HWND hwnd)
{
	if ((!hwnd) || (!_os_GetDpiForWindow))
	{
		return os_logical_wide;
	}
	
	return (int)_os_GetDpiForWindow(hwnd);
}

// fill lf with the message font (the dialog base font) at the window
// own dpi. the dialog templates carry a hard coded segoe ui face that
// never held the cjk glyphs: the chinese labels rendered through the
// per character font linking fallback with the latin line metrics,
// while the menu bar and the status bar draw the system locale font
// (microsoft yahei ui) - two type systems in one window. the message
// font is the locale native ui face at the right dpi, the same family
// the menu and the status bar use, so one type system covers the whole
// ui. returns 1 on success.
int os_dialog_font(LOGFONTW *lf,HWND hwnd)
{
	NONCLIENTMETRICSW ncm;
	
	if (!lf)
	{
		return 0;
	}
	
	os_zero_memory(&ncm,sizeof(ncm));
	
	ncm.cbSize = sizeof(ncm);
	
	if (_os_SystemParametersInfoForDpi)
	{
		// uiParam must match cbSize for the metric queries.
		if (_os_SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS,sizeof(ncm),&ncm,0,(UINT)os_window_dpi(hwnd)))
		{
			*lf = ncm.lfMessageFont;
			
			return 1;
		}
	}
	
	// pre 1607 systems (and any ForDpi refusal): the plain query.
	if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS,sizeof(ncm),&ncm,0))
	{
		*lf = ncm.lfMessageFont;
		
		return 1;
	}
	
	return 0;
}

// rc.13: the win11-only chrome attributes double as the platform probe.
// a round request (attribute 33) fails with E_INVALIDARG on windows 10
// and older; the first call latches the answer and the return tells the
// caller whether the modern chrome landed - the non-win11 callers pick
// their self drawn fallbacks from it (the drop shadow, the flat border).
static int _os_win11_chrome = 0;	// 0 = not probed, 1 = win11, 2 = older

int os_is_win11(void)
{
	return _os_win11_chrome == 1;
}

// windows 11 chrome: rounded window corners (attribute 33, round)
// and a caption color that matches the canvas (attribute 35). returns 0
// on windows 10 and older (the classic title bar stays and the caller
// falls back to its own frame).
int os_window_modern_chrome(HWND hwnd,COLORREF caption_color)
{
	DWORD corner;
	COLORREF color;
	
	if ((!hwnd) || (!_os_DwmSetWindowAttribute))
	{
		return 0;
	}
	
	// DWMWA_WINDOW_CORNER_PREFERENCE = 33, DWMWCP_ROUND = 2. the call
	// result is the platform probe.
	corner = 2;
	if (_os_DwmSetWindowAttribute(hwnd,33,&corner,sizeof(corner)) != 0)
	{
		_os_win11_chrome = 2;
		
		return 0;
	}
	
	_os_win11_chrome = 1;
	
	// DWMWA_CAPTION_COLOR = 35: the windowed canvas color gives a
	// seamless title bar in both themes (the dark mode attribute above
	// keeps the caption text readable).
	color = caption_color;
	_os_DwmSetWindowAttribute(hwnd,35,&color,sizeof(color));
	
	return 1;
}

// the popup menu layer gets the modern treatment too: rounded corners and
// a themed border. returns 0 on earlier windows (the native menu border
// stays).
int os_menu_modern_chrome(HWND hwnd,COLORREF border_color)
{
	int round;

	if ((!_os_DwmSetWindowAttribute) || (!hwnd))
	{
		return 0;
	}

	// DWMWA_WINDOW_CORNER_PREFERENCE = 33, DWMWCP_ROUND = 2.
	round = 2;
	if (_os_DwmSetWindowAttribute(hwnd,33,&round,sizeof(round)) != 0)
	{
		_os_win11_chrome = 2;

		return 0;
	}

	_os_win11_chrome = 1;

	// DWMWA_BORDER_COLOR = 34: a COLORREF in the native 0x00BBGGRR layout.
	_os_DwmSetWindowAttribute(hwnd,34,&border_color,sizeof(border_color));

	return 1;
}

// re-read the system theme after a WM_SETTINGCHANGE.
void os_dark_refresh(void)
{
	if (_os_RefreshImmersiveColorPolicyState)
	{
		_os_RefreshImmersiveColorPolicyState();
	}
	
	if (_os_FlushMenuThemes)
	{
		_os_FlushMenuThemes();
	}
}

void os_kill(void)
{
	if (_os_user32_hmodule)
	{
		FreeLibrary(_os_user32_hmodule);
	}
		
	if (_os_shell32_hmodule)
	{
		FreeLibrary(_os_shell32_hmodule);
	}
		
	if (_os_gdi32_hmodule)
	{
		FreeLibrary(_os_gdi32_hmodule);
	}
	
	if (_os_ucrtbase_hmodule)
	{
		FreeLibrary(_os_ucrtbase_hmodule);
	}
	
	if (_os_UxTheme_hmodule)
	{
		FreeLibrary(_os_UxTheme_hmodule);
	}
	
	if (_os_dwmapi_hmodule)
	{
		FreeLibrary(_os_dwmapi_hmodule);
	}
	
	if (_os_advapi32_hmodule)
	{
		FreeLibrary(_os_advapi32_hmodule);
	}
	
	if (_os_gdiplus_hmodule)
	{
		FreeLibrary(_os_gdiplus_hmodule);
	}
	
	if (_os_custom_colors)
	{
		mem_free(_os_custom_colors);
	}
}

ITEMIDLIST *os_ILCreateFromPath(const wchar_t *path)
{
	IShellFolder *psf;
	ITEMIDLIST *pidl;
	ITEMIDLIST *ret;
	
	ret = 0;

	if (SUCCEEDED(SHGetDesktopFolder(&psf)))
	{
		if (SUCCEEDED(psf->lpVtbl->ParseDisplayName(psf,NULL,0,(LPOLESTR)path,NULL,&pidl,NULL)))
		{
			ret = pidl;
		}

		psf->lpVtbl->Release(psf);
	}	           
	
	return ret;
}

void os_edit_move_caret_to_end(HWND hwnd)
{
	SetWindowRedraw(hwnd,FALSE);
	SendMessage(hwnd,EM_SETSEL,0,(LPARAM)-1); 
	SendMessage(hwnd,EM_SETSEL,(WPARAM)-1,(LPARAM)-1); 
	SetWindowRedraw(hwnd,TRUE);
	InvalidateRect(hwnd,0,FALSE);				
}

void os_make_sure_path_exists(const wchar_t *filename)
{
	wchar_t *p;
	wchar_t wbuf[STRING_SIZE];

	string_copy(wbuf,filename);
	
	p = wbuf;
	while(*p)
	{
		if (*p == '\\')
		{
			*p = 0;
			debug_printf("create dir %S\n",wbuf);
			CreateDirectory(wbuf,NULL);
			*p = '\\';
		}
		
		p++;
	}
	
	CreateDirectory(wbuf,NULL);
}

// returns null if path not found.
int os_get_special_folder_path(wchar_t *wbuf,int nFolder)
{
	LPITEMIDLIST pidl;
	int ret;
	
	ret = 0;
	
	// buffer size should be "at least MAX_PATH"...
	if (SUCCEEDED(SHGetSpecialFolderLocation(0,nFolder,&pidl)))
	{
		if (SHGetPathFromIDList(pidl,wbuf))
		{
			ret = 1;
		}

		CoTaskMemFree(pidl);
	}
	
	return ret;
}

// this is leaving a handle open somehow...
void os_create_shell_link(const wchar_t *exe_filename_wbuf,const wchar_t *lnk_wbuf)
{ 
	if (GetFileAttributes(exe_filename_wbuf) != INVALID_FILE_ATTRIBUTES)
	{
		IShellLinkW *psl;
		wchar_t path_part[STRING_SIZE];
		
		string_get_path_part(path_part,exe_filename_wbuf);
	 
		// Get a pointer to the IShellLink interface. 
		if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,&IID_IShellLinkW,(LPVOID*)&psl)))
		{
			IPersistFile *ppf;

			// convert to unicode   
			psl->lpVtbl->SetWorkingDirectory(psl,path_part);
			
			psl->lpVtbl->SetPath(psl,exe_filename_wbuf); 

			psl->lpVtbl->SetArguments(psl,L"");
			
			// Set the path to the shortcut target and add the description. 
			psl->lpVtbl->SetDescription(psl,L""); 

			// Query IShellLink for the IPersistFile interface for saving the 
			// shortcut in persistent storage. 
			if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl,&IID_IPersistFile,(LPVOID*)&ppf))) 
			{ 
				// Save the link by calling IPersistFileSave. 
				ppf->lpVtbl->Save(ppf,lnk_wbuf,1);
				
				ppf->lpVtbl->Release(ppf);
			} 
			
			psl->lpVtbl->Release(psl); 
		}
	}
}

void os_center_dialog(HWND hwnd)
{
	RECT parent_rect;
	RECT dialog_rect;
	RECT rect;
	
	GetWindowRect(GetParent(hwnd),&parent_rect);
	GetWindowRect(hwnd,&dialog_rect);
	
	rect.left = parent_rect.left + ((parent_rect.right - parent_rect.left) / 2) - ((dialog_rect.right - dialog_rect.left) / 2);
	rect.top = parent_rect.top + ((parent_rect.bottom - parent_rect.top) / 2) - ((dialog_rect.bottom - dialog_rect.top) / 2);
	rect.right = rect.left + (dialog_rect.right - dialog_rect.left);
	rect.bottom = rect.top + (dialog_rect.bottom - dialog_rect.top);
		
	os_make_rect_completely_visible(hwnd,&rect);
	
	SetWindowPos(hwnd,0,rect.left,rect.top,0,0,SWP_NOSIZE|SWP_NOZORDER);
}

int os_shell_execute(HWND hwnd,const wchar_t *filename,int wait,const char *verb,const wchar_t *params)
{
	__unaligned ITEMIDLIST *pidl;
	int ret;
	
	ret = 0;
	
	pidl = os_ILCreateFromPath(filename);
	if (pidl)
	{
		wchar_t verb_wbuf[STRING_SIZE];
		wchar_t *pverb;
		SHELLEXECUTEINFO sei;
		ULONG mask;
		
		mask = SEE_MASK_INVOKEIDLIST;
		if (wait)
		{
			mask |= SEE_MASK_NOCLOSEPROCESS;
		}
		
		if (verb)
		{
			string_copy_utf8_string(verb_wbuf,verb);
			pverb = verb_wbuf;
		}
		else
		{
			pverb = 0;
		}
		
		os_zero_memory(&sei,sizeof(SHELLEXECUTEINFO));
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = mask;
		sei.hwnd = hwnd;
		sei.lpVerb = pverb;
		sei.lpIDList = pidl;
		sei.lpParameters = params;
		sei.nShow = SW_SHOWNORMAL;
	
		if (ShellExecuteEx(&sei))
		{
			ret = 1;
			
			if (wait)
			{
				WaitForSingleObject(sei.hProcess,INFINITE);
			
				CloseHandle(sei.hProcess);
			}
		}
	
		CoTaskMemFree(pidl);
	}			
	
	return ret;
}

static int CALLBACK _os_BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData)
{
	switch(uMsg)
	{
		case BFFM_INITIALIZED:
			SendMessage(hwnd,BFFM_SETSELECTIONW,(WPARAM)1,(LPARAM)lpData);
			break;
	}
	
	return 0;
}

// pidl MUST be absolute.
// the advandage of using IShellItemImageFactory is
// IShellItemImageFactory will use thumbs.db or windows thumbnail cache
// where as IExtractImage does not.
static IShellItem *_os_get_shellitem(const ITEMIDLIST *pidl)
{
	IShellItem *ret;
	
	ret = 0;
	
	if (_os_SHCreateItemFromIDList)
	{
		IShellItem *si;
		
		if (SUCCEEDED(_os_SHCreateItemFromIDList((ITEMIDLIST *)pidl,&_os_IID_IShellItem,&si)))
		{
			ret = si;
		}
	}
	
	return ret;
}

// get a shellitem interface from a filename.
static IShellItem *_os_shellitem_from_filename(const wchar_t *filename)
{
	IShellFolder *psf;
	IShellItem *ret;

	ret = 0;

	if (SUCCEEDED(SHGetDesktopFolder(&psf)))
	{
		ITEMIDLIST *pidl;
	    
		if (SUCCEEDED(psf->lpVtbl->ParseDisplayName(psf,NULL,0,(LPOLESTR)filename,NULL,&pidl,NULL)))
		{
			ret = _os_get_shellitem(pidl);
			
			// free pidl allocated by ParseDisplayName
			CoTaskMemFree(pidl);		
		}
		
		psf->lpVtbl->Release(psf);
	}
		
	return ret;	
}

//TODO: resolve shortcuts.. need to use IFileOpenDialog on vista and later to do this easily.
// returns null cbuf if failure.
int os_browse_for_folder(HWND parent,wchar_t *filename)
{
	if (config_browse_file_open_dialog)
	{
		_os_IFileOpenDialog_t *file_open_dialog;
		
		if (SUCCEEDED(CoCreateInstance(&_os_CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&_os_IID_IFileOpenDialog,&file_open_dialog)))
		{
			int ret;
			IShellItem *shell_item;
			
			ret = 0;
			
			file_open_dialog->lpVtbl->SetOptions(file_open_dialog,0x2028);

			shell_item = _os_shellitem_from_filename(filename);
			if (shell_item)
			{
				file_open_dialog->lpVtbl->SetFolder(file_open_dialog,shell_item);
				
				shell_item->lpVtbl->Release(shell_item);
			}

			if (SUCCEEDED(file_open_dialog->lpVtbl->Show(file_open_dialog,parent)))
			{	
				if (SUCCEEDED(file_open_dialog->lpVtbl->GetResult(file_open_dialog,&shell_item)))
				{
					LPOLESTR open_filename;
					
					if (SUCCEEDED(shell_item->lpVtbl->GetDisplayName(shell_item,SIGDN_FILESYSPATH,&open_filename)))
					{
						string_copy(filename,open_filename);
						
						ret = 1;
					
						CoTaskMemFree(open_filename);
					}
					
					shell_item->lpVtbl->Release(shell_item);
				}
			}
			
			file_open_dialog->lpVtbl->Release(file_open_dialog);
			
			return ret;
		}
	}
	
	{
		BROWSEINFOW bi;
		ITEMIDLIST *iil;
		int ret;
		
		// BIF_USENEWUI wont bring the initial selection into view.
		// BIF_NOTRANSLATETARGETS we dont want symbolic links targets, we want the actual path.
		os_zero_memory(&bi,sizeof(BROWSEINFOW));
		bi.hwndOwner = parent;
		bi.lpszTitle = 0;
		bi.pszDisplayName = filename;
		bi.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS | BIF_NOTRANSLATETARGETS;
		bi.lpfn = _os_BrowseCallbackProc;
		bi.lParam = (LPARAM)filename;

		ret = 0;
		
		iil = (ITEMIDLIST *)SHBrowseForFolderW(&bi);

		if (iil)
		{
			if (SHGetPathFromIDListW(iil,filename))
			{
				ret = 1;
			}

			CoTaskMemFree(iil);
		}

		return ret;
	}
}

// returns tick
VIV_UINT64 os_get_tick_count(void)
{
	ULARGE_INTEGER tick;
		
	// why doesn't this take a ULARGE_INTEGER?
	if (QueryPerformanceCounter((LARGE_INTEGER *)&tick))
	{
		return tick.QuadPart;
	}
	
	return 0;
}

VIV_UINT64 os_get_tick_freq(void)
{
	ULARGE_INTEGER freq;
	
	// why doesn't this take a ULARGE_INTEGER?
	if (QueryPerformanceFrequency((LARGE_INTEGER *)&freq))
	{
		return freq.QuadPart;
	}
	
	return 0;
}

// The cool Win95 hack, so the compiler can launch on it, even compiled with VS2005
// Basically, this function is needed by the libcmt.lib/gs_report.obj
//
BOOL __stdcall _imp__IsDebuggerPresent()
{
	if (_os_IsDebuggerPresent)
	{
		return _os_IsDebuggerPresent();
	}
	
	return FALSE;
}

WNDPROC os_set_window_proc(HWND hwnd,WNDPROC proc)
{
	return (WNDPROC)(uintptr_t)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)proc);
}

int os_is_windows_7_or_later(void)
{
	if (os_major_version > 6)
	{
		return 1;
	}
	
	if (os_major_version == 6)
	{
		if (os_minor_version >= 1)
		{
			return 1;
		}
	}
	
	return 0;
}

int os_is_windows_8_or_later(void)
{
	if (os_major_version > 6)
	{
		return 1;
	}
	
	if (os_major_version == 6)
	{
		if (os_minor_version >= 2)
		{
			return 1;
		}
	}
	
	return 0;
}
// true when the comctl dark explorer control classes actually render dark:
// windows 10 1903 (build 18362) and later. older builds accept the calls
// but keep the light control rendering, so the dialog fallbacks (owner
// drawn buttons and combos) carry the dark ui alone on those builds.
int os_dark_controls_supported(void)
{
	if (!os_is_nt)
	{
		return 0;
	}
	
	if (os_major_version > 10)
	{
		return 1;
	}
	
	if (os_major_version == 10)
	{
		return (os_build_number >= 18362) ? 1 : 0;
	}
	
	return 0;
}


// windows creates funky regions if left > right
HRGN os_CreateRectRgn(int left,int top,int right,int bottom)
{
	if (left >= right) return CreateRectRgn(0,0,0,0);
	if (top >= bottom) return CreateRectRgn(0,0,0,0);
	
	return CreateRectRgn(left,top,right,bottom);
}

HRGN os_mirror_region(HRGN hrgn,int wide)
{
	HRGN new_hrgn;
	DWORD i;
	RGNDATA *region_data;
	DWORD region_size;
	small_pool_t small_pool;

	small_pool_init(&small_pool);

	region_size = GetRegionData(hrgn,0,0);

	region_data = small_pool_alloc(&small_pool,region_size);

	GetRegionData(hrgn,region_size,region_data);
	
	for(i=0;i<region_data->rdh.nCount;i++)
	{
		int left;
		int right;
		
		right = wide - ((RECT *)region_data->Buffer)[i].left;
		left = wide - ((RECT *)region_data->Buffer)[i].right;
		
		((RECT *)region_data->Buffer)[i].left = left;
		((RECT *)region_data->Buffer)[i].right = right;
	}
	
	new_hrgn = ExtCreateRegion(NULL,region_size,region_data);
	
	small_pool_kill(&small_pool);
	
	return new_hrgn;
}

// prect is out, pclip is in.
void os_fill_clamped_rect(HDC hdc,int left,int top,int right,int bottom,int clamp_left,int clamp_top,int clamp_right,int clamp_bottom,HBRUSH hbrush)
{
	if (left < clamp_left)
	{
		left = clamp_left;
	}

	if (top < clamp_top)
	{
		top = clamp_top;
	}
	
	if (right > clamp_right)
	{
		right = clamp_right;
	}
	
	if (bottom > clamp_bottom)
	{
		bottom = clamp_bottom;
	}
	
	os_fill_rect(hdc,left,top,right - left,bottom - top,hbrush);
}

void os_set_rect(RECT *rect,int x,int y,int wide,int high)
{
	rect->left = x;
	rect->top = y;
	rect->right = x + wide;
	rect->bottom = y + high;
}

void os_fill_rect(HDC hdc,int x,int y,int wide,int high,HBRUSH hbrush)
{
	if ((wide > 0) && (high > 0))
	{
		RECT rect;
		
		os_set_rect(&rect,x,y,wide,high);

		FillRect(hdc,&rect,hbrush);
	}
}

// prect is out, pclip is in.
void os_fill_clipped_rect(HDC hdc,int x,int y,int wide,int high,int clip_x,int clip_y,int clip_wide,int clip_high,HBRUSH hbrush)
{
	// make sure we have a rect with some size!
	// otherwise we overdraw!
	if ((wide > 0) && (high > 0))
	{
		int right;
		int bottom;
		int clip_right;
		int clip_bottom;
		
		right = x + wide;
		bottom = y + high;
		clip_right = clip_x + clip_wide;
		clip_bottom = clip_y + clip_high;
		
		// top
		os_fill_clamped_rect(hdc,x,y,right,clip_y,x,y,right,bottom,hbrush);
		
		// left
		os_fill_clamped_rect(hdc,x,clip_y,clip_x,clip_bottom,x,y,right,bottom,hbrush);
		
		// right
		os_fill_clamped_rect(hdc,clip_right,clip_y,right,clip_bottom,x,y,right,bottom,hbrush);
		
		// bottom
		os_fill_clamped_rect(hdc,x,clip_bottom,right,bottom,x,y,right,bottom,hbrush);
	}	
}

// #define PHOTO_ORIENTATION_NORMAL            1u
// #define PHOTO_ORIENTATION_FLIPHORIZONTAL    2u
// #define PHOTO_ORIENTATION_ROTATE180         3u
// #define PHOTO_ORIENTATION_FLIPVERTICAL      4u
// #define PHOTO_ORIENTATION_TRANSPOSE         5u
// #define PHOTO_ORIENTATION_ROTATE270         6u
// #define PHOTO_ORIENTATION_TRANSVERSE        7u
// #define PHOTO_ORIENTATION_ROTATE90          8u
int os_get_orientation(const wchar_t *filename)
{
	int ret;
	HRESULT hr;
	
	ret = 0;
	
#ifdef _MSC_VER
	__try
	{
#endif
		if (_os_SHGetPropertyStoreFromIDList)
		{
			ITEMIDLIST *pidl;
			
			pidl = os_ILCreateFromPath(filename);
			
			if (pidl)
			{
				_os_IPropertyStore_t *ps;
				
				// GPS_HANDLERPROPERTIESONLY	0x01
				// GPS_OPENSLOWITEM	0x10 (offline files)
				// GPS_BESTEFFORT 0x40

				// without GPS_BESTEFFORT this returns 0x8007000d for some files.
				hr = _os_SHGetPropertyStoreFromIDList(pidl,0x10|0x40,&_os_IID_IPropertyStore,&ps);
				
				if (SUCCEEDED(hr))
				{
					_os_PROPERTYKEY_t key;
					PROPVARIANT pv;

					PropVariantInit(&pv);
					
					//  Name:     System.Photo.Orientation -- PKEY_Photo_Orientation
					os_copy_memory(&key.fmtid,&_os_FMTID_ImageProperties,sizeof(GUID));
					key.pid = 274; // PropertyTagOrientation
					
					hr = ps->lpVtbl->GetValue(ps,&key,&pv);
					if (SUCCEEDED(hr))
					{
						if (pv.vt == VT_UI2)
						{
							ret = pv.uiVal;
						}
						else
						{
	//						debug_error_printf((const utf8_t *)"%s: empty value\n",filename)	;
						}
					}
					else
					{
						debug_printf("%S: GetValue %08x\n",filename,hr)	;
					}

					PropVariantClear(&pv);
				
					ps->lpVtbl->Release(ps);
				}
				else
				{
					debug_printf((const utf8_t *)"%s: SHGetPropertyStoreFromIDList %08x\n",filename,hr)	;
				}

				// free pidl allocated by ParseDisplayName
				ILFree(pidl);		
			}
			else
			{
				debug_printf((const utf8_t *)"%s: no pidl\n",filename)	;
			}
		}
		else
		{
			debug_printf((const utf8_t *)"no SHGetPropertyStoreFromIDList\n")	;
		}
#ifdef _MSC_VER
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		debug_printf((const utf8_t *)"IPropertyStore::GetValue exception %08x\n",GetExceptionCode());
	}
#endif
	
	return ret;
}

__declspec(noinline) DWORD os_get_window_style(HWND hwnd) 
{ 
	return GetWindowLong(hwnd,GWL_STYLE); 
}

__declspec(noinline) DWORD os_get_window_ex_style(HWND hwnd) 
{ 
	return GetWindowLong(hwnd,GWL_EXSTYLE); 
}

void os_adjust_window_rect(HWND hwnd,RECT *window_rect,int window_x,int window_y,int client_wide,int client_high)
{
	BOOL is_menu;
	DWORD style;
	
	window_rect->left = 0;
	window_rect->top = 0;
	window_rect->right = client_wide;
	window_rect->bottom = client_high;
	
	is_menu = FALSE;
	style = os_get_window_style(hwnd);
	
	if (!(style & WS_CHILD))
	{
		if (GetMenu(hwnd))
		{
			is_menu = TRUE;
		}
	}

//	debug_printf("GetWindowLong(hwnd,GWL_EXSTYLE) %08x\n",GetWindowLong(hwnd,GWL_EXSTYLE));
	AdjustWindowRectEx(window_rect,style,is_menu,os_get_window_ex_style(hwnd));
	
//DEBUG:
//debug_printf((const utf8_t *)"%d %d %d %d\n",window_rect->left,window_rect->top,window_rect->right,window_rect->bottom);
	
	window_rect->right -= window_rect->left;
	window_rect->bottom -= window_rect->top;
	window_rect->left = window_x;
	window_rect->top = window_y;
	window_rect->right += window_x;
	window_rect->bottom += window_y;

//DEBUG:
//debug_printf((const utf8_t *)"%d %d %d %d\n",window_rect->left,window_rect->top,window_rect->right,window_rect->bottom);
}

int os_get_text_wideW(HDC hdc,const wchar_t *s,uintptr_t slen_in_wchars)
{
	int ret;

	ret = 0;
	
	if (slen_in_wchars)
	{
		if (slen_in_wchars <= INT_MAX)
		{
/*
			SIZE size;
			
			GetTextExtentPoint32W(hdc,s,(int)slen_in_wchars,&size);
			
			ret = size.cx;
*/			
			
			RECT rect;
			
			rect.left = 0;
			rect.right = 0;
			rect.top = 0;
			rect.bottom = 0;
			
			if (slen_in_wchars > _OS_DRAWTEXT_MAX_LEN)
			{
				slen_in_wchars = _OS_DRAWTEXT_MAX_LEN;
			}

			// DrawText performance is about the same as GetTextExtentPoint32W.
			// DrawText works with unicode characters that map to non-printable ascii chars. (eg: music note)
			// GetTextExtentPoint32W does not work for some unicode chars (eg: music note)
			
			if (DrawText(hdc,s,(int)slen_in_wchars,&rect,DT_NOCLIP| DT_CALCRECT))
			{
				ret = rect.right - rect.left;
			}
		}
	}
	
	return ret;
}

int os_get_static_wide(HWND hwnd,int id)
{
	HWND dialog_item_hwnd;
	HDC hdc;
	int wide;
	
	dialog_item_hwnd = GetDlgItem(hwnd,id);
	wide = 0;

	// get the text width
	hdc = GetDC(dialog_item_hwnd);
	if (hdc)
	{
		HFONT hfont;
		HGDIOBJ lastfont;
		wchar_t wbuf[STRING_SIZE];

		hfont = GetWindowFont(dialog_item_hwnd);
		lastfont = SelectObject(hdc,hfont);

		GetWindowText(dialog_item_hwnd,wbuf,STRING_SIZE);

		wide = os_get_text_wideW(hdc,wbuf,string_get_length(wbuf));
		
		SelectObject(hdc,lastfont);
		
		ReleaseDC(hwnd,hdc);
	}
	
	return wide;
}

int os_expand_static_wide(HWND hwnd,int id,int static_wide)
{
	int wide;
	
	wide = os_get_static_wide(hwnd,id);
	
	if (wide > static_wide)
	{
		return wide;
	}
	
	return static_wide;
}


void os_set_window_rect(HWND hwnd,int x,int y,int wide,int high)
{
	SetWindowPos(hwnd,0,x,y,wide,high,SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
}

void os_set_dialog_item_x_wide(HWND hwnd,int id,int x,int wide)
{
	HWND dialog_item_hwnd;
	RECT rect;
	POINT pt;
	
	dialog_item_hwnd = GetDlgItem(hwnd,id);

	GetWindowRect(dialog_item_hwnd,&rect);
	pt.x = rect.left;
	pt.y = rect.top;
	ScreenToClient(hwnd,&pt);
	
	os_set_window_rect(dialog_item_hwnd,x,pt.y,wide,rect.bottom-rect.top);
}

