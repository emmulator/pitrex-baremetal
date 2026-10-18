#!/bin/sh
# Installs the new PiTrex menu release over an unpacked Malban "PiTrex_sd_Jul29_25" archive
# (or over the SD card made from it). Linux, macOS, WSL.
#
#   ./install.sh [--arcade-tweaks] [--dry-run] <folder>
#   ./install.sh --restore <folder>
#
# Replaced files are saved in <folder>/backup-menu-<date>/ with a MANIFEST.tsv that --restore reads.
# Nothing outside the files listed in payload/ (and, with --arcade-tweaks, arcade-tweaks/) is touched.
set -eu

here=$(cd "$(dirname "$0")" && pwd)
payload="$here/payload"
tweaks="$here/arcade-tweaks"
mode=install
dry=0
arcade=0
target=

usage() {
  sed -n '2,10p' "$0" | sed 's/^# \{0,1\}//'
  exit "${1:-2}"
}

while [ $# -gt 0 ]; do
  case $1 in
    --restore) mode=restore ;;
    --dry-run) dry=1 ;;
    --arcade-tweaks) arcade=1 ;;
    -h|--help) usage 0 ;;
    -*) echo "unknown option: $1" >&2; usage ;;
    *) [ -z "$target" ] || usage; target=$1 ;;
  esac
  shift
done
[ -n "$target" ] || usage
[ -d "$target" ] || { echo "not a folder: $target" >&2; exit 1; }
target=$(cd "$target" && pwd)

die() { echo "ERROR: $*" >&2; exit 1; }

# ---------------------------------------------------------------- restore
if [ "$mode" = restore ]; then
  bk=
  for d in "$target"/backup-menu-*; do
    [ -f "$d/MANIFEST.tsv" ] && bk=$d
  done
  [ -n "$bk" ] || die "no backup-menu-* folder with a MANIFEST.tsv in $target"
  echo "restoring from $bk"
  # undo in reverse order of installation
  tab=$(printf '\t')
  tac_file=$(mktemp)
  awk '{ l[NR] = $0 } END { for (i = NR; i >= 1; i--) print l[i] }' "$bk/MANIFEST.tsv" > "$tac_file"
  while IFS="$tab" read -r kind a b; do
    case $kind in
      R) cp "$bk/$a" "$target/$a" && echo "  restored $a" ;;
      A) rm -f "$target/$a" && echo "  removed  $a" ;;
      M) if [ -e "$target/$b" ] && [ ! -e "$target/$a" ]; then mv "$target/$b" "$target/$a" && echo "  renamed back $a"; fi ;;
    esac
  done < "$tac_file"
  rm -f "$tac_file"
  mv "$bk/MANIFEST.tsv" "$bk/MANIFEST.tsv.done"
  # remove now-empty folders we may have created (menu/)
  rmdir "$target/menu" 2>/dev/null || true
  echo "done. (the backup folder was kept: $bk)"
  exit 0
fi

# ---------------------------------------------------------------- install
[ -d "$payload" ] || die "payload/ folder missing next to this script"
for f in pitrex.img loader.ini piZero1/loader.pit; do
  [ -f "$target/$f" ] || die "$target does not look like the unpacked PiTrex_sd_Jul29_25 archive ($f is missing).
Pass the folder that contains pitrex.img, loader.ini and the piZero1 and piZero2 folders."
done

stamp=$(date +%Y%m%d-%H%M%S)
bk="$target/backup-menu-$stamp"
n=1
while [ -e "$bk" ]; do n=$((n + 1)); bk="$target/backup-menu-$stamp-$n"; done
manifest="$bk/MANIFEST.tsv"
tab=$(printf '\t')
changed=0

start_backup() {
  [ "$dry" = 1 ] && return 0
  [ -d "$bk" ] || mkdir -p "$bk"
}

note() { [ "$dry" = 1 ] || printf '%s\t%s\t%s\n' "$1" "$2" "${3:-}" >> "$manifest"; }

# put_file <source file> <path relative to the card root>
put_file() {
  src=$1; rel=$2; dest="$target/$rel"
  if [ -e "$dest" ] && cmp -s "$src" "$dest"; then
    echo "  same     $rel"
    return 0
  fi
  changed=$((changed + 1))
  if [ -e "$dest" ]; then
    echo "  replace  $rel"
    if [ "$dry" = 0 ]; then
      start_backup
      mkdir -p "$(dirname "$bk/$rel")"
      cp "$dest" "$bk/$rel"
      note R "$rel"
    fi
  else
    echo "  add      $rel"
    [ "$dry" = 1 ] || { start_backup; note A "$rel"; }
  fi
  if [ "$dry" = 0 ]; then
    mkdir -p "$(dirname "$dest")"
    cp "$src" "$dest"
    cmp -s "$src" "$dest" || die "copy of $rel does not match the original; the SD card or disk may be failing. Run: $0 --restore $target"
  fi
}

echo "installing into $target"
[ "$dry" = 0 ] || echo "(dry run: nothing will be written)"

list=$(mktemp)
(cd "$payload" && find . -type f | sed 's|^\./||' | sort) > "$list"
while IFS= read -r rel; do
  put_file "$payload/$rel" "$rel"
done < "$list"
rm -f "$list"

# loader.ini: NEW_MENU = 1 (keeps the file's own line endings)
ini="$target/loader.ini"
tmp=$(mktemp)
if tr -d '\r' < "$ini" | grep -Eiq '^[[:space:]]*NEW_MENU[[:space:]]*=[[:space:]]*1'; then
  echo "  same     loader.ini (NEW_MENU already 1)"
else
  crlf=0
  grep -q "$(printf '\r')" "$ini" && crlf=1
  if tr -d '\r' < "$ini" | grep -Eiq '^[[:space:]]*NEW_MENU[[:space:]]*='; then
    tr -d '\r' < "$ini" | sed -E 's/^[[:space:]]*NEW_MENU[[:space:]]*=.*/NEW_MENU = 1/' > "$tmp"
  else
    tr -d '\r' < "$ini" > "$tmp"
    # last byte must be a newline before the new line is added
    [ -z "$(tail -c 1 "$tmp")" ] || printf '\n' >> "$tmp"
    printf 'NEW_MENU = 1\n' >> "$tmp"
  fi
  if [ "$crlf" = 1 ]; then awk '{ printf "%s\r\n", $0 }' "$tmp" > "$tmp.2" && mv "$tmp.2" "$tmp"; fi
  changed=$((changed + 1))
  echo "  edit     loader.ini (NEW_MENU = 1)"
  if [ "$dry" = 0 ]; then
    start_backup
    cp "$ini" "$bk/loader.ini"
    note R loader.ini
    cp "$tmp" "$ini"
  fi
fi
rm -f "$tmp"

# optional arcade tweaks
if [ "$arcade" = 1 ]; then
  [ -d "$tweaks" ] || die "arcade-tweaks/ folder missing next to this script"
  echo "arcade tweaks:"
  for f in blackwidow.ini battlezone.ini; do
    put_file "$tweaks/$f" "$f"
  done
  for f in starwars.ini lunar.ini; do
    if [ -e "$target/$f" ] && [ ! -e "$target/$f.author-calibration" ]; then
      echo "  rename   $f -> $f.author-calibration"
      changed=$((changed + 1))
      if [ "$dry" = 0 ]; then
        start_backup
        mv "$target/$f" "$target/$f.author-calibration"
        note M "$f" "$f.author-calibration"
      fi
    fi
  done
fi

if [ "$dry" = 1 ]; then
  echo "dry run finished: $changed change(s) would be made."
elif [ "$changed" = 0 ]; then
  echo "nothing to do: this folder already has the release installed."
else
  echo "done: $changed change(s). Originals saved in $bk"
  echo "To undo:  $0 --restore \"$target\""
fi
