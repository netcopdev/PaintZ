enum PaintZ_PersistenceReadResult
{
    PZ_PERSISTENCE_LEGACY,
    PZ_PERSISTENCE_VALID,
    PZ_PERSISTENCE_INVALID
}

class PaintZ_PaintPersistence
{
    protected static const string MARKER = "PaintZ.PaintState";
    protected static const int VERSION = 1;

    static void Save(ParamsWriteContext ctx, string paintCode)
    {
        TStringArray payload = new TStringArray();
        if (paintCode != PaintZ_PaintConstants.PAINT_NONE)
            payload.Insert(paintCode);

        ctx.Write(MARKER);
        ctx.Write(VERSION);
        ctx.Write(payload);
    }

    static PaintZ_PersistenceReadResult Load(ParamsReadContext ctx, EntityAI target, out string paintCode)
    {
        paintCode = PaintZ_PaintConstants.PAINT_NONE;

        // Entity streams with no remaining value are normal legacy saves from
        // before PaintZ persistence existed.
        if (!ctx.CanRead())
            return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_LEGACY;

        string marker;
        // CanRead may still report true for an empty ScriptReadWriteContext.
        // A failed first read is therefore also the normal legacy case. Once
        // the marker has been read, missing later values are malformed.
        if (!ctx.Read(marker))
            return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_LEGACY;

        if (marker != MARKER)
        {
            Warn(target, "malformed payload: unexpected marker");
            return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_INVALID;
        }

        int persistenceVersion;
        if (!ctx.Read(persistenceVersion))
        {
            Warn(target, "malformed payload: version could not be read");
            return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_INVALID;
        }

        TStringArray payload = new TStringArray();
        if (!ctx.Read(payload))
        {
            Warn(target, "malformed payload: state could not be read");
            return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_INVALID;
        }

        // The payload is one serializer value. Unsupported future versions can
        // therefore be consumed without leaving the entity stream misaligned.
        if (persistenceVersion != VERSION)
        {
            Warn(target, "unsupported persistence version=" + persistenceVersion);
            return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_INVALID;
        }

        if (payload.Count() == 0)
            return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_VALID;

        if (payload.Count() != 1 || payload.Get(0) == PaintZ_PaintConstants.PAINT_NONE)
        {
            Warn(target, "malformed version 1 state");
            return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_INVALID;
        }

        paintCode = payload.Get(0);
        return PaintZ_PersistenceReadResult.PZ_PERSISTENCE_VALID;
    }

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

    static bool RestorePersistedVisual(EntityAI target, string paintCode, out int selectionIndex)
    {
        selectionIndex = -1;
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

        selectionIndex = inspection.m_SelectionIndex;
        if (!PaintZ_PaintVisuals.Apply(target, paintCode, selectionIndex))
        {
            PaintZ_PaintLog.Warning("persistence target=" + target.GetType() + " finish=" + paintCode + " visual application failed");
            selectionIndex = -1;
            return false;
        }

        return true;
    }
}
