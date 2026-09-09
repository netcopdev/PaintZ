[CmdletBinding()]
param(
    [string]$AddonBuilderExe,
    [string]$OutputRoot = (Join-Path $env:LOCALAPPDATA "PaintZSandbox\@PaintZ-PaintPackApiFixture")
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Resolve-AddonBuilder {
    param([string]$ExplicitPath)

    if ($ExplicitPath) {
        if (-not (Test-Path -LiteralPath $ExplicitPath -PathType Leaf)) {
            throw "AddonBuilder.exe was not found at '$ExplicitPath'."
        }
        return (Resolve-Path -LiteralPath $ExplicitPath).Path
    }

    $candidates = @(
        "C:\Program Files (x86)\Steam\steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe",
        "C:\Program Files\Steam\steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe",
        "D:\SteamLibrary\steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe",
        "E:\SteamLibrary\steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe"
    )

    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    throw "AddonBuilder.exe was not found. Pass -AddonBuilderExe '<path>'."
}

$addonBuilder = Resolve-AddonBuilder $AddonBuilderExe
$sourceRoot = Join-Path $PSScriptRoot "paint-pack-api-fixture\PaintZ_PaintPackApiFixture"
if (-not (Test-Path -LiteralPath $sourceRoot -PathType Container)) {
    throw "Paint Pack API fixture source was not found at '$sourceRoot'."
}

$outputRootFull = [System.IO.Path]::GetFullPath($OutputRoot)
$addons = Join-Path $outputRootFull "Addons"
New-Item -ItemType Directory -Force -Path $addons | Out-Null

$expectedPbo = Join-Path $addons "PaintZ_PaintPackApiFixture.pbo"
if (Test-Path -LiteralPath $expectedPbo -PathType Leaf) {
    Remove-Item -LiteralPath $expectedPbo -Force
}

Write-Host "Fixture source : $sourceRoot"
Write-Host "Fixture mod    : $outputRootFull"
Write-Host "AddonBuilder   : $addonBuilder"

& $addonBuilder $sourceRoot $addons -clear -packonly
if ($LASTEXITCODE -ne 0) {
    throw "AddonBuilder failed with exit code $LASTEXITCODE."
}

if (-not (Test-Path -LiteralPath $expectedPbo -PathType Leaf)) {
    throw "AddonBuilder did not create '$expectedPbo'."
}

$modCpp = @'
name = "PaintZ Paint Pack API Fixture";
author = "PaintZ contributors";
version = "1";
'@
Set-Content -LiteralPath (Join-Path $outputRootFull "mod.cpp") -Value $modCpp -Encoding ASCII

Write-Host ""
Write-Host "Paint Pack API fixture ready:"
Write-Host "  $outputRootFull"
Write-Host ""
Write-Host "Load it after @PaintZ, for example by passing this directory through the sandbox -AdditionalMods parameter."
