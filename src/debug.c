
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
