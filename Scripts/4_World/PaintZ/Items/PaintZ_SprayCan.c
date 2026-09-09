class PaintZ_SprayCanBase extends Spraycan_ColorBase
{
    void PaintZ_SprayCanBase()
    {
        Print("[PaintZ][Actions] Constructed spray can instance: " + GetType());
    }

    string GetPaintZPaintCode()
    {
        string path = "CfgVehicles " + GetType();
        string paintCode = "";

        if (GetGame().ConfigIsExisting(path + " paintzFinish"))
            GetGame().ConfigGetText(path + " paintzFinish", paintCode);

        if (paintCode == "" && GetGame().ConfigIsExisting(path + " paintzCode"))
            GetGame().ConfigGetText(path + " paintzCode", paintCode);

        paintCode.ToUpper();
        return paintCode;
    }

    override void SetActions()
    {
        super.SetActions();
        AddAction(ActionPaintZPaint);
        AddAction(ActionPaintZCannotPaint);
        Print("[PaintZ][Actions] Attached generic paint actions to: " + GetType());
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
