modded class ItemBase
{
    protected string m_PaintZPaintCode = PaintZ_PaintConstants.PAINT_NONE;
    protected int m_PaintZPaintSelection = -1;
    protected int m_PaintZPaintCodeHash;

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

    void PaintZ_RestoreLoadedPaint()
    {
        PaintZ_PaintStateRuntime.RestorePersistedVisual(this, m_PaintZPaintCode, m_PaintZPaintSelection);
        m_PaintZPaintCodeHash = PaintZ_PaintStateRuntime.GetNetworkHash(m_PaintZPaintCode);
        if (GetGame().IsServer())
            SetSynchDirty();
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
