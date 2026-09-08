class PaintZ_PaintTarget
{
    static EntityAI ResolveNewPaintTarget(ActionTarget actionTarget)
    {
        if (!actionTarget)
            return null;

        EntityAI entity = EntityAI.Cast(actionTarget.GetObject());
        if (PaintZ_ItemPolicy.IsRelevantTarget(entity))
            return entity;

        EntityAI parent = EntityAI.Cast(actionTarget.GetParent());
        if (PaintZ_ItemPolicy.IsRelevantTarget(parent))
            return parent;

        return entity;
    }

    static EntityAI ResolvePaintedTarget(ActionTarget actionTarget)
    {
        if (!actionTarget)
            return null;

        EntityAI entity = EntityAI.Cast(actionTarget.GetObject());
        if (PaintZ_PaintedState.GetPaintedSelection(entity) >= 0)
            return entity;

        EntityAI parent = EntityAI.Cast(actionTarget.GetParent());
        if (PaintZ_PaintedState.GetPaintedSelection(parent) >= 0)
            return parent;

        return entity;
    }

    static bool SetPaint(EntityAI target, string paintCode, int selectionIndex)
    {
        if (!target)
            return false;

        // Eligibility controls new applications only. PAINT_NONE is stripping
        // and deliberately bypasses the runtime allow/exclude policy.
        if (paintCode != PaintZ_PaintConstants.PAINT_NONE)
        {
            if (!PaintZ_ItemPolicy.IsRelevantTarget(target) || target.IsRuined() || !PaintZ_ItemPolicy.IsPaintApplicationAllowed(target))
                return false;
        }

        // PaintZ state belongs to the shared inventory-item base, not to today's
        // configured domains. Weapons, magazines, attachments, clothing and
        // future ItemBase-derived domains all use exactly the same state path.
        ItemBase item = ItemBase.Cast(target);
        if (!item)
            return false;

        return item.PaintZ_SetPaintState(paintCode, selectionIndex);
    }
};
