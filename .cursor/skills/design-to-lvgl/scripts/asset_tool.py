#!/usr/bin/env python3
import argparse
import json
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

try:
    from PIL import Image, ImageColor, ImageDraw, ImageFilter, ImageFont
except ImportError as exc:
    print("Pillow is required. Install it with: pip install pillow", file=sys.stderr)
    raise SystemExit(2) from exc

if hasattr(Image, "Resampling"):
    RESAMPLE = Image.Resampling.LANCZOS
else:
    RESAMPLE = Image.LANCZOS


def parse_color(value, default=(0, 0, 0, 0)):
    if value is None:
        return default
    return ImageColor.getcolor(value, "RGBA")


def apply_opacity(image, opacity):
    if opacity >= 1.0:
        return image
    opacity = max(0.0, min(1.0, opacity))
    alpha = image.getchannel("A").point(lambda pixel: int(pixel * opacity))
    output = image.copy()
    output.putalpha(alpha)
    return output


def resolve_path(raw_path, base_dir):
    path = Path(raw_path)
    if path.is_absolute():
        return path
    return (base_dir / path).resolve()


def open_rgba(path):
    if path.suffix.lower() == ".svg":
        return rasterize_svg(path)
    return Image.open(path).convert("RGBA")


def get_image_magick_command():
    for tool in ("magick", "convert"):
        resolved = shutil.which(tool)
        if resolved:
            return tool
    return None


def rasterize_svg(path):
    tool = get_image_magick_command()
    if tool is None:
        raise RuntimeError(
            "SVG input requires ImageMagick (`magick` or `convert`) to be installed"
        )

    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmp_file:
        tmp_path = Path(tmp_file.name)

    try:
        if tool == "magick":
            command = [
                tool,
                "-background",
                "none",
                str(path),
                f"PNG32:{tmp_path}",
            ]
        else:
            command = [
                tool,
                "-background",
                "none",
                str(path),
                f"PNG32:{tmp_path}",
            ]

        result = subprocess.run(
            command,
            capture_output=True,
            text=True,
            check=False,
        )
        if result.returncode != 0:
            raise RuntimeError(
                f"failed to rasterize SVG with {tool}: {result.stderr.strip() or result.stdout.strip()}"
            )

        return Image.open(tmp_path).convert("RGBA")
    finally:
        if tmp_path.exists():
            tmp_path.unlink()


def ensure_parent(path):
    path.parent.mkdir(parents=True, exist_ok=True)


def fit_to_box(image, width, height, fit, background=(0, 0, 0, 0)):
    if fit == "stretch":
        return image.resize((width, height), RESAMPLE)

    src_w, src_h = image.size
    if src_w == 0 or src_h == 0:
        raise ValueError("source image has invalid size")

    if fit == "contain":
        scale = min(width / src_w, height / src_h)
    elif fit == "cover":
        scale = max(width / src_w, height / src_h)
    else:
        raise ValueError(f"unsupported fit mode: {fit}")

    new_w = max(1, round(src_w * scale))
    new_h = max(1, round(src_h * scale))
    resized = image.resize((new_w, new_h), RESAMPLE)

    if fit == "cover":
        left = max(0, (new_w - width) // 2)
        top = max(0, (new_h - height) // 2)
        return resized.crop((left, top, left + width, top + height))

    canvas = Image.new("RGBA", (width, height), background)
    left = (width - new_w) // 2
    top = (height - new_h) // 2
    canvas.alpha_composite(resized, (left, top))
    return canvas


def flatten_for_save(image, output_path, background):
    suffix = output_path.suffix.lower()
    if suffix in {".jpg", ".jpeg"}:
        base = Image.new("RGB", image.size, background[:3])
        base.paste(image, mask=image.getchannel("A"))
        return base
    return image


def command_inspect(args):
    payload = []
    for raw_path in args.paths:
        path = Path(raw_path).expanduser().resolve()
        if not path.exists():
            payload.append({
                "path": str(path),
                "exists": False,
            })
            continue

        if path.suffix.lower() == ".svg":
            with open_rgba(path) as image:
                payload.append({
                    "path": str(path),
                    "exists": True,
                    "format": "SVG",
                    "mode": image.mode,
                    "width": image.width,
                    "height": image.height,
                })
        else:
            with Image.open(path) as image:
                payload.append({
                    "path": str(path),
                    "exists": True,
                    "format": image.format,
                    "mode": image.mode,
                    "width": image.width,
                    "height": image.height,
                })

    print(json.dumps(payload, ensure_ascii=False, indent=2))
    return 0


def command_convert(args):
    input_path = Path(args.input).expanduser().resolve()
    output_path = Path(args.output).expanduser().resolve()
    background = parse_color(args.background)

    image = open_rgba(input_path)

    if args.rotate:
        image = image.rotate(-args.rotate, expand=True)

    if args.width and args.height:
        image = fit_to_box(image, args.width, args.height, args.fit, background)
    elif args.width or args.height:
        raise ValueError("--width and --height must be provided together")

    image = flatten_for_save(image, output_path, background)
    ensure_parent(output_path)

    save_kwargs = {}
    if output_path.suffix.lower() in {".jpg", ".jpeg"}:
        save_kwargs["quality"] = args.quality

    image.save(output_path, **save_kwargs)
    print(str(output_path))
    return 0


def anchor_position(x, y, width, height, anchor):
    mapping = {
        "lt": (x, y),
        "mt": (x - width // 2, y),
        "rt": (x - width, y),
        "lm": (x, y - height // 2),
        "mm": (x - width // 2, y - height // 2),
        "rm": (x - width, y - height // 2),
        "lb": (x, y - height),
        "mb": (x - width // 2, y - height),
        "rb": (x - width, y - height),
    }
    if anchor not in mapping:
        raise ValueError(f"unsupported anchor: {anchor}")
    return mapping[anchor]


def draw_text(canvas, item, base_dir):
    draw = ImageDraw.Draw(canvas)
    font_path = resolve_path(item["font"], base_dir)
    font = ImageFont.truetype(str(font_path), int(item["font_size"]))
    text = item["text"]
    spacing = int(item.get("spacing", 0))
    stroke_width = int(item.get("stroke_width", 0))
    fill = parse_color(item.get("fill", "#000000"))
    stroke_fill = parse_color(item.get("stroke_fill", "#00000000"))

    if "opacity" in item:
        alpha = int(fill[3] * max(0.0, min(1.0, float(item["opacity"]))))
        fill = (fill[0], fill[1], fill[2], alpha)

    bbox = draw.textbbox(
        (0, 0),
        text,
        font=font,
        spacing=spacing,
        stroke_width=stroke_width,
    )
    width = bbox[2] - bbox[0]
    height = bbox[3] - bbox[1]
    left, top = anchor_position(
        int(item["x"]),
        int(item["y"]),
        width,
        height,
        item.get("anchor", "lt"),
    )

    draw.text(
        (left - bbox[0], top - bbox[1]),
        text,
        font=font,
        fill=fill,
        spacing=spacing,
        stroke_width=stroke_width,
        stroke_fill=stroke_fill,
    )


def draw_image(canvas, item, base_dir):
    image_path = resolve_path(item["path"], base_dir)
    image = open_rgba(image_path)

    if item.get("rotate"):
        image = image.rotate(-float(item["rotate"]), expand=True)

    opacity = float(item.get("opacity", 1.0))
    if "width" in item and "height" in item:
        box_background = parse_color(item.get("background"))
        image = fit_to_box(
            image,
            int(item["width"]),
            int(item["height"]),
            item.get("fit", "contain"),
            box_background,
        )

    image = apply_opacity(image, opacity)
    canvas.alpha_composite(image, (int(item["x"]), int(item["y"])))


def draw_glow(canvas, item):
    overlay = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    radius = int(item["radius"])
    color = parse_color(item.get("color", "#FFFFFF"))
    alpha = float(item.get("alpha", 0.2))
    fill = (color[0], color[1], color[2], int(255 * max(0.0, min(1.0, alpha))))
    cx = int(item["x"])
    cy = int(item["y"])
    draw.ellipse(
        (cx - radius, cy - radius, cx + radius, cy + radius),
        fill=fill,
    )
    blur = float(item.get("blur", 0))
    if blur > 0:
        overlay = overlay.filter(ImageFilter.GaussianBlur(radius=blur))
    canvas.alpha_composite(overlay)


def draw_rectangle(canvas, item):
    overlay = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    x = int(item["x"])
    y = int(item["y"])
    width = int(item["width"])
    height = int(item["height"])
    radius = int(item.get("radius", 0))
    fill = parse_color(item.get("fill"))
    outline = parse_color(item.get("outline"))
    outline_width = int(item.get("outline_width", 0))

    if "opacity" in item:
        opacity = max(0.0, min(1.0, float(item["opacity"])))
        if fill is not None:
            fill = (fill[0], fill[1], fill[2], int(fill[3] * opacity))
        if outline is not None:
            outline = (outline[0], outline[1], outline[2], int(outline[3] * opacity))

    draw.rounded_rectangle(
        (x, y, x + width, y + height),
        radius=radius,
        fill=fill,
        outline=outline,
        width=outline_width,
    )
    canvas.alpha_composite(overlay)


def command_compose(args):
    spec_path = Path(args.spec).expanduser().resolve()
    output_path = Path(args.output).expanduser().resolve()
    spec = json.loads(spec_path.read_text(encoding="utf-8"))
    base_dir = spec_path.parent

    width = int(spec["width"])
    height = int(spec["height"])
    background = parse_color(spec.get("background"))
    canvas = Image.new("RGBA", (width, height), background)

    for item in spec.get("items", []):
        item_type = item["type"]
        if item_type == "image":
            draw_image(canvas, item, base_dir)
        elif item_type == "text":
            draw_text(canvas, item, base_dir)
        elif item_type == "glow":
            draw_glow(canvas, item)
        elif item_type == "rectangle":
            draw_rectangle(canvas, item)
        else:
            raise ValueError(f"unsupported item type: {item_type}")

    image = flatten_for_save(canvas, output_path, background)
    ensure_parent(output_path)

    save_kwargs = {}
    if output_path.suffix.lower() in {".jpg", ".jpeg"}:
        save_kwargs["quality"] = int(spec.get("quality", 95))

    image.save(output_path, **save_kwargs)
    print(str(output_path))
    return 0


def build_parser():
    parser = argparse.ArgumentParser(description="Asset utility for design-to-LVGL workflow.")
    subparsers = parser.add_subparsers(dest="command", required=True)

    inspect_parser = subparsers.add_parser("inspect", help="Inspect image metadata.")
    inspect_parser.add_argument("paths", nargs="+")
    inspect_parser.set_defaults(func=command_inspect)

    convert_parser = subparsers.add_parser("convert", help="Convert or fit a single image.")
    convert_parser.add_argument("--input", required=True)
    convert_parser.add_argument("--output", required=True)
    convert_parser.add_argument("--width", type=int)
    convert_parser.add_argument("--height", type=int)
    convert_parser.add_argument("--fit", choices=["contain", "cover", "stretch"], default="contain")
    convert_parser.add_argument("--rotate", type=float, default=0.0)
    convert_parser.add_argument("--background", default="#00000000")
    convert_parser.add_argument("--quality", type=int, default=95)
    convert_parser.set_defaults(func=command_convert)

    compose_parser = subparsers.add_parser("compose", help="Compose an image from a JSON spec.")
    compose_parser.add_argument("--spec", required=True)
    compose_parser.add_argument("--output", required=True)
    compose_parser.set_defaults(func=command_compose)

    return parser


def main():
    parser = build_parser()
    args = parser.parse_args()
    try:
        return args.func(args)
    except Exception as exc:
        print(f"asset_tool error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
