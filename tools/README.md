# PaintZ build tools

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

By default the signing key pair is resolved as:

```text
E:\DayZServer\keys\PaintZ.biprivatekey
E:\DayZServer\keys\PaintZ.bikey
```

Override with `-PrivateKey` and `-PublicKey`, or change `-KeyDir` / `-KeyName`.

The public `.bikey` inside the mod release is for distribution. A signature-verifying dedicated server must also have that public key in its server-root `keys` directory.

## Raw PBO helper

`tools\build-pbo.ps1` is the internal/raw packaging helper. It creates `PaintZ.pbo` but does not sign or assemble a deployable mod folder. Use it only for tooling such as the isolated sandbox or diagnostics that explicitly do not need a release package.

`tools\build-release.ps1` is retained as a compatibility wrapper and delegates to the canonical `build.ps1` release build.

PaintZ core owns no finish textures. Finish assets are generated and built by content packs such as PaintZ-Standard-Pack. The legacy `-ImageToPAA` and `-SkipPaintZGen` parameters are retained only for compatibility and are not needed for a normal PaintZ core build.

## Local safe sandbox

For the zero-CE, auto-spawned test environment, double-click `Run-PaintZSandbox.cmd` at the repository root. The sandbox uses raw PBO packaging internally rather than the signed release workflow. Full details and optional arguments are in `tools/sandbox/README.md`.
