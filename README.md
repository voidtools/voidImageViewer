# void Image Viewer (Touch + Languages)

[![stable](https://img.shields.io/badge/status-stable-brightgreen.svg)](https://github.com/purrfecto114-lgtm/voidImageViewer_PLUS/releases)
[![release](https://img.shields.io/github/v/release/purrfecto114-lgtm/voidImageViewer_PLUS.svg?display_name=tag)](https://github.com/purrfecto114-lgtm/voidImageViewer_PLUS/releases)
[![license](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

> A stable fork of [voidtools/voidImageViewer](https://github.com/voidtools/voidImageViewer) with **touch optimizations**, **on-screen zoom controls**, a **complete dark UI**, and a **bilingual installer + UI language switcher**. Issues welcome in the [issue tracker](https://github.com/purrfecto114-lgtm/voidImageViewer_PLUS/issues).

A lightweight Windows image viewer (BMP, GIF, ICO, PNG, JPG, TIF, WEBP, EMF, WMF — animated GIF/WEBP included) that opens and displays images as fast as possible.

[Download](#download) · [What's new](#whats-new) · [Touch & zoom](#touch--zoom-controls) · [Canvas & backdrop](#canvas-backdrop--dark-mode) · [Recent files](#recent-files) · [Languages](#languages) · [Build](#build-from-source)

Download
--------
Stable binaries (setup + zip, x86/x64, SHA-256 checksums):

https://github.com/purrfecto114-lgtm/voidImageViewer_PLUS/releases

What's new
--------
**1.1.12-rc.14 — the zoom pane editor round (the current release candidate):**

- **The corner zoom percent needed two clicks, and showed a "select box"** — the click never reached the pane: the pane doubled as the window-drag anchor (inherited from upstream), its subclass ate the button down and the move loop ate the up, so the open command only fired from an inactive window's orphan up. The pane is pure input now — the drag lives on the toolbar, the menu gaps and the title bar.
- **The editor is in place, not in a 1998 box** — the pane itself becomes a borderless number field in the strip's own font and colors; enter commits, escape cancels, focus lost commits, typing replaces the selected digits. The centered dialog (sunken field, native buttons, select-all blue block) is retired along with its template and strings. The system selection color stays — the edit control offers no custom one, and it lives on four digits for the seconds the editor is open.
- Full narrative: `Changes.txt`.

**1.1.12-rc.13 — the non-win11 field round (the previous release candidate):**

- **The play button showed “pause” on an idle photo** — the toolbar read the animation preference bit as the playing state (it defaults to on and every image switch resets it); the face now gates on the frame count, and every clock-changing path notifies the toolbar and recomputes the on-top window.
- **“Zoom needs two clicks to work”** — the first zoom-in from a fresh best fit snapped the wrong way (31% → 30, invisible) and a zoom-out from best fit hit a dead gate written before the below-fit ladder. Stepping is direction-strict now, the floor gate reads the true ladder floor, and a sparse-zone fallback guarantees a move.
- **The non-Win11 look** — the settings window gets a real drop shadow on Windows 10/7, its dropdowns ride the same owner-drawn rows as the menus (no more system-blue lists next to the accent pills), the light hover/nav/strip tokens recalibrate for real contrast and layering, and the message box buttons take the pill radius. The DWM chrome calls double as the platform probe, so every fallback is one honest check.
- Full narrative: `Changes.txt`.

**1.1.12-rc.12 — the platform guardrails round (the previous release candidate):**

- **The Windows 8.1 manifest GUID was corrupted** — the supportedOS id for 8.1 was a hand-mangled value, so Windows 8.1 hosts never matched their own entry; the manifest now carries the official GUID (verified against the Microsoft application-manifests documentation).
- **The dead UnicoWS dependency is gone** — the Win32 link lines carried `UnicoWS.lib` (the Windows 9x unicode layer) straight from the original upstream import; nothing in the code imports an MSLU symbol and the x64 builds never carried it. Modern SDKs are phasing the stub out, so its removal is the forward-compatibility clean — the CI legs still link today.
- **`WM_DPICHANGED` no longer dereferences a null rect** — the system always supplies the suggested rectangle, but synthetic messages and compatibility layers may not; the resize is now optional while the DPI refresh still runs.
- Full narrative: `Changes.txt`.

**1.1.12-rc.11 — the carpet repair round (the previous release candidate):**

- **The remake's two headline breaks are fixed** — the menu bar roots measured empty since the rc.9 owner-draw flip (the labels now re-derive from the live rows, Alt mnemonics with them), and the status date pane rendered a lone "0" (a wide format through the narrow parser; the stamp now uses the localized system formatters).
- **The image viewport caught up with the top strips** — the toolbar moved under the menu bar in rc.8 but the image math still reserved its band above the status bar: a 40dip dead strip and the image top hidden behind the chrome. One view-top origin now rides every size, blit, mouse anchor and the zoom pill.
- **The theme flip flushes the token cache** — the half-skinned UI after a system theme change is gone; the toolbar rides the same tokens as the menus (light hover is the quiet 3D-light, no more selection blue); the message boxes are owned, leak-free and close-correct; the shortcut editor keeps a no-op confirm a no-op; **Win 7 closes the DPI ladder** (legacy `dpiAware` + runtime `SetProcessDPIAware` — it was DPI-*unaware*, not system-aware); the pill percent matches the status bar; the play button stops forcing fullscreen.
- Full narrative: `Changes.txt`.

**1.1.12-rc.10 — the dpi correctness round (the previous release candidate):**

- **The high-dpi font bug is fixed at the root** — the per-monitor-v2 declaration never reached the binary the way it was read: the zig builds shipped no manifest at all, and the `os_init` runtime safety net sat *before* its own `GetProcAddress` block, so the pointers were still null when it ran and it silently never fired. A dpi-unaware process gets bitmap-stretched by the system at high dpi — every font blurry and oversized. The claim now runs after the api probes and stacks three independent, all-guarded layers: a process-wide claim (Win10 1703+ `SetProcessDpiAwarenessContext`), a per-thread claim (1607+), and the Win 8.1 `shcore` fallback (Win 7 keeps the classic system-aware behavior); the zig build also drops a `viv.exe.manifest` next to the exe for loader-level coverage.
- **The message box scales from its parent's monitor** (not the stale global logical dpi) — correct on mixed-dpi monitor pairs.
- **17 splice-accident double-brace wrappers with dead `DefWindowProc` tails unwound** in the wndproc domain.
- Verified under wine: the pre-fix binary renders the settings window bitmap-stretched to ~2x at 150% scaling; the fixed binary renders 1:1 physical and self-scales to exactly 1.5x (660×628 → 990×942) with the fonts riding the same per-window dip pipeline.

**1.1.12-rc.9 — the gui remake round 2: one palette for everything:**

- **One theme system for the whole ui** — the new `viv_theme` domain holds 17 semantic tokens × dark/light × five accents (azure, teal, violet, amber, rose). The chrome strips, the menus, the dialogs, the settings window, the zoom pill and the tooltips all resolve their colors through it, so a theme or accent flip re-skins the app with zero per surface edits. The accent ships with an ini key (`accent_color`) and a live swatch row in the settings window.
- **The popup menus are app drawn** — every visible row carries a draw record and the painters re-derive the label at draw time (command table + localization + live key bindings), so a language or shortcut change never shows a stale row. Accent check marks, right aligned shortcuts, hairline separators; the popup layer gets the win11 rounded corners and a themed border through the dwm; `WM_MENUCHAR` keeps alt+letter working. No undocumented uxtheme ordinal in the menu path anymore.
- **Themed message boxes** — `viv_msgbox` draws the vector icon, the wrapped text and the accent primary button with a real modal pump (tab/enter/esc); the five `MessageBox` call sites moved over (the fatal paths in debug/mem keep the native box on purpose).
- **The classic dialogs join the theme** — the dark dialog skin reads the same tokens (the old hardcoded palette is gone) and every dialog frame gets the win11 rounded corners plus the caption color from the dwm. The old tabbed options route opens the remade settings window.
- **A build the mockup cannot drift from** — `scripts/extract-theme.mjs` transcribes `src/viv_theme.c` into the page simulator, and zig cc builds the whole tree (`build-zig/build.sh`, 98 translation units, about 465 KB x64 with `-Os`).

**1.1.12-rc.7 — the zoom pill rework:**

- **The zoom pill is one self-drawn window** — no nested owner-drawn buttons fighting the layered tray: `zoomui.c` paints the tray and every capsule in a single `WM_PAINT` pass (true stadium caps at every DPI), hit tests the button rects itself, tracks presses with capture (no command when the drag leaves the button), and passes clicks through everything outside the capsules so a fading pill never blocks the image.

Recent versions, one line each — full per-round detail in [Changes.txt](Changes.txt):

- **1.1.12-rc.6** — the top bar remake: the menu bar is a fully self-drawn child window (`viv_menubar`), the status bar owns its dark face, the toolbar button spacing is pinned uniform in both themes.
- **1.1.12-rc.5** — the structure round: the gesture cluster home in the view domain, the 2,208-line window procedure split into 42 per-message handlers in the new `viv_wndproc.c`, the splice guard's file manifest pinned.
- **1.1.12-rc.4** — the white band after opening Options or switching the theme fixed at the root (the dark re-apply lost its flip gate; the sweep is now unconditional and idempotent); the Options navigation tree reads in the dark UI.
- **1.1.12-rc.3** — the monolith is gone: `viv.c` (21,129 lines) is now a 4,718-line core plus eleven domain modules and a `viv_state.h` shared-context layer — a pure physical move (function bodies byte-identical; `/GL` whole-program optimization keeps cross-module inlining, performance unchanged); the setup offers the EMF/WMF associations; the vendored libwebp pruned of non-Windows build systems, fuzzers and docs (62 files — `COPYING`/`PATENTS`/`AUTHORS` kept).
- **1.1.12-rc.2** — the About dialog's band and title move into the resource template (correct at every DPI and in both themes); the post-theme-flip white band fixed (the frame repaint joined the sweep, the toolbar strip relayouts with the system metrics).
- **1.1.11** — one type system for the whole UI (the system message font per dialog at its own window DPI — CJK-safe, no more fallback-font mismatch); EMF/WMF in every association surface (metafiles rasterize via GDI+ — display-level support); recent files, transparency backdrop, complete dark dialogs.

Touch & zoom controls
--------

| Gesture / control | Action |
| --- | --- |
| Two finger pinch | Zoom in / out (anchored at finger center) |
| Two finger drag | Pan / scroll image (with inertia) |
| Two finger tap | Reset zoom |
| Double tap (touch) | Toggle 1:1 / best fit |
| Toolbar zoom buttons | Zoom in / out |
| Floating zoom bar | Windowed: zoom pill. Fullscreen: prev / play / pause / next / zoom (bottom center, idle fade) |

Gestures need Windows 7+ with touch hardware. Single-finger input stays mouse-compatible, so configured click actions are unaffected. Toggle the floating controls via **View → Zoom Controls**. A pinch keeps shrinking below the windowed fit, down to about fit/16 (mirroring the 16× zoom cap) — `Allow shrinking` in Options keeps its meaning.

Canvas, backdrop & dark mode
--------

- **Windowed / fullscreen background color** — Options → View, or the View menu picker; the mat around the image and the empty-window canvas.
- **Backdrop under transparency** — View → Transparency backdrop: follow the window background color, black, white, custom color, or checkerboard. Alpha images (PNG/GIF/WEBP) composite over it at load time. This is *not* the canvas around the image — that color is the windowed background above.
- **Dark UI rule** — the light UI always shows your exact colors. In the dark UI a light mat keeps its hue but drops into the dark range (the default white maps to the dark palette canvas), and the same rule applies to the custom backdrop, so nothing glares out of the dark chrome. The Win11 caption tint follows the mat.
- **The dark UI is complete** — dialogs, options pages, the menu bar and the navigation tree all follow the theme (owner-drawn where the theme API falls short on older builds); light dialogs stay light.
- Dark mode itself: Options → General → Dark mode — light, dark, or follow Windows. Theme flips repaint the whole window and re-assert the dark chrome unconditionally (a delayed re-check heals any system-side light repaint), and the open dialogs re-theme live.

Recent files
--------
File → Recent keeps the last ten opened paths (deduplicated case-insensitively; missing files drop out on the next open; Clear empties the list). The list is capped at ten on every path and writes are debounced off the open path.

Languages
--------
English and 简体中文 ship built-in.

- The setup picks the language on its first page.
- **Options → General → Language** switches Auto / English / 简体中文 on the fly (no restart).
- Stored as `language=auto|english|chinese` in `voidImageViewer.ini`; unattended installs may pass `/language english|chinese|auto`.

Build from source
--------
Plain C + Win32 API, Visual Studio:

1. Open `vs2019/voidImageViewer.sln` (VS2022+, v143 toolset) or `vs2026/voidImageViewer.sln` (v145 toolset). Both share one file list (`voidImageViewer.files.props`). VS2019 works with `/p:PlatformToolset=v142` (not CI-covered).
2. Build the `voidImageViewer` project (x64 or Win32).
3. Optional setup: NSIS 3 via `nsis\build_installer.ps1` (auto-detects the VS version; sources compile with `/utf-8`).

The zig cross build needs no Visual Studio: `sh build-zig/build.sh` (98 translation units, about 465 KB x64 with `-Os`). The vs linker embeds `res/voidImageViewer.Manifest` (per-monitor v2); the zig build keeps no embedded manifest, so it writes `viv.exe.manifest` next to the exe and the runtime claim in `os_init` covers even a stripped copy — keep the two files together, or build through vs when you need a single-file binary.

The source layout: one core (`src/viv.c` — the window procedure, startup, the command line, the state definitions) plus eleven domain modules (`src/viv_<domain>.c/.h`) and the shared-context header (`src/viv_state.h`); see `docs/architecture/viv-split-spec.md`.

GitHub Actions compiles every push (pinned `windows-2022`/v143 + `windows-2025`/v145 legs); tag pushes run the tests, verify SHA-256 end to end and publish the release assets.

![Void Image Viewer Image View](https://www.voidtools.com/voidImageViewer.Image.View10.gif)

See also
--------
Upstream project: https://github.com/voidtools/voidImageViewer
