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
// a temporary allocator for data
// initial chunk is stored on the stack.
// aka a bump allocator

#ifdef __cplusplus
extern "C" {
#endif

// MUST be a power of 2.
#define SMALL_POOL_STACK_SIZE			256
#define SMALL_POOL_MAX_CHUNK_SIZE		65536

#define SMALL_POOL_CHUNK_DATA(chunk)	((BYTE *)(((small_pool_chunk_t *)(chunk)) + 1))

typedef struct small_pool_chunk_s
{
	union
	{
		struct small_pool_chunk_s *next;
		
		// enforce 8-byte alignment.
		QWORD padding1;
	};
	
}small_pool_chunk_t;

typedef struct small_pool_s
{
	BYTE *p;
	uintptr_t avail;
	small_pool_chunk_t *chunk_start;
	uintptr_t cur_alloc_size;

	// stack MUST be 8 byte aligned.
	BYTE stack[SMALL_POOL_STACK_SIZE];
	
}small_pool_t;

void small_pool_init(small_pool_t *buf);
void small_pool_empty(small_pool_t *buf);
void small_pool_kill(small_pool_t *buf);
void *small_pool_alloc(small_pool_t *buf,uintptr_t size);

#ifdef __cplusplus
}
#endif
