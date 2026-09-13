# gui-remake-2 轮 · 子代理实施规格

基线：rc.8 内容（gui-remake 分支）。设计目标：用户 ChatGPT 参考图 = 全暗色现代外壳
（标题栏/菜单条/工具栏/状态栏/居中胶囊/现代设置窗），本轮补齐所有剩余原生亮色表面。

## 共享纪律（所有代理）
- C89（声明在块首）、tab 缩进、CRLF 行尾（例外：localization_en_us.h / localization_zh_cn.h 是 LF）。
- 所有颜色走 src/viv_theme.h 的 token（viv_theme_color(token)），禁止新增硬编码 RGB。
- 不改其他代理的文件。改动文件必须逐个自查编译：
  `python3 -m ziglang cc -target x86_64-windows-gnu -Os -DNDEBUG -DVERSION_X64 -Ilibwebp -Wno-macro-redefined -c src/<file>.c -o /tmp/<file>.o`
- 完成后在**自己的 worktree** 提交（git -c user.email=z@local -c user.name=z commit），
  并向 /home/z/my-project/worklog.md 追加一节（--- 开头，Task ID / Agent / Task / Work Log / Stage Summary）。

## 3-a 弹出菜单全自绘（worktree /home/z/wt-3a）
可改：src/viv_menu.c、viv_menu.h、viv_menubar.c、viv_menubar.h、viv_recent.c、viv_wndproc.c
1. 弹窗菜单（TrackPopupMenuEx 弹层）owner-draw：
   - 可见项构建时（viv_menu.c _viv_create_menu / viv_recent.c recent 菜单）加 MF_OWNERDRAW，
     itemData 传 static 指针结构 {command_index, recent 文本指针或 0}；
     **文字在绘制时重推导**（localization_get_string(_viv_commands[i].localization_id) +
     "\t"+_viv_get_key_text，key 变更后无需重建即正确）。viv.c 的表内 MF_OWNERDRAW=隐藏
     的既有语义不得破坏（跳过项依旧跳过）。
   - viv_wndproc.c 主窗 dispatch 已有 WM_DRAWITEM 入口（_viv_on_wm_drawitem）——扩展它；
     新增 WM_MEASUREITEM / WM_DELETEITEM 处理（挂同一 dispatch 风格）。
   - 测量：宽=文本(含 \t 快捷键)+左右 padding(各 12dip)+勾选槽(20dip)；高=字体高+10dip；
     分隔项=高 9dip（线画中）；popup 头不画快捷键列，画右箭头。
   - 绘制：整项填 VIV_TK_FACE；hover=VIV_TK_HOVER、按下=VIV_TK_DOWN（ODS_SELECTED/
     ODS_SCROLLBARBAR？用 itemState）；灰=VIV_TK_TEXTOFF；主文本左对齐（DT_LEFT|
     SINGLELINE|VCENTER），'\t' 后快捷键 DT_RIGHT 用 VIV_TK_TEXT2；
     勾选(ODS_CHECKED)/单选(ODS_RADIOCHECK)画强调色矢量勾/圆点（20dip 槽内居中）；
     子菜单箭头=实心三角 VIV_TK_TEXT2。助记符下划线始终绘制（DrawTextW 前缀处理）。
   - 菜单层背景：WM_INITMENUPOPUP（已有入口）里对 FindWindowW(L"#32768",0)
     SetClassLongPtrW(GCLP_HBRBACKGROUND,(LONG_PTR)viv_theme_brush(VIV_TK_FACE))；
     同处尝试 DwmSetWindowAttribute(menu_hwnd,33/34,圆角/边框色 VIV_TK_LINE)
     （动态加载 dwmapi 或复用 os.c 现有导出；静默失败）。
2. 菜单条 token 化：viv_menubar.c 全部 {#202020,#454545,#E8E8E8,#9A9A9A} 与 GetSysColor 项 →
   VIV_TK_CHROME / VIV_TK_HOVER / VIV_TK_TEXT / VIV_TK_TEXT2（亮色语义由 token 亮侧映射）。
3. 不许删 os.c 的 SetPreferredAppMode 机制（仍服务 MessageBox/comdlg）。
4. 测试：python3 tests/menu_structure_test.py —— 你域内 pin 必须绿（菜单结构/ID/顺序未变）。

## 3-b 对话框套件 + viv_msgbox（主线程自己做，本文件仅存档）
可改：viv_dialogs.c/h、viv_msgbox.c/h(新)、viv_view.c、viv_playlist.c、viv_load.c/h
要点：7 个 DialogBox 模板对话框重制为主题化模态面板（WS_POPUP|WS_DLGFRAME +
os_dark_titlebar + DWM caption 色=VIV_TK_CHROME；居中 owner；自泵模态循环；
IsDialogMessage tab/enter/esc）；_viv_options() → _viv_settings_show()；
viv_msgbox(parent,caption,text,type) 支持 MB_OK/OKCANCEL/YESNO + 图标 i/!/X/?；
替换 5 个调用点（viv_load 642/737、viv_dialogs 1999、viv_view 2039、viv_playlist 902）；
debug.c/mem.c 原生保留。功能逐 proc 保留（edit_key 按键捕获、set_zoom 钳制、rename 校验等）。

## 3-c 设置窗对齐设计图（worktree /home/z/wt-3c）
只改：src/viv_settings.c（必要时 +h、+三表同步）
1. _viv_settings_color() 全部 14 个 case → viv_theme_color 薄映射（删硬编码 RGB）。
2. 新增「强调色」行（LOCALIZATION_ID_ACCENT_COLOR 已预分配）：5 圆 swatch
   (viv_theme_accent_swatch(0..4))，选中=2px 强调环+白勾；点击 viv_theme_set_accent(i) +
   刷新主窗/自身（InvalidateRect(_viv_hwnd)）；取消恢复初值，确定沿用 config_save_settings。
3. 主题下拉=现有 DARK_MODE_AUTO/LIGHT/DARK 文案；切换后 _viv_apply_dark_mode(1)
   （viv_chrome.h 声明；不可见则 extern）。
4. 视觉对齐：标题行齿轮+「设置」+X；侧栏选中=强调色圆角块；底部 取消/确定（主按钮强调色）；
   DPI 缩放沿用文件内 dip 体系。
5. 三页与旧 IDD_OPTIONS 选项对照，缺项补齐或如实报告。

## 主线程收尾（合并后）
zoomui/tooltip/HUD token 化 → 全量编译 + 测试 → 模拟器转录（theme→TS 自动提取）→
agent-browser 视觉比对 → wine 实物截图（若可用）→ README/Changes/docs 更新 → 打包 zip。
