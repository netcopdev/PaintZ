modded class Weapon_Base
{
    override void OnStoreSave(ParamsWriteContext ctx)
    {
        super.OnStoreSave(ctx);
        PaintZ_PaintPersistence.Save(ctx, PaintZ_GetPaintCode());
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version)
    {
        if (!super.OnStoreLoad(ctx, version))
            return false;

        string paintCode;
        PaintZ_PersistenceReadResult result = PaintZ_PaintPersistence.Load(ctx, this, paintCode);
        if (result == PaintZ_PersistenceReadResult.PZ_PERSISTENCE_VALID)
            PaintZ_LoadPaintState(paintCode);

        return true;
    }

    override void AfterStoreLoad()
    {
        super.AfterStoreLoad();
        PaintZ_RestoreLoadedPaint();
    }
}
