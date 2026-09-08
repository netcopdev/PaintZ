class PaintZ_PaintVisuals
{
    static bool HasPaint(EntityAI target, int selectionIndex)
    {
        if (!target || selectionIndex < 0)
            return false;

        // Read the current texture, not the config defaults.
        string texture = target.GetObjectTexture(selectionIndex);
        texture.ToLower();
        texture.Replace("/", "\\");
        return texture.IndexOf("paintz\\data\\surfaces\\pz_") == 0;
    }

    static void Apply(EntityAI target, string paintCode, int selectionIndex)
    {
        if (!target || selectionIndex < 0)
            return;

        if (paintCode == PaintZ_PaintConstants.PAINT_NONE)
            RestoreConfiguredTexture(target, selectionIndex);
        else
            target.SetObjectTexture(selectionIndex, PaintZ_PaintConstants.GetSurfaceTexture(paintCode));
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

