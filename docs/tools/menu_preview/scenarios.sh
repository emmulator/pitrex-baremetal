#!/usr/bin/env bash
# Drives the new menu in the simulator with scripted input and checks what it starts.
#   scenarios.sh <preview card folder>     (made by make_card.sh; the folder is copied, never changed)
# Each scenario is: a name, the number of frames, the input script, and the text the output must contain.
set -uo pipefail
here=$(cd "$(dirname "$0")" && pwd)
cache=${PREVIEW_CACHE:-$HOME/.cache/pitrex-preview}
out=${PREVIEW_OUT:-$cache/build}
base=$(realpath "$1")
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
pass=0; fail=0

downs() { local n=$1 start=$2 s=""; for ((i=0;i<n;i++)); do s="$s,down@$((start+i*8))"; done; echo "${s#,}"; }

run() {   # name frames script expect [card]
  local name=$1 frames=$2 script=$3 expect=$4 card=${5:-$base}
  local res
  res=$(podman exec -e PITREX_SHOT="$work/s.bmp" -e PITREX_FRAMES="$frames" -e PITREX_SCRIPT="$script" \
        -w "$out" pitrex-dev ./preview "$card" 2>&1 | grep -v '^sim:\|^New menu')
  if grep -qF -- "$expect" <<<"$res"; then pass=$((pass+1)); echo "ok    $name"
  else fail=$((fail+1)); echo "FAIL  $name"; echo "      wanted: $expect"; echo "      got:    ${res:-(nothing started)}"; fi
}
runnot() {  # name frames script text-that-must-not-appear [card]
  local name=$1 frames=$2 script=$3 bad=$4 card=${5:-$base}
  local res
  res=$(podman exec -e PITREX_SHOT="$work/s.bmp" -e PITREX_FRAMES="$frames" -e PITREX_SCRIPT="$script" \
        -w "$out" pitrex-dev ./preview "$card" 2>&1 | grep -v '^sim:\|^New menu')
  if grep -qF -- "$bad" <<<"$res"; then fail=$((fail+1)); echo "FAIL  $name (found: $bad)"; echo "      got: $res"
  else pass=$((pass+1)); echo "ok    $name"; fi
}

# ARCADE (1 down) > ATARI > MAJOR HAVOC (9 down) > Button 4
run "Major Havoc starts aae.img with game byte 0x2d" 170 "down@10,right@20,right@30,$(downs 9 40),b4@130" 'image=piZero1/aae.img param1="-" param2=- button=4 id=2009'
run "Button 1 starts it with button=1" 170 "down@10,right@20,right@30,$(downs 9 40),b1@130" 'button=1 id=2009'

# VECTREX > SPEEDY VECTREX (down 1) > WEB WARS
run "Speedy preset Web Wars" 90 "right@10,down@20,right@30,b4@50" 'image=piZero1/vectrexspeedy.img param1="original/WEBWARS.BIN"'

# VECTREX > ALL ROMS (4 down) > DEMOS (first group) > EPIC (2 down)
roms="right@10,$(downs 4 20),right@64,right@76,down@88,down@96"
run "ROM starts the Exact emulator with its path" 150 "$roms,b4@110" 'image=piZero1/vectrexexact.img param1="demos/epic.bin"'
run "Button 2 starts the same ROM with Speedy" 150 "$roms,b2@110" 'image=piZero1/vectrexspeedy.img param1="demos/epic.bin"'

# PROGRAMS (5 down) > first program
run "Discovered program starts its own image, no parameters" 100 "$(downs 5 10),right@60,b4@80" 'image=piZero1/asteroids.img param1=- param2='
run "SETTINGS opens the calibration screen" 100 "$(downs 6 10),b4@70" 'SETTINGS GUI opened'

# sidecars: hide a program, switch a ROM to Speedy
mkdir -p "$work/card"; cp -r "$base"/. "$work/card"/
printf '[entry]\nhide = 1\n' > "$work/card/menu/maze.ini"
run "hide = 1 removes MAZE (4 down in PROGRAMS now reaches PACMAN)" 140 "$(downs 5 10),right@60,$(downs 4 70),b4@110" 'image=piZero1/pacman.img' "$work/card"
printf '[entry]\nemulator = speedy\n' > "$work/card/menu/rom.demos_epic_bin.ini"
run "sidecar emulator = speedy: Button 4 starts Speedy" 150 "$roms,b4@110" 'image=piZero1/vectrexspeedy.img param1="demos/epic.bin"' "$work/card"
run "...and Button 2 then starts Exact" 150 "$roms,b2@110" 'image=piZero1/vectrexexact.img param1="demos/epic.bin"' "$work/card"

# back navigation: enter ARCADE, go back with the stick, the next Button 4 enters VECTREX again (first row)
run "stick left goes back; selection returns to ARCADE" 80 "down@10,right@20,left@30,right@40" 'WILL-NOT-LAUNCH-ANYTHING' >/dev/null 2>&1 || true
runnot "entering and leaving a category starts nothing" 80 "down@10,right@20,left@30" 'LAUNCH'
runnot "category rows are not started by Button 4 on a category" 60 "b4@10" 'LAUNCH'

echo; echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
