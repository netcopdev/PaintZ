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
    protected int m_PaintZStaleRecoveryRevision = -1;

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

    void PaintZ_ResolveStaleFinishState()
    {
        if (!GetGame() || !GetGame().IsServer() || !m_PaintZHasState)
            return;

        int recoveryRevision = PaintZ_StaleFinishRecovery.GetRevision();
        if (recoveryRevision <= 0 || m_PaintZStaleRecoveryRevision == recoveryRevision)
            return;

        m_PaintZStaleRecoveryRevision = recoveryRevision;
        PaintZ_StaleFinishRecovery.Resolve(this);
    }

    protected string PaintZ_GetDisplayPaintCode()
    {
        if (!m_PaintZHasState)
            return PaintZ_PaintConstants.PAINT_NONE;

        if (m_PaintZPaintCode != PaintZ_PaintConstants.PAINT_NONE)
            return m_PaintZPaintCode;

        return PaintZ_PaintStateRuntime.GetNetworkPaintCode(m_PaintZPaintCodeHash);
    }

    override string GetDisplayName()
    {
        string baseName = super.GetDisplayName();
        string paintCode = PaintZ_GetDisplayPaintCode();
        if (paintCode == PaintZ_PaintConstants.PAINT_NONE)
            return baseName;

        string finishName = PaintZ_ItemDisplay.ResolveFinishName(paintCode);
        return PaintZ_ItemDisplay.FormatDisplayName(baseName, finishName);
    }

    override string GetTooltip()
    {
        string baseDescription = super.GetTooltip();
        string paintCode = PaintZ_GetDisplayPaintCode();
        if (paintCode == PaintZ_PaintConstants.PAINT_NONE)
            return baseDescription;

        baseDescription = Widget.TranslateString(baseDescription);
        string finishName = PaintZ_ItemDisplay.ResolveFinishName(paintCode);
        return PaintZ_ItemDisplay.FormatDescription(baseDescription, finishName, paintCode);
    }

    void PaintZ_LoadPaintState(string paintCode)
    {
        m_PaintZPaintCode = paintCode;
        m_PaintZPaintSelection = -1;
        m_PaintZPaintCodeHash = PaintZ_PaintStateRuntime.GetNetworkHash(paintCode);
        m_PaintZPatternScalePercent = 100;
        m_PaintZHasState = paintCode != PaintZ_PaintConstants.PAINT_NONE;
        m_PaintZStaleRecoveryRevision = -1;
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
        PaintZ_ResolveStaleFinishState();

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
