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
// VoidImageViewer
// viv_playlist.h - playlist, everything search, sorting and navigation items.
#ifndef VIV_PLAYLIST_H
#define VIV_PLAYLIST_H

#include "viv.h"

// exported to other domains / the viv.c core
int _viv_icompare_filename(const wchar_t *s1,const wchar_t *s2);
int _viv_fd_compare(const WIN32_FIND_DATA *a,const WIN32_FIND_DATA *b);
int _viv_is_valid_filename(WIN32_FIND_DATA *fd);
const char *_viv_get_copydata_string(const char *p,const char *e,wchar_t *buf,int bufsize);
void _viv_playlist_add_current_if_empty(void);
void _viv_playlist_clearall(void);
void _viv_playlist_delete(const WIN32_FIND_DATA *fd);
void _viv_playlist_rename(const wchar_t *old_filename,const wchar_t *new_filename);
_viv_playlist_t *_viv_playlist_add(const WIN32_FIND_DATA *fd);
void _viv_playlist_add_path(const wchar_t *full_path_and_filename);
void _viv_playlist_add_filename(const wchar_t *filename);
void _viv_nav_item_free_all(void);
void _viv_nav_item_add(WIN32_FIND_DATA *fd);
int _viv_nav_compare(const void *va,const void *vb);
void _viv_search_everything(int add);
int _viv_send_everything_search(HWND hwnd,int add,int randomize,const wchar_t *search);
int _viv_playlist_shuffle_index_from_fd(const WIN32_FIND_DATA *fd);
_viv_playlist_t *_viv_playlist_from_fd(const WIN32_FIND_DATA *fd);
void _viv_do_initial_shuffle(void);
void _viv_send_random_everything_search(void);
int _viv_everything_item_to_fd(const COPYDATASTRUCT *cds,const EVERYTHING_IPC_ITEM2 *item,WIN32_FIND_DATA *fd);

#endif
