
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
// renderer abstraction

#ifdef __cplusplus
extern "C" {
#endif

typedef struct render_s
{
	void *user_data;
	
	// GDIPlus will render the imgae onto this hbitmap
	HBITMAP (*create_hbitmap)(render_t *renderer,int wide,int high)
	
	// convert this bitmap to a texture.
	void *(*get_texture)(render_t *renderer,HBITMAP hbitmap);
	void (*destroy_texture)(void *texture);
		
	void (*render)(render_t *renderer,void *texture,HWND hwnd,int x,int y,int wide,int high);

	void (*init)(render_t *renderer);
	void (*kill)(render_t *renderer);
	
}render_t;

#ifdef __cplusplus
}
#endif
