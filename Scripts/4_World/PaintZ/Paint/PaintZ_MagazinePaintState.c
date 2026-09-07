// Magazine is a native engine class in DayZ 1.29 (Magazine_Base is only a
// typedef), so it cannot carry modded script fields. Keep magazine application
// generic and operate on the existing instance. SetSynchDirty asks the engine
// to replicate the changed native object state; multiplayer visibility remains
// an explicit runtime acceptance check.
class PaintZ_MagazinePaintState
{
    static void SetPaint(Magazine magazine, string paintCode, int selectionIndex)
    {
        if (!magazine)
            return;

        PaintZ_PaintVisuals.Apply(magazine, paintCode, selectionIndex);
        magazine.SetSynchDirty();
    }
};
