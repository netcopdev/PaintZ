class PaintZ_PatternScaling
{
    static const string PROFILE_DIRECTORY = "$profile:PaintZ";
    static const string PROFILE_PATH = "$profile:PaintZ/paintz_pattern_scaling.json";
    static const string PROFILE_README_PATH = "$profile:PaintZ/paintz_pattern_scaling_README.txt";
    static const string BUNDLED_DEFAULT_PATH = "PaintZ/config/paintz_pattern_scaling.default.json";
    static const string BUNDLED_README_PATH = "PaintZ/config/paintz_pattern_scaling_README.txt";

    protected static ref PaintZ_PatternScalingConfig s_ActiveConfig;
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
                Warn("Could not create runtime pattern-scaling README from " + BUNDLED_README_PATH);
            else
                Info("created runtime README path=" + PROFILE_README_PATH);
        }

        if (!FileExist(PROFILE_PATH))
        {
            if (!CopyFile(BUNDLED_DEFAULT_PATH, PROFILE_PATH))
            {
                Error("Could not create runtime pattern-scaling config from bundled default " + BUNDLED_DEFAULT_PATH);
                return;
            }

            Info("created runtime config from default path=" + PROFILE_PATH);
        }

        if (!ReloadInternal(false))
            Error("startup config load failed; pattern finishes will use 1x until a valid config is loaded");
    }

    static void StopServer()
    {
        if (GetGame())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(ReloadScheduled);

        s_ServerStarted = false;
        s_ActiveConfig = null;
    }

    static int ResolveScalePercent(EntityAI target, string paintCode, out float maxDimensionMeters)
    {
        maxDimensionMeters = -1.0;

        if (!target || paintCode == PaintZ_PaintConstants.PAINT_NONE || !PaintZ_PaintCatalog.IsPatternPaint(paintCode))
            return 100;

        if (!s_ActiveConfig || !s_ActiveConfig.enabled)
            return 100;

        int defaultScalePercent;
        if (!ResolveSupportedScalePercent(s_ActiveConfig.default_scale, defaultScalePercent))
            defaultScalePercent = 100;

        if (!GetMaxDimensionMeters(target, maxDimensionMeters))
            return defaultScalePercent;

        for (int i = 0; s_ActiveConfig.ranges && i < s_ActiveConfig.ranges.Count(); i++)
        {
            PaintZ_PatternScaleRange range = s_ActiveConfig.ranges.Get(i);
            if (maxDimensionMeters <= range.max_dimension_m)
            {
                int rangeScalePercent;
                if (ResolveSupportedScalePercent(range.scale, rangeScalePercent))
                    return rangeScalePercent;

                return defaultScalePercent;
            }
        }

        return defaultScalePercent;
    }

    static bool GetMaxDimensionMeters(EntityAI target, out float maxDimensionMeters)
    {
        maxDimensionMeters = -1.0;
        if (!target)
            return false;

        vector minMax[2];
        if (!target.GetCollisionBox(minMax))
            return false;

        vector size = minMax[1] - minMax[0];
        float x = Math.AbsFloat(size[0]);
        float y = Math.AbsFloat(size[1]);
        float z = Math.AbsFloat(size[2]);

        maxDimensionMeters = x;
        if (y > maxDimensionMeters)
            maxDimensionMeters = y;
        if (z > maxDimensionMeters)
            maxDimensionMeters = z;

        return maxDimensionMeters > 0.0;
    }

    static int GetReloadSeconds()
    {
        if (!s_ActiveConfig)
            return -1;

        return s_ActiveConfig.reload_seconds;
    }

    protected static void ReloadScheduled()
    {
        ReloadInternal(true);
    }

    protected static bool ReloadInternal(bool periodic)
    {
        PaintZ_PatternScalingConfig candidate = new PaintZ_PatternScalingConfig();
        string loadError;
        if (!JsonFileLoader<PaintZ_PatternScalingConfig>.LoadFile(PROFILE_PATH, candidate, loadError))
        {
            Reject("invalid JSON or incompatible value: " + loadError);
            if (periodic && s_ActiveConfig)
                Error("reload rejected; continuing with previous valid pattern-scaling config");
            ScheduleNextReload();
            return false;
        }

        string validationError;
        if (!Validate(candidate, validationError))
        {
            Reject(validationError);
            if (periodic && s_ActiveConfig)
                Error("reload rejected; continuing with previous valid pattern-scaling config");
            ScheduleNextReload();
            return false;
        }

        s_ActiveConfig = candidate;

        if (periodic)
            Info("config successfully reloaded enabled=" + candidate.enabled + " ranges=" + candidate.ranges.Count());
        else
            Info("config loaded enabled=" + candidate.enabled + " ranges=" + candidate.ranges.Count());

        if (candidate.reload_seconds == -1)
            Info("reload mode=startup-only reload_seconds=-1");
        else
            Info("reload mode=periodic interval_seconds=" + candidate.reload_seconds);

        // Deliberately do not scan/reapply existing items here. A new mapping is
        // consumed on the next repaint or when persisted paint is restored after
        // a server restart/load.
        ScheduleNextReload();
        return true;
    }

    protected static bool Validate(PaintZ_PatternScalingConfig config, out string error)
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

        if (config.default_scale == -999999.0)
        {
            error = "default_scale is required";
            return false;
        }

        int defaultScalePercent;
        if (!ResolveSupportedScalePercent(config.default_scale, defaultScalePercent))
        {
            error = "default_scale=" + config.default_scale + " has no generated PaintZ pattern asset";
            return false;
        }

        if (!config.ranges)
            config.ranges = new array<ref PaintZ_PatternScaleRange>;

        float previousMax = -1.0;
        for (int i = 0; i < config.ranges.Count(); i++)
        {
            PaintZ_PatternScaleRange range = config.ranges.Get(i);
            if (!range)
            {
                error = "range " + i + " must be a JSON object";
                return false;
            }

            if (range.max_dimension_m <= 0.0)
            {
                error = "range " + i + " max_dimension_m must be greater than 0";
                return false;
            }

            if (previousMax >= 0.0 && range.max_dimension_m <= previousMax)
            {
                error = "ranges must be ordered by strictly increasing max_dimension_m";
                return false;
            }

            int scalePercent;
            if (!ResolveSupportedScalePercent(range.scale, scalePercent))
            {
                error = "range " + i + " scale=" + range.scale + " has no generated PaintZ pattern asset";
                return false;
            }

            previousMax = range.max_dimension_m;
        }

        return true;
    }

    protected static bool ResolveSupportedScalePercent(float scale, out int scalePercent)
    {
        scalePercent = Math.Round(scale * 100.0);
        if (scalePercent <= 0 || scalePercent > 1000)
            return false;

        float normalizedScale = scalePercent * 0.01;
        if (Math.AbsFloat(normalizedScale - scale) > 0.0001)
            return false;

        return PaintZ_PaintCatalog.IsSupportedPatternScale(scalePercent);
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
        PaintZ_PaintLog.Info("pattern_scaling " + text);
    }

    protected static void Warn(string text)
    {
        PaintZ_PaintLog.Warning("pattern_scaling " + text);
    }

    protected static void Error(string text)
    {
        PaintZ_PaintLog.Warning("pattern_scaling ERROR " + text);
    }

    protected static void Reject(string text)
    {
        PaintZ_PaintLog.Warning("pattern_scaling config rejected: " + text);
    }
};