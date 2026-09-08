modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        PaintZ_ItemPolicy.StartServer();
    }

    override void OnMissionFinish()
    {
        PaintZ_ItemPolicy.StopServer();
        super.OnMissionFinish();
    }
};
