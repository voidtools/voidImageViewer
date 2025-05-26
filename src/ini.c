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
// load and sort ini files

#include "viv.h"

// types
// an ini key to load.
typedef struct _ini_keyvalue_s
{
	// next key.
	// this is wasted memory after converting to a binary tree.
	union
	{
		struct _ini_keyvalue_s *next;
		int index;
	};

	// key
	utf8_t *key;
	
	// value
	utf8_t *value;
	
}_ini_keyvalue_t;

// an ini file.
// an ini can not have zero items.
// ini_open returns zero when no items exist.
typedef struct _ini_s
{
	// number of keys
	int count;
	
	// keyvalue array
	_ini_keyvalue_t **indexes;

}_ini_t;

static int _ini_keyvalue_compare(const _ini_keyvalue_t *a,const _ini_keyvalue_t *b);
static _ini_keyvalue_t *_ini_keyvalue_find(ini_t *ini,const utf8_t *key);

ini_t *ini_open(const wchar_t *filename,const utf8_t *ascii_section)
{
	ini_t *ini;
	_ini_keyvalue_t *kv_start;
	_ini_keyvalue_t *kv_last;
	DWORD count;
	HANDLE h;

	kv_start = 0;
	kv_last = 0;
	count = 0;
	ini = 0;
		
	h = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
	if (h != INVALID_HANDLE_VALUE)
	{
		DWORD size;

		size = GetFileSize(h,0);
		
		if (size)
		{
			char *buf;
			DWORD numread;
			
			buf = mem_alloc(size + 1);
			
			if (ReadFile(h,buf,size,&numread,NULL))
			{
				if (numread == size)
				{
					const utf8_t *p;
					int is_section;

					// null terminate.
					buf[size] = 0;
				
					is_section = 0;

					// skip UTF-8 header
					p = utf8_skip_utf8_header(buf);
					
					for(;;)
					{

next_line:
						
						if (*p == '[')
						{
							const utf8_t *ascii_section_p;
							
							// skip the '['
							p++;
							
							is_section = 0;
							ascii_section_p = ascii_section;
							
							for(;;)
							{
								if (!*ascii_section_p)
								{
									if (*p == ']') 
									{
										p++;
										
										is_section = 1;
									}
									
									break;
								}
								
								if (*p == '\n')
								{
									p++;
									goto next_line;
								}

								if ((*p == '\r') && (p[1] == '\n'))
								{
									p+=2;
									goto next_line;
								}			
								
								// compare
								if (*ascii_section_p != *p)
								{
									break;
								}
								
								ascii_section_p++;
								p++;
							}
						}
						else
						if ((*p == '#') || (*p == ';'))
						{
							// ignore this line.
							p++;
						}
						else		
						if (is_section)
						{
							int key_len;
							const utf8_t *key_start;
							
							key_start = p;
							
							while(*p)
							{
								if (*p == '\n')
								{
									p++;
									goto next_line;
								}

								if ((*p == '\r') && (p[1] == '\n'))
								{
									p+=2;
									goto next_line;
								}	
							
								if (*p == '=')
								{
									const utf8_t *value_start;
									int value_len;
									_ini_keyvalue_t *kv;
									
									key_len = (int)(p - key_start);

									// skip =
									p++;
									
									value_start = p;
									
									for(;;)
									{
										if (!*p)
										{
											value_len = (int)(p - value_start);

											break;
										}
										
										if (*p == '\n')
										{
											value_len = (int)(p - value_start);
											
											p++;

											break;
										}

										if ((*p == '\r') && (p[1] == '\n'))
										{
											value_len = (int)(p - value_start);
											
											p += 2;

											break;
										}						
										
										p++;
									}

									// add a key / value pair.
									kv = mem_alloc(sizeof(_ini_keyvalue_t));
									
									// init
									kv->key = utf8_alloc_utf8_n(key_start,key_len);
									kv->value = utf8_alloc_utf8_n(value_start,value_len);

									// insert
									if (kv_start)
									{
										kv_last->next = kv;
									}
									else
									{
										kv_start = kv;
									}
									
									kv->next = 0;
									kv_last = kv;
									count++;
									
									goto next_line;
								}
								
								p++;
							}
						}
						
						// skip to next line
						for(;;)
						{
							if (!*p) 
							{
								goto done;
							}
							
							if (*p == '\n')
							{
								p++;
								
								goto next_line;
							}

							if ((*p == '\r') && (p[1] == '\n'))
							{
								p += 2;
								
								goto next_line;
							}
							
							p++;
						}
					}

				done:	 ;
				}
			}
			
			mem_free(buf);
		}

		CloseHandle(h);
	}
	

	if (count)
	{
		_ini_keyvalue_t **indexes;

		// alloc
		ini = mem_alloc(sizeof(_ini_t));
		
		// allocate indexes		
		indexes = mem_alloc(sizeof(_ini_keyvalue_t *) * count);

		// fill in indexes
		// and conect next to index
		{		
			_ini_keyvalue_t **kv_d;
			_ini_keyvalue_t *kv;
			_ini_keyvalue_t *next_kv;
			int index;

			kv = kv_start;
			kv_d = indexes;
			index = 0;
			
			while(kv)
			{
				next_kv = kv->next;
				
				kv->index = index;

				*kv_d++ = kv;
				index++;
				
				kv = next_kv;
			}
		}
		
		// sort indexes
		os_qsort(indexes,count,_ini_keyvalue_compare);
		
		// find and remove dupes
		{		
			_ini_keyvalue_t **kv_d;
			_ini_keyvalue_t **kv_e;
			_ini_keyvalue_t **kv_p;
			_ini_keyvalue_t *last;
			
			kv_d = indexes;
			kv_e = indexes + count;
			kv_p = indexes;
			last = 0;

			while(kv_p != kv_e)
			{
				if (last)
				{
					if (utf8_compare(last->key,(*kv_p)->key) == 0)
					{
						// free last
						mem_free(last->key);
						mem_free(last->value);
						mem_free(last);

						// replace last
						kv_d--;						
					}
				}

				*kv_d++ = *kv_p;
				last = *kv_p;
				kv_p++;
			}
			
			// calculate new count
			// count can not be zero.
			count = (uintptr_t)(kv_d - indexes);
		}
		
		// init ini
		ini->indexes = indexes;
		ini->count = count;
	}
	
	return ini;
}

void ini_close(ini_t *ini)
{
	// free
	{
		_ini_keyvalue_t **kv_p;
		_ini_keyvalue_t **kv_e;
		
		kv_p = ini->indexes;
		kv_e = ini->indexes + ini->count;
		
		while(kv_p != kv_e)
		{
			mem_free((*kv_p)->key);
			mem_free((*kv_p)->value);
			mem_free(*kv_p);
		
			kv_p++;
		}
	}	
	
	mem_free(ini->indexes);
	mem_free(ini);
}

static _ini_keyvalue_t *_ini_keyvalue_find(ini_t *ini,const utf8_t *key)
{
	int blo;
	int bhi;
	int bpos;
	int i;
	
	blo = 0;
	bhi = ini->count - 1;
	
	while(blo <= bhi)
	{
		_ini_keyvalue_t *kv;
		
		bpos = blo + ((bhi - blo) / 2);
		
		kv = ini->indexes[bpos];
		
		i = utf8_compare(key,kv->key);
		if (i < 0)
		{
			bhi = bpos - 1;
		}
		else
		if (!i)
		{
			return kv;
		}
		else
		{
			blo = bpos + 1;
		}
	}
	
	// not found
	return 0;
}

// compare two key values.
static int _ini_keyvalue_compare(const _ini_keyvalue_t *a,const _ini_keyvalue_t *b)
{
	int i;

	i = utf8_compare(a->key,b->key);
	if (i) return i;
	
	return a->index - b->index;
}

void ini_get_int(ini_t *ini,const utf8_t *key,int *pint)
{
	_ini_keyvalue_t *kv;
	
	// lookup key.
	kv = _ini_keyvalue_find(ini,key);
	if (kv)
	{
		*pint = utf8_to_int(kv->value);
	}
}

const utf8_t *ini_get_string(ini_t *ini,const utf8_t *key)
{
	_ini_keyvalue_t *kv;
	
	// lookup key.
	kv = _ini_keyvalue_find(ini,key);
	if (kv)
	{
		return kv->value;
	}
	
	return 0;
}
