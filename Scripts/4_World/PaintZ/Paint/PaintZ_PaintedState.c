class PaintZ_PaintedState
{
    static bool SupportsTarget(EntityAI target)
    {
        if (!target)
            return false;

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

        ItemBase item;
        if (Class.CastTo(item, target))
        {
            int recordedSelection = item.PaintZ_GetPaintSelection();
            if (item.PaintZ_GetPaintCode() != PaintZ_PaintConstants.PAINT_NONE && PaintZ_PaintVisuals.HasPaint(target, recordedSelection))
                return recordedSelection;
        }

        // Native Magazine cannot carry modded script fields. Scanning the live
        // PaintZ texture marker also recovers state after heuristics or domains
        // change, and keeps stripping independent from new-paint eligibility.
        TStringArray selections = PaintZ_PaintInspector.GetRuntimeSelections(target);
        for (int i = 0; selections && i < selections.Count(); i++)
        {
            if (PaintZ_PaintVisuals.HasPaint(target, i))
                return i;
        }

        return -1;
    }
};
