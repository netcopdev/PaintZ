param(
    [string]$AddonBuilder,
    [string]$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
    [string]$OutputDir = (Join-Path (Resolve-Path (Join-Path $PSScriptRoot "..")).Path "dist"),
    [string]$ImageToPAA,
    [switch]$SkipPaintZGen
)

$ErrorActionPreference = "Stop"

if (-not $SkipPaintZGen) {
    & (Join-Path $PSScriptRoot "generate-paints.ps1") -ProjectRoot $ProjectRoot -ImageToPAA $ImageToPAA
}

if (-not $AddonBuilder) {
    $candidates = @(
        "C:\Program Files (x86)\Steam\steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe",
        "C:\Program Files\Steam\steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe"
    )

    $AddonBuilder = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
}

if (-not $AddonBuilder -or -not (Test-Path $AddonBuilder)) {
    throw "AddonBuilder.exe not found. Pass -AddonBuilder '<path-to-AddonBuilder.exe>'."
}

New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

Write-Host "PaintZ source : $ProjectRoot"
Write-Host "Output      : $OutputDir"
Write-Host "AddonBuilder: $AddonBuilder"
Write-Host ""
Write-Host "NOTE: Verify these AddonBuilder arguments against your existing DayZ Tools workflow before relying on this helper."

& $AddonBuilder $ProjectRoot $OutputDir -clear -packonly "-include=$ProjectRoot\include.lst"
if ($LASTEXITCODE -ne 0) {
    throw "AddonBuilder failed with exit code $LASTEXITCODE"
}
