class PaintZ_ActionTuning
{
    static const int RPC_ACTION_TUNING_SYNC = 782343;
    static const string PROFILE_DIRECTORY = "$profile:PaintZ";
    static const string PROFILE_PATH = "$profile:PaintZ/paintz_action_tuning.json";
    static const string PROFILE_README_PATH = "$profile:PaintZ/paintz_action_tuning_README.txt";
    static const string BUNDLED_DEFAULT_PATH = "PaintZ/config/paintz_action_tuning.default.json";
    static const string BUNDLED_README_PATH = "PaintZ/config/paintz_action_tuning_README.txt";

    static const float DEFAULT_MIN_DIMENSION_M = 0.2;
    static const float DEFAULT_MAX_DIMENSION_M = 0.8;
    static const float DEFAULT_MIN_TIME_SECONDS = 5.0;
    static const float DEFAULT_MAX_TIME_SECONDS = 20.0;
    static const float DEFAULT_PAINT_APPLICATIONS_PER_FULL_CAN_AT_MAX_SIZE = 3.0;
    static const float DEFAULT_STRIP_APPLICATIONS_PER_FULL_CAN_AT_MAX_SIZE = 3.0;

    protected static ref PaintZ_ActionTuningConfig s_ActiveConfig;
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
                Warn("Could not create runtime action-tuning README from " + BUNDLED_README_PATH);
            else
                Info("created runtime README path=" + PROFILE_README_PATH);
        }

        if (!FileExist(PROFILE_PATH))
        {
            if (!CopyFile(BUNDLED_DEFAULT_PATH, PROFILE_PATH))
            {
                Error("Could not create runtime action-tuning config from bundled default " + BUNDLED_DEFAULT_PATH);
                return;
            }

            Info("created runtime config from default path=" + PROFILE_PATH);
        }

        if (!ReloadInternal(false))
            Error("startup config load failed; built-in action-tuning defaults remain active until a valid config is loaded");
    }

    static void StopServer()
    {
        if (GetGame())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(ReloadScheduled);

        s_ServerStarted = false;
        s_ActiveConfig = null;
    }

    static float ResolvePaintTime(ActionTarget actionTarget)
    {
        EntityAI target = PaintZ_PaintTarget.ResolveNewPaintTarget(actionTarget);
        return ResolveActionTime(target);
    }

    static float ResolveStripTime(ActionTarget actionTarget)
    {
        EntityAI target = PaintZ_PaintTarget.ResolvePaintedTarget(actionTarget);
        return ResolveActionTime(target);
    }

    static float ResolveActionTime(EntityAI target)
    {
        float minDimension = GetMinDimensionMeters();
        float maxDimension = GetMaxDimensionMeters();
        float minTime = GetMinTimeSeconds();
        float maxTime = GetMaxTimeSeconds();
        float dimension = ResolveClampedDimensionMeters(target, minDimension, maxDimension);
        float ratio = (dimension - minDimension) / (maxDimension - minDimension);
        return minTime + ((maxTime - minTime) * ratio);
    }

    static float ResolvePaintUsage(EntityAI target, ItemBase applicator)
    {
        return ResolveUsage(target, applicator, GetPaintApplicationsPerFullCanAtMaxSize());
    }

    static float ResolveStripUsage(EntityAI target, ItemBase applicator)
    {
        return ResolveUsage(target, applicator, GetStripApplicationsPerFullCanAtMaxSize());
    }

    static float ResolveDimensionMeters(EntityAI target)
    {
        float minDimension = GetMinDimensionMeters();
        float maxDimension = GetMaxDimensionMeters();
        return ResolveClampedDimensionMeters(target, minDimension, maxDimension);
    }

    static void SendToClient(PlayerBase player, PlayerIdentity identity)
    {
        if (!player || !identity || !s_ActiveConfig)
            return;

        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(s_ActiveConfig.version);
        rpc.Write(s_ActiveConfig.reload_seconds);
        rpc.Write(s_ActiveConfig.min_dimension_m);
        rpc.Write(s_ActiveConfig.max_dimension_m);
        rpc.Write(s_ActiveConfig.min_time_seconds);
        rpc.Write(s_ActiveConfig.max_time_seconds);
        rpc.Write(s_ActiveConfig.paint_applications_per_full_can_at_max_size);
        rpc.Write(s_ActiveConfig.strip_applications_per_full_can_at_max_size);
        rpc.Send(player, RPC_ACTION_TUNING_SYNC, true, identity);
    }

    static bool ReceiveFromServer(ParamsReadContext ctx)
    {
        PaintZ_ActionTuningConfig candidate = new PaintZ_ActionTuningConfig();
        if (!ctx.Read(candidate.version) || !ctx.Read(candidate.reload_seconds) || !ctx.Read(candidate.min_dimension_m) || !ctx.Read(candidate.max_dimension_m) || !ctx.Read(candidate.min_time_seconds) || !ctx.Read(candidate.max_time_seconds) || !ctx.Read(candidate.paint_applications_per_full_can_at_max_size) || !ctx.Read(candidate.strip_applications_per_full_can_at_max_size))
        {
            Error("client action-tuning sync was truncated");
            return false;
        }

        string validationError;
        if (!ValidateAndNormalize(candidate, validationError))
        {
            Error("client action-tuning sync rejected: " + validationError);
            return false;
        }

        s_ActiveConfig = candidate;
        Info("client action tuning synchronized min_dimension_m=" + candidate.min_dimension_m + " max_dimension_m=" + candidate.max_dimension_m);
        return true;
    }

    protected static float ResolveUsage(EntityAI target, ItemBase applicator, float applicationsPerFullCanAtMaxSize)
    {
        if (!applicator || applicationsPerFullCanAtMaxSize <= 0.0)
            return 0.0;

        float maxQuantity = applicator.GetQuantityMax();
        if (maxQuantity <= 0.0)
            return 0.0;

        float minDimension = GetMinDimensionMeters();
        float maxDimension = GetMaxDimensionMeters();
        float dimension = ResolveClampedDimensionMeters(target, minDimension, maxDimension);
        float sizeFraction = dimension / maxDimension;
        float fullCanFraction = sizeFraction / applicationsPerFullCanAtMaxSize;
        return maxQuantity * fullCanFraction;
    }

    protected static float ResolveClampedDimensionMeters(EntityAI target, float minDimension, float maxDimension)
    {
        float measuredDimension;
        if (!PaintZ_PatternScaling.GetMaxDimensionMeters(target, measuredDimension))
            measuredDimension = maxDimension;

        if (measuredDimension < minDimension)
            return minDimension;
        if (measuredDimension > maxDimension)
            return maxDimension;
        return measuredDimension;
    }

    protected static float GetMinDimensionMeters()
    {
        if (s_ActiveConfig)
            return s_ActiveConfig.min_dimension_m;
        return DEFAULT_MIN_DIMENSION_M;
    }

    protected static float GetMaxDimensionMeters()
    {
        if (s_ActiveConfig)
            return s_ActiveConfig.max_dimension_m;
        return DEFAULT_MAX_DIMENSION_M;
    }

    protected static float GetMinTimeSeconds()
    {
        if (s_ActiveConfig)
            return s_ActiveConfig.min_time_seconds;
        return DEFAULT_MIN_TIME_SECONDS;
    }

    protected static float GetMaxTimeSeconds()
    {
        if (s_ActiveConfig)
            return s_ActiveConfig.max_time_seconds;
        return DEFAULT_MAX_TIME_SECONDS;
    }

    protected static float GetPaintApplicationsPerFullCanAtMaxSize()
    {
        if (s_ActiveConfig)
            return s_ActiveConfig.paint_applications_per_full_can_at_max_size;
        return DEFAULT_PAINT_APPLICATIONS_PER_FULL_CAN_AT_MAX_SIZE;
    }

    protected static float GetStripApplicationsPerFullCanAtMaxSize()
    {
        if (s_ActiveConfig)
            return s_ActiveConfig.strip_applications_per_full_can_at_max_size;
        return DEFAULT_STRIP_APPLICATIONS_PER_FULL_CAN_AT_MAX_SIZE;
    }

    protected static void ReloadScheduled()
    {
        ReloadInternal(true);
    }

    protected static bool ReloadInternal(bool periodic)
    {
        PaintZ_ActionTuningConfig candidate = new PaintZ_ActionTuningConfig();
        string loadError;
        if (!JsonFileLoader<PaintZ_ActionTuningConfig>.LoadFile(PROFILE_PATH, candidate, loadError))
        {
            Reject("invalid JSON or incompatible value: " + loadError);
            if (periodic && s_ActiveConfig)
                Error("reload rejected; continuing with previous valid action-tuning config");
            ScheduleNextReload();
            return false;
        }

        string validationError;
        if (!ValidateAndNormalize(candidate, validationError))
        {
            Reject(validationError);
            if (periodic && s_ActiveConfig)
                Error("reload rejected; continuing with previous valid action-tuning config");
            ScheduleNextReload();
            return false;
        }

        s_ActiveConfig = candidate;
        BroadcastToClients();

        if (periodic)
            Info("config successfully reloaded min_dimension_m=" + candidate.min_dimension_m + " max_dimension_m=" + candidate.max_dimension_m);
        else
            Info("config loaded min_dimension_m=" + candidate.min_dimension_m + " max_dimension_m=" + candidate.max_dimension_m);

        if (candidate.reload_seconds == -1)
            Info("reload mode=startup-only reload_seconds=-1");
        else
            Info("reload mode=periodic interval_seconds=" + candidate.reload_seconds);

        ScheduleNextReload();
        return true;
    }

    protected static bool ValidateAndNormalize(PaintZ_ActionTuningConfig config, out string error)
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

        if (config.reload_seconds == 0 || config.reload_seconds < -1)
        {
            Warn("reload_seconds=" + config.reload_seconds + " is unsafe; treating it as -1 (startup-only)");
            config.reload_seconds = -1;
        }

        if (config.min_dimension_m <= 0.0)
        {
            error = "min_dimension_m must be greater than 0";
            return false;
        }

        if (config.max_dimension_m <= config.min_dimension_m)
        {
            error = "max_dimension_m must be greater than min_dimension_m";
            return false;
        }

        if (config.min_time_seconds <= 0.0)
        {
            error = "min_time_seconds must be greater than 0";
            return false;
        }

        if (config.max_time_seconds < config.min_time_seconds)
        {
            error = "max_time_seconds must be greater than or equal to min_time_seconds";
            return false;
        }

        if (config.paint_applications_per_full_can_at_max_size <= 0.0)
        {
            error = "paint_applications_per_full_can_at_max_size must be greater than 0";
            return false;
        }

        if (config.strip_applications_per_full_can_at_max_size <= 0.0)
        {
            error = "strip_applications_per_full_can_at_max_size must be greater than 0";
            return false;
        }

        return true;
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
        PaintZ_PaintLog.Info("action_tuning " + text);
    }

    protected static void Warn(string text)
    {
        PaintZ_PaintLog.Warning("action_tuning " + text);
    }

    protected static void Error(string text)
    {
        PaintZ_PaintLog.Warning("action_tuning ERROR " + text);
    }

    protected static void Reject(string text)
    {
        PaintZ_PaintLog.Warning("action_tuning config rejected: " + text);
    }
};
