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
            string configRoot;

            Weapon_Base weapon;
            Magazine magazine;
            if (Class.CastTo(weapon, target))
                configRoot = "CfgWeapons";
            else if (Class.CastTo(magazine, target))
                configRoot = "CfgMagazines";

            if (configRoot != "")
                GetGame().ConfigGetTextArray(configRoot + " " + target.GetType() + " hiddenSelectionsTextures", textures);
        }

        string configuredTexture = "";
        if (textures && selectionIndex < textures.Count())
            configuredTexture = textures.Get(selectionIndex);

        target.SetObjectTexture(selectionIndex, configuredTexture);
    }
};

