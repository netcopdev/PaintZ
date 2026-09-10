[CmdletBinding()]
param(
    [string]$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
    [string]$OutputDir = (Join-Path (Resolve-Path (Join-Path $PSScriptRoot "..")).Path "dist"),
    [string]$ReleaseRoot = (Join-Path (Resolve-Path (Join-Path $PSScriptRoot "..")).Path "dist\release"),
    [string]$BuildConfig,
    [string]$PrivateKey,
    [string]$PublicKey,
    [string]$AddonBuilder,
    [string]$DSSignFile,
    [string]$BankRev
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$buildScript = Join-Path $PSScriptRoot "build.ps1"
if (-not (Test-Path -LiteralPath $buildScript -PathType Leaf)) {
    throw "Canonical build script was not found at '$buildScript'."
}

$buildArgs = @{
    ProjectRoot = $ProjectRoot
    OutputDir = $OutputDir
    ReleaseRoot = $ReleaseRoot
}
if ($BuildConfig) { $buildArgs.BuildConfig = $BuildConfig }
if ($PrivateKey) { $buildArgs.PrivateKey = $PrivateKey }
if ($PublicKey) { $buildArgs.PublicKey = $PublicKey }
if ($AddonBuilder) { $buildArgs.AddonBuilder = $AddonBuilder }
if ($DSSignFile) { $buildArgs.DSSignFile = $DSSignFile }
if ($BankRev) { $buildArgs.BankRev = $BankRev }

Write-Warning "tools\build-release.ps1 is retained for compatibility; tools\build.ps1 is now the canonical signed release build."
& $buildScript @buildArgs
if (-not $?) {
    throw "PaintZ release build failed."
}
