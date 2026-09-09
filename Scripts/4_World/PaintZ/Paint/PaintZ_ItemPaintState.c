class PaintZ_ItemDisplay
{
    static string ResolveFinishName(string paintCode)
    {
        string finishName = PaintZ_PaintConstants.GetFinishName(paintCode);
        if (finishName == "")
            return paintCode;

        return finishName;
    }

    static string FormatDisplayName(string baseName, string finishName)
    {
        return baseName + " [" + finishName + "]";
    }

    static string FormatDescription(string baseDescription, string finishName, string finishId)
    {
        string finishLine = "Finish: " + finishName + " (" + finishId + ")";
        if (baseDescription == "")
            return finishLine;

        return baseDescription + "\n \n" + finishLine;
    }
};

modded class ItemBase
{
    protected string m_PaintZPaintCode = PaintZ_PaintConstants.PAINT_NONE;
    protected int m_PaintZPaintSelection = -1;
    protected int m_PaintZPaintCodeHash;
    protected int m_PaintZPatternScalePercent = 100;
    protected bool m_PaintZHasState;
    protected bool m_PaintZRestoreQueued;

    void ItemBase()
    {
        RegisterNetSyncVariableInt("m_PaintZPaintCodeHash", int.MIN, int.MAX);
        RegisterNetSyncVariableInt("m_PaintZPaintSelection", -1, 255);
        RegisterNetSyncVariableInt("m_PaintZPatternScalePercent", 1, 1000);
        RegisterNetSyncVariableBool("m_PaintZHasState");
    }

    bool PaintZ_SetPaintState(string paintCode, int selectionIndex)
    {
        if (paintCode == PaintZ_PaintConstants.PAINT_NONE)
        {
            if (selectionIndex >= 0 && !PaintZ_PaintVisuals.Apply(this, paintCode, selectionIndex, 100))
                return false;

            m_PaintZPaintCode = PaintZ_PaintConstants.PAINT_NONE;
            m_PaintZPaintSelection = -1;
            m_PaintZPaintCodeHash = 0;
            m_PaintZPatternScalePercent = 100;
            m_PaintZHasState = false;
            SetSynchDirty();
            return true;
        }

        int scalePercent = 100;
        float maxDimensionMeters = -1.0;
        scalePercent = PaintZ_PatternScaling.ResolveScalePercent(this, paintCode, maxDimensionMeters);

        if (!PaintZ_PaintVisuals.Apply(this, paintCode, selectionIndex, scalePercent))
            return false;

        m_PaintZPaintCode = paintCode;
        m_PaintZPaintSelection = selectionIndex;
        m_PaintZPaintCodeHash = PaintZ_PaintStateRuntime.GetNetworkHash(paintCode);
        m_PaintZPatternScalePercent = scalePercent;
        m_PaintZHasState = true;
        SetSynchDirty();

        if (PaintZ_PaintPackRegistry.IsPatternFinish(paintCode))
        {
            float selectedScale = scalePercent * 0.01;
            string logText = "pattern_scale target=" + GetType();
            logText += " max_dimension_m=" + maxDimensionMeters.ToString();
            logText += " scale=" + selectedScale.ToString();
            PaintZ_PaintLog.Info(logText);
        }

        return true;
    }

    string PaintZ_GetPaintCode()
    {
        return m_PaintZPaintCode;
    }

    int PaintZ_GetPaintSelection()
    {
        return m_PaintZPaintSelection;
    }

    int PaintZ_GetPatternScalePercent()
    {
        return m_PaintZPatternScalePercent;
    }

    bool PaintZ_HasState()
    {
        return m_PaintZHasState;
    }

    protected string PaintZ_GetDisplayFinishName()
    {
        return PaintZ_ItemDisplay.ResolveFinishName(m_PaintZPaintCode);
    }

    override bool NameOverride(out string output)
    {
        string baseName;
        bool hasUpstreamOverride = super.NameOverride(baseName);
        if (m_PaintZPaintCode == PaintZ_PaintConstants.PAINT_NONE)
        {
            output = baseName;
            return hasUpstreamOverride;
        }

        if (hasUpstreamOverride)
            baseName = Widget.TranslateString(baseName);
        else
            g_Game.ObjectGetDisplayName(this, baseName);

        output = PaintZ_ItemDisplay.FormatDisplayName(baseName, PaintZ_GetDisplayFinishName());
        return true;
    }

    override bool DescriptionOverride(out string output)
    {
        string baseDescription;
        bool hasUpstreamOverride = super.DescriptionOverride(baseDescription);
        if (m_PaintZPaintCode == PaintZ_PaintConstants.PAINT_NONE)
        {
            output = baseDescription;
            return hasUpstreamOverride;
        }

        if (!hasUpstreamOverride)
            baseDescription = ConfigGetString("descriptionShort");
        baseDescription = Widget.TranslateString(baseDescription);
        output = PaintZ_ItemDisplay.FormatDescription(baseDescription, PaintZ_GetDisplayFinishName(), m_PaintZPaintCode);
        return true;
    }

    void PaintZ_LoadPaintState(string paintCode)
    {
        m_PaintZPaintCode = paintCode;
        m_PaintZPaintSelection = -1;
        m_PaintZPaintCodeHash = PaintZ_PaintStateRuntime.GetNetworkHash(paintCode);
        m_PaintZPatternScalePercent = 100;
        m_PaintZHasState = paintCode != PaintZ_PaintConstants.PAINT_NONE;
    }

    void PaintZ_QueueLoadedPaintRestore()
    {
        if (m_PaintZRestoreQueued || !m_PaintZHasState || !GetGame())
            return;

        m_PaintZRestoreQueued = true;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(PaintZ_RestoreLoadedPaintDeferred, 0, false);
    }

    protected void PaintZ_RestoreLoadedPaintDeferred()
    {
        m_PaintZRestoreQueued = false;
        PaintZ_RestoreLoadedPaint();
    }

    void PaintZ_RestoreLoadedPaint()
    {
        int restoredSelection = -1;
        int restoredScalePercent = 100;
        PaintZ_PaintStateRuntime.RestorePersistedVisual(this, m_PaintZPaintCode, restoredSelection, restoredScalePercent);
        m_PaintZPaintSelection = restoredSelection;
        m_PaintZPatternScalePercent = restoredScalePercent;
        m_PaintZPaintCodeHash = PaintZ_PaintStateRuntime.GetNetworkHash(m_PaintZPaintCode);
        m_PaintZHasState = m_PaintZPaintCode != PaintZ_PaintConstants.PAINT_NONE;
        if (GetGame().IsServer())
            SetSynchDirty();
    }

    override void CF_OnStoreSave(CF_ModStorageMap storage)
    {
        super.CF_OnStoreSave(storage);
        PaintZ_PaintPersistence.Save(storage, this, m_PaintZPaintCode);
    }

    override bool CF_OnStoreLoad(CF_ModStorageMap storage)
    {
        if (!super.CF_OnStoreLoad(storage))
            return false;

        string paintCode;
        if (!PaintZ_PaintPersistence.Load(storage, this, paintCode))
            return false;

        PaintZ_LoadPaintState(paintCode);
        PaintZ_QueueLoadedPaintRestore();
        return true;
    }

    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();
        m_PaintZPaintCode = PaintZ_PaintStateRuntime.GetNetworkPaintCode(m_PaintZPaintCodeHash);
        if (!m_PaintZHasState || m_PaintZPaintSelection < 0)
            return;

        PaintZ_PaintVisuals.Apply(this, m_PaintZPaintCode, m_PaintZPaintSelection, m_PaintZPatternScalePercent);
    }
};
