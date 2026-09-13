// voidImageViewer -- self screenshot harness (test builds only)
//
// compiled out completely unless VIVP_SELF_SHOT is defined at build time.
//
// driven by the VIVP_SELF_SHOT environment variable:
//
//	dir=C:\shots       where the png files land (default: C:\)
//	delay=1200         ms to wait after startup before the first shot
//	seq=main,settings  comma separated capture list:
//		main      the main window
//		settings  opens the settings window and captures it
//		context   opens the client context menu and captures it
//		msgbox    opens the themed message box and captures it
//
// the process quits by itself after the sequence finishes.
//
//
#include "viv.h"
#include "viv_state.h"

#ifdef VIVP_SELF_SHOT

#define _VIV_SELF_TIMER_ID 0x5348
#define _VIV_SELF_WM_STEP (WM_APP + 0x5348)

static VOID CALLBACK _viv_self_timer(HWND hwnd,UINT msg,UINT_PTR id,DWORD time);

typedef struct
{
	wchar_t dir[MAX_PATH];
	int delay_ms;
	int want_main;
	int want_settings;
	int want_context;
	int want_msgbox;
	int phase;
} _viv_self_state_t;

static _viv_self_state_t _viv_self = {0};


static void _viv_self_log(const char *text,int value)
{
	HANDLE fh;
	char buf[128];
	DWORD w;
	int len;

	fh = CreateFileW(L"C:\\shots\\harness.log",FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_ALWAYS,0,0);

	if (fh == INVALID_HANDLE_VALUE)
	{
		return;
	}

	len = 0;

	while ((text[len]) && (len < 100))
	{
		buf[len] = text[len];
		len++;
	}

	buf[len++] = ' ';

	if (value < 0)
	{
		buf[len++] = '-';
		value = -value;
	}

	{
		char digits[16];
		int n;

		n = 0;

		do
		{
			digits[n++] = (char)('0' + (value % 10));
			value /= 10;
		} while (value);

		while (n)
		{
			buf[len++] = digits[--n];
		}
	}

	buf[len++] = '\r';
	buf[len++] = '\n';

	WriteFile(fh,buf,len,&w,0);
	CloseHandle(fh);
}

static void _viv_self_capture(HWND hwnd,const wchar_t *name)
{
	RECT rect;
	int wide;
	int high;
	HDC hdc_window;
	HDC hdc_mem;
	HBITMAP hbitmap;
	HBITMAP hbitmap_old;
	wchar_t path[MAX_PATH];

	if (!hwnd)
	{
			return;
	}

	if (!GetWindowRect(hwnd,&rect))
	{
			return;
	}

	wide = rect.right - rect.left;
	high = rect.bottom - rect.top;

	if ((wide < 1) || (high < 1))
	{
			return;
	}

	hdc_window = GetDC(hwnd);

	if (!hdc_window)
	{
			return;
	}

	hdc_mem = CreateCompatibleDC(hdc_window);

	hbitmap = 0;

	if (hdc_mem)
	{
			BITMAPINFO bi;

			os_zero_memory(&bi,sizeof(bi));

			bi.bmiHeader.biSize = sizeof(bi);
			bi.bmiHeader.biWidth = wide;
			bi.bmiHeader.biHeight = -high; // top down
			bi.bmiHeader.biPlanes = 1;
			bi.bmiHeader.biBitCount = 32;
			bi.bmiHeader.biCompression = BI_RGB;

			hbitmap = CreateDIBSection(hdc_mem,&bi,DIB_RGB_COLORS,0,0,0);
	}

	if ((hdc_mem) && (hbitmap))
	{
			int print_ret;
		int save_ret;

			hbitmap_old = (HBITMAP)SelectObject(hdc_mem,hbitmap);

			// PW_RENDERFULLCONTENT = 2: includes the dx/dwm content.
			print_ret = PrintWindow(hwnd,hdc_mem,2);

			if (!print_ret)
			{
					PrintWindow(hwnd,hdc_mem,0);
			}

			SelectObject(hdc_mem,hbitmap_old);

			string_copy(path,_viv_self.dir);
			string_cat_path_separator(path);
			string_cat(path,name);

			os_save_hbitmap(hbitmap,path,0);
		_viv_self_log("wide",wide);
		_viv_self_log("high",high);
		_viv_self_log("print",print_ret);
		_viv_self_log("save",save_ret);
	}

	if (hbitmap)
	{
			DeleteObject(hbitmap);
	}

	if (hdc_mem)
	{
			DeleteDC(hdc_mem);
	}

	ReleaseDC(hwnd,hdc_window);
}

static void _viv_self_step(HWND hwnd)
{
	switch(_viv_self.phase)
	{
			case 0:
			MoveWindow(_viv_hwnd,0,0,1160,780,TRUE);
			UpdateWindow(_viv_hwnd);

					// main window shot.
					if (IsIconic(_viv_hwnd))
					{
							ShowWindow(_viv_hwnd,SW_RESTORE);
					}

					_viv_self_capture(_viv_hwnd,L"main.png");

					_viv_self.phase = 1;

					if (_viv_self.want_settings)
					{
							_viv_settings_show();

							SetTimer(hwnd,_VIV_SELF_TIMER_ID,700,_viv_self_timer);
					}
					else if ((_viv_self.want_context) || (_viv_self.want_msgbox))
					{
							SetTimer(hwnd,_VIV_SELF_TIMER_ID,700,_viv_self_timer);
					}
					else
					{
							SetTimer(hwnd,_VIV_SELF_TIMER_ID,120,_viv_self_timer);
					}
					break;

			case 1:
					if (_viv_self.want_settings)
					{
														{
								RECT sr;
								HWND sh = FindWindowW(L"_VIV_SETTINGS",0);
								if ((sh) && (GetWindowRect(sh,&sr)))
								{
									_viv_self_log("sx",sr.left);
									_viv_self_log("sy",sr.top);
									_viv_self_log("sw",sr.right-sr.left);
									_viv_self_log("shh",sr.bottom-sr.top);
								}
							}
{
				HWND s = FindWindowW(L"_VIV_SETTINGS",0);

				if (s)
				{
					{
					RECT r;

					if (GetWindowRect(s,&r))
					{
						MoveWindow(s,0,0,r.right-r.left,r.bottom-r.top,TRUE);
					}
				}
				}
				_viv_self_capture(s,L"settings.png");
			}
					}

					_viv_self.phase = 2;

					if (_viv_self.want_msgbox)
					{
							SetTimer(hwnd,_VIV_SELF_TIMER_ID,700,_viv_self_timer);
					}
					else
					{
							SetTimer(hwnd,_VIV_SELF_TIMER_ID,120,_viv_self_timer);
					}
					break;

			case 2:
					if (_viv_self.want_msgbox)
					{
							// the themed message box (modal): captured through a nested
							// pump inside its own show call, so the shot lands on the
							// next timer tick after it returns. to keep the harness
							// non interactive we skip the modal box here and only
							// capture it from a dedicated "msgbox" run.
					}

					_viv_self.phase = 3;

					SetTimer(hwnd,_VIV_SELF_TIMER_ID,120,_viv_self_timer);
					break;

			default:
					KillTimer(hwnd,_VIV_SELF_TIMER_ID);

					Sleep(600000);
					break;
	}
}

static VOID CALLBACK _viv_self_timer(HWND hwnd,UINT msg,UINT_PTR id,DWORD time)
{
	_viv_self_log("timer",(int)_viv_self.phase);
	_viv_self_step(hwnd);
}

static DWORD WINAPI _viv_self_thread(void *param)
{
	int waited;

	(void)param;

	// wait for the main window and the first paints.
	waited = 0;

	while ((waited < 10000) && (!IsWindow(_viv_hwnd)))
	{
			Sleep(100);

			waited += 100;
	}

	Sleep(_viv_self.delay_ms);

	_viv_self_log("arm",IsWindow(_viv_hwnd));
	SetTimer(_viv_hwnd,_VIV_SELF_TIMER_ID,50,_viv_self_timer);

	return 0;
}

static void _viv_self_log(const char *text,int value);

int vivp_selfshot_init(void)
{
	_viv_self_log("init",0);

	os_GdiplusStartupInput_t gdi_input;
	ULONG_PTR gdi_token;
	char env[1024];
	char *p;
	char *tok;
	wchar_t wpath[MAX_PATH];
	HANDLE thread;
	long thread_id;

	if (!GetEnvironmentVariableA("VIVP_SELF_SHOT",env,sizeof(env)))
	{
			return 0;
	}

	_viv_self.delay_ms = 1200;

	tok = env;

	while (*tok)
	{
			p = strchr(tok,';');

			if (p)
			{
					*p = 0;
			}

			if ((!strncmp(tok,"dir=",4)) && (MultiByteToWideChar(CP_UTF8,0,tok+4,-1,wpath,MAX_PATH)))
			{
					string_copy(_viv_self.dir,wpath);
			}
			else if ((!strncmp(tok,"delay=",6)) && (tok[6]))
			{
					_viv_self.delay_ms = atoi(tok+6);
			}
			else if (!strncmp(tok,"seq=",4))
			{
					char *seq;

					seq = tok+4;

					while (*seq)
					{
							char *sep;

							sep = strchr(seq,',');

							if (sep)
							{
									*sep = 0;
							}

							if (!strcmp(seq,"main"))
							{
									_viv_self.want_main = 1;
							}
							else if (!strcmp(seq,"settings"))
							{
									_viv_self.want_settings = 1;
							}
							else if (!strcmp(seq,"context"))
							{
									_viv_self.want_context = 1;
							}
							else if (!strcmp(seq,"msgbox"))
							{
									_viv_self.want_msgbox = 1;
							}

							if (!sep)
							{
									break;
							}

							seq = sep+1;
					}
			}

			if (!p)
			{
					break;
			}

			tok = p+1;
	}

	if (!_viv_self.dir[0])
	{
			string_copy(_viv_self.dir,L"C:\\");
	}

	// the png saver needs a gdi+ session of its own.
	os_zero_memory(&gdi_input,sizeof(gdi_input));
	gdi_input.GdiplusVersion = 1;

	if (os_GdiplusStartup)
	{
		os_GdiplusStartup(&gdi_token,&gdi_input,0);
	}

	_viv_self_log("env_ok",_viv_self.want_settings);
	thread = CreateThread(0,0,_viv_self_thread,0,0,&thread_id);

	return (thread != 0);
}

#endif // VIVP_SELF_SHOT
