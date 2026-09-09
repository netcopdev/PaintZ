modded class ActionConstructor
{
    override void RegisterActions(TTypenameArray actions)
    {
        super.RegisterActions(actions);
        actions.Insert(ActionPaintZPaint);
        actions.Insert(ActionPaintZStripPaint);
        actions.Insert(ActionPaintZCannotPaint);
        Print("[PaintZ][Actions] Registered generic Paint, Strip Paint, and Cannot Paint actions");
    }
};
