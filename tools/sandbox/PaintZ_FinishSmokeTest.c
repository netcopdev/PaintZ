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
        CheckWildcards();
        CheckPolicyEvaluation();
        CheckTargetDomains();
        CheckNewPaintEvaluation();
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
        CheckPolicyActionRace(player);
    }

    static void CheckWildcards()
    {
        Check(PaintZ_ItemPolicy.GlobMatches("TTC_AK*", "TTC_AK74"), "glob star matches TTC_AK74");
        Check(PaintZ_ItemPolicy.GlobMatches("TTC_AK*", "TTC_AKM_Black"), "glob star matches TTC_AKM_Black");
        Check(!PaintZ_ItemPolicy.GlobMatches("TTC_AK*", "ABC_AK74"), "glob star rejects different prefix");
        Check(PaintZ_ItemPolicy.GlobMatches("*_AKM", "TTC_AKM"), "glob suffix matches");
        Check(PaintZ_ItemPolicy.GlobMatches("Morty_?K*", "Morty_AK74"), "glob question mark matches one character");
        Check(!PaintZ_ItemPolicy.GlobMatches("Morty_?K*", "Morty_AAK74"), "glob question mark rejects two characters");
        Check(PaintZ_ItemPolicy.GlobMatches("m4a1", "M4A1"), "plain classname is case-insensitive exact match");
        Check(!PaintZ_ItemPolicy.GlobMatches("m4a1", "M4A1_Black"), "plain classname is not substring match");
    }

    static void CheckPolicyEvaluation()
    {
        PaintZ_ItemPolicyConfig saved = PaintZ_ItemPolicy.GetActiveConfigForTests();
        EntityAI weapon = EntityAI.Cast(GetGame().CreateObjectEx("M4A1", "4580 0 10200", ECE_PLACE_ON_SURFACE));
        EntityAI magazine = EntityAI.Cast(GetGame().CreateObjectEx("Mag_CMAG_30Rnd_Black", "4580 0 10200", ECE_PLACE_ON_SURFACE));
        Check(weapon && magazine, "policy fixtures spawned");
        if (!weapon || !magazine)
            return;

        PaintZ_ItemPolicyConfig ordered = MakePolicy("allow");
        ordered.rules.Insert(MakePatternRule("exclude", "weapon", "M4*"));
        ordered.rules.Insert(MakePatternRule("include", "weapon", "M4A1"));
        Check(PaintZ_ItemPolicy.InstallConfigForTests(ordered), "ordered policy validates");
        Check(PaintZ_ItemPolicy.IsPaintApplicationAllowed(weapon), "later narrow include wins");
        Check(PaintZ_ItemPolicy.IsPaintApplicationAllowed(magazine), "weapon rule does not affect magazine");

        PaintZ_ItemPolicyConfig invalid = MakePolicy("allow");
        invalid.rules.Insert(MakePatternRule("ban", "weapon", "*"));
        Check(!PaintZ_ItemPolicy.InstallConfigForTests(invalid), "invalid rule rejects candidate policy");
        Check(PaintZ_ItemPolicy.IsPaintApplicationAllowed(weapon), "invalid candidate retains previous policy");

        PaintZ_ItemPolicyConfig unsafeReload = MakePolicy("allow");
        unsafeReload.reload_seconds = 0;
        Check(PaintZ_ItemPolicy.InstallConfigForTests(unsafeReload), "zero reload interval is safely normalized");
        Check(PaintZ_ItemPolicy.GetReloadSeconds() == -1, "zero reload interval becomes startup-only");
        PaintZ_ItemPolicy.InstallConfigForTests(ordered);

        PaintZ_ItemPolicyConfig excluded = MakePolicy("allow");
        excluded.rules.Insert(MakePatternRule("exclude", "weapon", "M4*"));
        Check(PaintZ_ItemPolicy.InstallConfigForTests(excluded), "exclude policy validates");
        Check(!PaintZ_ItemPolicy.IsPaintApplicationAllowed(weapon), "matching broad rule excludes weapon");
        Check(PaintZ_ItemPolicy.IsPaintApplicationAllowed(magazine), "type filter preserves magazine");

        PaintZ_PaintInspectionResult excludedInspection = PaintZ_PaintInspector.Inspect(weapon);
        if (excludedInspection.m_Paintable)
        {
            ItemBase excludedCan = ItemBase.Cast(GetGame().CreateObjectEx("PaintZ_SprayCan_ODG", "4580 0 10200", ECE_PLACE_ON_SURFACE));
            ActionTarget excludedTarget = new ActionTarget(weapon, null, -1, weapon.GetPosition(), 0);
            ActionPaintZPaintBase excludedAction = new ActionPaintZPaint_S_ODG;
            Check(excludedCan && excludedAction && !excludedAction.ActionCondition(null, excludedTarget, excludedCan), "excluded target does not offer Paint action");
            GetGame().ObjectDelete(excludedCan);
        }

        PaintZ_ItemPolicyConfig magazineExcluded = MakePolicy("allow");
        magazineExcluded.rules.Insert(MakePatternRule("exclude", "magazine", "Mag_CMAG_30Rnd_Black"));
        Check(PaintZ_ItemPolicy.InstallConfigForTests(magazineExcluded), "exact magazine policy validates");
        Check(PaintZ_ItemPolicy.IsPaintApplicationAllowed(weapon), "magazine rule does not affect weapon");
        Check(!PaintZ_ItemPolicy.IsPaintApplicationAllowed(magazine), "exact magazine classname excludes");

        PaintZ_ItemPolicyConfig inheritance = MakePolicy("allow");
        inheritance.rules.Insert(MakeInheritanceRule("exclude", "weapon", "Rifle_Base"));
        Check(PaintZ_ItemPolicy.InstallConfigForTests(inheritance), "inheritance policy validates");
        Check(!PaintZ_ItemPolicy.IsPaintApplicationAllowed(weapon), "runtime inheritance selector excludes descendant");

        PaintZ_ItemPolicyConfig allowed = MakePolicy("allow");
        PaintZ_ItemPolicy.InstallConfigForTests(allowed);
        PaintZ_PaintInspectionResult inspection = PaintZ_PaintInspector.Inspect(weapon);
        array<string> paintCodes;
        PaintZ_PaintCatalog.GetPaintCodes(paintCodes);
        if (inspection.m_Paintable && paintCodes.Count() > 0)
        {
            Check(PaintZ_PaintTarget.SetPaint(weapon, paintCodes[0], inspection.m_SelectionIndex), "paint applied before exclusion");
            PaintZ_ItemPolicy.InstallConfigForTests(excluded);
            Check(PaintZ_PaintVisuals.HasPaint(weapon, inspection.m_SelectionIndex), "exclusion keeps existing paint");
            Check(PaintZ_PaintTarget.SetPaint(weapon, PaintZ_PaintConstants.PAINT_NONE, inspection.m_SelectionIndex), "stripping bypasses exclusion policy");
            Check(!PaintZ_PaintVisuals.HasPaint(weapon, inspection.m_SelectionIndex), "excluded painted item strips normally");
        }

        PaintZ_ItemPolicy.InstallConfigForTests(saved);
        GetGame().ObjectDelete(weapon);
        GetGame().ObjectDelete(magazine);
    }

    static void CheckTargetDomains()
    {
        PaintZ_ItemPolicyConfig saved = PaintZ_ItemPolicy.GetActiveConfigForTests();
        EntityAI weapon = EntityAI.Cast(GetGame().CreateObjectEx("M4A1", "4580 0 10200", ECE_PLACE_ON_SURFACE));
        EntityAI magazine = EntityAI.Cast(GetGame().CreateObjectEx("Mag_CMAG_30Rnd_Black", "4580 0 10200", ECE_PLACE_ON_SURFACE));
        EntityAI unrelated = EntityAI.Cast(GetGame().CreateObjectEx("Apple", "4580 0 10200", ECE_PLACE_ON_SURFACE));
        Check(weapon && magazine && unrelated, "domain fixtures spawned");
        if (!weapon || !magazine || !unrelated)
            return;

        PaintZ_ItemPolicyConfig defaults = MakePolicy("allow");
        Check(PaintZ_ItemPolicy.InstallConfigForTests(defaults), "missing domains use defaults");
        Check(PaintZ_ItemPolicy.IsRelevantTarget(weapon), "default weapon domain matches");
        Check(PaintZ_ItemPolicy.IsRelevantTarget(magazine), "default detachable-magazine domain matches");
        Check(!PaintZ_ItemPolicy.IsRelevantTarget(unrelated), "unrelated object is outside defaults");

        PaintZ_ItemPolicyConfig combined = MakePolicy("allow");
        combined.domains = new array<ref PaintZ_TargetDomainRule>;
        combined.domains.Insert(MakeDomain("Weapon_Base", "M4*"));
        Check(PaintZ_ItemPolicy.InstallConfigForTests(combined), "type plus class domain validates");
        Check(PaintZ_ItemPolicy.IsRelevantTarget(weapon), "type plus class requires and matches both");
        Check(!PaintZ_ItemPolicy.IsRelevantTarget(magazine), "type plus class rejects wrong type");

        combined.domains.Insert(MakeDomain("Magazine_Base", "Mag_CMAG*"));
        Check(PaintZ_ItemPolicy.InstallConfigForTests(combined), "separate domain entries validate");
        Check(PaintZ_ItemPolicy.IsRelevantTarget(weapon) && PaintZ_ItemPolicy.IsRelevantTarget(magazine), "separate domains are OR");
        Check(!PaintZ_ItemPolicy.IsRelevantTarget(unrelated), "domain wildcard rejects unrelated class");

        PaintZ_ItemPolicyConfig invalid = MakePolicy("allow");
        invalid.domains = new array<ref PaintZ_TargetDomainRule>;
        invalid.domains.Insert(MakeDomain("NoSuchPaintZType", ""));
        Check(!PaintZ_ItemPolicy.InstallConfigForTests(invalid), "invalid domain rejects candidate");
        Check(PaintZ_ItemPolicy.IsRelevantTarget(weapon), "invalid domain retains previous config");

        PaintZ_PaintInspectionResult inspection = PaintZ_PaintInspector.Inspect(weapon);
        array<string> paintCodes;
        PaintZ_PaintCatalog.GetPaintCodes(paintCodes);
        if (inspection.m_Paintable && paintCodes.Count() > 0)
        {
            Check(PaintZ_PaintTarget.SetPaint(weapon, paintCodes[0], inspection.m_SelectionIndex), "paint applied while in domain");
            PaintZ_ItemPolicyConfig none = MakePolicy("allow");
            none.domains = new array<ref PaintZ_TargetDomainRule>;
            none.domains.Insert(MakeDomain("", "PaintZ_Disabled_*"));
            Check(PaintZ_ItemPolicy.InstallConfigForTests(none), "nonmatching domain validates");
            Check(!PaintZ_ItemPolicy.IsRelevantTarget(weapon), "removed domain makes new painting silent");
            Check(PaintZ_PaintedState.GetPaintedSelection(weapon) == inspection.m_SelectionIndex, "painted state survives domain removal");
            PaintZ_PaintStripperCan stripper = PaintZ_PaintStripperCan.Cast(GetGame().CreateObjectEx("PaintZ_PaintStripperCan", weapon.GetPosition(), ECE_PLACE_ON_SURFACE));
            ActionTarget paintedTarget = new ActionTarget(weapon, null, -1, weapon.GetPosition(), 0);
            ActionPaintZStripPaint stripAction = new ActionPaintZStripPaint();
            if (stripper)
                stripper.SetQuantity(stripper.GetQuantityMax());
            Check(stripper && stripAction.ActionCondition(null, paintedTarget, stripper), "Strip Paint action survives domain removal");
            Check(PaintZ_PaintTarget.SetPaint(weapon, PaintZ_PaintConstants.PAINT_NONE, inspection.m_SelectionIndex), "strip bypasses removed domain");
            if (stripper)
                GetGame().ObjectDelete(stripper);
        }

        PaintZ_ItemPolicy.InstallConfigForTests(saved);
        GetGame().ObjectDelete(weapon);
        GetGame().ObjectDelete(magazine);
        GetGame().ObjectDelete(unrelated);
    }

    static void CheckNewPaintEvaluation()
    {
        PaintZ_ItemPolicyConfig saved = PaintZ_ItemPolicy.GetActiveConfigForTests();
        PaintZ_ItemPolicy.InstallConfigForTests(MakePolicy("allow"));
        array<string> canTypes;
        PaintZ_PaintCatalog.GetCanTypes(canTypes);
        if (canTypes.Count() == 0)
        {
            Check(false, "new-paint evaluation has a can fixture");
            return;
        }

        EntityAI weapon = EntityAI.Cast(GetGame().CreateObjectEx("M4A1", "4580 0 10200", ECE_PLACE_ON_SURFACE));
        EntityAI unsupported = EntityAI.Cast(GetGame().CreateObjectEx("SCARH", "4580 0 10200", ECE_PLACE_ON_SURFACE));
        EntityAI unrelated = EntityAI.Cast(GetGame().CreateObjectEx("Apple", "4580 0 10200", ECE_PLACE_ON_SURFACE));
        ItemBase can = ItemBase.Cast(GetGame().CreateObjectEx(canTypes[0], "4580 0 10200", ECE_PLACE_ON_SURFACE));
        Check(weapon && unsupported && unrelated && can, "new-paint evaluation fixtures spawned");
        if (!weapon || !unsupported || !unrelated || !can)
            return;

        can.SetQuantity(can.GetQuantityMax());
        ActionTarget weaponTarget = new ActionTarget(weapon, null, -1, weapon.GetPosition(), 0);
        Check(PaintZ_NewPaintEvaluation.Evaluate(weaponTarget, can).m_Result == PaintZ_NewPaintResult.PZ_NEW_PAINT_READY, "allowed supported weapon evaluates Paint");

        ActionTarget unrelatedTarget = new ActionTarget(unrelated, null, -1, unrelated.GetPosition(), 0);
        Check(PaintZ_NewPaintEvaluation.Evaluate(unrelatedTarget, can).m_Result == PaintZ_NewPaintResult.PZ_NEW_PAINT_SILENT, "unrelated object is silent");

        can.SetQuantity(0);
        Check(PaintZ_NewPaintEvaluation.Evaluate(weaponTarget, can).m_Result == PaintZ_NewPaintResult.PZ_NEW_PAINT_CAN_EMPTY, "empty can result");
        can.SetQuantity(can.GetQuantityMax());

        float canHealth = can.GetMaxHealth("", "Health");
        can.SetHealth("", "Health", 0);
        Check(PaintZ_NewPaintEvaluation.Evaluate(weaponTarget, can).m_Result == PaintZ_NewPaintResult.PZ_NEW_PAINT_CAN_RUINED, "ruined can result");
        can.SetHealth("", "Health", canHealth);

        float weaponHealth = weapon.GetMaxHealth("", "Health");
        weapon.SetHealth("", "Health", 0);
        Check(PaintZ_NewPaintEvaluation.Evaluate(weaponTarget, can).m_Result == PaintZ_NewPaintResult.PZ_NEW_PAINT_TARGET_RUINED, "ruined target result");
        weapon.SetHealth("", "Health", weaponHealth);

        PaintZ_ItemPolicyConfig excluded = MakePolicy("allow");
        excluded.rules.Insert(MakePatternRule("exclude", "weapon", "M4A1"));
        PaintZ_ItemPolicy.InstallConfigForTests(excluded);
        Check(PaintZ_ItemPolicy.IsRelevantTarget(weapon), "policy exclusion retains domain membership");
        Check(PaintZ_NewPaintEvaluation.Evaluate(weaponTarget, can).m_Result == PaintZ_NewPaintResult.PZ_NEW_PAINT_EXCLUDED, "excluded target result");

        PaintZ_ItemPolicy.InstallConfigForTests(MakePolicy("allow"));
        ActionTarget unsupportedTarget = new ActionTarget(unsupported, null, -1, unsupported.GetPosition(), 0);
        Check(PaintZ_NewPaintEvaluation.Evaluate(unsupportedTarget, can).m_Result == PaintZ_NewPaintResult.PZ_NEW_PAINT_UNSUPPORTED, "technically unsupported target result");

        PaintZ_ItemPolicy.InstallConfigForTests(saved);
        GetGame().ObjectDelete(weapon);
        GetGame().ObjectDelete(unsupported);
        GetGame().ObjectDelete(unrelated);
        GetGame().ObjectDelete(can);
    }

    static void CheckPolicyActionRace(PlayerBase player)
    {
        if (!player)
            return;

        PaintZ_ItemPolicyConfig saved = PaintZ_ItemPolicy.GetActiveConfigForTests();
        PaintZ_ItemPolicy.InstallConfigForTests(MakePolicy("allow"));
        EntityAI item = EntityAI.Cast(GetGame().CreateObjectEx("M4A1", player.GetPosition(), ECE_PLACE_ON_SURFACE));
        ItemBase can = ItemBase.Cast(GetGame().CreateObjectEx("PaintZ_SprayCan_S_ODG", player.GetPosition(), ECE_PLACE_ON_SURFACE));
        Check(item && can, "policy race fixtures spawned");
        if (!item || !can)
            return;

        can.SetQuantity(can.GetQuantityMax());
        float quantity = can.GetQuantity();
        ActionTarget target = new ActionTarget(item, null, -1, item.GetPosition(), 0);
        ActionPaintZPaint_S_ODG action = new ActionPaintZPaint_S_ODG;
        Check(action.ActionCondition(player, target, can), "paint action starts while policy allows");

        PaintZ_ItemPolicyConfig excluded = MakePolicy("allow");
        excluded.rules.Insert(MakePatternRule("exclude", "weapon", "M4*"));
        PaintZ_ItemPolicy.InstallConfigForTests(excluded);

        ActionData data = new ActionData;
        data.m_Player = player;
        data.m_Target = target;
        data.m_MainItem = can;
        action.OnFinishProgressServer(data);
        int selection = PaintZ_PaintInspector.Inspect(item).m_SelectionIndex;
        Check(!PaintZ_PaintVisuals.HasPaint(item, selection), "new policy blocks completion after action start");
        Check(can.GetQuantity() == quantity, "policy-rejected completion consumes no paint");

        PaintZ_ItemPolicy.InstallConfigForTests(saved);
        GetGame().ObjectDelete(item);
        GetGame().ObjectDelete(can);
    }

    static PaintZ_ItemPolicyConfig MakePolicy(string defaultAction)
    {
        PaintZ_ItemPolicyConfig config = new PaintZ_ItemPolicyConfig();
        config.version = 1;
        config.reload_seconds = -1;
        config.default_action = defaultAction;
        return config;
    }

    static PaintZ_ItemPolicyRule MakePatternRule(string action, string type, string pattern)
    {
        PaintZ_ItemPolicyRule rule = new PaintZ_ItemPolicyRule();
        rule.action = action;
        rule.type = type;
        rule.class_pattern = pattern;
        return rule;
    }

    static PaintZ_ItemPolicyRule MakeInheritanceRule(string action, string type, string inheritedClass)
    {
        PaintZ_ItemPolicyRule rule = new PaintZ_ItemPolicyRule();
        rule.action = action;
        rule.type = type;
        rule.inherits = inheritedClass;
        return rule;
    }

    static PaintZ_TargetDomainRule MakeDomain(string type, string pattern)
    {
        PaintZ_TargetDomainRule domain = new PaintZ_TargetDomainRule();
        domain.type = type;
        domain.class_pattern = pattern;
        return domain;
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

