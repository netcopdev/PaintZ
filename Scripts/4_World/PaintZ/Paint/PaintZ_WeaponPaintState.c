modded class Weapon_Base
{
    protected static const int PAINTZ_RPC_PAINT_STATE = 782341;

    protected string m_PaintZPaintCode = PaintZ_PaintConstants.PAINT_NONE;
    protected int m_PaintZPaintSelection = -1;

    void PaintZ_SetPaintState(string paintCode, int selectionIndex)
    {
        m_PaintZPaintCode = paintCode;
        m_PaintZPaintSelection = selectionIndex;

        PaintZ_ApplyPaintVisual();
        SetSynchDirty();

        if (GetGame().IsServer() && GetGame().IsMultiplayer())
            GetGame().RPCSingleParam(this, PAINTZ_RPC_PAINT_STATE, new Param2<string, int>(m_PaintZPaintCode, m_PaintZPaintSelection), true, null);
    }

    protected void PaintZ_ApplyPaintVisual()
    {
        PaintZ_PaintVisuals.Apply(this, m_PaintZPaintCode, m_PaintZPaintSelection);
    }

    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);

        if (rpc_type != PAINTZ_RPC_PAINT_STATE || !GetGame().IsClient())
            return;

        Param2<string, int> state;
        if (!ctx.Read(state))
            return;

        m_PaintZPaintCode = state.param1;
        m_PaintZPaintSelection = state.param2;
        PaintZ_ApplyPaintVisual();
    }
};
