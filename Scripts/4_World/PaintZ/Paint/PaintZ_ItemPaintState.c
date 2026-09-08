modded class ItemBase
{
    protected string m_PaintZPaintCode = PaintZ_PaintConstants.PAINT_NONE;
    protected int m_PaintZPaintSelection = -1;
    protected int m_PaintZPaintCodeHash;
    protected int m_PaintZPatternScalePercent = 100;
    protected bool m_PaintZRestoreQueued;

    void ItemBase()
    {
        RegisterNetSyncVariableInt("m_PaintZPaintCodeHash", int.MIN, int.MAX);
        RegisterNetSyncVariableInt("m_PaintZPaintSelection", -1, 255);

        // Transient visual state only. CF persistence deliberately stores the
        // finish ID and derives this scale again on repaint/post-load restore.
        RegisterNetSyncVariableInt("m_PaintZPatternScalePercent", 1, 1000);
    }

    bool PaintZ_SetPaintState(string paintCode, int selectionIndex)
    {
        int scalePercent = 100;
        float maxDimensionMeters = -1.0;

        if (paintCode != PaintZ_PaintConstants.PAINT_NONE)
            scalePercent = PaintZ_PatternScaling.ResolveScalePercent(this, paintCode, maxDimensionMeters);

        if (!PaintZ_PaintVisuals.Apply(this, paintCode, selectionIndex, scalePercent))
            return false;

        m_PaintZPaintCode = paintCode;
        m_PaintZPaintSelection = selectionIndex;
        m_PaintZPaintCodeHash = PaintZ_PaintStateRuntime.GetNetworkHash(paintCode);
        m_PaintZPatternScalePercent = scalePercent;
        SetSynchDirty();

        if (PaintZ_PaintCatalog.IsPatternPaint(paintCode))
        {
            float selectedScale = scalePercent * 0.01;
            string logText = "pattern_scale target=" + GetType();
            logText += " max_dimension_m=" + maxDimensionMeters;
            logText += " scale=" + selectedScale;
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

    void PaintZ_LoadPaintState(string paintCode)
    {
        m_PaintZPaintCode = paintCode;
        m_PaintZPaintSelection = -1;
        m_PaintZPaintCodeHash = PaintZ_PaintStateRuntime.GetNetworkHash(paintCode);
        m_PaintZPatternScalePercent = 100;
    }

    void PaintZ_QueueLoadedPaintRestore()
    {
        if (m_PaintZRestoreQueued || m_PaintZPaintCode == PaintZ_PaintConstants.PAINT_NONE || !GetGame())
            return;

        // CF_OnStoreLoad runs inside the persistence serializer. Defer only the
        // model/hidden-selection work so every ItemBase descendant uses the same
        // persistence hook without depending on subclass AfterStoreLoad chains.
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
        PaintZ_PaintStateRuntime.RestorePersistedVisual(
            this,
            m_PaintZPaintCode,
            m_PaintZPaintSelection,
            m_PaintZPatternScalePercent
        );

        m_PaintZPaintCodeHash = PaintZ_PaintStateRuntime.GetNetworkHash(m_PaintZPaintCode);
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
        if (m_PaintZPaintSelection < 0)
            return;

        m_PaintZPaintCode = PaintZ_PaintStateRuntime.GetNetworkPaintCode(m_PaintZPaintCodeHash);
        PaintZ_PaintVisuals.Apply(
            this,
            m_PaintZPaintCode,
            m_PaintZPaintSelection,
            m_PaintZPatternScalePercent
        );
    }
};