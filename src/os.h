
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
void os_make_rect_completely_visible(RECT *prect);
void os_MonitorRectFromWindowRect(const RECT *window_rect,RECT *monitor_rect);
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

extern HINSTANCE os_hinstance;
extern DWORD os_major_version;
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
extern BOOL (STDAPICALLTYPE *os_IsUserAnAdmin)(void);
extern HRESULT (__stdcall *os_EnableThemeDialogTexture)(HWND hwnd, DWORD dwFlags);
extern BOOL (WINAPI *os_ChangeWindowMessageFilterEx)(HWND hWnd,UINT message,DWORD action,void *pChangeFilterStruct);
extern int os_logical_wide;
extern int os_logical_high;

#ifdef __cplusplus
}
#endif
