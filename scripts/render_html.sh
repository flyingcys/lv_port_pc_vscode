#!/usr/bin/env bash
# 用法: render_html.sh <page:0..2> <panel:none|control|notify> <out.png>
set -euo pipefail
PAGE="${1:-0}"; PANEL="${2:-none}"; OUT="${3:-out_html.png}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PARAM="page=${PAGE}"; [ "$PANEL" != "none" ] && PARAM="${PARAM}&panel=${PANEL}"
google-chrome --headless --disable-gpu --hide-scrollbars \
  --no-sandbox \
  --force-device-scale-factor=1 --window-size=800,480 \
  --screenshot="$OUT" \
  "file://${ROOT}/design-ui/render.html?${PARAM}" >/dev/null 2>&1
echo "wrote $OUT"
