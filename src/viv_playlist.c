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
// viv_playlist.c - playlist, everything search, sorting and navigation items.
// Pure physical move from viv.c (R70 split): bodies are unchanged.
#include "viv.h"
#include "viv_state.h"
#include "viv_playlist.h"
#include "viv_dark.h"
#include "viv_load.h"
#include "viv_view.h"

// forward declarations (order preserved from viv.c)
int _viv_icompare_filename(const wchar_t *s1,const wchar_t *s2);
static int _viv_compare_id(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b);
static int _viv_fd_compare_name(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b);
static int _viv_fd_compare_path_and_name(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b);
int _viv_fd_compare(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b);
int _viv_is_valid_filename(WIN32_FIND_DATA *fd);
const char *_viv_get_copydata_string(const char *p,const char *e,wchar_t *buf,int bufsize);
void _viv_playlist_add_current_if_empty(void);
void _viv_playlist_clearall(void);
void _viv_playlist_delete(const WIN32_FIND_DATA *fd);
void _viv_playlist_rename(const wchar_t *old_filename,const wchar_t *new_filename);
_viv_playlist_t *_viv_playlist_add(const WIN32_FIND_DATA *fd);
void _viv_playlist_add_path(const wchar_t *full_path_and_filename);
void _viv_playlist_add_filename(const wchar_t *filename);
static void _viv_shuffle_playlist(void);
void _viv_nav_item_free_all(void);
void _viv_nav_item_add(WIN32_FIND_DATA *fd);
int _viv_nav_compare(const void *va,const void *vb);
static INT_PTR CALLBACK _viv_search_everything_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
void _viv_search_everything(int add);
int _viv_send_everything_search(HWND hwnd,int add,int randomize,const wchar_t *search);
static void _viv_add_current_path_to_playlist(void);
int _viv_playlist_shuffle_index_from_fd(const WIN32_FIND_DATA *fd);
_viv_playlist_t *_viv_playlist_from_fd(const WIN32_FIND_DATA *fd);
void _viv_do_initial_shuffle(void);
void _viv_send_random_everything_search(void);
static const char *_viv_copydata_read(const COPYDATASTRUCT *cds,const char *p,void *dst,DWORD size);
int _viv_everything_item_to_fd(const COPYDATASTRUCT *cds,const EVERYTHING_IPC_ITEM2 *item,WIN32_FIND_DATA *fd);


static _viv_playlist_t *_viv_playlist_last = 0;
static LARGE_INTEGER _viv_playlist_id = {0};
static _viv_nav_item_t *_viv_nav_item_last = 0;
static DWORD _viv_everything_request_flags = 0;
int _viv_icompare_filename(const wchar_t *s1,const wchar_t *s2)
{
	// windows paths are case preserving but case insensitive: fold ascii
	// letters before comparing so one file cannot enter the mru twice with
	// different capitalization.
	while((*s1) && (*s2))
	{
		wchar_t c1;
		wchar_t c2;
		
		c1 = *s1;
		c2 = *s2;
		
		if ((c1 >= 'A') && (c1 <= 'Z'))
		{
			c1 = (wchar_t)(c1 + ('a' - 'A'));
		}
		
		if ((c2 >= 'A') && (c2 <= 'Z'))
		{
			c2 = (wchar_t)(c2 + ('a' - 'A'));
		}
		
		if (c1 != c2)
		{
			return 1;
		}
		
		s1++;
		s2++;
	}
	
	if ((*s1) || (*s2))
	{
		return 1;
	}
	
	return 0;
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

	ret = CompareString(LOCALE_USER_DEFAULT,dwCmpFlags,afilename,string_get_length(afilename),bfilename,string_get_length(bfilename));
	
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

	ret = CompareString(LOCALE_USER_DEFAULT,dwCmpFlags,a->cFileName,string_get_length(a->cFileName),b->cFileName,string_get_length(b->cFileName));
	
	switch(ret)
	{
		case CSTR_LESS_THAN:	
			return -1;
			
		case CSTR_GREATER_THAN:
			return 1;
	}

	return _viv_compare_id(a,b);
}
int _viv_fd_compare(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b)
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
int _viv_is_valid_filename(WIN32_FIND_DATA *fd)
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
const char *_viv_get_copydata_string(const char *p,const char *e,wchar_t *buf,int bufsize)
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
void _viv_playlist_add_current_if_empty(void)
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
void _viv_playlist_clearall(void)
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
void _viv_playlist_delete(const WIN32_FIND_DATA *fd)
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
void _viv_playlist_rename(const wchar_t *old_filename,const wchar_t *new_filename)
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
_viv_playlist_t *_viv_playlist_add(const WIN32_FIND_DATA *fd)
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
			
			new_indexes = mem_alloc(safe_size_mul_sizeof_pointer((SIZE_T)_viv_playlist_shuffle_allocated));
			
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
void _viv_playlist_add_path(const wchar_t *full_path_and_filename)
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
					
					string_copy_with_bufsize(fd.cFileName,MAX_PATH,buf);
					
					_viv_playlist_add(&fd);
				}
			}

			if (!FindNextFile(h,&fd)) break;
		}
		
		FindClose(h);
	}
}
void _viv_playlist_add_filename(const wchar_t *filename)
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
			string_copy_with_bufsize(fd.cFileName,MAX_PATH,full_path_and_filename);

			if (_viv_is_valid_filename(&fd))
			{
				_viv_playlist_add(&fd);
			}
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
			
			srand((unsigned int)(counter.LowPart ^ counter.HighPart));
		}
		
		_viv_playlist_shuffle_allocated = _VIV_DEFAULT_SHUFFLE_ALLOCATED;
		
		while(_viv_playlist_shuffle_allocated < _viv_playlist_count)
		{
			_viv_playlist_shuffle_allocated *= 2;
		}
		
		// allocate indexes.
		// release the previous indexes first: the grow path in
		// _viv_playlist_add_item frees before replacing, the initial
		// shuffle must not leak the old block.
		if (_viv_playlist_shuffle_indexes)
		{
			mem_free(_viv_playlist_shuffle_indexes);
			
			_viv_playlist_shuffle_indexes = 0;
		}
		
		_viv_playlist_shuffle_indexes = mem_alloc(safe_size_mul_sizeof_pointer((SIZE_T)_viv_playlist_shuffle_allocated));
		
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
void _viv_nav_item_free_all(void)
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
void _viv_nav_item_add(WIN32_FIND_DATA *fd)
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
int _viv_nav_compare(const void *va,const void *vb)
{
	const _viv_nav_item_t *a;
	const _viv_nav_item_t *b;

	a = (_viv_nav_item_t *)va;
	b = (_viv_nav_item_t *)vb;

	return _viv_fd_compare_name(&a->fd,&b->fd);
}
static INT_PTR CALLBACK _viv_search_everything_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	{
		INT_PTR dark_dialog_reply;
		
		dark_dialog_reply = _viv_dialog_dark_proc(hwnd,msg,wParam,lParam);
		
		if (dark_dialog_reply != -1)
		{
			return dark_dialog_reply;
		}
	}
	
	switch(msg)
	{
		case WM_INITDIALOG:
			// dark chrome: title bar and dark explorer control style.
			_viv_dark_dialog(hwnd);
			
			
			os_center_dialog(hwnd);
			
			SetWindowLongPtr(hwnd,GWLP_USERDATA,lParam);
			
			os_SetWindowText_localization_id(hwnd,lParam ? LOCALIZATION_ID_EVERYTHING_ADD_EVERYTHING_SEARCH_CAPTION : LOCALIZATION_ID_EVERYTHING_LOAD_EVERYTHING_SEARCH_CAPTION);
			os_SetDlgItemText_localization_id(hwnd,IDC_SEARCH_EVERYTHING_RANDOM_CHECKBOX,LOCALIZATION_ID_RANDOMIZE);
			
			return TRUE;
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					wchar_t search_wbuf[STRING_SIZE];
					
					GetDlgItemText(hwnd,IDC_EVERYTHING_EDIT,search_wbuf,STRING_SIZE);
					
					if (_viv_send_everything_search(hwnd,GetWindowLongPtr(hwnd,GWLP_USERDATA),IsDlgButtonChecked(hwnd,IDC_SEARCH_EVERYTHING_RANDOM_CHECKBOX) == BST_CHECKED,search_wbuf))
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
void _viv_search_everything(int add)
{
	DialogBoxParam(os_hinstance,MAKEINTRESOURCE(IDD_EVERYTHING),_viv_hwnd,_viv_search_everything_proc,add);
}
int _viv_send_everything_search(HWND hwnd,int add,int randomize,const wchar_t *search)
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
			
			srand((unsigned int)(counter.LowPart ^ counter.HighPart));
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
			
			string_copy_utf8_string(new_search,"ext:bmp;gif;ico;jpeg;jpg;png;tif;tiff;webp;emf;wmf <");
			string_cat(new_search,search);
			string_cat_utf8(new_search,">");

			size = (DWORD)safe_size_add(sizeof(EVERYTHING_IPC_QUERY2),safe_size_mul_sizeof_wchar(safe_size_add_one(string_get_length(new_search))));
			
			_viv_everything_request_flags = EVERYTHING_IPC_QUERY2_REQUEST_FULL_PATH_AND_NAME; 
					
			if (SendMessage(everything_hwnd,EVERYTHING_WM_IPC,EVERYTHING_IPC_IS_FILE_INFO_INDEXED,EVERYTHING_IPC_FILE_INFO_FILE_SIZE))
			{
				_viv_everything_request_flags |= EVERYTHING_IPC_QUERY2_REQUEST_SIZE; // date modified is requested by its own indexed check below 
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
			os_copy_memory(q+1,new_search,(string_get_length(new_search) + 1) * sizeof(wchar_t));
			
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
				
			string_copy_utf8_string(caption_wbuf,localization_get_string(LOCALIZATION_ID_APP_NAME));

			// MB_ICONQUESTION avoids the messagebeep.
			viv_msgbox(hwnd,caption_wbuf,text_wbuf,MB_OK|MB_ICONERROR);
				
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
				// cFileName is only MAX_PATH wchars: clamp the full path
				// copy or a deep directory smashes the stack find data.
				string_copy_with_bufsize(fd.cFileName,MAX_PATH,search_wbuf);
				
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
int _viv_playlist_shuffle_index_from_fd(const WIN32_FIND_DATA *fd)
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
_viv_playlist_t *_viv_playlist_from_fd(const WIN32_FIND_DATA *fd)
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
void _viv_do_initial_shuffle(void)
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
void _viv_send_random_everything_search(void)
{
	HWND everything_hwnd;
	
	everything_hwnd = FindWindowA(EVERYTHING_IPC_WNDCLASSA,0);
	
	if (everything_hwnd)
	{
		EVERYTHING_IPC_QUERY2 *q;
		COPYDATASTRUCT cds;
		DWORD size;
		wchar_t new_search[STRING_SIZE];
		
		string_copy_utf8_string(new_search,"ext:bmp;gif;ico;jpeg;jpg;png;tif;tiff;webp;emf;wmf <");
		string_cat(new_search,_viv_random);
		string_cat_utf8(new_search,">");

		size = (DWORD)safe_size_add(sizeof(EVERYTHING_IPC_QUERY2),safe_size_mul_sizeof_wchar(safe_size_add_one(string_get_length(new_search))));
		
		_viv_everything_request_flags = EVERYTHING_IPC_QUERY2_REQUEST_FULL_PATH_AND_NAME; 
				
		if (SendMessage(everything_hwnd,EVERYTHING_WM_IPC,EVERYTHING_IPC_IS_FILE_INFO_INDEXED,EVERYTHING_IPC_FILE_INFO_FILE_SIZE))
		{
			_viv_everything_request_flags |= EVERYTHING_IPC_QUERY2_REQUEST_SIZE; // date modified is requested by its own indexed check below 
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
		os_copy_memory(q+1,new_search,(string_get_length(new_search) + 1) * sizeof(wchar_t));
		
		cds.dwData = EVERYTHING_IPC_COPYDATA_QUERY2;
		cds.cbData = size;
		cds.lpData = q;
		
		SendMessage(everything_hwnd,WM_COPYDATA,(WPARAM)_viv_hwnd,(LPARAM)&cds);
		
		mem_free(q);
	}
}
// read size bytes at p from a WM_COPYDATA message into dst, advancing
// p. returns 0 when the read would leave the message: WM_COPYDATA
// arrives from arbitrary processes and every offset and length in it
// must be validated before it is trusted.
static const char *_viv_copydata_read(const COPYDATASTRUCT *cds,const char *p,void *dst,DWORD size)
{
	if (_viv_safe_copy_data(cds->lpData,cds->cbData,p,dst,size))
	{
		return p + size;
	}
	
	return 0;
}
// parse one Everything IPC item into a find data. the list2 header and
// the item array are validated by the caller; the per item data walk
// (filename, size, dates) is validated here field by field. returns 0
// when the item data lies outside the message: the item is then skipped
// instead of reading whatever the sender pointed at.
int _viv_everything_item_to_fd(const COPYDATASTRUCT *cds,const EVERYTHING_IPC_ITEM2 *item,WIN32_FIND_DATA *fd)
{
	const char *p;
	DWORD filename_len;
	
	os_zero_memory(fd,sizeof(WIN32_FIND_DATA));
	
	// EVERYTHING_IPC_QUERY2_REQUEST_FULL_PATH_AND_NAME
	p = ((const char *)cds->lpData) + item->data_offset;
	
	p = _viv_copydata_read(cds,p,&filename_len,sizeof(DWORD));
	
	if (!p)
	{
		return 0;
	}
	
	if (filename_len >= MAX_PATH)
	{
		return 0;
	}
	
	if (!(p = _viv_copydata_read(cds,p,fd->cFileName,filename_len * sizeof(wchar_t))))
	{
		return 0;
	}
	
	fd->cFileName[filename_len] = 0;
	
	// the null terminator after the filename, when present.
	if ((SIZE_T)(((const char *)cds->lpData) + cds->cbData - p) >= sizeof(wchar_t))
	{
		p += sizeof(wchar_t);
	}
	
	// EVERYTHING_IPC_QUERY2_REQUEST_SIZE
	if (_viv_everything_request_flags & EVERYTHING_IPC_QUERY2_REQUEST_SIZE)
	{
		if (!(p = _viv_copydata_read(cds,p,&fd->nFileSizeLow,sizeof(DWORD)))) return 0;
		if (!(p = _viv_copydata_read(cds,p,&fd->nFileSizeHigh,sizeof(DWORD)))) return 0;
	}
	
	// EVERYTHING_IPC_QUERY2_REQUEST_DATE_CREATED
	if (_viv_everything_request_flags & EVERYTHING_IPC_QUERY2_REQUEST_DATE_CREATED)
	{
		if (!(p = _viv_copydata_read(cds,p,&fd->ftCreationTime.dwLowDateTime,sizeof(DWORD)))) return 0;
		if (!(p = _viv_copydata_read(cds,p,&fd->ftCreationTime.dwHighDateTime,sizeof(DWORD)))) return 0;
	}
	
	// EVERYTHING_IPC_QUERY2_REQUEST_DATE_MODIFIED
	if (_viv_everything_request_flags & EVERYTHING_IPC_QUERY2_REQUEST_DATE_MODIFIED)
	{
		if (!(p = _viv_copydata_read(cds,p,&fd->ftLastWriteTime.dwLowDateTime,sizeof(DWORD)))) return 0;
		if (!(p = _viv_copydata_read(cds,p,&fd->ftLastWriteTime.dwHighDateTime,sizeof(DWORD)))) return 0;
	}
	
	return 1;
}
