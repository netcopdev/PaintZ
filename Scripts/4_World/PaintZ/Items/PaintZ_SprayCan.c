class PaintZ_SprayCanBase extends Spraycan_ColorBase
{
    void PaintZ_SprayCanBase()
    {
        Print("[PaintZ][Actions] Constructed spray can instance: " + GetType());
    }

    string GetPaintZPaintCode()
    {
        string paintCode;
        GetGame().ConfigGetText("CfgVehicles " + GetType() + " paintzCode", paintCode);
        return paintCode;
    }

    override void SetActions()
    {
        super.SetActions();

        // DayZ builds and shares this action map by script class, while all
        // generated config variants currently use PaintZ_SprayCanBase. Attach
        // every generated action to that shared map and let each action's
        // paint-code condition select the matching can at interaction time.
        PaintZ_PaintCatalog.AttachActionsToCan(this);
        AddAction(ActionPaintZCannotPaint);
        Print("[PaintZ][Actions] Attached paint actions to: " + GetType());
    }
};

class PaintZ_PaintStripperCan extends Spraycan_ColorBase
{
    override void SetActions()
    {
        super.SetActions();
        AddAction(ActionPaintZStripPaint);
    }
};
