param(
    [string]$AddonBuilder,
    [string]$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
    [string]$OutputDir = (Join-Path (Resolve-Path (Join-Path $PSScriptRoot "..")).Path "dist"),
    [string]$ImageToPAA,
    [switch]$SkipPaintZGen
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Copy-RuntimeFile {
    param(
        [Parameter(Mandatory = $true)][string]$Source,
        [Parameter(Mandatory = $true)][string]$RelativeDestination,
        [Parameter(Mandatory = $true)][string]$StageRoot
    )

    if (-not (Test-Path -LiteralPath $Source -PathType Leaf)) {
        throw "Required runtime file was not found: '$Source'."
    }

    $destination = Join-Path $StageRoot $RelativeDestination
    $destinationDirectory = Split-Path -Parent $destination
    New-Item -ItemType Directory -Force -Path $destinationDirectory | Out-Null
    Copy-Item -LiteralPath $Source -Destination $destination -Force
}

function Copy-RuntimeTree {
    param(
        [Parameter(Mandatory = $true)][string]$SourceRoot,
        [Parameter(Mandatory = $true)][string]$RelativeDestination,
        [Parameter(Mandatory = $true)][string]$StageRoot,
        [Parameter(Mandatory = $true)][string[]]$AllowedExtensions
    )

    if (-not (Test-Path -LiteralPath $SourceRoot -PathType Container)) {
        throw "Required runtime directory was not found: '$SourceRoot'."
    }

    $files = Get-ChildItem -LiteralPath $SourceRoot -Recurse -File
    foreach ($file in $files) {
        $extension = $file.Extension.ToLowerInvariant()
        if ($AllowedExtensions -notcontains $extension) {
            continue
        }

        $relative = $file.FullName.Substring($SourceRoot.Length).TrimStart('\', '/')
        $destinationRelative = Join-Path $RelativeDestination $relative
        Copy-RuntimeFile -Source $file.FullName -RelativeDestination $destinationRelative -StageRoot $StageRoot
    }
}

function Assert-RuntimeStage {
    param(
        [Parameter(Mandatory = $true)][string]$StageRoot
    )

    $forbiddenExtensions = @(
        '.png', '.jpg', '.jpeg', '.webp', '.bmp', '.tga',
        '.py', '.pyc', '.pyo', '.ps1', '.psm1', '.psd', '.xcf', '.svg',
        '.md', '.ttf', '.otf', '.zip', '.7z'
    )

    $forbidden = Get-ChildItem -LiteralPath $StageRoot -Recurse -File | Where-Object {
        $forbiddenExtensions -contains $_.Extension.ToLowerInvariant()
    }

    if ($forbidden) {
        $paths = $forbidden | ForEach-Object { $_.FullName.Substring($StageRoot.Length).TrimStart('\', '/') }
        throw "Runtime staging contains forbidden development assets:`n  $($paths -join "`n  ")"
    }
}

if (-not $SkipPaintZGen) {
    & (Join-Path $PSScriptRoot "generate-paints.ps1") -ProjectRoot $ProjectRoot -ImageToPAA $ImageToPAA
    if (-not $?) {
        throw "PaintZ asset generation failed."
    }
}

if (-not $AddonBuilder) {
    $candidates = @(
        "C:\Program Files (x86)\Steam\steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe",
        "C:\Program Files\Steam\steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe",
        "D:\SteamLibrary\steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe",
        "E:\SteamLibrary\steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe"
    )

    $AddonBuilder = $candidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
}

if (-not $AddonBuilder -or -not (Test-Path -LiteralPath $AddonBuilder -PathType Leaf)) {
    throw "AddonBuilder.exe not found. Pass -AddonBuilder '<path-to-AddonBuilder.exe>'."
}

$projectRootFull = (Resolve-Path -LiteralPath $ProjectRoot).Path
$outputDirFull = [System.IO.Path]::GetFullPath($OutputDir)
New-Item -ItemType Directory -Force -Path $outputDirFull | Out-Null

$projectName = Split-Path -Leaf $projectRootFull.TrimEnd('\')
$stagingParent = Join-Path ([System.IO.Path]::GetTempPath()) ("PaintZ-AddonSource-" + [guid]::NewGuid().ToString("N"))
$stagedProjectRoot = Join-Path $stagingParent $projectName
$expectedPbo = Join-Path $outputDirFull "$projectName.pbo"
$previousPboWriteTime = $null
if (Test-Path -LiteralPath $expectedPbo -PathType Leaf) {
    $previousPboWriteTime = (Get-Item -LiteralPath $expectedPbo).LastWriteTimeUtc
}

New-Item -ItemType Directory -Force -Path $stagedProjectRoot | Out-Null

Write-Host "PaintZ source : $projectRootFull"
Write-Host "Build staging : $stagedProjectRoot"
Write-Host "Output        : $outputDirFull"
Write-Host "AddonBuilder  : $AddonBuilder"
Write-Host ""
Write-Host "Creating runtime-only staging tree..."

try {
    # Root runtime metadata required by AddonBuilder / the game.
    Copy-RuntimeFile -Source (Join-Path $projectRootFull 'config.cpp') -RelativeDestination 'config.cpp' -StageRoot $stagedProjectRoot

    # Runtime scripts.
    Copy-RuntimeTree -SourceRoot (Join-Path $projectRootFull 'Scripts') -RelativeDestination 'Scripts' -StageRoot $stagedProjectRoot -AllowedExtensions @('.c')

    # Bundled server policy defaults copied to $profile:PaintZ at runtime.
    Copy-RuntimeTree -SourceRoot (Join-Path $projectRootFull 'config') -RelativeDestination 'config' -StageRoot $stagedProjectRoot -AllowedExtensions @('.json', '.txt')

    # Runtime textures only. PNG generator intermediates must never enter the PBO.
    Copy-RuntimeTree -SourceRoot (Join-Path $projectRootFull 'data\cans') -RelativeDestination 'data\cans' -StageRoot $stagedProjectRoot -AllowedExtensions @('.paa')
    Copy-RuntimeTree -SourceRoot (Join-Path $projectRootFull 'data\surfaces') -RelativeDestination 'data\surfaces' -StageRoot $stagedProjectRoot -AllowedExtensions @('.paa')

    # Generated DayZ runtime fragments referenced directly by config.cpp.
    $generatedDayz = Join-Path $projectRootFull 'tools\paintzgen\generated\dayz'
    Copy-RuntimeTree -SourceRoot $generatedDayz -RelativeDestination 'tools\paintzgen\generated\dayz' -StageRoot $stagedProjectRoot -AllowedExtensions @('.inc', '.c')

    Assert-RuntimeStage -StageRoot $stagedProjectRoot

    $stageFiles = Get-ChildItem -LiteralPath $stagedProjectRoot -Recurse -File
    $stageBytes = ($stageFiles | Measure-Object Length -Sum).Sum
    Write-Host ("Runtime staging: {0} files, {1:N2} MB" -f $stageFiles.Count, ($stageBytes / 1MB))

    # The staging tree itself is the whitelist. Do not rely on AddonBuilder's
    # include filtering to keep repository/development files out of the PBO.
    & $AddonBuilder $stagedProjectRoot $outputDirFull -clear -packonly
    $builderExitCode = $LASTEXITCODE
    if ($builderExitCode -ne 0) {
        throw "AddonBuilder failed with exit code $builderExitCode."
    }

    if (-not (Test-Path -LiteralPath $expectedPbo -PathType Leaf)) {
        throw "AddonBuilder did not create the expected package '$expectedPbo'."
    }

    $builtPbo = Get-Item -LiteralPath $expectedPbo
    if ($previousPboWriteTime -and $builtPbo.LastWriteTimeUtc -eq $previousPboWriteTime) {
        throw "AddonBuilder did not refresh '$expectedPbo'; see its report for the underlying failure."
    }

    Write-Host ("Built PBO      : {0:N2} MB" -f ($builtPbo.Length / 1MB))
}
finally {
    $tempRoot = [System.IO.Path]::GetFullPath([System.IO.Path]::GetTempPath()).TrimEnd('\') + '\'
    $stagingParentFull = [System.IO.Path]::GetFullPath($stagingParent)
    if ($stagingParentFull.StartsWith($tempRoot, [System.StringComparison]::OrdinalIgnoreCase) -and (Test-Path -LiteralPath $stagingParentFull)) {
        Remove-Item -LiteralPath $stagingParentFull -Recurse -Force
    }
}
