modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        PaintZ_RuntimeDocs.Refresh();
        PaintZ_PatternScaling.StartServer();
        PaintZ_ItemPolicy.StartServer();
    }

    override void OnMissionFinish()
    {
        PaintZ_ItemPolicy.StopServer();
        PaintZ_PatternScaling.StopServer();
        super.OnMissionFinish();
    }

    override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
    {
        super.InvokeOnConnect(player, identity);

        // The player network object is not guaranteed to accept a targeted RPC
        // from inside InvokeOnConnect itself. Defer one tick of connection setup;
        // periodic reload broadcasts remain a later recovery path.
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(PaintZ_ItemPolicy.SendToClient, 1000, false, player, identity);
    }
};
