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
// viv_load.c - image loading, decode thread, clipboard images, preload.
// Pure physical move from viv.c (R70 split): bodies are unchanged.
#include "viv.h"
#include "viv_state.h"
#include "viv_load.h"
#include "viv_anim.h"
#include "viv_chrome.h"
#include "viv_playlist.h"
#include "viv_recent.h"
#include "viv_render.h"
#include "viv_view.h"

// forward declarations (order preserved from viv.c)
void _viv_clear(void);
void _viv_process_pending_clear(void);
void _viv_clear_loading_preload(void);
void _viv_clear_preload_frames(void);
void _viv_clear_preload(void);
BOOL _viv_open_from_filename(const wchar_t *filename);
void _viv_open(WIN32_FIND_DATA *fd,int is_preload);
void _viv_set_clipboard_image(void);
static void _viv_show_clipboard_image(HBITMAP hbitmap,int wide,int high);
void _viv_paste_clipboard_image(void);
static int _viv_get_extension_format(const wchar_t *filename);
void _viv_save_image_as(void);
void _viv_doing_cancel(void);
void _viv_blank(void);
static DWORD WINAPI _viv_load_image_thread_proc(void *param);
void _viv_reply_free(_viv_reply_t *e);
_viv_reply_t *_viv_reply_add(DWORD type,DWORD size,void *data);
void _viv_reply_clear_all(void);
void _viv_preload_next(void);
void _viv_activate_preload(void);
void viv_copy_current_image_to_last_image(void);
static void _viv_activate_last(void);
void _viv_clear_last(void);
void _viv_refresh(void);
void _viv_open_preload(void);
static int _viv_pixel_budget_refused(SIZE_T pixels);
int _viv_safe_copy_data(const void *base,SIZE_T src_size,const void *src,void *dst,SIZE_T dst_size);


static GUID _viv_FrameDimensionTime = {0x6aedbd6d,0x3fb5,0x418a,{0x83,0xa6,0x7f,0x45,0x22,0x9d,0xc8,0x72}};
static BYTE _viv_preload_is_prev = 0; // preload next or previous?
static _viv_frame_t *_viv_pending_clear_frames = NULL;
static int _viv_pending_clear_frame_loaded_count = 0;
static int _viv_last_image_wide = 0; // last image width
static int _viv_last_image_high = 0; // last image height
// clearing is really slow.
// delay this until after the new image is shown.
// we add the clear to a queue which is cleared with _viv_process_pending_clear.
// _viv_process_pending_clear should be called after the new frame is shown.
void _viv_clear(void)
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
	_viv_image_is_low_res = 0;
	_viv_image_wide = 0;
	_viv_image_high = 0;
	_viv_animation_play = 1;
}
void _viv_process_pending_clear(void)
{
	if (_viv_pending_clear_frames)
	{
		_viv_clear_frames(_viv_pending_clear_frames,_viv_pending_clear_frame_loaded_count);
		
		_viv_pending_clear_frames = NULL;
	}

	_viv_pending_clear_frame_loaded_count = 0;
}
void _viv_clear_loading_preload(void)
{
	if (_viv_load_image_thread)
	{
		if (_viv_load_is_preload)
		{
			_viv_load_image_terminate = 1;
		}
	}
}
void _viv_clear_preload_frames(void)
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
void _viv_clear_preload(void)
{
	_viv_clear_preload_frames();
	
	_viv_preload_fd->cFileName[0] = 0;
}
BOOL _viv_open_from_filename(const wchar_t *filename)
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
			string_copy_with_bufsize(fd.cFileName,MAX_PATH,full_path_and_filename);
			
			// every single-file open (dialog, drop, command line, mru itself)
			// feeds the recent list.
			_viv_recent_file_push(full_path_and_filename);
			
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
					string_copy_with_bufsize(fd.cFileName,MAX_PATH,full_path_and_filename);
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
void _viv_open(WIN32_FIND_DATA *fd,int is_preload)
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
		_viv_load_render_high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_view_top();
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
void _viv_set_clipboard_image(void)
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
							
							// if SetClipboardData fails we keep ownership
							// and must free the bitmap ourselves.
							if (!SetClipboardData(CF_BITMAP,mem1_hbitmap))
							{
								DeleteObject(mem1_hbitmap);
							}
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
// show a bitmap that came from the clipboard.
// pasted images have no filename: the title falls back to the application
// name and every filename based command (save as, delete, rename, copy to,
// ...) already checks for an empty filename and quietly does nothing.
static void _viv_show_clipboard_image(HBITMAP hbitmap,int wide,int high)
{
	// stop an in flight file load from clobbering the pasted image.
	_viv_load_image_allow_draw = 0;
	_viv_load_image_terminate = 1;
	
	// the current image moves to the last image slot, exactly like
	// navigating to a new image does.
	viv_copy_current_image_to_last_image();
	
	_viv_clear();
	
	_viv_image_wide = wide;
	_viv_image_high = high;
	_viv_frame_count = 1;
	_viv_frame_loaded_count = 1;
	_viv_frames = (_viv_frame_t *)mem_alloc(sizeof(_viv_frame_t));
	
	_viv_frames[0].hbitmap = hbitmap;
	_viv_frames[0].mipmap = 0; // built lazily on the first paint.
	_viv_frames[0].delay = 0;
	
	// a clipboard image has no filename.
	_viv_current_fd->cFileName[0] = 0;
	
	_viv_update_title();
	
	_viv_start_first_frame();
	
	_viv_process_pending_clear();
}
// the clipboard must already be open by the caller.
// dib first: the system synthesizes a CF_DIB for nearly every image
// source. a plain bitmap handle is the fallback.
void _viv_paste_clipboard_image(void)
{
	HGLOBAL hglobal;
	
	hglobal = GetClipboardData(CF_DIB);
	
	if (hglobal)
	{
		BITMAPINFOHEADER *bih;
		
		bih = (BITMAPINFOHEADER *)GlobalLock(hglobal);
		if (bih)
		{
			// only the classic 40 byte header with an uncompressed (or
			// bitfields) layout is handled here, anything else falls through
			// to the CF_BITMAP copy below.
			if ((bih->biSize == sizeof(BITMAPINFOHEADER)) && (bih->biPlanes == 1) && (bih->biWidth > 0) && (bih->biHeight != 0) && ((bih->biCompression == 0 /* BI_RGB */) || (bih->biCompression == 3 /* BI_BITFIELDS */)) && ((bih->biBitCount == 1) || (bih->biBitCount == 4) || (bih->biBitCount == 8) || (bih->biBitCount == 16) || (bih->biBitCount == 24) || (bih->biBitCount == 32)))
			{
				HDC screen_hdc;
				
				screen_hdc = GetDC(0);
				if (screen_hdc)
				{
					void *bits;
					HBITMAP hbitmap;
					int budget_height;
					
					// apply the same decode-time pixel budget as the file loaders: a
					// hostile clipboard dib must not force a giant allocation.
					budget_height = (bih->biHeight < 0) ? -bih->biHeight : bih->biHeight;
					hbitmap = 0;
					if (!_viv_pixel_budget_refused(safe_size_mul((SIZE_T)bih->biWidth,(SIZE_T)budget_height)))
					{
						hbitmap = CreateDIBSection(screen_hdc,(BITMAPINFO *)bih,DIB_RGB_COLORS,&bits,NULL,0);
					}
					if (hbitmap)
					{
						DWORD color_count;
						int mask_size;
						const char *src;
						int height;
						int stride;
						
						// the clipboard dib layout: header, bitfield masks (40 byte
						// headers with BI_BITFIELDS only), palette, bits.
						color_count = 0;
						if (bih->biBitCount <= 8)
						{
							color_count = bih->biClrUsed ? bih->biClrUsed : (1 << bih->biBitCount);
						}
						
						mask_size = ((bih->biCompression == 3 /* BI_BITFIELDS */) && ((bih->biBitCount == 16) || (bih->biBitCount == 32))) ? 12 : 0;
						
						src = (const char *)bih + bih->biSize + mask_size + color_count * 4;
						
						height = bih->biHeight;
						if (height < 0)
						{
							height = -height;
						}
						
						stride = (int)(((DWORD)bih->biWidth * (DWORD)bih->biBitCount + 31) / 32) * 4;
						
						// the clipboard global must actually contain the whole
						// dib (header, masks, palette and bits): a short or hostile
						// clipboard would otherwise be read past its end.
						if (((SIZE_T)GlobalSize(hglobal)) >= ((SIZE_T)bih->biSize + (SIZE_T)mask_size + (SIZE_T)color_count * 4) + ((SIZE_T)stride * (SIZE_T)height))
						{
							// size_t length: stride*height crosses int_max inside the 64-bit
						// pixel budget (400 mp x 4 bytes per pixel).
						os_copy_memory(bits,src,(SIZE_T)stride * (SIZE_T)height);
							
							_viv_show_clipboard_image(hbitmap,(int)bih->biWidth,height);
							
							ReleaseDC(0,screen_hdc);
							GlobalUnlock(hglobal);
							
							return;
						}
						
						DeleteObject(hbitmap);
					}
					
					ReleaseDC(0,screen_hdc);
				}
			}
			
			GlobalUnlock(hglobal);
		}
	}
	
	// fall back to a plain bitmap handle.
	{
		HBITMAP hbitmap;
		
		hbitmap = (HBITMAP)GetClipboardData(CF_BITMAP);
		
		if (hbitmap)
		{
			HBITMAP hbitmap_copy;
			BITMAP bm;
			
			// the clipboard owns the original: make a private dib copy.
			hbitmap_copy = (HBITMAP)CopyImage(hbitmap,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
			
			if (hbitmap_copy)
			{
				if ((GetObject(hbitmap_copy,sizeof(BITMAP),&bm)) && (bm.bmWidth > 0) && (bm.bmHeight > 0))
				{
					_viv_show_clipboard_image(hbitmap_copy,bm.bmWidth,bm.bmHeight);
					
					return;
				}
				
				DeleteObject(hbitmap_copy);
			}
		}
	}
}
// get the save format for a filename extension. (0 = png, 1 = jpeg, 2 = bmp, -1 = unknown)
static int _viv_get_extension_format(const wchar_t *filename)
{
	int i;
	int last_dot;
	
	last_dot = -1;
	
	for(i=string_get_length(filename)-1;i>=0;i--)
	{
		if (filename[i] == L'\\')
		{
			break;
		}
		
		if (filename[i] == L'.')
		{
			last_dot = i;
			break;
		}
	}
	
	if (last_dot == -1)
	{
		return -1;
	}
	
	if (_viv_icompare_w(filename + last_dot + 1,L"png") == 0)
	{
		return 0;
	}
	
	if ((_viv_icompare_w(filename + last_dot + 1,L"jpg") == 0) || (_viv_icompare_w(filename + last_dot + 1,L"jpeg") == 0))
	{
		return 1;
	}
	
	if (_viv_icompare_w(filename + last_dot + 1,L"bmp") == 0)
	{
		return 2;
	}
	
	return -1;
}
// save the current image to a new file. (png, jpeg or bmp)
// re-encodes with the GDI+ built-in encoders, no new dependencies.
// the in-memory rotation is included, alpha is flattened over the window background.
void _viv_save_image_as(void)
{
	if (*_viv_current_fd->cFileName)
	{
		if (_viv_frame_count)
		{
			// do not save while the progressive preview is on screen:
			// frames[0] is still the low resolution thumbnail until the
			// full first frame lands.
			if (_viv_image_is_low_res)
			{
				wchar_t message_wbuf[STRING_SIZE];
				wchar_t caption_wbuf[STRING_SIZE];
				
				string_copy_utf8_string(message_wbuf,localization_get_string(LOCALIZATION_ID_STATUS_BAR_LOADING));
				string_copy_utf8_string(caption_wbuf,localization_get_string(LOCALIZATION_ID_APP_NAME));
				
				viv_msgbox(_viv_hwnd,caption_wbuf,message_wbuf,MB_OK|MB_ICONINFORMATION);
				
				return;
			}
			
			OPENFILENAME ofn;
			wchar_t tobuf[STRING_SIZE+1];
			wchar_t filter_wbuf[STRING_SIZE];
			wchar_t title_wbuf[STRING_SIZE];
			int i;
			int last_dot;
			
			os_zero_memory(&ofn,sizeof(OPENFILENAME));
			
			// default to the current filename with a .png extension.
			string_copy(tobuf,_viv_current_fd->cFileName);
			
			last_dot = -1;
			
			for(i=string_get_length(tobuf)-1;i>=0;i--)
			{
				if (tobuf[i] == L'\\')
				{
					break;
				}
				
				if (tobuf[i] == L'.')
				{
					last_dot = i;
					break;
				}
			}
			
			if (last_dot != -1)
			{
				tobuf[last_dot] = 0;
			}
			
			string_cat(tobuf,L".png");
			
			string_printf(filter_wbuf,"%s (*.png)%c*.png%c%s (*.jpg)%c*.jpg%c%s (*.bmp)%c*.bmp%c",
				localization_get_string(LOCALIZATION_ID_SAVE_AS_PNG),0,
				localization_get_string(LOCALIZATION_ID_SAVE_AS_JPEG),0,
				localization_get_string(LOCALIZATION_ID_SAVE_AS_BMP),0,0);
			
			string_copy_utf8_string(title_wbuf,localization_get_string(LOCALIZATION_ID_SAVE_AS_CAPTION));
			
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
				int format;
				
				// use the typed extension when we recognize it, otherwise the selected filter.
				format = _viv_get_extension_format(tobuf);
				
				if (format == -1)
				{
					const wchar_t *extension;
					
					format = (int)ofn.nFilterIndex - 1;
					
					if ((format < 0) || (format > 2))
					{
						format = 0;
					}
					
					extension = (format == 1) ? L".jpg" : ((format == 2) ? L".bmp" : L".png");
					
					// make room for the extension: string_cat stops at the
					// STRING_SIZE budget, a full buffer would silently save
					// the file without its extension.
					if (string_get_length(tobuf) > ((STRING_SIZE - 1) - string_get_length(extension)))
					{
						tobuf[(STRING_SIZE - 1) - string_get_length(extension)] = 0;
					}
					
					string_cat(tobuf,extension);
				}
				
				// save the frame on screen, not frame 0.
				if (!os_save_hbitmap(_viv_frames[_viv_frame_position].hbitmap,tobuf,format))
				{
					wchar_t message_wbuf[STRING_SIZE];
					wchar_t caption_wbuf[STRING_SIZE];
					
					string_copy_utf8_string(message_wbuf,localization_get_string(LOCALIZATION_ID_SAVE_AS_FAILED));
					string_copy_utf8_string(caption_wbuf,localization_get_string(LOCALIZATION_ID_APP_NAME));
					
					viv_msgbox(_viv_hwnd,caption_wbuf,message_wbuf,MB_OK|MB_ICONERROR);
				}
			}
		}
	}
}
void _viv_doing_cancel(void)
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
void _viv_blank(void)
{
	_viv_clear();

	// the blank state has no file: the stale error flags from the last
	// load attempt must not outlive it (the field report: close after a
	// failed load kept "failed to load image" on the status bar with no
	// image open - the flags only reset on the next _viv_open).
	if (_viv_file_not_found)
	{
		_viv_file_not_found = 0;
	}

	if (_viv_load_failed)
	{
		_viv_load_failed = 0;
	}

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
	first_frame.is_low_res = 0;

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
						// pixel budget: a hostile or corrupted header can claim
						// a canvas that decodes to gigabytes of rgba. refuse the
						// load before any frame allocation happens; the dispose
						// below still runs so this fails like an unloadable file.
						if ((os_GdipGetImageHeight(image,&first_frame.high) == 0) && (!_viv_pixel_budget_refused(safe_size_mul((SIZE_T)first_frame.wide,(SIZE_T)first_frame.high))))
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
							// progressive display: post the embedded thumbnail as a
							// low resolution first frame while the full decode continues.
							// cameras and phones embed a small thumbnail in the exif data;
							// gdi+ returns it without decoding a single scanline of the
							// full image, so a big image appears instantly (blurry) and
							// sharpens when the full frame arrives.
							if ((!_viv_load_is_preload) && (os_GdipGetImageThumbnail) && (((VIV_UINT64)load_wide * (VIV_UINT64)load_high) > 2000000))
							{
								UINT thumb_data_size;
								
								// PropertyTagThumbnailData 0x501A: only take this path when an
								// embedded thumbnail actually exists, otherwise gdi+ would
								// decode the full image just to build one.
								thumb_data_size = 0;
								
								if ((os_GdipGetPropertyItemSize(image,0x501A,&thumb_data_size) == 0) && (thumb_data_size > 0))
								{
									void *thumb_image;
									
									thumb_image = NULL;
									
									// request at most 160x120: gdi+ serves the embedded thumbnail
									// without decoding when the request fits inside it.
									if (os_GdipGetImageThumbnail(image,160,120,&thumb_image,NULL,NULL) == 0)
									{
										UINT thumb_wide;
										UINT thumb_high;
										
										thumb_wide = 0;
										thumb_high = 0;
										
										if ((os_GdipGetImageWidth(thumb_image,&thumb_wide) == 0) && (os_GdipGetImageHeight(thumb_image,&thumb_high) == 0) && (thumb_wide) && (thumb_high))
										{
											HDC screen_hdc;
											
											screen_hdc = GetDC(0);
											
											if (screen_hdc)
											{
												HDC mem_hdc;
												
												mem_hdc = CreateCompatibleDC(screen_hdc);
												
												if (mem_hdc)
												{
													HBITMAP hbitmap;
													
													hbitmap = CreateCompatibleBitmap(screen_hdc,(int)thumb_wide,(int)thumb_high);
													
													if (hbitmap)
													{
														HGDIOBJ last_hbitmap;
														
														last_hbitmap = SelectObject(mem_hdc,hbitmap);
														
														{
															void *g;
															
															if (os_GdipCreateFromHDC(mem_hdc,&g) == 0)
															{
																// thumbnails carry no alpha worth preserving.
																os_GdipSetCompositingMode(g,1);
																
																// Gdiplus::InterpolationModeNearestNeighbor: the thumbnail
																// is drawn at its own size.
																os_GdipSetInterpolationMode(g,5);
																
																if (os_GdipDrawImageRectI(g,thumb_image,0,0,(int)thumb_wide,(int)thumb_high) == 0)
																{
																	// apply orientation: 5-8 swap the axes.
																	if ((orientation >= 5) && (orientation <= 8))
																	{
																		UINT temp;
																		
																		temp = thumb_wide;
																		thumb_wide = thumb_high;
																		thumb_high = temp;
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
																	
																	{
																		_viv_reply_load_image_first_frame_t low_res_first_frame;
																		
																		low_res_first_frame.wide = thumb_wide;
																		low_res_first_frame.high = thumb_high;
																		low_res_first_frame.frame_count = 1;
																		low_res_first_frame.frame.hbitmap = hbitmap;
																		low_res_first_frame.frame.mipmap = NULL; // no mipmap: the full frame provides them
																		low_res_first_frame.frame.delay = 0;
																		low_res_first_frame.is_low_res = 1;
																		
																		_viv_reply_add(_VIV_REPLY_LOAD_IMAGE_FIRST_FRAME,sizeof(low_res_first_frame),&low_res_first_frame);
																		
																		// ownership moved to the reply.
																		hbitmap = 0;
																	}
																}
																
																os_GdipDeleteGraphics(g);
															}
														}
														
														SelectObject(mem_hdc,last_hbitmap);
													}
													
													if (hbitmap)
													{
														DeleteObject(hbitmap);
													}
												}
												
												DeleteDC(mem_hdc);
											}
											
											ReleaseDC(0,screen_hdc);
										}
									}
									
									if (thumb_image)
									{
										os_GdipDisposeImage(thumb_image);
									}
								}
							}
							


							//First of all we should get the number of frame dimensions
							//Images considered by GDI+ as:
							//frames[animation_frame_index][how_many_animation];
							if (os_GdipImageGetFrameDimensionsCount(image,&count) == 0)
							{
								os_PropertyItem_t *frame_delay;
								SIZE_T frame_delay_size;
								
								frame_delay = NULL;
								frame_delay_size = 0;
								
								//Now we should get the identifiers for the frame dimensions 
								{
									GUID *DimensionIDs;
									
									// validate the count from gdi+ before trusting it: a zero count
									// would read dimension set #0 outside a zero length allocation and
									// an oversized count would wrap the size multiplication.
									if ((count >= 1) && (count <= (SIZE_MAX / sizeof(GUID))))
									{
										DimensionIDs = mem_alloc(sizeof(GUID) * count);
										
										if (os_GdipImageGetFrameDimensionsList(image,DimensionIDs,count) == 0)
										{
											//For gif image , we only care about animation set#0
											os_GdipImageGetFrameCount(image,&DimensionIDs[0],&first_frame.frame_count);
										}
										
										mem_free(DimensionIDs);
									}
								}

								// get frame delays.
								if (first_frame.frame_count > 1)
								{
									UINT size;
									
									size = 0;
									
									// PropertyTagFrameDelay 0x5100: gdi+ leaves *size untouched
									// when the property is absent, so seed it and check the return.
									if ((os_GdipGetPropertyItemSize(image,0x5100,&size) == 0) && (size >= sizeof(os_PropertyItem_t)))
									{
										frame_delay = (os_PropertyItem_t *)mem_alloc(size);
										
										if (frame_delay)
										{
											frame_delay_size = size;
											
											// PropertyTagFrameDelay 0x5100: only trust the buffer when
											// gdi+ actually filled it.
											if (os_GdipGetPropertyItem(image,0x5100,size,frame_delay) != 0)
											{
												mem_free(frame_delay);
												frame_delay = 0;
												frame_delay_size = 0;
											}
										}
									}
									
									debug_printf("frame delay size %u\n",(unsigned int)frame_delay_size);
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
																// fill the backdrop under the transparent
															// pixels (cached brushes, a single FillRect).
															_viv_fill_backdrop(mem_hdc,load_wide,load_high);
														
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
														UINT frame_data_value;

														frame.hbitmap = hbitmap;
														frame.mipmap = NULL;
														
														// therube: we are accessing bad data here for some images.
														// just use a value of 0 for bad data.
														frame_data_value = _viv_frame_delay_at(frame_delay,frame_delay_size,i);
														
														frame.delay = frame_data_value * 10;
															
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
														UINT frame_data_value;

														first_frame.frame.hbitmap = hbitmap;
														first_frame.frame.mipmap = NULL;
														first_frame.frame.delay = 0;
														
														if (first_frame.frame_count > 1)
														{
															// therube: we are accessing bad data here for some images.
															// just use a value of 0 for bad data.
															frame_data_value = _viv_frame_delay_at(frame_delay,frame_delay_size,i);
															
															first_frame.frame.delay = frame_data_value * 10;
															
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
				
				if (webp_load(stream,&viv_webp,
					(int (*)(void *,DWORD,DWORD,DWORD,int))_viv_webp_info_proc,
					(int (*)(void *,BYTE *,int))_viv_webp_frame_proc))
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
void _viv_reply_free(_viv_reply_t *e)
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
_viv_reply_t *_viv_reply_add(DWORD type,DWORD size,void *data)
{
	_viv_reply_t *e;
	int is_first;
	
	is_first = 0;
	
	e = (_viv_reply_t *)mem_alloc(safe_size_add(sizeof(_viv_reply_t),size));
	
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
void _viv_reply_clear_all(void)
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
void _viv_preload_next(void)
{
	if (config_preload_next)
	{
		//UpdateWindow(_viv_hwnd);
		
		_viv_next(_viv_last_is_prev,0,1,0);
	}
}
// the preload has completed and we 
// want to set it as the current image.
void _viv_activate_preload(void)
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
void viv_copy_current_image_to_last_image(void)
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
void _viv_clear_last(void)
{
	if (_viv_last_frames)
	{
		_viv_clear_frames(_viv_last_frames,_viv_last_frame_count);
		
		_viv_last_frames = NULL;
		_viv_last_frame_count = 0;
	}
}
void _viv_refresh(void)
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

	// no file open: the reload below would only fail against the empty
	// name and pin a load error on the blank window (the field report:
	// changing the backdrop or the canvas color on the bare program showed
	// a load failure at the bottom left). the blank state is the correct
	// result there: reset the stale flags and keep the canvas.
	if (!fd.cFileName[0])
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
		
		return;
	}

	_viv_open(&fd,0);
}
void _viv_open_preload(void)
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
// read the 10ms delay of frame i. returns 0 on any failure, the caller
// falls back to 100ms. the gdi+ value pointer is validated against the
// property buffer before it is used, and the delay array may hold fewer
// entries than there are frames (the gif GCE block is optional), so delays
// are reused modulo the available count.
static int _viv_pixel_budget_refused(SIZE_T pixels)
{
	if (pixels > VIV_MAX_IMAGE_PIXELS)
	{
		debug_printf("pixel budget: refusing a %u mp canvas (ceiling %u mp)\r\n",(unsigned int)(pixels / 1000000),(unsigned int)(VIV_MAX_IMAGE_PIXELS / 1000000));
		
		return 1;
	}
	
	return 0;
}
int _viv_safe_copy_data(const void *base,SIZE_T src_size,const void *src,void *dst,SIZE_T dst_size)
{
	const BYTE *p;
	BYTE *d;
	SIZE_T run;
	SIZE_T end;
	
	if (((const BYTE *)src) < ((const BYTE *)base))
	{
		return 0;
	}
	
	end = safe_size_add(((const BYTE *)src) - ((const BYTE *)base),dst_size);
	
	if (end == SIZE_MAX)
	{
		return 0;
	}
	
	if (end > src_size)
	{
		return 0;
	}
	
	p = (const BYTE *)src;
	d = (BYTE *)dst;
	run = dst_size;
	
	while(run)
	{
		*d++ = *p;
		p++;
		run--;
	}
	
	return 1;
}
