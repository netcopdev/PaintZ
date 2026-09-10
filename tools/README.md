# PaintZ build tools

## One-time workstation configuration

Signed PaintZ builds use one machine-local PowerShell data file shared by PaintZ core and official PaintZ content-pack builders.

Default location:

```text
%LOCALAPPDATA%\PaintZ\build.psd1
```

The real file lives outside every repository and must not be committed. Start from `tools\build-config.example.psd1` and fill in at least `PrivateKey` and `PublicKey`. Optional entries can pin DayZ Tools executables, Python, or a nonstandard PackKit checkout.

Resolution order is:

1. an explicit command-line parameter;
2. the config selected by `-BuildConfig`;
3. the config selected by the `PAINTZ_BUILD_CONFIG` environment variable;
4. `%LOCALAPPDATA%\PaintZ\build.psd1`;
5. built-in tool auto-discovery where supported.

Signing never silently falls back to an unsigned release. If the private/public key paths cannot be resolved, the build fails before packaging a deployable release.

Example first-time setup:

```powershell
New-Item -ItemType Directory -Force "$env:LOCALAPPDATA\PaintZ" | Out-Null
Copy-Item .\tools\build-config.example.psd1 "$env:LOCALAPPDATA\PaintZ\build.psd1"
micro "$env:LOCALAPPDATA\PaintZ\build.psd1"
```

A different local config can be selected without changing repository files:

```powershell
$env:PAINTZ_BUILD_CONFIG = 'D:\private\paintz-build.psd1'
```

or per invocation:

```powershell
pwsh -File .\tools\build.ps1 -BuildConfig 'D:\private\paintz-build.psd1'
```

## Normal build

Use:

```powershell
pwsh -File .\tools\build.ps1
```

`build.ps1` is the canonical deployable build. It:

1. stages only PaintZ runtime files;
2. builds `dist\PaintZ.pbo` with AddonBuilder;
3. audits the PBO contents with BankRev;
4. removes stale signatures and signs the new PBO with DSSignFile;
5. recreates the release package;
6. copies the PBO, matching `.bisign`, public `.bikey`, and `mod.cpp` when present;
7. verifies the required release files exist.

Default release layout:

```text
dist/release/@PaintZ/
  addons/
    PaintZ.pbo
    PaintZ.pbo.<key>.bisign
  keys/
    <key>.bikey
  mod.cpp
```

The public `.bikey` inside the mod release is for distribution. A signature-verifying dedicated server must also have that public key in its server-root `keys` directory.

## Raw PBO helper

`tools\build-pbo.ps1` is the internal/raw packaging helper. It creates `PaintZ.pbo` but does not sign or assemble a deployable mod folder. Use it only for tooling such as the isolated sandbox or diagnostics that explicitly do not need a release package.

`tools\build-release.ps1` is retained as a compatibility wrapper and delegates to the canonical `build.ps1` release build, including the same external build-config resolution.

PaintZ core owns no finish catalogue or finish-generation tooling. Finish definitions, can artwork, and target-surface assets belong to content packs and are authored/generated through PaintZ-PackKit or an equivalent conforming workflow.

## Local safe sandbox

For the zero-CE, auto-spawned test environment, double-click `Run-PaintZSandbox.cmd` at the repository root. The sandbox uses raw PBO packaging internally rather than the signed release workflow. Full details and optional arguments are in `tools/sandbox/README.md`.
