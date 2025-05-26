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
// debugging functions

#include "viv.h"

// fatal error, does not return
// gives a UI message to the user and bails.
void DECLSPEC_NORETURN debug_fatal(char *format,...)
{
	wchar_t wbuf[STRING_SIZE];
	va_list args;
	
	va_start(args,format);
	string_vprintf(wbuf,format,args);
	va_end(args);

	MessageBox(0,wbuf,L"voidImageViewer",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
	ExitProcess(0);
}

#ifdef _DEBUG
void debug_printf(char *format,...)
{
	wchar_t wbuf[STRING_SIZE];
	va_list args;
	DWORD number_of_chars_written;
	
	va_start(args,format);
	string_vprintf(wbuf,format,args);
	va_end(args);
	
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE),wbuf,string_length(wbuf),&number_of_chars_written,NULL);
}
#endif							
