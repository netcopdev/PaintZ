class PaintZ_PaintInspector
{
    static PaintZ_PaintInspectionResult Inspect(EntityAI target)
    {
        PaintZ_PaintInspectionResult result = new PaintZ_PaintInspectionResult();

        if (!target || !PaintZ_PaintedState.SupportsTarget(target))
        {
            result.m_Reason = "Target type cannot carry PaintZ state";
            return result;
        }

        TStringArray selections = GetRuntimeSelections(target);
        if (!selections || selections.Count() == 0)
        {
            result.m_Reason = "Model exposes no hidden selections";
            PaintZ_PaintLog.Inspect(target, selections, result);
            return result;
        }

        // First pass: exact globally preferred body-like names.
        for (int i = 0; i < selections.Count(); i++)
        {
            string candidate = selections.Get(i);
            string lowered = candidate;
            lowered.ToLower();

            if (IsBlockedName(lowered))
                continue;

            if (IsPreferredName(lowered))
            {
                int preferredIndex = GetRuntimeSelectionIndex(target, selections, candidate);
                if (preferredIndex < 0)
                    continue;

                result.m_Paintable = true;
                result.m_SelectionName = candidate;
                result.m_SelectionIndex = preferredIndex;
                result.m_Reason = "Matched preferred body selection";
                PaintZ_PaintLog.Inspect(target, selections, result);
                return result;
            }
        }

        // Conservative generic fallback: if the model exposes exactly one
        // selection, accept it unless its name clearly describes a non-body
        // visual such as glass/reticle/display/light.
        if (selections.Count() == 1)
        {
            string onlyName = selections.Get(0);
            string onlyLower = onlyName;
            onlyLower.ToLower();

            if (!IsBlockedName(onlyLower))
            {
                int onlyIndex = GetRuntimeSelectionIndex(target, selections, onlyName);
                if (onlyIndex >= 0)
                {
                    result.m_Paintable = true;
                    result.m_SelectionName = onlyName;
                    result.m_SelectionIndex = onlyIndex;
                    result.m_Reason = "Accepted sole non-blocked hidden selection";
                    PaintZ_PaintLog.Inspect(target, selections, result);
                    return result;
                }
            }
        }

        result.m_Reason = "No safe body/camo hidden selection could be inferred";
        PaintZ_PaintLog.Inspect(target, selections, result);
        return result;
    }

    protected static bool IsPreferredName(string name)
    {
        if (name == "camo" || name == "zbytek" || name == "body" || name == "body1")
            return true;

        if (name == "body2" || name == "receiver" || name == "weapon" || name == "mag" || name == "magazine")
            return true;

        return false;
    }

    static TStringArray GetRuntimeSelections(EntityAI target)
    {
        TStringArray selections = target.GetHiddenSelections();
        if (selections && selections.Count() > 0)
            return selections;

        // DayZ 1.29 EntityAI initializes HiddenSelectionsData from CfgVehicles
        // for every entity. That leaves the live getter empty for CfgWeapons
        // and CfgMagazines objects. Fall back to the actual target type's
        // inherited config array; this remains runtime inspection and contains
        // no item classname knowledge.
        selections = new TStringArray();
        string configRoot = GetConfigRoot(target);
        if (configRoot == "")
            return selections;

        GetGame().ConfigGetTextArray(configRoot + " " + target.GetType() + " hiddenSelections", selections);
        return selections;
    }

    static string GetConfigRoot(EntityAI target)
    {
        if (!target || !GetGame())
            return "";

        string className = target.GetType();
        if (GetGame().ConfigIsExisting("CfgWeapons " + className))
            return "CfgWeapons";
        if (GetGame().ConfigIsExisting("CfgMagazines " + className))
            return "CfgMagazines";
        if (GetGame().ConfigIsExisting("CfgVehicles " + className))
            return "CfgVehicles";
        return "";
    }

    protected static int GetRuntimeSelectionIndex(EntityAI target, TStringArray selections, string selectionName)
    {
        int liveIndex = target.GetHiddenSelectionIndex(selectionName);
        if (liveIndex >= 0)
            return liveIndex;

        // The SetObjectTexture index is the position in hiddenSelections[].
        // Use that position when the DayZ 1.29 EntityAI cache is empty.
        for (int i = 0; i < selections.Count(); i++)
        {
            if (selections.Get(i) == selectionName)
                return i;
        }

        return -1;
    }

    protected static bool IsBlockedName(string name)
    {
        TStringArray blocked = {
            "glass",
            "lens",
            "reticle",
            "optic",
            "display",
            "screen",
            "led",
            "light",
            "emissive",
            "glow",
            "flame"
        };

        for (int i = 0; i < blocked.Count(); i++)
        {
            if (name.IndexOf(blocked.Get(i)) != -1)
                return true;
        }

        return false;
    }
};
