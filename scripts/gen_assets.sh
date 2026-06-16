#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/main/icon_replace_2/assets"
FONTS="$OUT/fonts"
TMP="$(mktemp -d)"
WP_URL='https://images.unsplash.com/photo-1579546929518-9e396f3cc809?q=80&w=2070&auto=format&fit=crop'

gen_wallpaper() {  # $1=W $2=H
  local W=$1 H=$2 base="$TMP/img_wallpaper_${1}x${2}"
  convert "$TMP/wp_src.jpg" -resize ${W}x${H}^ -gravity center -extent ${W}x${H} "$base.png"
  convert -size ${W}x$((H/4)) gradient:'rgba(0,0,0,0.4)-rgba(0,0,0,0)' "$TMP/top.png"
  convert -size ${W}x$((H/5)) gradient:'rgba(0,0,0,0)-rgba(0,0,0,0.3)' "$TMP/bot.png"
  convert "$base.png" "$TMP/top.png" -gravity north -composite \
          "$TMP/bot.png" -gravity south -composite "$base.png"
  python3 "$ROOT/scripts/LVGLImage.py" --ofmt C --cf RGB565 -o "$OUT" "$base.png"
}

echo "[wallpaper] downloading..."
curl -sSL "$WP_URL" -o "$TMP/wp_src.jpg"
echo "[wallpaper] downloaded $(du -h "$TMP/wp_src.jpg" | cut -f1)"

gen_wallpaper 800 480
echo "[wallpaper] 800x480 done"
gen_wallpaper 640 480
echo "[wallpaper] 640x480 done"
gen_wallpaper 480 272
echo "[wallpaper] 480x272 done"

rm -rf "$TMP"
echo "[wallpaper] done"
