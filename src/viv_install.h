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
// viv_install.h - install/uninstall, associations, registry, shortcuts, elevation.
#ifndef VIV_INSTALL_H
#define VIV_INSTALL_H

#include "viv.h"

// exported to other domains / the viv.c core
int _viv_process_install_command_line_options(wchar_t *cl);
void _viv_install_association_by_extension(const char *association,const char *description,const char *icon_location);
void _viv_uninstall_association_by_extension(const char *association);
int _viv_is_association(const char *association);
int _viv_is_start_menu_shortcuts(void);
void _viv_append_admin_param(wchar_t *wbuf,const utf8_t *param);
void _viv_get_exe_filename(wchar_t filename[STRING_SIZE]);

#endif
