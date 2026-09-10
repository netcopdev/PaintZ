class PaintZ_StaleFinishRecovery
{
    static const string PROFILE_DIRECTORY = "$profile:PaintZ";
    static const string PROFILE_PATH = "$profile:PaintZ/paintz_stale_finishes.json";
    static const string PROFILE_README_PATH = "$profile:PaintZ/paintz_stale_finishes_README.txt";
    static const string BUNDLED_DEFAULT_PATH = "PaintZ/config/paintz_stale_finishes.default.json";
    static const string BUNDLED_README_PATH = "PaintZ/config/paintz_stale_finishes_README.txt";

    protected static ref PaintZ_StaleFinishRecoveryConfig s_ActiveConfig;
    protected static bool s_ServerStarted;
    protected static int s_Revision;

    static void StartServer()
    {
        if (s_ServerStarted)
            return;

        s_ServerStarted = true;
        MakeDirectory(PROFILE_DIRECTORY);

        if (!FileExist(PROFILE_README_PATH))
        {
            if (!CopyFile(BUNDLED_README_PATH, PROFILE_README_PATH))
                Warn("Could not create runtime stale-finish README from " + BUNDLED_README_PATH);
            else
                Info("created runtime README path=" + PROFILE_README_PATH);
        }

        if (!FileExist(PROFILE_PATH))
        {
            if (!CopyFile(BUNDLED_DEFAULT_PATH, PROFILE_PATH))
            {
                Error("Could not create runtime stale-finish config from bundled default " + BUNDLED_DEFAULT_PATH);
                return;
            }

            Info("created runtime config from default path=" + PROFILE_PATH);
        }

        if (!ReloadInternal(false))
            Error("startup config load failed; stale finish state will be preserved until a valid config is loaded");
    }

    static void StopServer()
    {
        if (GetGame())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(ReloadScheduled);

        s_ServerStarted = false;
        s_ActiveConfig = null;
        s_Revision = 0;
    }

    static int GetRevision()
    {
        return s_Revision;
    }

    static bool Resolve(ItemBase item)
    {
        if (!item || !GetGame() || !GetGame().IsServer() || !s_ActiveConfig)
            return false;

        string staleFinish = item.PaintZ_GetPaintCode();
        if (staleFinish == "" || staleFinish == PaintZ_PaintConstants.PAINT_NONE)
            return false;

        staleFinish.ToUpper();
        if (PaintZ_PaintPackRegistry.HasFinish(staleFinish))
            return false;

        string replacementFinish;
        if (s_ActiveConfig.migrations && s_ActiveConfig.migrations.Find(staleFinish, replacementFinish))
            return ResolveMigration(item, staleFinish, replacementFinish);

        if (!s_ActiveConfig.prune_unknown)
            return false;

        return ResolvePrune(item, staleFinish);
    }

    protected static bool ResolveMigration(ItemBase item, string staleFinish, string replacementFinish)
    {
        if (!PaintZ_PaintPackRegistry.HasFinish(replacementFinish))
        {
            Warn("migration skipped target=" + item.GetType() + " from=" + staleFinish + " to=" + replacementFinish + " reason=target_not_registered state_preserved=1");
            return false;
        }

        int selectionIndex = item.PaintZ_GetPaintSelection();
        if (selectionIndex < 0)
        {
            PaintZ_PaintInspectionResult inspection = PaintZ_PaintInspector.Inspect(item);
            if (inspection && inspection.m_Paintable)
                selectionIndex = inspection.m_SelectionIndex;
        }

        if (selectionIndex < 0)
        {
            Warn("migration skipped target=" + item.GetType() + " from=" + staleFinish + " to=" + replacementFinish + " reason=no_safe_selection state_preserved=1");
            return false;
        }

        if (!item.PaintZ_SetPaintState(replacementFinish, selectionIndex))
        {
            Warn("migration skipped target=" + item.GetType() + " from=" + staleFinish + " to=" + replacementFinish + " reason=apply_failed state_preserved=1");
            return false;
        }

        Info("migrated target=" + item.GetType() + " from=" + staleFinish + " to=" + replacementFinish + " selection_index=" + selectionIndex);
        return true;
    }

    protected static bool ResolvePrune(ItemBase item, string staleFinish)
    {
        int selectionIndex = item.PaintZ_GetPaintSelection();
        if (selectionIndex < 0)
        {
            PaintZ_PaintInspectionResult inspection = PaintZ_PaintInspector.Inspect(item);
            if (inspection && inspection.m_Paintable)
                selectionIndex = inspection.m_SelectionIndex;
        }

        if (!item.PaintZ_SetPaintState(PaintZ_PaintConstants.PAINT_NONE, selectionIndex))
        {
            Warn("prune skipped target=" + item.GetType() + " stale_finish=" + staleFinish + " reason=clear_failed state_preserved=1");
            return false;
        }

        string visualState = "not_available";
        if (selectionIndex >= 0)
            visualState = "restored";

        Info("pruned target=" + item.GetType() + " stale_finish=" + staleFinish + " selection_index=" + selectionIndex + " visual=" + visualState);
        return true;
    }

    protected static void ReloadScheduled()
    {
        ReloadInternal(true);
    }

    protected static bool ReloadInternal(bool periodic)
    {
        PaintZ_StaleFinishRecoveryConfig candidate = new PaintZ_StaleFinishRecoveryConfig();
        string loadError;
        if (!JsonFileLoader<PaintZ_StaleFinishRecoveryConfig>.LoadFile(PROFILE_PATH, candidate, loadError))
        {
            Reject("invalid JSON or incompatible value: " + loadError);
            if (periodic && s_ActiveConfig)
                Error("reload rejected; continuing with previous valid stale-finish config");
            ScheduleNextReload();
            return false;
        }

        string validationError;
        if (!Validate(candidate, validationError))
        {
            Reject(validationError);
            if (periodic && s_ActiveConfig)
                Error("reload rejected; continuing with previous valid stale-finish config");
            ScheduleNextReload();
            return false;
        }

        s_ActiveConfig = candidate;
        s_Revision++;

        int migrationCount = 0;
        if (candidate.migrations)
            migrationCount = candidate.migrations.Count();

        if (periodic)
            Info("config successfully reloaded prune_unknown=" + candidate.prune_unknown + " migrations=" + migrationCount + " revision=" + s_Revision);
        else
            Info("config loaded prune_unknown=" + candidate.prune_unknown + " migrations=" + migrationCount + " revision=" + s_Revision);

        if (candidate.reload_seconds == -1)
            Info("reload mode=startup-only reload_seconds=-1");
        else
            Info("reload mode=periodic interval_seconds=" + candidate.reload_seconds);

        ScheduleNextReload();
        return true;
    }

    protected static bool Validate(PaintZ_StaleFinishRecoveryConfig config, out string error)
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

        if (!config.migrations)
            config.migrations = new map<string, string>;

        map<string, string> normalized = new map<string, string>;
        for (int i = 0; i < config.migrations.Count(); i++)
        {
            string sourceId = config.migrations.GetKey(i);
            string targetId = config.migrations.GetElement(i);
            sourceId.ToUpper();
            targetId.ToUpper();

            if (sourceId == "" || sourceId == PaintZ_PaintConstants.PAINT_NONE)
            {
                error = "migration source at index " + i + " is empty or invalid";
                return false;
            }

            if (targetId == "" || targetId == PaintZ_PaintConstants.PAINT_NONE)
            {
                error = "migration target for " + sourceId + " is empty or invalid";
                return false;
            }

            if (sourceId.Contains("*") || sourceId.Contains("?") || targetId.Contains("*") || targetId.Contains("?"))
            {
                error = "migrations require exact finish IDs; wildcards are not allowed for " + sourceId + " -> " + targetId;
                return false;
            }

            if (sourceId == targetId)
            {
                error = "migration source and target must differ: " + sourceId;
                return false;
            }

            if (normalized.Contains(sourceId))
            {
                error = "duplicate migration source after case normalization: " + sourceId;
                return false;
            }

            normalized.Set(sourceId, targetId);
        }

        for (int j = 0; j < normalized.Count(); j++)
        {
            string oldId = normalized.GetKey(j);
            string newId = normalized.GetElement(j);
            if (normalized.Contains(newId))
            {
                error = "migration chains are not allowed: " + oldId + " -> " + newId + " and " + newId + " is also a source";
                return false;
            }
        }

        config.migrations = normalized;
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

    protected static void Info(string text)
    {
        PaintZ_PaintLog.Info("stale_finish_recovery " + text);
    }

    protected static void Warn(string text)
    {
        PaintZ_PaintLog.Warning("stale_finish_recovery " + text);
    }

    protected static void Error(string text)
    {
        PaintZ_PaintLog.Warning("stale_finish_recovery ERROR " + text);
    }

    protected static void Reject(string text)
    {
        PaintZ_PaintLog.Warning("stale_finish_recovery config rejected: " + text);
    }
};
