
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

#ifdef __cplusplus
extern "C" {
#endif

#define STRING_SIZE	(1024)

uintptr_t string_length(const wchar_t *text);
wchar_t *string_alloc(const wchar_t *s);
wchar_t *string_alloc_utf8(const utf8_t *s);
void string_copy(wchar_t *d,const wchar_t *s);
void string_cat(wchar_t *buf,const wchar_t *s);
int string_compare(const wchar_t *s1,const wchar_t *s2);
int string_istartwith_lowercase_ascii(const wchar_t *s1,const char *lowercase_ascii_s2);
int string_icompare_lowercase_ascii(const wchar_t *s1,const char *lowercase_ascii_s2);
int string_compare_ascii(const wchar_t *s1,const char *ascii_s2);
int string_to_int(wchar_t *s);
wchar_t *string_format_number(wchar_t *buf,__int64 number);
int string_get_appdata_path(wchar_t *buf);
void string_vprintf(wchar_t *wbuf,char *format,va_list argptr);
void string_printf(wchar_t *wbuf,char *format,...);
const wchar_t *string_get_filename_part(const wchar_t *s);
void string_get_path_part(wchar_t *wbuf,const wchar_t *s);
void string_path_combine(wchar_t *wbuf,const wchar_t *path,const wchar_t *filename);
void string_path_combine_utf8(wchar_t *wbuf,const wchar_t *path,const utf8_t *filename);
void string_copy_utf8(wchar_t *buf,const utf8_t *s);
void string_copy_utf8_double_null(wchar_t *buf,const utf8_t *s);
void string_cat_utf8(wchar_t *buf,const utf8_t *s);
void string_get_exe_path(wchar_t *wbuf);
void string_copy_double_null(wchar_t *buf,wchar_t *s);
void string_remove_extension(wchar_t *buf);
wchar_t *string_get_extension(const wchar_t *s);

#ifdef __cplusplus
}
#endif

