class PaintZ_PaintConstants
{
    // A finish's complete Paint Pack API ID is its sole persistent identity.
    // Pattern scale is derived visual state and never becomes part of the ID.
    static const string PAINT_NONE = "";

    static string GetSurfaceTexture(string paintCode, int scalePercent = 100)
    {
        if (paintCode == PAINT_NONE)
            return "";

        return PaintZ_PaintPackRegistry.GetSurfaceTexture(paintCode, scalePercent);
    }

    static string GetFinishName(string paintCode)
    {
        return PaintZ_PaintPackRegistry.GetFinishName(paintCode);
    }
};
