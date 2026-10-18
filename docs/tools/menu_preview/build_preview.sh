#!/usr/bin/env bash
# Builds the desktop preview of the new menu inside the pitrex-dev container (needs libsdl2-dev,
# g++ and imagemagick there: distrobox enter pitrex-dev -- sudo apt-get install -y libsdl2-dev g++ imagemagick).
#
#   build_preview.sh            builds $OUT/preview
#   shot.sh                     renders screenshots (see shot.sh)
#
# The PiTrex simulator is by Roger Boesch (MIT licence, from the Gong Rider source package at
# https://rogerboesch.games/learn/gong-rider/). It is downloaded into the cache, not stored here.
set -euo pipefail
here=$(cd "$(dirname "$0")" && pwd)
repo=$(cd "$here/../../.." && pwd)
cache=${PREVIEW_CACHE:-$HOME/.cache/pitrex-preview}
out=${PREVIEW_OUT:-$cache/build}
sim=$cache/gong-rider-pitrex
mkdir -p "$cache" "$out"
if [ ! -d "$sim" ]; then
  curl -fsSL -o "$cache/sim.zip" https://rogerboesch.games/learn/gong-rider/gong-rider-pitrex-source.zip
  unzip -q -o "$cache/sim.zip" -d "$cache"
fi
podman exec -w "$sim" pitrex-dev make -s                       # builds the simulator objects in build_sim/
podman exec -w "$out" pitrex-dev bash -c "
  gcc -c -w -Wno-error=implicit-function-declaration -include stdio.h -include string.h -include ctype.h \
      -include stdlib.h -include errno.h -I $repo/pitrex/vectrex -o ini.o $repo/pitrex/vectrex/ini.c &&
  gcc -O1 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -I $out -I $sim/simulator/include -I $repo/pitrex \
      -I $repo/pitrex/loader -I $sim/sim -I $sim/simulator -c $here/preview.c -o preview.o &&
  g++ preview.o ini.o $sim/build_sim/sim/*.o \$(sdl2-config --libs) -lm -o preview"
echo "built $out/preview"
