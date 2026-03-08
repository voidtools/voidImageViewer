;
; Copyright 2025 voidtools / David Carpenter
; 
; Multi-language and multi-VS version support added by hesphoros (2026)
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
; Supported versions: vs2005, vs2019, vs2026, etc.
; Added by hesphoros (2026)
!ifndef VS_VERSION
	!define VS_VERSION "vs2026"  ; Default VS version
!endif

; Build configuration (Release, Debug, etc.)
; Added by hesphoros (2026)
!ifndef BUILD_CONFIG
	!define BUILD_CONFIG "Release"  ; Default build configuration
!endif

; Language configuration
; Can be overridden via command line: makensis.exe /DLANG=Chinese installer.nsi
; Supported languages: English, Chinese
; Added by hesphoros (2026)
!ifndef LANG
	!define LANG "Chinese"  ; Default language
!endif

; we need admin access to write to program files and registry (associations).
RequestExecutionLevel user

CRCCheck On
XPStyle on

; includes
!include "MUI.nsh"

!include "version.nsh"

!include WinMessages.nsh
!include InstallOptions.nsh
!include FileFunc.nsh

; Language-specific file names
!if "${LANG}" == "Chinese"
	!define LICENSE_FILE "installer_license_Chinese.txt"
	!define INSTALL_OPTIONS_FILE "InstallOptions_Chinese.ini"
	!define INSTALL_OPTIONS2_FILE "InstallOptions2_Chinese.ini"
	!define LANG_CODE "zh-CN"
	!define LANG_NAME "Chinese"
!else
	!define LICENSE_FILE "installer_license_English.txt"
	!define INSTALL_OPTIONS_FILE "InstallOptions.ini"
	!define INSTALL_OPTIONS2_FILE "InstallOptions2.ini"
	!define LANG_CODE "en-US"
	!define LANG_NAME "English"
!endif

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

BrandingText "void Image Viewer ${VERSION}${BETAVERSION} (${TARGETMACHINE}) Setup"

; settings /SOLID will save a few KBs
SetCompressor /SOLID lzma
Name "void Image Viewer"

; Output file name with language code
OutFile "voidImageViewer-${VERSION}${BETAVERSION}.${TARGETMACHINE}.${LANG_CODE}-Setup.exe"
	
; MUI settings
!define MUI_ICON "..\res\voidImageViewer.ico"

!insertmacro MUI_PAGE_LICENSE "${LICENSE_FILE}"

!insertmacro MUI_PAGE_DIRECTORY

; options page
Page custom InstallOptions
Page custom InstallOptions2

!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_PAGE_FINISH

;!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Language selection
!if "${LANG}" == "Chinese"
	!insertmacro MUI_LANGUAGE "SimpChinese"
!else
	!insertmacro MUI_LANGUAGE "English"
!endif

!insertmacro GetOptions

; Version Info
VIProductVersion "${VERSION}${BETAVERSION}"

; don't localize these:
VIAddVersionKey "ProductName" "void Image Viewer"
VIAddVersionKey "Comments" ""
VIAddVersionKey "CompanyName" ""
VIAddVersionKey "LegalTrademarks" ""
VIAddVersionKey "LegalCopyright" "Copyright (c) 2025 David Carpenter"
VIAddVersionKey "FileDescription" "void Image Viewer Setup"

VIAddVersionKey "FileVersion" "${VERSION}${BETAVERSION}.${TARGETMACHINE}.${LANG_CODE}"
VIAddVersionKey "ProductVersion" "${VERSION}${BETAVERSION}.${TARGETMACHINE}.${LANG_CODE}"

Function .onInit

	; init options with language-specific files.
	!insertmacro INSTALLOPTIONS_EXTRACT "${INSTALL_OPTIONS_FILE}"
	!insertmacro INSTALLOPTIONS_EXTRACT "${INSTALL_OPTIONS2_FILE}"

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
	!insertmacro MUI_INSTALLOPTIONS_WRITE "${INSTALL_OPTIONS_FILE}" "Field 2" "State" "0"
	!insertmacro MUI_INSTALLOPTIONS_WRITE "${INSTALL_OPTIONS_FILE}" "Field 3" "State" "1"

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
	
	MessageBox MB_YESNOCANCEL|MB_ICONEXCLAMATION "OS is not x64.$\nInstall anyway?" IDYES is64
	Abort
	
is64:
	
!endif ; !ifdef x64

FunctionEnd

Function un.onInit

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
	!insertmacro MUI_INSTALLOPTIONS_READ $R0 "${INSTALL_OPTIONS_FILE}" "Field 2" "State"
	strcmp $R0 "0" no_app_data
	StrCpy $admin_install_options "$admin_install_options /appdata"
	Goto skip_app_data

no_app_data:

	; this option is special, we MUST unset any option that voidImageViewer was previously installed with.
	StrCpy $admin_install_options "$admin_install_options /noappdata"
	
skip_app_data:	

	; startmenu
	!insertmacro MUI_INSTALLOPTIONS_READ $R0 "${INSTALL_OPTIONS2_FILE}" "Field 1" "State"
	strcmp $R0 "0" no_startmenu
	StrCpy $admin_install_options "$admin_install_options /startmenu"
	Goto skip_startmenu
	
no_startmenu:

	StrCpy $admin_install_options "$admin_install_options /nostartmenu"

skip_startmenu:

	; BMP Associations
	!insertmacro MUI_INSTALLOPTIONS_READ $R0 "${INSTALL_OPTIONS2_FILE}" "Field 3" "State"
	strcmp $R0 "0" no_bmp_association
	StrCpy $user_install_options "$user_install_options /bmp"
	Goto skip_bmp_association
	
no_bmp_association:

	StrCpy $user_install_options "$user_install_options /nobmp"

skip_bmp_association:

	; GIF Associations
	!insertmacro MUI_INSTALLOPTIONS_READ $R0 "${INSTALL_OPTIONS2_FILE}" "Field 4" "State"
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

	; ----------------------------------
	; begin voidImageViewer installation
	; ----------------------------------
	
	SectionIn RO

	InitPluginsDir
	SetOutPath "$pluginsdir\voidImageViewer"

	; write out files to copy.
	; VS version and build config are configurable via defines

	!ifdef x64
	
		File "..\${VS_VERSION}\x64\${BUILD_CONFIG}\voidImageViewer.exe"
		
	!else
	
		File "..\${VS_VERSION}\${BUILD_CONFIG}\voidImageViewer.exe"
		
	!endif

	; File "..\Changes.txt"
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
	
	MessageBox MB_OK|MB_ICONSTOP "Failed to execute admin command"

exec_admin_ok:

	ClearErrors
	ExecWait '"$INSTDIR\voidImageViewer.exe" $user_install_options' $0
	IfErrors exec_install_options_error
	IntCmp $0 0 exec_install_options_ok

exec_install_options_error:
	
	MessageBox MB_OK|MB_ICONSTOP "Failed to execute install options"

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

	!insertmacro INSTALLOPTIONS_INITDIALOG "${INSTALL_OPTIONS_FILE}"

	!insertmacro MUI_HEADER_TEXT "Select Install Options" "Choose any additional install options."
	
	!insertmacro INSTALLOPTIONS_SHOW

FunctionEnd

Function InstallOptions2
	
	!insertmacro INSTALLOPTIONS_INITDIALOG "${INSTALL_OPTIONS2_FILE}"

	!insertmacro MUI_HEADER_TEXT "Select Install Options" "Choose any additional install options."
	
	!insertmacro INSTALLOPTIONS_SHOW

FunctionEnd

