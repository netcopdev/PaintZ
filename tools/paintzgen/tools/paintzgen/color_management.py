from __future__ import annotations

from functools import lru_cache
from io import BytesIO

from PIL import Image

try:
    from PIL import ImageCms
except ImportError:  # pragma: no cover - depends on Pillow build
    ImageCms = None


@lru_cache(maxsize=1)
def srgb_icc_profile() -> bytes | None:
    """Return a reusable ICC blob for the standard sRGB profile."""
    if ImageCms is None:
        return None

    try:
        profile = ImageCms.ImageCmsProfile(
            ImageCms.createProfile("sRGB")
        )
        return profile.tobytes()
    except Exception:
        return None


def tag_srgb(image: Image.Image) -> Image.Image:
    """Mark an image as sRGB without changing its pixel values."""
    profile = srgb_icc_profile()
    if profile is not None:
        image.info["icc_profile"] = profile
    else:
        image.info.pop("icc_profile", None)
    return image


def normalize_to_srgb(
    image: Image.Image,
    mode: str = "RGBA",
) -> Image.Image:
    """
    Convert a loaded image into sRGB when it has an embedded ICC profile,
    then return it in the requested mode with a consistent sRGB tag.

    Untagged images are treated as sRGB, matching normal PNG/web practice.
    """
    embedded = image.info.get("icc_profile")

    if embedded and ImageCms is not None:
        try:
            source_profile = ImageCms.ImageCmsProfile(BytesIO(embedded))
            target_profile = ImageCms.createProfile("sRGB")

            alpha = None
            if "A" in image.getbands():
                alpha = image.getchannel("A")

            rgb = image.convert("RGB")
            converted = ImageCms.profileToProfile(
                rgb,
                source_profile,
                target_profile,
                outputMode="RGB",
            )

            if mode == "RGBA":
                converted = converted.convert("RGBA")
                if alpha is not None:
                    converted.putalpha(alpha)
            else:
                converted = converted.convert(mode)

            return tag_srgb(converted)
        except Exception:
            # A malformed/unsupported input profile must not make asset
            # generation fail. Fall back to the normal untagged-as-sRGB path.
            pass

    return tag_srgb(image.convert(mode))
