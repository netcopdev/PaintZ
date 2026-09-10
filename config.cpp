class CfgPatches
{
    class PaintZ_DynamicPaint
    {
        units[] =
        {
            "PaintZ_PaintStripperCan"
        };
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] =
        {
            "DZ_Data",
            "DZ_Gear_Consumables",
            "JM_CF_Scripts"
        };
    };
};

class CfgMods
{
    class PaintZ
    {
        dir = "PaintZ";
        name = "PaintZ";
        author = "PaintZ contributors";
        version = "0.0.1";
        storageVersion = 1;
        type = "mod";
        dependencies[] =
        {
            "World",
            "Mission"
        };

        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] =
                {
                    "PaintZ/Scripts/4_World"
                };
            };

            class missionScriptModule
            {
                value = "";
                files[] =
                {
                    "PaintZ/Scripts/5_Mission"
                };
            };
        };
    };
};

// Paint Pack API v1 discovery root.
// PaintZ owns the official PZ namespace identity but deliberately owns no
// individual official finishes. Independent official content packs register
// PZ-* finishes against this owner while depending only on PaintZ.
class CfgPaintZPacks
{
    class PZ_PaintZOfficial
    {
        apiVersion = 1;
        prefix = "PZ";
        displayName = "PaintZ Official";
        official = 1;
    };
};

class CfgPaintZFinishes
{
};

class CfgVehicles
{
    class Spraycan_ColorBase;

    class PaintZ_SprayCanBase : Spraycan_ColorBase
    {
        scope = 0;

        class AnimEvents
        {
            class SoundWeapon
            {
                class PaintZ_DisinfectantLoop
                {
                    soundSet = "disinfectant_loop_SoundSet";
                    id = 212;
                };
                class PaintZ_DisinfectantLoop2
                {
                    soundSet = "disinfectant_loop_SoundSet";
                    id = 213;
                };
            };
        };
    };

    class PaintZ_PaintStripperCan : Spraycan_ColorBase
    {
        scope = 2;
        displayName = "Paint Stripper";
        descriptionShort = "Removes PaintZ paint and restores the original finish. Paint spray cans cannot strip paint.";
        hiddenSelectionsTextures[] = {"#(argb,8,8,3)color(0.65,0.18,0.04,1.0,CO)"};

        class AnimEvents
        {
            class SoundWeapon
            {
                class PaintZ_DisinfectantLoop
                {
                    soundSet = "disinfectant_loop_SoundSet";
                    id = 212;
                };
                class PaintZ_DisinfectantLoop2
                {
                    soundSet = "disinfectant_loop_SoundSet";
                    id = 213;
                };
            };
        };
    };
};
