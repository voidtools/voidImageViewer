;
; Copyright 2025 voidtools / David Carpenter
; 
; Multi-language and multi-VS version support added by hesphoros (2026)
; Unified single-file bilingual (English + Simplified Chinese) installer (2026)
; 
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
; 
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
; 
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.
;

; defines
!verbose 3

; Visual Studio version configuration
; Can be overridden via command line: makensis.exe /DVS_VERSION=vs2026 installer.nsi
; Supported versions: vs2019, vs2026
!ifndef VS_VERSION
        !define VS_VERSION "vs2026"  ; Default VS version
!endif

; Build configuration (Release, Debug, etc.)
!ifndef BUILD_CONFIG
        !define BUILD_CONFIG "Release"  ; Default build configuration
!endif

; Source location of voidImageViewer.exe.
; Override with /DVIV_EXE_DIR=<path> when the exe is not in the default
; project output directory (used by CI builds).
!ifndef VIV_EXE_DIR
        !ifdef x64
                !define VIV_EXE_DIR "..\${VS_VERSION}\x64\${BUILD_CONFIG}"
        !else
                !define VIV_EXE_DIR "..\${VS_VERSION}\${BUILD_CONFIG}"
        !endif
!endif

; we need admin access to write to program files and registry (associations).
; the application elevates itself when required.
RequestExecutionLevel user

CRCCheck On
XPStyle on

; includes
!include "MUI.nsh"

!include "version.nsh"

!include WinMessages.nsh
!include InstallOptions.nsh
!include FileFunc.nsh

!ifdef x64
        
        !define TARGETMACHINE "x64"
        InstallDir "$PROGRAMFILES64\voidImageViewer"
        
!else
        
        !define TARGETMACHINE "x86"
        InstallDir "$PROGRAMFILES\voidImageViewer"
        
!endif
        
; vars

Var existing_ini_filename
Var admin_install_options
Var user_install_options

BrandingText "void Image Viewer ${DISPLAYVERSION} (${TARGETMACHINE}) Setup"

; settings /SOLID will save a few KBs
SetCompressor /SOLID lzma
Name "void Image Viewer"

; unified output file. the installer is bilingual, no language code in the name.
OutFile "voidImageViewer-${DISPLAYVERSION}-${TARGETMACHINE}-Setup.exe"

; MUI settings
!define MUI_ICON "..\res\voidImageViewer.ico"

; remember the selected installer language for future installs and uninstalls.
; always show the language dialog too (preselecting the remembered language):
; without ALWAYSSHOW an upgrade install silently reuses the remembered
; language and the user never sees a way to switch it.
!define MUI_LANGDLL_ALWAYSSHOW
!define MUI_LANGDLL_REGISTRY_ROOT "HKCU"
!define MUI_LANGDLL_REGISTRY_KEY "Software\voidImageViewer"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

; pages
!insertmacro MUI_PAGE_LICENSE "$(LicenseData)"

!insertmacro MUI_PAGE_DIRECTORY

; options page
Page custom InstallOptions
Page custom InstallOptions2

!insertmacro MUI_PAGE_INSTFILES

; offer to run void Image Viewer after a successful install.
!define MUI_FINISHPAGE_RUN "$INSTDIR\voidImageViewer.exe"

!insertmacro MUI_PAGE_FINISH

;!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; installer languages (the first language is the fallback).
; MUI language macros must come after the page macros.
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "SimpChinese"

; per language license data. (name, language id, license file)
LicenseLangString LicenseData ${LANG_ENGLISH} "installer_license_English.txt"
LicenseLangString LicenseData ${LANG_SIMPCHINESE} "installer_license_Chinese.txt"

; localized installer messages.
LangString MsgSelectOptionsTitle ${LANG_ENGLISH} "Select Install Options"
LangString MsgSelectOptionsTitle ${LANG_SIMPCHINESE} "选择安装选项"
LangString MsgSelectOptionsSub ${LANG_ENGLISH} "Choose any additional install options."
LangString MsgSelectOptionsSub ${LANG_SIMPCHINESE} "选择其他安装选项。"
LangString MsgOsNotX64 ${LANG_ENGLISH} "OS is not x64.$\nInstall anyway?"
LangString MsgOsNotX64 ${LANG_SIMPCHINESE} "当前操作系统不是 64 位。$\n仍然要安装吗？"
LangString MsgExecAdminFailed ${LANG_ENGLISH} "Failed to execute admin command"
LangString MsgExecAdminFailed ${LANG_SIMPCHINESE} "执行管理员安装命令失败"
LangString MsgExecOptionsFailed ${LANG_ENGLISH} "Failed to execute install options"
LangString MsgExecOptionsFailed ${LANG_SIMPCHINESE} "执行安装选项失败"

!insertmacro GetOptions

; Version Info
VIProductVersion "${VERSION}"

; don't localize these:
VIAddVersionKey "ProductName" "void Image Viewer"
VIAddVersionKey "Comments" ""
VIAddVersionKey "CompanyName" ""
VIAddVersionKey "LegalTrademarks" ""
VIAddVersionKey "LegalCopyright" "Copyright (c) 2025 David Carpenter"
VIAddVersionKey "FileDescription" "void Image Viewer Setup"

VIAddVersionKey "FileVersion" "${DISPLAYVERSION}"
VIAddVersionKey "ProductVersion" "${DISPLAYVERSION}"

Function .onInit

        ; show the language selection dialog first.
        ; the selection is remembered in HKCU and reused by the uninstaller.
        !insertmacro MUI_LANGDLL_DISPLAY

        ; extract the installer option pages for both languages.
        ; the english files provide the "active" page names, the chinese pages
        ; are copied over the active ones when chinese is selected.
        !insertmacro INSTALLOPTIONS_EXTRACT "InstallOptions.ini"
        !insertmacro INSTALLOPTIONS_EXTRACT "InstallOptions2.ini"
        !insertmacro INSTALLOPTIONS_EXTRACT "InstallOptions_Chinese.ini"
        !insertmacro INSTALLOPTIONS_EXTRACT "InstallOptions2_Chinese.ini"

        ; switch the active installer option pages to the selected language.
        StrCmp $LANGUAGE ${LANG_SIMPCHINESE} 0 language_pages_done
                CopyFiles /SILENT "$PLUGINSDIR\InstallOptions_Chinese.ini" "$PLUGINSDIR\InstallOptions.ini"
                CopyFiles /SILENT "$PLUGINSDIR\InstallOptions2_Chinese.ini" "$PLUGINSDIR\InstallOptions2.ini"
        language_pages_done:

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; remember last install dir.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        ClearErrors
        
        ; use the appropriate reg view.
        ; dont install to the previous x86 location C:\Program Files (x86) if we are x64

!ifdef x64
        SetRegView 64
!endif

        ReadRegStr $R2 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\voidImageViewer" 'UninstallString'

!ifdef x64
        SetRegView 32
!endif

        IfErrors no_existing_install_dir

        ; the exe writes a quoted UninstallString (safe with spaces in the
        ; path): strip the leading quote so PathRemoveFileSpec sees a plain
        ; file spec (it removes through the trailing quote).
        StrCpy $R3 $R2 1
        StrCmp $R3 '"' 0 +2
        StrCpy $R2 $R2 "" 1

        ClearErrors
        system::Call 'Shlwapi::PathRemoveFileSpec(tR2R2) i.r1'
        IfErrors no_existing_install_dir
        
        StrCpy $INSTDIR $R2

no_existing_install_dir:

        ; get the existing ini filename.
        StrCpy $existing_ini_filename "$APPDATA\voidImageViewer\voidImageViewer.ini"
        
        ; Check if appdata is set to zero
        ; $INSTDIR is the existing install location (or the default one if it does not exist)
        ReadINIStr $0 "$INSTDIR\voidImageViewer.ini" "voidImageViewer" "appdata"
    StrCmp $0 "0" 0 skip_check_app_data
        StrCpy $existing_ini_filename "$INSTDIR\voidImageViewer.ini"
        !insertmacro MUI_INSTALLOPTIONS_WRITE "InstallOptions.ini" "Field 2" "State" "0"
        !insertmacro MUI_INSTALLOPTIONS_WRITE "InstallOptions.ini" "Field 3" "State" "1"

skip_check_app_data:

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; localization
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

!ifdef x64

        ; check if OS is x64 capable.
        System::Call "kernel32::GetCurrentProcess() i .s"
        System::Call "kernel32::IsWow64Process(i s, *i .r0)"
        IntCmp $0 0 is32
        goto is64

is32:
        
        MessageBox MB_YESNOCANCEL|MB_ICONEXCLAMATION "$(MsgOsNotX64)" IDYES is64
        Abort
        
is64:
        
!endif ; !ifdef x64

FunctionEnd

Function un.onInit

        ; use the language that was selected during the install.
        !insertmacro MUI_UNGETLANGUAGE

        ; get the existing ini filename.
        StrCpy $existing_ini_filename "$APPDATA\voidImageViewer\voidImageViewer.ini"
        
        ; Check if appdata is set to zero
        ; $INSTDIR is the existing install location
        ReadINIStr $0 "$INSTDIR\voidImageViewer.ini" "voidImageViewer" "appdata"
    StrCmp $0 "0" 0 skip_check_app_data
        StrCpy $existing_ini_filename "$INSTDIR\voidImageViewer.ini"

skip_check_app_data:

FunctionEnd

; sections
Section "voidImageViewer" SECTION_VOIDIMAGEVIEWER

        ; init
        StrCpy $admin_install_options ""
        StrCpy $user_install_options ""

        ; app data
        !insertmacro MUI_INSTALLOPTIONS_READ $R0 "InstallOptions.ini" "Field 2" "State"
        strcmp $R0 "0" no_app_data
        StrCpy $admin_install_options "$admin_install_options /appdata"
        Goto skip_app_data

no_app_data:

        ; this option is special, we MUST unset any option that voidImageViewer was previously installed with.
        StrCpy $admin_install_options "$admin_install_options /noappdata"
        
skip_app_data:  

        ; the application language is intentionally NOT forwarded from the
        ; installer any more: a fresh install starts in "auto" (follows the
        ; system language; the viewer options offer auto/english/chinese).
        ; the installer language only localizes the installer itself.

        ; startmenu
        !insertmacro MUI_INSTALLOPTIONS_READ $R0 "InstallOptions2.ini" "Field 1" "State"
        strcmp $R0 "0" no_startmenu
        StrCpy $admin_install_options "$admin_install_options /startmenu"
        Goto skip_startmenu
        
no_startmenu:

        StrCpy $admin_install_options "$admin_install_options /nostartmenu"

skip_startmenu:

        ; BMP Associations
        !insertmacro MUI_INSTALLOPTIONS_READ $R0 "InstallOptions2.ini" "Field 3" "State"
        strcmp $R0 "0" no_bmp_association
        StrCpy $user_install_options "$user_install_options /bmp"
        Goto skip_bmp_association
        
no_bmp_association:

        StrCpy $user_install_options "$user_install_options /nobmp"

skip_bmp_association:

        ; GIF Associations
        !insertmacro MUI_INSTALLOPTIONS_READ $R0 "InstallOptions2.ini" "Field 4" "State"
        strcmp $R0 "0" no_gif_association
        StrCpy $user_install_options "$user_install_options /gif"
        Goto skip_gif_association
        
no_gif_association:

        StrCpy $user_install_options "$user_install_options /nogif"

skip_gif_association:

        ; ico Associations
        !insertmacro MUI_INSTALLOPTIONS_READ $R0 "InstallOptions2.ini" "Field 5" "State"
        strcmp $R0 "0" no_ico_association
        StrCpy $user_install_options "$user_install_options /ico"
        Goto skip_ico_association
        
no_ico_association:

        StrCpy $user_install_options "$user_install_options /noico"

skip_ico_association:

        ; jpeg Associations
        !insertmacro MUI_INSTALLOPTIONS_READ $R0 "InstallOptions2.ini" "Field 6" "State"
        strcmp $R0 "0" no_jpeg_association
        StrCpy $user_install_options "$user_install_options /jpeg"
        Goto skip_jpeg_association
        
no_jpeg_association:

        StrCpy $user_install_options "$user_install_options /nojpeg"

skip_jpeg_association:

        ; jpg Associations
        !insertmacro MUI_INSTALLOPTIONS_READ $R0 "InstallOptions2.ini" "Field 7" "State"
        strcmp $R0 "0" no_jpg_association
        StrCpy $user_install_options "$user_install_options /jpg"
        Goto skip_jpg_association
        
no_jpg_association:

        StrCpy $user_install_options "$user_install_options /nojpg"

skip_jpg_association:

        ; png Associations
        !insertmacro MUI_INSTALLOPTIONS_READ $R0 "InstallOptions2.ini" "Field 8" "State"
        strcmp $R0 "0" no_png_association
        StrCpy $user_install_options "$user_install_options /png"
        Goto skip_png_association
        
no_png_association:

        StrCpy $user_install_options "$user_install_options /nopng"

skip_png_association:

        ; tif Associations
        !insertmacro MUI_INSTALLOPTIONS_READ $R0 "InstallOptions2.ini" "Field 9" "State"
        strcmp $R0 "0" no_tif_association
        StrCpy $user_install_options "$user_install_options /tif"
        Goto skip_tif_association
        
no_tif_association:

        StrCpy $user_install_options "$user_install_options /notif"

skip_tif_association:

        ; tiff Associations
        !insertmacro MUI_INSTALLOPTIONS_READ $R0 "InstallOptions2.ini" "Field 10" "State"
        strcmp $R0 "0" no_tiff_association
        StrCpy $user_install_options "$user_install_options /tiff"
        Goto skip_tiff_association
        
no_tiff_association:

        StrCpy $user_install_options "$user_install_options /notiff"

skip_tiff_association:

        ; webp Associations
        !insertmacro MUI_INSTALLOPTIONS_READ $R0 "InstallOptions2.ini" "Field 11" "State"
        strcmp $R0 "0" no_webp_association
        StrCpy $user_install_options "$user_install_options /webp"
        Goto skip_webp_association
        
no_webp_association:

        StrCpy $user_install_options "$user_install_options /nowebp"

skip_webp_association:

        ; emf Associations (the metafile pair joined the viewer in 1.1.12;
        ; the exe-side install switches are generic over the extension
        ; table, so /emf and /noemf work like every other format)
        !insertmacro MUI_INSTALLOPTIONS_READ $R0 "InstallOptions2.ini" "Field 12" "State"
        strcmp $R0 "0" no_emf_association
        StrCpy $user_install_options "$user_install_options /emf"
        Goto skip_emf_association
        
no_emf_association:

        StrCpy $user_install_options "$user_install_options /noemf"

skip_emf_association:

        ; wmf Associations
        !insertmacro MUI_INSTALLOPTIONS_READ $R0 "InstallOptions2.ini" "Field 13" "State"
        strcmp $R0 "0" no_wmf_association
        StrCpy $user_install_options "$user_install_options /wmf"
        Goto skip_wmf_association
        
no_wmf_association:

        StrCpy $user_install_options "$user_install_options /nowmf"

skip_wmf_association:

        ; ----------------------------------
        ; begin voidImageViewer installation
        ; ----------------------------------
        
        SectionIn RO

        InitPluginsDir
        SetOutPath "$pluginsdir\voidImageViewer"

        ; write out files to copy.
        ; VS version and build config are configurable via defines

        File "${VIV_EXE_DIR}\voidImageViewer.exe"
        File "..\Changes.txt"
        WriteUninstaller "$pluginsdir\voidImageViewer\Uninstall.exe"

        ; check for command line options that will override the default install options.
        ${GetOptions} $CMDLINE "/install-options" $0
        IfErrors +2
        StrCpy $admin_install_options "$admin_install_options $0"

        ; install with admin rights.
        ; MessageBox MB_YESNOCANCEL|MB_ICONEXCLAMATION "ADMIN $admin_install_options"
        ClearErrors
        ExecWait '"$pluginsdir\voidImageViewer\voidImageViewer.exe" /install "$INSTDIR" /install-options "$admin_install_options"' $0
        IfErrors exec_admin_error
        IntCmp $0 0 exec_admin_ok
        
exec_admin_error:
        
        MessageBox MB_OK|MB_ICONSTOP "$(MsgExecAdminFailed)"

exec_admin_ok:

        ClearErrors
        ExecWait '"$INSTDIR\voidImageViewer.exe" $user_install_options' $0
        IfErrors exec_install_options_error
        IntCmp $0 0 exec_install_options_ok

exec_install_options_error:
        
        MessageBox MB_OK|MB_ICONSTOP "$(MsgExecOptionsFailed)"

exec_install_options_ok:

SectionEnd

Section "Uninstall"

        ; Make sure $InstDir is not the current directory so we can remove it
        SetOutPath $Temp
            
        ; copy voidImageViewer.exe to temp folder.      
        CopyFiles /SILENT $INSTDIR\voidImageViewer.exe $Temp\voidImageViewer.exe

        ; run uninstaller with admin rights
        ; this will uninstall any localized shortcuts etc..
        ; this also removes the service if installed
        ; which is something we can not do easily from the nsis installer.
        ; do this before we try to terminate the app.
    ExecWait '"$Temp\voidImageViewer.exe" /uninstall "$INSTDIR"'

        ; delete temp voidImageViewer
    Delete "$Temp\voidImageViewer.exe"

SectionEnd

Function InstallOptions

        !insertmacro INSTALLOPTIONS_INITDIALOG "InstallOptions.ini"

        !insertmacro MUI_HEADER_TEXT "$(MsgSelectOptionsTitle)" "$(MsgSelectOptionsSub)"
        
        !insertmacro INSTALLOPTIONS_SHOW

FunctionEnd

Function InstallOptions2
        
        !insertmacro INSTALLOPTIONS_INITDIALOG "InstallOptions2.ini"

        !insertmacro MUI_HEADER_TEXT "$(MsgSelectOptionsTitle)" "$(MsgSelectOptionsSub)"
        
        !insertmacro INSTALLOPTIONS_SHOW

FunctionEnd
