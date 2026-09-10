[CmdletBinding()]
param(
    [string]$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
    [string]$OutputDir = (Join-Path (Resolve-Path (Join-Path $PSScriptRoot "..")).Path "dist"),
    [string]$ReleaseRoot = (Join-Path (Resolve-Path (Join-Path $PSScriptRoot "..")).Path "dist\release"),
    [string]$KeyDir = "E:\DayZServer\keys",
    [string]$KeyName = "PaintZ",
    [string]$PrivateKey,
    [string]$PublicKey,
    [string]$AddonBuilder,
    [string]$DSSignFile,
    [string]$BankRev,
    [string]$ImageToPAA,
    [switch]$SkipPaintZGen
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Resolve-RequiredFile {
    param(
        [string]$ExplicitPath,
        [string[]]$Candidates,
        [string]$Description
    )

    if ($ExplicitPath) {
        if (-not (Test-Path -LiteralPath $ExplicitPath -PathType Leaf)) {
            throw "$Description was not found at '$ExplicitPath'."
        }
        return (Resolve-Path -LiteralPath $ExplicitPath).Path
    }

    foreach ($candidate in $Candidates) {
        if ($candidate -and (Test-Path -LiteralPath $candidate -PathType Leaf)) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    throw "$Description was not found. Pass its path explicitly."
}

function Assert-PboContents {
    param(
        [Parameter(Mandatory = $true)][string]$PboPath,
        [Parameter(Mandatory = $true)][string]$BankRevPath
    )

    $listing = @(& $BankRevPath -l $PboPath)
    if ($LASTEXITCODE -ne 0) {
        throw "BankRev failed while auditing '$PboPath' with exit code $LASTEXITCODE."
    }

    $forbiddenExtensions = @(
        '.png', '.jpg', '.jpeg', '.webp', '.bmp', '.tga',
        '.py', '.pyc', '.pyo', '.ps1', '.psm1', '.psd', '.xcf', '.svg',
        '.md', '.ttf', '.otf', '.zip', '.7z'
    )

    $forbidden = @($listing | Where-Object {
        $line = $_.Trim()
        if (-not $line) { return $false }
        $extension = [System.IO.Path]::GetExtension($line).ToLowerInvariant()
        return $forbiddenExtensions -contains $extension
    })

    if ($forbidden.Count -gt 0) {
        throw "Release PBO contains forbidden development assets:`n  $($forbidden -join "`n  ")"
    }

    $paaCount = @($listing | Where-Object { $_ -match '(?i)\.paa$' }).Count
    $scriptCount = @($listing | Where-Object { $_ -match '(?i)\.c$' }).Count
    $incCount = @($listing | Where-Object { $_ -match '(?i)\.inc$' }).Count
    $jsonCount = @($listing | Where-Object { $_ -match '(?i)\.json$' }).Count
    $txtCount = @($listing | Where-Object { $_ -match '(?i)\.txt$' }).Count

    Write-Host "PBO content audit passed:"
    Write-Host "  PAA textures : $paaCount"
    Write-Host "  Scripts      : $scriptCount"
    Write-Host "  Includes     : $incCount"
    Write-Host "  JSON         : $jsonCount"
    Write-Host "  TXT          : $txtCount"
}

$projectRootFull = (Resolve-Path -LiteralPath $ProjectRoot).Path
$outputDirFull = [System.IO.Path]::GetFullPath($OutputDir)
$releaseRootFull = [System.IO.Path]::GetFullPath($ReleaseRoot)

$steamRoots = @(
    "C:\Program Files (x86)\Steam\steamapps\common",
    "C:\Program Files\Steam\steamapps\common",
    "D:\SteamLibrary\steamapps\common",
    "E:\SteamLibrary\steamapps\common"
)

$addonBuilderCandidates = $steamRoots | ForEach-Object { Join-Path $_ "DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe" }
$dsSignCandidates = $steamRoots | ForEach-Object { Join-Path $_ "DayZ Tools\Bin\DsUtils\DSSignFile.exe" }
$bankRevCandidates = $steamRoots | ForEach-Object { Join-Path $_ "DayZ Tools\Bin\PboUtils\BankRev.exe" }

$addonBuilderExe = Resolve-RequiredFile $AddonBuilder $addonBuilderCandidates "AddonBuilder.exe"
$dsSignFileExe = Resolve-RequiredFile $DSSignFile $dsSignCandidates "DSSignFile.exe"
$bankRevExe = Resolve-RequiredFile $BankRev $bankRevCandidates "BankRev.exe"

if (-not $PrivateKey) {
    $PrivateKey = Join-Path $KeyDir "$KeyName.biprivatekey"
}
if (-not $PublicKey) {
    $PublicKey = Join-Path $KeyDir "$KeyName.bikey"
}

$privateKeyPath = Resolve-RequiredFile $PrivateKey @() "PaintZ private signing key"
$publicKeyPath = Resolve-RequiredFile $PublicKey @() "PaintZ public signing key"

$pboBuildScript = Join-Path $PSScriptRoot "build-pbo.ps1"
if (-not (Test-Path -LiteralPath $pboBuildScript -PathType Leaf)) {
    throw "PBO build helper was not found at '$pboBuildScript'."
}

New-Item -ItemType Directory -Force -Path $outputDirFull | Out-Null
New-Item -ItemType Directory -Force -Path $releaseRootFull | Out-Null

Write-Host "PaintZ release build"
Write-Host "  Project      : $projectRootFull"
Write-Host "  Output       : $outputDirFull"
Write-Host "  Release      : $releaseRootFull"
Write-Host "  AddonBuilder : $addonBuilderExe"
Write-Host "  DSSignFile   : $dsSignFileExe"
Write-Host "  BankRev      : $bankRevExe"
Write-Host "  Private key  : $privateKeyPath"
Write-Host "  Public key   : $publicKeyPath"
Write-Host ""

$pboBuildArgs = @{
    AddonBuilder = $addonBuilderExe
    ProjectRoot = $projectRootFull
    OutputDir = $outputDirFull
}
if ($ImageToPAA) {
    $pboBuildArgs.ImageToPAA = $ImageToPAA
}
if ($SkipPaintZGen) {
    $pboBuildArgs.SkipPaintZGen = $true
}

& $pboBuildScript @pboBuildArgs
if (-not $?) {
    throw "PaintZ PBO build failed."
}

$pboPath = Join-Path $outputDirFull "PaintZ.pbo"
if (-not (Test-Path -LiteralPath $pboPath -PathType Leaf)) {
    throw "Expected built PBO was not found at '$pboPath'."
}

Assert-PboContents -PboPath $pboPath -BankRevPath $bankRevExe
$pboInfo = Get-Item -LiteralPath $pboPath
Write-Host ("PBO size       : {0:N2} MB" -f ($pboInfo.Length / 1MB))

Get-ChildItem -LiteralPath $outputDirFull -Filter "PaintZ.pbo*.bisign" -File -ErrorAction SilentlyContinue | Remove-Item -Force

Write-Host ""
Write-Host "Signing $pboPath ..."
& $dsSignFileExe $privateKeyPath $pboPath
if ($LASTEXITCODE -ne 0) {
    throw "DSSignFile failed with exit code $LASTEXITCODE."
}

$signature = Get-ChildItem -LiteralPath $outputDirFull -Filter "PaintZ.pbo*.bisign" -File -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTimeUtc -Descending |
    Select-Object -First 1
if (-not $signature) {
    throw "DSSignFile returned success but no PaintZ .bisign file was created in '$outputDirFull'."
}

$releaseModRoot = Join-Path $releaseRootFull "@PaintZ"
$releaseAddons = Join-Path $releaseModRoot "addons"
$releaseKeys = Join-Path $releaseModRoot "keys"

if (Test-Path -LiteralPath $releaseModRoot) {
    Remove-Item -LiteralPath $releaseModRoot -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $releaseAddons | Out-Null
New-Item -ItemType Directory -Force -Path $releaseKeys | Out-Null

Copy-Item -LiteralPath $pboPath -Destination (Join-Path $releaseAddons "PaintZ.pbo") -Force
Copy-Item -LiteralPath $signature.FullName -Destination (Join-Path $releaseAddons $signature.Name) -Force
Copy-Item -LiteralPath $publicKeyPath -Destination (Join-Path $releaseKeys ([System.IO.Path]::GetFileName($publicKeyPath))) -Force

$modCpp = Join-Path $projectRootFull 'mod.cpp'
if (Test-Path -LiteralPath $modCpp -PathType Leaf) {
    Copy-Item -LiteralPath $modCpp -Destination (Join-Path $releaseModRoot 'mod.cpp') -Force
}

$releasePbo = Join-Path $releaseAddons "PaintZ.pbo"
$releaseBisign = Join-Path $releaseAddons $signature.Name
$releaseBikey = Join-Path $releaseKeys ([System.IO.Path]::GetFileName($publicKeyPath))
if (-not (Test-Path -LiteralPath $releasePbo -PathType Leaf) -or
    -not (Test-Path -LiteralPath $releaseBisign -PathType Leaf) -or
    -not (Test-Path -LiteralPath $releaseBikey -PathType Leaf)) {
    throw "Release package verification failed."
}

$releaseFiles = Get-ChildItem -LiteralPath $releaseModRoot -Recurse -File
$releaseBytes = ($releaseFiles | Measure-Object Length -Sum).Sum

Write-Host ""
Write-Host "Release package ready:"
Write-Host "  $releaseModRoot"
Write-Host ("  Total size   : {0:N2} MB" -f ($releaseBytes / 1MB))
Write-Host ""
Write-Host "Contents:"
Write-Host "  addons\PaintZ.pbo"
Write-Host "  addons\$($signature.Name)"
Write-Host "  keys\$([System.IO.Path]::GetFileName($publicKeyPath))"
if (Test-Path -LiteralPath (Join-Path $releaseModRoot 'mod.cpp') -PathType Leaf) {
    Write-Host "  mod.cpp"
}
Write-Host ""
Write-Host "Deploy @PaintZ to both server and client."
Write-Host "The server must also have the public .bikey in its root keys directory."
