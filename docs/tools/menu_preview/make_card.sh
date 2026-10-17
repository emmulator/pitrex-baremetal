#!/usr/bin/env bash
# Builds a folder that looks like the PiTrex SD card for the preview: every file of the real card
# layout as an EMPTY placeholder (only names matter), plus the real sidecar files.
#   make_card.sh <source card folder> <out folder> <sidecar menu folder> [folder with extra .bin/.vec names]
set -euo pipefail
src=${1:?source card}; out=${2:?out folder}; side=${3:?sidecar menu folder}; extra=${4:-}
rm -rf "$out"; mkdir -p "$out"
( cd "$src" && find piZero1 piZero2 vectrex roms -type f ! -name '.*' 2>/dev/null ) | while IFS= read -r f; do
  mkdir -p "$out/$(dirname "$f")"; : > "$out/$f"
done
cp "$src/config.txt" "$out/" 2>/dev/null || : > "$out/config.txt"
: > "$out/vectorburner-placeholder"; mkdir -p "$out/piZero1" "$out/piZero2"
: > "$out/piZero1/vectorburner.img"; : > "$out/piZero2/vectorburner.img"
mkdir -p "$out/menu"; cp "$side"/*.ini "$out/menu/"
if [ -n "$extra" ]; then
  for f in "$extra"/*; do
    case "${f,,}" in *.bin) : > "$out/vectrex/$(basename "$f")";; *.vec) : > "$out/vectrex/$(basename "${f%.*}").bin";; esac
  done
fi
echo "preview card: $(find "$out" -type f | wc -l) files in $out"
