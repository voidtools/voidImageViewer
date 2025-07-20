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
// fixed sized wchar string functions

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
	
	size = STRING_SIZE - 1;
	
	while(*s)
	{
		if (!size) 
		{
			break;
		}
		
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

void string_cat_path_separator(wchar_t *buf)
{
	uintptr_t len;
	
	len = string_length(buf);
	
	if ((len) && (buf[len-1] == '\\'))
	{
		return;
	}
	
	string_cat(buf,L"\\");
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
				case 'u':
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
			
				case 'p':
				{
					uintptr_t num;
					wchar_t numbuf[64];
					wchar_t *s;

					num = va_arg(argptr,uintptr_t);

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

wchar_t *string_format_number(wchar_t *buf,__int64 number)
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

// buf must have size STRING_SIZE + 1
void string_copy_double_null(wchar_t *buf,wchar_t *s)
{
	wchar_t *d;
	wchar_t *p;
	
	d = buf;
	p = s;
	
	while(*p)
	{
		*d++ = *p++;
	}

	*d++ = 0;
	*d = 0;	
}

// assumes buf is a filename only (no path)
void string_remove_extension(wchar_t *buf)
{
	wchar_t *p;
	wchar_t *last;
	
	last = 0;
	
	p = buf;
	while(*p)
	{
		if (*p == '.')
		{
			last = p;
		}
		
		p++;
	}
	
	if (last)
	{
		*last = 0;
	}
}

// does not include the dot.
// returns an empty string if no extension.
wchar_t *string_get_extension(const wchar_t *s)
{
	const wchar_t *p;
	const wchar_t *last;
	
	last = L"";
	
	p = s;
	while(*p)
	{
		if (*p == '.')
		{
			last = p + 1;
		}
		
		p++;
	}

	return (wchar_t *)last;	
}

wchar_t *string_skip_ws(const wchar_t *p)
{
	while(*p)
	{
		if (!wchar_is_ws(*p))
		{
			break;
		}
		
		p++;
	}
	
	return (wchar_t *)p;
}

wchar_t *string_get_word(wchar_t *p,wchar_t *buf)
{
	wchar_t *d;
	int is_quote;
	
	d = buf;
	is_quote = 0;

	while(*p)
	{
		if ((*p == '"') && (p[1] == '"'))
		{
			p += 2;
			*d++ = '"';
		}
		else
		if (*p == '"')
		{
			is_quote = !is_quote;
			p++;
		}
		else
		if ((!is_quote) && (wchar_is_ws(*p)))
		{
			break;
		}
		else
		{
			*d++ = *p;
			p++;
		}
	}
	
	*d = 0;
	
	return p;
}

int string_get_appdata_voidimageviewer_path(wchar_t *wbuf)
{
	wchar_t appdata_wbuf[STRING_SIZE];
	
	if (string_get_appdata_path(appdata_wbuf))
	{
		string_path_combine_utf8(wbuf,appdata_wbuf,(const utf8_t *)"voidImageViewer");

		return 1;
	}
	
	return 0;
}

