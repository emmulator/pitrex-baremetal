# Sidecar files for menu notes

Status: implemented in the loader on branch `menu-redesign` (commit "loader: read note lines
from menu/<key>.ini"). **Built for both Pi targets and tested on a PC; not yet tried on
hardware.**

## What it does

The loader shows a few note lines under an entry's name ("AAE EMU", "NO SOUND", "WORK IN
PROGRESS!!", credits). Those lines used to be compiled in. Now, when an entry is shown, the
loader looks for `menu/<key>.ini` on the SD card. If the file exists, its lines replace the
compiled ones. If it does not exist, nothing changes. Edit a file to correct a note, for
example after a game is fixed, or delete it to go back to the compiled text. No recompile.

Only the note lines are overridable so far. Names, images, parameters, icons, scroll text,
category order and which entries exist are still compiled in. The same files are meant to
become the data of the dynamic menu (plan, phase 3), so nothing here is throwaway.

## The key

`key = <image base name> [ "." <slug of param1> ] [ "." <slug of param2> ]`

* Image base name: the entry's image without folder and `.img`, lower-cased
  (`piZero1/aae.img` gives `aae`).
* A parameter that is exactly **one byte** (the AAE game number) becomes `x` plus two hex
  digits (`"-"`, 0x2d, gives `x2d`).
* Any other parameter is lower-cased, each run of characters that are not a-z or 0-9 becomes
  one `_`, and leading/trailing `_` are dropped (`original/CASTLE.BIN` gives `original_castle_bin`).
* The entry PICK A GAME (id 9999) never uses a sidecar. Entries with no image (pure
  categories) have no key.

Examples:

| Entry | Key | File |
|---|---|---|
| Major Havoc (AAE game 0x2d) | `aae.x2d` | `menu/aae.x2d.ini` |
| Speedy Vectrex, Star Castle | `vectrexspeedy.original_castle_bin` | `menu/vectrexspeedy.original_castle_bin.ini` |
| Demon | `cine.demon` | `menu/cine.demon.ini` |
| Lunar Lander | `lunar` | `menu/lunar.ini` |

The rules live in `pitrex/loader/menuSidecar.h` (`sidecarKey`). A key can in principle be
shared by two entries; the exporter reports that, and none are shared today.

## The file

A normal INI file, read with the loader's own INI parser, so `;` starts a comment.

```
; PiTrex menu sidecar: overrides the note lines shown for this entry.
[entry]
title = MAJOR HAVOC          ; for humans, ignored by the loader
category = ARCADE / ATARI    ; ignored
image = aae.img              ; ignored
param1 = 0x2d                ; ignored
skip = 4                     ; blank rows before the first line
text = WORK IN PROGRESS!!    ; one line per "text"; repeat the key
text = FLICKER, NO SOUND!
```

* `text` lines are printed top to bottom in order, 18 units apart, starting at the vertical
  centre, **below** `skip` blank rows. An empty `text =` is a blank row.
* A value wrapped in double quotes keeps its inner spaces: `text = "  SPEEDY VECTREX"`.
* New menu only (file 07): `hide = 1` leaves the entry out, `title = ...` renames a program
  found on the card, `emulator = speedy|exact` picks the emulator for a ROM. They do not
  replace the notes: a file with only these keys leaves the compiled notes alone.
* Unknown names are ignored. At most 12 rows, 47 characters per row, 2047 bytes per file.
* Keep `;` out of the note text when it follows a space.
* If a file has any `skip` or `text` line it **replaces** the compiled notes entirely. To
  show no notes at all, put one empty `text =` line.
* Line endings may be LF or CRLF.

## Getting the files

`docs/menu/sidecar/menu/` holds the 65 files generated from the notes compiled into the
current loader (every entry that has notes). Copy that `menu/` folder to the card root.
Regenerate after changing the source:

    python3 -I docs/tools/export_sidecars.py pitrex/loader/loaderMain.c docs/menu/sidecar

## Why the loader does not use `ini_parse()`

Checked in `pitrex/vectrex/ini.c`: the stock `ini_parse()` keeps a **10 KB buffer on the
stack** and writes the file name into `lastAccessedIni`, which the settings screen uses to
decide where to save. Reading a sidecar that way could make a later settings save go to the
sidecar. The loader instead reads the file itself into a 2 KB static buffer and calls
`ini_parse_string()`.

## Test

`docs/tools/test_sidecar_key.c` compiles the loader's key and line code and the real
`ini.c` on a PC. It checks that all 67 image/parameter combinations give the exporter's
keys, and that all 65 files parse to exactly the expected rows with LF and CRLF line
endings. It fails when a note or a key is changed (checked by editing a file). The
instructions to build and run it are at the top of the file.

What is not tested: the SD card path (`fopen("menu/...")` through the loader's FAT layer),
the current directory at the moment of the call, drawing, and stack use on the Pi.

## `control` lines (new menu)

`control = <text>` adds a line to the CONTROLS page that Button 2 shows for an entry that is not a Vectrex
game file. Up to 12 lines of 46 characters (A-Z, 0-9, space, `.`, `!` only). They do not replace the notes and
do not count as notes (`found` stays 0). `docs/tools/controls_sidecars.py <menu folder>` writes the lines for
the arcade games; the sources of each mapping are listed at the top of that script.

## Entry files (2026-10-17, late)

A sidecar file with an `image` line is an **entry file**: the new menu builds its list from these. Fields:

| Line | Meaning |
|---|---|
| `title` | name in the list (cleaned: upper case, brackets dropped, undrawable characters become spaces, a trailing "SBT" is dropped) |
| `category` | `A / B / C` path; created when missing; empty = PROGRAMS |
| `image` | image in `piZero1/` (or `piZero2/`) that starts it |
| `param1`, `param2` | parameters passed to the program. `0xNN...` (even number of hex digits) is raw bytes: the AAE game number is one byte |
| `needs` | a file that must exist; "ROM NOT FOUND!" is shown otherwise |
| `text`, `skip`, `control`, `hide` | as above |

`docs/tools/export_sidecars.py` writes one entry file for every game of the original compiled menu (plus
`aae.ini` and `vectorburner.ini`); `docs/tools/controls_sidecars.py` adds the controls. File names are not used
except as the key for notes: any name works for an entry file.
