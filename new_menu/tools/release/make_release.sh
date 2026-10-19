#!/bin/sh
# Assembles the release folder and zip from the saved build sets in pitrex-builds.
#   make_release.sh <version> <pitrex-builds dir> <fork checkout> <output dir>
# Nothing from Malban's archive (ROMs, BIOS, his images, ...) is included, only what we built or wrote (pitrex.img and
# pitrex7.img are our builds of the bootstrap). BUILDINFO.txt names the loader's source commit:
#   LOADER_COMMIT=<sha> make_release.sh ...     (default: the fork checkout's HEAD)
set -eu
[ $# -eq 4 ] || { sed -n '2,6p' "$0" | sed 's/^# \{0,1\}//'; exit 2; }
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
# the menu: one file made from the entry files (new_menu/menu/sidecar/menu), and the checksums of the 0.1.0 entry files
# so the installer can remove the ones an upgraded card still has unchanged
mkdir -p "$root/payload/menu"
python3 -I "$fork/new_menu/tools/entries_to_menu_ini.py" "$fork/new_menu/menu/sidecar/menu" "$root/payload/menu/menu.default.ini"
# the example collection: installed only on cards that do not have one, so it becomes the user's own list
mkdir -p "$root/payload-if-missing/menu"
cp "$fork/new_menu/menu/sidecar/favorites.ini" "$root/payload-if-missing/menu/"
git -C "$fork" ls-tree --name-only v0.1.0 new_menu/menu/sidecar/menu/ | while IFS= read -r f; do
  printf '%s  %s\n' "$(git -C "$fork" show "v0.1.0:$f" | sha256sum | cut -d' ' -f1)" "$(basename "$f")"
done > "$root/legacy-menu-files.sha256"

# the guides: README.md (from new_menu/README.md) is the front page. Links into the repository's other docs point at the
# source repository, which is public before a release goes out.
python3 -I "$here/release_links.py" "$fork" "$root"

# arcade tweaks: files exactly as tested on the card
cp "$here/arcade-tweaks/blackwidow.ini" "$here/arcade-tweaks/battlezone.ini" "$root/arcade-tweaks/"

cp "$here/settings/vectrexInterface.ini" "$here/settings/vectrex.ini" "$root/settings/"
cp "$here/install.sh" "$here/install.ps1" "$here/install.bat" "$here/make_card.py" "$root/"
chmod +x "$root/install.sh"
sed "s/@VERSION@/$ver/g" "$here/RELEASE.md" > "$root/RELEASE.md"
commit=${LOADER_COMMIT:-$(git -C "$fork" rev-parse --short HEAD)}
sed -e "s/@VERSION@/$ver/g" -e "s/@COMMIT@/$commit/g" "$here/BUILDINFO.txt" > "$root/BUILDINFO.txt"

(cd "$root" && find . -type f ! -name SHA256SUMS | sed 's|^\./||' | sort | xargs sha256sum > SHA256SUMS)
(cd "$out" && rm -f "$name.zip" && zip -qr "$name.zip" "$name")
echo "built $out/$name.zip"; (cd "$root" && find . -type f | wc -l) | xargs echo files:
