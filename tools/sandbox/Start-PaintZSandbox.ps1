[CmdletBinding()]
param(
    [string]$DayZServerExe,
    [string]$DayZClientExe,
    [string]$AddonBuilderExe,
    [string]$RuntimeRoot = (Join-Path $env:LOCALAPPDATA "PaintZSandbox"),
    [int]$Port = 2302,
    [string]$PlayerName = "NetCop",
    [string[]]$AdditionalMods = @(),
    [string]$ThirdPartyWeaponClass = "",
    [switch]$Build,
    [Alias("GeneratePaints")]
    [switch]$Generate,
    [switch]$SkipBuild,
    [switch]$ServerOnly,
    [switch]$LeaveServerRunning
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
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    throw "$Description was not found. Pass its path explicitly."
}

function Assert-ChildPath {
    param(
        [string]$ParentPath,
        [string]$ChildPath,
        [string]$Description
    )

    $parentFull = [System.IO.Path]::GetFullPath($ParentPath).TrimEnd('\') + '\'
    $childFull = [System.IO.Path]::GetFullPath($ChildPath)

    if (-not $childFull.StartsWith($parentFull, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "$Description resolved outside its expected parent: '$childFull'."
    }
}

function Quote-DayZArgument {
    param([string]$Value)
    return '"' + $Value + '"'
}

function Get-ProcessOwnerName {
    param([int]$ProcessId)

    try {
        $processInfo = Get-CimInstance Win32_Process -Filter "ProcessId=$ProcessId" -ErrorAction Stop
        $owner = Invoke-CimMethod -InputObject $processInfo -MethodName GetOwner -ErrorAction Stop
        if ($owner.ReturnValue -eq 0 -and $owner.User) {
            return "$($owner.Domain)\$($owner.User)"
        }
    }
    catch {
        return ""
    }

    return ""
}

if ($ThirdPartyWeaponClass -and $ThirdPartyWeaponClass -notmatch '^[A-Za-z_][A-Za-z0-9_]*$') {
    throw "ThirdPartyWeaponClass must be a single valid DayZ config classname."
}

if ($SkipBuild -and ($Build -or $Generate)) {
    throw "-SkipBuild cannot be combined with -Build or -Generate."
}

if ($SkipBuild) {
    Write-Warning "-SkipBuild is no longer necessary; the sandbox does not build by default."
}

$buildRequested = $Build -or $Generate

$projectRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..\..")).Path
$steamRoots = @(
    "C:\Program Files (x86)\Steam\steamapps\common",
    "C:\Program Files\Steam\steamapps\common",
    "D:\SteamLibrary\steamapps\common",
    "E:\SteamLibrary\steamapps\common"
)

$serverCandidates = $steamRoots | ForEach-Object { Join-Path $_ "DayZ\DayZDiag_x64.exe" }
$clientCandidates = $steamRoots | ForEach-Object { Join-Path $_ "DayZ\DayZDiag_x64.exe" }
$builderCandidates = $steamRoots | ForEach-Object { Join-Path $_ "DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe" }

$serverExe = Resolve-RequiredFile $DayZServerExe $serverCandidates "DayZDiag_x64.exe (server instance)"
$clientExe = Resolve-RequiredFile $DayZClientExe $clientCandidates "DayZDiag_x64.exe"
$addonBuilder = $null
if ($buildRequested) {
    $addonBuilder = Resolve-RequiredFile $AddonBuilderExe $builderCandidates "AddonBuilder.exe"
}
$serverRoot = Split-Path -Parent $serverExe

$steamProcess = Get-Process -Name "steam" -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $steamProcess) {
    throw "Steam must be running and signed in before starting the PaintZ sandbox."
}

$currentWindowsUser = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name
$steamWindowsUser = Get-ProcessOwnerName $steamProcess.Id
if ($steamWindowsUser -and $steamWindowsUser -ne $currentWindowsUser) {
    throw "Steam runs as '$steamWindowsUser', but this launcher runs as '$currentWindowsUser'. Start Run-PaintZSandbox.cmd directly from the same Windows desktop account as Steam."
}

$runtimeRootFull = [System.IO.Path]::GetFullPath($RuntimeRoot)
$buildOutput = Join-Path $runtimeRootFull "build"
$PaintZModRoot = Join-Path $runtimeRootFull "@PaintZ"
$PaintZAddons = Join-Path $PaintZModRoot "Addons"
$serverProfiles = Join-Path $runtimeRootFull "server-profiles"
$clientLogRoot = Join-Path $env:LOCALAPPDATA "DayZ"
$serverConfig = Join-Path $runtimeRootFull "PaintZSandboxServerDZ.cfg"
$missionParent = Join-Path $serverRoot "mpmissions"
$missionDirectory = Join-Path $missionParent "PaintZ_Sandbox.ChernarusPlus"
$missionInit = Join-Path $missionDirectory "init.c"

Assert-ChildPath $serverRoot $missionDirectory "Sandbox mission directory"

foreach ($directory in @($runtimeRootFull, $buildOutput, $PaintZAddons, $serverProfiles, $clientLogRoot, $missionDirectory)) {
    New-Item -ItemType Directory -Force -Path $directory | Out-Null
}

$missionTemplatePath = Join-Path $PSScriptRoot "templates\init.c.template"
$serverConfigTemplatePath = Join-Path $PSScriptRoot "templates\serverDZ.cfg.template"
$effectAreaTemplatePath = Join-Path $PSScriptRoot "templates\cfgeffectarea.json.template"
$undergroundTemplatePath = Join-Path $PSScriptRoot "templates\cfgundergroundtriggers.json.template"

$missionText = Get-Content -LiteralPath $missionTemplatePath -Raw
$missionText = $missionText.Replace("__PAINTZ_THIRD_PARTY_WEAPON_CLASS__", $ThirdPartyWeaponClass)
Set-Content -LiteralPath $missionInit -Value $missionText -Encoding UTF8
Copy-Item -LiteralPath $effectAreaTemplatePath -Destination (Join-Path $missionDirectory "cfgeffectarea.json") -Force
Copy-Item -LiteralPath $undergroundTemplatePath -Destination (Join-Path $missionDirectory "cfgundergroundtriggers.json") -Force

$serverConfigText = Get-Content -LiteralPath $serverConfigTemplatePath -Raw
$serverConfigText = $serverConfigText.Replace("__PAINTZ_PORT__", $Port.ToString())
Set-Content -LiteralPath $serverConfig -Value $serverConfigText -Encoding UTF8

if ($buildRequested) {
    if ($Generate) {
        Write-Host "Generating paint assets and building PaintZ..."
        & (Join-Path $projectRoot "tools\build.ps1") -AddonBuilder $addonBuilder -ProjectRoot $projectRoot -OutputDir $buildOutput
    }
    else {
        Write-Host "Building PaintZ from existing generated assets..."
        & (Join-Path $projectRoot "tools\build.ps1") -AddonBuilder $addonBuilder -ProjectRoot $projectRoot -OutputDir $buildOutput -SkipPaintZGen
    }
    if ($LASTEXITCODE -ne 0) {
        throw "PaintZ build failed with exit code $LASTEXITCODE."
    }
}

$runtimePbo = Join-Path $PaintZAddons "PaintZ.pbo"
if ($buildRequested) {
    $sourcePbo = Get-ChildItem -LiteralPath $buildOutput -Filter "PaintZ.pbo" -File -Recurse |
        Sort-Object LastWriteTimeUtc -Descending |
        Select-Object -First 1
}
else {
    $pboCandidates = @()
    if (Test-Path -LiteralPath $runtimePbo -PathType Leaf) {
        $pboCandidates += Get-Item -LiteralPath $runtimePbo
    }
    foreach ($searchRoot in @($buildOutput, (Join-Path $projectRoot "dist"))) {
        if (Test-Path -LiteralPath $searchRoot -PathType Container) {
            $pboCandidates += Get-ChildItem -LiteralPath $searchRoot -Filter "PaintZ.pbo" -File -Recurse
        }
    }
    $sourcePbo = $pboCandidates | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1
}

if (-not $sourcePbo) {
    throw "No packaged PaintZ.pbo was found. Run again with -Build, or use -Generate after changing the paint catalogue/assets."
}

if ($sourcePbo.FullName -ne $runtimePbo) {
    Copy-Item -LiteralPath $sourcePbo.FullName -Destination $runtimePbo -Force
}

Write-Host "Using PaintZ PBO: $runtimePbo"

$resolvedAdditionalMods = @()
foreach ($mod in $AdditionalMods) {
    if (-not (Test-Path -LiteralPath $mod -PathType Container)) {
        throw "Additional mod directory was not found: '$mod'."
    }

    $resolvedAdditionalMods += (Resolve-Path -LiteralPath $mod).Path
}

$modList = (@($PaintZModRoot) + $resolvedAdditionalMods) -join ';'
$serverArguments = @(
    "-server",
    "-config=$serverConfig",
    "-profiles=$serverProfiles",
    "-port=$Port",
    "-mod=$modList",
    "-dologs",
    "-adminlog",
    "-netlog",
    "-freezecheck"
)

Write-Host ""
Write-Host "Starting isolated PaintZ server..."
Write-Host "  Mission : $missionDirectory"
Write-Host "  Runtime : $runtimeRootFull"
Write-Host "  Mod     : $modList"

$serverStart = Get-Date
$serverProcessName = [System.IO.Path]::GetFileNameWithoutExtension($serverExe)
$existingServerIds = @(Get-Process -Name $serverProcessName -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id)
if ($existingServerIds.Count -gt 0) {
    throw "$serverProcessName is already running. Stop it before launching this isolated sandbox."
}

Push-Location $serverRoot
try {
    & $serverExe $serverArguments
}
finally {
    Pop-Location
}

$serverProcessDeadline = (Get-Date).AddSeconds(10)
$serverProcess = $null
while ((Get-Date) -lt $serverProcessDeadline -and -not $serverProcess) {
    Start-Sleep -Milliseconds 250
    $serverProcess = Get-Process -Name $serverProcessName -ErrorAction SilentlyContinue |
        Where-Object { $existingServerIds -notcontains $_.Id } |
        Sort-Object StartTime -Descending |
        Select-Object -First 1
}

if (-not $serverProcess) {
    throw "$serverProcessName did not create a server process within 10 seconds."
}

$clientProcess = $null
$preserveServer = $false

try {
    $ready = $false
    $deadline = (Get-Date).AddSeconds(90)
    $serverReport = $null
    $serverScriptLog = $null

    while ((Get-Date) -lt $deadline) {
        Start-Sleep -Milliseconds 500

        if ($serverProcess.HasExited) {
            throw "DayZ server exited during startup with code $($serverProcess.ExitCode)."
        }

        $serverReport = Get-ChildItem -LiteralPath $serverProfiles -Filter "$serverProcessName*.RPT" -File -ErrorAction SilentlyContinue |
            Where-Object { $_.LastWriteTime -ge $serverStart.AddSeconds(-2) } |
            Sort-Object LastWriteTimeUtc -Descending |
            Select-Object -First 1

        $serverScriptLog = Get-ChildItem -LiteralPath $serverProfiles -Filter "script*.log" -File -ErrorAction SilentlyContinue |
            Where-Object { $_.LastWriteTime -ge $serverStart.AddSeconds(-2) } |
            Sort-Object LastWriteTimeUtc -Descending |
            Select-Object -First 1

        if ($serverScriptLog -and (Select-String -LiteralPath $serverScriptLog.FullName -SimpleMatch "[PaintZ][Sandbox] READY" -Quiet)) {
            $ready = $true
            break
        }
    }

    if (-not $ready) {
        if ($serverReport) {
            Write-Host ""
            Write-Host "Latest server report: $($serverReport.FullName)"
            Get-Content -LiteralPath $serverReport.FullName -Tail 80
        }
        elseif ($serverScriptLog) {
            Write-Host ""
            Write-Host "Latest server script log: $($serverScriptLog.FullName)"
            Get-Content -LiteralPath $serverScriptLog.FullName -Tail 80
        }

        throw "The sandbox server did not report ready within 90 seconds."
    }

    Write-Host "Server ready. PID $($serverProcess.Id)"
    if ($serverReport) {
        Write-Host "Server log: $($serverReport.FullName)"
    }
    if ($serverScriptLog) {
        Write-Host "Script log: $($serverScriptLog.FullName)"
    }

    if ($ServerOnly) {
        Write-Host "Server-only mode selected. Connect to 127.0.0.1:$Port with mod list: $modList"
        if (-not $LeaveServerRunning) {
            Write-Host "Press Enter to stop the sandbox server."
            [void](Read-Host)
        }
        else {
            $preserveServer = $true
        }
        return
    }

    $clientArguments = @(
        "-name=$PlayerName",
        (Quote-DayZArgument "-mod=$modList"),
        "-connect=127.0.0.1",
        "-port=$Port"
    )

    $existingClients = @(Get-Process -Name "DayZ_x64", "DayZDiag_x64", "DayZ_BE", "DayZLauncher" -ErrorAction SilentlyContinue |
        Where-Object { $_.Id -ne $serverProcess.Id })
    if ($existingClients.Count -gt 0) {
        throw "A DayZ client or launcher is already running. Close it before launching this isolated sandbox."
    }

    Write-Host "Launching DayZDiag_x64 directly as $currentWindowsUser and connecting to 127.0.0.1:$Port..."
    $clientStart = Get-Date
    $clientProcess = Start-Process -FilePath $clientExe -ArgumentList $clientArguments -WorkingDirectory (Split-Path -Parent $clientExe) -PassThru

    $clientReady = $false
    $clientDeadline = (Get-Date).AddSeconds(90)
    $clientReport = $null
    while ((Get-Date) -lt $clientDeadline) {
        Start-Sleep -Milliseconds 500

        if ($clientProcess.HasExited) {
            throw "DayZ client exited during startup with code $($clientProcess.ExitCode)."
        }

        $clientReport = Get-ChildItem -LiteralPath $clientLogRoot -Filter "DayZDiag_x64*.RPT" -File -ErrorAction SilentlyContinue |
            Where-Object { $_.LastWriteTime -ge $clientStart.AddSeconds(-2) } |
            Sort-Object LastWriteTimeUtc -Descending |
            Select-Object -First 1

        # DayZ keeps the RPT at zero bytes for part of startup. File creation is
        # sufficient proof that the real engine process initialized; requiring
        # log content here can kill a healthy client while it is loading terrain.
        if ($clientReport) {
            $clientReady = $true
            break
        }
    }

    if (-not $clientReady) {
        throw "DayZ diagnostic client created no engine report within 90 seconds."
    }

    Write-Host "DayZ client initialized. PID $($clientProcess.Id)"
    Write-Host "Client log: $($clientReport.FullName)"

    if (-not $LeaveServerRunning) {
        Write-Host "The sandbox server will stop automatically when this DayZ client exits."
        $clientProcess.WaitForExit()
    }
    else {
        $preserveServer = $true
    }
}
finally {
    if (-not $preserveServer -and $clientProcess -and -not $clientProcess.HasExited) {
        Write-Host "Stopping uninitialized sandbox client PID $($clientProcess.Id)..."
        Stop-Process -Id $clientProcess.Id
        $clientProcess.WaitForExit(10000) | Out-Null
    }

    if (-not $preserveServer -and $serverProcess -and -not $serverProcess.HasExited) {
        Write-Host "Stopping sandbox server PID $($serverProcess.Id)..."
        Stop-Process -Id $serverProcess.Id
        $serverProcess.WaitForExit(10000) | Out-Null
    }
}
