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
// viv_view.h - command dispatch, navigation, zoom, mouse and touch gesture actions.
// VoidImageViewer
// viv_wndproc.c - the main window procedure: the message dispatch and the per-message handlers.

#include "viv.h"
#include "viv_state.h"
#include "viv_wndproc.h"
#include "viv_menu.h"
#include "viv_menubar.h"
#include "viv_recent.h"
#include "viv_playlist.h"
#include "viv_load.h"
#include "viv_anim.h"
#include "viv_render.h"
#include "viv_chrome.h"
#include "viv_dark.h"
#include "viv_dialogs.h"
#include "viv_view.h"
#include "viv_install.h"
#include "viv_menu.h"

// touch gesture messages. (not defined in older SDKs)
#ifndef WM_GESTURENOTIFY
#define WM_GESTURENOTIFY 0x011A
#endif

#ifndef WM_GESTURE
#define WM_GESTURE 0x0119
#endif

// moved from the viv.c core in one piece (R76): the window procedure, its
// case-body handlers, the drop-file helper and the paint/move statics they
// own. every case body moved byte-identical; only the case-exit break
// statements became explicit DefWindowProc returns.

// the move-window drag reference point (screen coordinates, captured at
// the WM_NCLBUTTONDOWN start; the deltas drive the WM_MOUSEMOVE drag).
static int _viv_mdoing_x;
static int _viv_mdoing_y;

// set while the WM_PAINT handler draws an animation frame: the erase
// pass reads it to skip clearing between frames.
static BYTE _viv_is_animation_paint = 0;

// cached background brush: one GDI allocation per color/theme change
// instead of one per paint.
HBRUSH _viv_background_hbrush = 0;
static COLORREF _viv_background_hbrush_color = 0;

// shared by the shell drop path and the clipboard file-list paste:
// queries the dropped names and rebuilds the playlist. the caller owns
// the hdrop lifetime - a shell wm_dropfiles must be followed by
// dragfinish (the shell allocated the list for the drop), a clipboard
// hdrop must not (the clipboard owns that memory).
static void _viv_drop_files(HWND hwnd,HDROP hdrop)
{
		wchar_t filename[STRING_SIZE];
		DWORD count;
		int is_shift;
		
		if (_viv_random)
		{
			mem_free(_viv_random);
			
			_viv_random = 0;
		}
		
		
		count = DragQueryFile(hdrop,0xFFFFFFFF,0,0);
		
		if (!count)
		{
			// a drop with zero files (e.g. a cancelled drag) must not
			// clear the current playlist.
			SetForegroundWindow(hwnd);
			return;
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
		
		
		if ((count >= 2) || (is_shift))
		{
			DWORD i;
			
			for(i=0;i<count;i++)
			{
				DragQueryFile(hdrop,i,filename,STRING_SIZE);
				
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
			DragQueryFile(hdrop,0,filename,STRING_SIZE);
			
			_viv_open_from_filename(filename);
		}
		
		SetForegroundWindow(hwnd);
}

WORD _viv_context_menu_items[] = 
{
	VIV_ID_NAV_NEXT,
	VIV_ID_NAV_PREV,
	0,
	// one zoom submenu groups the zoom commands instead of a flat pile
	// of them at the top level. the submenu markers push/pop the same way
	// the rate and sort submenus always have.
	_VIV_MENU_VIEW_ZOOM,
	VIV_ID_VIEW_ZOOM_IN,
	VIV_ID_VIEW_ZOOM_OUT,
	0,
	VIV_ID_VIEW_1TO1,
	VIV_ID_VIEW_BESTFIT,
	VIV_ID_VIEW_FILL_WINDOW,
	0,
	VIV_ID_VIEW_ALLOW_SHRINKING,
	VIV_ID_VIEW_KEEP_ASPECT_RATIO,
	_VIV_MENU_VIEW_ZOOM,
	0,
	VIV_ID_EDIT_ROTATE_90,
	VIV_ID_EDIT_ROTATE_270,
	0,
	VIV_ID_VIEW_FULLSCREEN,
	VIV_ID_SLIDESHOW_PAUSE,
	_VIV_MENU_SLIDESHOW_RATE,
	VIV_ID_SLIDESHOW_RATE_DEC,
	VIV_ID_SLIDESHOW_RATE_INC,
	0,
	// keep the handful of rates people actually use here; the complete
	// ladder still lives in the menu bar slideshow submenu.
	VIV_ID_SLIDESHOW_RATE_1000,
	VIV_ID_SLIDESHOW_RATE_3000,
	VIV_ID_SLIDESHOW_RATE_5000,
	VIV_ID_SLIDESHOW_RATE_10000,
	VIV_ID_SLIDESHOW_RATE_30000,
	VIV_ID_SLIDESHOW_RATE_60000,
	VIV_ID_SLIDESHOW_RATE_CUSTOM,
	_VIV_MENU_SLIDESHOW_RATE,
	0,
	VIV_ID_VIEW_MENU,
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
	VIV_ID_EDIT_CUT,
	VIV_ID_EDIT_COPY,
	VIV_ID_EDIT_COPY_IMAGE,
	VIV_ID_EDIT_PASTE,
	0,
	VIV_ID_FILE_DELETE,
	VIV_ID_FILE_RENAME,
	0,
	VIV_ID_FILE_PROPERTIES,
	VIV_ID_VIEW_OPTIONS,
	0,
	VIV_ID_FILE_EXIT,
};

#define _VIV_CONTEXT_MENU_ITEM_COUNT	(sizeof(_viv_context_menu_items) / sizeof(WORD))


static LRESULT _viv_on_wm_nchittest(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

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
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_nclbuttondown(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	
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
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_destroy(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// don't free the menu again.
	_viv_hmenu = 0;
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_queryendsession(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	return TRUE;
}

static LRESULT _viv_on_wm_endsession(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	if (wParam)
	{
		// save settings on logout. the deferred recent-files save
		// folds into this write: the timer cannot fire anymore.
		_viv_recent_save_fold();
		
		config_save_settings(config_appdata);
	}
	return 0;
}

static LRESULT _viv_on__retry_random_everything_search(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	_viv_send_random_everything_search();
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on__reply(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
					
					// defense in depth: the frame array below is allocated with
					// frame_count entries and frame 0 is written immediately after.
					// every current reply sender guarantees at least one frame, but
					// clamp here as well so a future sender can not corrupt the heap.
					if (!first_frame->frame_count)
					{
						first_frame->frame_count = 1;
					}
					
					// the count is a UINT here but is stored in an int below: clamp
					// the top as well so a huge count can not wrap the int (and
					// with it the frame array allocation). real animations are
					// orders of magnitude below this cap.
					if (first_frame->frame_count > 0x10000)
					{
						first_frame->frame_count = 0x10000;
					}
					
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
							_viv_preload_frames = (_viv_frame_t *)mem_alloc(safe_size_mul(sizeof(_viv_frame_t),(SIZE_T)_viv_preload_frame_count));
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
							// a low resolution preview must never end up in the last
							// image slot: the previous image would be lost to a
							// blurry thumbnail.
							if ((!(first_frame->is_low_res)) && (!(_viv_image_is_low_res)))
							{
								viv_copy_current_image_to_last_image();
							}

							_viv_clear();
							
							_viv_image_is_low_res = first_frame->is_low_res ? 1 : 0;

							_viv_image_wide = first_frame->wide;
							_viv_image_high = first_frame->high;
							_viv_frame_count = first_frame->frame_count;

							// allocate hbitmaps.
							_viv_frames = (_viv_frame_t *)mem_alloc(safe_size_mul(sizeof(_viv_frame_t),(SIZE_T)_viv_frame_count));
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
								// make sure we check the frame count too
								// incase we get an event from an old load.
								if (_viv_preload_frame_loaded_count < _viv_preload_frame_count)
								{
									_viv_preload_frames[_viv_preload_frame_loaded_count].hbitmap = additional_frame->hbitmap;
									_viv_preload_frames[_viv_preload_frame_loaded_count].mipmap = additional_frame->mipmap;
									_viv_preload_frames[_viv_preload_frame_loaded_count].delay = additional_frame->delay;
									
									_viv_preload_frame_loaded_count++;
									
									additional_frame->hbitmap = 0;
									additional_frame->mipmap = 0;
								}
								else
								{
									// a stale event from an old load: free the frame.
									if (additional_frame->hbitmap)
									{
										DeleteObject(additional_frame->hbitmap);
										
										additional_frame->hbitmap = 0;
									}
									
									if (additional_frame->mipmap)
									{
										_viv_mipmap_free(additional_frame->mipmap);
										
										additional_frame->mipmap = 0;
									}
								}
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
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_initmenu(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// the frame menu is gone: the popup state refresh runs against the
	// app menu tree (the remade top bar opens the same popups).
	_viv_check_menus(_viv_hmenu);
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_dropfiles(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	_viv_drop_files(hwnd,(HDROP)wParam);
	
	// the shell allocated the file list for this drop and expects
	// dragfinish to release it: every drop used to leak it.
	DragFinish((HDROP)wParam);
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_timer(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	
	switch(wParam)
	{
		case VIV_ID_DARK_RECHECK_TIMER:
		{
			// the delayed immersive color set re-check (see
			// wm_settingchange): the registry has settled by now.
			// the re-apply runs unconditionally: the flip catchers gate on
			// the dark answer, but the system side can repaint the chrome in
			// light without the answer ever changing (the app mode flush, the
			// theme broadcast sweep are asynchronous) - the sweep is idempotent
			// and heals any surface the system overpainted (the white band the
			// field caught after the options dialog and the theme switch).
			KillTimer(hwnd,VIV_ID_DARK_RECHECK_TIMER);
			
			os_dark_invalidate();
			
			if (config_dark_mode == 2)
			{
				os_dark_refresh();
			}
			
			_viv_apply_dark_mode(1);
		}
		break;

		case VIV_ID_RECENT_SAVE_TIMER:
		{
			// the deferred recent-files save (see _viv_recent_save_defer):
			// the open burst is over, write the settings once.
			KillTimer(hwnd,VIV_ID_RECENT_SAVE_TIMER);
			
			if (_viv_recent_save_dirty)
			{
				_viv_recent_save_dirty = 0;
				
				config_save_settings(config_appdata);
			}
		}
		break;
		
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
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_lbuttondblclk(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	_viv_show_cursor();
	_viv_update_show_cursor();


	if (_viv_is_touch_click())
	{
		// double tap on a touch screen: toggle 1:1 / best fit.
		_viv_touch_double_click();

		return DefWindowProc(hwnd,msg,wParam,lParam);
	}
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
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_lbuttondown(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	// a canvas click takes the keyboard home: the zoom pane editor and
	// the pill rows hold the focus while they are alive, and without this
	// the arrow keys and the page keys stay routed into their fields (a
	// dead navigation zone until enter, escape or a click elsewhere). the
	// focus move is what commits an open zoom editor (its kill focus
	// path).
	SetFocus(hwnd);
	
	_viv_show_cursor();
	_viv_update_show_cursor();
	
	_viv_do_left_click_action(config_left_click_action);

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_mbuttondown(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
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
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_rbuttondown_wm_rbuttondblclk(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	switch(config_right_click_action)
	{
		case 1:
			_viv_zoom_in(1,1,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
			return 0;

		case 2:
			_viv_next(1,1,0,0);
			return 0;
	}

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_rbuttonup(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	switch(config_right_click_action)
	{
		case 1:
			return 0;

		case 2:
			return 0;
	}

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_contextmenu(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
	
	// fresh owner draw rows for this throwaway menu (it is destroyed below).
	_viv_menu_row_pool_reset(_VIV_MENU_POOL_CONTEXT);
	
	{
		int i;
		HMENU submenu;
		HMENU curmenu;
		WORD submenuid;
		int was_seperator;
		
		submenuid = 0;
		curmenu = hmenu;
		was_seperator = 1;
		
		for(i=0;i<_VIV_CONTEXT_MENU_ITEM_COUNT;i++)
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
								string_copy_utf8_string(text_wbuf,localization_get_string(LOCALIZATION_ID_PLAY_PAUSE));
								break;
								
							default:
								string_copy_utf8_string(text_wbuf,localization_get_string(_viv_commands[command_index].localization_id));
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
							
						{
							void *row;

							row = _viv_menu_row_alloc(_VIV_MENU_POOL_CONTEXT,_VIV_MENU_DRAW_COMMAND,command_index,0,0);

							if (row)
							{
								AppendMenuW(curmenu,(_viv_commands[command_index].flags & (~(MF_DELETE|MF_OWNERDRAW))) | MF_OWNERDRAW,_viv_commands[command_index].command_id,(LPCWSTR)row);
							}
							else
							{
								AppendMenuW(curmenu,_viv_commands[command_index].flags & (~(MF_DELETE|MF_OWNERDRAW)),_viv_commands[command_index].command_id,text_wbuf);
							}
						}
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
								string_copy_utf8_string(text_wbuf,localization_get_string(LOCALIZATION_ID_RATE));
								break;
						
							default:
								string_copy_utf8_string(text_wbuf,localization_get_string(_viv_commands[command_index].localization_id));
								break;
						}
						
						submenuid = _viv_context_menu_items[i];
						submenu = CreatePopupMenu();
						{
							void *row;

							row = _viv_menu_row_alloc(_VIV_MENU_POOL_CONTEXT,_VIV_MENU_DRAW_POPUP,command_index,0,0);

							if (row)
							{
								AppendMenuW(curmenu,MF_POPUP | MF_OWNERDRAW,(UINT_PTR)submenu,(LPCWSTR)row);
							}
							else
							{
								AppendMenuW(curmenu,MF_POPUP,(UINT_PTR)submenu,text_wbuf);
							}
						}
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
					{
					void *row;

					row = _viv_menu_row_alloc(_VIV_MENU_POOL_CONTEXT,_VIV_MENU_DRAW_SEPARATOR,0,0,0);

					if (row)
					{
						AppendMenuW(curmenu,MF_SEPARATOR | MF_OWNERDRAW,0,(LPCWSTR)row);
					}
					else
					{
						AppendMenuW(curmenu,MF_SEPARATOR,0,0);
					}
				}

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
	
	_viv_recent_menu_flush();
	
	DestroyMenu(hmenu);
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_activate(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	
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
	
	// the top bar dims its labels when the window is inactive: repaint
	// on every activation change.
	_viv_menubar_repaint();
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_mouseleave(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

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

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_mousemove(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	// mouse motion is overlay activity (idle fade timer).
	zoomui_activity();

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
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_lbuttonup_wm_mbuttonup(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	_viv_doing_cancel();
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_mousewheel(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	_viv_do_mousewheel_action(_viv_get_current_key_mod_flags() == CONFIG_KEYFLAG_CTRL ? config_ctrl_mouse_wheel_action : config_mouse_wheel_action,GET_WHEEL_DELTA_WPARAM(wParam),GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_gesturenotify(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	if (os_SetGestureConfig)
	{
		os_GestureConfig_t gesture_configs[3];

		// windows gesture ids (winuser.h): GID_ZOOM 3, GID_PAN 4,
		// GID_TWOFINGERTAP 6 (5 is GID_ROTATE - do not "fix" this to 5;
		// verified against winuser.h). the want/block flags: GC_ZOOM 1, GC_PAN 1
		// and GC_PAN_WITH_INERTIA 0x10; single finger pan (0x02|0x04) and
		// gutters (0x08) are blocked so single touches keep mouse
		// semantics.
		gesture_configs[0].dwID = 3; // GID_ZOOM
		gesture_configs[0].dwWant = 1; // GC_ZOOM
		gesture_configs[0].dwBlock = 0;

		gesture_configs[1].dwID = 4; // GID_PAN
		gesture_configs[1].dwWant = 0x11; // GC_PAN | GC_PAN_WITH_INERTIA
		gesture_configs[1].dwBlock = 0x0E; // single finger v|h | gutter

		gesture_configs[2].dwID = 6; // GID_TWOFINGERTAP
		gesture_configs[2].dwWant = 1; // GC_TWOFINGERTAP
		gesture_configs[2].dwBlock = 0;

		// cIDs is the number of configurations, cbSize is the size of
		// one GESTURECONFIG (the wrapper signature matches winuser.h).
		os_SetGestureConfig(hwnd,0,3,gesture_configs,sizeof(os_GestureConfig_t));
	}

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_gesture(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	if (_viv_on_gesture(hwnd,(void *)lParam))
	{
		return 0;
	}

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_tablet_querysystemgesturestatus(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// disable press-and-hold (0x1, the wait circle) and flicks
	// (0x10000, the navigation gestures): both fight the touch pan
	// and the two finger tap. tap and pen feedback stay enabled.
	return 0x00000001 | 0x00010000;
}

static LRESULT _viv_on_wm_copydata(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
			EVERYTHING_IPC_LIST2 list;
			
			// validate the list header before trusting anything in the
			// reply: WM_COPYDATA arrives from arbitrary processes.
			if (_viv_safe_copy_data(cds->lpData,cds->cbData,cds->lpData,&list,sizeof(list)))
			{
				debug_printf("%d / %d results\n",list.numitems,list.totitems);
				
				if (list.numitems)
				{
					EVERYTHING_IPC_ITEM2 item;
					
					if (_viv_safe_copy_data(cds->lpData,cds->cbData,((char *)cds->lpData) + sizeof(EVERYTHING_IPC_LIST2),&item,sizeof(item)))
					{
						if (item.flags & EVERYTHING_IPC_FOLDER)
						{
							// add this folder ?
						}
						else
						{
							WIN32_FIND_DATA fd;
							
							if (_viv_everything_item_to_fd(cds,&item,&fd))
							{
								if (_viv_is_valid_filename(&fd))
								{
									_viv_open(&fd,0);
								}
							}
						}
					}
				}
				else
				if (list.totitems)
				{
					// our random index was too high, try again.
					_viv_random_tot_results = list.totitems;
					
					PostMessage(hwnd,_VIV_WM_RETRY_RANDOM_EVERYTHING_SEARCH,0,0);
				}
			}
			
			break;
		}

		// Everything reply
		case _VIV_COPYDATA_OPEN_EVERYTHING_SEARCH:
		case _VIV_COPYDATA_ADD_EVERYTHING_SEARCH:
		{
			EVERYTHING_IPC_LIST2 list;
			
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
			
			// validate the list header and the item array before
			// trusting any item offsets from the sender.
			if (_viv_safe_copy_data(cds->lpData,cds->cbData,cds->lpData,&list,sizeof(list)))
			{
				DWORD i;
				DWORD max_items;
				
				debug_printf("%d / %d results\n",list.numitems,list.totitems);
				
				// clamp the item count to what the message can actually hold:
				// a lying numitems would otherwise spin the loop below for
				// billions of iterations of failing validations.
				max_items = (DWORD)((cds->cbData - sizeof(EVERYTHING_IPC_LIST2)) / sizeof(EVERYTHING_IPC_ITEM2));
				
				for(i=0;(i < list.numitems) && (i < max_items);i++)
				{
					EVERYTHING_IPC_ITEM2 item;
					
					if (_viv_safe_copy_data(cds->lpData,cds->cbData,((char *)cds->lpData) + sizeof(EVERYTHING_IPC_LIST2) + (i * sizeof(EVERYTHING_IPC_ITEM2)),&item,sizeof(item)))
					{
						if (item.flags & EVERYTHING_IPC_FOLDER)
						{
							// add this folder ?
						}
						else
						{
							WIN32_FIND_DATA fd;
							
							if (_viv_everything_item_to_fd(cds,&item,&fd))
							{
								if (_viv_is_valid_filename(&fd))
								{
									_viv_playlist_add(&fd);
								}
							}
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
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_close(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	_viv_exit();
	return 0;
}

static LRESULT _viv_on_wm_syscommand(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	
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
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_size(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	_viv_on_size();

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_dpichanged(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	const RECT *suggested_rect;
	int dpi_changed;
	
	// refresh the dpi derived metrics before the resize below
	// triggers any layout work.
	dpi_changed = os_window_update_dpi(hwnd);
	
	if (dpi_changed)
	{
		glyphs_flush_cache();
		
		// the menu bar font follows the new dpi.
		_viv_menu_font_drop();
	}
	
	// accept the suggested rectangle: it keeps the window at its
	// logical size at the new dpi.
	suggested_rect = (const RECT *)lParam;
	/* WM_DPICHANGED supplies a suggested RECT, but synthetic messages and
	 * compatibility layers may not.  Keep the resize optional instead of
	 * dereferencing an invalid pointer on older Windows paths. */
	if (suggested_rect)
	{
		SetWindowPos(hwnd,0,suggested_rect->left,suggested_rect->top,suggested_rect->right - suggested_rect->left,suggested_rect->bottom - suggested_rect->top,SWP_NOZORDER|SWP_NOACTIVATE);
	}
	
	if (dpi_changed)
	{
		// the strip re-measures its labels at the new dpi inside the size
		// sweep (the self drawn strip owns its metrics); the top bar
		// re-reads its labels at the new font size too.
		_viv_menubar_layout();
		
		_viv_on_size();
	}
	
	return 0;
}

static LRESULT _viv_on_wm_move(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

// the popup menu rows measure themselves on the theme font: the menu
// sizes to the widest row.
static LRESULT _viv_on_wm_measureitem(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	if ((lParam) && (((MEASUREITEMSTRUCT *)lParam)->CtlType == ODT_MENU))
	{
		if (_viv_menu_measure_item((MEASUREITEMSTRUCT *)lParam))
		{
			return TRUE;
		}
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

// the classic palette changed (system colors, high contrast): the token
// brushes hold resolved system colors, so flush them and repaint - a
// silent no-op when nothing actually moved.
static LRESULT _viv_on_wm_syscolorchange(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	viv_theme_refresh();
	
	_viv_apply_dark_mode(1);
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

// owner drawn rows ride static pools: the delete notification is just
// acknowledged (nothing to free, but the reply must be TRUE so the
// system knows the item data was consumed).
static LRESULT _viv_on_wm_deleteitem(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	if ((lParam) && (((DELETEITEMSTRUCT *)lParam)->CtlType == ODT_MENU))
	{
		return TRUE;
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

// mnemonic keys resolve against the live row text (owner drawn rows keep
// their labels out of the system, so the scan is ours).
static LRESULT _viv_on_wm_menuchar(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	int index;
	
	index = _viv_menu_char_item((HMENU)lParam,(wchar_t)(wParam & 0xffff),(int)HIWORD(wParam));
	
	if (index != -1)
	{
		return MAKELRESULT(index,MNC_EXECUTE);
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_drawitem(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// the status panes are owner drawn: draw them FIRST. the comctl
	// status bar sends its pane draws with CtlType == ODT_MENU (a
	// historical quirk), so the menu branch below can never be the
	// discriminator: it would cast the pane index (the itemData) into a
	// menu row pointer and dereference address 0x1 - the crash the
	// smoke sweep caught on every slow-decoded image (the preload pane
	// draws while the load thread still runs; the baseline routed the
	// panes by the control id and that check is the true one).
	if ((wParam == VIV_ID_STATUS) && (_viv_status_draw_item((DRAWITEMSTRUCT *)lParam)))
	{
		return TRUE;
	}
	
	// the popup menus are owner drawn: every row paints on the theme
	// tokens (the system painter never touches them). menu draws carry
	// no control id, so the pane check above never takes them.
	if ((lParam) && (((DRAWITEMSTRUCT *)lParam)->CtlType == ODT_MENU))
	{
		if (_viv_menu_draw_item((DRAWITEMSTRUCT *)lParam))
		{
			return TRUE;
		}
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_syschar(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// alt + mnemonic: the remade top bar owns the root item mnemonics
	// (the frame menu is gone, so the system no longer handles them).
	if (_viv_menubar_open_mnemonic((int)wParam))
	{
		return 0;
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_syskeydown(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// f10 opens the first menu: the classic menu key the frame menu
	// used to own.
	if (wParam == VK_F10)
	{
		_viv_menubar_open_first();
		
		return 0;
	}
	
	// the alt press flips the underline policy (hidden until alt is
	// held): repaint the bar on the first press, not the auto repeats.
	if ((wParam == VK_MENU) && (!(lParam & 0x40000000)))
	{
		_viv_menubar_repaint();
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_syskeyup(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// the alt release hides the mnemonics again: repaint the bar.
	if (wParam == VK_MENU)
	{
		_viv_menubar_repaint();
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_initmenupopup(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// trackpopupmenuex sends this before showing a popup: the state
	// refresh the frame menu used to get from wm_initmenu runs here.
	_viv_check_menus(_viv_hmenu);
	
	// the popup layer theming is shared now (viv_menu owns it): the
	// settings dropdowns run the same code from their own owner
	// window.
	_viv_menu_popup_theme();
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_settingchange(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	// system settings changed (theme, high contrast, ...). the uxtheme
	// color policy is only current after a refresh and the menus only
	// re-theme after a flush, so do both when the immersive color set
	// changed.
	if ((lParam) && (string_compare((const wchar_t *)lParam,L"ImmersiveColorSet") == 0))
	{
		os_dark_refresh();
		
		// the broadcast can arrive before the personalize registry
		// value settles: the immediate re-read below then sees the
		// old state and skips the apply, and nothing re-triggers. a
		// one-shot timer re-checks after the settle window so the flip
		// always lands.
		SetTimer(_viv_hwnd,VIV_ID_DARK_RECHECK_TIMER,400,0);
	}
	
	// re-read the dark state (the theme itself or the high contrast
	// accessibility switch may flip it) and re-apply the chrome only
	// when it actually changed: unrelated broadcasts are frequent and
	// must not cause chrome churn.
	{
		int was_dark;
		int is_dark;
		
		was_dark = _viv_is_dark();
		
		os_dark_invalidate();
		
		is_dark = _viv_is_dark();
		
		if (was_dark != is_dark)
		{
			if (config_dark_mode == 2)
			{
				os_dark_refresh();
			}
			
			_viv_apply_dark_mode(1);
		}
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_themechanged(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	// the system font metrics may follow the theme: drop the cached
	// menu font and re-read the top bar layout at the new metrics.
	_viv_menu_font_drop();
	
	_viv_menubar_layout();
	
	// the visual style flip can race the registry the same way the
	// immersive color set broadcast does: schedule the one shot
	// re-check so a late settle still lands the theme.
	SetTimer(_viv_hwnd,VIV_ID_DARK_RECHECK_TIMER,400,0);
	
	// the visual style changed (classic, high contrast or a theme
	// switch). re-read the dark state and re-apply the chrome
	// unconditionally: wm_themechanged means the system re-themed the
	// comctl classes and the frame, so the per window dark state must be
	// re-asserted even when the dark answer itself did not flip - the
	// flip gate is what let the system light repaint survive (the white
	// band after the theme switch).
	{
		os_dark_invalidate();
		
		if (config_dark_mode == 2)
		{
			os_dark_refresh();
		}
		
		_viv_apply_dark_mode(1);
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_setcursor(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	// hand cursor over the clickable zoom pane (status bar part 0).
	if (_viv_status_hwnd && ((HWND)wParam == _viv_status_hwnd))
	{
		POINT pt;
		RECT pane_rect;
		
		GetCursorPos(&pt);
		ScreenToClient(_viv_status_hwnd,&pt);
		
		if ((SendMessage(_viv_status_hwnd,SB_GETRECT,0,(LPARAM)&pane_rect)) && (PtInRect(&pane_rect,pt)))
		{
			SetCursor(LoadCursor(NULL,IDC_HAND));
			
			return TRUE;
		}
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_notify(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	switch(((NMHDR *)lParam)->idFrom)
	{
		case VIV_ID_STATUS:
		
			if (_viv_status_hwnd)
			{

				switch(((NMHDR *)lParam)->code)
				{
					case NM_CUSTOMDRAW:
					{
						NMCUSTOMDRAW *custom_draw;
						
						// dark status bar: the comctl32 status bar has no dark
						// theme, so the parts are painted dark here. (the size grip
						// is theme drawn and stays light.)
						custom_draw = (NMCUSTOMDRAW *)lParam;
						
						if (custom_draw->dwDrawStage == CDDS_PREPAINT)
						{
							return CDRF_NOTIFYITEMDRAW;
						}
						
						if (custom_draw->dwDrawStage == CDDS_ITEMPREPAINT)
						{
							if (_viv_is_dark())
							{
								SetTextColor(custom_draw->hdc,RGB(0xE8,0xE8,0xE8));
								SetBkColor(custom_draw->hdc,RGB(0x20,0x20,0x20));
							}
							
							return CDRF_DODEFAULT;
						}
						
						break;
					}
					
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
							case 0:
								// the zoom pane: type an exact percent.
								_viv_set_zoom_dialog();
								break;
							
							default:
								// the frame counter pane is followed only by the dimension
								// pane. clicking it toggles the frame numbering direction.
								if (item == (int)SendMessage(_viv_status_hwnd,SB_GETPARTS,0,0) - 2)
								{
									config_frame_minus = !config_frame_minus;
									_viv_status_update();
								}
								break;
						}
													
						break;
					}
				}
			}

			break;
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_command(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	_viv_command(LOWORD(wParam));

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_paste(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

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
				// reuse the drop handler directly: posting wm_dropfiles to
			// ourselves skipped the dragfinish discipline, and finishing
			// the clipboards own hdrop would corrupt the clipboard memory.
			_viv_drop_files(hwnd,hdrop);

				GlobalUnlock(hglobal);
			}
		}
		else
		{
			// no filenames on the clipboard: show an image copied from
			// another application instead (paint, a browser, a screenshot
			// tool, ...).
			_viv_paste_clipboard_image();
		}

		CloseClipboard();
	}

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static LRESULT _viv_on_wm_enter_idle(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// the menu modal loop idles on the owner while a popup tracks: pump the
	// pending mouse moves so the bar keeps hovering and the popup follows
	// across the roots.
	if (wParam == MSGF_MENU)
	{
		_viv_menubar_idle_pump();
	}

	return 0;
}

static LRESULT _viv_on_wm_paint(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	RECT rect;
	int wide;
	int high;
	int view_top;
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
	// the viewport sits between the top strips and the status bar: the
	// image math below is viewport relative, the blits land in client
	// coordinates (origin added at each dst y).
	high = rect.bottom - rect.top - _viv_get_status_high() - _viv_get_view_top();
	
	view_top = _viv_get_view_top();

	if (BeginPaint(hwnd,&ps))
	{
		HRGN update_hrgn;
		HDC paint_hdc;
		int paint_use_backbuffer;

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

		// double buffer: render into a memory bitmap and present it to the
		// screen with a single blit. this makes each paint atomic and prevents
		// tearing (partial frames) while panning or zooming.
		// rtl layouts keep painting directly, blitting a mirrored memory dc would be wrong.
		paint_use_backbuffer = 0;
		paint_hdc = ps.hdc;
		
//debug_printf("WM_PAINT\n")			;
		if ((wide) && (high))
		{
			int rx;
			int ry;
			int rw;
			int rh;

			// begin the backbuffer only when there is a viewport to render into.
			if (!((os_GetLayout) && (os_GetLayout(ps.hdc) & LAYOUT_RTL)))
			{
				if (_viv_paint_begin(ps.hdc,rect.right - rect.left,rect.bottom - rect.top))
				{
					paint_use_backbuffer = 1;
					paint_hdc = _viv_paint_hdc;
				}
			}

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
								if (BitBlt(paint_hdc,rx,ry + view_top,rw,rh,mem_hdc,0,0,SRCCOPY))
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
										last_stretch_mode = SetStretchBltMode(paint_hdc,HALFTONE);
										SetBrushOrgEx(paint_hdc,-rx,-ry - view_top,NULL);
									}
									else
									{
										last_stretch_mode = SetStretchBltMode(paint_hdc,COLORONCOLOR);
									}

									did_set_stretch_blt_mode = 1;
								}
								else
								if ((rw > mip_wide) || (rh > mip_high))
								{
									if (config_mag_filter == CONFIG_MAG_FILTER_HALFTONE)
									{
										is_halftone = 1;
										last_stretch_mode = SetStretchBltMode(paint_hdc,HALFTONE);
									}
									else
									{
										last_stretch_mode = SetStretchBltMode(paint_hdc,COLORONCOLOR);
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
														if (SelectClipRgn(paint_hdc,clip_hrgn) != ERROR)
														{
															if (StretchBlt(paint_hdc,rx,ry + view_top,rw,rh,mem_hdc,0,0,mip_wide,mip_high,SRCCOPY))
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
														// magnified StretchBlt ignores the clipping region: it stretches the
														// entire destination rectangle even when only a small part of it is
														// visible. only take the whole-destination path when the destination
														// is fully on screen, otherwise use the clip limited stretch. (a deep
														// zoom renders up to 16x the fit size; stretching hundreds of
														// megapixels on every paint was the zoom lag.)
														if ((rw <= wide) && (rh <= high) && (rect_p->right - rect_p->left >= paint_wide) && (rect_p->bottom - rect_p->top >= paint_high))
														{
															// if the clipping region is the full area, just use stretchblt, which is faster.
															if (_viv_StretchBltStitch(paint_hdc,rx,ry + view_top,rw,rh,mem_hdc,0,0,mip_wide,mip_high,SRCCOPY,rect_p->left,rect_p->top,rect_p->right - rect_p->left,rect_p->bottom - rect_p->top))
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
															// where-as StretchBlt ignores the clipping region and renders the entire dst region.
															_viv_stretch_blt(paint_hdc,rx,ry + view_top,rw,rh,mem_hdc,mip_wide,mip_high,rect_p->left,rect_p->top,rect_p->right - rect_p->left,rect_p->bottom - rect_p->top);
														}
													}
													else
													{
														if (_viv_StretchBltStitch(paint_hdc,rx,ry + view_top,rw,rh,mem_hdc,0,0,mip_wide,mip_high,SRCCOPY,rect_p->left,rect_p->top,rect_p->right - rect_p->left,rect_p->bottom - rect_p->top))
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
												SelectClipRgn(paint_hdc,NULL);
											}
										}

										small_pool_kill(&region_data_small_pool);
									}
								}

								if (did_set_stretch_blt_mode)
								{
									SetStretchBltMode(paint_hdc,last_stretch_mode);
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
				COLORREF brush_color;
				
				brush_color = _viv_is_fullscreen ? RGB(config_fullscreen_background_color_r,config_fullscreen_background_color_g,config_fullscreen_background_color_b) : _viv_windowed_background();
				
				// reuse the background brush across paints: its color only changes
				// with the config or the theme, so a paint no longer allocates and
				// frees a GDI brush each frame.
				if ((!_viv_background_hbrush) || (_viv_background_hbrush_color != brush_color))
				{
					if (_viv_background_hbrush)
					{
						DeleteObject(_viv_background_hbrush);
					}
					
					_viv_background_hbrush = CreateSolidBrush(brush_color);
					_viv_background_hbrush_color = brush_color;
				}
				
				if (_viv_background_hbrush)
				{
					os_fill_clipped_rect(paint_hdc,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,rx,ry + view_top,rw,rh,_viv_background_hbrush);
				}
			}
		}
		
		if (paint_use_backbuffer)
		{
			// present the frame: copy the invalidated region to the screen in one blit.
			SelectClipRgn(ps.hdc,update_hrgn);
			
			if (!BitBlt(ps.hdc,0,0,rect.right - rect.left,rect.bottom - rect.top,paint_hdc,0,0,SRCCOPY))
			{
				debug_printf("paint present BitBlt failed %d\n",GetLastError());
			}
			
			SelectClipRgn(ps.hdc,NULL);
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

static LRESULT _viv_on_wm_erasebkgnd(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// the dark ui erases with the chrome face: a paint that bypasses the
	// handlers (a relayout gap, a system ghost redraw) must not flash the
	// light class brush the window still carries for the light ui (the
	// 1.1.03 lesson). the canvas paint covers the whole client right
	// after, so the fill only shows where nothing else paints - dark
	// instead of the white slab.
	if (_viv_is_dark())
	{
		RECT rect;
		
		GetClientRect(hwnd,&rect);
		
		FillRect((HDC)wParam,&rect,_viv_dark_chrome_brush(0));
	}
	
	return 1;
}

static LRESULT _viv_on_wm_getminmaxinfo(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	{
		int wide;
		int high;
		RECT rect;
		BOOL is_menu;
		
		// the strip is full width now: the minimum window is a floor, not
		// the toolbar's content width (the overflow rule hides groups).
		wide = (480 * os_logical_wide) / 96;
		high = _viv_get_status_high() + _viv_get_view_top();
		
		is_menu = GetMenu(_viv_hwnd) ? TRUE : FALSE;
	
		rect.left = 0;
		rect.top = 0;
		rect.right = wide;
		rect.bottom = high;
		
		AdjustWindowRectEx(&rect,os_get_window_style(hwnd),is_menu,os_get_window_ex_style(hwnd));

		((MINMAXINFO *)lParam)->ptMinTrackSize.x = rect.right - rect.left; 
		((MINMAXINFO *)lParam)->ptMinTrackSize.y = rect.bottom - rect.top;
	}

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

LRESULT CALLBACK _viv_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
#ifdef VIVP_SELF_SHOT
	{
		static HANDLE fh;
		char m[4];
		DWORD w;
		if (!fh) fh = CreateFileW(L"C:\\shots\\msgs.log",FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_ALWAYS,0,0);
		m[0]=(char)(msg&0xff);
		m[1]=(char)((msg>>8)&0xff);
		m[2]=(char)((msg>>16)&0xff);
		m[3]=(char)((msg>>24)&0xff);
		if (fh != INVALID_HANDLE_VALUE) { WriteFile(fh,m,4,&w,0); FlushFileBuffers(fh); }
	}
#endif
	switch (msg)
	{
		case WM_NCHITTEST:
			return _viv_on_wm_nchittest(hwnd,msg,wParam,lParam);
		case WM_NCLBUTTONDOWN:
			return _viv_on_wm_nclbuttondown(hwnd,msg,wParam,lParam);
		case WM_DESTROY:
			return _viv_on_wm_destroy(hwnd,msg,wParam,lParam);
		case WM_QUERYENDSESSION:
			return _viv_on_wm_queryendsession(hwnd,msg,wParam,lParam);
		case WM_ENDSESSION:
			return _viv_on_wm_endsession(hwnd,msg,wParam,lParam);
		case _VIV_WM_RETRY_RANDOM_EVERYTHING_SEARCH:
			return _viv_on__retry_random_everything_search(hwnd,msg,wParam,lParam);
		case _VIV_WM_REPLY:
			return _viv_on__reply(hwnd,msg,wParam,lParam);
		case WM_INITMENU:
			return _viv_on_wm_initmenu(hwnd,msg,wParam,lParam);
		case WM_DROPFILES:
			return _viv_on_wm_dropfiles(hwnd,msg,wParam,lParam);
		case WM_TIMER:
			return _viv_on_wm_timer(hwnd,msg,wParam,lParam);
		case WM_LBUTTONDBLCLK:
			return _viv_on_wm_lbuttondblclk(hwnd,msg,wParam,lParam);
		case WM_LBUTTONDOWN:
			return _viv_on_wm_lbuttondown(hwnd,msg,wParam,lParam);
		case WM_MBUTTONDOWN:
			return _viv_on_wm_mbuttondown(hwnd,msg,wParam,lParam);
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			return _viv_on_wm_rbuttondown_wm_rbuttondblclk(hwnd,msg,wParam,lParam);
		case WM_RBUTTONUP:
			return _viv_on_wm_rbuttonup(hwnd,msg,wParam,lParam);
		case WM_CONTEXTMENU:
			return _viv_on_wm_contextmenu(hwnd,msg,wParam,lParam);
		case WM_ACTIVATE:
			return _viv_on_wm_activate(hwnd,msg,wParam,lParam);
		case WM_MOUSELEAVE:
			return _viv_on_wm_mouseleave(hwnd,msg,wParam,lParam);
		case WM_MOUSEMOVE:
			return _viv_on_wm_mousemove(hwnd,msg,wParam,lParam);
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
			return _viv_on_wm_lbuttonup_wm_mbuttonup(hwnd,msg,wParam,lParam);
		case WM_MOUSEWHEEL:
			return _viv_on_wm_mousewheel(hwnd,msg,wParam,lParam);
		case WM_GESTURENOTIFY:
			return _viv_on_wm_gesturenotify(hwnd,msg,wParam,lParam);
		case WM_GESTURE:
			return _viv_on_wm_gesture(hwnd,msg,wParam,lParam);
		case 0x2C4: // WM_TABLET_QUERYSYSTEMGESTURESTATUS (winuser.h)
			return _viv_on_wm_tablet_querysystemgesturestatus(hwnd,msg,wParam,lParam);
		case WM_COPYDATA:
			return _viv_on_wm_copydata(hwnd,msg,wParam,lParam);
		case WM_CLOSE:
			return _viv_on_wm_close(hwnd,msg,wParam,lParam);
		case WM_SYSCOMMAND:
			return _viv_on_wm_syscommand(hwnd,msg,wParam,lParam);
		case WM_SIZE:
			return _viv_on_wm_size(hwnd,msg,wParam,lParam);
		case WM_DPICHANGED:
			return _viv_on_wm_dpichanged(hwnd,msg,wParam,lParam);
		case WM_MOVE:
			return _viv_on_wm_move(hwnd,msg,wParam,lParam);
		case WM_DRAWITEM:
			return _viv_on_wm_drawitem(hwnd,msg,wParam,lParam);
		case WM_DELETEITEM:
			return _viv_on_wm_deleteitem(hwnd,msg,wParam,lParam);
		case WM_MEASUREITEM:
			return _viv_on_wm_measureitem(hwnd,msg,wParam,lParam);
		case WM_SYSCOLORCHANGE:
			return _viv_on_wm_syscolorchange(hwnd,msg,wParam,lParam);
		case WM_MENUCHAR:
			return _viv_on_wm_menuchar(hwnd,msg,wParam,lParam);
		case WM_ENTERIDLE:
			return _viv_on_wm_enter_idle(hwnd,msg,wParam,lParam);
		case WM_SYSCHAR:
			return _viv_on_wm_syschar(hwnd,msg,wParam,lParam);
		case WM_SYSKEYDOWN:
			return _viv_on_wm_syskeydown(hwnd,msg,wParam,lParam);
		case WM_SYSKEYUP:
			return _viv_on_wm_syskeyup(hwnd,msg,wParam,lParam);
		case WM_INITMENUPOPUP:
			return _viv_on_wm_initmenupopup(hwnd,msg,wParam,lParam);
		case WM_SETTINGCHANGE:
			return _viv_on_wm_settingchange(hwnd,msg,wParam,lParam);
		case WM_THEMECHANGED:
			return _viv_on_wm_themechanged(hwnd,msg,wParam,lParam);
		case WM_SETCURSOR:
			return _viv_on_wm_setcursor(hwnd,msg,wParam,lParam);
		case WM_NOTIFY:
			return _viv_on_wm_notify(hwnd,msg,wParam,lParam);
		case WM_COMMAND:
			return _viv_on_wm_command(hwnd,msg,wParam,lParam);
		case WM_PASTE:
			return _viv_on_wm_paste(hwnd,msg,wParam,lParam);
		case WM_PAINT:
			return _viv_on_wm_paint(hwnd,msg,wParam,lParam);
		case WM_ERASEBKGND:
			return _viv_on_wm_erasebkgnd(hwnd,msg,wParam,lParam);
		case WM_GETMINMAXINFO:
			return _viv_on_wm_getminmaxinfo(hwnd,msg,wParam,lParam);
	}

	return DefWindowProc(hwnd,msg,wParam,lParam);
}
