class PaintZ_PaintedState
{
    static bool SupportsTarget(EntityAI target)
    {
        if (!target)
            return false;

        Weapon_Base weapon;
        if (Class.CastTo(weapon, target))
            return true;

        ItemBase item;
        if (Class.CastTo(item, target))
            return true;

        Magazine magazine;
        return Class.CastTo(magazine, target) && !target.IsAmmoPile();
    }

    static int GetPaintedSelection(EntityAI target)
    {
        if (!SupportsTarget(target))
            return -1;

        Weapon_Base weapon;
        if (Class.CastTo(weapon, target))
        {
            int weaponSelection = weapon.PaintZ_GetPaintSelection();
            if (weapon.PaintZ_GetPaintCode() != PaintZ_PaintConstants.PAINT_NONE && PaintZ_PaintVisuals.HasPaint(target, weaponSelection))
                return weaponSelection;
        }

        ItemBase item;
        if (Class.CastTo(item, target))
        {
            int recordedSelection = item.PaintZ_GetPaintSelection();
            if (item.PaintZ_GetPaintCode() != PaintZ_PaintConstants.PAINT_NONE && PaintZ_PaintVisuals.HasPaint(target, recordedSelection))
                return recordedSelection;
        }

        Magazine magazine;
        if (Class.CastTo(magazine, target))
        {
            int magazineSelection = magazine.PaintZ_GetPaintSelection();
            if (magazine.PaintZ_GetPaintCode() != PaintZ_PaintConstants.PAINT_NONE && PaintZ_PaintVisuals.HasPaint(target, magazineSelection))
                return magazineSelection;
        }

        // Scanning the live PaintZ texture marker keeps stripping independent
        // from new-paint eligibility and recovers from external texture changes.
        TStringArray selections = PaintZ_PaintInspector.GetRuntimeSelections(target);
        for (int i = 0; selections && i < selections.Count(); i++)
        {
            if (PaintZ_PaintVisuals.HasPaint(target, i))
                return i;
        }

        return -1;
    }
};
