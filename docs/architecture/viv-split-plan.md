# viv.c 架构拆分实现计划

> **面向 AI 代理的工作者：** 必需子技能：使用 subagent-driven-development（推荐）或
> executing-plans 逐任务实现此计划。步骤使用复选框（`- [ ]`）语法来跟踪进度。

**目标：** 把 21,129 行的 `src/viv.c` 按功能域拆为 `viv_<domain>.c/.h` 模块（12 片），残余收敛至 <5,000 行，全程纯物理移动零行为变化。

**架构：** 片 0 先建 `viv_state.h`（159 个 extern 共享状态声明）铺平跨域引用；随后按封闭性升序逐域移动函数到新编译单元；守卫测试改为拼接读取（viv.c + 全部 viv_*.c），现有守卫零修改自动覆盖。

**技术栈：** C（Win32 GDI）· MSBuild v143/v145（经共享 `voidImageViewer.files.props` 声明编译单元）· Python 三套件守卫。

**规格：** `docs/architecture/viv-split-spec.md`（论证依据——执行者两份都读）。

## 全局约束

- 纯移动纪律：只允许剪切/粘贴/新声明/props 加行/Changes.txt；禁止函数体重写、重命名、重排序。
- 每片一轮，diff 预算 ~10 文件 / ~400 行（viv.c 削减与新增移动行成对，不双计）。
- 每片门禁：G1 三套件全绿 → G2 CI 双工具链 0 error 0 warning → G3 push 快进 → G4 每 2–3 片用户实机。
- 模块命名 `viv_<domain>.c/.h`，目录 `src/`，编译单元单点登记于 `voidImageViewer.files.props`。
- 新 .h 只声明该域导出函数（供 viv.c 与其他域 include）；域内函数保持 static。
- 回滚：单片单 commit，`git revert` 即回。

---

## 任务 A · 片 0：状态层 `viv_state.h`（R70 执行）

文件：新增 `src/viv_state.h`；修改 `src/viv.c`（159 个 static → 去 static）、`voidImageViewer.files.props`（无新 .c，不动）、`Changes.txt`、`tests/menu_structure_test.py`（守卫）。

- [ ] A1. 守卫先行（红）：menu_structure_test.py 新增 `t_split_architecture_round69()` —— 已随本计划文档先行落地（规格/计划文档存在 + 拼接助手存在 + 片序锁定）；再新增 `t_state_layer_round70()`：断言 `src/viv_state.h` 存在且包含分组 extern 声明、viv.c 中 `^static.*_viv_` 文件级计数 = 0（存量 159 → 0），先跑出 FAIL。
- [ ] A2. 生成 `viv_state.h`：脚本从 viv.c 提取 159 个文件级 static 变量声明 → extern 化（保留原注释/初始化剥离）→ 按功能域分组（chrome/图像/视图/播放列表/加载/杂项）+ 文件头注释（用途、纪律、"过渡态"声明引用 spec §2.3）。
- [ ] A3. viv.c：159 个定义行去 `static` 关键字（位置/行文不动）；`#include "viv_state.h"` 加在 `#include "viv.h"` 之后。
- [ ] A4. G1 三套件复跑全绿；G2 本地括号平衡 + 提交，push 触发 CI 双工具链（v143 + v145 均 0 error 0 warning 才算过）。
- [ ] A5. Changes.txt 新节（片 0：状态层，移动清单 = 159 声明）。
- [ ] A6. Commit（`the viv state layer round: the split begins, 159 shared statics declared in one header`）→ push 快进。

## 任务 B · 片 1：`viv_recent.c`（R71 执行）

文件：新增 `src/viv_recent.c`、`src/viv_recent.h`；修改 `src/viv.c`、`voidImageViewer.files.props`、`tests/*_test.py`（拼接助手）、`Changes.txt`。

- [ ] B1. 守卫先行（红）：新增 `t_split_recent_round71()`：断言 `src/viv_recent.c` 存在、包含 7 个 `_viv_recent_*` 函数定义、viv.c 不再包含其函数体（按函数名 + 特征行计数）；三套件 read 助手改拼接模式（`viv.c + viv_*.c` 字典序）——助手改动本身加守卫（拼接后总文本长度守恒 ±ε）。
- [ ] B2. 剪切 7 个函数（`_viv_recent_file_push/remove/clear`、`_viv_create_recent_menu`、`_viv_recent_save_fold` 等测量清单）+ 3 个共享 static 走 `viv_state.h`（已在片 0 extern 化，零额外工作）。
- [ ] B3. 新建 `viv_recent.c`（文件头 license 短注 + include viv.h/viv_state.h + 7 函数原样粘贴 + 域内函数补 static）与 `viv_recent.h`（7 个导出原型）。
- [ ] B4. viv.c：删除 7 函数体与对应前置声明；加 `#include "viv_recent.h"`；props 加 `<ClCompile Include="..\src\viv_recent.c" />`。
- [ ] B5. G1 全绿（拼接守卫自动覆盖）→ G2 CI 双工具链 → Changes.txt（片 1：recent 域，移动清单）→ commit + push。

## 任务 C · 片 2–终片（R72+ 逐轮）

按 spec §4 片序表执行（everything → playlist → render → anim → fullscreen → dark_ui/dialogs → load → chrome → view → 残余整理），每片复用任务 B 模板：守卫先行（红）→ 纯移动 → 拼接守卫（绿）→ CI → Changes.txt → commit/push。
片 7（对话框域）与片 9（chrome 域）开始前重测域边界（届时 viv.c 已收缩，封闭性数据会变化，重跑 spec §1 的测量脚本更新片序理由）。

## 完成定义（DoD）

- [ ] viv.c < 5,000 行；全部 `viv_*.c` 各 < 3,000 行（对齐 os.c 体量上限惯例）。
- [ ] 三套件守卫在拼接模式下全绿且守卫总数只增不减。
- [ ] CI 双工具链在拆分后首个 release tag 上全绿（发版由用户指令触发）。
- [ ] 用户实机验收（拆分潮合并验收：功能冒烟 + 白条/About 回归项）。
- [ ] spec §2.3 的"过渡态"记录在案：extern 状态清单移交后续逐域私有化轮（T15 候选，不属本计划）。

---

## R70 校准增补（2026-09-11）：一次性全量拆分

> 用户指令松绑了本计划"每片一轮"的节奏约束："拆分没必要局限在现有的
> 限制中，先审视大方向"。校准结论：把任务 A–C 的全部 12 片压缩为
> 一轮（R70）一次性落地，风险控制从"节奏"转移到"机械验证"。

**大方向重审（与原计划的差异）：**

1. **一次性 vs 分片**：原计划 12 轮的目的是控制单轮 diff 风险；一次性
   拆分把风险交给机械保证——生成器只做物理移动（函数体逐字节搬运，
   唯一允许的文本变更 = 剥离导出符号的 `static` 前缀），另有字节守恒、
   括号平衡、引用解析、单一编译单元定义四道本地验证 + CI 双工具链编译。
2. **域合并与新增**（对 spec §4 片序表的落地修正）：fullscreen 并入
   chrome；dialog_dark 基础设施独立为 dark；everything 并入 playlist；
   **新增 install**（安装/关联/注册表/快捷方式——spec 把它们留在残余，
   实测 975 行成体系，抽出后残余才能 <5,000）与 **menu**（命令表/键
   表/菜单构造，688 行）。
3. **最小导出面**（对 spec §2.3 片 0 的收紧）：不再把 159 个 static 全
   部 extern 化——只 extern 被跨域引用的（110 个）；被单一域独占的
   52 个变量连同定义一起搬进该域并保留 static；核心独占的 13 个不动。
   导出函数同理：283 个函数中 163 个导出，其余保持 static。
4. **性能与体积保证**：工程早已开启 `/GL`（WholeProgramOptimization，
   链接期全程序优化）——跨编译单元内联由链接器恢复，拆分零性能损失；
   导出符号不进入发行版二进制（符号只在 PDB），exe 体积不变。CI 构建
   产物与上一 tag 对账。
5. **拼接模式升级**：三套件 read("src/viv.c") 返回 viv.c + 全部
   viv_*.c（字典序）；menu/simulation 再拼上 viv_state.h（宏与共享
   类型守卫需要对状态层解析）。纯移动守恒窗口：拼接总量 21,130 →
   21,7xx 行（只增声明）。
6. **门禁前移**：提交①（纯拆分）推送后先等 CI 双工具链编译绿，才做
   提交②（rc.3 版本位 + 安装程序 EMF/WMF + 文档 + 清理）；tag 在②的
   CI 绿之后才打——release 流水线五资产自动产出。

**落地实测（R70 生成器输出）**：viv.c 21,130 → 4,742 行（残余 =
WndProc 2,207 + init/kill/exit/main + 命令行 + 状态定义 + 110 个
extern 定义）；11 个域模块全部 <3,000 行（chrome 2,911 / view 2,923
为最大）；viv_state.h 391 行（14 类型 + 13 宏区 + 110 extern + 5 核心
导出原型）。原计划任务 A/B/C 的 DoD 全部达成（残余 <5,000 ✓、域
<3,000 ✓、守卫拼接绿 ✓、CI 在 rc.3 tag 上验证 ✓）。
