class PaintZ_ItemPolicy
{
    static const int RPC_POLICY_SYNC = 782342;
    static const int MAX_SYNCHRONIZED_RULES = 4096;
    static const int MAX_SYNCHRONIZED_DOMAINS = 1024;
    static const int MAX_SYNCHRONIZED_SELECTOR_VALUES = 4096;
    static const string PROFILE_DIRECTORY = "$profile:PaintZ";
    static const string PROFILE_PATH = "$profile:PaintZ/paintz_items.json";
    static const string PROFILE_README_PATH = "$profile:PaintZ/paintz_items_README.txt";
    static const string BUNDLED_DEFAULT_PATH = "PaintZ/config/paintz_items.default.json";
    static const string BUNDLED_README_PATH = "PaintZ/config/paintz_items_README.txt";

    protected static ref PaintZ_ItemPolicyConfig s_ActiveConfig;
    protected static ref map<string, bool> s_DecisionCache = new map<string, bool>;
    protected static ref map<string, bool> s_DomainCache = new map<string, bool>;
    protected static bool s_ServerStarted;

    static void StartServer()
    {
        if (s_ServerStarted)
            return;

        s_ServerStarted = true;
        MakeDirectory(PROFILE_DIRECTORY);

        if (!FileExist(PROFILE_README_PATH))
        {
            if (!CopyFile(BUNDLED_README_PATH, PROFILE_README_PATH))
                Warn("Could not create runtime item-policy README from " + BUNDLED_README_PATH);
            else
                Info("created runtime README path=" + PROFILE_README_PATH);
        }

        if (!FileExist(PROFILE_PATH))
        {
            if (!CopyFile(BUNDLED_DEFAULT_PATH, PROFILE_PATH))
            {
                Error("Could not create runtime item policy from bundled default " + BUNDLED_DEFAULT_PATH);
                return;
            }

            Info("created runtime config from default path=" + PROFILE_PATH);
        }

        if (!ReloadInternal(false))
            Error("startup config load failed; new paint applications will fail closed until a valid policy is loaded");
    }

    static void StopServer()
    {
        if (GetGame())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(ReloadScheduled);

        s_ServerStarted = false;
        s_ActiveConfig = null;
        s_DecisionCache.Clear();
        s_DomainCache.Clear();
    }

    static bool IsRelevantTarget(EntityAI target)
    {
        if (!target || !s_ActiveConfig || !s_ActiveConfig.domains)
            return false;

        string className = target.GetType();
        string normalizedClass = className;
        normalizedClass.ToLower();

        bool cached;
        if (s_DomainCache.Find(normalizedClass, cached))
            return cached;

        bool relevant;
        for (int i = 0; i < s_ActiveConfig.domains.Count(); i++)
        {
            if (DomainMatches(target, className, normalizedClass, s_ActiveConfig.domains.Get(i)))
            {
                relevant = true;
                break;
            }
        }

        s_DomainCache.Insert(normalizedClass, relevant);
        return relevant;
    }

    static bool IsPaintApplicationAllowed(EntityAI target)
    {
        if (!target || !s_ActiveConfig || !IsRelevantTarget(target))
            return false;

        string className = target.GetType();
        string normalizedClass = className;
        normalizedClass.ToLower();

        bool cached;
        if (s_DecisionCache.Find(normalizedClass, cached))
            return cached;

        bool allowed = EvaluateConfig(target, className, normalizedClass, s_ActiveConfig);
        s_DecisionCache.Insert(normalizedClass, allowed);
        return allowed;
    }

    static bool GlobMatches(string pattern, string value)
    {
        pattern.ToLower();
        value.ToLower();
        return GlobMatchesNormalized(pattern, value);
    }

    static int GetReloadSeconds()
    {
        if (!s_ActiveConfig)
            return -1;
        return s_ActiveConfig.reload_seconds;
    }

    static int GetRuleCount()
    {
        if (!s_ActiveConfig || !s_ActiveConfig.rules)
            return 0;
        return s_ActiveConfig.rules.Count();
    }

    static int GetDomainRuleCount()
    {
        if (!s_ActiveConfig || !s_ActiveConfig.domains)
            return 0;
        return s_ActiveConfig.domains.Count();
    }

    protected static void ReloadScheduled()
    {
        ReloadInternal(true);
    }

    protected static bool ReloadInternal(bool periodic)
    {
        PaintZ_ItemPolicyConfig candidate = new PaintZ_ItemPolicyConfig();
        string loadError;
        if (!JsonFileLoader<PaintZ_ItemPolicyConfig>.LoadFile(PROFILE_PATH, candidate, loadError))
        {
            Reject("invalid JSON or incompatible value: " + loadError);
            if (periodic && s_ActiveConfig)
                Error("reload rejected; continuing with previous valid policy");
            ScheduleNextReload();
            return false;
        }

        string validationError;
        if (!ValidateAndNormalize(candidate, validationError))
        {
            Reject(validationError);
            if (periodic && s_ActiveConfig)
                Error("reload rejected; continuing with previous valid policy");
            ScheduleNextReload();
            return false;
        }

        s_ActiveConfig = candidate;
        s_DecisionCache.Clear();
        s_DomainCache.Clear();

        BroadcastToClients();

        if (periodic)
            Info("config successfully reloaded rules=" + GetRuleCount() + " domains=" + GetDomainRuleCount());
        else
            Info("config loaded rules=" + GetRuleCount() + " domains=" + GetDomainRuleCount());

        if (candidate.reload_seconds == -1)
            Info("reload mode=startup-only reload_seconds=-1");
        else
            Info("reload mode=periodic interval_seconds=" + candidate.reload_seconds);

        ScheduleNextReload();
        return true;
    }

    static void SendToClient(PlayerBase player, PlayerIdentity identity)
    {
        if (!player || !identity || !s_ActiveConfig)
            return;

        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(s_ActiveConfig.version);
        rpc.Write(s_ActiveConfig.reload_seconds);
        rpc.Write(s_ActiveConfig.default_action);
        rpc.Write(s_ActiveConfig.rules.Count());

        for (int i = 0; i < s_ActiveConfig.rules.Count(); i++)
        {
            PaintZ_ItemPolicyRule rule = s_ActiveConfig.rules.Get(i);
            rpc.Write(rule.action);
            rpc.Write(rule.type);
            rpc.Write(rule.class_pattern);
            WriteStringArray(rpc, rule.class_patterns);
            rpc.Write(rule.inherits);
            WriteStringArray(rpc, rule.inherits_any);
            rpc.Write(rule.inventory_slot);
            WriteStringArray(rpc, rule.inventory_slots);
            rpc.Write(rule.inventory_slot_pattern);
            WriteStringArray(rpc, rule.inventory_slot_patterns);
        }

        rpc.Write(s_ActiveConfig.domains.Count());
        for (int domainIndex = 0; domainIndex < s_ActiveConfig.domains.Count(); domainIndex++)
        {
            PaintZ_TargetDomainRule domain = s_ActiveConfig.domains.Get(domainIndex);
            rpc.Write(domain.type);
            rpc.Write(domain.class_pattern);
            rpc.Write(domain.inventory_slot);
            rpc.Write(domain.inventory_slot_pattern);
        }

        rpc.Send(player, RPC_POLICY_SYNC, true, identity);
    }

    static bool ReceiveFromServer(ParamsReadContext ctx)
    {
        PaintZ_ItemPolicyConfig candidate = new PaintZ_ItemPolicyConfig();
        int ruleCount;

        if (!ctx.Read(candidate.version) || !ctx.Read(candidate.reload_seconds) || !ctx.Read(candidate.default_action) || !ctx.Read(ruleCount))
        {
            Error("client policy sync was truncated");
            return false;
        }

        if (ruleCount < 0 || ruleCount > MAX_SYNCHRONIZED_RULES)
        {
            Error("client policy sync has invalid rule count=" + ruleCount);
            return false;
        }

        for (int i = 0; i < ruleCount; i++)
        {
            PaintZ_ItemPolicyRule rule = new PaintZ_ItemPolicyRule();
            if (!ctx.Read(rule.action) || !ctx.Read(rule.type) || !ctx.Read(rule.class_pattern) || !ReadStringArray(ctx, rule.class_patterns) || !ctx.Read(rule.inherits) || !ReadStringArray(ctx, rule.inherits_any) || !ctx.Read(rule.inventory_slot) || !ReadStringArray(ctx, rule.inventory_slots) || !ctx.Read(rule.inventory_slot_pattern) || !ReadStringArray(ctx, rule.inventory_slot_patterns))
            {
                Error("client policy sync was truncated at rule=" + i);
                return false;
            }
            candidate.rules.Insert(rule);
        }

        int domainCount;
        if (!ctx.Read(domainCount) || domainCount < 0 || domainCount > MAX_SYNCHRONIZED_DOMAINS)
        {
            Error("client policy sync has invalid domain count");
            return false;
        }

        candidate.domains = new array<ref PaintZ_TargetDomainRule>;
        for (int domainIndex = 0; domainIndex < domainCount; domainIndex++)
        {
            PaintZ_TargetDomainRule domain = new PaintZ_TargetDomainRule();
            if (!ctx.Read(domain.type) || !ctx.Read(domain.class_pattern) || !ctx.Read(domain.inventory_slot) || !ctx.Read(domain.inventory_slot_pattern))
            {
                Error("client policy sync was truncated at domain=" + domainIndex);
                return false;
            }
            candidate.domains.Insert(domain);
        }

        string validationError;
        if (!ValidateAndNormalize(candidate, validationError))
        {
            Error("client policy sync rejected: " + validationError);
            return false;
        }

        s_ActiveConfig = candidate;
        s_DecisionCache.Clear();
        s_DomainCache.Clear();
        Info("client policy synchronized rules=" + ruleCount + " domains=" + domainCount);
        return true;
    }

    protected static void WriteStringArray(ScriptRPC rpc, array<string> values)
    {
        int count;
        if (values)
            count = values.Count();

        rpc.Write(count);
        for (int i = 0; i < count; i++)
            rpc.Write(values.Get(i));
    }

    protected static bool ReadStringArray(ParamsReadContext ctx, array<string> values)
    {
        int count;
        if (!ctx.Read(count) || count < 0 || count > MAX_SYNCHRONIZED_SELECTOR_VALUES)
            return false;

        values.Clear();
        for (int i = 0; i < count; i++)
        {
            string value;
            if (!ctx.Read(value))
                return false;
            values.Insert(value);
        }

        return true;
    }

    protected static bool ValidateAndNormalize(PaintZ_ItemPolicyConfig config, out string error)
    {
        if (!config)
        {
            error = "root object is missing";
            return false;
        }

        if (config.version != 1)
        {
            error = "unsupported version=" + config.version + " (expected 1)";
            return false;
        }

        if (config.reload_seconds == -999999)
        {
            error = "reload_seconds is required";
            return false;
        }

        config.default_action.ToLower();
        if (config.default_action != "allow" && config.default_action != "exclude")
        {
            error = "default_action must be allow or exclude";
            return false;
        }

        if (config.reload_seconds == 0 || config.reload_seconds < -1)
        {
            Warn("reload_seconds=" + config.reload_seconds + " is unsafe; treating it as -1 (startup-only)");
            config.reload_seconds = -1;
        }

        if (!config.rules)
            config.rules = new array<ref PaintZ_ItemPolicyRule>;

        if (!config.domains || config.domains.Count() == 0)
        {
            config.domains = CreateDefaultDomains();
            Info("domains missing or empty; using weapon and detachable-magazine defaults");
        }

        bool valid = true;
        string firstError;
        for (int i = 0; i < config.rules.Count(); i++)
        {
            PaintZ_ItemPolicyRule rule = config.rules.Get(i);
            string ruleError;
            if (!ValidateAndNormalizeRule(rule, ruleError))
            {
                Error("invalid rule index=" + i + " error=" + ruleError);
                if (firstError == "")
                    firstError = "rule " + i + " " + ruleError;
                valid = false;
            }
        }

        if (!valid)
        {
            error = firstError;
            return false;
        }

        for (int domainIndex = 0; domainIndex < config.domains.Count(); domainIndex++)
        {
            PaintZ_TargetDomainRule domain = config.domains.Get(domainIndex);
            string domainError;
            if (!ValidateAndNormalizeDomain(domain, domainError))
            {
                error = "domain " + domainIndex + " " + domainError;
                Error("invalid domain index=" + domainIndex + " error=" + domainError);
                return false;
            }
        }

        return true;
    }

    protected static bool ValidateAndNormalizeRule(PaintZ_ItemPolicyRule rule, out string error)
    {
        if (!rule)
        {
            error = "must be a JSON object.";
            return false;
        }

        rule.action.ToLower();
        if (rule.action != "include" && rule.action != "exclude")
        {
            error = "action must be include or exclude";
            return false;
        }

        if (rule.type == "")
            rule.type = "all";

        string normalizedType = rule.type;
        normalizedType.ToLower();
        if (normalizedType == "all" || normalizedType == "weapon" || normalizedType == "magazine")
            rule.type = normalizedType;
        else if (!DomainTypeExists(rule.type))
        {
            error = "type must be all, legacy weapon/magazine, or a valid DayZ base class: " + rule.type;
            return false;
        }

        if (!rule.class_patterns)
            rule.class_patterns = new array<string>;
        if (!rule.inherits_any)
            rule.inherits_any = new array<string>;
        if (!rule.inventory_slots)
            rule.inventory_slots = new array<string>;
        if (!rule.inventory_slot_patterns)
            rule.inventory_slot_patterns = new array<string>;

        bool hasSelector = rule.class_pattern != "" || rule.class_patterns.Count() > 0 || rule.inherits != "" || rule.inherits_any.Count() > 0 || rule.inventory_slot != "" || rule.inventory_slots.Count() > 0 || rule.inventory_slot_pattern != "" || rule.inventory_slot_patterns.Count() > 0;
        if (!hasSelector)
        {
            error = "at least one selector is required: class_pattern/class_patterns, inherits/inherits_any, inventory_slot/inventory_slots, or inventory_slot_pattern/inventory_slot_patterns";
            return false;
        }

        if (rule.class_pattern != "")
            rule.class_pattern.ToLower();

        string arrayError;
        if (!NormalizeStringArray(rule.class_patterns, "class_patterns", false, arrayError))
        {
            error = arrayError;
            return false;
        }

        if (rule.inherits != "")
        {
            rule.inherits.ToLower();
            if (!InheritanceClassExists(rule.inherits))
            {
                error = "inherits class does not exist: " + rule.inherits;
                return false;
            }
        }

        if (!NormalizeStringArray(rule.inherits_any, "inherits_any", true, arrayError))
        {
            error = arrayError;
            return false;
        }

        if (rule.inventory_slot != "")
            rule.inventory_slot.ToLower();
        if (!NormalizeStringArray(rule.inventory_slots, "inventory_slots", false, arrayError))
        {
            error = arrayError;
            return false;
        }

        if (rule.inventory_slot_pattern != "")
            rule.inventory_slot_pattern.ToLower();
        if (!NormalizeStringArray(rule.inventory_slot_patterns, "inventory_slot_patterns", false, arrayError))
        {
            error = arrayError;
            return false;
        }

        return true;
    }

    protected static bool NormalizeStringArray(array<string> values, string fieldName, bool validateInheritance, out string error)
    {
        if (!values)
            return true;

        if (values.Count() > MAX_SYNCHRONIZED_SELECTOR_VALUES)
        {
            error = fieldName + " exceeds maximum value count=" + MAX_SYNCHRONIZED_SELECTOR_VALUES;
            return false;
        }

        for (int i = 0; i < values.Count(); i++)
        {
            string value = values.Get(i);
            if (value == "")
            {
                error = fieldName + " contains an empty value at index=" + i;
                return false;
            }

            value.ToLower();
            if (validateInheritance && !InheritanceClassExists(value))
            {
                error = fieldName + " class does not exist: " + value;
                return false;
            }

            values.Set(i, value);
        }

        return true;
    }

    protected static bool InheritanceClassExists(string className)
    {
        if (!GetGame())
            return false;

        return GetGame().ConfigIsExisting("CfgWeapons " + className) || GetGame().ConfigIsExisting("CfgMagazines " + className) || GetGame().ConfigIsExisting("CfgVehicles " + className);
    }

    protected static bool ValidateAndNormalizeDomain(PaintZ_TargetDomainRule domain, out string error)
    {
        if (!domain)
        {
            error = "must be a JSON object";
            return false;
        }

        if (domain.type == "" && domain.class_pattern == "" && domain.inventory_slot == "" && domain.inventory_slot_pattern == "")
        {
            error = "requires type, class_pattern, inventory_slot, inventory_slot_pattern, or a combination";
            return false;
        }

        if (domain.type != "" && !DomainTypeExists(domain.type))
        {
            error = "type class does not exist: " + domain.type;
            return false;
        }

        domain.class_pattern.ToLower();
        domain.inventory_slot.ToLower();
        domain.inventory_slot_pattern.ToLower();
        return true;
    }

    protected static bool DomainTypeExists(string typeName)
    {
        string normalized = typeName;
        normalized.ToLower();

        // These are script hierarchy names/aliases rather than guaranteed
        // config classes. Keep them as generic roots available to JSON policy.
        if (normalized == "itembase" || normalized == "inventoryitembase" || normalized == "inventoryitemsuper")
            return true;
        if (normalized == "weapon_base" || normalized == "magazine_base")
            return true;

        if (!GetGame())
            return false;

        return GetGame().ConfigIsExisting("CfgWeapons " + typeName) || GetGame().ConfigIsExisting("CfgMagazines " + typeName) || GetGame().ConfigIsExisting("CfgVehicles " + typeName);
    }

    protected static bool DomainMatches(EntityAI target, string className, string normalizedClass, PaintZ_TargetDomainRule domain)
    {
        if (domain.type != "" && !DomainTypeMatches(target, className, domain.type))
            return false;

        if (domain.class_pattern != "" && !GlobMatchesNormalized(domain.class_pattern, normalizedClass))
            return false;

        if (!InventorySlotMatches(target, domain.inventory_slot, domain.inventory_slot_pattern))
            return false;

        return true;
    }

    protected static bool DomainTypeMatches(EntityAI target, string className, string typeName)
    {
        string normalized = typeName;
        normalized.ToLower();

        if (normalized == "itembase" || normalized == "inventoryitembase" || normalized == "inventoryitemsuper")
        {
            ItemBase item;
            if (Class.CastTo(item, target))
                return true;

            // Current DayZ declares Weapon and Magazine through the
            // InventoryItemSuper/ItemBase alias. Keep explicit casts as a safe
            // bridge for engine-backed classes and third-party descendants.
            Weapon_Base anyWeapon;
            if (Class.CastTo(anyWeapon, target))
                return true;

            Magazine anyMagazine;
            return Class.CastTo(anyMagazine, target);
        }

        Weapon_Base weapon;
        if (normalized == "weapon_base" || normalized == "weapon")
            return Class.CastTo(weapon, target);

        Magazine magazine;
        if (normalized == "magazine_base" || normalized == "magazine")
            return Class.CastTo(magazine, target) && !target.IsAmmoPile();

        return GetGame() && GetGame().IsKindOf(className, typeName);
    }

    protected static ref array<ref PaintZ_TargetDomainRule> CreateDefaultDomains()
    {
        ref array<ref PaintZ_TargetDomainRule> domains = new array<ref PaintZ_TargetDomainRule>;
        PaintZ_TargetDomainRule weapons = new PaintZ_TargetDomainRule();
        weapons.type = "Weapon_Base";
        domains.Insert(weapons);

        PaintZ_TargetDomainRule magazines = new PaintZ_TargetDomainRule();
        magazines.type = "Magazine_Base";
        domains.Insert(magazines);
        return domains;
    }

    protected static bool EvaluateConfig(EntityAI target, string className, string normalizedClass, PaintZ_ItemPolicyConfig config)
    {
        bool allowed = config.default_action == "allow";
        for (int i = 0; i < config.rules.Count(); i++)
        {
            PaintZ_ItemPolicyRule rule = config.rules.Get(i);
            if (RuleMatches(target, className, normalizedClass, rule))
                allowed = rule.action == "include";
        }

        return allowed;
    }

    protected static bool RuleMatches(EntityAI target, string className, string normalizedClass, PaintZ_ItemPolicyRule rule)
    {
        if (!RuleTypeMatches(target, className, rule.type))
            return false;

        if (HasClassPatternGroup(rule) && !ClassPatternGroupMatches(normalizedClass, rule))
            return false;

        if (HasInheritanceGroup(rule) && !InheritanceGroupMatches(className, rule))
            return false;

        if (HasExactSlotGroup(rule) && !ExactSlotGroupMatches(target, rule))
            return false;

        if (HasSlotPatternGroup(rule) && !SlotPatternGroupMatches(target, rule))
            return false;

        return true;
    }

    protected static bool HasClassPatternGroup(PaintZ_ItemPolicyRule rule)
    {
        return rule.class_pattern != "" || (rule.class_patterns && rule.class_patterns.Count() > 0);
    }

    protected static bool HasInheritanceGroup(PaintZ_ItemPolicyRule rule)
    {
        return rule.inherits != "" || (rule.inherits_any && rule.inherits_any.Count() > 0);
    }

    protected static bool HasExactSlotGroup(PaintZ_ItemPolicyRule rule)
    {
        return rule.inventory_slot != "" || (rule.inventory_slots && rule.inventory_slots.Count() > 0);
    }

    protected static bool HasSlotPatternGroup(PaintZ_ItemPolicyRule rule)
    {
        return rule.inventory_slot_pattern != "" || (rule.inventory_slot_patterns && rule.inventory_slot_patterns.Count() > 0);
    }

    protected static bool ClassPatternGroupMatches(string normalizedClass, PaintZ_ItemPolicyRule rule)
    {
        if (rule.class_pattern != "" && GlobMatchesNormalized(rule.class_pattern, normalizedClass))
            return true;

        for (int i = 0; rule.class_patterns && i < rule.class_patterns.Count(); i++)
        {
            if (GlobMatchesNormalized(rule.class_patterns.Get(i), normalizedClass))
                return true;
        }

        return false;
    }

    protected static bool InheritanceGroupMatches(string className, PaintZ_ItemPolicyRule rule)
    {
        if (rule.inherits != "" && GetGame().IsKindOf(className, rule.inherits))
            return true;

        for (int i = 0; rule.inherits_any && i < rule.inherits_any.Count(); i++)
        {
            if (GetGame().IsKindOf(className, rule.inherits_any.Get(i)))
                return true;
        }

        return false;
    }

    protected static bool ExactSlotGroupMatches(EntityAI target, PaintZ_ItemPolicyRule rule)
    {
        TStringArray slots = GetDeclaredInventorySlots(target);
        if (!slots || slots.Count() == 0)
            return false;

        for (int i = 0; i < slots.Count(); i++)
        {
            string normalizedSlot = slots.Get(i);
            normalizedSlot.ToLower();

            if (rule.inventory_slot != "" && normalizedSlot == rule.inventory_slot)
                return true;

            for (int j = 0; rule.inventory_slots && j < rule.inventory_slots.Count(); j++)
            {
                if (normalizedSlot == rule.inventory_slots.Get(j))
                    return true;
            }
        }

        return false;
    }

    protected static bool SlotPatternGroupMatches(EntityAI target, PaintZ_ItemPolicyRule rule)
    {
        TStringArray slots = GetDeclaredInventorySlots(target);
        if (!slots || slots.Count() == 0)
            return false;

        for (int i = 0; i < slots.Count(); i++)
        {
            string normalizedSlot = slots.Get(i);
            normalizedSlot.ToLower();

            if (rule.inventory_slot_pattern != "" && GlobMatchesNormalized(rule.inventory_slot_pattern, normalizedSlot))
                return true;

            for (int j = 0; rule.inventory_slot_patterns && j < rule.inventory_slot_patterns.Count(); j++)
            {
                if (GlobMatchesNormalized(rule.inventory_slot_patterns.Get(j), normalizedSlot))
                    return true;
            }
        }

        return false;
    }

    protected static bool RuleTypeMatches(EntityAI target, string className, string ruleType)
    {
        string normalized = ruleType;
        normalized.ToLower();
        if (normalized == "" || normalized == "all")
            return true;

        return DomainTypeMatches(target, className, ruleType);
    }

    protected static bool InventorySlotMatches(EntityAI target, string inventorySlot, string inventorySlotPattern)
    {
        if (inventorySlot == "" && inventorySlotPattern == "")
            return true;

        TStringArray slots = GetDeclaredInventorySlots(target);
        if (!slots || slots.Count() == 0)
            return false;

        bool exactMatches = inventorySlot == "";
        bool patternMatches = inventorySlotPattern == "";

        for (int i = 0; i < slots.Count(); i++)
        {
            string normalizedSlot = slots.Get(i);
            normalizedSlot.ToLower();

            if (!exactMatches && normalizedSlot == inventorySlot)
                exactMatches = true;
            if (!patternMatches && GlobMatchesNormalized(inventorySlotPattern, normalizedSlot))
                patternMatches = true;

            if (exactMatches && patternMatches)
                return true;
        }

        return false;
    }

    protected static TStringArray GetDeclaredInventorySlots(EntityAI target)
    {
        TStringArray slots = new TStringArray();
        if (!target || !GetGame())
            return slots;

        string className = target.GetType();
        string configRoot = GetConfigRootForClass(className);
        if (configRoot == "")
            return slots;

        string inventorySlotPath = configRoot + " " + className + " inventorySlot";
        GetGame().ConfigGetTextArray(inventorySlotPath, slots);

        // inventorySlot is conventionally an array, but tolerate a single text
        // value for third-party configs without making slot matching class-specific.
        if (slots.Count() == 0)
        {
            string singleSlot;
            if (GetGame().ConfigGetText(inventorySlotPath, singleSlot) && singleSlot != "")
                slots.Insert(singleSlot);
        }

        return slots;
    }

    protected static string GetConfigRootForClass(string className)
    {
        if (!GetGame())
            return "";

        if (GetGame().ConfigIsExisting("CfgWeapons " + className))
            return "CfgWeapons";
        if (GetGame().ConfigIsExisting("CfgMagazines " + className))
            return "CfgMagazines";
        if (GetGame().ConfigIsExisting("CfgVehicles " + className))
            return "CfgVehicles";
        return "";
    }

    protected static bool GlobMatchesNormalized(string pattern, string value)
    {
        int patternIndex;
        int valueIndex;
        int starIndex = -1;
        int starValueIndex = -1;

        while (valueIndex < value.Length())
        {
            if (patternIndex < pattern.Length())
            {
                string token = pattern.Substring(patternIndex, 1);
                if (token == "?" || token == value.Substring(valueIndex, 1))
                {
                    patternIndex++;
                    valueIndex++;
                    continue;
                }

                if (token == "*")
                {
                    starIndex = patternIndex;
                    starValueIndex = valueIndex;
                    patternIndex++;
                    continue;
                }
            }

            if (starIndex >= 0)
            {
                patternIndex = starIndex + 1;
                starValueIndex++;
                valueIndex = starValueIndex;
                continue;
            }

            return false;
        }

        while (patternIndex < pattern.Length() && pattern.Substring(patternIndex, 1) == "*")
            patternIndex++;

        return patternIndex == pattern.Length();
    }

    protected static void ScheduleNextReload()
    {
        if (!GetGame())
            return;

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(ReloadScheduled);
        if (s_ActiveConfig && s_ActiveConfig.reload_seconds > 0)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ReloadScheduled, s_ActiveConfig.reload_seconds * 1000, false);
    }

    protected static void BroadcastToClients()
    {
        if (!GetGame() || !GetGame().IsServer() || !s_ActiveConfig)
            return;

        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        foreach (Man man : players)
        {
            PlayerBase player = PlayerBase.Cast(man);
            if (player && player.GetIdentity())
                SendToClient(player, player.GetIdentity());
        }
    }

    protected static void Info(string text)
    {
        Print("[PaintZ][ItemPolicy] " + text);
    }

    protected static void Warn(string text)
    {
        Print("[PaintZ][ItemPolicy] WARNING: " + text);
    }

    protected static void Error(string text)
    {
        Print("[PaintZ][ItemPolicy] ERROR: " + text);
    }

    protected static void Reject(string reason)
    {
        Print("[PaintZ][ItemPolicy] Config rejected: " + reason);
        Print("[PaintZ][ItemPolicy] See " + PROFILE_README_PATH);
    }

#ifdef DIAG_DEVELOPER
    static PaintZ_ItemPolicyConfig GetActiveConfigForTests()
    {
        return s_ActiveConfig;
    }

    static bool InstallConfigForTests(PaintZ_ItemPolicyConfig config)
    {
        string error;
        if (!ValidateAndNormalize(config, error))
            return false;

        s_ActiveConfig = config;
        s_DecisionCache.Clear();
        s_DomainCache.Clear();
        return true;
    }
#endif
};
