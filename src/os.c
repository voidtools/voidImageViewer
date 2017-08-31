
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

#include "viv.h"

#define _OS_QSORT_CUTOFF	8            /* testing shows that this is good value */
#define _OS_QSORT_STKSIZ	((8*sizeof(void*)) - 2)

static void _os_qsort_indexes_shortsort(void **lo,void **hi,int (*comp)(const void *,const void *));
static FARPROC __os_get_proc_address(HMODULE hmodule,const char *name);
static int CALLBACK _os_BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData);

HINSTANCE os_hinstance = 0;
DWORD os_major_version = 0; // 4 = 9x/nt, 5 = xp,2k,server2k3, 6 = vista
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
BOOL (STDAPICALLTYPE *os_IsUserAnAdmin)(void) = 0;
HRESULT (__stdcall *os_EnableThemeDialogTexture)(HWND hwnd, DWORD dwFlags) = 0;
unsigned int (__cdecl *_os_controlfp)(unsigned int _NewValue,unsigned int _Mask) = 0;
BOOL (WINAPI *os_ChangeWindowMessageFilterEx)(HWND hWnd,UINT message,DWORD action,void *pChangeFilterStruct) = 0;
int os_logical_wide = 96;
int os_logical_high = 96;

static COLORREF *_os_custom_colors = 0;
static HMODULE _os_shell32_hmodule = 0;
static HMODULE _os_user32_hmodule = 0;
static HMODULE _os_UxTheme_hmodule = 0;
static HMODULE _os_gdiplus_hmodule = 0;
static HMODULE _os_ucrtbase_hmodule = 0;

void os_zero_memory(void *data,int size)
{
	ZeroMemory(data,size);
}

void os_copy_memory(void *d,const void *s,int size)
{
	CopyMemory(d,s,size);
}

// make a rect fully visible
// pushes the rect onto the screen,the same as menus.
// does not allow monitor overlapping.
// if you have a window that is fully visible, but half is displayed on one display, and half on another, this function will
// push it onto just one of the displays. (the one with the most area shown)
// make sure you call os_is_rect_completely_visible before calling this.
void os_make_rect_completely_visible(RECT *prect)
{
	int wide;
	int high;
	RECT monitor_rect;

	// we hit something so reposition the window on this monitor.
	// note that MonitorFromRect can return null, even when MONITOR_DEFAULTTOPRIMARY is specified.
	os_MonitorRectFromWindowRect(prect,&monitor_rect);
	
	// get window width and height,
	wide = prect->right - prect->left;
	high = prect->bottom - prect->top;

	// push the window 
	if (prect->right > monitor_rect.right) 
	{
		prect->left = monitor_rect.right - wide;
		prect->right = monitor_rect.right;
	}
	
	if (prect->bottom > monitor_rect.bottom) 
	{
		prect->top = monitor_rect.bottom - high;
		prect->bottom = monitor_rect.bottom;
	}

	// push the window 
	if (prect->left < monitor_rect.left) 
	{
		prect->left = monitor_rect.left;
		prect->right = monitor_rect.left + wide;
	}
	
	if (prect->top < monitor_rect.top) 
	{
		prect->top = monitor_rect.top;
		prect->bottom = monitor_rect.top + high;
	}
}

void os_MonitorRectFromWindowRect(const RECT *window_rect,RECT *monitor_rect)
{
	HMONITOR hmonitor;
	MONITORINFO mi;
	
	hmonitor = MonitorFromRect(window_rect,MONITOR_DEFAULTTOPRIMARY);
	if (hmonitor)
	{
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hmonitor,&mi);
		
		CopyRect(monitor_rect,&mi.rcWork);
	}
	else
	{
		// work area
		SystemParametersInfo(SPI_GETWORKAREA,0,(PVOID)monitor_rect,0);
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
/*
#define _OS_QSORT_SWAP(block,comp,x,y) if (comp(block[y],block[x]) < 0) { void *tmp = block[x]; block[x] = block[y]; block[y] = tmp; }

static void _os_qsort_sort_block16(void **block,int (*comp)(const void *,const void *))
{
	_OS_QSORT_SWAP(block,comp,0,1)
	_OS_QSORT_SWAP(block,comp,2,3)
	_OS_QSORT_SWAP(block,comp,4,5)
	_OS_QSORT_SWAP(block,comp,6,7)
	_OS_QSORT_SWAP(block,comp,8,9)
	_OS_QSORT_SWAP(block,comp,10,11)
	_OS_QSORT_SWAP(block,comp,12,13)
	_OS_QSORT_SWAP(block,comp,14,15)
	_OS_QSORT_SWAP(block,comp,0,2)
	_OS_QSORT_SWAP(block,comp,4,6)
	_OS_QSORT_SWAP(block,comp,8,10)
	_OS_QSORT_SWAP(block,comp,12,14)
	_OS_QSORT_SWAP(block,comp,1,3)
	_OS_QSORT_SWAP(block,comp,5,7)
	_OS_QSORT_SWAP(block,comp,9,11)
	_OS_QSORT_SWAP(block,comp,13,15)
	_OS_QSORT_SWAP(block,comp,0,4)
	_OS_QSORT_SWAP(block,comp,8,12)
	_OS_QSORT_SWAP(block,comp,1,5)
	_OS_QSORT_SWAP(block,comp,9,13)
	_OS_QSORT_SWAP(block,comp,2,6)
	_OS_QSORT_SWAP(block,comp,10,14)
	_OS_QSORT_SWAP(block,comp,3,7)
	_OS_QSORT_SWAP(block,comp,11,15)
	_OS_QSORT_SWAP(block,comp,0,8)
	_OS_QSORT_SWAP(block,comp,1,9)
	_OS_QSORT_SWAP(block,comp,2,10)
	_OS_QSORT_SWAP(block,comp,3,11)
	_OS_QSORT_SWAP(block,comp,4,12)
	_OS_QSORT_SWAP(block,comp,5,13)
	_OS_QSORT_SWAP(block,comp,6,14)
	_OS_QSORT_SWAP(block,comp,7,15)
	_OS_QSORT_SWAP(block,comp,5,10)
	_OS_QSORT_SWAP(block,comp,6,9)
	_OS_QSORT_SWAP(block,comp,3,12)
	_OS_QSORT_SWAP(block,comp,13,14)
	_OS_QSORT_SWAP(block,comp,7,11)
	_OS_QSORT_SWAP(block,comp,1,2)
	_OS_QSORT_SWAP(block,comp,4,8)
	_OS_QSORT_SWAP(block,comp,1,4)
	_OS_QSORT_SWAP(block,comp,7,13)
	_OS_QSORT_SWAP(block,comp,2,8)
	_OS_QSORT_SWAP(block,comp,11,14)
	_OS_QSORT_SWAP(block,comp,5,6)
	_OS_QSORT_SWAP(block,comp,9,10)
	_OS_QSORT_SWAP(block,comp,2,4)
	_OS_QSORT_SWAP(block,comp,11,13)
	_OS_QSORT_SWAP(block,comp,3,8)
	_OS_QSORT_SWAP(block,comp,7,12)
	_OS_QSORT_SWAP(block,comp,6,8)
	_OS_QSORT_SWAP(block,comp,10,12)
	_OS_QSORT_SWAP(block,comp,3,5)
	_OS_QSORT_SWAP(block,comp,7,9)
	_OS_QSORT_SWAP(block,comp,3,4)
	_OS_QSORT_SWAP(block,comp,5,6)
	_OS_QSORT_SWAP(block,comp,7,8)
	_OS_QSORT_SWAP(block,comp,9,10)
	_OS_QSORT_SWAP(block,comp,11,12)
	_OS_QSORT_SWAP(block,comp,6,7)
	_OS_QSORT_SWAP(block,comp,8,9)
}
*/

int os_ComboBox_AddString(HWND hwnd,int id,const utf8_t *s)
{
	wchar_t wbuf[STRING_SIZE];
	
	string_copy_utf8(wbuf,s);
	
	return ComboBox_AddString(GetDlgItem(hwnd,id),wbuf);
}

void os_SetDlgItemText(HWND hwnd,int id,const utf8_t *s)
{	
	wchar_t wbuf[STRING_SIZE];
	
	string_copy_utf8(wbuf,s);
	
	SetDlgItemText(hwnd,id,wbuf);
}

HWND os_CreateWindowEx(DWORD dwExStyle,const utf8_t *lpClassName,const utf8_t *lpWindowName,DWORD dwStyle,int X,int Y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam)
{
	wchar_t lpClassName_wbuf[STRING_SIZE];
	wchar_t lpWindowName_wbuf[STRING_SIZE];
	
	string_copy_utf8(lpClassName_wbuf,lpClassName);
	string_copy_utf8(lpWindowName_wbuf,lpWindowName);
	
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
	
	string_copy_utf8(name_wbuf,name);

	// Initialize global strings
	os_zero_memory(&wcex,sizeof(WNDCLASSEXW));
	wcex.cbSize = sizeof(WNDCLASSEXW); 

	wcex.style = style;
	wcex.lpfnWndProc = lpfnWndProc;
	wcex.hInstance = os_hinstance;
	wcex.hIcon = hIcon;
	wcex.hCursor = LoadCursor(NULL,IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
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
	// OS Version
	{
		OSVERSIONINFOA osvi;
		
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
		GetVersionExA(&osvi);

		os_major_version = (char)osvi.dwMajorVersion;

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

	_os_shell32_hmodule = LoadLibraryA("shell32.dll");
	if (_os_shell32_hmodule)
	{
		os_SHOpenFolderAndSelectItems = (void *)GetProcAddress(_os_shell32_hmodule,"SHOpenFolderAndSelectItems");
		os_IsUserAnAdmin = (void *)GetProcAddress(_os_shell32_hmodule,"IsUserAnAdmin");
	}

	_os_user32_hmodule = LoadLibraryA("user32.dll");
	if (_os_user32_hmodule)
	{
		os_ChangeWindowMessageFilterEx = (void *)GetProcAddress(_os_user32_hmodule,"ChangeWindowMessageFilterEx");
	}

	_os_UxTheme_hmodule = LoadLibraryA("UxTheme.dll");
	if (_os_UxTheme_hmodule)
	{
		os_EnableThemeDialogTexture = (void *)GetProcAddress(_os_UxTheme_hmodule,"EnableThemeDialogTexture");
	}
	
	// set floating point to int mode to truncate
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
	}
	else
	{
		debug_fatal("no gdiplus");
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
		
	if (_os_ucrtbase_hmodule)
	{
		FreeLibrary(_os_ucrtbase_hmodule);
	}
	
	if (_os_UxTheme_hmodule)
	{
		FreeLibrary(_os_UxTheme_hmodule);
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
		
	os_make_rect_completely_visible(&rect);
	
	SetWindowPos(hwnd,0,rect.left,rect.top,0,0,SWP_NOSIZE|SWP_NOZORDER);
}

void os_shell_execute(HWND hwnd,const wchar_t *filename,int wait,const char *verb,const wchar_t *params)
{
	__unaligned ITEMIDLIST *pidl;
	
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
			string_copy_utf8(verb_wbuf,verb);
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
			if (wait)
			{
				WaitForSingleObject(sei.hProcess,INFINITE);
			
				CloseHandle(sei.hProcess);
			}
		}
	
		CoTaskMemFree(pidl);
	}			
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


//TODO: resolve shortcuts.. need to use IFileOpenDialog on vista and later to do this easily.
// returns null cbuf if failure.
int os_browse_for_folder(HWND parent,wchar_t *filename)
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
