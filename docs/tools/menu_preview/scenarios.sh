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

# the menu ignores the controls until they have rested for 15 frames, so every script starts 45 frames late (the menu reads the card for ~15 frames first)
late() { perl -pe 's/@(\d+)/"@".($1+45)/ge' <<<"$1"; }

downs() { local n=$1 start=$2 s=""; for ((i=0;i<n;i++)); do s="$s,down@$((start+i*8))"; done; echo "${s#,}"; }

run() {   # name frames script expect [card]
  local name=$1 frames=$2 script=$3 expect=$4 card=${5:-$base}
  local res
  res=$(podman exec -e PITREX_SHOT="$work/s.bmp" -e PITREX_FRAMES="$((frames+110))" -e PITREX_SCRIPT="$(late "$script")" \
        -w "$out" pitrex-dev ./preview "$card" 2>&1 | grep -v '^sim:\|^New menu')
  if grep -qF -- "$expect" <<<"$res"; then pass=$((pass+1)); echo "ok    $name"
  else fail=$((fail+1)); echo "FAIL  $name"; echo "      wanted: $expect"; echo "      got:    ${res:-(nothing started)}"; fi
}
runnot() {  # name frames script text-that-must-not-appear [card]
  local name=$1 frames=$2 script=$3 bad=$4 card=${5:-$base}
  local res
  res=$(podman exec -e PITREX_SHOT="$work/s.bmp" -e PITREX_FRAMES="$((frames+110))" -e PITREX_SCRIPT="$(late "$script")" \
        -w "$out" pitrex-dev ./preview "$card" 2>&1 | grep -v '^sim:\|^New menu')
  if grep -qF -- "$bad" <<<"$res"; then fail=$((fail+1)); echo "FAIL  $name (found: $bad)"; echo "      got: $res"
  else pass=$((pass+1)); echo "ok    $name"; fi
}

# ARCADE (first) > ATARI (2nd after ALL AAE GAMES) > MAJOR HAVOC (14th, the folders sort in with the games) > Button 4
run "Major Havoc starts aae.img with game byte 0x2d" 260 "right@10,down@20,right@30,$(downs 13 40),b4@170" 'image=piZero1/aae.img param1="-" param2=- button=4'
run "Button 1 starts it with button=1" 260 "right@10,down@20,right@30,$(downs 13 40),b1@170" 'param1="-" param2=- button=1'

# VECTREX (4th category) > SPEEDY VECTREX (4th: ALL ROMS, DEMOS, EXACT, SPEEDY) > first game
run "SPEEDY group is alphabetical: first game is Berzerk" 130 "$(downs 3 10),right@50,$(downs 3 60),right@90,b4@110" 'image=piZero1/vectrexspeedy.img param1="original/BERZERK.BIN"'

# VECTREX > ALL ROMS (first) > DEMOS FOLDER (first group) > EPIC (2 down)
roms="$(downs 3 10),right@50,right@60,right@70,down@80,down@88"
run "ROM starts the Exact emulator with its path" 150 "$roms,b4@110" 'image=piZero1/vectrexexact.img param1="demos/epic.bin"'
run "Button 2 starts the same ROM with Speedy" 150 "$roms,b2@110" 'image=piZero1/vectrexspeedy.img param1="demos/epic.bin"'

# PROGRAMS (6th category: only programs without an entry file are listed there; the card has zzprog.img)
run "Discovered program starts its own image, no parameters" 100 "$(downs 5 10),right@60,b4@80" 'image=piZero1/zzprog.img param1=- param2='
run "SETTINGS opens the calibration screen" 100 "$(downs 6 10),b4@70" 'SETTINGS GUI opened'
run "SETTINGS saves to vectrexInterface.ini, not the last ini read" 100 "$(downs 6 10),b4@70" 'save ini vectrexInterface.ini'


# ARCADE > ATARI > ASTEROIDS: Button 2 opens the CONTROLS page (it starts nothing), a button leaves it, Button 4 then starts the game
run "Button 2 shows the controls page, then Button 4 still starts the game" 190 "right@10,down@20,right@30,b2@50,b1@90,b4@120" 'image=piZero1/asteroids_sbt.img'

# sidecars: hide a program, switch a ROM to Speedy
mkdir -p "$work/card"; cp -r "$base"/. "$work/card"/
run "COMPUTER > SAMPLE PROGRAMS starts MAZE first (HELLO WORLD is hidden)" 170 "$(downs 2 10),right@40,$(downs 6 50),right@110,b4@130" 'image=piZero1/maze.img'
printf '[entry]\nhide = 1\n' > "$work/card/menu/maze.ini"
run "hide = 1 removes MAZE (the first program is now PACMAN)" 170 "$(downs 2 10),right@40,$(downs 6 50),right@110,b4@130" 'image=piZero1/pacman.img' "$work/card"
printf '[entry]\nemulator = speedy\n' > "$work/card/menu/rom.demos_epic_bin.ini"
run "sidecar emulator = speedy: Button 4 starts Speedy" 150 "$roms,b4@110" 'image=piZero1/vectrexspeedy.img param1="demos/epic.bin"' "$work/card"
run "...and Button 2 then starts Exact" 150 "$roms,b2@110" 'image=piZero1/vectrexexact.img param1="demos/epic.bin"' "$work/card"

# a brand new entry file: no recompiling. COMPUTER is the 3rd category; the new game sorts last in it
printf '[entry]\ntitle = ZZ NEW GAME\ncategory = COMPUTER\nimage = hello.img\nparam1 = my/param\n' > "$work/card/menu/zznew.ini"
run "a new entry file appears in its category, sorted, with its parameter" 200 "$(downs 2 10),right@40,$(downs 12 50),b4@170" 'image=piZero1/hello.img param1="my/param"' "$work/card"
rm -f "$work/card/menu/zznew.ini"

# entries whose ROM is not on the card are hidden; they come back with the file
nr="$work/noroms"; mkdir -p "$nr"; cp -r "$base"/. "$nr"/; find "$nr/roms" -type f -delete
run "no arcade ROMs: ARCADE is gone, the first category is AUDIO" 100 "right@10,b4@40" 'image=piZero1/vectrexaudio.img' "$nr"
: > "$nr/roms/aae/mhavoc.zip"
run "only mhavoc.zip: ARCADE > ATARI > MAJOR HAVOC is the only game" 150 "right@10,down@20,right@30,b4@60" 'image=piZero1/aae.img param1="-"' "$nr"
run "...and the AAE picker is listed (first row of ARCADE)" 100 "right@10,b4@40" 'image=piZero1/aae.img param1=-' "$nr"

# back navigation: enter ARCADE, go back with the stick, the next Button 4 enters VECTREX again (first row)
runnot "entering and leaving a category starts nothing" 80 "down@10,right@20,left@30" 'LAUNCH'
runnot "category rows are not started by Button 4 on a category" 60 "b4@10" 'LAUNCH'

echo; echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
