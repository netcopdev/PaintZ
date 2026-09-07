# ============================================================================
# PaintZ Design 3 SVG refactor
# Run from repository root:
#
#   E:\DayZDev\PaintZ
#
# This creates:
#
#   assets/templates/can_design3.svg
#   tools/paintzgen/tools/paintzgen/finish.py
#   tools/paintzgen/tools/paintzgen/svg_label.py
#   tools/paintzgen/tools/paintzgen/preview.py
#   tools/paintzgen/tools/paintzgen/render.py
#
# Existing generate_paints.py remains compatible.
# ============================================================================

$ErrorActionPreference = "Stop"

$root = (Get-Location).Path

$templateDir = Join-Path $root "assets\templates"
$fontDir = Join-Path $root "assets\fonts"
$moduleDir = Join-Path $root "tools\paintzgen\tools\paintzgen"

New-Item -ItemType Directory -Force -Path $templateDir | Out-Null
New-Item -ItemType Directory -Force -Path $moduleDir   | Out-Null


# ============================================================================
# assets/templates/can_design3.svg
#
# This is a REAL, valid SVG. Open it directly in Inkscape.
#
# Python changes:
#   - text
#   - contrast colors
#   - font sizes
#   - horizontal front-label width
#
# It does NOT bake the paint/camo into the SVG.
# ============================================================================

@'
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
    xmlns="http://www.w3.org/2000/svg"
    width="1024"
    height="2048"
    viewBox="0 0 1024 2048">

    <g
        id="design3"
        font-family="Barlow Condensed"
        text-anchor="middle">

        <!-- ================================================================
             PaintZ logo
             ================================================================ -->

        <text
            id="logo"
            x="512"
            y="465"
            font-size="150"
            font-weight="900"
            letter-spacing="-2"
            fill="#EFEEDC"
            stroke="#1D1F1B"
            stroke-width="3"
            stroke-opacity="0.28"
            stroke-linejoin="round"
            paint-order="stroke fill">
            Paint<tspan id="logo-z" fill="#992D26">Z</tspan>
        </text>


        <!-- ================================================================
             Paint identity
             ================================================================ -->

        <text
            id="paint-code"
            x="512"
            y="1050"
            font-size="68"
            font-weight="900"
            letter-spacing="2"
            fill="#EFEEDC"
            stroke="#1D1F1B"
            stroke-width="2"
            stroke-opacity="0.25"
            paint-order="stroke fill">
            PZ-C-UCP
        </text>

        <text
            id="paint-name"
            x="512"
            y="1160"
            font-size="58"
            font-weight="900"
            letter-spacing="1"
            fill="#EFEEDC"
            stroke="#1D1F1B"
            stroke-width="2"
            stroke-opacity="0.25"
            paint-order="stroke fill">
            UCP
        </text>


        <!-- ================================================================
             Divider
             ================================================================ -->

        <line
            id="divider"
            x1="390"
            y1="1278"
            x2="634"
            y2="1278"
            stroke="#EFEEDC"
            stroke-width="4"
            stroke-opacity="0.72"/>


        <!-- ================================================================
             Information badge
             ================================================================ -->

        <rect
            id="badge"
            x="324"
            y="1385"
            width="376"
            height="300"
            rx="18"
            ry="18"
            fill="#141513"
            fill-opacity="0.86"
            stroke="#E0DABF"
            stroke-width="3"
            stroke-opacity="0.62"/>

        <text
            id="series"
            x="512"
            y="1465"
            font-size="34"
            font-weight="900"
            letter-spacing="2"
            fill="#EFEEDC">
            CAMO SERIES
        </text>

        <text
            id="badge-line-2"
            x="512"
            y="1555"
            font-size="27"
            font-weight="600"
            letter-spacing="1.5"
            fill="#E0DABF">
            TACTICAL SURFACES
        </text>

        <text
            id="badge-line-3"
            x="512"
            y="1630"
            font-size="27"
            font-weight="600"
            letter-spacing="1.5"
            fill="#E0DABF">
            FIELD PROVEN
        </text>


        <!-- ================================================================
             Footer
             ================================================================ -->

        <text
            id="footer"
            x="512"
            y="1900"
            font-size="25"
            font-weight="600"
            letter-spacing="2"
            fill="#EFEEDC"
            stroke="#1D1F1B"
            stroke-width="1.5"
            stroke-opacity="0.22"
            paint-order="stroke fill">
            SPRAY • CUSTOMIZE • SURVIVE
        </text>

    </g>
</svg>
'@ | Set-Content `
    -Path (Join-Path $templateDir "can_design3.svg") `
    -Encoding utf8


# ============================================================================
# finish.py
#
# Paint/camo surface generation and deterministic wear.
# No label/layout code lives here.
# ============================================================================

@'
from __future__ import annotations

from functools import lru_cache
from pathlib import Path

import hashlib
import random

from PIL import Image


DEFAULT_APPEARANCE = {
    "default_profile": "used",
    "profiles": {
        "used": {
            "noise": 0.07,
            "scratches": 0.12,
            "grime": 0.08,
            "rust": 0.0,
            "edgewear": 0.08,
        }
    },
}


@lru_cache(maxsize=32)
def _load_overlay(path: str) -> Image.Image:
    with Image.open(path) as image:
        return image.convert("RGBA")


def _seed_for_label(code: str) -> int:
    digest = hashlib.sha256(
        f"paintz-finish-v1|{code}".encode("utf-8")
    ).digest()

    return int.from_bytes(
        digest[:8],
        "big",
    )


def _luminance(rgb: tuple[int, int, int]) -> float:
    return (
        0.2126 * rgb[0]
        + 0.7152 * rgb[1]
        + 0.0722 * rgb[2]
    ) / 255.0


def _resolve_profile(
    paint: dict,
    appearance_cfg: dict,
) -> tuple[str, dict]:

    profiles = appearance_cfg.get(
        "profiles",
        {},
    )

    default_name = appearance_cfg.get(
        "default_profile",
        "used",
    )

    name = (
        paint.get("appearance_profile")
        or default_name
    )

    if name not in profiles:
        raise ValueError(
            f"Unknown appearance profile "
            f"{name!r} for paint "
            f"{paint.get('name')!r}"
        )

    return name, profiles[name]


def _subtle_grain(
    image: Image.Image,
    seed: int,
    strength: float,
):
    rng = random.Random(seed)

    px = image.load()

    hits = max(
        1,
        int(
            image.width
            * image.height
            * (
                0.010
                + 0.014 * strength
            )
        ),
    )

    spread = max(
        1,
        int(
            18
            * max(
                0.25,
                strength,
            )
        ),
    )

    for _ in range(hits):
        x = rng.randrange(image.width)
        y = rng.randrange(image.height)

        r, g, b = px[x, y][:3]

        delta = rng.randint(
            -spread,
            spread,
        )

        px[x, y] = (
            max(
                0,
                min(
                    255,
                    r + delta,
                ),
            ),
            max(
                0,
                min(
                    255,
                    g + delta,
                ),
            ),
            max(
                0,
                min(
                    255,
                    b + delta,
                ),
            ),
            255,
        )


def _resize_overlay(
    path: Path,
    size: tuple[int, int],
) -> Image.Image:

    return _load_overlay(
        str(path)
    ).resize(
        size,
        Image.Resampling.LANCZOS,
    )


def _alpha_scaled(
    image: Image.Image,
    factor: float,
) -> Image.Image:

    if factor <= 0:
        return Image.new(
            "RGBA",
            image.size,
            (
                0,
                0,
                0,
                0,
            ),
        )

    output = image.copy()

    r, g, b, a = output.split()

    a = a.point(
        lambda value: max(
            0,
            min(
                255,
                int(
                    value * factor
                ),
            ),
        )
    )

    output.putalpha(a)

    return output


def pattern_fill(
    path: Path,
    size: tuple[int, int],
) -> Image.Image:

    with Image.open(path) as source:
        source = source.convert("RGB")

        target_width, target_height = size

        ratio = max(
            target_width / source.width,
            target_height / source.height,
        )

        new_width = max(
            1,
            int(
                source.width * ratio
            ),
        )

        new_height = max(
            1,
            int(
                source.height * ratio
            ),
        )

        source = source.resize(
            (
                new_width,
                new_height,
            ),
            Image.Resampling.LANCZOS,
        )

        left = (
            new_width
            - target_width
        ) // 2

        top = (
            new_height
            - target_height
        ) // 2

        return source.crop(
            (
                left,
                top,
                left + target_width,
                top + target_height,
            )
        ).convert("RGBA")


def _apply_finish_stack(
    surface: Image.Image,
    code: str,
    repo_root: Path,
    profile: dict,
):
    seed = _seed_for_label(code)

    rng = random.Random(seed)

    size = surface.size

    average = (
        surface
        .resize(
            (
                1,
                1,
            ),
            Image.Resampling.BOX,
        )
        .convert("RGB")
        .getpixel(
            (
                0,
                0,
            )
        )
    )

    lum = _luminance(average)

    _subtle_grain(
        surface,
        seed ^ 0xA91F,
        float(
            profile.get(
                "noise",
                0.07,
            )
        ),
    )

    overlay_dir = (
        repo_root
        / "assets"
        / "overlays"
    )

    # ------------------------------------------------------------------
    # Grime
    # ------------------------------------------------------------------

    grime_path = (
        overlay_dir
        / f"grime_{rng.choice([1, 2]):02d}.png"
    )

    grime = _resize_overlay(
        grime_path,
        size,
    )

    grime_factor = float(
        profile.get(
            "grime",
            0.08,
        )
    )

    if lum > 0.72:
        grime_factor *= 1.10

    surface.alpha_composite(
        _alpha_scaled(
            grime,
            grime_factor
            * (
                0.80
                + 0.40
                * rng.random()
            ),
        )
    )

    # ------------------------------------------------------------------
    # Scratches / scuffs
    # ------------------------------------------------------------------

    scratches_path = (
        overlay_dir
        / f"scratches_{rng.choice([1, 2, 3]):02d}.png"
    )

    scratches = _resize_overlay(
        scratches_path,
        size,
    )

    scratch_factor = float(
        profile.get(
            "scratches",
            0.12,
        )
    )

    if lum < 0.35:
        scratch_factor *= 1.18

    surface.alpha_composite(
        _alpha_scaled(
            scratches,
            scratch_factor
            * (
                0.80
                + 0.35
                * rng.random()
            ),
        )
    )

    # ------------------------------------------------------------------
    # Rust
    # ------------------------------------------------------------------

    rust_amount = float(
        profile.get(
            "rust",
            0.0,
        )
    )

    if rust_amount > 0:
        rust_path = (
            overlay_dir
            / f"rust_{rng.choice([1, 2]):02d}.png"
        )

        rust = _resize_overlay(
            rust_path,
            size,
        )

        surface.alpha_composite(
            _alpha_scaled(
                rust,
                rust_amount
                * (
                    0.85
                    + 0.30
                    * rng.random()
                ),
            )
        )

    # ------------------------------------------------------------------
    # Edge wear
    # ------------------------------------------------------------------

    edge_path = (
        overlay_dir
        / "edgewear_01.png"
    )

    edge = _resize_overlay(
        edge_path,
        size,
    )

    edge_factor = float(
        profile.get(
            "edgewear",
            0.08,
        )
    )

    if lum < 0.45:
        edge_factor *= 1.10
    else:
        edge_factor *= 0.95

    surface.alpha_composite(
        _alpha_scaled(
            edge,
            edge_factor,
        )
    )


def create_base(
    size=(1024, 2048),
) -> Image.Image:

    return Image.new(
        "RGBA",
        size,
        (
            0,
            0,
            0,
            0,
        ),
    )


def render_surface(
    paint: dict,
    code: str,
    repo_root: Path,
    size: tuple[int, int],
    appearance_cfg: dict | None = None,
) -> Image.Image:

    appearance_cfg = (
        appearance_cfg
        or DEFAULT_APPEARANCE
    )

    if paint.get("color"):
        value = (
            paint["color"]
            .lstrip("#")
        )

        rgb = tuple(
            int(
                value[i:i + 2],
                16,
            )
            for i in (
                0,
                2,
                4,
            )
        )

        surface = Image.new(
            "RGBA",
            size,
            rgb + (255,),
        )

    else:
        pattern_path = (
            repo_root
            / paint["pattern"]
        ).resolve()

        surface = pattern_fill(
            pattern_path,
            size,
        )

    profile_name, profile = _resolve_profile(
        paint,
        appearance_cfg,
    )

    _apply_finish_stack(
        surface,
        code,
        repo_root,
        profile,
    )

    surface.info[
        "appearance_profile"
    ] = profile_name

    return surface
'@ | Set-Content `
    -Path (Join-Path $moduleDir "finish.py") `
    -Encoding utf8


# ============================================================================
# svg_label.py
#
# The ONLY normal design-tuning value is FRONT_WIDTH.
#
# It is the fraction of the full can circumference occupied by the front
# printed artwork.
#
#   0.35 = narrower
#   0.40 = recommended
#   0.45 = wider
#
# Camo/paint still covers 100% of the can.
# ============================================================================

@'
from __future__ import annotations

from io import BytesIO
from pathlib import Path

import os
import xml.etree.ElementTree as ET

from PIL import Image, ImageFont

from .finish import DEFAULT_APPEARANCE, render_surface


# ===========================================================================
# ONE NORMAL LABEL-WIDTH CONTROL
# ===========================================================================

FRONT_WIDTH = 0.40


# SVG design coordinate system.
DESIGN_WIDTH = 1024
DESIGN_HEIGHT = 2048

DARK_HEX = "#1D1F1B"
LIGHT_HEX = "#EFEEDC"
CREAM_HEX = "#E0DABF"
RED_HEX = "#992D26"


SVG_NS = "http://www.w3.org/2000/svg"

ET.register_namespace(
    "",
    SVG_NS,
)


def _candidate_roots(
    repo_root: Path | None = None,
):
    seen = set()

    if repo_root is not None:
        resolved = Path(repo_root).resolve()

        if resolved not in seen:
            seen.add(resolved)
            yield resolved

    cwd = Path.cwd().resolve()

    if cwd not in seen:
        seen.add(cwd)
        yield cwd

    here = Path(__file__).resolve()

    for parent in here.parents:
        if parent not in seen:
            seen.add(parent)
            yield parent


def _find_asset(
    relative: Path,
    repo_root: Path | None = None,
) -> Path | None:

    for root in _candidate_roots(
        repo_root
    ):
        candidate = (
            root
            / relative
        )

        if candidate.exists():
            return candidate

    return None


def find_font(
    repo_root: Path | None = None,
) -> Path | None:
    """
    Backward-compatible display font lookup.
    generate_paints.py already imports this name.
    """

    env = (
        os.environ.get("PAINTZ_FONT")
        or os.environ.get(
            "PAINTZ_FONT_DISPLAY"
        )
    )

    if env:
        path = Path(env)

        if path.exists():
            return path

    bundled = _find_asset(
        Path(
            "assets/fonts/"
            "BarlowCondensed-Black.ttf"
        ),
        repo_root,
    )

    if bundled:
        return bundled

    fallbacks = [
        Path(
            "C:/Windows/Fonts/"
            "arialbd.ttf"
        ),
        Path(
            "/usr/share/fonts/truetype/"
            "dejavu/DejaVuSansCondensed-Bold.ttf"
        ),
    ]

    for path in fallbacks:
        if path.exists():
            return path

    return None


def find_text_font(
    repo_root: Path | None = None,
) -> Path | None:

    env = os.environ.get(
        "PAINTZ_FONT_TEXT"
    )

    if env:
        path = Path(env)

        if path.exists():
            return path

    bundled = _find_asset(
        Path(
            "assets/fonts/"
            "BarlowCondensed-SemiBold.ttf"
        ),
        repo_root,
    )

    if bundled:
        return bundled

    return find_font(
        repo_root
    )


def _require_fonts(
    repo_root: Path,
) -> tuple[Path, Path]:

    display = find_font(
        repo_root
    )

    text = find_text_font(
        repo_root
    )

    if not display:
        raise FileNotFoundError(
            "PaintZ display font not found. "
            "Expected: "
            "assets/fonts/"
            "BarlowCondensed-Black.ttf"
        )

    if not text:
        raise FileNotFoundError(
            "PaintZ text font not found. "
            "Expected: "
            "assets/fonts/"
            "BarlowCondensed-SemiBold.ttf"
        )

    return display, text


def _require_template(
    repo_root: Path,
) -> Path:

    template = _find_asset(
        Path(
            "assets/templates/"
            "can_design3.svg"
        ),
        repo_root,
    )

    if not template:
        raise FileNotFoundError(
            "PaintZ SVG template not found: "
            "assets/templates/"
            "can_design3.svg"
        )

    return template


def _fit_font_size(
    text: str,
    font_path: Path,
    max_width: float,
    start_size: int,
    min_size: int,
) -> int:
    """
    Reduce font size until the text fits.
    Never horizontally stretch text.
    """

    for size in range(
        int(start_size),
        int(min_size) - 1,
        -1,
    ):
        font = ImageFont.truetype(
            str(font_path),
            size=size,
        )

        box = font.getbbox(text)

        width = (
            box[2]
            - box[0]
        )

        if width <= max_width:
            return size

    return int(min_size)


def _sample_front_luminance(
    surface: Image.Image,
) -> float:
    """
    Sample only the central printed area rather than the entire circumference.
    """

    width, height = surface.size

    front_px = int(
        width * FRONT_WIDTH
    )

    x1 = (
        width
        - front_px
    ) // 2

    x2 = (
        x1
        + front_px
    )

    y1 = int(
        height * 0.14
    )

    y2 = int(
        height * 0.64
    )

    sample = (
        surface
        .crop(
            (
                x1,
                y1,
                x2,
                y2,
            )
        )
        .resize(
            (
                1,
                1,
            ),
            Image.Resampling.BOX,
        )
        .convert("RGB")
        .getpixel(
            (
                0,
                0,
            )
        )
    )

    return (
        0.2126 * sample[0]
        + 0.7152 * sample[1]
        + 0.0722 * sample[2]
    ) / 255.0


def _elements_by_id(
    root: ET.Element,
) -> dict[str, ET.Element]:

    result = {}

    for element in root.iter():
        element_id = element.get(
            "id"
        )

        if element_id:
            result[
                element_id
            ] = element

    return result


def _set_text(
    element: ET.Element,
    value: str,
):
    """
    Replace ordinary text content while preserving child tspans only where
    specifically needed elsewhere.
    """

    for child in list(element):
        element.remove(child)

    element.text = value


def _prepare_svg(
    surface: Image.Image,
    paint: dict,
    code: str,
    repo_root: Path,
) -> tuple[str, list[str]]:

    template_path = _require_template(
        repo_root
    )

    display_font, text_font = _require_fonts(
        repo_root
    )

    tree = ET.parse(
        template_path
    )

    root = tree.getroot()

    elements = _elements_by_id(
        root
    )

    required_ids = {
        "logo",
        "paint-code",
        "paint-name",
        "divider",
        "badge",
        "series",
        "badge-line-2",
        "badge-line-3",
        "footer",
    }

    missing = (
        required_ids
        - set(
            elements.keys()
        )
    )

    if missing:
        raise ValueError(
            "Design 3 SVG missing elements: "
            + ", ".join(
                sorted(missing)
            )
        )

    # ------------------------------------------------------------------
    # Dynamic text
    # ------------------------------------------------------------------

    name = str(
        paint["name"]
    ).upper()

    code_text = str(
        code
    ).upper()

    paint_type = str(
        paint.get(
            "type",
            "",
        )
    ).lower()

    if paint_type == "camo":
        series = "CAMO SERIES"

    elif paint.get("pattern"):
        series = "PATTERN SERIES"

    else:
        series = "SOLID SERIES"

    badge_line_2 = str(
        paint.get(
            "badge_text"
        )
        or "TACTICAL SURFACES"
    ).upper()

    badge_line_3 = str(
        paint.get(
            "field_text"
        )
        or "FIELD PROVEN"
    ).upper()

    footer = str(
        paint.get(
            "footer_text"
        )
        or "SPRAY • CUSTOMIZE • SURVIVE"
    ).upper()

    _set_text(
        elements["paint-code"],
        code_text,
    )

    _set_text(
        elements["paint-name"],
        name,
    )

    _set_text(
        elements["series"],
        series,
    )

    _set_text(
        elements["badge-line-2"],
        badge_line_2,
    )

    _set_text(
        elements["badge-line-3"],
        badge_line_3,
    )

    _set_text(
        elements["footer"],
        footer,
    )

    # ------------------------------------------------------------------
    # Dynamic logo, preserving red Z
    # ------------------------------------------------------------------

    logo = elements["logo"]

    for child in list(
        logo
    ):
        logo.remove(child)

    logo.text = "Paint"

    z = ET.SubElement(
        logo,
        f"{{{SVG_NS}}}tspan",
        {
            "fill": RED_HEX,
        },
    )

    z.text = "Z"

    # ------------------------------------------------------------------
    # Front-face width
    # ------------------------------------------------------------------

    front_width = (
        DESIGN_WIDTH
        * FRONT_WIDTH
    )

    center_x = (
        DESIGN_WIDTH
        / 2
    )

    badge_width = (
        front_width
        * 0.92
    )

    badge_x = (
        center_x
        - badge_width
        / 2
    )

    elements[
        "badge"
    ].set(
        "x",
        f"{badge_x:.2f}",
    )

    elements[
        "badge"
    ].set(
        "width",
        f"{badge_width:.2f}",
    )

    divider_width = (
        front_width
        * 0.60
    )

    elements[
        "divider"
    ].set(
        "x1",
        f"{center_x - divider_width / 2:.2f}",
    )

    elements[
        "divider"
    ].set(
        "x2",
        f"{center_x + divider_width / 2:.2f}",
    )

    # ------------------------------------------------------------------
    # Font sizing
    #
    # Actual font measurement is used. No horizontal stretching.
    # ------------------------------------------------------------------

    logo_size = _fit_font_size(
        "PaintZ",
        display_font,
        front_width * 0.92,
        int(
            front_width * 0.38
        ),
        70,
    )

    code_size = _fit_font_size(
        code_text,
        display_font,
        front_width * 0.92,
        int(
            front_width * 0.18
        ),
        32,
    )

    name_size = _fit_font_size(
        name,
        display_font,
        front_width * 0.94,
        int(
            front_width * 0.15
        ),
        28,
    )

    series_size = _fit_font_size(
        series,
        display_font,
        badge_width * 0.82,
        int(
            front_width * 0.085
        ),
        20,
    )

    badge_2_size = _fit_font_size(
        badge_line_2,
        text_font,
        badge_width * 0.84,
        int(
            front_width * 0.067
        ),
        18,
    )

    badge_3_size = _fit_font_size(
        badge_line_3,
        text_font,
        badge_width * 0.84,
        int(
            front_width * 0.067
        ),
        18,
    )

    footer_size = _fit_font_size(
        footer,
        text_font,
        front_width * 0.96,
        int(
            front_width * 0.064
        ),
        15,
    )

    elements[
        "logo"
    ].set(
        "font-size",
        str(logo_size),
    )

    elements[
        "paint-code"
    ].set(
        "font-size",
        str(code_size),
    )

    elements[
        "paint-name"
    ].set(
        "font-size",
        str(name_size),
    )

    elements[
        "series"
    ].set(
        "font-size",
        str(series_size),
    )

    elements[
        "badge-line-2"
    ].set(
        "font-size",
        str(badge_2_size),
    )

    elements[
        "badge-line-3"
    ].set(
        "font-size",
        str(badge_3_size),
    )

    elements[
        "footer"
    ].set(
        "font-size",
        str(footer_size),
    )

    # ------------------------------------------------------------------
    # Contrast
    # ------------------------------------------------------------------

    luminance = _sample_front_luminance(
        surface
    )

    if luminance >= 0.50:
        ink = DARK_HEX
        halo = LIGHT_HEX

    else:
        ink = LIGHT_HEX
        halo = DARK_HEX

    for element_id in (
        "logo",
        "paint-code",
        "paint-name",
        "footer",
    ):
        element = elements[
            element_id
        ]

        element.set(
            "fill",
            ink,
        )

        element.set(
            "stroke",
            halo,
        )

    elements[
        "divider"
    ].set(
        "stroke",
        ink,
    )

    # Badge is deliberately stable and always readable.
    elements[
        "series"
    ].set(
        "fill",
        LIGHT_HEX,
    )

    elements[
        "badge-line-2"
    ].set(
        "fill",
        CREAM_HEX,
    )

    elements[
        "badge-line-3"
    ].set(
        "fill",
        CREAM_HEX,
    )

    svg_string = ET.tostring(
        root,
        encoding="unicode",
    )

    return (
        svg_string,
        [
            str(display_font),
            str(text_font),
        ],
    )


def _rasterize_svg(
    svg_string: str,
    font_files: list[str],
    width: int,
    height: int,
) -> Image.Image:

    try:
        import resvg_py

    except ImportError as exc:
        raise RuntimeError(
            "PaintZ SVG renderer requires resvg_py. "
            "Install it with: "
            "python -m pip install resvg_py"
        ) from exc

    png_bytes = resvg_py.svg_to_bytes(
        svg_string=svg_string,
        width=width,
        height=height,
        skip_system_fonts=True,
        font_files=font_files,
        shape_rendering="geometric_precision",
        text_rendering="geometric_precision",
        image_rendering="optimize_quality",
    )

    with Image.open(
        BytesIO(
            png_bytes
        )
    ) as image:
        return image.convert(
            "RGBA"
        )


def render_label(
    base: Image.Image,
    paint: dict,
    code: str,
    repo_root: Path,
    appearance_cfg: dict | None = None,
) -> Image.Image:
    """
    Build Design 3:

        full paint/camo coating
              +
        dynamic SVG label
              =
        finished spray-can texture
    """

    appearance_cfg = (
        appearance_cfg
        or DEFAULT_APPEARANCE
    )

    width, height = base.size

    surface = render_surface(
        paint=paint,
        code=code,
        repo_root=repo_root,
        size=(
            width,
            height,
        ),
        appearance_cfg=appearance_cfg,
    )

    svg_string, fonts = _prepare_svg(
        surface=surface,
        paint=paint,
        code=code,
        repo_root=repo_root,
    )

    overlay = _rasterize_svg(
        svg_string=svg_string,
        font_files=fonts,
        width=width,
        height=height,
    )

    surface.alpha_composite(
        overlay
    )

    return surface
'@ | Set-Content `
    -Path (Join-Path $moduleDir "svg_label.py") `
    -Encoding utf8


# ============================================================================
# preview.py
#
# Preview generation is deliberately separate from the actual DayZ texture.
#
# It shows only the central visible part of the circumference instead of
# crushing the full 360-degree texture onto the front of the fake can.
# ============================================================================

@'
from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

from .svg_label import find_font


VISIBLE_CIRCUMFERENCE = 0.50


def _preview_font(
    size: int,
):
    path = find_font()

    if path:
        return ImageFont.truetype(
            str(path),
            size=size,
        )

    return ImageFont.load_default()


def _center_text(
    draw: ImageDraw.ImageDraw,
    center_x: float,
    y: float,
    text: str,
    font,
    fill,
):
    box = draw.textbbox(
        (
            0,
            0,
        ),
        text,
        font=font,
    )

    width = (
        box[2]
        - box[0]
    )

    draw.text(
        (
            center_x
            - width / 2,
            y,
        ),
        text,
        font=font,
        fill=fill,
    )


def _visible_front(
    label: Image.Image,
) -> Image.Image:
    """
    Extract roughly the central visible 180 degrees of the can.

    This is intentionally simple. It is only a catalogue preview, not part
    of the generated DayZ texture.
    """

    width, height = label.size

    crop_width = int(
        width
        * VISIBLE_CIRCUMFERENCE
    )

    x1 = (
        width
        - crop_width
    ) // 2

    return label.crop(
        (
            x1,
            0,
            x1 + crop_width,
            height,
        )
    )


def save_preview(
    label: Image.Image,
    out: Path,
):
    canvas_width = 900
    canvas_height = 1250

    canvas = Image.new(
        "RGB",
        (
            canvas_width,
            canvas_height,
        ),
        (
            224,
            224,
            216,
        ),
    )

    draw = ImageDraw.Draw(
        canvas
    )

    # Shadow
    draw.ellipse(
        (
            225,
            1110,
            675,
            1185,
        ),
        fill=(
            180,
            180,
            172,
        ),
    )

    # Can body
    body_x1 = 250
    body_y1 = 160
    body_x2 = 650
    body_y2 = 1080

    body_width = (
        body_x2
        - body_x1
    )

    body_height = (
        body_y2
        - body_y1
    )

    front = _visible_front(
        label
    ).resize(
        (
            body_width,
            body_height,
        ),
        Image.Resampling.LANCZOS,
    ).convert(
        "RGB"
    )

    mask = Image.new(
        "L",
        (
            body_width,
            body_height,
        ),
        0,
    )

    mask_draw = ImageDraw.Draw(
        mask
    )

    mask_draw.rounded_rectangle(
        (
            0,
            0,
            body_width - 1,
            body_height - 1,
        ),
        radius=42,
        fill=255,
    )

    canvas.paste(
        front,
        (
            body_x1,
            body_y1,
        ),
        mask,
    )

    # Cylinder edge shading
    shade = Image.new(
        "RGBA",
        (
            body_width,
            body_height,
        ),
        (
            0,
            0,
            0,
            0,
        ),
    )

    pixels = shade.load()

    for x in range(
        body_width
    ):
        normalized = abs(
            (
                x
                / max(
                    1,
                    body_width - 1,
                )
            )
            * 2.0
            - 1.0
        )

        alpha = int(
            90
            * (
                normalized ** 2.3
            )
        )

        for y in range(
            body_height
        ):
            pixels[
                x,
                y,
            ] = (
                0,
                0,
                0,
                alpha,
            )

    rgba = canvas.convert(
        "RGBA"
    )

    rgba.alpha_composite(
        shade,
        (
            body_x1,
            body_y1,
        ),
    )

    canvas = rgba.convert(
        "RGB"
    )

    draw = ImageDraw.Draw(
        canvas
    )

    # Body outline
    draw.rounded_rectangle(
        (
            body_x1,
            body_y1,
            body_x2,
            body_y2,
        ),
        radius=42,
        outline=(
            34,
            34,
            31,
        ),
        width=5,
    )

    # Lower rim
    draw.line(
        (
            body_x1 + 12,
            body_y2 - 20,
            body_x2 - 12,
            body_y2 - 20,
        ),
        fill=(
            38,
            38,
            35,
        ),
        width=6,
    )

    # Metal top
    draw.rectangle(
        (
            278,
            115,
            622,
            205,
        ),
        fill=(
            68,
            69,
            64,
        ),
        outline=(
            30,
            30,
            29,
        ),
        width=4,
    )

    draw.ellipse(
        (
            278,
            95,
            622,
            150,
        ),
        fill=(
            105,
            106,
            101,
        ),
        outline=(
            35,
            35,
            32,
        ),
        width=4,
    )

    # Nozzle
    draw.rounded_rectangle(
        (
            365,
            45,
            535,
            125,
        ),
        radius=20,
        fill=(
            25,
            25,
            24,
        ),
        outline=(
            15,
            15,
            15,
        ),
        width=3,
    )

    canvas.save(
        out
    )


def save_preview_catalog(
    items: list[
        tuple[
            dict,
            str,
            Path,
        ]
    ],
    out: Path,
    columns: int = 4,
):
    if not items:
        return

    thumb_width = 360
    thumb_height = 500

    cell_width = 420
    cell_height = 610

    margin_x = 50
    margin_y = 42

    header_height = 145

    rows = (
        len(items)
        + columns
        - 1
    ) // columns

    width = (
        margin_x * 2
        + columns
        * cell_width
    )

    height = (
        header_height
        + margin_y
        + rows
        * cell_height
        + margin_y
    )

    canvas = Image.new(
        "RGB",
        (
            width,
            height,
        ),
        (
            232,
            232,
            224,
        ),
    )

    draw = ImageDraw.Draw(
        canvas
    )

    draw.text(
        (
            margin_x,
            28,
        ),
        "PaintZ — Design 3",
        font=_preview_font(
            52
        ),
        fill=(
            29,
            31,
            27,
        ),
    )

    draw.text(
        (
            margin_x,
            88,
        ),
        "Generated paint catalogue",
        font=_preview_font(
            27
        ),
        fill=(
            83,
            82,
            55,
        ),
    )

    for index, (
        paint,
        code,
        preview_path,
    ) in enumerate(
        items
    ):
        column = (
            index
            % columns
        )

        row = (
            index
            // columns
        )

        x0 = (
            margin_x
            + column
            * cell_width
        )

        y0 = (
            header_height
            + margin_y
            + row
            * cell_height
        )

        with Image.open(
            preview_path
        ) as image:
            image = image.convert(
                "RGB"
            )

            image.thumbnail(
                (
                    thumb_width,
                    thumb_height,
                ),
                Image.Resampling.LANCZOS,
            )

            paste_x = (
                x0
                + (
                    cell_width
                    - image.width
                )
                // 2
            )

            canvas.paste(
                image,
                (
                    paste_x,
                    y0,
                ),
            )

        name = str(
            paint["name"]
        ).upper()

        _center_text(
            draw,
            x0
            + cell_width
            / 2,
            y0 + 505,
            name,
            _preview_font(
                27
            ),
            (
                29,
                31,
                27,
            ),
        )

        _center_text(
            draw,
            x0
            + cell_width
            / 2,
            y0 + 548,
            code,
            _preview_font(
                20
            ),
            (
                83,
                82,
                55,
            ),
        )

    canvas.save(
        out
    )
'@ | Set-Content `
    -Path (Join-Path $moduleDir "preview.py") `
    -Encoding utf8


# ============================================================================
# render.py
#
# Compatibility facade.
#
# Existing generate_paints.py currently imports:
#
#   create_base
#   render_label
#   render_surface
#   save_preview
#   save_preview_catalog
#   find_font
#
# So it does not need to change at all.
# ============================================================================

@'
from __future__ import annotations

from .finish import (
    create_base,
    pattern_fill,
    render_surface,
)

from .svg_label import (
    FRONT_WIDTH,
    find_font,
    find_text_font,
    render_label,
)

from .preview import (
    save_preview,
    save_preview_catalog,
)


__all__ = [
    "FRONT_WIDTH",
    "create_base",
    "find_font",
    "find_text_font",
    "pattern_fill",
    "render_label",
    "render_surface",
    "save_preview",
    "save_preview_catalog",
]
'@ | Set-Content `
    -Path (Join-Path $moduleDir "render.py") `
    -Encoding utf8


# ============================================================================
# Dependency
# ============================================================================

Write-Host ""
Write-Host "Checking resvg_py..."

python -c "import resvg_py" 2>$null

if ($LASTEXITCODE -ne 0) {
    Write-Host "Installing resvg_py 0.5.0..."

    python -m pip install "resvg_py==0.5.0"

    if ($LASTEXITCODE -ne 0) {
        throw "Could not install resvg_py."
    }
}
else {
    Write-Host "resvg_py already installed."
}


# ============================================================================
# Font check
# ============================================================================

$blackFont = Join-Path $fontDir "BarlowCondensed-Black.ttf"
$textFont = Join-Path $fontDir "BarlowCondensed-SemiBold.ttf"

Write-Host ""

if (-not (Test-Path $blackFont)) {
    Write-Warning "Missing: assets\fonts\BarlowCondensed-Black.ttf"
}

if (-not (Test-Path $textFont)) {
    Write-Warning "Missing: assets\fonts\BarlowCondensed-SemiBold.ttf"
}


# ============================================================================
# Finished
# ============================================================================

Write-Host ""
Write-Host "PaintZ Design 3 SVG refactor complete."
Write-Host ""
Write-Host "Editable design:"
Write-Host "  assets\templates\can_design3.svg"
Write-Host ""
Write-Host "Front-label width:"
Write-Host "  tools\paintzgen\tools\paintzgen\svg_label.py"
Write-Host "  FRONT_WIDTH = 0.40"
Write-Host ""
Write-Host "Now run:"
Write-Host "  .\tools\generate-paints.ps1"