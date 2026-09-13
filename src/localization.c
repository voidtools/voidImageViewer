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

#include "viv.h"
#include "localization_en_us.h"
#include "localization_zh_cn.h"

static const utf8_t **_localization_language_array[LOCALIZATION_LANGUAGE_COUNT] = 
{
	_localization_string_array_en_us, // LOCALIZATION_LANGUAGE_ENGLISH
	_localization_string_array_zh_cn, // LOCALIZATION_LANGUAGE_CHINESE_SIMPLIFIED
};

BYTE localization_language = LOCALIZATION_LANGUAGE_ENGLISH;

const utf8_t *localization_get_string(localization_id_t localization_id)
{
	// a bad id used to be an out-of-bounds read in release builds: the
	// range check was debug only. clamp to the english fallback so a
	// missed translation index degrades instead of crashing.
	if ((localization_id < 0) || (localization_id >= LOCALIZATION_ID_COUNT))
	{
#ifdef _DEBUG
		debug_fatal("bad localization id %d\n",localization_id);
#endif
		return _localization_string_array_en_us[0];
	}
	
	return _localization_language_array[localization_language][localization_id];
}

const utf8_t *localization_get_en_us_string(localization_id_t localization_id)
{
	if ((localization_id < 0) || (localization_id >= LOCALIZATION_ID_COUNT))
	{
		return _localization_string_array_en_us[0];
	}
	
	return _localization_string_array_en_us[localization_id];
}

const utf8_t *localization_get_language_name(BYTE language)
{
	// Return the name of a language in that language itself.
	// Used by the options dialog so every language is listed in its own script.
	
	if (language < LOCALIZATION_LANGUAGE_COUNT)
	{
		return _localization_language_array[language][LOCALIZATION_ID_LANGUAGE];
	}
	
	return _localization_string_array_en_us[LOCALIZATION_ID_LANGUAGE];
}

void localization_set_language(BYTE language)
{
	// Explicitly select the active language.
	// Invalid values are ignored (the current language is kept).
	
	if (language < LOCALIZATION_LANGUAGE_COUNT)
	{
		localization_language = language;
	}
}

void localization_init(void)
{
	// Detect system language
	if (os_GetUserDefaultUILanguage)
	{
		LANGID langid;
	
		langid = os_GetUserDefaultUILanguage();

		// Check if it's Chinese (Simplified or Traditional).
		// the simplified table is the only chinese table we ship, so every
		// chinese ui locale maps to it (best effort for traditional readers).
		// 0x0804 = Chinese (Simplified, PRC)
		// 0x1004 = Chinese (Simplified, Singapore)
		// 0x0404 = Chinese (Traditional, Taiwan)
		// 0x0C04 = Chinese (Traditional, Hong Kong)
		// 0x1404 = Chinese (Traditional, Macau)
		if ((langid == 0x0804) || (langid == 0x1004) || (langid == 0x0404) || (langid == 0x0C04) || (langid == 0x1404))
		{
			localization_language = LOCALIZATION_LANGUAGE_CHINESE_SIMPLIFIED;
		}
	}
}
