
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

uintptr_t utf8_length(const utf8_t *text)
{
	const utf8_t *p;
	
	p = text;
	
	while(*p)
	{
		p++;
	}
	
	return (uintptr_t)(p - text);
}

int utf8_to_int(const utf8_t *str)
{
	int i;
	int sign;
	register const utf8_t *s;
	
	s = str;
	
	i = 0;
	
	// check for hex.
	// hex can not have negative numbers
	if (*s == '0')
	{
		if ((s[1] == 'x') || (s[1] == 'X'))
		{
			// hex.
			s += 2;
			
			while (*s)
			{
				if ((*s >= '0') && (*s <= '9'))
				{
					i <<= 4;
					i += *s - '0';
				}
				else
				if ((*s >= 'A') && (*s <= 'F'))
				{
					i <<= 4;
					i += *s - 'A' + 10;
				}
				else
				if ((*s >= 'a') && (*s <= 'f'))
				{
					i <<= 4;
					i += *s - 'a' + 10;
				}
				else
				{
					break;
				}
				
				s++;
			}
			
			return i;
		}
	}
	
	// check for '-'
	if (*s == '-')
	{
		sign = -1;
		s++;
	}
	else
	{
		sign = 1;
	}
	
	// dec
	while (*s)
	{
		if ((*s >= '0') && (*s <= '9'))
		{
			i *= 10;
			i += *s - '0';
		}
		else
		{
			break;
		}
		
		s++;
	}
	
	return i * sign;
}

utf8_t *utf8_skip_utf8_header(const utf8_t *p)
{
	// skip UTF-8 header
	if ((p[0] == 0xEF) && (p[1] == 0xBB) && (p[2] == 0xBF)) 
	{
		return (utf8_t *)(p + 3);
	}
	
	return (utf8_t *)p;
}

utf8_t *utf8_alloc_utf8_n(const char *s,uintptr_t size_in_bytes)
{
	utf8_t *p;
	
	p = mem_alloc(size_in_bytes + 1);
	
	os_copy_memory(p,s,size_in_bytes);
	p[size_in_bytes] = 0;
	
	return p;
}

int utf8_compare(const utf8_t *s1,const utf8_t *s2)
{
	const utf8_t *p1;
	const utf8_t *p2;
	int c;
	
	p1 = s1;
	p2 = s2;
	
	for(;;)
	{
		c = *p1 - *p2;
		if (c) return c;
		
		if (!*p1) break;
		if (!*p2) break;
		
		p1++;
		p2++;
	}
	
	return 0;
}

