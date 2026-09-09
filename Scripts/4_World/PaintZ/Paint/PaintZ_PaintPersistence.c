#ifdef DIAG_DEVELOPER
enum PaintZ_PersistenceReadResult
{
    PZ_PERSISTENCE_LEGACY,
    PZ_PERSISTENCE_VALID,
    PZ_PERSISTENCE_INVALID
}
#endif

class PaintZ_PaintPersistence
{
    protected static const string STORAGE_KEY = "PaintZ";
    protected static const int STORAGE_VERSION = 1;

    static void Save(CF_ModStorageMap storage, EntityAI target, string paintCode)
    {
        if (paintCode == PaintZ_PaintConstants.PAINT_NONE)
            return;

        CF_ModStorage ctx = storage[STORAGE_KEY];
        if (!ctx)
        {
            Warn(target, "CF ModStorage context is unavailable while saving");
            return;
        }

        ctx.Write(paintCode);
    }

    static bool Load(CF_ModStorageMap storage, EntityAI target, out string paintCode)
    {
        paintCode = PaintZ_PaintConstants.PAINT_NONE;
        CF_ModStorage ctx = storage[STORAGE_KEY];
        if (!ctx)
            return true;

        if (ctx.GetVersion() != STORAGE_VERSION)
        {
            Warn(target, "unsupported storage version=" + ctx.GetVersion() + "; ignoring PaintZ state");
            return true;
        }

        if (!ctx.Read(paintCode))
        {
            Warn(target, "malformed CF ModStorage payload: finish ID could not be read; ignoring PaintZ state");
            paintCode = PaintZ_PaintConstants.PAINT_NONE;
            return true;
        }

        if (paintCode == "" || paintCode == PaintZ_PaintConstants.PAINT_NONE)
        {
            Warn(target, "malformed CF ModStorage payload: invalid finish ID; ignoring PaintZ state");
            paintCode = PaintZ_PaintConstants.PAINT_NONE;
            return true;
        }

        paintCode.ToUpper();
        return true;
    }

#ifdef DIAG_DEVELOPER
    static void Save(ParamsWriteContext ctx, string paintCode)
    {
        TStringArray payload = new TStringArray();
        if (paintCode != PaintZ_PaintConstants.PAINT_NONE)
            payload.Insert(paintCode);

        ctx.Write("PaintZ.TestPaintState");
        ctx.Write(STORAGE_VERSION);
        ctx.Write(payload);
    }

    static PaintZ_PersistenceReadResult Load(ParamsReadContext ctx, EntityAI target, out string paintCode)
    {
        paintCode = PaintZ_PaintConstants.PAINT_NONE;
        if (!ctx.CanRead())
            return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_LEGACY;

        string marker;
        if (!ctx.Read(marker))
            return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_LEGACY;
        if (marker != "PaintZ.TestPaintState")
            return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_INVALID;

        int version;
        TStringArray payload = new TStringArray();
        if (!ctx.Read(version) || !ctx.Read(payload) || version != STORAGE_VERSION)
            return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_INVALID;

        if (payload.Count() == 0)
            return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_VALID;
        if (payload.Count() != 1)
            return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_INVALID;

        paintCode = payload.Get(0);
        return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_VALID;
    }
#endif

    protected static void Warn(EntityAI target, string reason)
    {
        string targetType = "<null>";
        if (target)
            targetType = target.GetType();

        PaintZ_PaintLog.Warning("persistence target=" + targetType + " " + reason);
    }
}

class PaintZ_PaintStateRuntime
{
    static int GetNetworkHash(string paintCode)
    {
        if (paintCode == PaintZ_PaintConstants.PAINT_NONE)
            return 0;

        // Only registered finishes may be reconstructed from a network hash.
        // Historical/unresolved IDs remain server-side persistent state and
        // synchronize hash=0 so they can never alias an unrelated active finish.
        if (!PaintZ_PaintPackRegistry.HasFinish(paintCode))
            return 0;

        return paintCode.Hash();
    }

    static string GetNetworkPaintCode(int paintHash)
    {
        if (paintHash == 0)
            return PaintZ_PaintConstants.PAINT_NONE;

        return PaintZ_PaintPackRegistry.GetFinishIdByNetworkHash(paintHash);
    }

    static bool RestorePersistedVisual(EntityAI target, string paintCode, out int selectionIndex, out int scalePercent)
    {
        selectionIndex = -1;
        scalePercent = 100;
        if (!target || paintCode == PaintZ_PaintConstants.PAINT_NONE)
            return true;

        PaintZ_PaintInspectionResult inspection = PaintZ_PaintInspector.Inspect(target);
        if (!inspection.m_Paintable)
        {
            PaintZ_PaintLog.Warning("persistence target=" + target.GetType() + " finish=" + paintCode + " visual_restore_failed=" + inspection.m_Reason + " state_preserved=1");
            return false;
        }

        selectionIndex = inspection.m_SelectionIndex;
        if (!PaintZ_PaintPackRegistry.HasFinish(paintCode))
        {
            PaintZ_PaintVisuals.RestoreIfKnown(target, selectionIndex);
            PaintZ_PaintLog.Warning("persistence target=" + target.GetType() + " unresolved_finish=" + paintCode + " state_preserved=1 visual=original");
            return false;
        }

        float maxDimensionMeters;
        scalePercent = PaintZ_PatternScaling.ResolveScalePercent(target, paintCode, maxDimensionMeters);
        if (!PaintZ_PaintVisuals.Apply(target, paintCode, selectionIndex, scalePercent))
        {
            PaintZ_PaintVisuals.RestoreIfKnown(target, selectionIndex);
            PaintZ_PaintLog.Warning("persistence target=" + target.GetType() + " finish=" + paintCode + " visual_application_failed state_preserved=1");
            scalePercent = 100;
            return false;
        }

        return true;
    }
}
