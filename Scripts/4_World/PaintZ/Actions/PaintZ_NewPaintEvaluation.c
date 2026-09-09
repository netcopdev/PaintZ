enum PaintZ_NewPaintResult
{
    PZ_NEW_PAINT_SILENT = 0,
    PZ_NEW_PAINT_CAN_RUINED,
    PZ_NEW_PAINT_CAN_EMPTY,
    PZ_NEW_PAINT_TARGET_RUINED,
    PZ_NEW_PAINT_EXCLUDED,
    PZ_NEW_PAINT_UNSUPPORTED,
    PZ_NEW_PAINT_READY
};

class PaintZ_NewPaintEvaluation
{
    PaintZ_NewPaintResult m_Result = PaintZ_NewPaintResult.PZ_NEW_PAINT_SILENT;
    EntityAI m_Target;
    ref PaintZ_PaintInspectionResult m_Inspection;

    static PaintZ_NewPaintEvaluation Evaluate(ActionTarget actionTarget, ItemBase item)
    {
        PaintZ_NewPaintEvaluation evaluation = new PaintZ_NewPaintEvaluation();
        PaintZ_SprayCanBase spray = PaintZ_SprayCanBase.Cast(item);
        if (!spray || !actionTarget)
            return evaluation;

        string paintCode = spray.GetPaintZPaintCode();
        if (!PaintZ_PaintPackRegistry.HasFinish(paintCode))
            return evaluation;

        evaluation.m_Target = PaintZ_PaintTarget.ResolveNewPaintTarget(actionTarget);
        if (!PaintZ_ItemPolicy.IsRelevantTarget(evaluation.m_Target))
            return evaluation;

        if (spray.IsRuined())
        {
            evaluation.m_Result = PaintZ_NewPaintResult.PZ_NEW_PAINT_CAN_RUINED;
            return evaluation;
        }

        float requiredPaint = PaintZ_ActionTuning.ResolvePaintUsage(evaluation.m_Target, spray);
        if (spray.HasQuantity() && spray.GetQuantity() < requiredPaint)
        {
            evaluation.m_Result = PaintZ_NewPaintResult.PZ_NEW_PAINT_CAN_EMPTY;
            return evaluation;
        }

        if (evaluation.m_Target.IsRuined())
        {
            evaluation.m_Result = PaintZ_NewPaintResult.PZ_NEW_PAINT_TARGET_RUINED;
            return evaluation;
        }

        if (!PaintZ_ItemPolicy.IsPaintApplicationAllowed(evaluation.m_Target))
        {
            evaluation.m_Result = PaintZ_NewPaintResult.PZ_NEW_PAINT_EXCLUDED;
            return evaluation;
        }

        evaluation.m_Inspection = PaintZ_PaintInspector.Inspect(evaluation.m_Target);
        if (!evaluation.m_Inspection.m_Paintable || evaluation.m_Inspection.m_SelectionIndex < 0)
        {
            evaluation.m_Result = PaintZ_NewPaintResult.PZ_NEW_PAINT_UNSUPPORTED;
            return evaluation;
        }

        evaluation.m_Result = PaintZ_NewPaintResult.PZ_NEW_PAINT_READY;
        return evaluation;
    }

    static void SendFailure(PlayerBase player, PaintZ_NewPaintEvaluation evaluation)
    {
        if (!player || !evaluation)
            return;

        string reason;
        switch (evaluation.m_Result)
        {
            case PaintZ_NewPaintResult.PZ_NEW_PAINT_CAN_RUINED:
                player.MessageStatus("Ruined Spray Can");
                break;
            case PaintZ_NewPaintResult.PZ_NEW_PAINT_CAN_EMPTY:
                player.MessageStatus("Not enough paint remaining for this item.");
                break;
            case PaintZ_NewPaintResult.PZ_NEW_PAINT_TARGET_RUINED:
                player.MessageStatus("Cannot Paint: Item is ruined");
                break;
            case PaintZ_NewPaintResult.PZ_NEW_PAINT_EXCLUDED:
                player.MessageStatus("Cannot Paint: Item is excluded by server policy");
                break;
            case PaintZ_NewPaintResult.PZ_NEW_PAINT_UNSUPPORTED:
                if (evaluation.m_Inspection)
                    reason = " " + evaluation.m_Inspection.m_Reason + ".";
                player.MessageStatus("Cannot Paint: Unsupported." + reason);
                break;
        }
    }

    static string GetActionText(PaintZ_NewPaintResult result)
    {
        switch (result)
        {
            case PaintZ_NewPaintResult.PZ_NEW_PAINT_CAN_RUINED:
                return "Ruined Spray Can";
            case PaintZ_NewPaintResult.PZ_NEW_PAINT_CAN_EMPTY:
                return "Not Enough Paint";
            case PaintZ_NewPaintResult.PZ_NEW_PAINT_TARGET_RUINED:
                return "Cannot Paint - Ruined";
            case PaintZ_NewPaintResult.PZ_NEW_PAINT_EXCLUDED:
                return "Cannot Paint - Excluded";
            case PaintZ_NewPaintResult.PZ_NEW_PAINT_UNSUPPORTED:
                return "Cannot Paint - Unsupported";
        }

        return "Cannot Paint";
    }
};
