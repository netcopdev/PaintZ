@{
    # Required for signed release builds. Keep real paths only in your local
    # %LOCALAPPDATA%\PaintZ\build.psd1 (or another file selected with
    # PAINTZ_BUILD_CONFIG / -BuildConfig). Do not commit real key paths.
    PrivateKey = ''
    PublicKey  = ''

    # Optional workstation overrides. Leave blank to use normal auto-discovery
    # where supported by the calling repository.
    AddonBuilder = ''
    DSSignFile   = ''
    BankRev      = ''
    ImageToPAA   = ''

    # Used by content-pack builders when needed. PaintZ core safely ignores
    # fields it does not consume.
    Python      = ''
    PackKitRoot = ''
}
