class ActionPaintZStripPaint : ActionContinuousBase
{
    void ActionPaintZStripPaint()
    {
        m_CallbackClass = ActionPaintZPaintCB;
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_SPRAYPLANT;
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

        EntityAI entity = PaintZ_PaintTarget.ResolvePaintedTarget(target);
        return PaintZ_PaintedState.GetPaintedSelection(entity) >= 0;
    }

    override void OnFinishProgressServer(ActionData action_data)
    {
        if (!action_data.m_Target)
            return;

        EntityAI entity = PaintZ_PaintTarget.ResolvePaintedTarget(action_data.m_Target);
        PaintZ_PaintStripperCan stripper = PaintZ_PaintStripperCan.Cast(action_data.m_MainItem);
        PlayerBase player = action_data.m_Player;
        if (!entity || !stripper || !player)
            return;

        if (stripper.HasQuantity() && stripper.GetQuantity() < PaintZ_PaintConstants.STRIP_COST)
        {
            player.MessageStatus("Not enough paint stripper remaining.");
            return;
        }

        // Another player may have stripped it while this action was running.
        int selectionIndex = PaintZ_PaintedState.GetPaintedSelection(entity);
        if (selectionIndex < 0)
        {
            PaintZ_PaintLog.Info("strip rejected target=" + entity.GetType() + " reason=No PaintZ painted state");
            player.MessageStatus("This item has no PaintZ paint to strip.");
            return;
        }

        if (!PaintZ_PaintTarget.SetPaint(entity, PaintZ_PaintConstants.PAINT_NONE, selectionIndex))
        {
            player.MessageStatus("PaintZ could not restore this item's original finish.");
            return;
        }

        stripper.AddQuantity(-PaintZ_PaintConstants.STRIP_COST, false);
        PaintZ_PaintLog.Info("stripped target=" + entity.GetType() + " selection_index=" + selectionIndex);
        player.MessageStatus("Original finish restored on " + entity.GetDisplayName() + ".");
    }
};
