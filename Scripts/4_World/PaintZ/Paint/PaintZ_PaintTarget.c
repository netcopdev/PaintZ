class PaintZ_PaintTarget
{
    static EntityAI ResolveActionTarget(ActionTarget actionTarget)
    {
        if (!actionTarget)
            return null;

        EntityAI entity = EntityAI.Cast(actionTarget.GetObject());
        if (PaintZ_PaintInspector.IsSupportedTarget(entity))
            return entity;

        EntityAI parent = EntityAI.Cast(actionTarget.GetParent());
        if (PaintZ_PaintInspector.IsSupportedTarget(parent))
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
            if (target.IsRuined() || !PaintZ_ItemPolicy.IsPaintApplicationAllowed(target))
                return false;
        }

        Weapon_Base weapon;
        if (Class.CastTo(weapon, target))
        {
            weapon.PaintZ_SetPaintState(paintCode, selectionIndex);
            return true;
        }

        Magazine magazine;
        if (Class.CastTo(magazine, target))
        {
            PaintZ_MagazinePaintState.SetPaint(magazine, paintCode, selectionIndex);
            return true;
        }

        return false;
    }
};
