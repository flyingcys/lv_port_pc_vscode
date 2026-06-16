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

echo "[wallpaper] done"

# ---- icons ----
IMG="$ROOT/main/images"
ICON=96; R=20

process_icon() {  # $1=src_glob_prefix  $2=sym
  local sym="$2"
  # use glob to find the source file (handles special chars / NBSP / ® / ！)
  local src
  src="$(find "$IMG" -maxdepth 1 -name "$1*" | head -1 || true)"
  if [ -z "$src" ] || [ ! -f "$src" ]; then
    echo "MISSING: $IMG/$1*" >&2; exit 1
  fi
  convert "$src" -resize ${ICON}x${ICON} "$TMP/$sym.png"
  convert -size ${ICON}x${ICON} xc:none \
    -draw "roundrectangle 0,0,$((ICON-1)),$((ICON-1)),$R,$R" \
    "$TMP/mask.png"
  convert "$TMP/$sym.png" "$TMP/mask.png" \
    -alpha set -compose DstIn -composite \
    "$TMP/$sym.png"
  python3 "$ROOT/scripts/LVGLImage.py" --ofmt C --cf ARGB8888 -o "$OUT" "$TMP/$sym.png"
  echo "  [icon] $sym"
}

process_icon "Clock-iOS-"                    "img_app_clock"
process_icon "Photos-iOS-"                   "img_app_photos"
process_icon "Calculator-iOS-"               "img_app_calc"
process_icon "Calculator₊-iOS-"             "img_app_calc_plus"
process_icon "文件管理器 - 文件浏览器ZIP"   "img_app_files"
process_icon "网易云音乐-数亿音乐畅听"      "img_app_netease"
process_icon "QQ音乐 - 听我想听-iOS-"       "img_app_qqmusic"
process_icon "Apple Music-iOS-"              "img_app_applemusic"
process_icon "2048_ Number Puzzle Game-iOS-" "img_app_2048"
process_icon "Block*P"                       "img_app_blockpuzzle"
process_icon "Block*B"                       "img_app_blockblast"
process_icon "Fruit Ninja"                   "img_app_fruitninja"

rm -rf "$TMP"
echo "[icons] done"

# ---- fonts ----
TMP2="$(mktemp -d)"
PH_TTF="$TMP2/Phosphor-Fill.ttf"
PH_CSS="$TMP2/phosphor.css"
mkdir -p "$FONTS"

echo "[fonts] downloading Phosphor..."
curl -sSL "https://unpkg.com/@phosphor-icons/web/src/fill/Phosphor-Fill.ttf" -o "$PH_TTF"
curl -sSL "https://unpkg.com/@phosphor-icons/web/src/fill/style.css" -o "$PH_CSS"
echo "[fonts] Phosphor TTF: $(du -h "$PH_TTF" | cut -f1), CSS: $(du -h "$PH_CSS" | cut -f1)"

# Extract codepoints for 10 status-bar icons
RANGE=$(python3 - "$PH_CSS" <<'PY'
import re, sys
css = open(sys.argv[1], encoding='utf-8').read()
names = "cell-signal-full wifi-high battery-full battery-medium battery-warning bluetooth airplane-tilt moon sun speaker-high".split()
cps = []
for n in names:
    m = re.search(r'\.ph-fill\.ph-' + re.escape(n) + r':before\s*\{[^}]*content:\s*"\\([0-9a-fA-F]+)"', css)
    if m:
        cps.append('0x' + m.group(1))
    else:
        print(f"ERROR: codepoint not found for {n}", file=__import__('sys').stderr)
        raise SystemExit(1)
print(",".join(cps))
PY
)
echo "[fonts] Phosphor RANGE=$RANGE"

# Build SimSun CJK symbol set from data.c + fixed UI strings
SIMSUN="/home/share/samba/lvgl/lv_binding_js/deps/lvgl/scripts/built_in_font/SimSun.woff"
CJK=$(python3 - <<'PY'
import re
texts = []
texts.append(open("main/icon_replace_2/icon_replace_2_data.c", encoding="utf-8").read())
texts.append("控制中心 通知中心 下午")
s = "".join(texts)
seen = []
for ch in s:
    if ord(ch) > 0x7F and ch not in seen and ch.strip():
        seen.append(ch)
print("".join(seen))
PY
)
echo "[fonts] CJK set (${#CJK} chars): $CJK"

gen_simsun() {  # $1=size $2=name
    lv_font_conv --font "$SIMSUN" -r 0x20-0x7E --symbols "$CJK" \
        --size "$1" --bpp 4 --format lvgl --no-compress --no-prefilter \
        --lv-font-name "$2" -o "$FONTS/$2.c"
    echo "  [font] $2 done"
}

gen_phosphor() {  # $1=size $2=name
    lv_font_conv --font "$PH_TTF" -r "$RANGE" \
        --size "$1" --bpp 4 --format lvgl --no-compress --no-prefilter \
        --lv-font-name "$2" -o "$FONTS/$2.c"
    echo "  [font] $2 done"
}

gen_simsun 16 ir2_simsun_16
gen_simsun 12 ir2_simsun_12
gen_phosphor 20 ir2_phosphor_20
gen_phosphor 14 ir2_phosphor_14

rm -rf "$TMP2"
echo "[fonts] done"
