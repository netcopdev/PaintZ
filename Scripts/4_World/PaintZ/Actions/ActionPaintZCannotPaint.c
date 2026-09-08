class ActionPaintZCannotPaint : ActionSingleUseBase
{
    void ActionPaintZCannotPaint()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_OPENDOORFW;
        m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
        m_Text = "Cannot Paint";
    }

    override void CreateConditionComponents()
    {
        m_ConditionItem = new CCINone;
        m_ConditionTarget = new CCTCursor;
    }

    override string GetText()
    {
        return m_Text;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        PaintZ_SprayCanBase spray = PaintZ_SprayCanBase.Cast(item);
        if (!spray || !target)
            return false;

        PaintZ_NewPaintEvaluation evaluation = PaintZ_NewPaintEvaluation.Evaluate(target, item);
        m_Text = PaintZ_NewPaintEvaluation.GetActionText(evaluation.m_Result);
        return evaluation.m_Result != PaintZ_NewPaintResult.PZ_NEW_PAINT_SILENT && evaluation.m_Result != PaintZ_NewPaintResult.PZ_NEW_PAINT_READY;
    }

    override void OnExecuteServer(ActionData action_data)
    {
        if (!action_data.m_Target)
            return;

        PlayerBase player = action_data.m_Player;
        if (!player)
            return;

        PaintZ_NewPaintEvaluation evaluation = PaintZ_NewPaintEvaluation.Evaluate(action_data.m_Target, action_data.m_MainItem);
        PaintZ_NewPaintEvaluation.SendFailure(player, evaluation);
    }
};
