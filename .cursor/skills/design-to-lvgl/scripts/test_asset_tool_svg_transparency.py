#!/usr/bin/env python3
import subprocess
import sys
import tempfile
from pathlib import Path

from PIL import Image


REPO_ROOT = Path(__file__).resolve().parents[4]
ASSET_TOOL = REPO_ROOT / ".cursor/skills/design-to-lvgl/scripts/asset_tool.py"


def convert_svg(input_path: Path, output_path: Path) -> None:
    result = subprocess.run(
        [
            sys.executable,
            str(ASSET_TOOL),
            "convert",
            "--input",
            str(input_path),
            "--output",
            str(output_path),
        ],
        cwd=REPO_ROOT,
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        raise AssertionError(
            f"convert failed for {input_path.name}: {result.stderr.strip() or result.stdout.strip()}"
        )


def assert_has_transparency(path: Path) -> None:
    image = Image.open(path).convert("RGBA")
    alpha_values = [pixel[3] for pixel in image.getdata()]
    if not any(alpha == 0 for alpha in alpha_values):
        raise AssertionError(f"{path.name} has no transparent pixels")


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp_dir:
        tmp_path = Path(tmp_dir)
        weather_output = tmp_path / "weather.png"
        wifi_output = tmp_path / "wifi.png"

        convert_svg(REPO_ROOT / "main/assets/home/weather-icon.svg", weather_output)
        convert_svg(REPO_ROOT / "main/assets/home/wifi-icon.svg", wifi_output)

        assert_has_transparency(weather_output)
        assert_has_transparency(wifi_output)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
