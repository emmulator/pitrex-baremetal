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

for pi in piZero1 piZero2; do
  mkdir -p "$root/payload/$pi"
  cp "$builds/loader-latest/$pi/loader.pit" "$root/payload/$pi/"
  cp "$builds/emulators-2025src-gcc10.3.1/$pi/"vectrexspeedy.img "$builds/emulators-2025src-gcc10.3.1/$pi/"vectrexexact.img "$root/payload/$pi/"
  cp "$builds/vectrex-players-fork-source/$pi/"*.img "$root/payload/$pi/"
  cp "$builds/arcade-fixes-20261017/$pi/"*.img "$root/payload/$pi/"
done
mkdir -p "$root/payload/menu"
cp "$fork/docs/menu/sidecar/menu/"*.ini "$root/payload/menu/"
cp "$fork/NEW_MENU_README.md" "$root/payload/NEW_MENU_README.md"
cp "$fork/NEW_MENU_README.md" "$root/NEW_MENU_README.md"

# arcade tweaks: files exactly as tested on the card
cp "$here/arcade-tweaks/blackwidow.ini" "$here/arcade-tweaks/battlezone.ini" "$root/arcade-tweaks/"

cp "$here/settings/vectrexInterface.ini" "$root/settings/"
cp "$here/install.sh" "$here/install.ps1" "$here/install.bat" "$root/"
chmod +x "$root/install.sh"
sed "s/@VERSION@/$ver/g" "$here/RELEASE.md" > "$root/RELEASE.md"

(cd "$root" && find . -type f ! -name SHA256SUMS | sed 's|^\./||' | sort | xargs sha256sum > SHA256SUMS)
(cd "$out" && rm -f "$name.zip" && zip -qr "$name.zip" "$name")
echo "built $out/$name.zip"; (cd "$root" && find . -type f | wc -l) | xargs echo files:
