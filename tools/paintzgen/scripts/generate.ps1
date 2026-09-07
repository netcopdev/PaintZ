$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
python "$Root\tools\generate_paints.py" --clean
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# Optional DayZ Tools conversion. Set PAINTZ_IMAGE_TO_PAA to ImageToPAA.exe.
if ($env:PAINTZ_IMAGE_TO_PAA -and (Test-Path $env:PAINTZ_IMAGE_TO_PAA)) {
    foreach ($textureDirectory in @("labels", "surfaces")) {
        Get-ChildItem "$Root\generated\$textureDirectory\*_co.png" | ForEach-Object {
            $paa = [System.IO.Path]::ChangeExtension($_.FullName, ".paa")
            & $env:PAINTZ_IMAGE_TO_PAA $_.FullName $paa
            if ($LASTEXITCODE -ne 0) { throw "ImageToPAA failed for $($_.FullName)" }
        }
    }
}
