from __future__ import annotations
from pathlib import Path
import json
from .ids import TYPE_CODES, normalize_hex, normalize_suffix


def load_manifest(path: Path) -> dict:
    data = json.loads(path.read_text(encoding="utf-8"))
    if data.get("schema_version") != 1:
        raise ValueError("Unsupported schema_version; expected 1")
    paints = data.get("paints")
    if not isinstance(paints, list) or not paints:
        raise ValueError("Manifest must contain a non-empty paints array")

    for i, p in enumerate(paints):
        if not isinstance(p, dict):
            raise ValueError(f"paints[{i}] must be an object")
        name = str(p.get("name", "")).strip()
        if not name:
            raise ValueError(f"paints[{i}].name is required")
        p["name"] = name

        typ = str(p.get("type", "")).casefold()
        if typ not in TYPE_CODES:
            allowed = ", ".join(sorted(TYPE_CODES))
            raise ValueError(f"Paint {name!r}: type must be one of: {allowed}")
        p["type"] = typ

        if "id" in p:
            p["id"] = normalize_suffix(str(p["id"]))

        has_color = bool(p.get("color"))
        has_pattern = bool(p.get("pattern"))
        if has_color and has_pattern:
            raise ValueError(f"Paint {name!r}: specify either 'color' or 'pattern', not both")
        if not has_color and not has_pattern:
            raise ValueError(f"Paint {name!r}: needs either 'color' or 'pattern' artwork data")
        if has_color:
            p["color"] = normalize_hex(p["color"])

        if "appearance_profile" in p:
            p["appearance_profile"] = str(p["appearance_profile"]).strip()
            if not p["appearance_profile"]:
                raise ValueError(f"Paint {name!r}: appearance_profile cannot be empty")

        if "dayz_class" in p:
            dayz_class = str(p["dayz_class"]).strip()
            if not dayz_class or not (dayz_class[0].isalpha() or dayz_class[0] == "_") or not all(c.isalnum() or c == "_" for c in dayz_class):
                raise ValueError(f"Paint {name!r}: dayz_class must be a valid config classname")
            p["dayz_class"] = dayz_class

    return data
