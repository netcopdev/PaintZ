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

$projectRootFull = (Resolve-Path -LiteralPath $ProjectRoot).Path
$outputDirFull = [System.IO.Path]::GetFullPath($OutputDir)
$releaseRootFull = [System.IO.Path]::GetFullPath($ReleaseRoot)

$steamRoots = @(
    "C:\Program Files (x86)\Steam\steamapps\common",
    "C:\Program Files\Steam\steamapps\common",
    "D:\SteamLibrary\steamapps\common",
    "E:\SteamLibrary\steamapps\common"
)

$addonBuilderCandidates = $steamRoots | ForEach-Object {
    Join-Path $_ "DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe"
}
$dsSignCandidates = $steamRoots | ForEach-Object {
    Join-Path $_ "DayZ Tools\Bin\DsUtils\DSSignFile.exe"
}

$addonBuilderExe = Resolve-RequiredFile $AddonBuilder $addonBuilderCandidates "AddonBuilder.exe"
$dsSignFileExe = Resolve-RequiredFile $DSSignFile $dsSignCandidates "DSSignFile.exe"

if (-not $PrivateKey) {
    $PrivateKey = Join-Path $KeyDir "$KeyName.biprivatekey"
}
if (-not $PublicKey) {
    $PublicKey = Join-Path $KeyDir "$KeyName.bikey"
}

$privateKeyPath = Resolve-RequiredFile $PrivateKey @() "PaintZ private signing key"
$publicKeyPath = Resolve-RequiredFile $PublicKey @() "PaintZ public signing key"

$buildScript = Join-Path $PSScriptRoot "build.ps1"
if (-not (Test-Path -LiteralPath $buildScript -PathType Leaf)) {
    throw "Build helper was not found at '$buildScript'."
}

New-Item -ItemType Directory -Force -Path $outputDirFull | Out-Null

Write-Host "PaintZ release build"
Write-Host "  Project      : $projectRootFull"
Write-Host "  Output       : $outputDirFull"
Write-Host "  Release      : $releaseRootFull"
Write-Host "  AddonBuilder : $addonBuilderExe"
Write-Host "  DSSignFile   : $dsSignFileExe"
Write-Host "  Private key  : $privateKeyPath"
Write-Host "  Public key   : $publicKeyPath"
Write-Host ""

$buildArgs = @{
    AddonBuilder = $addonBuilderExe
    ProjectRoot = $projectRootFull
    OutputDir = $outputDirFull
}

if ($ImageToPAA) {
    $buildArgs.ImageToPAA = $ImageToPAA
}
if ($SkipPaintZGen) {
    $buildArgs.SkipPaintZGen = $true
}

& $buildScript @buildArgs
if (-not $?) {
    throw "PaintZ build failed."
}

$pboPath = Join-Path $outputDirFull "PaintZ.pbo"
if (-not (Test-Path -LiteralPath $pboPath -PathType Leaf)) {
    throw "Expected built PBO was not found at '$pboPath'."
}

# Remove only previous signatures for this PBO so a stale .bisign cannot make a
# failed signing step look successful.
Get-ChildItem -LiteralPath $outputDirFull -Filter "PaintZ.pbo*.bisign" -File -ErrorAction SilentlyContinue |
    Remove-Item -Force

Write-Host ""
Write-Host "Signing $pboPath ..."
& $dsSignFileExe $privateKeyPath $pboPath
$signExitCode = $LASTEXITCODE
if ($signExitCode -ne 0) {
    throw "DSSignFile failed with exit code $signExitCode."
}

$signature = Get-ChildItem -LiteralPath $outputDirFull -Filter "PaintZ.pbo*.bisign" -File -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTimeUtc -Descending |
    Select-Object -First 1

if (-not $signature) {
    throw "DSSignFile returned success but no PaintZ .bisign file was created in '$outputDirFull'."
}

$releaseModRoot = Join-Path $releaseRootFull "@PaintZ"
$releaseAddons = Join-Path $releaseModRoot "Addons"
$releaseKeys = Join-Path $releaseModRoot "Keys"

# Always recreate only the packaged @PaintZ folder. Other files under ReleaseRoot
# are left untouched.
if (Test-Path -LiteralPath $releaseModRoot) {
    Remove-Item -LiteralPath $releaseModRoot -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $releaseAddons | Out-Null
New-Item -ItemType Directory -Force -Path $releaseKeys | Out-Null

Copy-Item -LiteralPath $pboPath -Destination (Join-Path $releaseAddons "PaintZ.pbo") -Force
Copy-Item -LiteralPath $signature.FullName -Destination (Join-Path $releaseAddons $signature.Name) -Force
Copy-Item -LiteralPath $publicKeyPath -Destination (Join-Path $releaseKeys ([System.IO.Path]::GetFileName($publicKeyPath))) -Force

$releasePbo = Join-Path $releaseAddons "PaintZ.pbo"
$releaseBisign = Join-Path $releaseAddons $signature.Name
$releaseBikey = Join-Path $releaseKeys ([System.IO.Path]::GetFileName($publicKeyPath))

if (-not (Test-Path -LiteralPath $releasePbo -PathType Leaf) -or
    -not (Test-Path -LiteralPath $releaseBisign -PathType Leaf) -or
    -not (Test-Path -LiteralPath $releaseBikey -PathType Leaf)) {
    throw "Release package verification failed."
}

Write-Host ""
Write-Host "Release package ready:"
Write-Host "  $releaseModRoot"
Write-Host ""
Write-Host "Contents:"
Write-Host "  Addons\PaintZ.pbo"
Write-Host "  Addons\$($signature.Name)"
Write-Host "  Keys\$([System.IO.Path]::GetFileName($publicKeyPath))"
Write-Host ""
Write-Host "Deploy @PaintZ to both server and client."
Write-Host "The server must also have the public .bikey in its root keys directory."
Write-Host "Load Community Framework before PaintZ in -mod, for example: @CF;@PaintZ"
