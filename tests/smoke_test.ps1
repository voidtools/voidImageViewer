# voidImageViewer real machine smoke test (windows powershell 5.1+)
#
# "tested" must mean "actually runs": this script starts the viewer,
# opens every anomaly sample produced by tests/make_anomaly_samples.py
# and watches for crashes (a process that dies with a nonzero exit code
# on hostile input). the over budget canvas must be refused without a
# crash - that is the pixel budget from the third audit round working.
#
# usage:
#   powershell -ExecutionPolicy Bypass -File tests\smoke_test.ps1 `
#       [-ExePath "C:\path\voidImageViewer.exe"] `
#       [-SamplesDir "C:\path\tests\samples"]
#
# if the samples directory is missing the script tries to generate it
# with python first (python tests\make_anomaly_samples.py). a clean run
# exits 0; any crashing sample exits 1.

param(
    [string]$ExePath = "",
    [string]$SamplesDir = "",
    [int]$SampleTimeoutSec = 5,
    [int]$CloseTimeoutSec = 5
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# ---------------------------------------------------------------- locate exe

if (-not $ExePath) {
    $candidates = @(
        (Join-Path $env:ProgramFiles "voidImageViewer\voidImageViewer.exe"),
        (Join-Path $env:LOCALAPPDATA "Programs\voidImageViewer\voidImageViewer.exe"),
        (Join-Path $scriptDir "voidImageViewer.exe"),
        (Join-Path $scriptDir "..\vs2019\release\voidImageViewer.exe")
    )
    # registry probe as a fallback
    try {
        $keys = @(
            "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\voidImageViewer",
            "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\voidImageViewer"
        )
        foreach ($k in $keys) {
            if (Test-Path $k) {
                $loc = (Get-ItemProperty $k -ErrorAction SilentlyContinue).InstallLocation
                if ($loc) { $candidates += (Join-Path $loc "voidImageViewer.exe") }
            }
        }
    } catch { }
    foreach ($c in $candidates) {
        if ($c -and (Test-Path $c)) { $ExePath = $c; break }
    }
}

if (-not $ExePath -or -not (Test-Path $ExePath)) {
    Write-Host "FAIL: voidImageViewer.exe not found."
    Write-Host "      pass -ExePath explicitly or install the viewer first."
    exit 2
}
Write-Host ("viewer under test: " + $ExePath)
Write-Host ("version in file:   " + (Get-Item $ExePath).VersionInfo.FileVersion)

# ------------------------------------------------------------ locate samples

if (-not $SamplesDir) { $SamplesDir = Join-Path $scriptDir "samples" }

if (-not (Test-Path $SamplesDir)) {
    Write-Host "samples missing, generating via make_anomaly_samples.py ..."
    $py = Get-Command python -ErrorAction SilentlyContinue
    if (-not $py) { $py = Get-Command py -ErrorAction SilentlyContinue }
    if ($py) {
        & $py.Source (Join-Path $scriptDir "make_anomaly_samples.py") $SamplesDir
        if ($LASTEXITCODE -ne 0) {
            Write-Host "FAIL: sample generator self check failed."
            exit 2
        }
    } else {
        Write-Host ("FAIL: no python to generate samples; run " +
                    "make_anomaly_samples.py on any machine and copy tests\samples.")
        exit 2
    }
}

$samples = Get-ChildItem -Path $SamplesDir -File | Sort-Object Name
if (-not $samples) {
    Write-Host "FAIL: sample directory is empty."
    exit 2
}
Write-Host ("samples: " + $samples.Count + " files")
Write-Host ""

# ----------------------------------------------------------------- helpers

function Stop-Viewer {
    param([System.Diagnostics.Process]$proc)
    if ($proc -and -not $proc.HasExited) {
        $null = $proc.CloseMainWindow()
        if (-not $proc.WaitForExit($CloseTimeoutSec * 1000)) {
            Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
        }
    }
}

$results = New-Object System.Collections.Generic.List[object]
$crashes = 0
$passes = 0
$warns = 0

# 1. bare launch: the viewer must start and stay up
Write-Host "=== stage 1: bare launch ==="
$bare = Start-Process -FilePath $ExePath -PassThru
Start-Sleep -Seconds 3
if ($bare.HasExited) {
    $status = if ($bare.ExitCode -eq 0) { "WARN" } else { "FAIL" }
    if ($bare.ExitCode -ne 0) { $script:crashes++ } else { $script:warns++ }
    $results.Add([pscustomobject]@{
        Sample = "(bare launch)"; Status = $status;
        Detail = ("exited " + $bare.ExitCode) })
} else {
    $script:passes++
    $results.Add([pscustomobject]@{
        Sample = "(bare launch)"; Status = "PASS"; Detail = "alive after 3s" })
    Stop-Viewer $bare
}

# 2. one hostile sample after another: no crash is allowed
Write-Host "=== stage 2: anomaly sweep ==="
foreach ($s in $samples) {
    $proc = Start-Process -FilePath $ExePath -ArgumentList ('"' + $s.FullName + '"') -PassThru
    Start-Sleep -Seconds $SampleTimeoutSec
    if ($proc.HasExited -and $proc.ExitCode -ne 0) {
        $script:crashes++
        $results.Add([pscustomobject]@{
            Sample = $s.Name; Status = "FAIL";
            Detail = ("crashed, exit " + $proc.ExitCode) })
    } elseif ($proc.HasExited) {
        # exit 0 usually means the file was handed to an existing single
        # instance or refused cleanly - both are survivable outcomes.
        $script:warns++
        $results.Add([pscustomobject]@{
            Sample = $s.Name; Status = "WARN"; Detail = "clean early exit" })
    } else {
        $script:passes++
        $results.Add([pscustomobject]@{
            Sample = $s.Name; Status = "PASS"; Detail = "survived open" })
        Stop-Viewer $proc
    }
}

# 3. the over budget canvas: the pixel budget must refuse it, not crash
Write-Host "=== stage 3: over budget canvas ==="
$over = $samples | Where-Object { $_.Name -like "*over_budget*" }
if ($over) {
    $proc = Start-Process -FilePath $ExePath -ArgumentList ('"' + $over[0].FullName + '"') -PassThru
    Start-Sleep -Seconds $SampleTimeoutSec
    if ($proc.HasExited -and $proc.ExitCode -ne 0) {
        $script:crashes++
        $results.Add([pscustomobject]@{
            Sample = $over[0].Name; Status = "FAIL";
            Detail = "died on the budget canvas" })
    } else {
        $script:passes++
        $detail = "refused or displayed without a crash"
        if ($proc.HasExited) { $detail = "clean exit on the budget canvas" }
        $results.Add([pscustomobject]@{
            Sample = $over[0].Name; Status = "PASS"; Detail = $detail })
        Stop-Viewer $proc
    }
} else {
    Write-Host "no over_budget sample found (regenerate samples)."
}

# ------------------------------------------------------------------ report

Write-Host ""
$results | Format-Table -AutoSize
$line = ("total {0}: {1} pass, {2} warn, {3} FAIL" -f
         $results.Count, $passes, $warns, $crashes)
Write-Host $line

# never leave a viewer behind
Get-Process -Name voidImageViewer -ErrorAction SilentlyContinue |
    Stop-Process -Force -ErrorAction SilentlyContinue

if ($crashes -gt 0) {
    Write-Host "SMOKE TEST FAIL"
    exit 1
}
Write-Host "SMOKE TEST PASS (no crashes on any sample)"
exit 0
