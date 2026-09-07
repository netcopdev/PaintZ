# Tools

`build.ps1` is a convenience wrapper for a standard local DayZ Tools AddonBuilder installation. It intentionally accepts paths as parameters instead of hardcoding the developer's machine.

The wrapper runs paintzgen before AddonBuilder and converts both can-label assets
and separate clean coating assets with ImageToPAA. Can artwork is exported to
`data/cans`; paint applied to a target is exported to `data/surfaces`. Use
`-ImageToPAA` when DayZ Tools is installed outside the standard Steam locations.
`-SkipPaintZGen` is available for build diagnostics with already-generated outputs.

If your existing DayZ mod build workflow is already established, use that instead and let Codex adapt/remove this helper.

## Local safe sandbox

For the zero-CE, auto-spawned test environment, double-click
`Run-PaintZSandbox.cmd` at the repository root. It reuses an existing packaged PBO
by default. Pass `-Build` to package without regenerating paint assets, or
`-Generate` to regenerate and package before launch. Full details and optional
arguments are in `tools/sandbox/README.md`.
