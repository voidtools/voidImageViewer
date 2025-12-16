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

#ifdef __cplusplus
extern "C" {
#endif

#define STRING_SIZE	(1024)

uintptr_t string_length(const wchar_t *text);
wchar_t *string_alloc(const wchar_t *s);
wchar_t *string_alloc_utf8(const utf8_t *s);
void string_copy(wchar_t *d,const wchar_t *s);
void string_copy_with_bufsize(wchar_t *d,SIZE_T bufsize,const wchar_t *s);
void string_cat(wchar_t *buf,const wchar_t *s);
int string_compare(const wchar_t *s1,const wchar_t *s2);
int string_istartwith_lowercase_ascii(const wchar_t *s1,const char *lowercase_ascii_s2);
int string_icompare_lowercase_ascii(const wchar_t *s1,const char *lowercase_ascii_s2);
int string_compare_ascii(const wchar_t *s1,const char *ascii_s2);
int string_to_int(wchar_t *s);
wchar_t *string_format_number(wchar_t *buf,__int64 number);
int string_get_appdata_path(wchar_t *buf);
void string_vprintf(wchar_t *wbuf,const char *format,va_list argptr);
void string_printf(wchar_t *wbuf,const char *format,...);
const wchar_t *string_get_filename_part(const wchar_t *s);
void string_get_path_part(wchar_t *wbuf,const wchar_t *s);
void string_path_combine(wchar_t *wbuf,const wchar_t *path,const wchar_t *filename);
void string_path_combine_utf8(wchar_t *wbuf,const wchar_t *path,const utf8_t *filename);
void string_copy_utf8(wchar_t *buf,const utf8_t *s);
void string_copy_utf8_double_null(wchar_t *buf,const utf8_t *s);
void string_cat_utf8(wchar_t *buf,const utf8_t *s);
void string_cat_path_separator(wchar_t *buf);
void string_get_exe_path(wchar_t *wbuf);
void string_copy_double_null(wchar_t *buf,wchar_t *s);
void string_remove_extension(wchar_t *buf);
wchar_t *string_get_extension(const wchar_t *s);
wchar_t *string_skip_ws(const wchar_t *p);
wchar_t *string_get_word(wchar_t *p,wchar_t *buf);
int string_get_appdata_voidimageviewer_path(wchar_t *wbuf);
BOOL string_is_dot(const wchar_t *s);

#ifdef __cplusplus
}
#endif

