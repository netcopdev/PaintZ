class ActionPaintZStripPaintCB : ActionContinuousBaseCB
{
    override void CreateActionComponent()
    {
        float durationSeconds = PaintZ_ActionTuning.ResolveStripTime(m_ActionData.m_Target);
        m_ActionData.m_ActionComponent = new CAContinuousTime(durationSeconds);
    }
};

class ActionPaintZStripPaint : ActionContinuousBase
{
    void ActionPaintZStripPaint()
    {
        m_CallbackClass = ActionPaintZStripPaintCB;
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

        EntityAI entity = PaintZ_PaintTarget.ResolvePaintedTarget(target);
        if (!PaintZ_PaintedState.HasPaintState(entity))
            return false;

        float requiredStripper = PaintZ_ActionTuning.ResolveStripUsage(entity, stripper);
        if (stripper.HasQuantity() && stripper.GetQuantity() < requiredStripper)
            return false;

        return true;
    }

    override void OnFinishProgressServer(ActionData action_data)
    {
        if (!action_data.m_Target)
            return;

        EntityAI entity = PaintZ_PaintTarget.ResolvePaintedTarget(action_data.m_Target);
        PaintZ_PaintStripperCan stripper = PaintZ_PaintStripperCan.Cast(action_data.m_MainItem);
        PlayerBase player = action_data.m_Player;
        if (!entity || !stripper || !player || !PaintZ_PaintedState.HasPaintState(entity))
            return;

        float stripUsage = PaintZ_ActionTuning.ResolveStripUsage(entity, stripper);
        if (stripper.HasQuantity() && stripper.GetQuantity() < stripUsage)
        {
            player.MessageStatus("Not enough paint stripper remaining for this item.");
            return;
        }

        int selectionIndex = PaintZ_PaintedState.GetPaintedSelection(entity);
        float effectiveDimensionMeters = PaintZ_ActionTuning.ResolveDimensionMeters(entity);
        if (!PaintZ_PaintTarget.SetPaint(entity, PaintZ_PaintConstants.PAINT_NONE, selectionIndex))
        {
            player.MessageStatus("PaintZ could not restore this item's original finish.");
            return;
        }

        stripper.AddQuantity(-stripUsage, false);
        PaintZ_PaintLog.Info("stripped target=" + entity.GetType() + " selection_index=" + selectionIndex + " effective_dimension_m=" + effectiveDimensionMeters + " stripper_usage=" + stripUsage);
        player.MessageStatus("Original finish restored on " + entity.GetDisplayName() + ".");
    }
};
