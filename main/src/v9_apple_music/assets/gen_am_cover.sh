#!/usr/bin/env bash
# 生成"正在播放"歌曲封面的烘焙火焰图 am_cover_fire.c(188x188 ARGB8888)。
# 直接用 mockup(.cover)的径向火焰 CSS 经 headless chrome 渲染 → 保证与样机一致。
# 方形输出(不带圆角),LVGL 运行时用 radius+clip_corner 裁圆角。
# 依赖:google-chrome、python3(lvgl/scripts/LVGLImage.py)。
set -eu
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
cd "$ROOT"
ADIR=main/src/v9_apple_music/assets
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
cat > "$TMP/cover.html" <<'HTML'
<!DOCTYPE html><html><head><meta charset="UTF-8"><style>
html,body{margin:0;padding:0}
.c{width:188px;height:188px;position:relative;overflow:hidden;
 background:radial-gradient(circle at 44% 40%, #ffe27a 0%, #ffb02f 18%, #ff7a18 38%, #e0440f 60%, #9a2407 82%, #4a1303 100%);}
.c::before{content:'';position:absolute;inset:0;background:
 radial-gradient(26px 9px at 60% 58%, rgba(80,12,0,0.55) 60%, transparent 62%),
 radial-gradient(32px 10px at 38% 70%, rgba(70,10,0,0.5) 60%, transparent 62%),
 radial-gradient(20px 7px at 54% 82%, rgba(60,8,0,0.45) 60%, transparent 62%);}
</style></head><body><div class="c"></div></body></html>
HTML
google-chrome --headless=new --disable-gpu --no-sandbox --hide-scrollbars \
  --force-device-scale-factor=1 --run-all-compositor-stages-before-draw \
  --virtual-time-budget=2000 --screenshot="$TMP/am_cover_fire.png" --window-size=188,188 \
  "file://$TMP/cover.html"
python3 lvgl/scripts/LVGLImage.py --ofmt C --cf ARGB8888 -o "$ADIR" "$TMP/am_cover_fire.png"
echo "生成 $ADIR/am_cover_fire.c"
