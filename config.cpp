class CfgPatches
{
    class PaintZ_DynamicPaint
    {
        units[] =
        {
            #include "tools\paintzgen\generated\dayz\PaintZ_Units.generated.inc"
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
                    "PaintZ/Scripts/4_World",
                    "PaintZ/tools/paintzgen/generated/dayz"
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

// Paint Pack API v1 discovery roots. Paint packs add child classes here.
class CfgPaintZPacks
{
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

    // Transitional bridge only. These generated official cans/finishes remain
    // in core until PaintZ-Standard-Pack is migrated to Paint Pack API v1.
    #include "tools\paintzgen\generated\dayz\PaintZ_Paints.generated.inc"

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
