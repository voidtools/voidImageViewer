
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
