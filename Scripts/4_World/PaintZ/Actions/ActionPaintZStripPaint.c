class ActionPaintZStripPaint : ActionContinuousBase
{
    void ActionPaintZStripPaint()
    {
        m_CallbackClass = ActionPaintZPaintCB;
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
        m_FullBody = true;
        m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
    }

    override void CreateConditionComponents()
    {
        m_ConditionItem = new CCINone;
        m_ConditionTarget = new CCTCursor;
    }

    override string GetText()
    {
        return "Strip Paint";
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        PaintZ_PaintStripperCan stripper = PaintZ_PaintStripperCan.Cast(item);
        if (!stripper || !target)
            return false;

        if (stripper.HasQuantity() && stripper.GetQuantity() < PaintZ_PaintConstants.STRIP_COST)
            return false;

        EntityAI entity = PaintZ_PaintTarget.ResolveActionTarget(target);
        PaintZ_PaintInspectionResult inspection = PaintZ_PaintInspector.Inspect(entity);
        return inspection.m_Paintable && PaintZ_PaintVisuals.HasPaint(entity, inspection.m_SelectionIndex);
    }

    override void OnFinishProgressServer(ActionData action_data)
    {
        if (!action_data.m_Target)
            return;

        EntityAI entity = PaintZ_PaintTarget.ResolveActionTarget(action_data.m_Target);
        PaintZ_PaintStripperCan stripper = PaintZ_PaintStripperCan.Cast(action_data.m_MainItem);
        PlayerBase player = action_data.m_Player;
        if (!entity || !stripper || !player)
            return;

        if (stripper.HasQuantity() && stripper.GetQuantity() < PaintZ_PaintConstants.STRIP_COST)
        {
            player.MessageStatus("Not enough paint stripper remaining.");
            return;
        }

        PaintZ_PaintInspectionResult inspection = PaintZ_PaintInspector.Inspect(entity);
        if (!inspection.m_Paintable || inspection.m_SelectionIndex < 0)
        {
            player.MessageStatus("This item's paint cannot be stripped safely.");
            return;
        }

        // Another player may have stripped it while this action was running.
        if (!PaintZ_PaintVisuals.HasPaint(entity, inspection.m_SelectionIndex))
        {
            PaintZ_PaintLog.Info("strip rejected target=" + entity.GetType() + " reason=No PaintZ paint on selected surface");
            player.MessageStatus("This item has no PaintZ paint to strip.");
            return;
        }

        if (!PaintZ_PaintTarget.SetPaint(entity, PaintZ_PaintConstants.PAINT_NONE, inspection.m_SelectionIndex))
        {
            player.MessageStatus("PaintZ could not restore this item's original finish.");
            return;
        }

        stripper.AddQuantity(-PaintZ_PaintConstants.STRIP_COST, false);
        PaintZ_PaintLog.Info("stripped target=" + entity.GetType() + " selection=" + inspection.m_SelectionName + " index=" + inspection.m_SelectionIndex);
        player.MessageStatus("Original finish restored on " + entity.GetDisplayName() + ".");
    }
};
