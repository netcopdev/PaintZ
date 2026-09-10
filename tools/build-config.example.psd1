@{
    # Required for signed release builds. Keep real paths only in your local
    # %LOCALAPPDATA%\PaintZ\build.psd1 (or another file selected with
    # PAINTZ_BUILD_CONFIG / -BuildConfig). Do not commit real key paths.
    PrivateKey = ''
    PublicKey  = ''

    # Optional PaintZ core build overrides. Leave blank to use auto-discovery.
    AddonBuilder = ''
    DSSignFile   = ''
    BankRev      = ''

    # Shared content-pack authoring/build fields. PaintZ core does not consume
    # these, but the same machine-local config may be reused by official packs.
    ImageToPAA   = ''
    Python       = ''
    PackKitRoot  = ''
}
