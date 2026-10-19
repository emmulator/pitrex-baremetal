#!/bin/sh
# Installs the new PiTrex menu release over an unpacked Malban "PiTrex_sd_Jul29_25" archive
# (or over the SD card made from it). Linux, macOS, WSL.
#
#   ./install.sh [--settings tuned] [--arcade-tweaks] [--dry-run] <folder>
#   ./install.sh --restore <folder>
#
# Replaced files are saved in <folder>/backup-menu-<date>/ with a MANIFEST.tsv that --restore reads.
# Nothing outside the files listed in payload/ and payload-if-missing/ (only added when missing) is touched (and, with
# --arcade-tweaks, arcade-tweaks/; with --settings tuned, settings/).
set -eu

here=$(cd "$(dirname "$0")" && pwd)
payload="$here/payload"
tweaks="$here/arcade-tweaks"
mode=install
dry=0
arcade=0
settings=keep
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
    --settings) shift; [ $# -gt 0 ] || usage; settings=$1; case $settings in keep|tuned) ;; *) echo "--settings takes keep or tuned" >&2; usage ;; esac ;;
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
hashf() { if command -v sha256sum >/dev/null 2>&1; then sha256sum "$1" | cut -d' ' -f1; else shasum -a 256 "$1" | cut -d' ' -f1; fi; }

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
      R|D) cp "$bk/$a" "$target/$a" && echo "  restored $a" ;;
      A) if [ -n "$b" ] && [ -f "$target/$a" ] && [ "$(hashf "$target/$a")" != "$b" ]; then
           echo "  keep     $a (changed since it was installed: yours now)"
         else rm -f "$target/$a" && echo "  removed  $a"; fi ;;
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
    [ "$dry" = 1 ] || { start_backup; note A "$rel" "$(hashf "$src")"; }   # restore keeps it if it was changed since
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

# files that become the user's own once installed (menu/favorites.ini): put on the card only if it has none yet
if [ -d "$here/payload-if-missing" ]; then
  list=$(mktemp)
  (cd "$here/payload-if-missing" && find . -type f | sed 's|^\./||' | sort) > "$list"
  while IFS= read -r rel; do
    if [ -e "$target/$rel" ]; then echo "  keep     $rel (yours: never replaced)"
    else put_file "$here/payload-if-missing/$rel" "$rel"; fi
  done < "$list"
  rm -f "$list"
fi

# 0.1.0 had one menu/<name>.ini per entry; 0.2.0 has them all in menu/menu.default.ini. Old files that are exactly
# as 0.1.0 shipped them are removed (saved in the backup); files you changed are kept and still read.
legacy="$here/legacy-menu-files.sha256"
if [ -f "$legacy" ] && [ -d "$target/menu" ]; then
  kept=0
  while read -r sum name; do
    f="$target/menu/$name"
    [ -f "$f" ] || continue
    if [ "$(hashf "$f")" = "$sum" ]; then
      changed=$((changed + 1))
      echo "  remove   menu/$name (now in menu.default.ini)"
      if [ "$dry" = 0 ]; then
        start_backup
        mkdir -p "$bk/menu"
        cp "$f" "$bk/menu/$name"
        note D "menu/$name"
        rm -f "$f"
      fi
    else
      kept=$((kept + 1))
      echo "  keep     menu/$name (you changed it; it is still read after the menu file)"
    fi
  done < "$legacy"
fi
if [ -f "$target/menu/menu.ini" ]; then
  echo "  keep     menu/menu.ini (your changes; they are read after the new menu.default.ini)"
fi

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

# vectrexInterface.ini (the screen settings): an existing file is the user's and stays, unless it is missing or
# empty (the original settings screen can leave it empty), or --settings tuned asks for ours.
cur="$target/vectrexInterface.ini"
size=-1
[ -f "$cur" ] && size=$(wc -c < "$cur" | tr -d ' ')
if [ "$settings" = tuned ] || [ "$size" -lt 100 ]; then
  [ -d "$here/settings" ] || die "settings/ folder missing next to this script"
  if [ "$size" -ge 100 ] && [ -n "${PITREX_FRESH_CARD:-}" ]; then echo "settings: installing the tuned file"
  elif [ "$size" -ge 100 ]; then echo "settings: replacing yours with the tuned file (saved in the backup)"
  elif [ "$size" -ge 0 ]; then echo "settings: your vectrexInterface.ini is empty; installing the tuned one"
  else echo "settings: no vectrexInterface.ini; installing the tuned one"; fi
  put_file "$here/settings/vectrexInterface.ini" vectrexInterface.ini
  # vectrex.ini (which BIOS Exact loads): only replaced on request; the tuned one uses the original BIOS 7ADB.bin
  if [ "$settings" = tuned ]; then put_file "$here/settings/vectrex.ini" vectrex.ini; fi
else
  echo "  keep     vectrexInterface.ini (your settings; use --settings tuned to replace them)"
fi

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
elif [ -n "${PITREX_FRESH_CARD:-}" ]; then
  echo "done: $changed change(s)."                # make_card.py: a new card, its backup is removed
else
  echo "done: $changed change(s). Originals saved in $bk"
  echo "To undo:  $0 --restore \"$target\""
fi
