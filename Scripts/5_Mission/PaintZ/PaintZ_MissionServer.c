modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        PaintZ_RuntimeDocs.Refresh();
        PaintZ_PaintPackRegistry.EnsureInitialized();
        PaintZ_PatternScaling.StartServer();
        PaintZ_ActionTuning.StartServer();
        PaintZ_ItemPolicy.StartServer();
    }

    override void OnMissionFinish()
    {
        PaintZ_ItemPolicy.StopServer();
        PaintZ_ActionTuning.StopServer();
        PaintZ_PatternScaling.StopServer();
        super.OnMissionFinish();
    }

    override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
    {
        super.InvokeOnConnect(player, identity);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(PaintZ_ItemPolicy.SendToClient, 1000, false, player, identity);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(PaintZ_ActionTuning.SendToClient, 1000, false, player, identity);
    }
};
