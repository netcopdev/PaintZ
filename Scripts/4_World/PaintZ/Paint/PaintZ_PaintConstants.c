class PaintZ_PaintConstants
{
    // A paint's product code is its sole persistent identity. Pattern scale is
    // derived visual state and must never become part of the product ID.
    static const string PAINT_NONE = "";

    static string GetSurfaceTexture(string paintCode, int scalePercent = 100)
    {
        if (paintCode == PAINT_NONE)
            return "";

        string textureStem = paintCode;
        textureStem.ToLower();
        textureStem.Replace("-", "_");

        if (!PaintZ_PaintCatalog.IsPatternPaint(paintCode) || !PaintZ_PaintCatalog.IsSupportedPatternScale(scalePercent))
            scalePercent = 100;

        if (scalePercent == 100)
            return "paintz\\data\\surfaces\\" + textureStem + "_co.paa";

        string scaleText = "" + scalePercent;
        if (scalePercent < 10)
            scaleText = "00" + scaleText;
        else if (scalePercent < 100)
            scaleText = "0" + scaleText;

        return "paintz\\data\\surfaces\\" + textureStem + "_s" + scaleText + "_co.paa";
    }

    static string GetFinishName(string paintCode)
    {
        return PaintZ_PaintCatalog.GetFinishName(paintCode);
    }
};
