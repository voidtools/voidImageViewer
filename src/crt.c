
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

#ifdef VERSION_X64

// avoid crt linkage.
int _fltused=0; 

/*
// don't use intrisics
#pragma function(memcpy)
#pragma function(memset)

void *__cdecl memcpy(void *dest,const void *src,uintptr_t count)
{
	unsigned char *d;
	const unsigned char *s;
	
	d = dest;
	s = src;
		
	{	
		uintptr_t run;

		run = count / sizeof(uintptr_t);
		
		while(run)
		{
			*(uintptr_t *)d = *(uintptr_t *)s;
			
			d += sizeof(uintptr_t);
			s += sizeof(uintptr_t);
			
			run--;
		}
	}

	{	
		uintptr_t run;

		run = count & (sizeof(uintptr_t) - 1);
		
		while(run)
		{
			*d++ = *s++;
			
			run--;
		}
	}
	
	return dest;
}

// review:
void * __cdecl memset(void *dest,int c,uintptr_t count)
{
	if (c == 0)
	{
		unsigned char *d;
		
		d = dest;
			
		{	
			uintptr_t run;

			run = count / sizeof(uintptr_t);
			
			while(run)
			{
				*(uintptr_t *)d = 0;
				
				d += sizeof(uintptr_t);
				
				run--;
			}
		}

		{	
			uintptr_t run;

			run = count & (sizeof(uintptr_t) - 1);
			
			while(run)
			{
				*d++ = 0;
				
				run--;
			}
		}
	}
	else
	{
		unsigned char *d;
		uintptr_t run;
		
		run = count;
		d = dest;
	
		while(run)
		{
			*d++ = c;
			
			run--;
		}		
	}
	
	return dest;
}

void __cdecl WinMainCRTStartup() 
{
	int mainret;
	STARTUPINFO si;
	
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);

	mainret = WinMain(GetModuleHandle(0),0,0,si.wShowWindow);

    ExitProcess(mainret);
}
*/
#endif // VERSION_X64
