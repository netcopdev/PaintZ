class PaintZ_PaintVisuals
{
    static bool HasPaint(EntityAI target, int selectionIndex)
    {
        if (!target || selectionIndex < 0)
            return false;

        string texture = target.GetObjectTexture(selectionIndex);
        return PaintZ_PaintPackRegistry.IsRegisteredSurfaceTexture(texture);
    }

    static bool Apply(EntityAI target, string paintCode, int selectionIndex, int scalePercent = 100)
    {
        if (!target || selectionIndex < 0)
            return false;

        if (paintCode == PaintZ_PaintConstants.PAINT_NONE)
        {
            RestoreConfiguredTexture(target, selectionIndex);
            return true;
        }

        if (!PaintZ_PaintPackRegistry.HasFinish(paintCode))
            return false;

        string texture = PaintZ_PaintConstants.GetSurfaceTexture(paintCode, scalePercent);
        if (texture == "")
            return false;

        target.SetObjectTexture(selectionIndex, texture);
        return true;
    }

    static bool RestoreIfKnown(EntityAI target, int selectionIndex)
    {
        if (!target || selectionIndex < 0)
            return false;

        RestoreConfiguredTexture(target, selectionIndex);
        return true;
    }

    protected static void RestoreConfiguredTexture(EntityAI target, int selectionIndex)
    {
        TStringArray textures = target.GetHiddenSelectionsTextures();
        if (!textures || textures.Count() == 0)
        {
            textures = new TStringArray();
            string configRoot = PaintZ_PaintInspector.GetConfigRoot(target);
            if (configRoot != "")
                GetGame().ConfigGetTextArray(configRoot + " " + target.GetType() + " hiddenSelectionsTextures", textures);
        }

        string configuredTexture = "";
        if (textures && selectionIndex < textures.Count())
            configuredTexture = textures.Get(selectionIndex);

        target.SetObjectTexture(selectionIndex, configuredTexture);
    }
};
