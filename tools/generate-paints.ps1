[CmdletBinding()]
param(
    [string]$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
    [string]$ImageToPAA
)

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
$catalog = Get-Content -LiteralPath (Join-Path $generatorRoot "generated\catalog.json") -Raw | ConvertFrom-Json
$exported = @($catalog.paints)

# These directories contain generator-owned paint outputs. Remove obsolete PNGs
# now; retain the last usable PAAs until ImageToPAA availability is confirmed.
foreach ($outputPath in @($canOutput, $surfaceOutput)) {
    Get-ChildItem -LiteralPath $outputPath -File -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match '^pz_[a-z0-9_]+_co\.png$' } |
        Remove-Item -Force
}

foreach ($paint in $exported) {
    $pngName = $paint.texture_stem + "_co.png"
    $source = Join-Path $generatorRoot "generated\labels\$pngName"
    Copy-Item -LiteralPath $source -Destination (Join-Path $canOutput $pngName) -Force

    foreach ($variant in @($paint.surface_variants)) {
        $surfaceName = $variant.texture_stem + "_co.png"
        $surfaceSource = Join-Path $generatorRoot "generated\surfaces\$surfaceName"
        Copy-Item -LiteralPath $surfaceSource -Destination (Join-Path $surfaceOutput $surfaceName) -Force
    }
}

if (-not $ImageToPAA) {
    $candidates = @(
        "E:\SteamLibrary\steamapps\common\DayZ Tools\Bin\ImageToPAA\ImageToPAA.exe",
        "C:\Program Files (x86)\Steam\steamapps\common\DayZ Tools\Bin\ImageToPAA\ImageToPAA.exe",
        "C:\Program Files\Steam\steamapps\common\DayZ Tools\Bin\ImageToPAA\ImageToPAA.exe"
    )
    $ImageToPAA = $candidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
}
if (-not $ImageToPAA -or -not (Test-Path -LiteralPath $ImageToPAA -PathType Leaf)) {
    throw "ImageToPAA.exe was not found. Pass -ImageToPAA '<path-to-ImageToPAA.exe>'."
}

# Conversion is available, so stale generated PAAs can now be removed safely.
foreach ($outputPath in @($canOutput, $surfaceOutput)) {
    Get-ChildItem -LiteralPath $outputPath -File -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match '^pz_[a-z0-9_]+_co\.paa$' } |
        Remove-Item -Force
}

foreach ($paint in $exported) {
    $stem = $paint.texture_stem + "_co"
    $png = Join-Path $canOutput ($stem + ".png")
    $paa = Join-Path $canOutput ($stem + ".paa")
    & $ImageToPAA $png $paa
    if ($LASTEXITCODE -ne 0) {
        throw "ImageToPAA failed for '$png'."
    }

    foreach ($variant in @($paint.surface_variants)) {
        $surfaceStem = $variant.texture_stem + "_co"
        $surfacePng = Join-Path $surfaceOutput ($surfaceStem + ".png")
        $surfacePaa = Join-Path $surfaceOutput ($surfaceStem + ".paa")
        & $ImageToPAA $surfacePng $surfacePaa
        if ($LASTEXITCODE -ne 0) {
            throw "ImageToPAA failed for '$surfacePng'."
        }
    }
}

$surfaceCount = 0
foreach ($paint in $exported) {
    $surfaceCount += @($paint.surface_variants).Count
}

Write-Host "paintzgen exported $($exported.Count) can textures and $surfaceCount surface textures"
