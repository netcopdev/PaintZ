class PaintZ_PaintedState
{
    static bool SupportsTarget(EntityAI target)
    {
        return ItemBase.Cast(target) != null;
    }

    static int GetPaintedSelection(EntityAI target)
    {
        ItemBase item = ItemBase.Cast(target);
        if (!item)
            return -1;

        int recordedSelection = item.PaintZ_GetPaintSelection();
        if (item.PaintZ_GetPaintCode() != PaintZ_PaintConstants.PAINT_NONE && PaintZ_PaintVisuals.HasPaint(target, recordedSelection))
            return recordedSelection;

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
