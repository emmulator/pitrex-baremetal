#!/usr/bin/env bash
# Renders the new menu in the simulator, headless, to PNG files.
#   shot.sh <card folder> <out dir> <name> <frames> "<script>"
# The script is the simulator's: "down@10,right@20,b4@30" (names joined by + and @ frame; each
# input is held for 4 frames). Prints what the menu started, if anything.
set -euo pipefail
here=$(cd "$(dirname "$0")" && pwd)
cache=${PREVIEW_CACHE:-$HOME/.cache/pitrex-preview}
out=${PREVIEW_OUT:-$cache/build}
card=$(realpath "$1"); dir=$2; name=$3; frames=$4; script=${5:-}
mkdir -p "$dir"
podman exec -e PITREX_SHOT="$dir/$name.bmp" -e PITREX_FRAMES="$frames" -e PITREX_SCRIPT="$script" \
  -w "$out" pitrex-dev ./preview "$card" 2>&1 | grep -v '^sim:' || true
podman exec pitrex-dev convert "$dir/$name.bmp" "$dir/$name.png"
rm -f "$dir/$name.bmp"
echo "$dir/$name.png"
