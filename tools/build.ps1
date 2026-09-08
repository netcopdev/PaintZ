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

$projectName = Split-Path -Leaf $ProjectRoot.TrimEnd('\')
$stagingParent = Join-Path ([System.IO.Path]::GetTempPath()) ("PaintZ-AddonSource-" + [guid]::NewGuid().ToString("N"))
$stagedProjectRoot = Join-Path $stagingParent $projectName
$expectedPbo = Join-Path $OutputDir "$projectName.pbo"
$previousPboWriteTime = $null
if (Test-Path -LiteralPath $expectedPbo -PathType Leaf) {
    $previousPboWriteTime = (Get-Item -LiteralPath $expectedPbo).LastWriteTimeUtc
}

New-Item -ItemType Directory -Force -Path $stagedProjectRoot | Out-Null

Write-Host "PaintZ source : $ProjectRoot"
Write-Host "Build staging : $stagedProjectRoot"
Write-Host "Output      : $OutputDir"
Write-Host "AddonBuilder: $AddonBuilder"
Write-Host ""
Write-Host "NOTE: Verify these AddonBuilder arguments against your existing DayZ Tools workflow before relying on this helper."

try {
    # AddonBuilder copies the complete source tree before applying include.lst.
    # Stage without repository metadata and local caches so internal .git paths
    # cannot exceed the legacy copy task's MAX_PATH limit.
    $copyArguments = @(
        $ProjectRoot,
        $stagedProjectRoot,
        "/E",
        "/NFL",
        "/NDL",
        "/NJH",
        "/NJS",
        "/NP",
        "/XD",
        (Join-Path $ProjectRoot ".git"),
        (Join-Path $ProjectRoot ".tmp"),
        (Join-Path $ProjectRoot ".vscode"),
        (Join-Path $ProjectRoot ".vs"),
        (Join-Path $ProjectRoot ".idea"),
        (Join-Path $ProjectRoot "dist"),
        (Join-Path $ProjectRoot "build"),
        (Join-Path $ProjectRoot "out"),
        (Join-Path $ProjectRoot "@TagZ")
    )
    & robocopy @copyArguments | Out-Null
    $copyExitCode = $LASTEXITCODE
    if ($copyExitCode -gt 7) {
        throw "Build staging failed with robocopy exit code $copyExitCode."
    }

    $stagedInclude = Join-Path $stagedProjectRoot "include.lst"
    & $AddonBuilder $stagedProjectRoot $OutputDir -clear -packonly "-include=$stagedInclude"
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
}
finally {
    $tempRoot = [System.IO.Path]::GetFullPath([System.IO.Path]::GetTempPath()).TrimEnd('\') + '\'
    $stagingParentFull = [System.IO.Path]::GetFullPath($stagingParent)
    if ($stagingParentFull.StartsWith($tempRoot, [System.StringComparison]::OrdinalIgnoreCase) -and (Test-Path -LiteralPath $stagingParentFull)) {
        Remove-Item -LiteralPath $stagingParentFull -Recurse -Force
    }
}
