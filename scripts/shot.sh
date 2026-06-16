#!/usr/bin/env bash
# 用法: shot.sh <res:800x480|640x480|480x272> <page> <panel:none|control|notify> <out.png>
# 用 LVGL 无头截图（SDL offscreen + AM_SHOT 钩子）出图，PPM 转 PNG。
set -euo pipefail
RES="${1:-800x480}"; PAGE="${2:-0}"; PANEL="${3:-none}"; OUT="${4:-out_lvgl.png}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PPM="$(mktemp /tmp/lvgl_shot_XXXX.ppm)"
SDL_VIDEODRIVER=offscreen AM_RES="$RES" AM_PAGE="$PAGE" AM_PANEL="$PANEL" AM_SHOT="$PPM" \
  "${ROOT}/bin/main" >/dev/null 2>&1 || true
convert "$PPM" "$OUT" && rm -f "$PPM"
echo "wrote $OUT"
