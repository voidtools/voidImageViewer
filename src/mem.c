
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

#ifdef _DEBUG

#define MEM_MAGIC_SIZE 64

uintptr_t mem_usage = 0;

typedef struct mem_debug_s
{
	struct mem_debug_s *next;
	struct mem_debug_s *prev;
	uintptr_t size;
	const char *filename;
	int line;
	DWORD align[3];
	void *magic[MEM_MAGIC_SIZE];
	
}mem_debug_t; 

mem_debug_t *mem_debug_start=0;
mem_debug_t *mem_debug_last=0;
int mem_debug_initialized=0;
int mem_debug_count=0;
CRITICAL_SECTION mem_debug_cs;

int get_mem_usage(void)
{
	return (int)mem_usage;
}

void mem_debug(void)
{
	mem_debug_t *start;
	
	// note: debug_printf can change the state of the memory.
	
	debug_printf("MEM_DEBUG_START (%d)\n",mem_debug_count);


	EnterCriticalSection(&mem_debug_cs);
	
	// make a copy of memory debugs
	{
		mem_debug_t *last;
		mem_debug_t *md;
		mem_debug_t *new_md;

		start = 0;
		last = 0;
		md = mem_debug_start;
		
		while(md)
		{
			new_md = HeapAlloc(GetProcessHeap(),0,sizeof(mem_debug_t));
			
			os_copy_memory(new_md,md,sizeof(mem_debug_t));
			
			// insert
			if (start)
			{
				last->next = new_md;
			}
			else
			{
				start = new_md;
			}
			
			last = new_md;
			new_md->next = 0;
			
			md = md->next;
		}
	}
	
	LeaveCriticalSection(&mem_debug_cs);
	
	{
		mem_debug_t *md;
		mem_debug_t *next_md;
		int count;
		uintptr_t size;
		int heapsize;
		
		count = 0;
		size = 0;
		heapsize = 0;

		md = start;
		while(md)
		{
			next_md = md->next;

			debug_printf("%s(%d): %p: %d bytes\n",md->filename,md->line,md+1,md->size);

			size += md->size;
			
			count++;	
			
			HeapFree(GetProcessHeap(),0,md);
		
			md = next_md;
		}

		debug_printf("Allocation count: %d\n",count);
		debug_printf("Allocation size: %d\n",size);
		debug_printf("Allocation heap size: %d\n",(int)mem_usage);
		debug_printf("heap overhead: %d\n",(int)(mem_usage - size));
		
		
//DEBUG:	
// catch leaks.	
		if (count) 
		{
			DebugBreak();
		}
	}
	
	debug_printf("MEM_DEBUG_END\n");
}


// allocate
void *mem_alloc_debug(const char *file,int line,uintptr_t size)
{
	mem_debug_t *p;
	
	if (!mem_debug_initialized)
	{
		InitializeCriticalSection(&mem_debug_cs);
		mem_debug_initialized = 1;
	}
	
	p = HeapAlloc(GetProcessHeap(),0,sizeof(mem_debug_t) + size + (sizeof(void *) * MEM_MAGIC_SIZE));

	// this is fatal
	if (!p) 
	{
		// enter critical section to block any other threads from running out of memory and throwing another error.
		EnterCriticalSection(&mem_debug_cs);
		
		MessageBoxA(0,"Out of Memory","Everything",MB_SYSTEMMODAL|MB_OK|MB_ICONERROR);

		DebugBreak();
		
		debug_fatal("out of memory");

		LeaveCriticalSection(&mem_debug_cs);
	}

	EnterCriticalSection(&mem_debug_cs);
	
	mem_usage += HeapSize(GetProcessHeap(),0,p);
	
	// setup mem_debug_t
	p->size = size;
	p->filename = file;
	p->line = line;
	
	if (mem_debug_start)
	{
		mem_debug_last->next = p;
		p->prev = mem_debug_last;
	}
	else
	{
		mem_debug_start = p;
		p->prev = 0;
	}
	
	mem_debug_last = p;
	p->next = 0;
	mem_debug_count++;
	
	{
		int magici;
		for(magici=0;magici<MEM_MAGIC_SIZE;magici++)
		{
			p->magic[magici] = (p+1);
		}
	}

	{
		int magici;
		for(magici=0;magici<MEM_MAGIC_SIZE;magici++)
		((void **)(((char *)p) + sizeof(mem_debug_t) + size))[magici] = (p+1);
	}

	LeaveCriticalSection(&mem_debug_cs);

	return (void *)(p + 1);
}

void mem_cc(void *p,int size)
{
	unsigned char *d;
	
	d = p;
	while(size)
	{
		*d++ = 0xcc;
		size--;
	}
}

// free
void mem_free_debug(const char *file,int line,void *p)
{
	if (!mem_debug_initialized)
	{
		InitializeCriticalSection(&mem_debug_cs);
		mem_debug_initialized = 1;
	}
	
	mem_usage -= HeapSize(GetProcessHeap(),0,(((mem_debug_t *)p)-1));

	if (!p)
	{
		debug_fatal("INVALID FREE from %s(%d): %p",file,line,p);
	}
	
	{
		int magici;
		for(magici=0;magici<MEM_MAGIC_SIZE;magici++)
		{
			if ((((mem_debug_t *)p)-1)->magic[magici] != p)
			{
				debug_fatal("INVALID FREE from %s(%d): %p bad magic 1 %u: %p",file,line,magici,p,(((mem_debug_t *)p)-1)->magic[magici]);
			}
		}
	}
	
	{
		int magici;
		for(magici=0;magici<MEM_MAGIC_SIZE;magici++)
		{
			if (((void **)(((char *)p) + (((mem_debug_t *)p)-1)->size))[magici] != p)
			{
				debug_fatal("INVALID FREE from %s(%d): %p bad magic 2 %u: %p",file,line,p,magici,((void **)(((char *)p) + (((mem_debug_t *)p)-1)->size))[magici]);
			}
		}
	}
	
	{
		mem_cc((((mem_debug_t *)p)-1)->magic,MEM_MAGIC_SIZE * sizeof(void *));
		mem_cc(((void **)(((char *)p) + (((mem_debug_t *)p)-1)->size)),MEM_MAGIC_SIZE * sizeof(void *));
	}

	EnterCriticalSection(&mem_debug_cs);

	// unlink
	if ((((mem_debug_t *)p)-1) == mem_debug_start)
	{
		mem_debug_start = (((mem_debug_t *)p)-1)->next;
	}
	else
	{
		(((mem_debug_t *)p)-1)->prev->next = (((mem_debug_t *)p)-1)->next;
	}

	if ((((mem_debug_t *)p)-1) == mem_debug_last)
	{
		mem_debug_last = (((mem_debug_t *)p)-1)->prev;
	}
	else
	{
		(((mem_debug_t *)p)-1)->next->prev = (((mem_debug_t *)p)-1)->prev;
	}
	
	mem_debug_count--;

	LeaveCriticalSection(&mem_debug_cs);

	mem_cc(p,(((mem_debug_t *)p)-1)->size);
	
	// release
	if (!HeapFree(GetProcessHeap(),0,(((mem_debug_t *)p)-1)))
	{
		// not fatal
		debug_printf("HeapFree(): %d: Failed to free %p.\n",GetLastError(),p);
	}
}


#else

void *mem_alloc(uintptr_t size)
{
	void *p;
	
	p = HeapAlloc(GetProcessHeap(),0,size);
	
	if (!p)
	{
		debug_fatal("alloc %d",size);
	}
	
	return p;
}

void mem_free(void *ptr)
{
	HeapFree(GetProcessHeap(),0,ptr);
}

#endif
