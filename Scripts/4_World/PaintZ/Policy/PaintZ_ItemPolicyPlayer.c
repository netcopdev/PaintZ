modded class PlayerBase
{
    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);

        if (!GetGame().IsClient())
            return;

        if (rpc_type == PaintZ_ItemPolicy.RPC_POLICY_SYNC)
        {
            PaintZ_ItemPolicy.ReceiveFromServer(ctx);
            return;
        }

        if (rpc_type == PaintZ_ActionTuning.RPC_ACTION_TUNING_SYNC)
            PaintZ_ActionTuning.ReceiveFromServer(ctx);
    }
};
