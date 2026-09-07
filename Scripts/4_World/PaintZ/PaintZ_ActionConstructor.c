modded class ActionConstructor
{
    override void RegisterActions(TTypenameArray actions)
    {
        super.RegisterActions(actions);
        PaintZ_PaintCatalog.RegisterActions(actions);
        actions.Insert(ActionPaintZStripPaint);
        actions.Insert(ActionPaintZCannotPaint);
        Print("[PaintZ][Actions] Registered generated paint catalogue, Strip Paint, and Cannot Paint");
    }
};
