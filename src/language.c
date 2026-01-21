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

#include "language.h"
#include <windows.h>
#include "utf8.h"

static int _language_current = LANGUAGE_ENGLISH;  // Current language defaults to English

// English strings (default)
static const utf8_t *_language_strings_english[LANG_STR_COUNT] = 
{
	// File menu
	"&File",
	"&Open File...",
	"Open &Folder...",
	"Open Everything &Search...",
	"&Add File...",
	"Add Folder...",
	"Add Everything Search...",
	"Open F&ile Location...",
	"&Edit...",
	"Pre&view...",
	"&Print...",
	"Set &Desktop Wallpaper",
	"&Close",
	"&Delete",
	"Delete (Recycle)",
	"Delete (Permanently)",
	"&Rename",
	"P&roperties",
	"E&xit",
	
	// Edit menu
	"&Edit",
	"Cu&t",
	"&Copy",
	"Copy Filename",
	"Cop&y Image",
	"&Paste",
	"Rotate &Cloc&kwise",
	"Rotate Cou&nterclockwise",
	"Copy to &Folder...",
	"Mo&ve to Folder...",
	
	// View menu
	"&View",
	"Caption",
	"Frame",
	"&Menu",
	"Status &Bar",
	"&Controls",
	"&Preset",
	"&Minimal",
	"&Compact",
	"&Normal",
	"F&ullscreen",
	"&Slideshow",
	"&Window Size",
	"&Auto Fit",
	"&Refresh",
	"&Allow Shrinking",
	"&Keep Aspect Ratio",
	"&Fill Window",
	"&Best Fit",
	"Pa&n && Scan",
	"&Increase Size",
	"&Decrease Size",
	"I&ncrease Width",
	"D&ecrease Width",
	"In&crease Height",
	"De&cre&ase Height",
	"&Zoom",
	"Zoom &In",
	"Zoom &Out",
	"&Reset",
	"Move &Up",
	"Move &Down",
	"Move &Left",
	"Move &Right",
	"Move Up Left",
	"Move Up Right",
	"Move Down Left",
	"Move Down Right",
	"Move Cen&ter",
	"On &Top",
	"&Always",
	"&While Playing Slideshow or Animating",
	"&Never",
	"&Options...",
	
	// Slideshow menu
	"&Slideshow",
	"&Play/Pause",
	"&Rate",
	"&Decrease Rate",
	"&Increase Rate",
	"Custom...",
	
	// Animation menu
	"&Animation",
	"Jump &Forward",
	"Jump &Backward",
	"Short Jump &Forward",
	"Short Jump &Backward",
	"Long Jump &Forward",
	"Long Jump &Backward",
	"F&rame Step",
	"Pre&vious Frame",
	"F&irst Frame",
	"&Last Frame",
	"&Decrease Rate",
	"&Increase Rate",
	"R&eset Rate",
	
	// Navigate menu
	"&Navigate",
	"&Next",
	"P&revious",
	"&Home",
	"&End",
	"S&ort",
	"&Name",
	"Full &Path",
	"&Size",
	"Date &Modified",
	"Date &Created",
	"&Ascending",
	"&Descending",
	"&Shuffle",
	"&Jump To...",
	
	// Help menu
	"&Help",
	"&Help",
	"&Command Line Options",
	"Home &Page",
	"&Donate",
	"&About",
	
	// Dialog strings
	"Options - Void Image Viewer",
	"General",
	"View",
	"Controls",
	"OK",
	"Cancel",
	"&Store settings in %APPDATA%\\voidImageViewer",
	"Allow multiple &instances",
	"Start &menu shortcuts",
	"Associations",
	"Check &All",
	"Check &None",
	"&Shrink blit mode:",
	"&Magnify blit mode:",
	"Auto si&ze window:",
	"&Play animations at least once in slideshow",
	"Preload &next image",
	"Cache &last image",
	"&Windowed background color:",
	"&Fullscreen background color:",
	"&Left click action:",
	"&Right click action:",
	"&Mouse wheel action:",
	"&Commands:",
	"Settings for selected command",
	"&Add...",
	"Add Keyboard Shortcut",
	"Edit Keyboard Shortcut",
	"Shortcut &key:",
	"Shortcut key currently used by:",
	"Remo&ve",
	"Set Custom Rate",
	"&Custom rate:",
	"About void Image Viewer",
	"Rename",
	"Jump To",
	"Search Everything",
	"Randomize",
	
	// Time units
	" Milliseconds",
	" Seconds",
	"1 Minute",
	" Minutes",
	
	// Application name
	"void Image Viewer",
};

// Chinese Simplified strings
static const utf8_t *_language_strings_chinese[LANG_STR_COUNT] = 
{
	// File menu
	"文件(&F)",
	"打开文件(&O)...",
	"打开文件夹(&F)...",
	"打开 Everything 搜索(&S)...",
	"添加文件(&A)...",
	"添加文件夹...",
	"添加 Everything 搜索...",
	"打开文件位置(&L)...",
	"编辑(&E)...",
	"预览(&V)...",
	"打印(&P)...",
	"设置为桌面壁纸(&D)",
	"关闭(&C)",
	"删除(&D)",
	"删除（回收站）",
	"删除（永久）",
	"重命名(&R)",
	"属性(&P)",
	"退出(&X)",
	
	// Edit menu
	"编辑(&E)",
	"剪切(&T)",
	"复制(&C)",
	"复制文件名",
	"复制图像(&Y)",
	"粘贴(&P)",
	"顺时针旋转(&K)",
	"逆时针旋转(&N)",
	"复制到文件夹(&F)...",
	"移动到文件夹(&V)...",
	
	// View menu
	"视图(&V)",
	"标题栏",
	"边框",
	"菜单(&M)",
	"状态栏(&B)",
	"控件(&C)",
	"预设(&P)",
	"最小(&M)",
	"紧凑(&C)",
	"正常(&N)",
	"全屏(&F)",
	"幻灯片(&S)",
	"窗口大小(&W)",
	"自动适应(&A)",
	"刷新(&R)",
	"允许缩小(&A)",
	"保持纵横比(&K)",
	"填充窗口(&F)",
	"最佳适应(&B)",
	"平移和扫描(&N)",
	"增大尺寸(&I)",
	"减小尺寸(&D)",
	"增加宽度(&W)",
	"减小宽度(&W)",
	"增加高度(&H)",
	"减小高度(&E)",
	"缩放(&Z)",
	"放大(&I)",
	"缩小(&O)",
	"重置(&R)",
	"向上移动(&U)",
	"向下移动(&D)",
	"向左移动(&L)",
	"向右移动(&R)",
	"向左上移动",
	"向右上移动",
	"向左下移动",
	"向右下移动",
	"居中(&C)",
	"置顶(&T)",
	"总是(&A)",
	"播放幻灯片或动画时(&W)",
	"从不(&N)",
	"选项(&O)...",
	
	// Slideshow menu
	"幻灯片(&S)",
	"播放/暂停(&P)",
	"速率(&R)",
	"降低速率(&D)",
	"提高速率(&I)",
	"自定义...",
	
	// Animation menu
	"动画(&A)",
	"向前跳转(&F)",
	"向后跳转(&B)",
	"短距离向前跳转(&F)",
	"短距离向后跳转(&B)",
	"长距离向前跳转(&F)",
	"长距离向后跳转(&B)",
	"下一帧(&S)",
	"上一帧(&V)",
	"第一帧(&I)",
	"最后一帧(&L)",
	"降低速率(&D)",
	"提高速率(&I)",
	"重置速率(&E)",
	
	// Navigate menu
	"导航(&N)",
	"下一个(&N)",
	"上一个(&P)",
	"首页(&H)",
	"末页(&E)",
	"排序(&S)",
	"名称(&N)",
	"完整路径(&P)",
	"大小(&S)",
	"修改日期(&M)",
	"创建日期(&C)",
	"升序(&A)",
	"降序(&D)",
	"随机(&S)",
	"跳转到(&J)...",
	
	// Help menu
	"帮助(&H)",
	"帮助(&H)",
	"命令行选项(&C)",
	"主页(&P)",
	"捐赠(&D)",
	"关于(&A)",
	
	// Dialog strings
	"选项 - Void Image Viewer",
	"常规",
	"视图",
	"控件",
	"确定",
	"取消",
	"在 %APPDATA%\\voidImageViewer 中存储设置(&S)",
	"允许多个实例(&I)",
	"开始菜单快捷方式(&M)",
	"文件关联",
	"全选(&A)",
	"全不选(&N)",
	"缩小位图模式(&S):",
	"放大位图模式(&M):",
	"自动调整窗口大小(&Z):",
	"在幻灯片中至少播放一次动画(&P)",
	"预加载下一张图像(&N)",
	"缓存最后一张图像(&L)",
	"窗口背景颜色(&W):",
	"全屏背景颜色(&F):",
	"左键操作(&L):",
	"右键操作(&R):",
	"鼠标滚轮操作(&M):",
	"命令(&C):",
	"所选命令的设置",
	"添加(&A)...",
	"添加键盘快捷键",
	"编辑键盘快捷键",
	"快捷键(&K):",
	"当前使用此快捷键的命令:",
	"删除(&V)",
	"设置自定义速率",
	"自定义速率(&C):",
	"关于 Void Image Viewer",
	"重命名",
	"跳转到",
	"搜索 Everything",
	"随机化",
	
	// Time units
	" 毫秒",
	" 秒",
	"1 分钟",
	" 分钟",
	
	// Application name
	"Void Image Viewer",
};

const utf8_t *language_get_string(int string_id)
{
	if (string_id < 0 || string_id >= LANG_STR_COUNT)
	{
		return "";
	}
	
	if (_language_current == LANGUAGE_CHINESE_SIMPLIFIED)
	{
		return _language_strings_chinese[string_id];
	}
	
	return _language_strings_english[string_id];
}

int language_get_current(void)
{
	return _language_current;
}

void language_set(int lang)
{
	if (lang >= 0 && lang < LANGUAGE_COUNT)
	{
		_language_current = lang;
	}
}

void language_init(void)
{
	// Detect system language
	LANGID langid = GetUserDefaultUILanguage();
	
	// Check if it's Chinese (Simplified or Traditional)
	// 0x0804 = Chinese (Simplified, PRC)
	// 0x0404 = Chinese (Traditional, Taiwan)
	// 0x0C04 = Chinese (Traditional, Hong Kong)
	if (langid == 0x0804 || langid == 0x0404 || langid == 0x0C04)
	{
		_language_current = LANGUAGE_CHINESE_SIMPLIFIED;
	}
	else
	{
		_language_current = LANGUAGE_ENGLISH;
	}
}
