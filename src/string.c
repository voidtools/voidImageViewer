
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

uintptr_t string_length(const wchar_t *text)
{
	const wchar_t *p;
	
	p = text;
	
	while(*p)
	{
		p++;
	}
	
	return (uintptr_t)(p - text);
}

wchar_t *string_alloc(const wchar_t *s)
{
	uintptr_t wlen;
	wchar_t *p;
	
	wlen = string_length(s);
	p = (wchar_t *)mem_alloc((wlen + 1) * sizeof(wchar_t));
	
	CopyMemory(p,s,(wlen + 1) * sizeof(wchar_t));
		
	return p;
}

wchar_t *string_alloc_utf8(const utf8_t *s)
{
	uintptr_t wlen;
	wchar_t *p;
	
	wlen = MultiByteToWideChar(CP_UTF8,0,s,-1,0,0);
	
	p = (wchar_t *)mem_alloc((wlen + 1) * sizeof(wchar_t));
	
	MultiByteToWideChar(CP_UTF8,0,s,-1,p,wlen+1);
		
	return p;
}

void string_copy(wchar_t *d,const wchar_t *s)
{
	uintptr_t size;
	
	size = STRING_SIZE;
	
	size--;
	
	while(*s)
	{
		if (!size) break;
		
		*d++ = *s++;
	}
	
	*d = 0;
}

void string_copy_utf8(wchar_t *buf,const utf8_t *s)
{
	MultiByteToWideChar(CP_UTF8,0,s,-1,buf,STRING_SIZE);
}

void string_copy_utf8_double_null(wchar_t *buf,const utf8_t *s)
{
	int slen;
	
	slen = utf8_length_double_null(s);
	
	MultiByteToWideChar(CP_UTF8,0,s,slen+1,buf,STRING_SIZE);
}

void string_cat(wchar_t *buf,const wchar_t *s)
{
	wchar_t *d;
	uintptr_t size;
	
	size = STRING_SIZE;
	
	size--;
	
	d = buf;
	while(*d)
	{
		d++;
		size--;
	}
	
	while(*s)
	{
		if (!size) break;
		
		*d++ = *s;
		s++;
	}
	
	*d = 0;
}

void string_cat_utf8(wchar_t *buf,const utf8_t *s)
{
	wchar_t s_wbuf[STRING_SIZE];
	
	string_copy_utf8(s_wbuf,s);
	
	string_cat(buf,s_wbuf);
}

int string_compare(const wchar_t *s1,const wchar_t *s2)
{
	const wchar_t *p1;
	const wchar_t *p2;
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

int string_istartwith_lowercase_ascii(const wchar_t *s1,const char *lowercase_ascii_s2)
{
	const wchar_t *p1;
	const char *p2;
	
	p1 = s1;
	p2 = lowercase_ascii_s2;
	
	while(*p2)
	{
		if (wchar_lowercase_ascii(*p2) != *p1)
		{
			return 0;
		}
		
		p1++;
		p2++;
	}
	
	return 1;
}

int string_icompare_lowercase_ascii(const wchar_t *s1,const char *lowercase_ascii_s2)
{
	const wchar_t *p1;
	const char *p2;
	int c;
	
	p1 = s1;
	p2 = lowercase_ascii_s2;
	
	for(;;)
	{
		c = wchar_lowercase_ascii(*p1) - *p2;
		if (c) return c;
		
		if (!*p1) break;
		if (!*p2) break;
		
		p1++;
		p2++;
	}
	
	return 0;
}


int string_compare_ascii(const wchar_t *s1,const char *ascii_s2)
{
	const wchar_t *p1;
	const char *p2;
	int c;
	
	p1 = s1;
	p2 = ascii_s2;
	
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

int string_to_int(wchar_t *s)
{
	int i;
	int sign;
	
	sign = 1;
	i = 0;
	
	if (*s == '-')
	{
		sign = -1;
		s++;
	}
	
	while(*s)
	{
		if ((*s >= '0') && (*s <= '9'))
		{
			i *= 10;
			i += *s - '0';
		}
		
		s++;
	}
	
	return i * sign;
}

// %s = utf8, %S = wchar, %f == 0.3f
void string_vprintf(wchar_t *wbuf,char *format,va_list argptr)
{
	char *fp;
	wchar_t *d;
	wchar_t *e;
	
	fp = format;
	d = wbuf;
	e = wbuf + STRING_SIZE - 1;
	
	while(*fp)
	{
		if (*fp == '%')
		{
			int leading_zero;
			//int dp;
			
			fp++;
			
			leading_zero = 0;
			//dp = 6;
			
			if (*fp == '0')
			{
				leading_zero = 1;
				fp++;
			}
			
			if (*fp == '.')
			{
				fp++;
				if ((*fp >= '0') && (*fp <= '9'))
				{
			//		dp = *fp - '0';
					fp++;
				}
			}
			
			switch(*fp)
			{
				case '%':
					if (d < e)
					{
						*d++ = '%';
					}
					break;

				case 'd':
				{
					int num;
					wchar_t numbuf[64];
					wchar_t *s;

					num = va_arg(argptr,int);

					string_format_number(numbuf,num);
					
					s = numbuf;
					
					while(*s)
					{
						if (d < e)
						{
							*d++ = *s;
						}
						
						s++;
					}
					
					break;
				}

				case 'f':
				{
					int num;
					wchar_t numbuf[64];
					wchar_t *s;
					uintptr_t lz;

					num = (int)(va_arg(argptr,double) * 1000.0f);

					string_format_number(numbuf,num / 1000);
					
					s = numbuf;
					
					while(*s)
					{
						if (d < e)
						{
							*d++ = *s;
						}
						
						s++;
					}

					if (d < e)
					{
						*d++ = '.';
					}

					string_format_number(numbuf,num % 1000);
					
					if (string_length(numbuf) <= 3)
					{
						for(lz=0;lz<3-string_length(numbuf);lz++)
						{
							if (d < e)
							{
								*d++ = '0';
							}
						}
					}

					s = numbuf;
					
					while(*s)
					{
						if (d < e)
						{
							*d++ = *s;
						}
						
						s++;
					}

					break;
				}

				case 's':
				{
					char *s;
					
					s = va_arg(argptr,char *);
					
					while(*s)
					{
						if (d < e)
						{
							*d++ = *s;
						}
						
						s++;
					}
					
					break;
				}

				case 'S':
				{
					wchar_t *s;
					
					s = va_arg(argptr,wchar_t *);
					
					while(*s)
					{
						if (d < e)
						{
							*d++ = *s;
						}
						
						s++;
					}
					
					break;
				}
			}
		}
		else
		{
			if (d < e)
			{
				*d++ = *fp;
			}
		}
		
		fp++;
	}
	
	*d = 0;
}

void string_printf(wchar_t *wbuf,char *format,...)
{
	va_list args;
	
	va_start(args,format);
	string_vprintf(wbuf,format,args);
	va_end(args);
}

wchar_t *string_format_number(wchar_t *buf,int number)
{
	wchar_t backbuf[MAX_PATH];
	wchar_t *n;
	wchar_t *d;
	
	d = buf;
	
	if (number < 0)
	{
		number = -number;
		*d++ = '-';
	}

	if (!number)
	{
		*d++ = '0';
		*d = 0;
		return d;
	}
	
	n = backbuf;
	
	while(number)
	{
		*n++ = (number % 10) + '0';
		number /= 10;
	}

	while(n > backbuf)
	{
		n--;
		
		*d = *n;
		
		d++;
	}
	*d = 0;
	
	return d;
}

int string_get_appdata_path(wchar_t *buf)
{
	LPITEMIDLIST pidl;
	int ret;
	
	ret = 0;

	if (SUCCEEDED(SHGetSpecialFolderLocation(0,CSIDL_APPDATA,&pidl)))
	{
		if (SHGetPathFromIDList(pidl,buf))
		{
			ret = 1;
		}

		CoTaskMemFree(pidl);
	}
	
	return ret;
}

const wchar_t *string_get_filename_part(const wchar_t *s)
{
	const wchar_t *p;
	const wchar_t *last;
	
	p = s;
	last = s;
	
	while(*p)	
	{
		if (*p == '\\')
		{
			last = p + 1;
		}
		
		p++;
	}
	
	return last;
}

void string_path_combine_utf8(wchar_t *wbuf,const wchar_t *path,const utf8_t *filename)
{
	wchar_t subpath[STRING_SIZE];
	
	string_copy_utf8(subpath,filename);
	
	string_path_combine(wbuf,path,subpath);
}

// if path is null the current directory is used.
void string_path_combine(wchar_t *wbuf,const wchar_t *path,const wchar_t *filename)
{
	PathCombine(wbuf,path,filename);
}

void string_get_path_part(wchar_t *wbuf,const wchar_t *s)
{
	const wchar_t *p;
	const wchar_t *last;
	
	p = s;
	last = s;
	
	while(*p)	
	{
		if (*p == '\\')
		{
			last = p;
		}
		
		p++;
	}
	
	os_copy_memory(wbuf,s,(char *)last - (char *)s);
	wbuf[last - s] = 0;
}

void string_get_exe_path(wchar_t *wbuf)
{
	wchar_t exe_filename[STRING_SIZE];

	GetModuleFileNameW(0,exe_filename,STRING_SIZE);
	string_get_path_part(wbuf,exe_filename);
}
