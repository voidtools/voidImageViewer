
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

// compiler options
#pragma warning(disable : 4311) // type cast void * to unsigned int
#pragma warning(disable : 4312) // type cast unsigned int to void *
#pragma warning(disable : 4244) // warning C4244: 'argument' : conversion from 'LONG_PTR' to 'LONG', possible loss of data
#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable : 4313) // 'debug_printf' : '%x' in format string conflicts with argument 2 of type 'line_t *'
#pragma warning(disable : 4996) // deprecation
#pragma warning(disable : 4701) // potentially uninitialized local variable 'last_stretch_mode' used

// REQUIRES IE 5.01
#define _WIN32_IE 0x0501

// WINNT required for some header definitions.
// minimum is really 0x0400
#define _WIN32_WINNT 0x0501

#define WIN32_LEAN_AND_MEAN

#define COBJMACROS // c object interface please
#define CINTERFACE // c interface only

typedef unsigned char utf8_t;

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
//#include <gdiplus.h>
#include "../res/resource.h"
//#include <stdio.h>
//#include <math.h>
//#include <shlobj.h>
//#include <istream>
#include <commdlg.h> // OPENFILENAME
#include <Shellapi.h> // ShellExecute
#include <uxtheme.h>
#include <process.h> // _beginthreadex
#include <shlobj.h> // DROPFILES

#include "version.h"
#include "debug.h"
#include "mem.h"
#include "os.h"
#include "crt.h"
#include "wchar.h"
#include "string.h"
#include "utf8.h"
#include "ini.h"

#include "everything_ipc.h"

#ifdef __cplusplus
}
#endif
