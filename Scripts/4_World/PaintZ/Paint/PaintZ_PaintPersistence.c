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
        // Absence of a PaintZ ModStorage entry is the canonical unpainted state.
        if (paintCode == PaintZ_PaintConstants.PAINT_NONE)
            return;

        CF_ModStorage ctx = storage[STORAGE_KEY];
        if (!ctx)
        {
            Warn(target, "CF ModStorage context is unavailable while saving");
            return;
        }

        // The finish ID is the entire persistent PaintZ state. Pattern scale,
        // derived texture path and selection index are intentionally transient.
        ctx.Write(paintCode);
    }

    static bool Load(CF_ModStorageMap storage, EntityAI target, out string paintCode)
    {
        paintCode = PaintZ_PaintConstants.PAINT_NONE;

        CF_ModStorage ctx = storage[STORAGE_KEY];
        if (!ctx)
            return true;

        // PaintZ persistence must never make the underlying DayZ item fail to
        // load. Bad/unsupported PaintZ data is isolated to PaintZ state: warn,
        // leave the item unpainted for this load, and keep the entity usable.
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

        return true;
    }

#ifdef DIAG_DEVELOPER
    // Sandbox codec adapter only. Production persistence never uses the flat
    // native stream; these overloads keep the existing isolated smoke test able
    // to test marker/version/unknown-ID handling without changing the CF design.
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

        return paintCode.Hash();
    }

    static string GetNetworkPaintCode(int paintHash)
    {
        if (paintHash == 0)
            return PaintZ_PaintConstants.PAINT_NONE;

        return PaintZ_PaintCatalog.GetPaintCodeByNetworkHash(paintHash);
    }

    static bool RestorePersistedVisual(EntityAI target, string paintCode, out int selectionIndex, out int scalePercent)
    {
        selectionIndex = -1;
        scalePercent = 100;

        if (!target || paintCode == PaintZ_PaintConstants.PAINT_NONE)
            return true;

        if (!PaintZ_PaintCatalog.HasPaintCode(paintCode))
        {
            PaintZ_PaintLog.Warning("persistence target=" + target.GetType() + " unknown_finish=" + paintCode + " state preserved");
            return false;
        }

        PaintZ_PaintInspectionResult inspection = PaintZ_PaintInspector.Inspect(target);
        if (!inspection.m_Paintable)
        {
            PaintZ_PaintLog.Warning("persistence target=" + target.GetType() + " finish=" + paintCode + " visual_restore_failed=" + inspection.m_Reason);
            return false;
        }

        float maxDimensionMeters;
        scalePercent = PaintZ_PatternScaling.ResolveScalePercent(target, paintCode, maxDimensionMeters);

        selectionIndex = inspection.m_SelectionIndex;
        if (!PaintZ_PaintVisuals.Apply(target, paintCode, selectionIndex, scalePercent))
        {
            PaintZ_PaintLog.Warning("persistence target=" + target.GetType() + " finish=" + paintCode + " visual application failed");
            selectionIndex = -1;
            scalePercent = 100;
            return false;
        }

        return true;
    }
}
