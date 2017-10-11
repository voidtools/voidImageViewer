
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
//
// GDI renderer

#ifdef __cplusplus
extern "C" {
#endif

void render_software_blit(HDC dest_hdc,int dest_x,int dest_y,int dest_wide,int dest_high,HBITMAP src_hbitmap,int src_x,int src_y,int src_wide,int src_high);

#ifdef __cplusplus
}
#endif
