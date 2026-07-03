#!/usr/bin/env bash
# 重生成 Apple Music 的 CJK 文本子集位图字体(修豆腐块)。
# 字符集 = app 源码中文字面量 + sources.tsv 电台名 + test_file 文件名(全部可显示的非 ASCII)。
# 源字体:Montserrat-Medium(拉丁,repo) + Noto Sans CJK SC(从系统 .ttc 抽取) + 图标符号(fonts/src)。
# 依赖:python3+fontTools、npx(lv_font_conv)、系统 /usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc
set -eu
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"   # 仓库根
cd "$ROOT"
FDIR=main/src/v9_apple_music/fonts
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

# 1) 抽取字符集
python3 - "$TMP/cjk.txt" <<'PY'
import pathlib, re, sys
base = pathlib.Path('main/src/v9_apple_music')
srcs = ['am_data.c','apple_music.c','am_page_list.c','am_shell.c','am_player.c','am_widgets.c']
text = ''
for s in srcs:
    p = base/s
    if p.exists():
        t = p.read_text(encoding='utf-8', errors='ignore')
        text += ''.join(a or b for a,b in re.findall(r'"([^"]*)"|`([^`]*)`', t))
text += pathlib.Path('third-party/hls_player_demo/qa/production_test/config/sources.tsv').read_text(encoding='utf-8', errors='ignore')
for f in pathlib.Path('third-party/hls_player_demo/test_file').iterdir():
    text += f.name
keep = sorted(set(ch for ch in text if ord(ch) > 0x7F))
pathlib.Path(sys.argv[1]).write_text(''.join(keep), encoding='utf-8')
print("charset:", len(keep))
PY
cp "$TMP/cjk.txt" "$FDIR/charset.txt"

# 2) 从系统 .ttc 抽取 Noto Sans CJK SC(face 2)为 .ttf(lv_font_conv 不支持 .ttc)
python3 - "$TMP/NotoSansSC.ttf" <<'PY'
from fontTools.ttLib import TTCollection
import sys
c = TTCollection('/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc')
c.fonts[2].save(sys.argv[1])   # 2 = Noto Sans CJK SC
PY

# 3) 逐字号生成
CJK="$(cat "$TMP/cjk.txt")"
SC="$TMP/NotoSansSC.ttf"
MONT=lvgl/scripts/built_in_font/Montserrat-Medium.ttf
DVM="$FDIR/src/DejaVuSansMono.ttf"
SYM="$FDIR/src/NotoSansSymbols2-Regular.ttf"
for N in 11 12 13 14 16 18 24 34; do
  npx -y lv_font_conv --no-compress --format lvgl --bpp 4 --size "$N" \
    --font "$MONT" -r 0x20-0x7F \
    --font "$SC"   --symbols "$CJK" \
    --font "$DVM"  --symbols "⌂◉♫≣⚙♪⌕◌▶↺♥≡" \
    --font "$SYM"  --symbols "⏮⏭◔" \
    --lv-font-name "am_font_$N" -o "$FDIR/am_font_$N.c" --force-fast-kern-format
  echo "am_font_$N done"
done

# 480x272 档字体(am_font_480_*),同字符集
for N in 10 12 15 18 22; do
  npx -y lv_font_conv --no-compress --format lvgl --bpp 4 --size "$N" \
    --font "$MONT" -r 0x20-0x7F \
    --font "$SC"   --symbols "$CJK" \
    --font "$DVM"  --symbols "⌂◉♫≣⚙♪⌕◌▶↺♥≡" \
    --font "$SYM"  --symbols "⏮⏭◔" \
    --lv-font-name "am_font_480_$N" -o "$FDIR/am_font_480_$N.c" --force-fast-kern-format
  echo "am_font_480_$N done"
done
echo "完成。"
