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
// viv_dialogs.c - modal dialogs: options, jump-to, about, rename, rate.
// the zoom editor lives in place on the status bar pane, not in a box.
// Pure physical move from viv.c (R70 split): bodies are unchanged.
#include "viv.h"
#include "viv_state.h"
#include "viv_dialogs.h"
#include "viv_menubar.h"
#include "viv_chrome.h"
#include "viv_dark.h"
#include "viv_install.h"
#include "viv_load.h"
#include "viv_menu.h"
#include "viv_playlist.h"
#include "viv_render.h"
#include "viv_view.h"

// forward declarations (order preserved from viv.c)
static HBRUSH _viv_about_light_brush(int which);
static BOOL CALLBACK _viv_dialog_font_child(HWND hwnd,LPARAM lParam);
void _viv_dialog_apply_font(HWND hwnd);
static INT_PTR CALLBACK _viv_rename_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
void _viv_rename(void);
static INT_PTR CALLBACK _viv_options_general_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void _viv_options_key_list_sel_change(HWND hwnd,int previous_key_index);
static void _viv_options_remove_key(HWND hwnd);
static INT_PTR CALLBACK _viv_edit_key_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void _viv_options_edit_key(HWND hwnd,int key_index);
static INT_PTR CALLBACK _viv_options_controls_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK _viv_options_view_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void _viv_options_treeview_changed(HWND hwnd);
static void _viv_options_update_sheild(HWND hwnd);
static LRESULT CALLBACK _viv_options_tab_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK _viv_options_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
void _viv_options(void);
INT_PTR CALLBACK _viv_custom_rate_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static INT_PTR _viv_about_colors(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK _viv_about_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
void _viv_set_zoom_dialog(void);
static void _viv_zoom_edit_end(HWND hwnd,int apply);
static LRESULT CALLBACK _viv_zoom_edit_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
void _viv_command_line_options(void);
static void _viv_update_color_button_bitmap(HWND hwnd);
static void _viv_delete_color_button_bitmap(HWND hwnd);
static LRESULT CALLBACK _viv_edit_key_edit_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void _viv_options_edit_key_changed(HWND hwnd);
static void _viv_edit_key_set_key(HWND hwnd,DWORD key_flags);
static void _viv_edit_key_remove_currently_used_by(_viv_key_list_t *keylist,DWORD keyflags);
static void _viv_jumpto_on_size(HWND hwnd);
static void _viv_jumpto_on_search(HWND hwnd);
static void _viv_jumpto_open_sel(HWND hwnd);
static LRESULT CALLBACK _viv_jumpto_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
void _viv_show_jumpto(void);
static void _viv_center_listbox_item(HWND listbox_hwnd,int item_index);


static localization_id_t _viv_options_page_localization_id_array[] = {LOCALIZATION_ID_OPTIONS_GENERAL_DIALOG,LOCALIZATION_ID_OPTIONS_VIEW_DIALOG,LOCALIZATION_ID_OPTIONS_CONTROLS_DIALOG};
static int _viv_options_tab_ids[] = {IDC_TAB1,IDC_TAB2,IDC_TAB3};
static int _viv_options_dialog_ids[] = {IDD_GENERAL,IDD_VIEW,IDD_CONTROLS};
typedef char _viv_options_dialog_ids_count_assert[(sizeof(_viv_options_dialog_ids) / sizeof(int) == _VIV_OPTIONS_PAGE_COUNT) ? 1 : -1]; // the count literal pins the table
static DLGPROC _viv_options_page_procs[] = {_viv_options_general_proc,_viv_options_view_proc,_viv_options_controls_proc};
static BYTE _viv_jump_ret = 0;
const WORD _viv_association_dlg_item_id[] = 
{
	IDC_BMP,
	IDC_GIF,
	IDC_ICO,
	IDC_JPEG,
	IDC_JPG,
	IDC_PNG,
	IDC_TIF,
	IDC_TIFF,
	IDC_WEBP,
	IDC_EMF,
	IDC_WMF,
};
// the about dialog band in the light ui takes the fixed win11 command
// bar palette instead of the system button colors (the band used to
// follow the windows theme of the machine - the fixed face holds one
// look everywhere). 0 = the separator line, 1 = the face. released in
// _viv_kill with the other cached brushes.
static HBRUSH _viv_about_light_brush(int which)
{
	static const COLORREF colors[2] = {RGB(0xEC,0xEC,0xEC),RGB(0xFF,0xFF,0xFF)};
	
	if ((which < 0) || (which > 1))
	{
		return 0;
	}
	
	if (!_viv_about_light_hbrushes[which])
	{
		_viv_about_light_hbrushes[which] = CreateSolidBrush(colors[which]);
	}
	
	return _viv_about_light_hbrushes[which];
}
static BOOL CALLBACK _viv_dialog_font_child(HWND hwnd,LPARAM lParam)
{
	SendMessage(hwnd,WM_SETFONT,(WPARAM)lParam,MAKELPARAM(TRUE,0));
	
	return TRUE;
}
// give a dialog and every child control the message font. the shared
// dialog proc runs this from wm_initdialog before each dialog's own
// case (and again from wm_dpichanged when the window crosses
// monitors): the dark flip captures the native combo field heights
// against the final font, the about title derives its larger face
// from it and the localized labels render with it.
void _viv_dialog_apply_font(HWND hwnd)
{
	LOGFONTW lf;
	HFONT font;
	HFONT old_font;
	
	if (!os_dialog_font(&lf,hwnd))
	{
		return;
	}
	
	font = CreateFontIndirectW(&lf);
	
	if (!font)
	{
		return;
	}
	
	old_font = (HFONT)GetPropW(hwnd,_VIV_DIALOG_FONT_PROP);
	
	SetPropW(hwnd,_VIV_DIALOG_FONT_PROP,(HANDLE)font);
	
	// the dialog window itself first: WM_GETFONT on the dialog
	// reports the template face until this set.
	SendMessage(hwnd,WM_SETFONT,(WPARAM)font,MAKELPARAM(TRUE,0));
	
	EnumChildWindows(hwnd,_viv_dialog_font_child,(LPARAM)font);
	
	// every child took the new face inside this same message: the old
	// handle has no user left, so it can die here (a paint can never
	// interleave the synchronous broadcast).
	if (old_font)
	{
		DeleteObject(old_font);
	}
}
static INT_PTR CALLBACK _viv_rename_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
			
		{
			wchar_t name[STRING_SIZE];
			
			os_center_dialog(hwnd);
			
			os_SetWindowText_localization_id(hwnd,LOCALIZATION_ID_RENAME_CAPTION);
			
			string_copy(name,string_get_filename_part((wchar_t *)lParam));
			string_remove_extension(name);

			SetDlgItemText(hwnd,IDC_RENAME_OLD_EDIT,(wchar_t *)lParam);
			SetDlgItemText(hwnd,IDC_RENAME_EDIT,name);

			return TRUE;
		}
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
					
					{
						wchar_t old_filename[STRING_SIZE];
						wchar_t new_filename[STRING_SIZE];
						wchar_t path[STRING_SIZE];
						wchar_t new_full_path_and_filename[STRING_SIZE];
						wchar_t *ext;
						int dont_end_dialog;
						
						GetDlgItemText(hwnd,IDC_RENAME_OLD_EDIT,old_filename,STRING_SIZE);
						GetDlgItemText(hwnd,IDC_RENAME_EDIT,new_filename,STRING_SIZE);
						
						string_get_path_part(path,old_filename);
						ext = string_get_extension(old_filename);
						dont_end_dialog = 0;
						
						string_path_combine(new_full_path_and_filename,path,new_filename);
						if (*ext)
						{
							string_cat_utf8(new_full_path_and_filename,(const utf8_t *)".");
							string_cat(new_full_path_and_filename,ext);
						}
						
						if (string_compare(old_filename,new_full_path_and_filename) != 0)
						{
							SHFILEOPSTRUCT fo;
							
							wchar_t old_filename_list[STRING_SIZE+1];
							wchar_t new_filename_list[STRING_SIZE+1];
							
							string_copy_double_null(old_filename_list,old_filename);
							string_copy_double_null(new_filename_list,new_full_path_and_filename);
							
							ZeroMemory(&fo,sizeof(SHFILEOPSTRUCT));
							fo.hwnd = _viv_hwnd;
							fo.wFunc = FO_RENAME;
							fo.pFrom = old_filename_list;
							fo.pTo = new_filename_list;
							fo.fFlags = FOF_ALLOWUNDO | FOF_WANTMAPPINGHANDLE;

							// returns ERROR_CANCELLED if user cancelled.
							if (SHFileOperation(&fo) == 0)
							{
								if (fo.fAnyOperationsAborted)
								{
									// user clicked No to a rename collision
									// (not cancelled).
									dont_end_dialog = 1;
								}
								else
								{	
									const wchar_t *file_op_new_name;
									
									file_op_new_name = new_full_path_and_filename;
									
									if (fo.hNameMappings)
									{
										_viv_name_mapping_t *mappings;
										
										mappings = (_viv_name_mapping_t *)fo.hNameMappings;
											
										if (mappings->count == 1)
										{
											// use the resolved name incase there was a rename collision.
											file_op_new_name = mappings->mappings[0].pszNewPath;
										}
									}	
									
									// has the current file changed? slideshow could make this a different filename
									if (string_compare(old_filename,_viv_current_fd->cFileName) == 0)
									{
										// rename
										string_copy_with_bufsize(_viv_current_fd->cFileName,MAX_PATH,file_op_new_name);
										
										_viv_playlist_rename(old_filename,file_op_new_name);

										_viv_update_title();
									}						
								}

								if (fo.hNameMappings)
								{
									SHFreeNameMappings(fo.hNameMappings);
								}							
							}
						}

						if (!dont_end_dialog)
						{
							EndDialog(hwnd,1);
						}
					}
					
					break;
				

				case IDCANCEL:
					EndDialog(hwnd,0);
					break;
			}

			break;
	}
	
	return FALSE;
}
void _viv_rename(void)
{
	if (*_viv_current_fd->cFileName)
	{
		DialogBoxParam(os_hinstance,MAKEINTRESOURCE(IDD_RENAME),_viv_hwnd,_viv_rename_proc,(LPARAM)_viv_current_fd->cFileName);
	}
}
static INT_PTR CALLBACK _viv_options_general_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
			
		{
			int exti;

			os_SetDlgItemText_localization_id(hwnd,IDC_APPDATA,LOCALIZATION_ID_STORE_SETTINGS_APPDATA);
			os_SetDlgItemText_localization_id(hwnd,IDC_MULTIPLE_INSTANCES,LOCALIZATION_ID_ALLOW_MULTIPLE_INSTANCES);
			os_SetDlgItemText_localization_id(hwnd,IDC_STARTMENU,LOCALIZATION_ID_STARTMENU_SHORTCUTS);
			os_SetDlgItemText_localization_id(hwnd,IDC_ASSOCIATIONS_GROUPBOX,LOCALIZATION_ID_ASSOCIATIONS);
			os_SetDlgItemText_localization_id(hwnd,IDC_CHECKALL,LOCALIZATION_ID_CHECK_ALL);
			os_SetDlgItemText_localization_id(hwnd,IDC_CHECKNONE,LOCALIZATION_ID_CHECK_NONE);
			
			// language selection. entries: auto, english, simplified chinese.
			// (language names are always shown in their own language)
			os_SetDlgItemText_localization_id(hwnd,IDC_LANGUAGE_STATIC,LOCALIZATION_ID_OPTIONS_LANGUAGE_STATIC);
			os_ComboBox_AddString_localization_id(hwnd,IDC_LANGUAGE,LOCALIZATION_ID_LANGUAGE_AUTO);
			os_ComboBox_AddString(hwnd,IDC_LANGUAGE,localization_get_language_name(LOCALIZATION_LANGUAGE_ENGLISH));
			os_ComboBox_AddString(hwnd,IDC_LANGUAGE,localization_get_language_name(LOCALIZATION_LANGUAGE_CHINESE_SIMPLIFIED));
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_LANGUAGE),config_language);

			// dark mode selection: automatic (follow the windows theme), light or dark.
			os_SetDlgItemText_localization_id(hwnd,IDC_DARKMODE_STATIC,LOCALIZATION_ID_OPTIONS_DARK_MODE_STATIC);
			os_ComboBox_AddString_localization_id(hwnd,IDC_DARKMODE,LOCALIZATION_ID_DARK_MODE_AUTO);
			os_ComboBox_AddString_localization_id(hwnd,IDC_DARKMODE,LOCALIZATION_ID_DARK_MODE_LIGHT);
			os_ComboBox_AddString_localization_id(hwnd,IDC_DARKMODE,LOCALIZATION_ID_DARK_MODE_DARK);
			// combo order: automatic, light, dark.
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_DARKMODE),config_dark_mode == 1 ? 2 : (config_dark_mode == 0 ? 1 : 0));

			if (config_appdata) 
			{
				CheckDlgButton(hwnd,IDC_APPDATA,BST_CHECKED);
			}
			
			if (config_multiple_instances) 
			{
				CheckDlgButton(hwnd,IDC_MULTIPLE_INSTANCES,BST_CHECKED);
			}
			
			for(exti=0;exti<_VIV_ASSOCIATION_COUNT;exti++)
			{
				if (_viv_is_association(_viv_association_extensions[exti])) 
				{
					CheckDlgButton(hwnd,_viv_association_dlg_item_id[exti],BST_CHECKED);
				}
			}
			
			if (_viv_is_start_menu_shortcuts()) 
			{
				CheckDlgButton(hwnd,IDC_STARTMENU,BST_CHECKED);
			}
			
			return FALSE;
		}
			
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDC_STARTMENU:
				case IDC_APPDATA:
					_viv_options_update_sheild(GetParent(hwnd));
					break;
					
				case IDC_CHECKALL:
				case IDC_CHECKNONE:
				{
					UINT check;
					
					check = LOWORD(wParam) == IDC_CHECKALL ? BST_CHECKED : BST_UNCHECKED;
					
					CheckDlgButton(hwnd,IDC_BMP,check);
					CheckDlgButton(hwnd,IDC_GIF,check);
					CheckDlgButton(hwnd,IDC_ICO,check);
					CheckDlgButton(hwnd,IDC_JPEG,check);
					CheckDlgButton(hwnd,IDC_JPG,check);
					CheckDlgButton(hwnd,IDC_PNG,check);
					CheckDlgButton(hwnd,IDC_TIF,check);
					CheckDlgButton(hwnd,IDC_TIFF,check);
					CheckDlgButton(hwnd,IDC_WEBP,check);
					CheckDlgButton(hwnd,IDC_EMF,check);
					CheckDlgButton(hwnd,IDC_WMF,check);
					
					break;
				}
			}
					
			break;

	}
	
	return FALSE;
}
static void _viv_options_key_list_sel_change(HWND hwnd,int previous_key_index)
{
	int index;
	_viv_key_list_t *keylist;
	int key_count;

	keylist = (_viv_key_list_t *)GetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA);
	
	index = ListBox_GetCurSel(GetDlgItem(hwnd,IDC_COMMANDS_LIST));
	
	ListBox_ResetContent(GetDlgItem(hwnd,IDC_KEYS_LIST));
	SetWindowRedraw(GetDlgItem(hwnd,IDC_KEYS_LIST),FALSE);
	
	key_count = 0;
	
	if (index != LB_ERR)
	{
		int command_index;
		config_key_t *key;
		
		command_index = ListBox_GetItemData(GetDlgItem(hwnd,IDC_COMMANDS_LIST),index);
		
		key = keylist->start[command_index];
		
		while(key)
		{
			wchar_t key_text[STRING_SIZE];
			int listbox_index;
			
			_viv_get_key_text(key_text,key->key);
			
			listbox_index = ListBox_AddString(GetDlgItem(hwnd,IDC_KEYS_LIST),key_text);
			
			if (listbox_index != LB_ERR)
			{
				ListBox_SetItemData(GetDlgItem(hwnd,IDC_KEYS_LIST),listbox_index,key->key);
			}

			key_count++;
			key = key->next;
		}
		
		EnableWindow(GetDlgItem(hwnd,IDC_ADD_KEY_BUTTON),TRUE);
		
		if (key_count)
		{
			EnableWindow(GetDlgItem(hwnd,IDC_EDIT_KEY_BUTTON),TRUE);
			EnableWindow(GetDlgItem(hwnd,IDC_REMOVE_KEY_BUTTON),TRUE);
			
			if ((previous_key_index == LB_ERR) || (previous_key_index > key_count - 1))
			{
				previous_key_index = key_count - 1;
			}
			
			ListBox_SetCurSel(GetDlgItem(hwnd,IDC_KEYS_LIST),previous_key_index);
		}
		else
		{
			EnableWindow(GetDlgItem(hwnd,IDC_EDIT_KEY_BUTTON),FALSE);
			EnableWindow(GetDlgItem(hwnd,IDC_REMOVE_KEY_BUTTON),FALSE);
		}
	}
	else
	{
		EnableWindow(GetDlgItem(hwnd,IDC_ADD_KEY_BUTTON),FALSE);
		EnableWindow(GetDlgItem(hwnd,IDC_EDIT_KEY_BUTTON),FALSE);
		EnableWindow(GetDlgItem(hwnd,IDC_REMOVE_KEY_BUTTON),FALSE);
	}

	SetWindowRedraw(GetDlgItem(hwnd,IDC_KEYS_LIST),TRUE);
}
static void _viv_options_remove_key(HWND hwnd)
{
	int index;
	_viv_key_list_t *keylist;

	keylist = (_viv_key_list_t *)GetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA);
	
	index = ListBox_GetCurSel(GetDlgItem(hwnd,IDC_COMMANDS_LIST));
	
	if (index != LB_ERR)
	{
		int key_index;
		
		key_index = ListBox_GetCurSel(GetDlgItem(hwnd,IDC_KEYS_LIST));
		
		if (key_index != LB_ERR)
		{
			int command_index;
			WORD keyflags;
			
			command_index = ListBox_GetItemData(GetDlgItem(hwnd,IDC_COMMANDS_LIST),index);
			keyflags = ListBox_GetItemData(GetDlgItem(hwnd,IDC_KEYS_LIST),key_index);
			
			_viv_key_remove(keylist,command_index,keyflags);
			
			_viv_options_key_list_sel_change(hwnd,key_index);
		}
	}
}
static INT_PTR CALLBACK _viv_edit_key_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
			
		{
			WNDPROC last_proc;
			wchar_t caption_wbuf[STRING_SIZE];
			
			string_printf(caption_wbuf,localization_get_string(lParam ? LOCALIZATION_ID_EDIT_KEYBOARD_SHORTCUT_CAPTION : LOCALIZATION_ID_ADD_KEYBOARD_SHORTCUT_CAPTION));
			SetWindowText(hwnd,caption_wbuf);

			os_SetDlgItemText_localization_id(hwnd,IDC_EDIT_KEYBOARD_SHORTCUT_KEY_STATIC,LOCALIZATION_ID_SHORTCUT_KEY);
			os_SetDlgItemText_localization_id(hwnd,IDC_EDIT_KEYBOARD_SHORTCUT_KEY_CURRENTLY_USED_BY_STATIC,LOCALIZATION_ID_SHORTCUT_KEY_CURRENTLY_USED_BY);
			os_SetDlgItemText_localization_id(hwnd,IDOK,LOCALIZATION_ID_OK_BUTTON);
			os_SetDlgItemText_localization_id(hwnd,IDCANCEL,LOCALIZATION_ID_CANCEL_BUTTON);

			os_center_dialog(hwnd);

			last_proc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwnd,IDC_EDIT_KEY_EDIT),GWLP_WNDPROC,(LONG_PTR)_viv_edit_key_edit_proc);
			SetWindowLongPtr(GetDlgItem(hwnd,IDC_EDIT_KEY_EDIT),GWLP_USERDATA,(LONG_PTR)last_proc);
			
			_viv_edit_key_set_key(GetDlgItem(hwnd,IDC_EDIT_KEY_EDIT),lParam);
			_viv_options_edit_key_changed(hwnd);
			
			return FALSE;
		}
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hwnd,GetWindowLongPtr(hwnd,GWLP_USERDATA));
					break;
					
				case IDCANCEL:
					EndDialog(hwnd,0);
					break;
			}

			break;
	}
	
	return FALSE;
}
static void _viv_options_edit_key(HWND hwnd,int key_index)
{
	int index;
	_viv_key_list_t *keylist;

	keylist = (_viv_key_list_t *)GetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA);
	
	index = ListBox_GetCurSel(GetDlgItem(hwnd,IDC_COMMANDS_LIST));
	
	if (index != LB_ERR)
	{
		WORD keyflags;
		
		if (key_index == LB_ERR)
		{
			keyflags = 0;
		}
		else
		{
			keyflags = ListBox_GetItemData(GetDlgItem(hwnd,IDC_KEYS_LIST),key_index);
		}
		
		keyflags = DialogBoxParam(os_hinstance,MAKEINTRESOURCE(IDD_EDIT_KEY),hwnd,_viv_edit_key_proc,keyflags);

		_viv_edit_key_remove_currently_used_by(keylist,keyflags);

		if (keyflags)
		{
			int command_index;

			command_index = ListBox_GetItemData(GetDlgItem(hwnd,IDC_COMMANDS_LIST),index);
			
			if (key_index == LB_ERR)
			{
				// new
				_viv_key_add(keylist,command_index,keyflags);
			}
			else
			{
				config_key_t *key;
				int keyi;
				
				// edit
				key = keylist->start[command_index];
				keyi = 0;
				
				while(key)
				{
					if (keyi == key_index)
					{
						key->key = keyflags;
						break;
					}
				
					keyi++;
					key = key->next;
				}
			}
			
			_viv_options_key_list_sel_change(hwnd,key_index);			
		}
	}
}
static INT_PTR CALLBACK _viv_options_controls_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
			
		{
			os_SetDlgItemText_localization_id(hwnd,IDC_LEFT_CLICK_ACTION_STATIC,LOCALIZATION_ID_LEFT_CLICK_ACTION_STATIC);
			os_ComboBox_AddString_localization_id(hwnd,IDC_LEFTCLICKACTION_COMBOBOX,LOCALIZATION_ID_OPTIONS_ACTION_SCROLL_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_LEFTCLICKACTION_COMBOBOX,LOCALIZATION_ID_OPTIONS_ACTION_PLAY_PAUSE_SLIDESHOW_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_LEFTCLICKACTION_COMBOBOX,LOCALIZATION_ID_OPTIONS_ACTION_PLAY_PAUSE_ANIMATION_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_LEFTCLICKACTION_COMBOBOX,LOCALIZATION_ID_OPTIONS_ACTION_ZOOM_IN_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_LEFTCLICKACTION_COMBOBOX,LOCALIZATION_ID_OPTIONS_ACTION_NEXT_IMAGE_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_LEFTCLICKACTION_COMBOBOX,LOCALIZATION_ID_OPTIONS_ACTION_ONE_TO_ONE_SCROLL_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_LEFTCLICKACTION_COMBOBOX,LOCALIZATION_ID_OPTIONS_ACTION_SCROLL_MOVE_WINDOW_COMBOBOXITEM);
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_LEFTCLICKACTION_COMBOBOX),config_left_click_action);
			
			os_SetDlgItemText_localization_id(hwnd,IDC_RIGHT_CLICK_ACTION_STATIC,LOCALIZATION_ID_RIGHT_CLICK_ACTION_STATIC);
			os_ComboBox_AddString_localization_id(hwnd,IDC_RIGHTCLICKACTION_COMBOBOX,LOCALIZATION_ID_OPTIONS_ACTION_CONTEXT_MENU_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_RIGHTCLICKACTION_COMBOBOX,LOCALIZATION_ID_OPTIONS_ACTION_ZOOM_OUT_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_RIGHTCLICKACTION_COMBOBOX,LOCALIZATION_ID_OPTIONS_ACTION_PREVIOUS_IMAGE_COMBOBOXITEM);
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_RIGHTCLICKACTION_COMBOBOX),config_right_click_action);
			
			os_SetDlgItemText_localization_id(hwnd,IDC_MOUSE_WHEEL_ACTION_STATIC,LOCALIZATION_ID_MOUSE_WHEEL_ACTION_STATIC);
			os_ComboBox_AddString_localization_id(hwnd,IDC_MOUSEWHEELACTION_COMBOBOX,LOCALIZATION_ID_OPTIONS_ACTION_ZOOM_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_MOUSEWHEELACTION_COMBOBOX,LOCALIZATION_ID_OPTIONS_ACTION_NEXT_PREV_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_MOUSEWHEELACTION_COMBOBOX,LOCALIZATION_ID_OPTIONS_ACTION_PREV_NEXT_COMBOBOXITEM);
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_MOUSEWHEELACTION_COMBOBOX),config_mouse_wheel_action);

			os_SetDlgItemText_localization_id(hwnd,IDC_COMMANDS_STATIC,LOCALIZATION_ID_COMMANDS_STATIC);
			os_SetDlgItemText_localization_id(hwnd,IDC_SETTINGS_FOR_SELECTED_COMMAND_STATIC,LOCALIZATION_ID_SETTINGS_FOR_SELECTED_COMMAND);
			os_SetDlgItemText_localization_id(hwnd,IDC_ADD_KEY_BUTTON,LOCALIZATION_ID_ADD_KEY_BUTTON);
			os_SetDlgItemText_localization_id(hwnd,IDC_EDIT_KEY_BUTTON,LOCALIZATION_ID_EDIT_KEY_BUTTON);
			os_SetDlgItemText_localization_id(hwnd,IDC_REMOVE_KEY_BUTTON,LOCALIZATION_ID_REMOVE_KEY_BUTTON);
			
			{
				int i;
				_viv_key_list_t *keylist;
				
				keylist = mem_alloc(sizeof(_viv_key_list_t));

				_viv_key_list_init(keylist);
				
				_viv_key_list_copy(keylist,_viv_key_list);
				
				SetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA,(LONG_PTR)keylist);
				
				for(i=0;i<_VIV_COMMAND_COUNT;i++)
				{
					if (!(_viv_commands[i].flags & MF_POPUP))
					{
						if (!(_viv_commands[i].flags & MF_SEPARATOR))
						{
							if (!(_viv_commands[i].flags & MF_DELETE))
							{
								wchar_t command_name_wbuf[STRING_SIZE];
								int index;

								_viv_get_command_name(command_name_wbuf,i);
								
								index = ListBox_AddString(GetDlgItem(hwnd,IDC_COMMANDS_LIST),command_name_wbuf);
								if (index != LB_ERR)
								{
									ListBox_SetItemData(GetDlgItem(hwnd,IDC_COMMANDS_LIST),index,i);
								}
							}
						}
					}
				}
			}
			
			ListBox_SetCurSel(GetDlgItem(hwnd,IDC_COMMANDS_LIST),0);
			_viv_options_key_list_sel_change(hwnd,0);
			
			return FALSE;
		}
		
		case WM_COMMAND:

			switch(LOWORD(wParam))
			{
				case IDC_COMMANDS_LIST:
					if (HIWORD(wParam) == LBN_SELCHANGE) 
					{	
						_viv_options_key_list_sel_change(hwnd,0);
					}
					break;
					
				case IDC_REMOVE_KEY_BUTTON:
					_viv_options_remove_key(hwnd);
					break;
					
				case IDC_EDIT_KEY_BUTTON:
					_viv_options_edit_key(hwnd,ListBox_GetCurSel(GetDlgItem(hwnd,IDC_KEYS_LIST)));
					break;
					
				case IDC_ADD_KEY_BUTTON:
					_viv_options_edit_key(hwnd,LB_ERR);
					break;
			}
			break;
		
		case WM_DESTROY:
		{
			_viv_key_list_t *keylist;
			
			keylist = (_viv_key_list_t *)GetWindowLongPtr(GetDlgItem(hwnd,IDC_COMMANDS_LIST),GWLP_USERDATA);
			
			_viv_key_clear_all(keylist);
			mem_free(keylist);
			
			break;
		}
	}
	
	return FALSE;
}
static INT_PTR CALLBACK _viv_options_view_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
			
			
			os_SetDlgItemText_localization_id(hwnd,IDC_SHRINK_BLIT_MODE_STATIC,LOCALIZATION_ID_SHRINK_BLIT_MODE_STATIC);
			os_ComboBox_AddString_localization_id(hwnd,IDC_SHRINK_BLIT_MODE_COMBOBOX,LOCALIZATION_ID_BLIT_MODE_NEAREST_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_SHRINK_BLIT_MODE_COMBOBOX,LOCALIZATION_ID_BLIT_MODE_LINEAR_COMBOBOXITEM);
			
			if (config_shrink_blit_mode == CONFIG_SHRINK_BLIT_MODE_HALFTONE)
			{
				ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_SHRINK_BLIT_MODE_COMBOBOX),1);
			}
			else
			{
				ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_SHRINK_BLIT_MODE_COMBOBOX),0);
			}

			os_SetDlgItemText_localization_id(hwnd,IDC_MAGNIFY_BLIT_MODE_STATIC,LOCALIZATION_ID_MAGNIFY_BLIT_MODE);
			os_ComboBox_AddString_localization_id(hwnd,IDC_MAGNIFY_BLIT_MODE_COMBOBOX,LOCALIZATION_ID_BLIT_MODE_NEAREST_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_MAGNIFY_BLIT_MODE_COMBOBOX,LOCALIZATION_ID_BLIT_MODE_LINEAR_COMBOBOXITEM);
			
			if (config_mag_filter == CONFIG_MAG_FILTER_HALFTONE)
			{
				ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_MAGNIFY_BLIT_MODE_COMBOBOX),1);
			}
			else
			{
				ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_MAGNIFY_BLIT_MODE_COMBOBOX),0);
			}

			os_SetDlgItemText_localization_id(hwnd,IDC_TITLE_BAR_FORMAT_STATIC,LOCALIZATION_ID_OPTIONS_TITLE_BAR_FORMAT_STATIC);
			os_ComboBox_AddString_localization_id(hwnd,IDC_TITLE_BAR_FORMAT_COMBOBOX,LOCALIZATION_ID_OPTIONS_TITLE_BAR_FORMAT_FULL_PATH_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_TITLE_BAR_FORMAT_COMBOBOX,LOCALIZATION_ID_OPTIONS_TITLE_BAR_FORMAT_FILENAME_ONLY_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_TITLE_BAR_FORMAT_COMBOBOX,LOCALIZATION_ID_OPTIONS_TITLE_BAR_FORMAT_NONE_COMBOBOXITEM);
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_TITLE_BAR_FORMAT_COMBOBOX),config_title_bar_format);
					
			CheckDlgButton(hwnd,IDC_AUTO_ZOOM,config_auto_zoom ? BST_CHECKED : BST_UNCHECKED);
			
			os_SetDlgItemText_localization_id(hwnd,IDC_AUTO_ZOOM,LOCALIZATION_ID_OPTIONS_VIEW_AUTO_SIZE_WINDOW_STATIC);
			os_ComboBox_AddString_localization_id(hwnd,IDC_AUTO_SIZE_WINDOW_COMBOBOX,LOCALIZATION_ID_OPTIONS_VIEW_AUTO_SIZE_WINDOW_50_PERCENT_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_AUTO_SIZE_WINDOW_COMBOBOX,LOCALIZATION_ID_OPTIONS_VIEW_AUTO_SIZE_WINDOW_100_PERCENT_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_AUTO_SIZE_WINDOW_COMBOBOX,LOCALIZATION_ID_OPTIONS_VIEW_AUTO_SIZE_WINDOW_200_PERCENT_COMBOBOXITEM);
			os_ComboBox_AddString_localization_id(hwnd,IDC_AUTO_SIZE_WINDOW_COMBOBOX,LOCALIZATION_ID_OPTIONS_VIEW_AUTO_SIZE_WINDOW_AUTO_FIT_COMBOBOXITEM);
			ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_AUTO_SIZE_WINDOW_COMBOBOX),config_auto_zoom_type);
			EnableWindow(GetDlgItem(hwnd,IDC_AUTO_SIZE_WINDOW_COMBOBOX),IsDlgButtonChecked(hwnd,IDC_AUTO_ZOOM) == BST_CHECKED);

			os_SetDlgItemText_localization_id(hwnd,IDC_LOOP_ANIMATIONS_ONCE_STATIC,LOCALIZATION_ID_PLAY_ANIMATIONS_ONCE_STATIC);
			CheckDlgButton(hwnd,IDC_LOOP_ANIMATIONS_ONCE_STATIC,config_loop_animations_once ? BST_CHECKED : BST_UNCHECKED);

			os_SetDlgItemText_localization_id(hwnd,IDC_PRELOAD_NEXT_IMAGE_STATIC,LOCALIZATION_ID_PRELOAD_NEXT_IMAGE_STATIC);
			CheckDlgButton(hwnd,IDC_PRELOAD_NEXT_IMAGE_STATIC,config_preload_next ? BST_CHECKED : BST_UNCHECKED);

			os_SetDlgItemText_localization_id(hwnd,IDC_CACHE_LAST_IMAGE_STATIC,LOCALIZATION_ID_CACHE_LAST_IMAGE_STATIC);
			CheckDlgButton(hwnd,IDC_CACHE_LAST_IMAGE_STATIC,config_cache_last ? BST_CHECKED : BST_UNCHECKED);
			
			{
				int static_wide;
				
				static_wide = 0;
				
				os_SetDlgItemText_localization_id(hwnd,IDC_WINDOWEDBACKGROUNDCOLOR_STATIC,LOCALIZATION_ID_WINDOWED_BACKGROUND_COLOR_STATIC);
				SetWindowLongPtr(GetDlgItem(hwnd,IDC_WINDOWEDBACKGROUNDCOLOR_BUTTON),GWLP_USERDATA,RGB(config_windowed_background_color_r,config_windowed_background_color_g,config_windowed_background_color_b));
				_viv_update_color_button_bitmap(GetDlgItem(hwnd,IDC_WINDOWEDBACKGROUNDCOLOR_BUTTON));
				static_wide = os_expand_static_wide(hwnd,IDC_WINDOWEDBACKGROUNDCOLOR_STATIC,static_wide);

				os_SetDlgItemText_localization_id(hwnd,IDC_FULLSCREENBACKGROUNDCOLOR_STATIC,LOCALIZATION_ID_FULLSCREEN_BACKGROUND_COLOR_STATIC);
				SetWindowLongPtr(GetDlgItem(hwnd,IDC_FULLSCREENBACKGROUNDCOLOR_BUTTON),GWLP_USERDATA,RGB(config_fullscreen_background_color_r,config_fullscreen_background_color_g,config_fullscreen_background_color_b));
				_viv_update_color_button_bitmap(GetDlgItem(hwnd,IDC_FULLSCREENBACKGROUNDCOLOR_BUTTON));
				static_wide = os_expand_static_wide(hwnd,IDC_FULLSCREENBACKGROUNDCOLOR_STATIC,static_wide);
				
				os_set_dialog_item_x_wide(hwnd,IDC_WINDOWEDBACKGROUNDCOLOR_STATIC,0,static_wide+6);
				os_set_dialog_item_x_wide(hwnd,IDC_WINDOWEDBACKGROUNDCOLOR_BUTTON,static_wide+6,75);
				os_set_dialog_item_x_wide(hwnd,IDC_FULLSCREENBACKGROUNDCOLOR_STATIC,0,static_wide+6);
				os_set_dialog_item_x_wide(hwnd,IDC_FULLSCREENBACKGROUNDCOLOR_BUTTON,static_wide+6,75);
			}
					
			return FALSE;
			
		case WM_DESTROY:	
			_viv_delete_color_button_bitmap(GetDlgItem(hwnd,IDC_WINDOWEDBACKGROUNDCOLOR_BUTTON));
			_viv_delete_color_button_bitmap(GetDlgItem(hwnd,IDC_FULLSCREENBACKGROUNDCOLOR_BUTTON));
			break;
		
		case WM_COMMAND:

			switch(LOWORD(wParam))
			{
				case IDC_AUTO_ZOOM:
				
					EnableWindow(GetDlgItem(hwnd,IDC_AUTO_SIZE_WINDOW_COMBOBOX),IsDlgButtonChecked(hwnd,IDC_AUTO_ZOOM) == BST_CHECKED);
				
					break; 
				
				case IDC_WINDOWEDBACKGROUNDCOLOR_BUTTON:
				case IDC_FULLSCREENBACKGROUNDCOLOR_BUTTON:
					
					{
						COLORREF color;
						
						color = GetWindowLongPtr(GetDlgItem(hwnd,LOWORD(wParam)),GWLP_USERDATA);
						
						if (os_choose_color(hwnd,&color))
						{
							SetWindowLongPtr(GetDlgItem(hwnd,LOWORD(wParam)),GWLP_USERDATA,color);
							_viv_update_color_button_bitmap(GetDlgItem(hwnd,LOWORD(wParam)));
						}
					}
					
					break;
			}
			
			break;
	}
	
	return FALSE;
}
static void _viv_options_treeview_changed(HWND hwnd)
{
	TV_ITEM tvi;

	// get the current index
	tvi.hItem = TreeView_GetSelection(GetDlgItem(hwnd,IDC_TREE1));
	if (tvi.hItem)
	{
		int i;

		tvi.mask = TVIF_PARAM;
	    
		TreeView_GetItem(GetDlgItem(hwnd,IDC_TREE1),&tvi);
		
		ShowWindow(GetDlgItem(hwnd,_viv_options_tab_ids[tvi.lParam]),SW_SHOW);
		ShowWindow(GetDlgItem(hwnd,_viv_options_page_ids[tvi.lParam]),SW_SHOW);
		
		for(i=0;i<_VIV_OPTIONS_PAGE_COUNT;i++)
		{
			if (i != tvi.lParam)
			{
				ShowWindow(GetDlgItem(hwnd,_viv_options_tab_ids[i]),SW_HIDE);
				ShowWindow(GetDlgItem(hwnd,_viv_options_page_ids[i]),SW_HIDE);
			}
		}

		config_options_last_page = tvi.lParam;
	}
}
static void _viv_options_update_sheild(HWND hwnd)
{
//	int exti;
	int need_admin;
	HWND general_page;
	
	general_page = GetDlgItem(hwnd,VIV_ID_OPTIONS_GENERAL);
	
	need_admin = 0;
	/*
	for(exti=0;exti<_VIV_ASSOCIATION_COUNT;exti++)
	{
		if (IsDlgButtonChecked(general_page,_viv_association_dlg_item_id[exti]) == BST_CHECKED) 
		{
			if (!_viv_is_association(_viv_association_extensions[exti]))
			{
				need_admin = 1;
			}
		}
		else 
		{
			if (_viv_is_association(_viv_association_extensions[exti]))
			{
				need_admin = 1;
			}
		}
	}
*/
	if ((!!config_appdata) != (IsDlgButtonChecked(general_page,IDC_APPDATA) == BST_CHECKED)) 
	{
		need_admin = 1;
	}
	
	if ((IsDlgButtonChecked(general_page,IDC_STARTMENU) == BST_CHECKED) != !!_viv_is_start_menu_shortcuts()) 
	{
		need_admin = 1;
	}
	
	if ((need_admin) && (!os_is_admin()))
	{
		SendMessage(GetDlgItem(hwnd,IDOK),BCM_SETSHIELD,0,TRUE);
	}
	else
	{
		SendMessage(GetDlgItem(hwnd,IDOK),BCM_SETSHIELD,0,FALSE);
	}
}
// the dark options tab: the comctl tab control never follows the dark
// explorer style (no dark variant on any build) and its own WM_PAINT
// covers the whole face - the strip behind the items, the body and the 3d
// edges - in the light style. the old custom draw only reached the items,
// so the strip and the edges stayed light (the white page-title header and
// the white sliver under every page in the field screenshots). the subclass
// takes WM_PAINT over completely in the dark ui: the body face, the strip
// one step above it, and the single page-title item drawn connected to the
// body. the page dialogs paint their own dark faces over the body.
static LRESULT CALLBACK _viv_options_tab_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	WNDPROC last_proc;
	
	if (_viv_is_dark())
	{
		if (msg == WM_ERASEBKGND)
		{
			RECT rect;
			
			GetClientRect(hwnd,&rect);
			
			FillRect((HDC)wParam,&rect,_viv_dark_chrome_brush(3));
			
			return 1;
		}
		
		if (msg == WM_PAINT)
		{
			PAINTSTRUCT ps;
			RECT rect;
			RECT item_rect;
			wchar_t text[STRING_SIZE];
			TCITEM tcitem;
			HFONT font;
			HFONT old_font;
			
			if (BeginPaint(hwnd,&ps))
			{
				GetClientRect(hwnd,&rect);
				
				// the body: the same face the page dialogs erase with.
				FillRect(ps.hdc,&rect,_viv_dark_chrome_brush(3));
				
				os_zero_memory(&tcitem,sizeof(tcitem));
				tcitem.mask = TCIF_TEXT;
				tcitem.pszText = text;
				tcitem.cchTextMax = STRING_SIZE;
				
				text[0] = 0;
				
				os_zero_memory(&item_rect,sizeof(item_rect));
				
				if ((TabCtrl_GetItem(hwnd,0,&tcitem)) && (text[0]) && (TabCtrl_GetItemRect(hwnd,0,&item_rect)))
				{
					RECT strip_rect;
					
					CopyRect(&strip_rect,&rect);
					
					// the strip band behind the item ends one pixel under it: a
					// hairline of the strip color separates the header from the
					// body.
					strip_rect.bottom = item_rect.bottom + 1;
					
					FillRect(ps.hdc,&strip_rect,_viv_dark_chrome_brush(0));
					
					// the single page-title item connects to the body face.
					FillRect(ps.hdc,&item_rect,_viv_dark_chrome_brush(3));
					
					SetBkMode(ps.hdc,TRANSPARENT);
					SetTextColor(ps.hdc,RGB(0xE8,0xE8,0xE8));
					
					font = _viv_menu_font();
					old_font = 0;
					
					if (font)
					{
						old_font = SelectObject(ps.hdc,font);
					}
					
					DrawTextW(ps.hdc,text,-1,&item_rect,DT_SINGLELINE | DT_CENTER | DT_VCENTER);
					
					if (old_font)
					{
						SelectObject(ps.hdc,old_font);
					}
				}
				
				EndPaint(hwnd,&ps);
			}
			
			return 0;
		}
	}
	
	last_proc = (WNDPROC)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	
	if (last_proc)
	{
		return CallWindowProc(last_proc,hwnd,msg,wParam,lParam);
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}
static INT_PTR CALLBACK _viv_options_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
		case WM_TIMER:
		
			// the creation race self heal: re-run the full dialog dark pass (the
			// caption, the control classes and the tree colors) with the settled
			// state, whatever branch the creation moment took.
			if (wParam == VIV_ID_DARK_DIALOG_ASSERT_TIMER)
			{
				KillTimer(hwnd,VIV_ID_DARK_DIALOG_ASSERT_TIMER);
				
				_viv_dark_dialog(hwnd);
				
				InvalidateRect(hwnd,0,TRUE);
			}
			
			break;
		
		case WM_NOTIFY:

			switch(((NMHDR *)lParam)->idFrom)
			{
				case IDC_TREE1:

					switch(((NMHDR *)lParam)->code)
					{
						case TVN_SELCHANGEDA:
						case TVN_SELCHANGEDW:
						
							if (IsWindowVisible(hwnd))
							{
								_viv_options_treeview_changed(hwnd);
							}
							
							break;
					}
					
					break;
			}
			
			return 0;
			
		case WM_INITDIALOG:
			// dark chrome: title bar, dark explorer control style and the
			// options navigation (tree + tabs).
			_viv_dark_dialog(hwnd);
			
			if (_viv_is_dark())
			{
				HWND tree_hwnd;
				int tabi;
				
				tree_hwnd = GetDlgItem(hwnd,IDC_TREE1);
				
				if (tree_hwnd)
				{
					os_dark_window_theme(tree_hwnd);
					
					SendMessage(tree_hwnd,TVM_SETBKCOLOR,0,RGB(0x20,0x20,0x20));
					SendMessage(tree_hwnd,TVM_SETTEXTCOLOR,0,RGB(0xE8,0xE8,0xE8));
				}
				
				for(tabi=0;tabi<(int)_VIV_OPTIONS_PAGE_COUNT;tabi++)
				{
					HWND tab_hwnd;
					WNDPROC last_proc;
					
					tab_hwnd = GetDlgItem(hwnd,_viv_options_tab_ids[tabi]);
					
					os_dark_window_theme(tab_hwnd);
					
					// the tab body never follows the dark style: subclass the tab so
					// the dark body face paints (the items paint in the custom draw
					// pass in the dialog proc).
					last_proc = (WNDPROC)SetWindowLongPtr(tab_hwnd,GWLP_WNDPROC,(LONG_PTR)_viv_options_tab_proc);
					
					SetWindowLongPtr(tab_hwnd,GWLP_USERDATA,(LONG_PTR)last_proc);
				}
			}
			

			// the dark init above read the live state: a theme race at the
			// creation moment (the registry unsettled, the flip broadcast in
			// flight) takes the light branch and the caption keeps the system
			// light frame while the per paint cticolor replies already darken
			// the body - the mixed dialog the field caught. the one shot assert
			// re-runs the full dark pass after the race window closes.
			SetTimer(hwnd,VIV_ID_DARK_DIALOG_ASSERT_TIMER,300,0);
			
			// update text.
			os_SetWindowText_localization_id(hwnd,LOCALIZATION_ID_OPTIONS_CAPTION);
			os_SetDlgItemText_localization_id(hwnd,IDOK,LOCALIZATION_ID_OK_BUTTON);
			os_SetDlgItemText_localization_id(hwnd,IDCANCEL,LOCALIZATION_ID_CANCEL_BUTTON);
			
			os_center_dialog(hwnd);

			{
				int i;
				
				for(i=0;i<_VIV_OPTIONS_PAGE_COUNT;i++)
				{
					TV_INSERTSTRUCT tvitem;
					TCITEM tcitem;
					RECT rect;
					HWND page_hwnd;
					HTREEITEM hitem;
					wchar_t text_wbuf[STRING_SIZE];
					
					string_copy_utf8_string(text_wbuf,localization_get_string(_viv_options_page_localization_id_array[i]));
					
					// treeview
					tvitem.hInsertAfter = TVI_LAST;
					tvitem.hParent = TVI_ROOT;
					tvitem.item.mask = TVIF_TEXT | TVIF_PARAM;
					tvitem.item.lParam = i;
					tvitem.item.pszText = text_wbuf;
					
					hitem = TreeView_InsertItem(GetDlgItem(hwnd,IDC_TREE1),&tvitem);

					if (i == config_options_last_page) 
					{
						TreeView_Select(GetDlgItem(hwnd,IDC_TREE1),hitem,TVGN_CARET);
					}
					
					// tab
					string_copy_utf8_string(text_wbuf,localization_get_string(_viv_options_page_localization_id_array[i]));
					
					tcitem.mask = TCIF_TEXT;
					tcitem.pszText = text_wbuf;
					TabCtrl_InsertItem(GetDlgItem(hwnd,_viv_options_tab_ids[i]),0,&tcitem);

					// page
					GetClientRect(GetDlgItem(hwnd,IDC_PAGEPLACEHOLDER),&rect);
					MapWindowPoints(GetDlgItem(hwnd,IDC_PAGEPLACEHOLDER),hwnd,(LPPOINT)&rect,2);

					page_hwnd = CreateDialog(os_hinstance,MAKEINTRESOURCE(_viv_options_dialog_ids[i]),hwnd,_viv_options_page_procs[i]);
					SetWindowLong(page_hwnd,GWL_ID,_viv_options_page_ids[i]);
					
					if (os_EnableThemeDialogTexture)
					{
						// the light tab texture would clash with the dark chrome.
						if (!_viv_is_dark())
						{
							os_EnableThemeDialogTexture(page_hwnd,ETDT_ENABLETAB);
						}
					}

					SetWindowPos(page_hwnd,HWND_TOP,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOSIZE|SWP_NOACTIVATE);
				}
			}

			_viv_options_treeview_changed(hwnd);
			_viv_options_update_sheild(hwnd);

			return TRUE;
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
				
					if (LOWORD(wParam) == IDOK)
					{
						HWND general_page;
						HWND view_page;
						HWND controls_page;
						int old_shrink_blit_mode;
						int exti;
						COLORREF colorref;
						wchar_t params[STRING_SIZE];
						int language_changed;
						
						params[0] = 0;
						language_changed = 0;
						
						general_page = GetDlgItem(hwnd,VIV_ID_OPTIONS_GENERAL);
						view_page = GetDlgItem(hwnd,VIV_ID_OPTIONS_VIEW);
						controls_page = GetDlgItem(hwnd,VIV_ID_OPTIONS_CONTROLS);
						config_multiple_instances = 0;
						
						if (IsDlgButtonChecked(general_page,IDC_APPDATA) == BST_CHECKED) 
						{
							if (!config_appdata)
							{
								_viv_append_admin_param(params,(const utf8_t *)"appdata");
								config_appdata = 1;
							}
						}
						else
						{
							if (config_appdata)
							{
								_viv_append_admin_param(params,(const utf8_t *)"noappdata");

								config_appdata = 0;
							}
						}

						if (IsDlgButtonChecked(general_page,IDC_MULTIPLE_INSTANCES) == BST_CHECKED) 
						{
							config_multiple_instances = 1;
						}
						
						// language.
						{
							int language;
							
							language = ComboBox_GetCurSel(GetDlgItem(general_page,IDC_LANGUAGE));
							
							if (language != config_language)
							{
								config_language = language;
								language_changed = 1;
								
								if (language == 1)
								{
									localization_set_language(LOCALIZATION_LANGUAGE_ENGLISH);
								}
								else
								if (language == 2)
								{
									localization_set_language(LOCALIZATION_LANGUAGE_CHINESE_SIMPLIFIED);
								}
								else
								{
									// auto: follow the system language again.
									localization_init();
								}
							}
						}
						// dark mode.
						{
							int dark_mode;
							
							dark_mode = ComboBox_GetCurSel(GetDlgItem(general_page,IDC_DARKMODE));
							
							if (dark_mode < 0)
							{
								dark_mode = 0;
							}
							
							// combo order: automatic, light, dark.
							if (dark_mode == 1)
							{
								config_dark_mode = 0;
							}
							else
							if (dark_mode == 2)
							{
								config_dark_mode = 1;
							}
							else
							{
								config_dark_mode = 2;
							}
							
							os_dark_set_app_mode(config_dark_mode);
							_viv_apply_dark_mode(1);
							
							// the app mode switch flushes the menu themes and the color
							// policy asynchronously: the system sweep can land after this
							// apply and repaint the chrome in light (the white band the
							// field caught right after the switch). the one shot assert
							// re-applies once the sweep has settled.
							SetTimer(_viv_hwnd,VIV_ID_DARK_RECHECK_TIMER,400,0);
						}
						
						if (IsDlgButtonChecked(general_page,IDC_STARTMENU) == BST_CHECKED) 
						{
							if (!_viv_is_start_menu_shortcuts())
							{
								_viv_append_admin_param(params,(const utf8_t *)"startmenu");
							}
						}
						else
						{
							if (_viv_is_start_menu_shortcuts())
							{
								_viv_append_admin_param(params,(const utf8_t *)"nostartmenu");
							}
						}
						
						
						for(exti=0;exti<_VIV_ASSOCIATION_COUNT;exti++)
						{
							if (IsDlgButtonChecked(general_page,_viv_association_dlg_item_id[exti]) == BST_CHECKED) 
							{
								if (!_viv_is_association(_viv_association_extensions[exti]))
								{
									_viv_install_association_by_extension(_viv_association_extensions[exti],localization_get_string(_viv_association_description_localization_id_array[exti]),_viv_association_icon_locations[exti]);
								}
							}
							else 
							{
								if (_viv_is_association(_viv_association_extensions[exti]))
								{
									_viv_uninstall_association_by_extension(_viv_association_extensions[exti]);
								}
							}
						}
						
						old_shrink_blit_mode = config_shrink_blit_mode;
						
						config_left_click_action = ComboBox_GetCurSel(GetDlgItem(controls_page,IDC_LEFTCLICKACTION_COMBOBOX));
						config_right_click_action = ComboBox_GetCurSel(GetDlgItem(controls_page,IDC_RIGHTCLICKACTION_COMBOBOX));
						config_mouse_wheel_action = ComboBox_GetCurSel(GetDlgItem(controls_page,IDC_MOUSEWHEELACTION_COMBOBOX));
						
						if (ComboBox_GetCurSel(GetDlgItem(view_page,IDC_SHRINK_BLIT_MODE_COMBOBOX)) == 1)
						{
							config_shrink_blit_mode = CONFIG_SHRINK_BLIT_MODE_HALFTONE;
						}
						else
						{
							config_shrink_blit_mode = CONFIG_SHRINK_BLIT_MODE_COLORONCOLOR;
						}
						
						colorref = (COLORREF)GetWindowLongPtr(GetDlgItem(view_page,IDC_WINDOWEDBACKGROUNDCOLOR_BUTTON),GWLP_USERDATA);

						if ((GetRValue(colorref) != config_windowed_background_color_r) || (GetGValue(colorref) != config_windowed_background_color_g) || (GetBValue(colorref) != config_windowed_background_color_b))
						{
							config_windowed_background_color_r = GetRValue(colorref);
							config_windowed_background_color_g = GetGValue(colorref);
							config_windowed_background_color_b = GetBValue(colorref);

							// the mat, the win11 caption tint and any follow mode
							// backdrop all read this color: re-tint the frame, repaint
							// the canvas and reload the image so transparency under
							// an open file picks the new mat up immediately.
							os_window_modern_chrome(_viv_hwnd,_viv_windowed_background());
							
							InvalidateRect(_viv_hwnd,0,FALSE);
							_viv_refresh();
						}
						
						colorref = (COLORREF)GetWindowLongPtr(GetDlgItem(view_page,IDC_FULLSCREENBACKGROUNDCOLOR_BUTTON),GWLP_USERDATA);
						
						if ((GetRValue(colorref) != config_fullscreen_background_color_r) || (GetGValue(colorref) != config_fullscreen_background_color_g) || (GetBValue(colorref) != config_fullscreen_background_color_b))
						{
							config_fullscreen_background_color_r = GetRValue(colorref);
							config_fullscreen_background_color_g = GetGValue(colorref);
							config_fullscreen_background_color_b = GetBValue(colorref);

							InvalidateRect(_viv_hwnd,0,FALSE);
						}
						
						if (old_shrink_blit_mode != config_shrink_blit_mode)
						{
							InvalidateRect(_viv_hwnd,0,FALSE);
						}
						
						old_shrink_blit_mode = config_mag_filter;
						
						if (ComboBox_GetCurSel(GetDlgItem(view_page,IDC_MAGNIFY_BLIT_MODE_COMBOBOX)) == 1)
						{
							config_mag_filter = CONFIG_SHRINK_BLIT_MODE_HALFTONE;
						}
						else
						{
							config_mag_filter = CONFIG_SHRINK_BLIT_MODE_COLORONCOLOR;
						}
						
						if (old_shrink_blit_mode != config_mag_filter)
						{
							InvalidateRect(_viv_hwnd,0,FALSE);
						}

					config_title_bar_format = ComboBox_GetCurSel(GetDlgItem(view_page,IDC_TITLE_BAR_FORMAT_COMBOBOX));
					_viv_update_title();

					config_auto_zoom = IsDlgButtonChecked(view_page,IDC_AUTO_ZOOM) == BST_CHECKED ? 1 : 0;
					config_auto_zoom_type = ComboBox_GetCurSel(GetDlgItem(view_page,IDC_AUTO_SIZE_WINDOW_COMBOBOX));
					config_loop_animations_once = IsDlgButtonChecked(view_page,IDC_LOOP_ANIMATIONS_ONCE_STATIC) == BST_CHECKED ? 1 : 0;
					config_preload_next = IsDlgButtonChecked(view_page,IDC_PRELOAD_NEXT_IMAGE_STATIC) == BST_CHECKED ? 1 : 0;
					config_cache_last = IsDlgButtonChecked(view_page,IDC_CACHE_LAST_IMAGE_STATIC) == BST_CHECKED ? 1 : 0;
					
					// copy keys.
					_viv_key_list_copy(_viv_key_list,(_viv_key_list_t *)GetWindowLongPtr(GetDlgItem(controls_page,IDC_COMMANDS_LIST),GWLP_USERDATA));
					
					// reinit menu.

					{
						HMENU new_hmenu;
						
						new_hmenu = _viv_create_menu();
						
						if (_viv_hmenu)
						{
							DestroyMenu(_viv_hmenu);
						}
						
						_viv_hmenu = new_hmenu;
						
						// the fresh menu: the top bar re-reads the labels and
						// re-lays them out at the current font.
						_viv_menubar_layout();
					}
					
					// refresh the visible controls when the language has changed.
					if (language_changed)
					{
						// recreate the toolbar so its texts and tooltips use the new language.
						_viv_controls_show(0);
						_viv_controls_show(config_show_controls);
						
						// the recreated toolbar has a fresh (light) tooltip control:
						// re-apply the dark chrome to it.
						_viv_apply_dark_mode(0);
						
						// update the floating zoom control tooltips.
						zoomui_localize();
						
						// relayout and redraw. (the title bar and status bar update here too)
						_viv_on_size();
						InvalidateRect(_viv_hwnd,0,FALSE);
					}
					
					// do admin commands.
					if (*params)
					{	
						wchar_t exe_filename[STRING_SIZE];
						
						_viv_get_exe_filename(exe_filename);
						
						os_shell_execute(0,exe_filename,1,NULL,params);
					}

					// save settings to disk						
					config_save_settings(config_appdata);
				}
			
				EndDialog(hwnd,0);
				break;
			}

			break;
	}
	
	return FALSE;
}
void _viv_options(void)
{
	// the remake settings window is the options surface now: the classic
	// tabbed dialog stays retired (the template ships but nothing opens it).
	_viv_settings_show();
}
INT_PTR CALLBACK _viv_custom_rate_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
			

			{
				int static_wide;
				RECT rect;
				int wide;
				
				os_center_dialog(hwnd);
				GetClientRect(hwnd,&rect);
				wide = rect.right - rect.left - 12 - 12;

				os_SetWindowText_localization_id(hwnd,LOCALIZATION_ID_SET_CUSTOM_RATE_CAPTION);

				os_SetDlgItemText_localization_id(hwnd,IDOK,LOCALIZATION_ID_OK_BUTTON);
				os_SetDlgItemText_localization_id(hwnd,IDCANCEL,LOCALIZATION_ID_CANCEL_BUTTON);

				SetDlgItemInt(hwnd,IDC_CUSTOM_RATE_EDIT,config_slideshow_custom_rate,FALSE);

				os_SetDlgItemText_localization_id(hwnd,IDC_CUSTOM_RATE_STATIC,LOCALIZATION_ID_CUSTOM_RATE_STATIC);
				static_wide = os_get_static_wide(hwnd,IDC_CUSTOM_RATE_STATIC);
				os_set_dialog_item_x_wide(hwnd,IDC_CUSTOM_RATE_STATIC,12,static_wide+6);
	
				{
					int edit_combo_wide;
					
					edit_combo_wide = (wide - (static_wide+6) - 6) / 2;
					
					os_set_dialog_item_x_wide(hwnd,IDC_CUSTOM_RATE_EDIT,12 + static_wide+6,edit_combo_wide);
					os_set_dialog_item_x_wide(hwnd,IDC_CUSTOM_RATE_TYPE_COMBOBOX,12 + static_wide+6 + edit_combo_wide + 6,edit_combo_wide);
				}
				
				os_ComboBox_AddString_localization_id(hwnd,IDC_CUSTOM_RATE_TYPE_COMBOBOX,LOCALIZATION_ID_CUSTOM_RATE_MILLISECONDS);
				os_ComboBox_AddString_localization_id(hwnd,IDC_CUSTOM_RATE_TYPE_COMBOBOX,LOCALIZATION_ID_CUSTOM_RATE_SECONDS);
				os_ComboBox_AddString_localization_id(hwnd,IDC_CUSTOM_RATE_TYPE_COMBOBOX,LOCALIZATION_ID_CUSTOM_RATE_MINUTES);
				
				ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_CUSTOM_RATE_TYPE_COMBOBOX),config_slideshow_custom_rate_type);
			}

			return TRUE;
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
					config_slideshow_custom_rate_type = ComboBox_GetCurSel(GetDlgItem(hwnd,IDC_CUSTOM_RATE_TYPE_COMBOBOX));
					config_slideshow_custom_rate = GetDlgItemInt(hwnd,IDC_CUSTOM_RATE_EDIT,NULL,FALSE);
					EndDialog(hwnd,1);
					break;

				case IDCANCEL:
					EndDialog(hwnd,0);
					break;
			}

			break;
	}
	
	return FALSE;
}
// the about dialog colors. the banner (aboutback + abouttitle) keeps its
// 1.1.11 face: the black band with the white title in the light ui, the
// shared dark canvas face in the dark. the band chrome (the two separator
// lines and the button strip) answers with the same palettes the wm_paint
// passes painted before the template round moved the geometry into the
// resource template. the rest of the controls answer the light face the
// 1.1.11 ctlcolor case painted and fall through to the shared dark handler
// in the dark ui (the caller runs this before the shared proc so the band
// controls never fall into its flat 0x20 reply).
static INT_PTR _viv_about_colors(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	if ((msg == WM_CTLCOLORSTATIC) || (msg == WM_CTLCOLOREDIT))
	{
		HDC hdc;
		HWND control;
		
		hdc = (HDC)wParam;
		control = (HWND)lParam;
		
		if ((control == GetDlgItem(hwnd,IDC_ABOUTBACK)) || (control == GetDlgItem(hwnd,IDC_ABOUTTITLE)))
		{
			if (_viv_is_dark())
			{
				SetTextColor(hdc,RGB(0xE8,0xE8,0xE8));
				SetBkColor(hdc,RGB(0x20,0x20,0x20));
				
				return (INT_PTR)_viv_dialog_dark_brush();
			}
			
			SetTextColor(hdc,RGB(255,255,255));
			SetBkColor(hdc,RGB(0,0,0));
			
			return (LRESULT)GetStockObject(BLACK_BRUSH);
		}
		
		if (control == GetDlgItem(hwnd,IDC_ABOUTLINE1))
		{
			if (_viv_is_dark())
			{
				SetBkColor(hdc,RGB(0x45,0x45,0x45));
				
				return (INT_PTR)_viv_dark_chrome_brush(1);
			}
			
			SetBkColor(hdc,RGB(0xEC,0xEC,0xEC));
			
			return (INT_PTR)_viv_about_light_brush(0);
		}
		
		if (control == GetDlgItem(hwnd,IDC_ABOUTLINE2))
		{
			if (_viv_is_dark())
			{
				SetBkColor(hdc,RGB(0x70,0x70,0x70));
				
				return (INT_PTR)_viv_dark_chrome_brush(2);
			}
			
			SetBkColor(hdc,RGB(0xEC,0xEC,0xEC));
			
			return (INT_PTR)_viv_about_light_brush(0);
		}
		
		if (control == GetDlgItem(hwnd,IDC_ABOUTBAND))
		{
			if (_viv_is_dark())
			{
				SetBkColor(hdc,RGB(0x25,0x25,0x25));
				
				return (INT_PTR)_viv_dark_chrome_brush(0);
			}
			
			SetBkColor(hdc,RGB(0xFF,0xFF,0xFF));
			
			return (INT_PTR)_viv_about_light_brush(1);
		}
		
		// the rest of the controls keep the 1.1.11 light face; the dark ui
		// answers them through the shared dark handler - this reply only
		// covers the light theme.
		if (!_viv_is_dark())
		{
			SetTextColor(hdc,RGB(0,0,0));
			SetBkColor(hdc,RGB(255,255,255));
			
			return (LRESULT)GetStockObject(WHITE_BRUSH);
		}
	}
	
	if (msg == WM_CTLCOLORDLG)
	{
		// the dialog face: the wm_paint pass used to paint it after the
		// default erase; the ctldlg reply now hands the dialog manager the
		// same brush for its own erase (the template round owns the
		// geometry, the palette travels the official pipeline).
		if (_viv_is_dark())
		{
			return (INT_PTR)_viv_dialog_dark_brush();
		}
		
		return (LRESULT)GetStockObject(WHITE_BRUSH);
	}
	
	return -1;
}
INT_PTR CALLBACK _viv_about_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	{
		INT_PTR own_reply;
		
		// the banner and the band controls answer here (the shared dark
		// handler below flattens every static to the 0x20 face and the
		// band needs the line and strip colors); the dialog face answers
		// here because the shared handler carries no ctldlg case. any
		// other message falls through to the shared pair, then to the
		// switch below.
		own_reply = _viv_about_colors(hwnd,msg,wParam,lParam);
		
		if (own_reply != -1)
		{
			return own_reply;
		}
	}
	
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
			
		{
			HFONT hfont;
			HFONT old_hfont;
			LOGFONTW lf;
			wchar_t version_wbuf[STRING_SIZE];

			os_center_dialog(hwnd);

			string_copy_utf8_string(version_wbuf,localization_get_string(LOCALIZATION_ID_ABOUT_CAPTION));
			SetWindowText(hwnd,version_wbuf);
			os_SetDlgItemText_localization_id(hwnd,IDC_ABOUTTITLE,LOCALIZATION_ID_APP_NAME);
			os_SetDlgItemText_localization_id(hwnd,IDC_ABOUTVOIDIMAGEVIEWER,LOCALIZATION_ID_APP_NAME);
			string_printf(version_wbuf,"%s%s %s",VERSION_STRING,VERSION_TYPE,VERSION_TARGET_MACHINE);
			SetDlgItemText(hwnd,IDC_ABOUTVERSION,version_wbuf);
			string_printf(version_wbuf,localization_get_string(LOCALIZATION_ID_ABOUT_COPYRIGHT_FORMAT),VERSION_YEAR);
			SetDlgItemText(hwnd,IDC_ABOUTCOPYRIGHT,version_wbuf);
			os_SetDlgItemText_localization_id(hwnd,IDC_ABOUTEMAIL,LOCALIZATION_ID_ABOUT_EMAIL);
			os_SetDlgItemText_localization_id(hwnd,IDC_ABOUTWEBSITE,LOCALIZATION_ID_ABOUT_WEBSITE);
			os_SetDlgItemText_localization_id(hwnd,IDOK,LOCALIZATION_ID_OK_BUTTON);
			os_SetDlgItemText_localization_id(hwnd,IDCANCEL,LOCALIZATION_ID_CANCEL_BUTTON);
			
			// the title face derives from the live dialog font end to
			// end: the message font the shared dialog proc applied at this
			// dialog window own dpi (wm_getfont reports it here: the apply
			// ran before this case), through the wide pipeline so a face
			// name survives whole (the unsuffixed forms borrowed the wide
			// path from the project unicode define), with the height
			// scaled by 8/3 - the ratio the literal 32 encoded at the 96
			// dpi design point (the message font is -12 there and
			// -12 * 8 / 3 = -32) - so the family, the weight and the dpi
			// scale all travel with the font the dialog actually draws.
			hfont = (HFONT)SendMessage(GetDlgItem(hwnd,IDC_ABOUTTITLE),WM_GETFONT,0,0);
			
			if ((hfont) && (GetObjectW(hfont,sizeof(LOGFONTW),&lf)))
			{
				lf.lfHeight = (lf.lfHeight * 8) / 3;
				
				old_hfont = _viv_about_hfont;
				
				_viv_about_hfont = CreateFontIndirectW(&lf);
				
				if (_viv_about_hfont)
				{
					// the face is rebuilt at every open: the old handle dies
					// inside this same message after the control took the new
					// face (a paint can never interleave the synchronous
					// wm_setfont), and a creation failure keeps the previous
					// face drawing instead of a null font.
					if (old_hfont)
					{
						DeleteObject(old_hfont);
					}
				}
				else
				{
					_viv_about_hfont = old_hfont;
				}
			}
			
			if (_viv_about_hfont)
			{
				SendMessage(GetDlgItem(hwnd,IDC_ABOUTTITLE),WM_SETFONT,(WPARAM)_viv_about_hfont,0);
			}
			
			return TRUE;
		}
		
		case WM_DESTROY:
			break;
		
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					EndDialog(hwnd,0);
					break;
			}

			break;
	}
	
	return FALSE;
}
static HWND _viv_zoom_edit_hwnd = 0; // the open in place zoom editor, or 0.
void _viv_set_zoom_dialog(void)
{
	// the status bar zoom pane click target: type an exact percent in place,
	// right on the pane. (the old centered dialog box carried a sunken edit,
	// native buttons and a select-all blue block that never met the remade
	// chrome, and the click itself had to survive the pane's drag subclass
	// first - a down eaten by the move loop, the box left to luck.)
	RECT pane_rect;
	wchar_t wbuf[STRING_SIZE];
	HFONT hfont;
	HWND hwnd;
	int wide;
	
	if (!_viv_image_wide)
	{
		return;
	}
	
	if (!_viv_status_hwnd)
	{
		return;
	}
	
	// one editor at a time: a click while one is open commits it, then
	// re-opens on the current percent.
	if (_viv_zoom_edit_hwnd)
	{
		_viv_zoom_edit_end(_viv_zoom_edit_hwnd,1);
	}
	
	// pane 0's rect, in status bar client coordinates.
	if (!SendMessage(_viv_status_hwnd,SB_GETRECT,0,(LPARAM)&pane_rect))
	{
		return;
	}
	
	// the field: the pane width minus breathing room, floor 32 (four digits).
	wide = (pane_rect.right - pane_rect.left) - 8;
	
	if (wide < 32)
	{
		wide = 32;
	}
	
	hwnd = os_CreateWindowEx(
		0,
		"EDIT",
		"",
		WS_CHILD|WS_VISIBLE|ES_NUMBER|ES_AUTOHSCROLL,
		pane_rect.left + 4,pane_rect.top + 1,wide,(pane_rect.bottom - pane_rect.top) - 2,
		_viv_status_hwnd,0,os_hinstance,NULL);
	
	if (!hwnd)
	{
		return;
	}
	
	_viv_zoom_edit_hwnd = hwnd;
	
	// the subclass stores the old proc in the userdata (the edit key
	// dialog's own editor idiom).
	{
		WNDPROC old_proc;
		
		old_proc = (WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)_viv_zoom_edit_proc);
		
		SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)old_proc);
	}
	
	// the pane's own font, so the digits match the strip text.
	hfont = (HFONT)SendMessage(_viv_status_hwnd,WM_GETFONT,0,0);
	
	if (hfont)
	{
		SendMessage(hwnd,WM_SETFONT,(WPARAM)hfont,0);
	}
	
	// the current percent as plain digits, selected: typing replaces.
	string_printf(wbuf,"%d",_viv_zoom_percent());
	SetWindowTextW(hwnd,wbuf);
	
	SendMessage(hwnd,EM_SETSEL,0,-1);
	
	SetFocus(hwnd);
}
static void _viv_zoom_edit_end(HWND hwnd,int apply)
{
	wchar_t wbuf[STRING_SIZE];
	RECT rect;
	POINT pt;
	int percent;
	
	// the guard stops the re-entries: the kill focus that the destroy
	// itself raises, and the status bar teardown cascading into a child
	// editor.
	if (hwnd != _viv_zoom_edit_hwnd)
	{
		return;
	}
	
	_viv_zoom_edit_hwnd = 0;
	
	if (apply)
	{
		GetWindowTextW(hwnd,wbuf,STRING_SIZE);
		
		percent = string_to_int(wbuf);
		
		// the ladder covers 1% .. 1600%: clamp anything above, ignore
		// empty or zero input (the old dialog's contract).
		if (percent > 1600)
		{
			percent = 1600;
		}
		
		if (percent >= 1)
		{
			GetClientRect(_viv_hwnd,&rect);
			pt.x = (rect.right - rect.left) / 2;
			pt.y = (_viv_get_view_top() + (rect.bottom - rect.top - _viv_get_status_high())) / 2;
			
			ClientToScreen(_viv_hwnd,&pt);
			
			_viv_zoom_set_percent(percent,pt.x,pt.y,0);
		}
	}
	
	// the keyboard goes home to the main window only while the editor
	// still owns it: a kill focus end means the focus already has a new
	// home (inside or outside the app) - stealing it back would make the
	// app a focus thief. (the kill focus this raises re-enters the guard
	// above.)
	if (GetFocus() == hwnd)
	{
		SetFocus(_viv_hwnd);
	}
	
	DestroyWindow(hwnd);
}
static LRESULT CALLBACK _viv_zoom_edit_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	WNDPROC old_proc;
	
	old_proc = (WNDPROC)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	
	switch(msg)
	{
		case WM_KEYDOWN:
		
			// enter commits, escape cancels: one short lived field, no
			// dialog machinery above it.
			if (wParam == VK_RETURN)
			{
				_viv_zoom_edit_end(hwnd,1);
				
				return 0;
			}
			
			if (wParam == VK_ESCAPE)
			{
				_viv_zoom_edit_end(hwnd,0);
				
				return 0;
			}
			
			break;
		
		case WM_KILLFOCUS:
		
			// focus went elsewhere: commit (the field never keeps a half
			// typed value).
			_viv_zoom_edit_end(hwnd,1);
			
			return 0;
		
		case WM_DESTROY:
		
			_viv_zoom_edit_hwnd = 0;
			
			break;
	}
	
	return CallWindowProc(old_proc,hwnd,msg,wParam,lParam);
}

void _viv_command_line_options(void)
{
	wchar_t *text_wbuf;
	wchar_t caption_wbuf[STRING_SIZE];

	text_wbuf = string_alloc_utf8("Usage:\nvoidImageViewer.exe [/switches] [filename(s)]\n"
		"\n"
		"Switches:\n"
		"/slideshow\tStart a slideshow.\n"
		"/fullscreen\tStart fullscreen.\n"
		"/maximized\tStart maximized.\n"
		"/window\t\tStart windowed.\n"
		"/ontop\t\tShow on top of other windows.\n"
		"/minimal\t\tBorderless window.\n"
		"/compact\t\tBordered window.\n"
		"/x <x> /y <y> /width <width> /height <height>\n"
		"\t\tSet the Window position and size.\n"
		"/rate <rate>\tSet the slideshow rate in milliseconds.\n"
		"/name\t\tSort by name.\n"
		"/path\t\tSort by full path.\n"
		"/size\t\tSort by size.\n"
		"/dm\t\tSort by date modified.\n"
		"/dc\t\tSort by date created.\n"
		"/ascending\tSort in ascending order.\n"
		"/descending\tSort in descending order.\n"
//		"/everything <search> Open files from an Everything search.\n"
//		"/random <search>\tOpen random files from an Everything search.\n"
		"/shuffle\t\tShuffle playlist.\n"
		"/<bmp|gif|ico|jpeg|jpg|png|tif|tiff|webp|emf|wmf>\n"
		"\t\tInstall association.\n"
		"/no<bmp|gif|ico|jpeg|jpg|png|tif|tiff|webp|emf|wmf>\n"
		"\t\tUninstall association.\n"
		"/appdata\t\tSave settings in appdata.\n"
		"/noappdata\tSave settings in exe path.\n"
		"/startmenu\tAdd Start menu shortcuts.\n"
		"/nostartmenu\tRemove Start menu shortcuts.\n"
		"/install <path>\tInstall to the specified path.\n"
		"/install-options <...> Run with the specified options after installation.\n"
		"/language <lang>\tSet the interface language: auto, english or chinese.\n"
		"/uninstall <path>\tUninstall from the specified path.\n");
		
	string_copy_utf8_string(caption_wbuf,localization_get_string(LOCALIZATION_ID_APP_NAME));

	// MB_ICONQUESTION avoids the messagebeep.
	viv_msgbox(_viv_hwnd,caption_wbuf,text_wbuf,MB_OK|MB_ICONQUESTION);
		
	mem_free(text_wbuf);
}
static void _viv_update_color_button_bitmap(HWND hwnd)
{	
	HBITMAP hbitmap;
	HDC screen_hdc;
	HDC mem_hdc;
	HGDIOBJ last_hbitmap;
	RECT rect;
	HBRUSH hbrush;
	COLORREF colorref;
	int wide;
	int high;
	
	wide = (64 * os_logical_wide) / 96;
	high = (13 * os_logical_high) / 96;
	
	colorref = (COLORREF)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	
	screen_hdc = GetDC(0);
	mem_hdc = CreateCompatibleDC(screen_hdc);
	
	hbitmap = CreateCompatibleBitmap(screen_hdc,wide,high);

	last_hbitmap = SelectObject(mem_hdc,hbitmap);
	
	hbrush = CreateSolidBrush(colorref);

	rect.left = 1;
	rect.top = 1;
	rect.right = wide-1;
	rect.bottom = high-1;
	FillRect(mem_hdc,&rect,hbrush);
	ExcludeClipRect(mem_hdc,rect.left,rect.top,rect.right,rect.bottom);
	
	rect.left = 0;
	rect.top = 0;
	rect.right = wide;
	rect.bottom = high;
	FillRect(mem_hdc,&rect,(HBRUSH)GetStockObject(BLACK_BRUSH));

	DeleteObject(hbrush);
	
	SelectObject(mem_hdc,last_hbitmap);
	
	DeleteDC(mem_hdc);
	ReleaseDC(0,screen_hdc);
	
	last_hbitmap = (HBITMAP)SendMessage(hwnd,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)hbitmap);
	if (last_hbitmap)
	{
		DeleteObject(last_hbitmap);
	}
}
static void _viv_delete_color_button_bitmap(HWND hwnd)
{	
	HGDIOBJ last_hbitmap;
	
	last_hbitmap = (HBITMAP)SendMessage(hwnd,BM_SETIMAGE,IMAGE_BITMAP,0);
	
	if (last_hbitmap)
	{
		DeleteObject(last_hbitmap);
	}
}
static LRESULT CALLBACK _viv_edit_key_edit_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	WNDPROC last_proc;
	
	last_proc = (WNDPROC)GetWindowLongPtr(hwnd,GWLP_USERDATA);
		
	switch(msg)
	{
		case WM_GETDLGCODE:
			return DLGC_WANTALLKEYS;
			
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			int key_flags;
			int vk;
			
			key_flags = _viv_get_current_key_mod_flags();
			vk = wParam;
			
			switch(vk)
			{
				case VK_CONTROL:
				case VK_SHIFT:
				case VK_MENU:
				case VK_LWIN:
				case VK_RWIN:
					return 0;

	//FIXME:						
	//					case VK_PROCESSKEY:
	//						// restore IME hacked key.
	//						vk = ImmGetVirtualKey(msg->hwnd);
	//						break;
			}

			_viv_edit_key_set_key(hwnd,key_flags | vk);
			_viv_options_edit_key_changed(GetParent(hwnd));

			return 0;
		}
	}
		
	if ((msg == WM_LBUTTONDOWN) || (msg == WM_RBUTTONDOWN))
	{
		SetFocus(hwnd);
		
		return 0;
	}
	
	// ignore mouse messages.
	if ((msg >= WM_MOUSEFIRST) && (msg <= WM_MOUSELAST))
	{
		return 0;
	}

	// ignore keyboard messages.
	if ((msg >= WM_KEYFIRST) && (msg <= WM_KEYLAST))
	{
		return 0;
	}
			
	return CallWindowProc(last_proc,hwnd,msg,wParam,lParam);
}
static void _viv_options_edit_key_changed(HWND hwnd)
{
	int keyflags;
	int command_index;
	
	keyflags = GetWindowLongPtr(hwnd,GWLP_USERDATA);
	
	ListBox_ResetContent(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST));
	SetWindowRedraw(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST),FALSE);
	
	for(command_index=0;command_index<_VIV_COMMAND_COUNT;command_index++)
	{
		config_key_t *key;
		
		key = _viv_key_list->start[command_index];
		
		while(key)
		{
			if (key->key == keyflags)
			{
				wchar_t command_wbuf[STRING_SIZE];
				
				_viv_get_command_name(command_wbuf,command_index);
				
				ListBox_AddString(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST),command_wbuf);
			}
		
			key = key->next;
		}
	}

	ListBox_SetCurSel(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST),0);

	SetWindowRedraw(GetDlgItem(hwnd,IDC_EDIT_KEY_CURRENTLY_USED_BY_LIST),TRUE);
}
static void _viv_edit_key_set_key(HWND hwnd,DWORD key_flags)
{
	wchar_t keytext_wbuf[STRING_SIZE];

	if (key_flags)
	{
		_viv_get_key_text(keytext_wbuf,key_flags);
	}
	else
	{
		keytext_wbuf[0] = 0;					
		SetWindowLongPtr(GetParent(hwnd),GWLP_USERDATA,0);
	}

	SetWindowLongPtr(GetParent(hwnd),GWLP_USERDATA,key_flags);

	SetWindowText(hwnd,keytext_wbuf);

	os_edit_move_caret_to_end(hwnd);
}
static void _viv_edit_key_remove_currently_used_by(_viv_key_list_t *keylist,DWORD keyflags)
{
	int command_index;
	
	for(command_index=0;command_index<_VIV_COMMAND_COUNT;command_index++)
	{
		_viv_key_remove(keylist,command_index,keyflags);
	}
}
static void _viv_jumpto_on_size(HWND hwnd)
{
	RECT rect;
	int wide;
	int high;
	int edit_high;
	int button_wide;
	int button_high;
	int y;
	int edge;
	int list_high;

	GetClientRect(hwnd,&rect);
	wide = rect.right - rect.left;
	high = rect.bottom - rect.top;

	GetWindowRect(GetDlgItem(hwnd,IDC_JUMPTO_EDIT),&rect);
	edit_high = rect.bottom - rect.top;

	GetWindowRect(GetDlgItem(hwnd,IDOK),&rect);
	button_wide = rect.right - rect.left;
	button_high = rect.bottom - rect.top;

	edge = (12 * os_logical_high) / 96;
	y = edge;

	SetWindowPos(GetDlgItem(hwnd,IDC_JUMPTO_EDIT),0,edge,y,wide-edge-edge,edit_high,SWP_NOACTIVATE|SWP_NOZORDER);

	y += edit_high + (6 * os_logical_high) / 96;

	high -= button_high + edge;

	list_high = (high - y) - edge;
	if (list_high < 0)
	{
		list_high = 0;
	}

	SetWindowPos(GetDlgItem(hwnd,IDC_JUMPTO_LIST),0,edge,y,wide-edge-edge,list_high,SWP_NOACTIVATE|SWP_NOZORDER);

	y += list_high + edge;

	SetWindowPos(GetDlgItem(hwnd,IDOK),0,wide - edge - button_wide - edge - button_wide,y,button_wide,button_high,SWP_NOACTIVATE|SWP_NOZORDER);
	SetWindowPos(GetDlgItem(hwnd,IDCANCEL),0,wide - edge - button_wide,y,button_wide,button_high,SWP_NOACTIVATE|SWP_NOZORDER);
}
static void _viv_jumpto_on_search(HWND hwnd)
{
	wchar_t search_wbuf[STRING_SIZE];
	int is_path_search;
	
	GetDlgItemText(hwnd,IDC_JUMPTO_EDIT,search_wbuf,STRING_SIZE);
	
	SendDlgItemMessage(hwnd,IDC_JUMPTO_LIST,WM_SETREDRAW,FALSE,0);
	
	ListBox_ResetContent(GetDlgItem(hwnd,IDC_JUMPTO_LIST));
	
	is_path_search = 0;
	
	{
		const wchar_t *p;
		
		p = search_wbuf;

		while(*p)
		{
			if ((*p == '\\') || (*p == '/') || (*p == ':'))
			{
				is_path_search = 1;
				break;
			}
			
			p++;
		}
	}
	
	{
		int i;
		
		for(i=0;i<_viv_nav_item_count;i++)
		{
			const wchar_t *name;
			const wchar_t *search_name;
			
			name = string_get_filename_part(_viv_nav_items[i]->fd.cFileName);
			
			if (is_path_search)
			{
				search_name = _viv_nav_items[i]->fd.cFileName;
			}
			else
			{
				search_name = name;
			}
			
			if ((!(*search_wbuf)) || (StrStrI(search_name,search_wbuf)))
			{
				int lb_index;
				
				lb_index = ListBox_AddString(GetDlgItem(hwnd,IDC_JUMPTO_LIST),string_get_filename_part(name));
				
				ListBox_SetItemData(GetDlgItem(hwnd,IDC_JUMPTO_LIST),lb_index,i);
			}
		}
	}
	
	ListBox_SetCurSel(GetDlgItem(hwnd,IDC_JUMPTO_LIST),0);
	
	SendDlgItemMessage(hwnd,IDC_JUMPTO_LIST,WM_SETREDRAW,TRUE,0);
}
static void _viv_jumpto_open_sel(HWND hwnd)
{
	int lb_index;
	
	lb_index = ListBox_GetCurSel(GetDlgItem(hwnd,IDC_JUMPTO_LIST));

	if (lb_index != LB_ERR)
	{	
		int index;
		
		index = ListBox_GetItemData(GetDlgItem(hwnd,IDC_JUMPTO_LIST),lb_index);
	
		if ((index >= 0) && (index < _viv_nav_item_count))
		{
			_viv_open(&_viv_nav_items[index]->fd,0);
		}
	}
}
static LRESULT CALLBACK _viv_jumpto_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
		

			{
				int cur_index;
				
				cur_index = LB_ERR;

				os_SetWindowText_localization_id(hwnd,LOCALIZATION_ID_JUMP_TO_TITLE);
				os_SetDlgItemText_localization_id(hwnd,IDOK,LOCALIZATION_ID_OK_BUTTON);
				os_SetDlgItemText_localization_id(hwnd,IDCANCEL,LOCALIZATION_ID_CANCEL_BUTTON);
					
				os_center_dialog(hwnd);
				
				_viv_nav_item_free_all();
				
				if (_viv_playlist_start)
				{
					_viv_playlist_t *d;
					
					d = _viv_playlist_start;
					while(d)
					{
						_viv_nav_item_add(&d->fd);
					
						d = d->next;
					}
				}
				else
				if (*_viv_current_fd->cFileName)
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
								string_path_combine(search_wbuf,path_wbuf,fd.cFileName);
								string_copy_with_bufsize(fd.cFileName,MAX_PATH,search_wbuf);
								
								_viv_nav_item_add(&fd);
							}
							
							if (!FindNextFile(h,&fd)) break;
						}

						FindClose(h);
					}
				}
				
				if (_viv_nav_item_count)
				{
					_viv_nav_item_t **d;
					_viv_nav_item_t *navitem;
					
					_viv_nav_items = (_viv_nav_item_t **)mem_alloc(safe_size_mul_sizeof_pointer((SIZE_T)_viv_nav_item_count));
					d = _viv_nav_items;
					
					navitem = __viv_nav_item_start;
					while(navitem)
					{
						*d++ = navitem;
						
						navitem = navitem->next;
					}
				}
				
				// sort by name 
				os_qsort(_viv_nav_items,_viv_nav_item_count,_viv_nav_compare);
				
				// find current.
				{
					_viv_nav_item_t **p;
					SIZE_T run;
					int index;
					
					p = _viv_nav_items;
					run = _viv_nav_item_count;
					index = 0;
					
					while(run)
					{
						if (string_compare((*p)->fd.cFileName,_viv_current_fd->cFileName) == 0)
						{
							cur_index = index;
						}
					
						index++;
						p++;
						run--;
					}
				}
				
				_viv_jumpto_on_size(hwnd);
				_viv_jumpto_on_search(hwnd);
				
				if (cur_index != LB_ERR)
				{
					ListBox_SetCurSel(GetDlgItem(hwnd,IDC_JUMPTO_LIST),cur_index);
					
					_viv_center_listbox_item(GetDlgItem(hwnd,IDC_JUMPTO_LIST),cur_index);
				}
			}
			
			return TRUE;
			
		case WM_COMMAND:
		
			switch(LOWORD(wParam))
			{
				case IDC_JUMPTO_EDIT:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						_viv_jumpto_on_search(hwnd);
						break;
					}
					break;

				case IDC_JUMPTO_LIST:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						_viv_jumpto_open_sel(hwnd);
						_viv_jump_ret = 1;
						break;
					}
					break;
					
				case IDOK:
					_viv_jumpto_open_sel(hwnd);
					_viv_jump_ret = 1;
					break;

				case IDCANCEL:
					_viv_jump_ret = 1;
					break;
			}
			
			break;
		
		case WM_SIZE:
			_viv_jumpto_on_size(hwnd);
			break;

		case WM_CLOSE:
			_viv_jump_ret = 1;
			return TRUE;
	    
		case WM_DESTROY:
			_viv_nav_item_free_all();
			break;
	}
	
	return FALSE;
}
void _viv_show_jumpto(void)
{
	HWND dialog_hwnd;
	
	_viv_jump_ret = 0;
	
	dialog_hwnd = CreateDialog(os_hinstance,MAKEINTRESOURCE(IDD_JUMPTO),_viv_hwnd,_viv_jumpto_proc);
	
	if (dialog_hwnd)
	{
		ShowWindow(dialog_hwnd,SW_SHOW);
		EnableWindow(_viv_hwnd,FALSE);

		while(!_viv_jump_ret)
		{
			MSG msg;
			
			if (!GetMessageW(&msg,NULL,0,0))
			{
				// the modal pump consumed a WM_QUIT (GetMessage returned 0):
				// re-inject it so the main message loop terminates instead of
				// blocking in WaitMessage forever.
				PostQuitMessage((int)msg.wParam);
				break;
			}
			
			if ((msg.hwnd == dialog_hwnd) || (IsChild(dialog_hwnd,msg.hwnd)))
			{
				switch (msg.message)
				{
					case WM_MOUSEWHEEL:
						
						SendMessage(GetDlgItem(dialog_hwnd,IDC_JUMPTO_LIST),WM_MOUSEWHEEL,msg.wParam,msg.lParam);
						continue;
						
					case WM_SYSKEYDOWN:
					case WM_KEYDOWN:
					
						if (msg.hwnd == GetDlgItem(dialog_hwnd,IDC_JUMPTO_EDIT))
						{
							int key_flags;
							int vk;
							
							key_flags = _viv_get_current_key_mod_flags();
							vk = msg.wParam;
							
							switch(vk)
							{
								case VK_UP:
								case VK_DOWN:
								case VK_NEXT:
								case VK_PRIOR:
									
									/*
									
									{
										int count;
										int index;

										// msg->hwnd = GetDlgItem(dialog_hwnd,IDC_JUMPTO_LIST);
										
										index = -1;
										count = ListBox_GetCount(GetDlgItem(dialog_hwnd,IDC_JUMPTO_LIST));
										
										if (count)
										{
											switch (msg.wParam)
											{
												case VK_DOWN:
												case VK_NEXT:
													index = 0;
													break;
													
												default:
													index = count - 1;
													break;
											}
										}

										if (index != -1)
										{
											ListBox_SetCurSel(GetDlgItem(dialog_hwnd,IDC_JUMPTO_LIST),index);
										}

										SetFocus(GetDlgItem(dialog_hwnd,IDC_JUMPTO_LIST));					
									}
									*/
									
									SendMessage(GetDlgItem(dialog_hwnd,IDC_JUMPTO_LIST),msg.message,msg.wParam,msg.lParam);

									//SetFocus(GetDlgItem(dialog_hwnd,IDC_JUMPTO_LIST));					
									
									continue;
							}
						}
						
						break;
							
				}
			}
			
			if (!IsDialogMessageW(dialog_hwnd,&msg))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
		
		EnableWindow(_viv_hwnd,TRUE);
		DestroyWindow(dialog_hwnd);
	}
}
static void _viv_center_listbox_item(HWND listbox_hwnd,int item_index)
{
    int item_high;
    
    item_high = ListBox_GetItemHeight(listbox_hwnd,item_index);
    
    if (item_high > 0)
    {
		RECT client_rect;
		int visible_count;
		
		GetClientRect(listbox_hwnd,&client_rect);

		visible_count = (client_rect.bottom - client_rect.top) / item_high;
		
		if (visible_count > 0)
		{
			int new_top;
			int item_count;
			
			item_count = ListBox_GetCount(listbox_hwnd);

			new_top = item_index - (visible_count / 2);
			
			if (new_top < 0)
			{
				new_top = 0;
			}

			if (new_top > item_count - visible_count)
			{
				new_top = item_count - visible_count;
			}
			
			if (new_top < 0)
			{
				new_top = 0;
			}

			ListBox_SetTopIndex(listbox_hwnd,new_top);
		}
	}
}											
