#!/usr/bin/env bash
# clock_shot.sh — mockup 真图(Chrome freeze) + LVGL AM_SHOT 截图,供 Read 比对
# 用法: scripts/clock_shot.sh [W H]  默认 800 480
set -e
W="${1:-800}"; H="${2:-480}"
FREEZE="20:42:08"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MOCKUP_DIR="$ROOT/design-ui/lvgl-clock-html"
SHOTS="$ROOT/shots"
mkdir -p "$SHOTS"

CHROME="/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"
if [ ! -x "$CHROME" ]; then CHROME="google-chrome"; fi

# 1) mockup 真图(冻结时刻)
"$CHROME" --headless=new --disable-gpu --no-sandbox --hide-scrollbars \
  --force-device-scale-factor=1 --run-all-compositor-stages-before-draw \
  --virtual-time-budget=4000 \
  --screenshot="$SHOTS/mockup_${W}x${H}.png" \
  --window-size="$W,$H" \
  "file://$MOCKUP_DIR/render.html?freeze=$FREEZE" >/dev/null 2>&1
echo "[mockup] $SHOTS/mockup_${W}x${H}.png"

# 2) LVGL 截图
cd "$ROOT"
export SDL_VIDEODRIVER=offscreen
export AM_PAGE=0
export AM_SHOT="$SHOTS/lvgl_${W}x${H}.ppm"
# args: W H (main 入口按 ./app W H 选档)
./bin/main "$W" "$H" >/dev/null 2>&1 || true
# convert ppm→png
if [ -f "$SHOTS/lvgl_${W}x${H}.ppm" ]; then
  convert "$SHOTS/lvgl_${W}x${H}.ppm" "$SHOTS/lvgl_${W}x${H}.png"
  rm -f "$SHOTS/lvgl_${W}x${H}.ppm"
fi
echo "[lvgl]   $SHOTS/lvgl_${W}x${H}.png"
echo "OK"
