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
// utf8 string functions

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

uintptr_t utf8_length_double_null(const utf8_t *text)
{
	const utf8_t *p;
	
	p = text;
	
	for(;;)
	{
		if ((*p == 0) && (p[1] == 0))
		{	
			p++;
			break;
		}
		
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

