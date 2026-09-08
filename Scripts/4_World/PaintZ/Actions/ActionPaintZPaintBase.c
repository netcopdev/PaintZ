class ActionPaintZPaintCB : ActionContinuousBaseCB
{
    override void CreateActionComponent()
    {
        m_ActionData.m_ActionComponent = new CAContinuousTime(2.0);
    }
};

class ActionPaintZPaintBase : ActionContinuousBase
{
    protected static int s_PaintZConditionTraceBudget = 12;

    void ActionPaintZPaintBase()
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

    string GetPaintCode()
    {
        return PaintZ_PaintConstants.PAINT_NONE;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        PaintZ_SprayCanBase spray = PaintZ_SprayCanBase.Cast(item);
        if (!spray || spray.GetPaintZPaintCode() != GetPaintCode() || !target)
            return false;

        PaintZ_NewPaintEvaluation evaluation = PaintZ_NewPaintEvaluation.Evaluate(target, item);
        TraceCondition(item, target, evaluation.m_Target);
        return evaluation.m_Result == PaintZ_NewPaintResult.PZ_NEW_PAINT_READY;
    }

    override void OnFinishProgressServer(ActionData action_data)
    {
        if (!action_data.m_Target)
            return;

        PaintZ_SprayCanBase spray = PaintZ_SprayCanBase.Cast(action_data.m_MainItem);
        PlayerBase player = action_data.m_Player;
        string paintCode = GetPaintCode();

        if (!spray || !player || spray.GetPaintZPaintCode() != paintCode)
            return;

        PaintZ_NewPaintEvaluation evaluation = PaintZ_NewPaintEvaluation.Evaluate(action_data.m_Target, action_data.m_MainItem);
        if (evaluation.m_Result != PaintZ_NewPaintResult.PZ_NEW_PAINT_READY)
        {
            PaintZ_NewPaintEvaluation.SendFailure(player, evaluation);
            return;
        }

        EntityAI entity = evaluation.m_Target;
        PaintZ_PaintInspectionResult inspection = evaluation.m_Inspection;

        if (!PaintZ_PaintTarget.SetPaint(entity, paintCode, inspection.m_SelectionIndex))
        {
            player.MessageStatus("PaintZ could not apply paint to this item.");
            return;
        }

        spray.AddQuantity(-PaintZ_PaintConstants.SPRAY_COST, false);
        string finishName = PaintZ_PaintConstants.GetFinishName(paintCode);
        PaintZ_PaintLog.Info("applied=" + paintCode + " name=" + finishName + " target=" + entity.GetType() + " selection=" + inspection.m_SelectionName + " index=" + inspection.m_SelectionIndex);
        player.MessageStatus(finishName + " paint applied to " + entity.GetDisplayName() + ".");
    }

    protected void TraceCondition(ItemBase item, ActionTarget target, EntityAI resolved)
    {
        if (s_PaintZConditionTraceBudget <= 0)
            return;

        s_PaintZConditionTraceBudget--;
        string itemType = "<null>";
        string objectType = "<null>";
        string parentType = "<null>";
        string resolvedType = "<null>";

        if (item)
            itemType = item.GetType();
        if (target.GetObject())
            objectType = target.GetObject().GetType();
        if (target.GetParent())
            parentType = target.GetParent().GetType();
        if (resolved)
            resolvedType = resolved.GetType();

        Print("[PaintZ][Actions] Paint condition item=" + itemType + " object=" + objectType + " parent=" + parentType + " resolved=" + resolvedType);
    }
};
