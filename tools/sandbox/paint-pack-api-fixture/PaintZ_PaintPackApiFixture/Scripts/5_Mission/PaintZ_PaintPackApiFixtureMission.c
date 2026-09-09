modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        PaintZ_PaintPackApiFixtureSmoke.RunRegistryChecks();
    }

    override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
    {
        super.InvokeOnConnect(player, identity);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(PaintZ_PaintPackApiFixtureSmoke.RunPlayerChecks, 1500, false, player);
    }
};
