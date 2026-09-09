class PaintZ_SprayCanBase extends Spraycan_ColorBase
{
    void PaintZ_SprayCanBase()
    {
        Print("[PaintZ][Actions] Constructed spray can instance: " + GetType());
    }

    string GetPaintZPaintCode()
    {
        string paintCode;
        GetGame().ConfigGetText("CfgVehicles " + GetType() + " paintzFinish", paintCode);
        if (paintCode == "")
            GetGame().ConfigGetText("CfgVehicles " + GetType() + " paintzCode", paintCode);

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
