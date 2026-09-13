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
# Validate (and fix) the encodings required by the bilingual installer:
#   - installer.nsi and installer_license_Chinese.txt:
#       UTF-8 with BOM (read by makensis at compile time).
#   - InstallOptions*_Chinese.ini:
#       UTF-16LE with BOM (read by the InstallOptions plugin at runtime,
#       non-unicode ini files would be converted with the target system's
#       ansi codepage and the chinese text would be corrupted).

$ErrorActionPreference = "Stop"

function Ensure-Utf8Bom {
    param([string]$Path)
    $bytes = [System.IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF) {
        Write-Host "  OK (UTF-8 BOM): $Path" -ForegroundColor Green
        return
    }
    Write-Host "  fixing (adding UTF-8 BOM): $Path" -ForegroundColor Yellow
    $content = [System.Text.Encoding]::UTF8.GetString($bytes)
    [System.IO.File]::WriteAllText($Path, $content, [System.Text.UTF8Encoding]::new($true))
}

function Ensure-Utf16LeBom {
    param([string]$Path)
    $bytes = [System.IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -ge 2 -and $bytes[0] -eq 0xFF -and $bytes[1] -eq 0xFE) {
        Write-Host "  OK (UTF-16LE BOM): $Path" -ForegroundColor Green
        return
    }
    Write-Host "  fixing (converting to UTF-16LE BOM): $Path" -ForegroundColor Yellow
    $content = [System.Text.Encoding]::UTF8.GetString($bytes)
    [System.IO.File]::WriteAllText($Path, $content, [System.Text.UnicodeEncoding]::new($false, $true))
}

Write-Host "Validating installer file encodings..." -ForegroundColor Cyan

Ensure-Utf8Bom -Path (Join-Path $PSScriptRoot "installer.nsi")
Ensure-Utf8Bom -Path (Join-Path $PSScriptRoot "installer_license_Chinese.txt")
Ensure-Utf16LeBom -Path (Join-Path $PSScriptRoot "InstallOptions_Chinese.ini")
Ensure-Utf16LeBom -Path (Join-Path $PSScriptRoot "InstallOptions2_Chinese.ini")

Write-Host "Encoding validation completed!" -ForegroundColor Green
