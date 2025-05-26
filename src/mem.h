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
// memory allocation

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
