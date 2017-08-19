
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


#ifdef _DEBUG

	#define mem_alloc(size)			mem_alloc_debug(__FILE__,__LINE__,size)
	#define mem_free(ptr)			mem_free_debug(__FILE__,__LINE__,ptr)

	void *mem_alloc_debug(const char *file,int line,uintptr_t size);
	void mem_free_debug(const char *file,int line,void *ptr);
	void mem_debug(void);

#else

	void *mem_alloc(uintptr_t size);
	void mem_free(void *ptr);

#endif

#ifdef __cplusplus
}
#endif
