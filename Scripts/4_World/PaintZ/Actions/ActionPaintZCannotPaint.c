class ActionPaintZCannotPaint : ActionSingleUseBase
{
    void ActionPaintZCannotPaint()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_OPENDOORFW;
        m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
    }

    override void CreateConditionComponents()
    {
        m_ConditionItem = new CCINone;
        m_ConditionTarget = new CCTCursor;
    }

    override string GetText()
    {
        return "Cannot Paint";
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!PaintZ_SprayCanBase.Cast(item) || !target)
            return false;

        EntityAI entity = PaintZ_PaintTarget.ResolveActionTarget(target);
        if (!PaintZ_PaintInspector.IsSupportedTarget(entity))
            return false;

        PaintZ_PaintInspectionResult inspection = PaintZ_PaintInspector.Inspect(entity);
        return !inspection.m_Paintable;
    }

    override void OnExecuteServer(ActionData action_data)
    {
        if (!action_data.m_Target)
            return;

        EntityAI entity = PaintZ_PaintTarget.ResolveActionTarget(action_data.m_Target);
        PlayerBase player = action_data.m_Player;

        if (!entity || !player)
            return;

        PaintZ_PaintInspectionResult inspection = PaintZ_PaintInspector.Inspect(entity);
        player.MessageStatus("This item cannot be painted. " + inspection.m_Reason + ".");
    }
};
