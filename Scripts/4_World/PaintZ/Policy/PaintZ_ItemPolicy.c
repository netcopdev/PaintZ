class PaintZ_ItemPolicy
{
    static const string PROFILE_DIRECTORY = "$profile:PaintZ";
    static const string PROFILE_PATH = "$profile:PaintZ/paintz_items.json";
    static const string BUNDLED_DEFAULT_PATH = "PaintZ/config/paintz_items.default.json";

    protected static ref PaintZ_ItemPolicyConfig s_ActiveConfig;
    protected static ref map<string, bool> s_DecisionCache = new map<string, bool>;
    protected static bool s_ServerStarted;

    static void StartServer()
    {
        if (s_ServerStarted)
            return;

        s_ServerStarted = true;
        MakeDirectory(PROFILE_DIRECTORY);

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
    }

    static bool IsPaintApplicationAllowed(EntityAI target)
    {
        if (!target || !PaintZ_PaintInspector.IsSupportedTarget(target))
            return false;

        // Remote clients do not read the server profile. The server checks the
        // action condition and checks again at the mutation boundary.
        if (GetGame() && !GetGame().IsServer())
            return true;

        if (!s_ActiveConfig)
            return false;

        string category = GetItemType(target);
        if (category == "")
            return false;

        string className = target.GetType();
        string normalizedClass = className;
        normalizedClass.ToLower();
        string cacheKey = category + "|" + normalizedClass;

        bool cached;
        if (s_DecisionCache.Find(cacheKey, cached))
            return cached;

        bool allowed = EvaluateConfig(target, category, className, normalizedClass, s_ActiveConfig);
        s_DecisionCache.Insert(cacheKey, allowed);
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
            Error("config load failed path=" + PROFILE_PATH + " error=" + loadError);
            if (periodic && s_ActiveConfig)
                Error("reload rejected; continuing with previous valid policy");
            ScheduleNextReload();
            return false;
        }

        string validationError;
        if (!ValidateAndNormalize(candidate, validationError))
        {
            Error("config validation failed path=" + PROFILE_PATH + " error=" + validationError);
            if (periodic && s_ActiveConfig)
                Error("reload rejected; continuing with previous valid policy");
            ScheduleNextReload();
            return false;
        }

        // Atomic from PaintZ's perspective: only a fully parsed and validated
        // detached candidate reaches the active-policy assignment.
        s_ActiveConfig = candidate;
        s_DecisionCache.Clear();

        if (periodic)
            Info("config successfully reloaded rules=" + GetRuleCount());
        else
            Info("config loaded rules=" + GetRuleCount());

        if (candidate.reload_seconds == -1)
            Info("reload mode=startup-only reload_seconds=-1");
        else
            Info("reload mode=periodic interval_seconds=" + candidate.reload_seconds);

        ScheduleNextReload();
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
                    firstError = "invalid rule index=" + i + ": " + ruleError;
                valid = false;
            }
        }

        if (!valid)
        {
            error = firstError;
            return false;
        }

        return true;
    }

    protected static bool ValidateAndNormalizeRule(PaintZ_ItemPolicyRule rule, out string error)
    {
        if (!rule)
        {
            error = "rule object is null";
            return false;
        }

        rule.action.ToLower();
        if (rule.action != "include" && rule.action != "exclude")
        {
            error = "action must be include or exclude";
            return false;
        }

        rule.type.ToLower();
        if (rule.type != "weapon" && rule.type != "magazine" && rule.type != "all")
        {
            error = "type must be weapon, magazine, or all";
            return false;
        }

        bool hasPattern = rule.class_pattern != "";
        bool hasInheritance = rule.inherits != "";
        if (hasPattern == hasInheritance)
        {
            error = "exactly one selector is required: class_pattern or inherits";
            return false;
        }

        if (hasPattern)
        {
            rule.class_pattern.ToLower();
            return true;
        }

        rule.inherits.ToLower();
        if (!InheritanceClassExists(rule.type, rule.inherits))
        {
            error = "inherits class does not exist for rule type: " + rule.inherits;
            return false;
        }
        return true;
    }

    protected static bool InheritanceClassExists(string ruleType, string className)
    {
        if (!GetGame())
            return false;

        if ((ruleType == "weapon" || ruleType == "all") && GetGame().ConfigIsExisting("CfgWeapons " + className))
            return true;

        if ((ruleType == "magazine" || ruleType == "all") && GetGame().ConfigIsExisting("CfgMagazines " + className))
            return true;

        return false;
    }

    protected static bool EvaluateConfig(EntityAI target, string category, string className, string normalizedClass, PaintZ_ItemPolicyConfig config)
    {
        bool allowed = config.default_action == "allow";
        for (int i = 0; i < config.rules.Count(); i++)
        {
            PaintZ_ItemPolicyRule rule = config.rules.Get(i);
            if (!TypeMatches(rule.type, category))
                continue;

            bool selectorMatches;
            if (rule.class_pattern != "")
                selectorMatches = GlobMatchesNormalized(rule.class_pattern, normalizedClass);
            else
                selectorMatches = GetGame().IsKindOf(className, rule.inherits);

            if (selectorMatches)
                allowed = rule.action == "include";
        }

        return allowed;
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

    protected static string GetItemType(EntityAI target)
    {
        Weapon_Base weapon;
        if (Class.CastTo(weapon, target))
            return "weapon";

        Magazine magazine;
        if (Class.CastTo(magazine, target) && !target.IsAmmoPile())
            return "magazine";

        return "";
    }

    protected static bool TypeMatches(string ruleType, string targetType)
    {
        return ruleType == "all" || ruleType == targetType;
    }

    protected static void ScheduleNextReload()
    {
        if (!GetGame())
            return;

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(ReloadScheduled);
        if (s_ActiveConfig && s_ActiveConfig.reload_seconds > 0)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ReloadScheduled, s_ActiveConfig.reload_seconds * 1000, false);
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
        return true;
    }
#endif
};
