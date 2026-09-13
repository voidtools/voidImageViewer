#
# Copyright 2026 hesphoros
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Build the unified bilingual (English + Simplified Chinese) NSIS installer
# for void Image Viewer. The language is selected inside the installer, there
# is no separate per-language build.
#
# Usage: .\build_installer.ps1 [arch] [vs_version] [build_config]
#   arch: x86 or x64 (default: x86)
#   vs_version: vs2019, vs2026 (default: auto-detect:
#               prefers a project dir with a built exe, then vswhere)
#   build_config: Release, Debug, etc. (default: Release)
#
# Examples:
#   .\build_installer.ps1 x64 vs2026 Release
#   .\build_installer.ps1 x86
#   .\build_installer.ps1 x64 vs2019 Release

param(
    [string]$Arch = "x86",
    [string]$VsVersion = "",
    [string]$BuildConfig = "Release"
)

# Change to script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

# Check if NSIS is installed
$makensis = Get-Command makensis.exe -ErrorAction SilentlyContinue
if (-not $makensis) {
    # also try the default install location
    $defaultMakensis = "C:\Program Files (x86)\NSIS\makensis.exe"
    if (Test-Path $defaultMakensis) {
        $makensis = $defaultMakensis
    }
}
if (-not $makensis) {
    Write-Host "Error: NSIS compiler (makensis.exe) not found!" -ForegroundColor Red
    Write-Host "Please install NSIS from https://nsis.sourceforge.io/Download" -ForegroundColor Yellow
    Write-Host "and add it to your system PATH." -ForegroundColor Yellow
    exit 1
}

# Check if version.nsh exists
if (-not (Test-Path "version.nsh")) {
    Write-Host "Error: version.nsh not found!" -ForegroundColor Red
    Write-Host "Please create version.nsh file first." -ForegroundColor Yellow
    exit 1
}

# Validate and fix the encoding of the bilingual installer files
if (Test-Path "ensure_encodings.ps1") {
    Write-Host "Validating installer file encodings..." -ForegroundColor Cyan
    & powershell -ExecutionPolicy Bypass -File "ensure_encodings.ps1" | Out-Null
}

# Auto-detect VS version if not specified.
# NOTE: a plain directory existence check says nothing about which toolchain
# the user actually has installed. Detection therefore prefers, in order:
#   1. a project directory that already contains a built executable for the
#      requested arch/config (packaging what was actually built),
#   2. an installed Visual Studio instance reported by vswhere (VS2017+).
if ([string]::IsNullOrEmpty($VsVersion)) {
    Write-Host "Auto-detecting Visual Studio version..." -ForegroundColor Cyan

    $detected = $null

    # 1) prefer a project directory that already has a built executable
    foreach ($vs in @("vs2026", "vs2019")) {
        $candidate = "..\$vs\$BuildConfig\voidImageViewer.exe"
        if ($Arch -eq "x64") {
            $candidate = "..\$vs\x64\$BuildConfig\voidImageViewer.exe"
        }
        if (Test-Path $candidate) {
            $detected = $vs
            break
        }
    }

    # 2) fall back to the installed toolchain (vswhere knows VS2017+)
    if (-not $detected) {
        $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
        if (Test-Path $vswhere) {
            # newest first: vs2026 = VS 18.x, vs2019 = VS 16.x
            foreach ($vs in @("vs2026", "vs2019")) {
                $major = "16"
                if ($vs -eq "vs2026") { $major = "18" }
                $versions = & $vswhere -products * -requires Microsoft.Component.MSBuild -property installationVersion 2>$null
                if ($versions | Where-Object { $_.StartsWith("$major.") }) {
                    if (Test-Path "..\$vs") {
                        $detected = $vs
                        break
                    }
                }
            }
        }
    }

    if ($detected) {
        $VsVersion = $detected
        Write-Host "Detected: $VsVersion" -ForegroundColor Green
    }
    else {
        Write-Host "Error: Could not auto-detect Visual Studio version!" -ForegroundColor Red
        Write-Host "No built executable was found and no matching Visual Studio installation was detected." -ForegroundColor Yellow
        Write-Host "Please build the solution first, or specify VS version manually:" -ForegroundColor Yellow
        Write-Host "  .\build_installer.ps1 -Arch $Arch -VsVersion vs2026" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Available VS directories:"
        if (Test-Path "..\vs2026") { Write-Host "  - vs2026" }
        if (Test-Path "..\vs2019") { Write-Host "  - vs2019" }
        exit 1
    }
}

# Determine executable path based on architecture
if ($Arch -eq "x64") {
    $ExePath = "..\$VsVersion\x64\$BuildConfig\voidImageViewer.exe"
    $ConfigName = "Release|x64"
}
else {
    $ExePath = "..\$VsVersion\$BuildConfig\voidImageViewer.exe"
    $ConfigName = "Release|Win32"
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Building void Image Viewer Installer" -ForegroundColor Cyan
Write-Host "Architecture: $Arch" -ForegroundColor White
Write-Host "Visual Studio: $VsVersion" -ForegroundColor White
Write-Host "Build Config: $BuildConfig" -ForegroundColor White
Write-Host "Languages: English + Simplified Chinese (selected in the installer)" -ForegroundColor White
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if voidImageViewer.exe exists
Write-Host "Checking for compiled executable..." -ForegroundColor Cyan
if (-not (Test-Path $ExePath)) {
    Write-Host ""
    Write-Host "Error: voidImageViewer.exe not found!" -ForegroundColor Red
    Write-Host "Expected location: $ExePath" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Please build the application first:" -ForegroundColor Yellow
    Write-Host "  1. Open $VsVersion\voidImageViewer.sln in Visual Studio" -ForegroundColor Yellow
    Write-Host "  2. Select configuration: $ConfigName" -ForegroundColor Yellow
    Write-Host "  3. Build the solution (Build -> Build Solution)" -ForegroundColor Yellow
    Write-Host ""
    exit 1
}
Write-Host "Found: $ExePath" -ForegroundColor Green
Write-Host ""

# Compile NSIS script
Write-Host "Compiling NSIS installer script..." -ForegroundColor Cyan
$makensisArgs = @(
    "/DVS_VERSION=$VsVersion",
    "/DBUILD_CONFIG=$BuildConfig"
)

if ($Arch -eq "x64") {
    $makensisArgs += "/Dx64"
}

$makensisArgs += "installer.nsi"

Write-Host "Command: makensis.exe $($makensisArgs -join ' ')" -ForegroundColor Gray
Write-Host ""

$process = Start-Process -FilePath $makensis -ArgumentList $makensisArgs -Wait -NoNewWindow -PassThru

if ($process.ExitCode -eq 0) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "Build completed successfully!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "The installer should be in the current directory." -ForegroundColor Green
    Write-Host "The user picks the setup language on the first page." -ForegroundColor Green
    exit 0
}
else {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "Build failed!" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    exit 1
}
