from pathlib import Path
from tempfile import TemporaryDirectory
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

from paintzgen.dayz import emit_dayz
from paintzgen.ids import code_for_paint, code_to_slug
from paintzgen.manifest import load_manifest
from paintzgen.render import render_surface


catalog = [
    {
        "name": "Woodland",
        "type": "camo",
        "type_code": "C",
        "id": "WDL",
        "code": "PZ-C-WDL",
        "texture_stem": "pz_c_wdl",
        "dayz_class": "PaintZ_SprayCan_Woodland",
    },
    {
        "name": "Preview Only",
        "type": "solid",
        "type_code": "S",
        "id": "PRV",
        "code": "PZ-S-PRV",
        "texture_stem": "pz_s_prv",
        "dayz_class": None,
    },
]
settings = {
    "base_class": "PaintZ_SprayCanBase",
    "texture_root": r"PaintZ\data\cans",
}

with TemporaryDirectory() as directory:
    output = Path(directory)
    emit_dayz(catalog, settings, output)
    config = (output / "PaintZ_Paints.generated.inc").read_text(encoding="utf-8")
    types = (output / "types.generated.xml").read_text(encoding="utf-8")
    units = (output / "PaintZ_Units.generated.inc").read_text(encoding="utf-8")
    script = (output / "PaintZ_PaintCatalog.generated.c").read_text(encoding="utf-8")

assert "class PaintZ_SprayCan_Woodland: PaintZ_SprayCanBase" in config
assert r"PaintZ\data\cans\pz_c_wdl_co.paa" in config
assert "class PaintZ_SprayCan_PRV: PaintZ_SprayCanBase" in config
assert 'displayName = "PaintZ Preview Only"' in config
assert 'type name="PaintZ_SprayCan_Woodland"' in types
assert 'type name="PaintZ_SprayCan_PRV"' in types
assert '"PaintZ_SprayCan_Woodland",' in units
assert "actions.Insert(ActionPaintZPaint_C_WDL);" in script
assert "static void AttachActionsToCan(PaintZ_SprayCanBase sprayCan)" in script
assert "sprayCan.AddAction(ActionPaintZPaint_C_WDL);" in script
assert "sprayCan.AddAction(ActionPaintZPaint_S_PRV);" in script
assert 'return "Paint Woodland";' in script
assert "static bool HasPaintCode(string paintCode)" in script
assert 'if (paintCode == "PZ-C-WDL")' in script
assert "static string GetPaintCodeByNetworkHash(int paintHash)" in script
assert 'if ("PZ-C-WDL".Hash() == paintHash)' in script
assert "return ActionPaintZPaint_S_PRV;" in script
assert "typename noAction;" in script
assert "return noAction;" in script
assert "return null;" not in script

print("PaintZ DayZ export tests OK")

duplicate_class_catalog = [dict(catalog[0]), dict(catalog[1])]
duplicate_class_catalog[0]["dayz_class"] = "PaintZ_Duplicate"
duplicate_class_catalog[1]["dayz_class"] = "PaintZ_Duplicate"
with TemporaryDirectory() as directory:
    try:
        emit_dayz(duplicate_class_catalog, settings, Path(directory))
        raise AssertionError("duplicate generated DayZ classnames must be rejected")
    except ValueError as error:
        assert "Duplicate generated DayZ classname" in str(error)

print("PaintZ duplicate DayZ classname validation OK")

# The checked-in include must stay usable before the first local build.
manifest = load_manifest(ROOT / "paints.json")
actual_catalog = []
for paint in manifest["paints"]:
    code, _ = code_for_paint(paint)
    actual_catalog.append(
        {
            "name": paint["name"],
            "type": paint["type"],
            "type_code": code.split("-")[1],
            "id": code.split("-")[-1],
            "code": code,
            "texture_stem": code_to_slug(code),
            "dayz_class": paint.get("dayz_class"),
        }
    )

with TemporaryDirectory() as directory:
    output = Path(directory)
    emit_dayz(actual_catalog, manifest["dayz"], output)
    expected = (output / "PaintZ_Paints.generated.inc").read_text(encoding="utf-8").strip()
    expected_types = (output / "types.generated.xml").read_text(encoding="utf-8").strip()
    expected_units = (output / "PaintZ_Units.generated.inc").read_text(encoding="utf-8").strip()
    expected_script = (output / "PaintZ_PaintCatalog.generated.c").read_text(encoding="utf-8").strip()

checked_in = (ROOT / "generated/dayz/PaintZ_Paints.generated.inc").read_text(encoding="utf-8").strip()
checked_in_types = (ROOT / "generated/dayz/types.generated.xml").read_text(encoding="utf-8").strip()
checked_in_units = (ROOT / "generated/dayz/PaintZ_Units.generated.inc").read_text(encoding="utf-8").strip()
checked_in_script = (ROOT / "generated/dayz/PaintZ_PaintCatalog.generated.c").read_text(encoding="utf-8").strip()
assert checked_in == expected, "checked-in PaintZ config include is stale; regenerate paintzgen"
assert checked_in_types == expected_types, "checked-in PaintZ types XML is stale; regenerate paintzgen"
assert checked_in_units == expected_units, "checked-in PaintZ CfgPatches units include is stale; regenerate paintzgen"
assert checked_in_script == expected_script, "checked-in PaintZ runtime catalogue is stale; regenerate paintzgen"

print("PaintZ checked-in config is current")

rendered_solid = render_surface(
    {"name": "Flat Dark Earth", "type": "solid", "color": "#7B6647"},
    "PZ-S-TEST",
    ROOT,
    (32, 32),
)
assert rendered_solid.size == (32, 32)
assert rendered_solid.getextrema() != ((123, 123), (102, 102), (71, 71), (255, 255))

print("PaintZ finish surface rendering tests OK")
