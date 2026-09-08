modded class ItemBase
{
    protected string m_PaintZPaintCode = PaintZ_PaintConstants.PAINT_NONE;
    protected int m_PaintZPaintSelection = -1;
    protected int m_PaintZPaintCodeHash;
    protected bool m_PaintZRestoreQueued;

    void ItemBase()
    {
        RegisterNetSyncVariableInt("m_PaintZPaintCodeHash", int.MIN, int.MAX);
        RegisterNetSyncVariableInt("m_PaintZPaintSelection", -1, 255);
    }

    bool PaintZ_SetPaintState(string paintCode, int selectionIndex)
    {
        if (!PaintZ_PaintVisuals.Apply(this, paintCode, selectionIndex))
            return false;

        m_PaintZPaintCode = paintCode;
        m_PaintZPaintSelection = selectionIndex;
        m_PaintZPaintCodeHash = PaintZ_PaintStateRuntime.GetNetworkHash(paintCode);
        SetSynchDirty();
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

    void PaintZ_LoadPaintState(string paintCode)
    {
        m_PaintZPaintCode = paintCode;
        m_PaintZPaintSelection = -1;
        m_PaintZPaintCodeHash = PaintZ_PaintStateRuntime.GetNetworkHash(paintCode);
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
        PaintZ_PaintStateRuntime.RestorePersistedVisual(this, m_PaintZPaintCode, m_PaintZPaintSelection);
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
        PaintZ_PaintVisuals.Apply(this, m_PaintZPaintCode, m_PaintZPaintSelection);
    }
};
