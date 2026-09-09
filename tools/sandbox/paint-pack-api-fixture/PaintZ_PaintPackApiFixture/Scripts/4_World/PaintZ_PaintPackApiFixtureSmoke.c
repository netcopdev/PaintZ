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

        Check(PaintZ_PaintPackRegistry.IsNamespaceValid("TST"), "valid TST namespace registered");
        Check(PaintZ_PaintPackRegistry.HasFinish("TST-S-RED"), "valid solid finish registered");
        Check(PaintZ_PaintPackRegistry.HasFinish("tst-s-red"), "finish lookup is case-normalized");
        Check(PaintZ_PaintPackRegistry.GetFinishName("TST-S-RED") == "Fixture Red", "finish display name resolved");
        Check(PaintZ_PaintPackRegistry.GetSurfaceTexture("TST-S-RED", 100) == "#(argb,8,8,3)color(1,0,0,1.0,CO)", "explicit solid surface resolved");

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

        int hash = "TST-S-RED".Hash();
        Check(PaintZ_PaintPackRegistry.GetFinishIdByNetworkHash(hash) == "TST-S-RED", "network hash resolves registered finish");

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

        Check(paintCan.GetPaintZPaintCode() == "TST-S-RED", "thin can class exposes paintzFinish");
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
        Check(paintAction.ActionCondition(player, actionTarget, paintCan), "generic paint action accepts external-pack can");

        float paintBefore = paintCan.GetQuantity();
        float expectedPaintUsage = PaintZ_ActionTuning.ResolvePaintUsage(target, paintCan);
        ActionData paintData = new ActionData();
        paintData.m_Player = player;
        paintData.m_Target = actionTarget;
        paintData.m_MainItem = paintCan;
        paintAction.OnFinishProgressServer(paintData);

        ItemBase targetItem = ItemBase.Cast(target);
        Check(targetItem && targetItem.PaintZ_GetPaintCode() == "TST-S-RED", "generic action stores external finish ID");
        Check(PaintZ_PaintVisuals.HasPaint(target, inspection.m_SelectionIndex), "generic action applies registered external surface");
        Check(Math.AbsFloat(paintCan.GetQuantity() - (paintBefore - expectedPaintUsage)) < 0.01, "generic action consumes configured paint amount");

        ActionPaintZStripPaint stripAction = new ActionPaintZStripPaint();
        Check(stripAction.ActionCondition(player, actionTarget, stripper), "strip action sees externally painted item");
        float stripBefore = stripper.GetQuantity();
        float expectedStripUsage = PaintZ_ActionTuning.ResolveStripUsage(target, stripper);
        ActionData stripData = new ActionData();
        stripData.m_Player = player;
        stripData.m_Target = actionTarget;
        stripData.m_MainItem = stripper;
        stripAction.OnFinishProgressServer(stripData);

        Check(targetItem.PaintZ_GetPaintCode() == PaintZ_PaintConstants.PAINT_NONE, "strip clears logical external finish ID");
        Check(targetItem.PaintZ_GetPaintSelection() < 0, "strip clears synchronized painted selection");
        Check(!PaintZ_PaintVisuals.HasPaint(target, inspection.m_SelectionIndex), "strip restores original visual");
        Check(Math.AbsFloat(stripper.GetQuantity() - (stripBefore - expectedStripUsage)) < 0.01, "strip consumes configured stripper amount");

        string unresolvedFinish = "ZZZ-C-OLD";
        targetItem.PaintZ_LoadPaintState(unresolvedFinish);
        targetItem.PaintZ_RestoreLoadedPaint();
        int unresolvedSelection = targetItem.PaintZ_GetPaintSelection();
        Check(targetItem.PaintZ_GetPaintCode() == unresolvedFinish, "unresolved historical finish ID remains logical state");
        Check(unresolvedSelection >= 0, "unresolved historical state retains a strip target selection");
        Check(!PaintZ_PaintVisuals.HasPaint(target, unresolvedSelection), "unresolved historical finish displays original visual");
        Check(PaintZ_PaintedState.HasPaintState(target), "unresolved historical state is still recognized as PaintZ state");
        Check(stripAction.ActionCondition(player, actionTarget, stripper), "unresolved historical state remains strippable");

        stripData.m_MainItem = stripper;
        stripAction.OnFinishProgressServer(stripData);
        Check(targetItem.PaintZ_GetPaintCode() == PaintZ_PaintConstants.PAINT_NONE, "stripping unresolved state clears logical finish ID");
        Check(targetItem.PaintZ_GetPaintSelection() < 0, "stripping unresolved state clears selection");

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
