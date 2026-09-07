class PaintZ_PaintConstants
{
    static const int SPRAY_COST = 10;
    static const int STRIP_COST = 10;

    // A paint's product code is its sole identity. Do not add numeric aliases.
    static const string PAINT_NONE = "";

    static string GetSurfaceTexture(string paintCode)
    {
        if (paintCode == PAINT_NONE)
            return "";

        string textureStem = paintCode;
        textureStem.ToLower();
        textureStem.Replace("-", "_");
        return "paintz\\data\\surfaces\\" + textureStem + "_co.paa";
    }

    static string GetFinishName(string paintCode)
    {
        return PaintZ_PaintCatalog.GetFinishName(paintCode);
    }
};
