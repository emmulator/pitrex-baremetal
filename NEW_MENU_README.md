# The new PiTrex menu: changing it and adding to it

This is the guide for people who want to change how the menu looks, add their own games, or edit the entry
files. To **install** the menu and find out which ROMs it wants, read [docs/README.md](docs/README.md) first.

## How it works

The new menu is a list built by the loader every time the PiTrex starts. **Nothing is compiled in.** It is
assembled from four things on the SD card:

| On the card | Becomes |
|---|---|
| `menu/*.ini` (entry files) | one game each: title, category, which program starts it, notes, controls |
| `piZero1/*.img` (program images) | the programs those entries start; an image with no entry file is listed under PROGRAMS |
| `vectrex/**/*.bin` | every Vectrex game, under VECTREX > ALL ROMS |
| `roms/*.zip`, `roms/aae/*.zip` | decide whether an arcade entry is shown (its `needs` line) |

The screen says READING SD CARD while it reads them. Everything is sorted and rebuilt at every start, so
changing the menu means editing text files on the card; the loader is never recompiled. To go back to the
original menu, remove `NEW_MENU = 1` from `loader.ini`.

What decides whether an entry is shown:

* its image must exist in `piZero1/` (otherwise the entry is left out);
* if it has a `needs` file, that file must exist (otherwise the entry is left out; `NEW_MENU_SHOW_MISSING = 1`
  in `loader.ini` shows such entries anyway, with "ROM NOT FOUND!" under them);
* a `needs` ending in `/` (the AAE picker has `roms/aae/`) means "show me while any other entry needing a file
  in that folder is showing";
* `hide = 1` in the entry hides it;
* a category with nothing visible in it disappears.

Categories are sorted alphabetically, then PROGRAMS, then SETTINGS. Everything below the top level is
alphabetical too (inside ALL ROMS the folders come first).

## Controls

(The user view is in [docs/README.md](docs/README.md).) Stick up/down moves, right or Button 4 enters or starts,
left goes back, Button 2 starts a Vectrex game with the other emulator or, on an arcade game, shows its CONTROLS
page, Button 1 is Button 4 plus "load the last saved state". The menu ignores the controls until they have
rested for 15 frames, so a Button 4 held at power-on does not open anything.

## Adding a game

**A Vectrex ROM:** copy the `.bin` into `vectrex/` or a sub-folder (two levels). It shows up under
VECTREX > ALL ROMS with a title made from its file name: upper case, `_` and `-` become spaces, `.bin` dropped
(`E-SRG.bin` is `E SRG`). Sub-folders become groups named `<FOLDER> FOLDER`; rename one with
`menu/dir.<folder>.ini` containing `title = ...`. A loose file whose name also exists in a folder is treated as a
copy and left out. `bios`, `movies` and `waves` are ignored. Button 4 starts it with Exact and Button 2 with
Speedy; `emulator = speedy` in its notes file swaps that.

**A program image** (`something.img` built for the Pi): copy it into `piZero1/` (and `piZero2/` for a Pi Zero 2).
It is listed under PROGRAMS, named after the file. To put it elsewhere, give it an entry file:

```
[entry]
title = MY GAME
category = PITREX
image = mygame.img
```

`category = ARCADE / ATARI` puts it in a sub-folder, and a category that does not exist yet is created.

**An arcade game on AAE:** `image = aae.img`, `param1 = 0x2d` where the number is the game's position in the AAE
driver table (`aae/aaemain.c`; the table is listed in [docs/menu/04-arcade-candidates.md](docs/menu/04-arcade-candidates.md)),
or the MAME name (`param1 = mhavoc2`; the picker docs are in [docs/menu/06-aae-picker.md](docs/menu/06-aae-picker.md)), and
`needs = roms/aae/<name>.zip`. The shipped files `menu/aae.*.ini` are a template.

## Entry file format

Every file `menu/<name>.ini` is plain text, read when the menu starts. The file name does not matter for entry
files (the shipped ones are named after the image and parameter).

```
[entry]
title = MAJOR HAVOC            ; shown in the list (capitals; brackets are dropped)
category = ARCADE / ATARI      ; where it is listed. Empty or missing: under PROGRAMS
image = aae.img                ; the program image in piZero1/ that starts it
param1 = 0x2d                  ; first parameter. 0x2d means the single byte 0x2d; anything else is text
param2 = original/X.BIN        ; second parameter, if the program takes one
needs = roms/aae/mhavoc.zip    ; the entry is only shown while this file exists
skip = 4                       ; blank rows before the first note line
text = WORK IN PROGRESS!!      ; note lines under the list (max 12). WIP, NO SOUND... add a ! to the list
control = FIRE  BUTTON 4       ; lines of the CONTROLS page (Button 2), max 12
hide = 1                       ; leave it out
```

A **notes file** has no `image` line and changes something that was *found* on the card:

| For | File name | Example |
|---|---|---|
| A Vectrex game file | `rom.` + the path under `vectrex/` with every run of other characters as `_`, lower case | `vectrex/homebrew/E-SRG.bin` is `menu/rom.homebrew_e_srg_bin.ini` |
| A program (`piZero1/foo.img`) | image name, lower case | `menu/foo.ini` |
| A ROM folder group | `dir.` + folder name | `menu/dir.original.ini` |

```
[entry]
title = MY GAME                ; rename it
text = WORKS GREAT.            ; note lines
hide = 1                       ; leave it out
emulator = speedy              ; for a ROM: which emulator Button 4 uses, speedy or exact
control = ...                  ; CONTROLS page lines
```

`;` starts a comment. A note containing NO SOUND, WIP / WORK IN PROGRESS, or BROKEN / NOT WORKING / CRASHING
makes the entry show a `!` in the list. Full format: [docs/menu/05-sidecar-format.md](docs/menu/05-sidecar-format.md).

**Text rules.** The PiTrex's vector font only draws `A-Z`, `0-9`, `.`, `!` and the arrows `<` `>`
(`$` is the folder symbol). Brackets, slashes, dashes, colons and commas print as blanks, so write text in
capitals with only those characters. Lines longer than about 46 characters run off the screen.

## Changing how it looks

Through files only you can change every title, category, note, controls line, what is hidden, where things are
listed and which emulator a ROM prefers. You cannot change the text size, the number of rows or the
sort order without rebuilding the loader. Those live in `pitrex/loader/newMenuUi.h`:

| What | Where |
|---|---|
| Rows shown (7), list position and spacing, notes position, button hints | `NM_ROWS` in `newMenuLogic.h`; `NM_LIST_Y`, `NM_LIST_STEP`, `NM_NOTE_Y`, `NM_NOTE_STEP` and the `v_printString` calls in `newMenuUi.h` (size 7 is the large text, 5 the small) |
| Help texts for the categories | `nmHelp()` in `newMenuLogic.h` |
| Sorting | `nmTitleCmp`, `nmSortChildren` in `newMenuLogic.h` |
| Limits | 700 menu nodes in total (entries, folders and ROMs), 320 files in `menu/`, 400 ROM files, 128 rows in a list |

Build the loader with `docs/tools/build.sh zero1 pitrex/loader` (or `zero2`) in the `pitrex-dev` container (see
the top of that script) and put `loader.pit` in `piZero1/` (or `piZero2/`) on the card. The menu code also runs on a
PC against Roger Boesch's PiTrex simulator, so a layout change can be looked at without a PiTrex:
[docs/menu/08-menu-preview.md](docs/menu/08-menu-preview.md). The logic (building and sorting the tree) has host
tests: `docs/tools/test_new_menu.c`.

## loader.ini

| Line | Effect |
|---|---|
| `NEW_MENU = 1` | use the new menu (remove it for the original menu) |
| `NEW_MENU_SHOW_MISSING = 1` | also list arcade games whose ROM is not on the card |

## Regenerating the shipped entry files

The files in `menu/` that come with the package are generated, so a change to all of them is one edit:

```
python3 -I docs/tools/export_sidecars.py ...        # from the original menu's list (docs/menu/sidecar/menu)
python3 -I docs/tools/more_sidecars.py  docs/menu/sidecar/menu   # the other AAE versions, QB3, sample programs
python3 -I docs/tools/controls_sidecars.py docs/menu/sidecar/menu   # the CONTROLS lines
```

(`export_sidecars.py` was run once on the original menu's list; the other two only edit the folder. Run them in this order.) The ROM tables in
`docs/README.md` come from `docs/tools/gen_rom_tables.py`.

## Where the code is

| File | What it does |
|---|---|
| `pitrex/loader/newMenuLogic.h` | building the tree from the entry files, program and ROM discovery, hiding, sorting, navigation, help texts. No hardware calls, tested on a PC. |
| `pitrex/loader/newMenuUi.h` | drawing, buttons, launching, scanning the card, the staged start-up |
| `pitrex/loader/menuSidecar.h`, `menuSidecarIo.h` | the entry file format and file reading |
| `pitrex/loader/loaderMain.c` | calls the new menu each frame when `NEW_MENU = 1` (the original compiled menu is untouched) |
| `aae/aaeSelect.h`, `aae/aaemain.c` | the arcade game picker |
| `docs/` | install guide, findings, format specs, tests, the build script and the desktop preview ([docs/DEVELOPMENT.md](docs/DEVELOPMENT.md)) |

Licensing of the upstream code: [docs/LICENSING.md](docs/LICENSING.md).
