[CmdletBinding()]
param(
    [string]$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
    [string]$ImageToPAA
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
$generatorRoot = Join-Path $ProjectRoot "tools\paintzgen"
$generator = Join-Path $generatorRoot "tools\generate_paints.py"
$canOutput = Join-Path $ProjectRoot "data\cans"
$surfaceOutput = Join-Path $ProjectRoot "data\surfaces"
$uvCache = Join-Path $ProjectRoot ".tmp\uv-cache"
$uvPython = Join-Path $ProjectRoot ".tmp\uv-python"

if (-not (Test-Path -LiteralPath $generator -PathType Leaf)) {
    throw "paintzgen was not found at '$generator'."
}

$python = Get-Command python -ErrorAction SilentlyContinue
$pythonHasPillow = $false
if ($python) {
    try {
        & $python.Source -c "import PIL" 2>$null
        $pythonHasPillow = ($LASTEXITCODE -eq 0)
    }
    catch {
        $pythonHasPillow = $false
    }
}

if ($pythonHasPillow) {
    & $python.Source $generator --clean
}
else {
    $uv = Get-Command uv -ErrorAction SilentlyContinue
    if (-not $uv) {
        throw "paintzgen requires Python with Pillow. Install Python, or install uv so the build can run it."
    }

    New-Item -ItemType Directory -Force -Path $uvCache | Out-Null
    New-Item -ItemType Directory -Force -Path $uvPython | Out-Null
    $env:UV_CACHE_DIR = $uvCache
    $env:UV_PYTHON_INSTALL_DIR = $uvPython
    & $uv.Source run --with "Pillow>=10.0,<13" $generator --clean
}

if ($LASTEXITCODE -ne 0) {
    throw "paintzgen failed with exit code $LASTEXITCODE."
}

New-Item -ItemType Directory -Force -Path $canOutput | Out-Null
New-Item -ItemType Directory -Force -Path $surfaceOutput | Out-Null

$catalogPath = Join-Path $generatorRoot "generated\catalog.json"
if (-not (Test-Path -LiteralPath $catalogPath -PathType Leaf)) {
    throw "paintzgen did not create '$catalogPath'."
}

$catalog = Get-Content -LiteralPath $catalogPath -Raw | ConvertFrom-Json
$exported = @($catalog.paints)

if (-not $ImageToPAA) {
    $candidates = @(
        "E:\SteamLibrary\steamapps\common\DayZ Tools\Bin\ImageToPAA\ImageToPAA.exe",
        "D:\SteamLibrary\steamapps\common\DayZ Tools\Bin\ImageToPAA\ImageToPAA.exe",
        "C:\Program Files (x86)\Steam\steamapps\common\DayZ Tools\Bin\ImageToPAA\ImageToPAA.exe",
        "C:\Program Files\Steam\steamapps\common\DayZ Tools\Bin\ImageToPAA\ImageToPAA.exe"
    )
    $ImageToPAA = $candidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
}

if (-not $ImageToPAA -or -not (Test-Path -LiteralPath $ImageToPAA -PathType Leaf)) {
    throw "ImageToPAA.exe was not found. Pass -ImageToPAA '<path-to-ImageToPAA.exe>'."
}

# data/ is runtime-only. Remove any old generator intermediates left by older
# builds, plus stale PaintZ-generated PAAs. Source PNGs remain under
# tools/paintzgen/generated and are converted directly from there.
foreach ($outputPath in @($canOutput, $surfaceOutput)) {
    Get-ChildItem -LiteralPath $outputPath -File -ErrorAction SilentlyContinue |
        Where-Object {
            $_.Name -match '^pz_[a-z0-9_]+_co\.(png|paa)$'
        } |
        Remove-Item -Force
}

$canCount = 0
$surfaceCount = 0

foreach ($paint in $exported) {
    $stem = $paint.texture_stem + "_co"
    $sourcePng = Join-Path $generatorRoot ("generated\labels\" + $stem + ".png")
    $destinationPaa = Join-Path $canOutput ($stem + ".paa")

    if (-not (Test-Path -LiteralPath $sourcePng -PathType Leaf)) {
        throw "Generated can texture was not found: '$sourcePng'."
    }

    & $ImageToPAA $sourcePng $destinationPaa
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $destinationPaa -PathType Leaf)) {
        throw "ImageToPAA failed for '$sourcePng'."
    }
    $canCount++

    foreach ($variant in @($paint.surface_variants)) {
        $surfaceStem = $variant.texture_stem + "_co"
        $surfaceSourcePng = Join-Path $generatorRoot ("generated\surfaces\" + $surfaceStem + ".png")
        $surfaceDestinationPaa = Join-Path $surfaceOutput ($surfaceStem + ".paa")

        if (-not (Test-Path -LiteralPath $surfaceSourcePng -PathType Leaf)) {
            throw "Generated surface texture was not found: '$surfaceSourcePng'."
        }

        & $ImageToPAA $surfaceSourcePng $surfaceDestinationPaa
        if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $surfaceDestinationPaa -PathType Leaf)) {
            throw "ImageToPAA failed for '$surfaceSourcePng'."
        }
        $surfaceCount++
    }
}

$runtimePngs = @(Get-ChildItem -LiteralPath (Join-Path $ProjectRoot 'data') -Recurse -Filter *.png -File -ErrorAction SilentlyContinue)
if ($runtimePngs.Count -gt 0) {
    $paths = $runtimePngs | ForEach-Object { $_.FullName }
    throw "Runtime data directory contains PNG intermediates after generation:`n  $($paths -join "`n  ")"
}

$paaFiles = @(Get-ChildItem -LiteralPath (Join-Path $ProjectRoot 'data') -Recurse -Filter *.paa -File -ErrorAction SilentlyContinue)
$paaBytes = ($paaFiles | Measure-Object Length -Sum).Sum

Write-Host "paintzgen exported $canCount can textures and $surfaceCount surface textures"
Write-Host ("runtime PAA payload: {0} files, {1:N2} MB" -f $paaFiles.Count, ($paaBytes / 1MB))
