class PaintZ_PatternScaleRange
{
    float max_dimension_m = -1.0;
    float scale = -1.0;
};

class PaintZ_PatternScalingConfig
{
    int version;
    int reload_seconds = -999999;
    bool enabled = true;
    float default_scale = -999999.0;
    ref array<ref PaintZ_PatternScaleRange> ranges;

    void PaintZ_PatternScalingConfig()
    {
        ranges = new array<ref PaintZ_PatternScaleRange>;
    }
};
