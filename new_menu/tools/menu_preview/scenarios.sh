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

# PITREX (3rd category): a program without an entry (the card has zzprog.img) is listed in it, last (25 downs: the list stops at its end)
run "A program no menu file mentions is listed under PITREX and starts its own image, no parameters" 280 "$(downs 2 10),right@40,$(downs 25 50),b4@260" 'image=piZero1/zzprog.img param1=- param2='
run "SETTINGS opens the calibration screen" 100 "$(downs 5 10),b4@70" 'SETTINGS GUI opened'
run "SETTINGS saves to vectrexInterface.ini, not the last ini read" 100 "$(downs 5 10),b4@70" 'save ini vectrexInterface.ini'


# ARCADE > ATARI > ASTEROIDS: Button 2 opens the CONTROLS page (it starts nothing), a button leaves it, Button 4 then starts the game
run "Button 2 shows the controls page, then Button 4 still starts the game" 190 "right@10,down@20,right@30,b2@50,b1@90,b4@120" 'image=piZero1/asteroids_sbt.img'

# sidecars: hide a program, switch a ROM to Speedy
mkdir -p "$work/card"; cp -r "$base"/. "$work/card"/
run "PITREX > SAMPLE PROGRAMS starts MAZE first (HELLO WORLD is hidden)" 170 "$(downs 2 10),right@40,$(downs 6 50),right@110,b4@130" 'image=piZero1/maze.img'
# 0.1.0 card: the entry file itself says hide; 0.2.0 card: a later file changes the [MAZE] section of the menu file
if [ -f "$work/card/menu/maze.ini" ]; then printf '[entry]\nhide = 1\n' > "$work/card/menu/maze.ini"
else printf '[MAZE]\nhide = 1\n' > "$work/card/menu/zz-my-changes.ini"; fi
run "hide = 1 removes MAZE (the first program is now PACMAN)" 170 "$(downs 2 10),right@40,$(downs 6 50),right@110,b4@130" 'image=piZero1/pacman.img' "$work/card"
printf '[entry]\nemulator = speedy\n' > "$work/card/menu/rom.demos_epic_bin.ini"
run "sidecar emulator = speedy: Button 4 starts Speedy" 150 "$roms,b4@110" 'image=piZero1/vectrexspeedy.img param1="demos/epic.bin"' "$work/card"
run "...and Button 2 then starts Exact" 150 "$roms,b2@110" 'image=piZero1/vectrexexact.img param1="demos/epic.bin"' "$work/card"

# a brand new entry file: no recompiling. PITREX is the 3rd category; the new game sorts last in it (after ZZPROG, the program
# the card has without an entry; 25 downs: the list stops at its end)
printf '[entry]\ntitle = ZZZ NEW GAME\ncategory = PITREX\nimage = hello.img\nparam1 = my/param\n' > "$work/card/menu/zznew.ini"
run "a new entry file appears in its category, sorted, with its parameter" 280 "$(downs 2 10),right@40,$(downs 25 50),b4@260" 'image=piZero1/hello.img param1="my/param"' "$work/card"
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

# layers: an add-on file hides TEMPEST, the user's menu.ini (read last) shows it again and makes a collection in its own
# order. "AAA FAVOURITES" sorts first at the top; Tempest is mentioned first, so it is the first row although M < T.
if [ -f "$base/menu/menu.default.ini" ]; then
  ly="$work/layers"; mkdir -p "$ly"; cp -r "$base"/. "$ly"/
  printf '[TEMPEST]\nhide = 1\n' > "$ly/menu/addon.ini"
  printf '[AAA FAVOURITES]\norder = listed\n\n[TEMPEST]\nhide = 0\nalso = AAA FAVOURITES\n\n[MAJOR HAVOC]\nalso = AAA FAVOURITES\n' > "$ly/menu/menu.ini"
  run "menu.ini is read last: Tempest shown again, first in a collection with order = listed" 100 "right@10,b4@40" 'image=piZero1/tempest.img' "$ly"
  run "...and Major Havoc is the second row of the collection" 110 "right@10,down@30,b4@50" 'image=piZero1/aae.img' "$ly"
fi

# needs = vectrex/...: a curated Vectrex entry disappears when its game file is gone. WEB WARS is the last row of
# SPEEDY VECTREX (VECTREX is the 4th category, SPEEDY VECTREX its 4th row); without WEBWARS.BIN, STAR CASTLE is last
nw="$work/nowebwars"; mkdir -p "$nw"; cp -r "$base"/. "$nw"/; rm -f "$nw"/vectrex/original/WEBWARS.BIN
run "an entry whose game file is missing is not listed (needs = vectrex/original/WEBWARS.BIN)" 300 "$(downs 3 10),right@40,$(downs 3 50),right@80,$(downs 20 90),b4@260" 'param1="original/CASTLE.BIN"' "$nw"

# the shipped favorites.ini: FAVORITES is the 3rd category (ARCADE, AUDIO, FAVORITES); Vectorblade first, the Vectrex
# Armor Attack third
fv="$work/favorites"; mkdir -p "$fv"; cp -r "$base"/. "$fv"/; cp "$here/../../menu/sidecar/favorites.ini" "$fv/menu/"
run "favorites.ini: FAVORITES starts with Vectorblade" 100 "$(downs 2 10),right@40,b4@60" 'image=piZero1/vectorblade.img' "$fv"
run "...and its third game is the Vectrex Armor Attack cartridge" 120 "$(downs 2 10),right@40,$(downs 2 50),b4@80" 'image=piZero1/vectrexexact.img param1="original/ARMOR.BIN"' "$fv"

# no metadata at all: no menu files. Top level: AUDIO, PITREX (the programs found), VECTREX, VIDEO (from the built-in folder rules), SETTINGS
nm="$work/nometa"; mkdir -p "$nm"; cp -r "$base"/. "$nm"/; rm -f "$nm"/menu/*.ini
run "no menu files: VIDEO lists vectrex/movies and starts the video player with the file" 120 "$(downs 3 10),right@40,b4@60" 'image=piZero1/vectrexvideo.img param1="vectrex/movies" param2="BadApple.bin"' "$nm"
printf 'g GCE 2026\200' > "$nm/vectrex/000_FIRST.vec"
run "a .vec file is listed under ALL ROMS (loose files come after the folders)" 160 "$(downs 2 10),right@30,right@40,$(downs 2 50),b4@90" 'image=piZero1/vectrexexact.img param1="000_FIRST.vec"' "$nm"

echo; echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
