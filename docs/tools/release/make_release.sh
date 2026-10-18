#!/bin/sh
# Assembles the release folder and zip from the saved build sets in pitrex-builds.
#   make_release.sh <version> <pitrex-builds dir> <fork checkout> <output dir>
# Nothing from Malban's archive (ROMs, pitrex.img, BIOS, ...) is included, only what we built or wrote.
set -eu
[ $# -eq 4 ] || { sed -n '2,4p' "$0" | sed 's/^# \{0,1\}//'; exit 2; }
ver=$1; builds=$2; fork=$3; out=$4
here=$(cd "$(dirname "$0")" && pwd)
name="pitrex-new-menu-$ver"
root="$out/$name"
rm -rf "$root"
mkdir -p "$root/payload" "$root/arcade-tweaks" "$root/settings"

# the bootstrap (pitrex.img, pitrex7.img) must come from the same source as the loader: Malban's May 2025 one does
# not start this loader (blank screen)
cp "$builds/loader-latest/pitrex.img" "$builds/loader-latest/pitrex7.img" "$root/payload/"
for pi in piZero1 piZero2; do
  mkdir -p "$root/payload/$pi"
  cp "$builds/loader-latest/$pi/aae.img" "$root/payload/$pi/"
  cp "$builds/loader-latest/$pi/loader.pit" "$root/payload/$pi/"
  cp "$builds/emulators-2025src-gcc10.3.1/$pi/"vectrexspeedy.img "$builds/emulators-2025src-gcc10.3.1/$pi/"vectrexexact.img "$root/payload/$pi/"
  cp "$builds/vectrex-players-fork-source/$pi/"*.img "$root/payload/$pi/"
  cp "$builds/arcade-fixes-20261017/$pi/"*.img "$root/payload/$pi/"
done
mkdir -p "$root/payload/menu"
cp "$fork/docs/menu/sidecar/menu/"*.ini "$root/payload/menu/"
cp "$fork/NEW_MENU_README.md" "$root/payload/NEW_MENU_README.md"
cp "$fork/NEW_MENU_README.md" "$root/NEW_MENU_README.md"
# the plug-and-play guide is the front page of the package; its links to the repository's docs/ become plain text
python3 -I - "$fork/docs/README.md" "$root/README.md" <<'PY'
import sys
s = open(sys.argv[1]).read()
s = s.replace("](../NEW_MENU_README.md)", "](NEW_MENU_README.md)")
s = s.replace("Licensing notes: [LICENSING.md](LICENSING.md).\nDeveloper notes for this fork: [DEVELOPMENT.md](DEVELOPMENT.md).",
              "Licensing notes and developer notes: `docs/` in the source repository.")
open(sys.argv[2], "w").write(s)
PY

# arcade tweaks: files exactly as tested on the card
cp "$here/arcade-tweaks/blackwidow.ini" "$here/arcade-tweaks/battlezone.ini" "$root/arcade-tweaks/"

cp "$here/settings/vectrexInterface.ini" "$here/settings/vectrex.ini" "$root/settings/"
cp "$here/install.sh" "$here/install.ps1" "$here/install.bat" "$here/make_card.py" "$root/"
chmod +x "$root/install.sh"
sed "s/@VERSION@/$ver/g" "$here/RELEASE.md" > "$root/RELEASE.md"

(cd "$root" && find . -type f ! -name SHA256SUMS | sed 's|^\./||' | sort | xargs sha256sum > SHA256SUMS)
(cd "$out" && rm -f "$name.zip" && zip -qr "$name.zip" "$name")
echo "built $out/$name.zip"; (cd "$root" && find . -type f | wc -l) | xargs echo files:
