$c = Get-Content res\voidImageViewer.rc -Raw
$c = $c -replace '1,0,0,15', '1,0,1,0'
$c = $c -replace '1\.0\.0\.15', '1.0.1.0'
Set-Content res\voidImageViewer.rc $c -Encoding Default
