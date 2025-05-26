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
// renderer abstraction

#if 0

#include "viv.h"

void render_software_blit(HDC dst_hdc,int dst_x,int dst_y,int dst_wide,int dst_high,DWORD *src_pixels,int src_pitch,int src_x,int src_y,int src_wide,int src_high)
{
	if (dst_wide * dst_high)
	{
		DWORD *dst_pixels;
		BITMAPINFOHEADER bi;
		int x;
		int y;
		
		dst_pixels = mem_alloc(dst_wide * dst_high);
		
		os_zero_memory(&bi,sizeof(BITMAPINFOHEADER));
		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biWidth = dst_wide;
		bi.biHeight = -dst_high;
		bi.biPlanes = 1;
		bi.biBitCount = 32;
		bi.biCompression = BI_RGB;
		
		for(y=0;y<dst_high;y++)
		{
			for(x=0;x<dst_wide;x++)
			{
				dst_pixels[x + y * dst_wide] = 0;
			}			
		}
		
		StretchDIBits(dst_hdc,dst_x,dst_y,dst_wide,dst_high,0,0,dst_wide,dst_high,dst_pixels,(BITMAPINFO *)&bi,DIB_RGB_COLORS,SRCCOPY);
	}
}

#endif 
