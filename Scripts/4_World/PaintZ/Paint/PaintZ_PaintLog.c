class PaintZ_PaintLog
{
    protected static ref map<string, string> s_LastInspectionByType;

    static void Info(string text)
    {
        Print("[PaintZ][Paint] " + text);
    }

    static void Inspect(EntityAI target, TStringArray selections, PaintZ_PaintInspectionResult result)
    {
        if (!target)
            return;

        string joined = "";
        if (selections)
        {
            for (int i = 0; i < selections.Count(); i++)
            {
                if (i > 0)
                    joined += ", ";
                joined += selections.Get(i);
            }
        }

        string targetType = target.GetType();
        string decision = "selections=[" + joined + "] paintable=" + result.m_Paintable + " selected=" + result.m_SelectionName + " index=" + result.m_SelectionIndex + " reason=" + result.m_Reason;

        if (!s_LastInspectionByType)
            s_LastInspectionByType = new map<string, string>();

        string previousDecision;
        if (s_LastInspectionByType.Find(targetType, previousDecision) && previousDecision == decision)
            return;

        s_LastInspectionByType.Set(targetType, decision);
        Info("target=" + targetType + " " + decision);
    }
};
