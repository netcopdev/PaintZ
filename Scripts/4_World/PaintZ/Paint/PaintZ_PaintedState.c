class PaintZ_PaintedState
{
    static bool SupportsTarget(EntityAI target)
    {
        return ItemBase.Cast(target) != null;
    }

    static bool HasPaintState(EntityAI target)
    {
        ItemBase item = ItemBase.Cast(target);
        if (!item)
            return false;

        if (item.PaintZ_GetPaintCode() != PaintZ_PaintConstants.PAINT_NONE)
            return true;
        if (item.PaintZ_GetPaintSelection() >= 0)
            return true;

        return GetPaintedSelection(target) >= 0;
    }

    static int GetPaintedSelection(EntityAI target)
    {
        ItemBase item = ItemBase.Cast(target);
        if (!item)
            return -1;

        int recordedSelection = item.PaintZ_GetPaintSelection();
        if (recordedSelection >= 0)
            return recordedSelection;

        TStringArray selections = PaintZ_PaintInspector.GetRuntimeSelections(target);
        for (int i = 0; selections && i < selections.Count(); i++)
        {
            if (PaintZ_PaintVisuals.HasPaint(target, i))
                return i;
        }

        return -1;
    }
};
