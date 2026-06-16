#!/usr/bin/env bash
# 用法: compare.sh <left.png> <right.png> <out.png>
# 把两张图左右并排拼成一张，便于一眼比对差异。
set -euo pipefail
convert "$1" "$2" +append "$3"
echo "wrote $3"
