// dpi probe test tool: asks every dpi api what it reports under wine.
#include <windows.h>
#include <stdio.h>

typedef UINT (WINAPI *fn_GetDpiForWindow)(HWND);
typedef HANDLE (WINAPI *fn_SetThreadDpiAwarenessContext)(HANDLE);

int main(void)
{
	HDC dc;
	fn_GetDpiForWindow pGet;
	fn_SetThreadDpiAwarenessContext pSet;
	HMODULE user32;

	dc = GetDC(0);
	printf("GetDeviceCaps LOGPIXELSX=%d Y=%d\n", GetDeviceCaps(dc,LOGPIXELSX), GetDeviceCaps(dc,LOGPIXELSY));
	printf("GetDeviceCaps HORZSIZE=%dmm VERTSIZE=%dmm\n", GetDeviceCaps(dc,HORZSIZE), GetDeviceCaps(dc,VERTSIZE));
	ReleaseDC(0,dc);

	user32 = GetModuleHandleA("user32.dll");
	pGet = (fn_GetDpiForWindow)(void *)GetProcAddress(user32,"GetDpiForWindow");
	pSet = (fn_SetThreadDpiAwarenessContext)(void *)GetProcAddress(user32,"SetThreadDpiAwarenessContext");

	if (pGet)
	{
		printf("GetDpiForWindow(desktop)=%u (unaware context)\n", pGet(GetDesktopWindow()));
	}
	else
	{
		printf("GetDpiForWindow: MISSING\n");
	}

	if (pSet)
	{
		pSet((HANDLE)-4);
		printf("claimed PerMonitorV2\n");
		if (pGet)
		{
			printf("GetDpiForWindow(desktop)=%u (PMv2 context)\n", pGet(GetDesktopWindow()));
		}
	}
	else
	{
		printf("SetThreadDpiAwarenessContext: MISSING\n");
	}

	{
		char env[64];
		DWORD n = GetEnvironmentVariableA("VIVP_TEST",env,sizeof(env));
		printf("VIVP_TEST=%.*s\n", n>63?63:(int)n, env);
	}
	fflush(stdout);
	return 0;
}
