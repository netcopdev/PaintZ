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
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(PaintZ_PaintPackApiFixtureManual.SpawnForPlayer, 2500, false, player);
    }
};

class PaintZ_PaintPackApiFixtureManual
{
    static void SpawnForPlayer(PlayerBase player)
    {
        if (!player)
            return;

        vector basePosition = player.GetPosition();
        vector redPosition = basePosition + "1.5 0 1.5";
        vector patternPosition = basePosition + "2.5 0 1.5";
        vector weaponPosition = basePosition + "3.5 0 1.5";

        ItemBase redCan = ItemBase.Cast(GetGame().CreateObjectEx("PaintZ_TestSprayCan_RED", redPosition, ECE_PLACE_ON_SURFACE));
        ItemBase patternCan = ItemBase.Cast(GetGame().CreateObjectEx("PaintZ_TestSprayCan_PAT", patternPosition, ECE_PLACE_ON_SURFACE));
        EntityAI weapon = EntityAI.Cast(GetGame().CreateObjectEx("M4A1", weaponPosition, ECE_PLACE_ON_SURFACE));

        if (redCan && redCan.HasQuantity())
            redCan.SetQuantity(redCan.GetQuantityMax());
        if (patternCan && patternCan.HasQuantity())
            patternCan.SetQuantity(patternCan.GetQuantityMax());

        Print("[PaintZ][PackAPI Manual] staged red_can=" + (redCan != null) + " pattern_can=" + (patternCan != null) + " m4=" + (weapon != null));
    }
};
