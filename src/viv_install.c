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
// viv_install.c - install/uninstall, associations, registry, shortcuts, elevation.
// Pure physical move from viv.c (R70 split): bodies are unchanged.
#include "viv.h"
#include "viv_state.h"
#include "viv_install.h"

// forward declarations (order preserved from viv.c)
int _viv_process_install_command_line_options(wchar_t *cl);
void _viv_install_association_by_extension(const char *association,const char *description,const char *icon_location);
void _viv_uninstall_association_by_extension(const char *association);
int _viv_is_association(const char *association);
static int _viv_get_registry_string(HKEY hkey,const utf8_t *value,wchar_t *wbuf,int size_in_wchars);
static int _viv_set_registry_string(HKEY hkey,const utf8_t *value,const wchar_t *wbuf);
static void _viv_install_association(DWORD flags);
static void _viv_uninstall_association(DWORD flags);
static int _viv_is_voidimageviewer_process(DWORD process_id);
static void _viv_close_existing_process(void);
static void _viv_uninstall_delete_file(const wchar_t *path,const utf8_t *filename);
int _viv_is_start_menu_shortcuts(void);
static void _viv_install_start_menu_shortcuts(void);
static void _viv_uninstall_start_menu_shortcuts(void);
void _viv_append_admin_param(wchar_t *wbuf,const utf8_t *param);
static void _viv_install_add_remove_programs(const wchar_t *install_path);
static void _viv_uninstall_add_remove_programs(void);
static void _viv_install_copy_file(const wchar_t *install_path,const wchar_t *temp_path,const utf8_t *filename,int critical);
void _viv_get_exe_filename(wchar_t filename[STRING_SIZE]);


int _viv_process_install_command_line_options(wchar_t *cl)
{
	wchar_t *p;
	wchar_t buf[STRING_SIZE];
	DWORD install_flags;
	DWORD uninstall_flags;
	int appdata;
	int is_runas;
	wchar_t install_path[STRING_SIZE];
	wchar_t install_options[STRING_SIZE];
	wchar_t uninstall_path[STRING_SIZE];
	int startmenu;
	int language;
	int language_set;
	wchar_t *cl_start;
	int is_admin_install;
	int is_standard_user_install;
	
	startmenu = 0;
	install_flags = 0;
	uninstall_flags = 0;
	appdata = 0;
	is_runas = 0;
	is_admin_install = 0;
	is_standard_user_install = 0;
	language = config_language;
	language_set = 0;
	install_path[0] = 0;
	install_options[0] = 0;
	uninstall_path[0] = 0;

	p = string_skip_ws(cl);
	
	// skip exe filename
	p = string_get_word(p,buf,STRING_SIZE);
	p = string_skip_ws(p);

	cl_start = p;

	// skip first parameter.
	for(;;)
	{
		wchar_t *bufstart;
		BOOL was_quote;
		
		// no more text?
		if (!*p)
		{
			break;
		}
		
		was_quote = FALSE;
		if (*p == '"')
		{
			was_quote = TRUE;
		}
		
		p = string_get_word(p,buf,STRING_SIZE);
		p = string_skip_ws(p);
		
		bufstart = buf;
		
		// treat switches with a '.' as a filename.
		// eg: voidimageviewer.exe -my-image.png
		if ((!was_quote) && ((*bufstart == '/') || (*bufstart == '-')) && (!string_is_dot(buf)))
		{
			bufstart++;
			
			if (string_icompare_lowercase_ascii(bufstart,"install") == 0)
			{
				p = string_get_word(p,install_path,STRING_SIZE);
				p = string_skip_ws(p);				

				uninstall_path[0] = 0;

				is_admin_install = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"install-options") == 0)
			{
				p = string_get_word(p,install_options,STRING_SIZE);
				p = string_skip_ws(p);				

				is_admin_install = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"uninstall") == 0)
			{
				p = string_get_word(p,uninstall_path,STRING_SIZE);
				p = string_skip_ws(p);			
				
				// no uninstall path?
				if (!uninstall_path[0])	
				{
					// caller will need to manually delete voidimageviewer.exe
					string_get_exe_path(uninstall_path);
				}

				// uninstall all
				uninstall_flags = 0xffffffff;
				install_flags = 0;
				startmenu = -1;

				install_path[0] = 0;
				
				is_admin_install = 1;
				is_standard_user_install = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"appdata") == 0)
			{
				appdata = 1;
				is_admin_install = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"noappdata") == 0)
			{
				appdata = -1;
				is_admin_install = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"startmenu") == 0)
			{
				startmenu = 1;
				is_admin_install = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"nostartmenu") == 0)
			{
				startmenu = -1;
				is_admin_install = 1;
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"language") == 0)
			{
				wchar_t language_wbuf[STRING_SIZE];
				
				p = string_get_word(p,language_wbuf,STRING_SIZE);
				p = string_skip_ws(p);
				
				// map the language name to a config_language value.
				// 0 = auto (system), 1 = english, 2 = simplified chinese.
				// this does not require admin rights, the setting is stored in the ini.
				
				if (string_icompare_lowercase_ascii(language_wbuf,"english") == 0)
				{
					language = 1;
					language_set = 1;
				}
				else
				if (string_icompare_lowercase_ascii(language_wbuf,"chinese") == 0)
				{
					language = 2;
					language_set = 1;
				}
				else
				if (string_icompare_lowercase_ascii(language_wbuf,"auto") == 0)
				{
					language = 0;
					language_set = 1;
				}
			}
			else
			if (string_icompare_lowercase_ascii(bufstart,"isrunas") == 0)
			{
				is_runas = 1;
			}
			else
			{
				int exti;
				int is_no;
				
				is_no = 0;
			
				if (string_istartwith_lowercase_ascii(bufstart,"no"))
				{
					bufstart += 2;
					is_no = 1;
				}
				
				for(exti=0;exti<_VIV_ASSOCIATION_COUNT;exti++)
				{
					if (string_icompare_lowercase_ascii(bufstart,_viv_association_extensions[exti]) == 0)
					{
						if (is_no)
						{
							uninstall_flags |= 1 << exti;
						}
						else
						{
							install_flags |= 1 << exti;
						}
						
						is_standard_user_install = 1;
						break;
					}
				}
			}
		}
	}
	
	// debug_printf("isrunas %d install_flags %u uninstall_flags %u\n",is_runas,install_flags,uninstall_flags);

	// do associationas as standard user
	if (!is_runas)
	{
		if (install_flags)
		{
			_viv_install_association(install_flags);
		}
	
		if (uninstall_flags)
		{
			_viv_uninstall_association(uninstall_flags);
		}
	}
		
	if (is_admin_install)
	{
		if (!os_is_admin())
		{
			if (!is_runas)
			{
				wchar_t exe_filename[STRING_SIZE];
				wchar_t params[STRING_SIZE];
				wchar_t cwd[STRING_SIZE];
				
				_viv_get_exe_filename(exe_filename);
				GetCurrentDirectory(STRING_SIZE,cwd);
				
				string_copy_utf8_string(params,(const utf8_t *)"/isrunas ");
				string_cat(params,cl_start);
				
				os_shell_execute(0,exe_filename,1,"runas",params);
				
				return 1;
			}
		}
	}
	
	if (language_set)
	{
		config_language = language;
		
		// save the language selection to the current settings location.
		// (before the appdata handling below so that its saves include the new language)
		config_save_settings(config_appdata);
	}
	
	if (appdata > 0)
	{	
		config_appdata = 1;
		
		config_save_settings(config_appdata);

		// appdata enabled.
		config_save_settings(0);
	}
	else
	if (appdata < 0)
	{
		config_appdata = 0;
		
		// appdata disabled
		config_save_settings(0);
	}
	
	if (startmenu > 0)
	{
		_viv_install_start_menu_shortcuts();
	}
	else
	if (startmenu < 0)
	{
		_viv_uninstall_start_menu_shortcuts();
	}
	
	if (install_path[0])
	{
		wchar_t temp_path[STRING_SIZE];
		
		// make sure no other process is running.
		_viv_close_existing_process();
		
		string_get_exe_path(temp_path);
		
		os_make_sure_path_exists(install_path);
		
		_viv_install_copy_file(install_path,temp_path,(const utf8_t *)"voidImageViewer.exe",1);
		_viv_install_copy_file(install_path,temp_path,(const utf8_t *)"Uninstall.exe",0);
		_viv_install_copy_file(install_path,temp_path,(const utf8_t *)"Changes.txt",0);
		
		// register in add/remove programs so the app shows up in
		// programs and features.
		_viv_install_add_remove_programs(install_path);
		
		if (install_options[0])
		{
			wchar_t new_exe_filename_wbuf[STRING_SIZE];
			
			string_path_combine_utf8(new_exe_filename_wbuf,install_path,(const utf8_t *)"voidImageViewer.exe");
			
			os_shell_execute(0,new_exe_filename_wbuf,1,NULL,install_options);
		}
	}
	
	if (uninstall_path[0])
	{
		wchar_t path[STRING_SIZE];
		
		// make sure no other process is running.
		_viv_close_existing_process();
		
		// remove our add/remove programs entry (from both hives).
		_viv_uninstall_add_remove_programs();
		
		// remove %APPDATA%\voidimageviewer
		if (string_get_appdata_voidimageviewer_path(path))
		{
			_viv_uninstall_delete_file(path,(const utf8_t *)"voidImageViewer.ini");

			RemoveDirectory(path);
		}
					
		_viv_uninstall_delete_file(uninstall_path,(const utf8_t *)"Uninstall.exe");
		_viv_uninstall_delete_file(uninstall_path,(const utf8_t *)"Changes.txt");
		_viv_uninstall_delete_file(uninstall_path,(const utf8_t *)"voidImageViewer.ini");
		_viv_uninstall_delete_file(uninstall_path,(const utf8_t *)"voidImageViewer.exe");
		
		RemoveDirectory(uninstall_path);
	}
	
	if ((is_admin_install) || (is_standard_user_install))
	{
		return 1;
	}
	
	return 0;
}
// use the default class description, ie: TXT File
void _viv_install_association_by_extension(const char *association,const char *description,const char *icon_location)
{
	HKEY hkey;
	wchar_t class_name[STRING_SIZE];
	wchar_t key[STRING_SIZE];
	wchar_t default_icon[STRING_SIZE];
	wchar_t dot_association[STRING_SIZE];
	LONG reg_ret;
	
	string_copy_utf8_string(dot_association,(const utf8_t *)".");
	string_cat_utf8(dot_association,association);
	
	// make sure we uninstall old associations first.
	_viv_uninstall_association_by_extension(association);

	string_copy_utf8_string(class_name,"voidImageViewer");
	string_cat(class_name,dot_association);

	string_copy_utf8_string(default_icon,"SOFTWARE\\Classes\\voidImageViewer");
	string_cat(default_icon,dot_association);
	string_cat_utf8(default_icon,"\\DefaultIcon");

	if (RegCreateKeyExW(HKEY_CURRENT_USER,default_icon,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		if (icon_location)
		{
			wchar_t icon_location_wbuf[STRING_SIZE];
			
			string_copy_utf8_string(icon_location_wbuf,icon_location);

			_viv_set_registry_string(hkey,0,icon_location_wbuf);
		}
		else
		{
			wchar_t filename[STRING_SIZE];
			wchar_t command[STRING_SIZE];

			_viv_get_exe_filename(filename);
			
			string_copy(command,filename);

			string_cat_utf8(command,(const utf8_t *)",0");
			
			_viv_set_registry_string(hkey,0,command);
		}
			
		RegCloseKey(hkey);
	}	
		
	string_copy_utf8_string(key,"SOFTWARE\\Classes\\");
	string_cat(key,class_name);
	
	if (RegCreateKeyExW(HKEY_CURRENT_USER,key,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t description_wbuf[STRING_SIZE];
		
		string_copy_utf8_string(description_wbuf,description);
		
		_viv_set_registry_string(hkey,0,description_wbuf);
		
		RegCloseKey(hkey);
	}		
	
	string_copy_utf8_string(key,"SOFTWARE\\Classes\\");
	string_cat(key,class_name);
	string_cat_utf8(key,"\\shell\\open\\command");
	
	if (RegCreateKeyExW(HKEY_CURRENT_USER,key,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t filename[STRING_SIZE];
		wchar_t command[STRING_SIZE];

		_viv_get_exe_filename(filename);
		
		string_copy_utf8_string(command,(const utf8_t *)"\"");
		string_cat(command,filename);
		string_cat_utf8(command,(const utf8_t *)"\" \"%1\"");
		
		_viv_set_registry_string(hkey,0,command);
		
		RegCloseKey(hkey);
	}

	string_copy_utf8_string(key,"SOFTWARE\\Classes\\");
	string_cat(key,dot_association);
	
	reg_ret = RegCreateKeyExW(HKEY_CURRENT_USER,key,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0);
	if (reg_ret == ERROR_SUCCESS)
	{
		wchar_t wbuf[STRING_SIZE];
		
		if (!_viv_get_registry_string(hkey,(const utf8_t *)"voidImageViewer.Backup",wbuf,STRING_SIZE))
		{
			if (!_viv_get_registry_string(hkey,0,wbuf,STRING_SIZE))
			{
				*wbuf = 0;
			}
			
			_viv_set_registry_string(hkey,(const utf8_t *)"voidImageViewer.Backup",wbuf);
		}

		_viv_set_registry_string(hkey,0,class_name);
		
		RegCloseKey(hkey);
	}
	else
	{
		debug_printf("RegCreateKeyExW failed %u\n",reg_ret);
	}
}
void _viv_uninstall_association_by_extension(const char *association)
{
	long reg_ret;
	HKEY hkey;
	wchar_t key[STRING_SIZE];
	
	string_copy_utf8_string(key,(const utf8_t *)"SOFTWARE\\Classes\\.");
	string_cat_utf8(key,association);
	
	// debug_printf("query %S\n",key);
	
	if (RegCreateKeyExW(HKEY_CURRENT_USER,key,0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t wbuf[STRING_SIZE];
		
		if (_viv_get_registry_string(hkey,(const utf8_t *)"voidImageViewer.Backup",wbuf,STRING_SIZE))
		{
			_viv_set_registry_string(hkey,0,wbuf);

			// debug_printf("Delete voidImageViewer.Backup\n");

			reg_ret = RegDeleteValueA(hkey,"voidImageViewer.Backup");
			if (reg_ret == ERROR_SUCCESS)
			{
				// debug_printf("Delete voidImageViewer.Backup OK\n");
			}
			else
			{
				debug_printf("RegDeleteValueA failed %u\n",reg_ret);
			}
		}

		RegCloseKey(hkey);
	}
	
	string_copy_utf8_string(key,(const utf8_t *)"SOFTWARE\\Classes\\voidImageViewer.");
	string_cat_utf8(key,association);
	
	RegDeleteKey(HKEY_CURRENT_USER,key);
}
int _viv_is_association(const char *association)
{
	int ret;
	HKEY hkey;
	wchar_t class_name[STRING_SIZE];
	wchar_t key[STRING_SIZE];
	
	string_copy_utf8_string(class_name,(const utf8_t *)"voidImageViewer.");
	string_cat_utf8(class_name,association);
	
	ret = 0;

	string_copy_utf8_string(key,"SOFTWARE\\Classes\\.");
	string_cat_utf8(key,association);
	
//debug_printf("key %S\n",key);
	
	if (RegOpenKeyExW(HKEY_CURRENT_USER,key,0,KEY_QUERY_VALUE,&hkey) == ERROR_SUCCESS)
	{
		wchar_t wbuf[STRING_SIZE];

//debug_printf("key OK\n",key);

		if (_viv_get_registry_string(hkey,0,wbuf,STRING_SIZE))
		{
//debug_printf("value cmp %S %S\n",wbuf,class_name);
			if (string_compare(wbuf,class_name) == 0)
			{
				ret++;
			}
		}

		RegCloseKey(hkey);
	}

	string_copy_utf8_string(key,"SOFTWARE\\Classes\\");
	string_cat(key,class_name);
	string_cat_utf8(key,(const utf8_t *)"\\shell\\open\\command");
	
//debug_printf("key %S\n",key);
	
	if (RegOpenKeyExW(HKEY_CURRENT_USER,key,0,KEY_QUERY_VALUE,&hkey) == ERROR_SUCCESS)
	{
		wchar_t wbuf[STRING_SIZE];

		if (_viv_get_registry_string(hkey,0,wbuf,STRING_SIZE))
		{
			wchar_t filename[STRING_SIZE];
			wchar_t command[STRING_SIZE];

//debug_printf("key OK\n",key);

			_viv_get_exe_filename(filename);
			
			string_copy_utf8_string(command,(const utf8_t *)"\"");
			string_cat(command,filename);
			string_cat_utf8(command,(const utf8_t *)"\" \"%1\"");

//debug_printf("value cmp %S %S\n",wbuf,command);
			
			if (string_compare(wbuf,command) == 0)
			{
				ret++;
			}
		}

		RegCloseKey(hkey);
	}

	return ret == 2;
}
static int _viv_get_registry_string(HKEY hkey,const utf8_t *value,wchar_t *wbuf,int size_in_wchars)
{
	wchar_t value_wbuf[STRING_SIZE];
	wchar_t *value_wp;
	DWORD cbData;
	DWORD type;
	
	if (value)
	{
		string_copy_utf8_string(value_wbuf,value);
		value_wp = value_wbuf;
	}
	else
	{
		value_wp = 0;
	}
	
	cbData = size_in_wchars * sizeof(wchar_t);
	
	if (RegQueryValueExW(hkey,value_wp,0,&type,(BYTE *)wbuf,&cbData) == ERROR_SUCCESS)
	{
		if ((type == REG_SZ) || (type == REG_EXPAND_SZ))
		{
			return 1;
		}
	}
	
	return 0;
}
static int _viv_set_registry_string(HKEY hkey,const utf8_t *value,const wchar_t *wbuf)
{
	wchar_t value_wbuf[STRING_SIZE];
	wchar_t *value_wp;
	LONG reg_ret;

	if (value)
	{
		string_copy_utf8_string(value_wbuf,value);
		value_wp = value_wbuf;
	}
	else
	{
		value_wp = 0;
	}	

	reg_ret = RegSetValueExW(hkey,value_wp,0,REG_SZ,(BYTE *)wbuf,(string_get_length(wbuf) + 1) * sizeof(wchar_t));
	if (reg_ret == ERROR_SUCCESS)
	{
		return 1;
	}
	else
	{
		debug_printf("failed to set reg value %u %s %S",reg_ret,value,wbuf);
	}
	
	return 0;
}
static void _viv_install_association(DWORD flags)
{
	int i;
	
	for(i=0;i<_VIV_ASSOCIATION_COUNT;i++)
	{
		if (flags & (1 << i))
		{
			_viv_install_association_by_extension(_viv_association_extensions[i],localization_get_string(_viv_association_description_localization_id_array[i]),_viv_association_icon_locations[i]);
		}
	}
}
static void _viv_uninstall_association(DWORD flags)
{
	int i;
	
	for(i=0;i<_VIV_ASSOCIATION_COUNT;i++)
	{
		if (flags & 1 << i)
		{
			_viv_uninstall_association_by_extension(_viv_association_extensions[i]);
		}
	}
}
// close every running instance before (un)install. the original loop
// waited forever: a hung instance would block install, uninstall and
// exit forever. (this resolves the upstream FIXME: bounded waits, with
// a last resort terminate.)
// FIXME: we should check for the process name voidImageViewer.exe rather than the
// window class name. -be careful when uninstalling as the non-admin process will be
// waiting for the admin process to exit.
// QueryFullProcessImageNameW (Vista+) is resolved lazily right here:
// the install/uninstall path runs before os_init so the central
// runtime table is not populated yet, and the headers gate the
// declaration behind a newer _WIN32_WINNT than the project targets.
static BOOL (WINAPI *_viv_query_full_process_image_name)(HANDLE,DWORD,wchar_t *,DWORD *);
static int _viv_is_voidimageviewer_process(DWORD process_id)
{
	HANDLE process_handle;
	int ret;
	
	// conservative default: if the image name can not be queried we
	// assume the window is ours. in an uninstall context closing a
	// real instance is preferred over leaving a locked file behind.
	ret = 1;
	
	process_handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,process_id);
	
	if (process_handle)
	{
		wchar_t image_name[STRING_SIZE];
		DWORD image_name_length;
		
		if (!_viv_query_full_process_image_name)
		{
			_viv_query_full_process_image_name = (void *)GetProcAddress(GetModuleHandleA("kernel32.dll"),"QueryFullProcessImageNameW");
		}
		
		image_name_length = STRING_SIZE;
		
		if ((_viv_query_full_process_image_name) && (_viv_query_full_process_image_name(process_handle,0,image_name,&image_name_length)))
		{
			const wchar_t *basename;
			const wchar_t *wanted;
			
			// the name past the last separator is the image file name.
			basename = image_name + image_name_length;
			while ((basename > image_name) && (basename[-1] != L'\\') && (basename[-1] != L':'))
			{
				basename--;
			}
			
			// case insensitive compare against voidImageViewer.exe.
			wanted = L"voidImageViewer.exe";
			
			while ((*basename) && (*wanted))
			{
				wchar_t c1;
				wchar_t c2;
				
				c1 = *basename;
				c2 = *wanted;
				
				if ((c1 >= L'A') && (c1 <= L'Z'))
				{
					c1 += (L'a' - L'A');
				}
				
				if ((c2 >= L'A') && (c2 <= L'Z'))
				{
					c2 += (L'a' - L'A');
				}
				
				if (c1 != c2)
				{
					break;
				}
				
				basename++;
				wanted++;
			}
			
			if ((*basename) || (*wanted))
			{
					// the image is not voidImageViewer.exe: not our window.
					ret = 0;
				}
		}
		
		CloseHandle(process_handle);
	}
	
	return ret;
}
static void _viv_close_existing_process(void)
{
	int attempts;
	
	// close at most 16 instances; a window that survives a close attempt
	// stops the loop early, so the cap only guards a pathological
	// instance spawning loop, not a normal machine.
	for(attempts = 0;attempts < 16;attempts++)
	{
		HWND hwnd;
		DWORD process_id;
		HANDLE process_handle;
		
		hwnd = FindWindowA("VOIDIMAGEVIEWER",0);
		
		if (!hwnd)
		{
			// no window open
			break;
		}
		
		// ask the instance to close. SMTO_ABORTIFHUNG gives up on an
		// unresponsive window instead of blocking us forever.
		SendMessageTimeoutA(hwnd,WM_CLOSE,0,0,SMTO_ABORTIFHUNG,5000,0);
		
		process_id = 0;
		GetWindowThreadProcessId(hwnd,&process_id);
		
		// the window class is a weak identity: any program can register
		// the same class name and the uninstaller would then close a
		// window it does not own. verify the process image name first.
		if ((process_id) && (!_viv_is_voidimageviewer_process(process_id)))
		{
			// the window belongs to another program: leave it alone and
			// stop looking (findwindow would return the same window).
			break;
		}
		
		process_handle = 0;
		
		if (process_id)
		{
			// PROCESS_TERMINATE is for the last resort kill below. opening
			// may fail (NULL) for an elevated instance from a non elevated
			// uninstaller: we then skip the wait and the window check below
			// stops the loop instead of spinning.
			process_handle = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE,FALSE,process_id);
		}
		
		if (process_handle)
		{
			if (WaitForSingleObject(process_handle,5000) == WAIT_TIMEOUT)
			{
				// the instance did not exit in time: force it.
				TerminateProcess(process_handle,1);
				
				WaitForSingleObject(process_handle,5000);
			}
			
			CloseHandle(process_handle);
		}
		
		// if the window survived the close (and the possible kill),
		// repeating the same failing attempt can not succeed: stop
		// instead of stalling setup.
		if (hwnd == FindWindowA("VOIDIMAGEVIEWER",0))
		{
			break;
		}
	}
}
static void _viv_uninstall_delete_file(const wchar_t *path,const utf8_t *filename)
{
	wchar_t full_path_and_filename[STRING_SIZE];
	
	string_path_combine_utf8(full_path_and_filename,path,filename);
	
	DeleteFile(full_path_and_filename);
}
int _viv_is_start_menu_shortcuts(void)
{
	wchar_t special_folder_path_wbuf[STRING_SIZE];
	wchar_t path_wbuf[STRING_SIZE];
	int ret;
	
	ret = 0;

	// delete old english shortcuts
	// delete shortcuts
	if (os_get_special_folder_path(special_folder_path_wbuf,CSIDL_COMMON_PROGRAMS))
	{
		string_path_combine_utf8(path_wbuf,special_folder_path_wbuf,(const utf8_t *)"void Image Viewer");

		// make sure this directory exists!		
		if (GetFileAttributesW(path_wbuf) != INVALID_FILE_ATTRIBUTES)
		{
			// at least one item exists, ret = 2
			ret = 1;
		}
	}

	return ret;
}
static void _viv_install_start_menu_shortcuts(void)
{
	wchar_t special_folder_path_wbuf[STRING_SIZE];
	
	// always uninstall and reinstall
	// this allows us to switch between english and another language.
	_viv_uninstall_start_menu_shortcuts();

	// create shortcuts
	if (os_get_special_folder_path(special_folder_path_wbuf,CSIDL_COMMON_PROGRAMS))
	{
		wchar_t path_wbuf[STRING_SIZE];
		wchar_t exe_filename_wbuf[STRING_SIZE];
		wchar_t exe_path_wbuf[STRING_SIZE];
		wchar_t uninstall_filename_wbuf[STRING_SIZE];
		wchar_t lnk_wbuf[STRING_SIZE];
		
		string_path_combine_utf8(path_wbuf,special_folder_path_wbuf,(const utf8_t *)"void Image Viewer");

		// make sure this directory exists!		
		os_make_sure_path_exists(path_wbuf);

		// void image viewer.lnk
		_viv_get_exe_filename(exe_filename_wbuf);
		string_path_combine_utf8(lnk_wbuf,path_wbuf,"void Image Viewer.lnk");
		os_create_shell_link(exe_filename_wbuf,lnk_wbuf);
	
		// uninstall
		string_get_path_part(exe_path_wbuf,exe_filename_wbuf);
		string_path_combine_utf8(uninstall_filename_wbuf,exe_path_wbuf,(const utf8_t *)"Uninstall.exe");

		string_path_combine_utf8(lnk_wbuf,path_wbuf,"Uninstall.lnk");
		os_create_shell_link(uninstall_filename_wbuf,lnk_wbuf);
	}
}
static void _viv_uninstall_start_menu_shortcuts(void)
{
	wchar_t special_folder_path_wbuf[STRING_SIZE];
	wchar_t path_wbuf[STRING_SIZE];

	// delete old english shortcuts
	// delete shortcuts
	if (os_get_special_folder_path(special_folder_path_wbuf,CSIDL_COMMON_PROGRAMS))
	{
		wchar_t lnk_wbuf[STRING_SIZE];

		string_path_combine_utf8(path_wbuf,special_folder_path_wbuf,(const utf8_t *)"void Image Viewer");
		
		// localized.
		string_path_combine_utf8(lnk_wbuf,path_wbuf,"void Image Viewer.lnk");
		DeleteFile(lnk_wbuf);
		
		string_path_combine_utf8(lnk_wbuf,path_wbuf,"Uninstall.lnk");
		DeleteFile(lnk_wbuf);
		
		RemoveDirectory(path_wbuf);
	}
}
void _viv_append_admin_param(wchar_t *wbuf,const utf8_t *param)
{
	string_cat_utf8(wbuf,(const utf8_t *)" /");
	string_cat_utf8(wbuf,param);
}
// register voidImageViewer in add/remove programs (programs and
// features). the setup runs the exe with /install so the exe owns this
// key: hklm for admin installs (what the setup's .onInit reads back),
// hkcu otherwise. the uninstall string is quoted so windows can run it
// with spaces in the path.
static void _viv_install_add_remove_programs(const wchar_t *install_path)
{
	HKEY hkey;
	HKEY root;

	root = os_is_admin() ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	
	// write the 64-bit view explicitly: a 32-bit build used to land in
	// wow6432node, which the nsis uninstall probe (setregview 64) never
	// reads. the flag is ignored by 64-bit builds and on 32-bit windows.
	if (RegCreateKeyExW(root,L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\voidImageViewer",0,0,0,KEY_QUERY_VALUE|KEY_SET_VALUE|KEY_WOW64_64KEY,0,&hkey,0) == ERROR_SUCCESS)
	{
		wchar_t uninstall_wbuf[STRING_SIZE];
		wchar_t icon_wbuf[STRING_SIZE];
		wchar_t version_wbuf[STRING_SIZE];
		DWORD no_modify_repair;
		
		uninstall_wbuf[0] = L'"';
		// string_cat budgets against the whole STRING_SIZE and ignores
		// the quote we already placed: reserve the suffix so a long
		// install path can never truncate it mid-way.
		string_copy_with_bufsize(uninstall_wbuf + 1,STRING_SIZE - 1 - 16,install_path);
		string_cat_utf8(uninstall_wbuf,(const utf8_t *)"\\Uninstall.exe\"");
		
		string_copy(icon_wbuf,install_path);
		string_cat_utf8(icon_wbuf,(const utf8_t *)"\\voidImageViewer.exe,0");
		
		// the release identity straight from version.h - must match the tag
		string_printf(version_wbuf,"%s%s",VERSION_STRING,VERSION_TYPE);
		
		no_modify_repair = 1;
		
		RegSetValueExW(hkey,L"DisplayName",0,REG_SZ,(BYTE *)L"void Image Viewer",sizeof(L"void Image Viewer"));
		RegSetValueExW(hkey,L"DisplayVersion",0,REG_SZ,(BYTE *)version_wbuf,(string_get_length(version_wbuf) + 1) * sizeof(wchar_t));
		RegSetValueExW(hkey,L"Publisher",0,REG_SZ,(BYTE *)L"voidtools",sizeof(L"voidtools"));
		RegSetValueExW(hkey,L"InstallLocation",0,REG_SZ,(BYTE *)install_path,(string_get_length(install_path) + 1) * sizeof(wchar_t));
		RegSetValueExW(hkey,L"DisplayIcon",0,REG_SZ,(BYTE *)icon_wbuf,(string_get_length(icon_wbuf) + 1) * sizeof(wchar_t));
		RegSetValueExW(hkey,L"UninstallString",0,REG_SZ,(BYTE *)uninstall_wbuf,(string_get_length(uninstall_wbuf) + 1) * sizeof(wchar_t));
		RegSetValueExW(hkey,L"NoModify",0,REG_DWORD,(BYTE *)&no_modify_repair,sizeof(DWORD));
		RegSetValueExW(hkey,L"NoRepair",0,REG_DWORD,(BYTE *)&no_modify_repair,sizeof(DWORD));
		
		RegCloseKey(hkey);
	}
}
// remove the add/remove programs entry from both hives: the admin
// install writes hklm, a standard user install writes hkcu.
static void _viv_uninstall_add_remove_programs(void)
{
	// regdeletekeyw cannot reach the alternate registry view (msdn), so
	// the 64-bit view the install writes needs regdeletekeyexw - resolved
	// lazily inside os_reg_delete_key_ex because this path runs before
	// os_init. the plain regdeletekeyw sweep after it removes the
	// wow6432node copy older 32-bit builds left behind (idempotent, and a
	// no-op on 64-bit builds where both calls hit the same view).
	os_reg_delete_key_ex(HKEY_LOCAL_MACHINE,L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\voidImageViewer",KEY_WOW64_64KEY);
	os_reg_delete_key_ex(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\voidImageViewer",KEY_WOW64_64KEY);
	RegDeleteKeyW(HKEY_LOCAL_MACHINE,L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\voidImageViewer");
	RegDeleteKeyW(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\voidImageViewer");
}
static void _viv_install_copy_file(const wchar_t *install_path,const wchar_t *temp_path,const utf8_t *filename,int critical)
{
	wchar_t dst_wbuf[STRING_SIZE];
	wchar_t src_wbuf[STRING_SIZE];
	
	string_path_combine_utf8(dst_wbuf,install_path,filename);
	string_path_combine_utf8(src_wbuf,temp_path,filename);
	
	if (!CopyFile(src_wbuf,dst_wbuf,FALSE))
	{
		if (critical)
		{
			debug_fatal("Error %d: unable to copy %S to %S",GetLastError(),src_wbuf,dst_wbuf);
		}
	}	
}
void _viv_get_exe_filename(wchar_t filename[STRING_SIZE])
{
	filename[0] = 0;
	
	GetModuleFileName(0,filename,STRING_SIZE);
	
	// make sure the drive letter is uppercase.
	// launching from .png == d:
	// launching from .exe == D:
	if ((*filename >= 'a') && (*filename <= 'z'))
	{
		if (filename[1] == ':')
		{
			*filename = *filename - 'a' + 'A';
		}
	}
}
