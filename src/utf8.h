
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

uintptr_t utf8_length(const utf8_t *text);
int utf8_to_int(const utf8_t *str);
utf8_t *utf8_skip_utf8_header(const utf8_t *p);
utf8_t *utf8_alloc_utf8_n(const char *s,uintptr_t size_in_bytes);
int utf8_compare(const utf8_t *s1,const utf8_t *s2);

#ifdef __cplusplus
}
#endif

