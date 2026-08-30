# void Image Viewer
A lightweight image viewer for Windows with animated GIF/WEBP support.  
Opens and displays BMP, GIF, ICO, PNG, JPG, TIF and WEBP images as fast as possible.  
Animate GIF/WEBP files as accurately as possible.  

[Download](#download)<br/>
[See also](#See-also)<br/>
<br/><br/><br/>



Download
--------
https://github.com/voidtools/voidImageViewer/releases

https://www.voidtools.com/forum/viewtopic.php?t=5623
<br/><br/><br/>


void Image Viewer main window:

![Void Image Viewer Image View](https://www.voidtools.com/voidImageViewer.Image.View10.gif)
<br/><br/><br/>



void Image Viewer General Options:

<img width="542" height="472" alt="image" src="https://github.com/user-attachments/assets/accca998-0a15-40f4-935b-d41d677f208b" />
<br/><br/><br/>



void Image Viewer View Options:

<img width="542" height="472" alt="image" src="https://github.com/user-attachments/assets/316a167b-af09-4cbe-bec0-60ce396f047b" />
<br/><br/><br/>



void Image Viewer Controls Options:

<img width="542" height="472" alt="image" src="https://github.com/user-attachments/assets/0a281f65-c849-4b0d-bcf9-0149badea0fc" />
<br/><br/><br/>



## Changelog

### Version 1.2.1.0 (Latest)
- **New Feature: Smooth Toggle in Main Viewer**
  - Added **Smooth** (Halftone / Bilinear interpolation) toggle directly in the `View` menu and right-click context menu.
  - Linked with configuration persistence (`shrink_blit_mode` / `mag_filter`), and seamlessly integrated with Compare Images initial smoothing state.
- **New Feature: Compare Images (Side-by-Side Image Comparison)**
  - Added multi-pane **Compare Images** window accessible via `View` -> `Compare Images...` or shortcut (`Ctrl+Alt+C`).
  - **Drag & Drop Support:**
    - Directly drag & drop image files from File Explorer to either the left or right pane to compare.
    - Dropping 2 files simultaneously automatically loads them into the left and right panes.
  - **"Use Current Image" Toggle:**
    - Convenient checkbox on each pane to instantly load the image currently open in the main viewer.
    - Automatically disables on the other pane when active to avoid duplicate/conflicting states.
  - **Synchronized Navigation & Precise Zoom:**
    - Mouse wheel zooms all compared images simultaneously using the exact same zoom engine (`_viv_calculate_next_zoom`) with synchronized viewport centers to eliminate drift.
    - Left-click drag pans all compared images synchronously.
    - **Individual Pane Mode:** Hold `Ctrl` key or use **Right Mouse Button** to zoom or pan only the hovered image pane.
  - **Per-Pane Mini Toolbar:**
    - Previous (`<`) and Next (`>`) buttons to browse folder/playlist images per pane.
    - Zoom In (`+`), Zoom Out (`-`), Zoom % display, `1:1` (Actual size), and `Fit` (Best Fit) buttons.
    - Rotate clockwise button (`90°`).
    - Change/Select image file button (`...`).
  - **Top Control Bar:**
    - Helpful shortcut and mouse tips.
    - **Auto-Fit** toggle to automatically scale images to fit pane viewports.
    - **Smooth** toggle (Halftone / High-quality interpolation).
    - **Filename** toggle with bright green overlay text (`filename`, `dimensions`, `file size`).
    - **Close** button (or `ESC` key) to quickly return to main viewer.
  - **Full Format Support & Alpha Transparency:**
    - Native support for PNG (with alpha transparency), GIF, animated GIF, BMP, ICO, TIFF, JPEG, and WebP (via `libwebp` fallback decoder).
    - Clean alpha blending over the window background color, preventing black background issues on transparent images.
  - **Full Multilingual Localization:** Added translations for all compare UI elements in English (`en-US`), Indonesian (`id-ID`), and Simplified Chinese (`zh-CN`).

### Version 1.1.0.0
- **New Feature: Zoom Options Tab & Calculation Engine**
  - Added dedicated **Zoom options** tab in Options dialog (`IDD_ZOOM`).
  - Added customizable zoom calculation methods:
    - **Relative step:** Multiplicative scaling using configured zoom step % (default: 10%).
    - **Absolute step:** Additive scaling by fixed percentage point steps.
    - **Fixed table:** Step-by-step navigation through a user-defined list of zoom percentage levels (default: `1;2;3;10;14;16;19;21;25;28;33;38;43;50;57;66;76;87;100;115;132;150;174;200;230;260;300;350;400;450;500;600;700;800;900;1000;1200;1400;1600;2000;3000;`).
    - Added **Load default table** button to quickly restore default values.
- **Improved Zoom Handling:** Pixel under cursor remains stationary during mouse wheel and shortcut zooms across all zoom methods.
- **Expanded Localization:** Added full UI translations for the Zoom options page in English (`en-US`), Indonesian (`id-ID`), and Simplified Chinese (`zh-CN`).
- **Configuration Persistence:** Zoom settings (`zoom_calc_method`, `zoom_step`, `zoom_fixed_table`) automatically saved to and loaded from `voidImageViewer.ini`.

| Feature / Behavior | Before (v1.0.1.0) | After (v1.1.0.0) |
| :--- | :--- | :--- |
| **Zoom Calculation Method** | Hardcoded fixed exponential power steps (`_viv_zoom_pos` from 0 to 63) | Choice between **Relative step (%)**, **Absolute step (%)**, or customizable **Fixed table** |
| **Zoom Step Config** | Fixed / non-configurable | Configurable zoom step percentage (default `10%`) |
| **Fixed Table Values** | Not available | Semicolon-delimited custom table support (up to 50 ordered values) with default reset |
| **Settings UI** | 3 tabs (*General*, *View*, *Controls*) | 4 tabs (*General*, *View*, *Controls*, ***Zoom options***) |
| **Localization** | Dialog strings for 3 tabs | Full multilingual translations for all 4 tabs (English, Indonesian, Chinese) |
| **INI Storage** | No zoom calculation preferences saved | Saves `zoom_calc_method`, `zoom_step`, `zoom_fixed_table` to INI |

<br/><br/><br/>

See also
--------
https://www.voidtools.com/forum/viewtopic.php?t=5623

