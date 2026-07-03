#!/usr/bin/env bash
# Apple Music 截图比对回路
# 用法: scripts/am_shots.sh [res]        res ∈ 800x480(默认) | 640x480 | 480x272
# 产物: /tmp/am/mk_<view>.png(mockup 基线) /tmp/am/lv_<view>.png(LVGL) /tmp/am/cmp_<view>.png(并排)
# 说明: 本机 SDL 无法建窗,LVGL 出图走 main.c 的 AM_SHOT 软件显示分支(无需 SDL 环境变量)。
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
RES="${1:-800x480}"
W="${RES%x*}"; H="${RES#*x}"
OUT=/tmp/am
mkdir -p "$OUT"
MOCKUP="$ROOT/design-ui/music/music-player-macos.html"   # mockup 自带 ?view= 钩子

VIEWS="now radio fav"

echo "[1/3] 渲染 mockup 基线 ($W x $H) ..."
for V in $VIEWS; do
  google-chrome --headless=new --disable-gpu --no-sandbox --hide-scrollbars \
    --force-device-scale-factor=1 --run-all-compositor-stages-before-draw \
    --virtual-time-budget=4000 --screenshot="$OUT/mk_${V}.png" --window-size="$W,$H" \
    "file://$MOCKUP?view=$V" >/dev/null 2>&1
done

echo "[2/3] 渲染 LVGL ($RES) ..."
for V in $VIEWS; do
  AM_EXTRA=""
  SDL_AUDIODRIVER=dummy AM_RES="$RES" AM_APP=apple_music AM_PAGE="$V" \
    AM_SHOT="$OUT/lv_${V}.ppm" AM_SHOT_FRAMES=150 ./bin/main >/dev/null 2>&1
  convert "$OUT/lv_${V}.ppm" "$OUT/lv_${V}.png" 2>/dev/null
done

echo "[3/3] 生成并排对比图 (mockup 左 | lvgl 右) ..."
for V in $VIEWS; do
  convert +append "$OUT/mk_${V}.png" "$OUT/lv_${V}.png" "$OUT/cmp_${V}.png" 2>/dev/null
done

echo "完成: $OUT/cmp_{now,radio,fav}.png"
ls -1 "$OUT"/cmp_*.png 2>/dev/null
