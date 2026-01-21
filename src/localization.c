//
// Copyright 2026 hesphoros
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
// Language/Localization support implementation
// NOTE: This file must be saved with UTF-8 encoding (with or without BOM)

#if defined(_MSC_VER) && _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "viv.h"
#include "localization_en_us.h"
#include "localization_zh_cn.h"

static const utf8_t **_localization_language_array[LOCALIZATION_LANGUAGE_COUNT] = 
{
	_localization_string_array_en_us,
	_localization_string_array_zh_cn,
};

BYTE _localization_language = LOCALIZATION_LANGUAGE_ENGLISH;

const utf8_t *localization_get_string(localization_id_t localization_id)
{
#ifdef _DEBUG

if ((localization_id < 0) || (localization_id >= LOCALIZATION_ID_COUNT))
{
	debug_fatal("bad localization id %d\n",localization_id);
}

#endif
	
	return _localization_language_array[_localization_language][localization_id];
}

void localization_init(void)
{
	LANGID langid;
	
	// Detect system language
	langid = GetUserDefaultUILanguage();
	
	// Check if it's Chinese (Simplified or Traditional)
	// 0x0804 = Chinese (Simplified, PRC)
	// 0x0404 = Chinese (Traditional, Taiwan)
	// 0x0C04 = Chinese (Traditional, Hong Kong)
	if ((langid == 0x0804) || (langid == 0x0404) || (langid == 0x0C04))
	{
		_localization_language = LOCALIZATION_LANGUAGE_CHINESE_SIMPLIFIED;
	}
	else
	{
		_localization_language = LOCALIZATION_LANGUAGE_ENGLISH;
	}
}
