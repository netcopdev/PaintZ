from __future__ import annotations

from io import BytesIO
from pathlib import Path

import os
import xml.etree.ElementTree as ET

from PIL import Image, ImageFont

from .finish import DEFAULT_APPEARANCE, render_surface


# ===========================================================================
# PaintZ Design 3
# ===========================================================================
#
# Fraction of the full can circumference occupied by the printed front label.
#
# 0.35 = narrower
# 0.40 = recommended
# 0.45 = wider
#
# This affects ONLY:
#   - logo
#   - paint ID
#   - paint name
#   - divider
#   - badge
#   - footer
#
# The paint/camouflage itself still covers the complete texture.
# ===========================================================================

FRONT_WIDTH = 0.40


# Normalized SVG coordinate space.
#
# This is NOT the output texture resolution.
# The SVG is designed in a simple 1000 × 1000 square coordinate system and
# then rasterized to the actual texture size, e.g. 1254 × 1254.
VIEWBOX_SIZE = 1000.0


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
    Backward-compatible display-font lookup.

    generate_paints.py imports this function through paintzgen.render.
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
    Fit text inside the normalized SVG design space.

    No horizontal font stretching is used.
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

        box = font.getbbox(
            text
        )

        width = (
            box[2]
            - box[0]
        )

        if width <= max_width:
            return size

    return int(
        min_size
    )


def _sample_front_luminance(
    surface: Image.Image,
) -> float:
    """
    Sample the central printed area of the actual texture rather than
    averaging the complete circumference.
    """

    width, height = surface.size

    front_px = max(
        1,
        int(
            width
            * FRONT_WIDTH
        ),
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
        height
        * 0.12
    )

    y2 = int(
        height
        * 0.95
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
        0.2126
        * sample[0]
        + 0.7152
        * sample[1]
        + 0.0722
        * sample[2]
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
    for child in list(
        element
    ):
        element.remove(
            child
        )

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

    # The SVG design remains normalized and square.
    #
    # Actual output dimensions are taken from the generated texture.
    root.set(
        "viewBox",
        f"0 0 {VIEWBOX_SIZE:g} {VIEWBOX_SIZE:g}",
    )

    root.set(
        "width",
        str(
            surface.width
        ),
    )

    root.set(
        "height",
        str(
            surface.height
        ),
    )

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
                sorted(
                    missing
                )
            )
        )

    # ------------------------------------------------------------------
    # Variable label text
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

    elif paint.get(
        "pattern"
    ):
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
    # PaintZ logo with red Z
    # ------------------------------------------------------------------

    logo = elements[
        "logo"
    ]

    for child in list(
        logo
    ):
        logo.remove(
            child
        )

    logo.text = "Paint"

    z = ET.SubElement(
        logo,
        f"{{{SVG_NS}}}tspan",
        {
            "id": "logo-z",
            "fill": RED_HEX,
        },
    )

    z.text = "Z"

    # ------------------------------------------------------------------
    # Horizontal front-face layout
    # ------------------------------------------------------------------

    center_x = (
        VIEWBOX_SIZE
        / 2.0
    )

    front_width = (
        VIEWBOX_SIZE
        * FRONT_WIDTH
    )

    badge_width = (
        front_width
        * 0.92
    )

    badge_x = (
        center_x
        - badge_width
        / 2.0
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
        f"{center_x - divider_width / 2.0:.2f}",
    )

    elements[
        "divider"
    ].set(
        "x2",
        f"{center_x + divider_width / 2.0:.2f}",
    )

    # ------------------------------------------------------------------
    # Dynamic font fitting
    #
    # Measurements are performed in the same normalized coordinate system
    # used by the SVG.
    # ------------------------------------------------------------------

    logo_size = _fit_font_size(
        "PaintZ",
        display_font,
        front_width
        * 0.92,
        int(
            front_width
            * 0.38
        ),
        55,
    )

    code_size = _fit_font_size(
        code_text,
        display_font,
        front_width
        * 0.92,
        int(
            front_width
            * 0.12
        ),
        16,
    )

    name_size = _fit_font_size(
        name,
        display_font,
        front_width
        * 0.94,
        int(
            front_width
            * 0.15
        ),
        25,
    )

    series_size = _fit_font_size(
        series,
        display_font,
        badge_width
        * 0.84,
        int(
            front_width
            * 0.085
        ),
        18,
    )

    badge_2_size = _fit_font_size(
        badge_line_2,
        text_font,
        badge_width
        * 0.86,
        int(
            front_width
            * 0.067
        ),
        16,
    )

    badge_3_size = _fit_font_size(
        badge_line_3,
        text_font,
        badge_width
        * 0.86,
        int(
            front_width
            * 0.067
        ),
        16,
    )

    footer_size = _fit_font_size(
        footer,
        text_font,
        front_width
        * 0.96,
        int(
            front_width
            * 0.064
        ),
        14,
    )

    elements[
        "logo"
    ].set(
        "font-size",
        str(
            logo_size
        ),
    )

    elements[
        "paint-code"
    ].set(
        "font-size",
        str(
            code_size
        ),
    )

    elements[
        "paint-name"
    ].set(
        "font-size",
        str(
            name_size
        ),
    )

    elements[
        "series"
    ].set(
        "font-size",
        str(
            series_size
        ),
    )

    elements[
        "badge-line-2"
    ].set(
        "font-size",
        str(
            badge_2_size
        ),
    )

    elements[
        "badge-line-3"
    ].set(
        "font-size",
        str(
            badge_3_size
        ),
    )

    elements[
        "footer"
    ].set(
        "font-size",
        str(
            footer_size
        ),
    )

    # ------------------------------------------------------------------
    # Background-aware contrast
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

    # Badge content stays deliberately consistent for readability.
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
            str(
                display_font
            ),
            str(
                text_font
            ),
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
    Build the final PaintZ can texture:

        square paint/camo surface
              +
        normalized square SVG artwork
              =
        final square can texture
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