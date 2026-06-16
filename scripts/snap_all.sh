#!/usr/bin/env bash
# 批量出图：三档 × (page0/1/2 + control + notify)；800 档额外与 HTML 并排对比
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; OUT=/tmp/ir2_snaps; mkdir -p "$OUT"
for R in 800x480 640x480 480x272; do
  for P in 0 1 2; do "$ROOT/scripts/shot.sh" $R $P none "$OUT/lvgl_${R}_p${P}.png"; done
  "$ROOT/scripts/shot.sh" $R 0 control "$OUT/lvgl_${R}_control.png"
  "$ROOT/scripts/shot.sh" $R 0 notify  "$OUT/lvgl_${R}_notify.png"
done
# HTML 真图（仅 800 档有真值）+ 并排对比
for P in 0 1 2; do "$ROOT/scripts/render_html.sh" $P none "$OUT/html_p${P}.png"; "$ROOT/scripts/compare.sh" "$OUT/lvgl_800x480_p${P}.png" "$OUT/html_p${P}.png" "$OUT/cmp_p${P}.png"; done
"$ROOT/scripts/render_html.sh" 0 control "$OUT/html_control.png"; "$ROOT/scripts/compare.sh" "$OUT/lvgl_800x480_control.png" "$OUT/html_control.png" "$OUT/cmp_control.png"
"$ROOT/scripts/render_html.sh" 0 notify "$OUT/html_notify.png"; "$ROOT/scripts/compare.sh" "$OUT/lvgl_800x480_notify.png" "$OUT/html_notify.png" "$OUT/cmp_notify.png"
echo "snaps in $OUT"; ls -la "$OUT"/cmp_*.png "$OUT"/lvgl_480x272_*.png "$OUT"/lvgl_640x480_*.png
