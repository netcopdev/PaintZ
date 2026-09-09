from pathlib import Path
from tempfile import TemporaryDirectory
import sys

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

from paintzgen.color_management import (
    normalize_to_srgb,
    srgb_icc_profile,
    tag_srgb,
)
from paintzgen.scaled_surface import scale_pattern_fill


def test_tag_srgb_does_not_change_pixels():
    image = Image.new("RGBA", (4, 4), (85, 75, 67, 255))
    tagged = tag_srgb(image)

    assert tagged.getpixel((0, 0)) == (85, 75, 67, 255)

    profile = srgb_icc_profile()
    if profile is not None:
        assert tagged.info.get("icc_profile") == profile


def test_normalize_untagged_image_preserves_rgb():
    source = Image.new("RGB", (4, 4), (92, 98, 78))
    converted = normalize_to_srgb(source, "RGBA")

    assert converted.getpixel((0, 0)) == (92, 98, 78, 255)

    if srgb_icc_profile() is not None:
        assert converted.info.get("icc_profile")


def test_png_save_round_trip_preserves_srgb_profile():
    profile = srgb_icc_profile()
    if profile is None:
        return

    with TemporaryDirectory() as tmp:
        path = Path(tmp) / "solid.png"
        image = tag_srgb(
            Image.new("RGBA", (4, 4), (85, 75, 67, 255))
        )
        image.save(path, icc_profile=image.info.get("icc_profile"))

        with Image.open(path) as reopened:
            assert reopened.getpixel((0, 0)) == (85, 75, 67, 255)
            assert reopened.info.get("icc_profile")


def test_scaled_pattern_variants_keep_srgb_profile():
    with TemporaryDirectory() as tmp:
        path = Path(tmp) / "pattern.png"
        source = Image.new("RGB", (8, 8), (50, 70, 40))
        source.save(path)

        enlarged = scale_pattern_fill(path, (16, 16), 2.0)
        tiled = scale_pattern_fill(path, (16, 16), 0.5)

        assert enlarged.mode == "RGBA"
        assert tiled.mode == "RGBA"

        if srgb_icc_profile() is not None:
            assert enlarged.info.get("icc_profile")
            assert tiled.info.get("icc_profile")
