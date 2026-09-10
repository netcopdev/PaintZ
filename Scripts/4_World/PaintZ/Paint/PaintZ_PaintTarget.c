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
        if (PaintZ_PaintedState.HasPaintState(entity))
            return entity;

        EntityAI parent = EntityAI.Cast(actionTarget.GetParent());
        if (PaintZ_PaintedState.HasPaintState(parent))
            return parent;

        return entity;
    }

    static bool SetPaint(EntityAI target, string paintCode, int selectionIndex)
    {
        if (!target)
            return false;

        ItemBase item = ItemBase.Cast(target);
        if (!item)
            return false;

        item.PaintZ_ResolveStaleFinishState();

        if (paintCode != PaintZ_PaintConstants.PAINT_NONE)
        {
            if (!PaintZ_PaintPackRegistry.HasFinish(paintCode))
                return false;
            if (!PaintZ_ItemPolicy.IsRelevantTarget(target) || target.IsRuined() || !PaintZ_ItemPolicy.IsPaintApplicationAllowed(target))
                return false;
        }

        return item.PaintZ_SetPaintState(paintCode, selectionIndex);
    }
};
