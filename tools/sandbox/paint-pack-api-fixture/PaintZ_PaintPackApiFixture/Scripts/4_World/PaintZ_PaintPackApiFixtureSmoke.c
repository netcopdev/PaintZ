class PaintZ_PaintPackApiFixtureSmoke
{
    protected static int s_Passed;
    protected static int s_Failed;

    static void Check(bool passed, string label)
    {
        if (passed)
        {
            s_Passed++;
            Print("[PaintZ][PackAPI Smoke] PASS " + label);
        }
        else
        {
            s_Failed++;
            Print("[PaintZ][PackAPI Smoke] FAIL " + label);
        }
    }

    static void RunRegistryChecks()
    {
        s_Passed = 0;
        s_Failed = 0;

        PaintZ_PaintPackRegistry.EnsureInitialized();

        Check(PaintZ_PaintPackRegistry.IsNamespaceValid("PZ"), "core-owned PZ namespace registered without a content-pack owner");
        Check(PaintZ_PaintPackRegistry.HasFinish("PZ-S-API"), "independent official contributor finish registered against core PZ owner");
        Check(PaintZ_PaintPackRegistry.GetFinishName("PZ-S-API") == "Official API Fixture", "official contributor finish resolves through core-owned PZ namespace");

        Check(PaintZ_PaintPackRegistry.IsNamespaceValid("TST"), "valid TST namespace registered");
        Check(PaintZ_PaintPackRegistry.HasFinish("TST-B-RED"), "valid Basic finish registered");
        Check(PaintZ_PaintPackRegistry.HasFinish("tst-b-red"), "finish lookup is case-normalized");
        Check(PaintZ_PaintPackRegistry.GetFinishName("TST-B-RED") == "Fixture Basic Red", "Basic finish display name resolved");
        Check(PaintZ_PaintPackRegistry.GetSurfaceTexture("TST-B-RED", 100) == "#(argb,8,8,3)color(1,0,0,1.0,CO)", "procedural Basic surface resolved");
        Check(!PaintZ_PaintPackRegistry.HasFinish("TST-B-BAD"), "Basic finish with non-procedural surface rejected");

        Check(PaintZ_PaintPackRegistry.HasFinish("TST-C-PAT"), "valid pattern finish registered");
        Check(PaintZ_PaintPackRegistry.IsPatternFinish("TST-C-PAT"), "pattern flag registered");
        Check(PaintZ_PaintPackRegistry.SupportsPatternScale("TST-C-PAT", 50), "declared 50 percent pattern scale available");
        Check(PaintZ_PaintPackRegistry.SupportsPatternScale("TST-C-PAT", 100), "declared 100 percent pattern scale available");
        Check(!PaintZ_PaintPackRegistry.SupportsPatternScale("TST-C-PAT", 75), "undeclared pattern scale rejected");

        Check(!PaintZ_PaintPackRegistry.HasFinish("TST-S-BAD"), "owner mismatch finish rejected");
        Check(!PaintZ_PaintPackRegistry.HasFinish("TST-S-DUP"), "duplicate complete finish ID disabled");

        Check(!PaintZ_PaintPackRegistry.IsNamespaceValid("DUP"), "duplicate namespace owners disable whole namespace");
        Check(!PaintZ_PaintPackRegistry.HasFinish("DUP-S-RED"), "finish in conflicted namespace disabled");

        Check(!PaintZ_PaintPackRegistry.IsNamespaceValid("PZA"), "third-party reserved PZ prefix rejected");
        Check(!PaintZ_PaintPackRegistry.HasFinish("PZA-S-RED"), "finish in rejected reserved namespace disabled");

        Check(!PaintZ_PaintPackRegistry.IsNamespaceValid("AP2"), "unsupported API namespace rejected");
        Check(!PaintZ_PaintPackRegistry.HasFinish("AP2-S-RED"), "finish under unsupported API namespace disabled");

        int hash = "TST-B-RED".Hash();
        Check(PaintZ_PaintPackRegistry.GetFinishIdByNetworkHash(hash) == "TST-B-RED", "network hash resolves registered Basic finish");
        Check(PaintZ_PaintStateRuntime.GetNetworkHash("ZZZ-C-OLD") == 0, "unresolved finish is never exposed as a network hash");

        Print("[PaintZ][PackAPI Smoke] REGISTRY COMPLETE passed=" + s_Passed + " failed=" + s_Failed);
    }

    static void RunPlayerChecks(PlayerBase player)
    {
        if (!player)
            return;

        EntityAI target = EntityAI.Cast(GetGame().CreateObjectEx("M4A1", player.GetPosition(), ECE_PLACE_ON_SURFACE));
        PaintZ_SprayCanBase paintCan = PaintZ_SprayCanBase.Cast(GetGame().CreateObjectEx("PaintZ_TestSprayCan_RED", player.GetPosition(), ECE_PLACE_ON_SURFACE));
        PaintZ_PaintStripperCan stripper = PaintZ_PaintStripperCan.Cast(GetGame().CreateObjectEx("PaintZ_PaintStripperCan", player.GetPosition(), ECE_PLACE_ON_SURFACE));

        Check(target && paintCan && stripper, "player fixtures spawned");
        if (!target || !paintCan || !stripper)
        {
            Cleanup(target, paintCan, stripper);
            return;
        }

        Check(paintCan.GetPaintZPaintCode() == "TST-B-RED", "thin can class exposes Basic paintzFinish");
        paintCan.SetQuantity(paintCan.GetQuantityMax());
        stripper.SetQuantity(stripper.GetQuantityMax());

        PaintZ_PaintInspectionResult inspection = PaintZ_PaintInspector.Inspect(target);
        Check(inspection && inspection.m_Paintable && inspection.m_SelectionIndex >= 0, "M4 fixture has paintable selection");
        if (!inspection || !inspection.m_Paintable || inspection.m_SelectionIndex < 0)
        {
            Cleanup(target, paintCan, stripper);
            return;
        }

        ActionTarget actionTarget = new ActionTarget(target, null, -1, target.GetPosition(), 0);
        ActionPaintZPaint paintAction = new ActionPaintZPaint();
        Check(paintAction.ActionCondition(player, actionTarget, paintCan), "generic paint action accepts external-pack Basic can");

        float paintBefore = paintCan.GetQuantity();
        float expectedPaintUsage = PaintZ_ActionTuning.ResolvePaintUsage(target, paintCan);
        ActionData paintData = new ActionData();
        paintData.m_Player = player;
        paintData.m_Target = actionTarget;
        paintData.m_MainItem = paintCan;
        paintAction.OnFinishProgressServer(paintData);

        ItemBase targetItem = ItemBase.Cast(target);
        float expectedPaintQuantity = paintBefore - expectedPaintUsage;
        float paintQuantityDelta = Math.AbsFloat(paintCan.GetQuantity() - expectedPaintQuantity);
        Check(targetItem && targetItem.PaintZ_GetPaintCode() == "TST-B-RED", "generic action stores external Basic finish ID");
        Check(targetItem && targetItem.PaintZ_HasState(), "generic action sets synchronized PaintZ state marker");
        Check(PaintZ_PaintVisuals.HasPaint(target, inspection.m_SelectionIndex), "generic action applies registered procedural Basic surface");
        Check(paintQuantityDelta < 0.01, "generic action consumes configured paint amount");

        ActionPaintZStripPaint stripAction = new ActionPaintZStripPaint();
        Check(stripAction.ActionCondition(player, actionTarget, stripper), "strip action sees externally painted Basic item");
        float stripBefore = stripper.GetQuantity();
        float expectedStripUsage = PaintZ_ActionTuning.ResolveStripUsage(target, stripper);
        ActionData stripData = new ActionData();
        stripData.m_Player = player;
        stripData.m_Target = actionTarget;
        stripData.m_MainItem = stripper;
        stripAction.OnFinishProgressServer(stripData);

        float expectedStripQuantity = stripBefore - expectedStripUsage;
        float stripQuantityDelta = Math.AbsFloat(stripper.GetQuantity() - expectedStripQuantity);
        Check(targetItem.PaintZ_GetPaintCode() == PaintZ_PaintConstants.PAINT_NONE, "strip clears logical external finish ID");
        Check(!targetItem.PaintZ_HasState(), "strip clears synchronized PaintZ state marker");
        Check(targetItem.PaintZ_GetPaintSelection() < 0, "strip clears synchronized painted selection");
        Check(!PaintZ_PaintVisuals.HasPaint(target, inspection.m_SelectionIndex), "strip restores original visual");
        Check(stripQuantityDelta < 0.01, "strip consumes configured stripper amount");

        stripper.SetQuantity(stripper.GetQuantityMax());
        string unresolvedFinish = "ZZZ-C-OLD";
        targetItem.PaintZ_LoadPaintState(unresolvedFinish);
        targetItem.PaintZ_RestoreLoadedPaint();
        int unresolvedSelection = targetItem.PaintZ_GetPaintSelection();
        Check(targetItem.PaintZ_GetPaintCode() == unresolvedFinish, "unresolved historical finish ID remains logical state");
        Check(targetItem.PaintZ_HasState(), "unresolved historical finish keeps synchronized state marker");
        Check(PaintZ_PaintStateRuntime.GetNetworkHash(unresolvedFinish) == 0, "unresolved historical finish synchronizes no active finish hash");
        Check(unresolvedSelection >= 0, "unresolved historical state retains a strip target selection when target remains inspectable");
        Check(!PaintZ_PaintVisuals.HasPaint(target, unresolvedSelection), "unresolved historical finish displays original visual");
        Check(PaintZ_PaintedState.HasPaintState(target), "unresolved historical state is still recognized as PaintZ state");
        Check(stripAction.ActionCondition(player, actionTarget, stripper), "unresolved historical state remains strippable");

        stripData.m_MainItem = stripper;
        stripAction.OnFinishProgressServer(stripData);
        Check(targetItem.PaintZ_GetPaintCode() == PaintZ_PaintConstants.PAINT_NONE, "stripping unresolved state clears logical finish ID");
        Check(!targetItem.PaintZ_HasState(), "stripping unresolved state clears synchronized state marker");
        Check(targetItem.PaintZ_GetPaintSelection() < 0, "stripping unresolved state clears selection");

        EntityAI unsupported = EntityAI.Cast(GetGame().CreateObjectEx("Ammo_556x45", player.GetPosition(), ECE_PLACE_ON_SURFACE));
        ItemBase unsupportedItem = ItemBase.Cast(unsupported);
        Check(unsupportedItem != null, "unpaintable historical-state fixture spawned");
        if (unsupportedItem)
        {
            unsupportedItem.PaintZ_LoadPaintState(unresolvedFinish);
            unsupportedItem.PaintZ_RestoreLoadedPaint();
            Check(unsupportedItem.PaintZ_HasState(), "unpaintable unresolved item retains synchronized state marker");
            Check(unsupportedItem.PaintZ_GetPaintSelection() < 0, "unpaintable unresolved item needs no paint selection");

            ActionTarget unsupportedTarget = new ActionTarget(unsupported, null, -1, unsupported.GetPosition(), 0);
            stripper.SetQuantity(stripper.GetQuantityMax());
            Check(stripAction.ActionCondition(player, unsupportedTarget, stripper), "unpaintable unresolved item still offers Strip Paint");

            ActionData unsupportedStripData = new ActionData();
            unsupportedStripData.m_Player = player;
            unsupportedStripData.m_Target = unsupportedTarget;
            unsupportedStripData.m_MainItem = stripper;
            stripAction.OnFinishProgressServer(unsupportedStripData);
            Check(!unsupportedItem.PaintZ_HasState(), "stripping unpaintable unresolved item clears state marker");
            Check(unsupportedItem.PaintZ_GetPaintCode() == PaintZ_PaintConstants.PAINT_NONE, "stripping unpaintable unresolved item clears logical state");
        }

        if (unsupported)
            GetGame().ObjectDelete(unsupported);
        Cleanup(target, paintCan, stripper);
        Print("[PaintZ][PackAPI Smoke] PLAYER COMPLETE passed=" + s_Passed + " failed=" + s_Failed);
    }

    protected static void Cleanup(EntityAI target, EntityAI paintCan, EntityAI stripper)
    {
        if (target)
            GetGame().ObjectDelete(target);
        if (paintCan)
            GetGame().ObjectDelete(paintCan);
        if (stripper)
            GetGame().ObjectDelete(stripper);
    }
};
