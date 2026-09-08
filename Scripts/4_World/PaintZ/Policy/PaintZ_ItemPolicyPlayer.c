modded class PlayerBase
{
    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);

        if (rpc_type != PaintZ_ItemPolicy.RPC_POLICY_SYNC || !GetGame().IsClient())
            return;

        PaintZ_ItemPolicy.ReceiveFromServer(ctx);
    }
};
