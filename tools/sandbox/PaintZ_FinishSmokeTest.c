// Opt-in server smoke test. Call PaintZ_FinishSmokeTest.Run() from a test mission.
// Fixture classnames are test inputs only, never compatibility registrations.
class PaintZ_FinishSmokeTest
{
    static void Check(bool passed, string label)
    {
        if (passed)
            Print("[PaintZ][Smoke] PASS " + label);
        else
            Print("[PaintZ][Smoke] FAIL " + label);
    }

    static void Run()
    {
        Check(!PaintZ_PaintVisuals.HasPaint(null, 0), "null target");
        array<string> canTypes;
        array<string> paintCodes;
        PaintZ_PaintCatalog.GetCanTypes(canTypes);
        PaintZ_PaintCatalog.GetPaintCodes(paintCodes);
        Check(canTypes.Count() == paintCodes.Count(), "generated can/code counts agree");
        for (int i = 0; i < canTypes.Count(); i++)
            CheckCan(canTypes[i], paintCodes[i]);
        CheckTarget("M4A1");
        CheckTarget("AKM");
        CheckTarget("Mag_CMAG_30Rnd_Black");
        CheckRejected("SCARH");
        CheckRejected("Mag_STANAG_30Rnd");
        Print("[PaintZ][Smoke] COMPLETE");
    }

    static void CheckCan(string type, string paintCode)
    {
        EntityAI item = EntityAI.Cast(GetGame().CreateObjectEx(type, "4580 0 10200", ECE_PLACE_ON_SURFACE));
        PaintZ_SprayCanBase can = PaintZ_SprayCanBase.Cast(item);
        Check(can && can.GetPaintZPaintCode() == paintCode, type + " paint code");
        if (item)
            GetGame().ObjectDelete(item);
    }

    static void CheckLoadout(PlayerBase player)
    {
        array<EntityAI> items = new array<EntityAI>;
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        array<string> catalogTypes;
        PaintZ_PaintCatalog.GetCanTypes(catalogTypes);
        array<string> types = new array<string>;
        if (catalogTypes.Count() > 0)
            types.Insert(catalogTypes[0]);
        types.Insert("PaintZ_PaintStripperCan");
        Check(player.FindAttachmentBySlotName("Back") != null, "backpack equipped");
        foreach (string type : types)
        {
            int count = 0;
            foreach (EntityAI item : items)
            {
                if (item.GetType() == type)
                {
                    count++;
                    ItemBase can = ItemBase.Cast(item);
                    Check(can.GetQuantity() == can.GetQuantityMax(), type + " full");
                }
            }
            Check(count == 2, type + " two in player inventory");
        }
    }

    static void CheckStripCompletion(PlayerBase player)
    {
        EntityAI item = EntityAI.Cast(GetGame().CreateObjectEx("M4A1", player.GetPosition(), ECE_PLACE_ON_SURFACE));
        PaintZ_PaintStripperCan stripper = PaintZ_PaintStripperCan.Cast(GetGame().CreateObjectEx("PaintZ_PaintStripperCan", player.GetPosition(), ECE_PLACE_ON_SURFACE));
        array<string> canTypes;
        array<string> paintCodes;
        PaintZ_PaintCatalog.GetCanTypes(canTypes);
        PaintZ_PaintCatalog.GetPaintCodes(paintCodes);
        if (canTypes.Count() == 0 || paintCodes.Count() == 0)
        {
            Check(false, "generated paint catalogue is not empty");
            return;
        }
        ItemBase paintCan = ItemBase.Cast(GetGame().CreateObjectEx(canTypes[0], player.GetPosition(), ECE_PLACE_ON_SURFACE));
        Check(item && stripper && paintCan, "completion fixtures spawned");
        if (!item || !stripper || !paintCan)
            return;
        stripper.SetQuantity(stripper.GetQuantityMax());
        float quantity = stripper.GetQuantity();
        int selection = PaintZ_PaintInspector.Inspect(item).m_SelectionIndex;
        PaintZ_PaintTarget.SetPaint(item, paintCodes[0], selection);
        ActionData data = new ActionData;
        data.m_Player = player;
        data.m_Target = new ActionTarget(item, null, -1, item.GetPosition(), 0);
        data.m_MainItem = paintCan;
        ActionPaintZStripPaint action = new ActionPaintZStripPaint;
        action.OnFinishProgressServer(data);
        Check(PaintZ_PaintVisuals.HasPaint(item, selection), "server rejects stripping with paint can");
        data.m_MainItem = stripper;
        action.OnFinishProgressServer(data);
        Check(!PaintZ_PaintVisuals.HasPaint(item, selection), "stripper completion removes paint");
        Check(stripper.GetQuantity() == quantity - PaintZ_PaintConstants.STRIP_COST, "stripper completion consumes quantity");
        action.OnFinishProgressServer(data);
        Check(stripper.GetQuantity() == quantity - PaintZ_PaintConstants.STRIP_COST, "stale strip consumes nothing");
        GetGame().ObjectDelete(item);
        GetGame().ObjectDelete(stripper);
        GetGame().ObjectDelete(paintCan);
    }

    static void CheckRejected(string type)
    {
        EntityAI item = EntityAI.Cast(GetGame().CreateObjectEx(type, "4580 0 10200", ECE_PLACE_ON_SURFACE));
        Check(item != null, type + " rejection fixture spawned");
        if (!item)
            return;
        PaintZ_PaintInspectionResult inspection = PaintZ_PaintInspector.Inspect(item);
        Check(!inspection.m_Paintable && inspection.m_Reason != "", type + " safely rejected with reason");
        CheckStripAction(item, false);
        GetGame().ObjectDelete(item);
    }

    static void CheckStripAction(EntityAI item, bool expected)
    {
        array<string> canTypes;
        PaintZ_PaintCatalog.GetCanTypes(canTypes);
        canTypes.Insert("PaintZ_PaintStripperCan");
        ActionPaintZStripPaint action = new ActionPaintZStripPaint;
        ActionTarget target = new ActionTarget(item, null, -1, item.GetPosition(), 0);
        foreach (string canType : canTypes)
        {
            ItemBase can = ItemBase.Cast(GetGame().CreateObjectEx(canType, item.GetPosition(), ECE_PLACE_ON_SURFACE));
            if (can && can.HasQuantity())
                can.SetQuantity(can.GetQuantityMax());
            bool canStrip = expected && PaintZ_PaintStripperCan.Cast(can) != null;
            Check(can && action.ActionCondition(null, target, can) == canStrip, item.GetType() + " strip action with " + canType + " expected=" + canStrip);
            if (PaintZ_PaintStripperCan.Cast(can))
            {
                Check(!PaintZ_SprayCanBase.Cast(can), "stripper is not a paint can");
                can.SetQuantity(PaintZ_PaintConstants.STRIP_COST - 1);
                Check(!action.ActionCondition(null, target, can), "insufficient stripper rejected");
            }
            if (can)
                GetGame().ObjectDelete(can);
        }
    }

    static void CheckTarget(string type)
    {
        EntityAI item = EntityAI.Cast(GetGame().CreateObjectEx(type, "4580 0 10200", ECE_PLACE_ON_SURFACE));
        Check(item != null, type + " spawned");
        if (!item)
            return;

        PaintZ_PaintInspectionResult inspection = PaintZ_PaintInspector.Inspect(item);
        Check(inspection.m_Paintable, type + " inspected");
        if (!inspection.m_Paintable)
        {
            GetGame().ObjectDelete(item);
            return;
        }

        int selection = inspection.m_SelectionIndex;
        float health = item.GetHealth("", "Health");
        string originalTexture = item.GetObjectTexture(selection);
        Magazine magazine = Magazine.Cast(item);
        int ammo;
        if (magazine)
        {
            magazine.ServerSetAmmoCount(7);
            ammo = magazine.GetAmmoCount();
        }

        Check(!PaintZ_PaintVisuals.HasPaint(item, -1), type + " invalid selection");
        Check(!PaintZ_PaintVisuals.HasPaint(item, selection), type + " untouched has no strip");
        CheckStripAction(item, false);
        array<string> paintCodes;
        PaintZ_PaintCatalog.GetPaintCodes(paintCodes);
        foreach (string paintCode : paintCodes)
        {
            string label = type + " " + paintCode;
            Check(PaintZ_PaintTarget.SetPaint(item, paintCode, selection), label + " apply");
            Check(PaintZ_PaintVisuals.HasPaint(item, selection), label + " has strip");
            CheckStripAction(item, true);
            Print("[PaintZ][Smoke] texture=" + item.GetObjectTexture(selection));
            Check(item.GetType() == type && item.GetHealth("", "Health") == health, label + " identity/health");
            if (magazine)
                Check(magazine.GetAmmoCount() == ammo, label + " ammo");
            Check(PaintZ_PaintTarget.SetPaint(item, PaintZ_PaintConstants.PAINT_NONE, selection), label + " strip");
            Check(!PaintZ_PaintVisuals.HasPaint(item, selection), label + " stripped has no strip");
            CheckStripAction(item, false);
            Check(item.GetObjectTexture(selection) == originalTexture, label + " restored texture");
        }
        GetGame().ObjectDelete(item);
    }
};

