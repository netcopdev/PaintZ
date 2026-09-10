class CfgPatches
{
    class PaintZ_PaintPackApiFixture
    {
        units[] =
        {
            "PaintZ_TestSprayCan_RED",
            "PaintZ_TestSprayCan_PAT"
        };
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] =
        {
            "PaintZ_DynamicPaint"
        };
    };
};

class CfgMods
{
    class PaintZ_PaintPackApiFixture
    {
        dir = "PaintZ_PaintPackApiFixture";
        name = "PaintZ Paint Pack API Fixture";
        author = "PaintZ contributors";
        version = "1";
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
                    "PaintZ_PaintPackApiFixture/Scripts/4_World"
                };
            };

            class missionScriptModule
            {
                value = "";
                files[] =
                {
                    "PaintZ_PaintPackApiFixture/Scripts/5_Mission"
                };
            };
        };
    };
};

class CfgPaintZPacks
{
    class TST_ApiFixture
    {
        apiVersion = 1;
        prefix = "TST";
        displayName = "PaintZ API Fixture";
    };

    // Deliberate namespace collision: neither DUP owner may win.
    class DUP_FirstOwner
    {
        apiVersion = 1;
        prefix = "DUP";
        displayName = "Duplicate Owner A";
    };

    class DUP_SecondOwner
    {
        apiVersion = 1;
        prefix = "DUP";
        displayName = "Duplicate Owner B";
    };

    // Deliberate invalid third-party claim on a reserved official namespace.
    class PZA_ThirdPartyClaim
    {
        apiVersion = 1;
        prefix = "PZA";
        displayName = "Invalid Reserved Prefix Claim";
    };

    // Deliberate unsupported API declaration.
    class AP2_UnsupportedApi
    {
        apiVersion = 2;
        prefix = "AP2";
        displayName = "Unsupported API Fixture";
    };
};

class CfgPaintZFinishes
{
    // Valid official-content contribution. The fixture deliberately does not
    // declare PZ; PaintZ core owns that namespace through PZ_PaintZOfficial.
    class PZ_API_S_API
    {
        id = "PZ-S-API";
        owner = "PZ_PaintZOfficial";
        displayName = "Official API Fixture";
        type = "solid";

        class Surfaces
        {
            class S100
            {
                scalePercent = 100;
                texture = "#(argb,8,8,3)color(0.2,0.4,0.8,1.0,CO)";
            };
        };
    };

    class TST_S_RED
    {
        id = "TST-S-RED";
        owner = "TST_ApiFixture";
        displayName = "Fixture Red";
        type = "solid";

        class Surfaces
        {
            class S100
            {
                scalePercent = 100;
                texture = "#(argb,8,8,3)color(1,0,0,1.0,CO)";
            };
        };
    };

    class TST_C_PAT
    {
        id = "TST-C-PAT";
        owner = "TST_ApiFixture";
        displayName = "Fixture Pattern";
        type = "camouflage";
        isPattern = 1;

        class Surfaces
        {
            class S050
            {
                scalePercent = 50;
                texture = "#(argb,8,8,3)color(0,1,0,1.0,CO)";
            };
            class S100
            {
                scalePercent = 100;
                texture = "#(argb,8,8,3)color(0,0,1,1.0,CO)";
            };
        };
    };

    // Deliberate owner mismatch inside an otherwise valid TST namespace.
    class TST_S_BADOWNER
    {
        id = "TST-S-BAD";
        owner = "DUP_FirstOwner";
        displayName = "Invalid Owner Fixture";
        type = "solid";

        class Surfaces
        {
            class S100
            {
                scalePercent = 100;
                texture = "#(argb,8,8,3)color(1,1,0,1.0,CO)";
            };
        };
    };

    // Deliberate duplicate complete finish ID: neither declaration may win.
    class TST_S_DUP_A
    {
        id = "TST-S-DUP";
        owner = "TST_ApiFixture";
        displayName = "Duplicate Finish A";
        type = "solid";

        class Surfaces
        {
            class S100
            {
                scalePercent = 100;
                texture = "#(argb,8,8,3)color(1,0,1,1.0,CO)";
            };
        };
    };

    class TST_S_DUP_B
    {
        id = "TST-S-DUP";
        owner = "TST_ApiFixture";
        displayName = "Duplicate Finish B";
        type = "solid";

        class Surfaces
        {
            class S100
            {
                scalePercent = 100;
                texture = "#(argb,8,8,3)color(0,1,1,1.0,CO)";
            };
        };
    };

    // These finishes belong to owners that must be rejected before finish registration.
    class DUP_S_RED
    {
        id = "DUP-S-RED";
        owner = "DUP_FirstOwner";
        displayName = "Conflicted Namespace Finish";
        type = "solid";

        class Surfaces
        {
            class S100
            {
                scalePercent = 100;
                texture = "#(argb,8,8,3)color(0.5,0,0,1.0,CO)";
            };
        };
    };

    class PZA_S_RED
    {
        id = "PZA-S-RED";
        owner = "PZA_ThirdPartyClaim";
        displayName = "Reserved Namespace Finish";
        type = "solid";

        class Surfaces
        {
            class S100
            {
                scalePercent = 100;
                texture = "#(argb,8,8,3)color(0.5,0.5,0,1.0,CO)";
            };
        };
    };

    class AP2_S_RED
    {
        id = "AP2-S-RED";
        owner = "AP2_UnsupportedApi";
        displayName = "Unsupported API Finish";
        type = "solid";

        class Surfaces
        {
            class S100
            {
                scalePercent = 100;
                texture = "#(argb,8,8,3)color(0.5,0,0.5,1.0,CO)";
            };
        };
    };
};

class CfgVehicles
{
    class PaintZ_SprayCanBase;

    class PaintZ_TestSprayCan_RED : PaintZ_SprayCanBase
    {
        scope = 2;
        displayName = "PaintZ API Fixture Red";
        descriptionShort = "TST-S-RED Paint Pack API fixture can";
        paintzFinish = "TST-S-RED";
        hiddenSelectionsTextures[] =
        {
            "#(argb,8,8,3)color(1,0,0,1.0,CO)"
        };
    };

    class PaintZ_TestSprayCan_PAT : PaintZ_SprayCanBase
    {
        scope = 2;
        displayName = "PaintZ API Fixture Pattern";
        descriptionShort = "TST-C-PAT Paint Pack API fixture can";
        paintzFinish = "TST-C-PAT";
        hiddenSelectionsTextures[] =
        {
            "#(argb,8,8,3)color(0,0,1,1.0,CO)"
        };
    };
};
