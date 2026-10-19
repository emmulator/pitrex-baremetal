# The new PiTrex menu: changing it and adding to it

This is the guide for people who want to change how the menu looks, add their own games, or arrange things their
own way. To **install** the menu and find out which ROMs it wants, read [docs/README.md](docs/README.md) first.

## How it works

The new menu is a list built by the loader every time the PiTrex starts. **Nothing is compiled in.** It is
assembled from what is on the SD card:

| On the card | Becomes |
|---|---|
| `vectrex/**/*.bin`, `*.vec` | every Vectrex game, under VECTREX > ALL ROMS (only files that start like a real cartridge, with `g GCE`) |
| `vectrex/movies/*`, `vectrex/waves/*` | the videos and songs, under VIDEO and AUDIO, played by the video and audio players |
| `piZero1/*.img` (program images) | programs; one the menu file does not mention is listed under PROGRAMS |
| `menu/menu.default.ini`, then your `menu/menu.ini` | titles, notes, controls and categories: how everything is arranged |
| `roms/*.zip`, `roms/aae/*.zip` | decide whether an arcade entry is shown (its `needs` line) |

**Without any menu file** the menu still lists every Vectrex game, video, song and program on the card, and the
arcade games through the AAE game picker under PROGRAMS. The menu file adds the arrangement, the notes and the
arcade entries.

The screen says READING SD CARD while it reads all this. To go back to the original menu, remove `NEW_MENU = 1`
from `loader.ini`.

## The menu files: the default, and your changes on top

The menu is described in text files in `menu/`, read in layers, each one able to change what the ones before it
said:

1. **`menu.default.ini`** comes with the package: every entry, its notes, controls and place in the menu. **Do not
   edit it**: every update replaces it.
2. Any other `.ini` file in `menu/`, in alphabetical order. A game you add can come with its own small file
   (`menu/mygame.ini`).
3. **`menu.ini`: your changes.** It does not exist until you create it, updates never touch it, and it is read
   last, so it always wins. It only needs the lines you want to be different; everything you do not mention
   stays as the default says, and new games in a later update still appear.

The files are plain text in the same style as `loader.ini`: `key = value` lines, and `;` starts a comment.

### Making changes in menu.ini

To change something, write a section with **the same name in brackets** as in `menu.default.ini` (usually the
title you see in the menu; open the default file to check) and only the lines to change:

| To | Write in menu.ini |
|---|---|
| change the notes | `[MAJOR HAVOC]` / `text = WORKS GREAT NOW.` (your `text` lines replace all of its note lines; `control` lines work the same way) |
| rename it | `[MAJOR HAVOC]` / `title = HAVOC` |
| move it somewhere else | `[TEMPEST]` / `category = FAVOURITES` |
| **also** list it somewhere else | `[TEMPEST]` / `also = FAVOURITES` (it stays where it was) |
| hide it | `[MAZE]` / `hide = 1` |
| show something the default hides | `[HELLO WORLD]` / `hide = 0` |
| keep a category in your own order | `[FAVOURITES]` / `order = listed` |
| add your own entry | a new name: `[MY GAME]` / `category = PITREX` / `image = mygame.img` |

### Collections

`also` lists an entry in more categories at once, so collections can overlap: a game can be in its usual place, in
FAVOURITES and in TWO PLAYERS. A category that does not exist yet is created. For example, a `menu.ini` with:

```
[FAVOURITES]
order = listed                      ; in the order of this file, not alphabetical
text = THE GAMES I PLAY MOST.       ; the help shown when FAVOURITES is highlighted

[TEMPEST]
also = FAVOURITES

[MAJOR HAVOC]
also = FAVOURITES, TWO PLAYERS

[BERZERK]                           ; a Vectrex game the menu finds by itself under ALL ROMS
file = vectrex/original/BERZERK.BIN
also = FAVOURITES, VECTREX CLASSICS
```

gives a FAVOURITES category with Tempest, Major Havoc and Berzerk, a TWO PLAYERS category with Major Havoc, and a
VECTREX CLASSICS category with Berzerk, while all three stay in their usual places too. The copies have the same
notes and controls, and a change to the entry changes every copy. A game is never listed twice in the same
category.

### Section format

```
[MAJOR HAVOC]                       ; the name in brackets is the title shown in the menu
category = ARCADE / ATARI           ; where it is listed. A new path makes new folders
image    = aae.img                  ; the program that starts it (in piZero1/)
param1   = 0x2d                     ; what the program is given: 0x2d is the single byte 0x2d, anything else is text
needs    = roms/aae/mhavoc.zip      ; only listed while this file is on the card
text     = WORK IN PROGRESS!!       ; a note line under the list (up to 12)
control  = FIRE  BUTTON 4           ; a line of the CONTROLS page (Button 2), up to 12
```

Each section in `[brackets]` is one thing in the menu. One line says what it is:

| The section has | It is | Example |
|---|---|---|
| `image = ...` | an entry that starts a program | `[MAJOR HAVOC]` above |
| `file = ...` | a file the menu finds by itself (a Vectrex game, a video, a program); the section gives it a title, notes, a category | `[BERZERK]` / `file = vectrex/original/BERZERK.BIN` |
| `folder = ...` and `image = ...` | a rule: every file in that folder becomes an entry | `[VIDEOS]` / `folder = vectrex/movies` |
| `folder = ...` without `image` | a Vectrex ROM folder: its name in ALL ROMS, or `hide = 1` | `[MY HOMEBREW]` / `folder = vectrex/homebrew` |
| none of these | a change to the section of the same name in an earlier file; if there is none, settings of a category | `[MAJOR HAVOC]` / `text = ...`, `[ARCADE / ATARI]` / `order = listed` |

All the keys:

| Key | Meaning |
|---|---|
| `title = ...` | the title, when it should differ from the section name (two entries can have the same title: `[SOLAR QUEST ARCADE]` with `title = SOLAR QUEST`) |
| `category = A / B` | where it is listed. Empty: PROGRAMS. For a found Vectrex game: also listed there (it stays in ALL ROMS). For a found program: moved there |
| `also = A, B / C` | also listed in these categories (a copy with the same notes; commas between categories) |
| `image`, `param1`, `param2` | the program and what it is given. In a folder rule `{dir}` is the folder and `{file}` the file name |
| `files = bin vec` | folder rule: only files with these extensions (default: all) |
| `needs = path` | only listed while that file is on the card. A path ending in `/` (the AAE picker's `roms/aae/`): listed while any other entry needing a file in that folder is |
| `text = ...` | a note line (up to 12). For a category: replaces its help text |
| `control = ...` | a line of the CONTROLS page that Button 2 opens (up to 12) |
| `hide = 1` | leave it out (`hide = 0` shows it again when an earlier file hid it) |
| `emulator = speedy` or `exact` | for a Vectrex game: which emulator Button 4 uses |
| `order = listed` | for a category: its entries in the order the menu files mention them (the last mention counts, so the order of your `menu.ini`) instead of alphabetical |

When two sections describe the same thing (the same section name, the same `file`, the same `folder`), the later
one changes the earlier one: what it sets replaces what was there, everything else stays.

A note containing NO SOUND, WORK IN PROGRESS, NOT WORKING or CRASH (any case, also inside a longer word such as
CRASHES) makes the entry show a `!` in the list.

**Text rules.** The PiTrex's vector font only draws `A-Z`, `0-9`, `.`, `!` and the arrows `<` `>` (`$` is the
folder symbol); lower case is drawn as capitals. Brackets, slashes, dashes, colons and commas print as blanks, so
write text with only those characters. Note lines longer than about 46 characters run off the screen; titles are
cut at 31 characters and about 28 fit in the list.

## What decides whether something is shown

* an entry's program must be in `piZero1/` (otherwise it is left out);
* if it has a `needs` file, that file must be there (`NEW_MENU_SHOW_MISSING = 1` in `loader.ini` lists such entries
  anyway, with "ROM NOT FOUND!" under them);
* `hide = 1` hides it;
* a category with nothing visible in it disappears;
* the same program with the same parameters twice in one category is listed once.

Categories are sorted alphabetically, then PROGRAMS, then SETTINGS. Everything below the top level is
alphabetical too (inside ALL ROMS the folders come first), unless the category says `order = listed`: then its
entries come in the order the menu files mention them, with the last mention counting.

## Adding things

**A Vectrex game:** copy the `.bin` or `.vec` into `vectrex/` or a sub-folder (two levels). It shows up under
VECTREX > ALL ROMS with a title made from its file name: upper case, `_` and `-` become spaces, the extension
dropped (`E-SRG.bin` is `E SRG`). Sub-folders become groups named `<FOLDER> FOLDER`. A loose file whose name also
exists in a folder is treated as a copy and left out. `bios`, `movies` and `waves` are not game folders. Button 4
starts a game with Exact and Button 2 with Speedy. To give it a title, notes or a place of its own:

```
[E SRG]
file     = vectrex/homebrew/E-SRG.bin
category = VECTREX / HOMEBREW
text     = A GAME BY JJ.
```

**A video or song:** copy it into `vectrex/movies/` or `vectrex/waves/`. It is listed under VIDEO or AUDIO by its
file name. For any other folder of files that one program plays, write a folder rule:

```
[MY VIDEOS]
folder   = vectrex/myvideos
category = VIDEO
image    = vectrexvideo.img
param1   = {dir}
param2   = {file}
```

**A program image** (`something.img` built for the Pi): copy it into `piZero1/` (and `piZero2/` for a Pi Zero 2).
It is listed under PROGRAMS, named after the file. To put it elsewhere:

```
[MY GAME]
category = PITREX
image    = mygame.img
```

**An entry that passes a file to a program** (a Vectrex game to an emulator, a video to the player): add a
`needs` line naming that file, so the entry disappears when the file is not on the card instead of starting the
program with nothing to load. The menu cannot tell this from `param1` itself, because for other programs it is a
name or a number, not a file:

```
[WEB WARS]
category = VECTREX / SPEEDY VECTREX
image    = vectrexspeedy.img
param1   = original/WEBWARS.BIN            ; the emulator looks for vectrex/original/WEBWARS.BIN
needs    = vectrex/original/WEBWARS.BIN    ; listed only while that file is there
```

**An arcade game on AAE:** `image = aae.img`, `param1 = 0x2d` where the number is the game's position in the AAE
driver table (`aae/aaemain.c`; listed in [docs/menu/04-arcade-candidates.md](docs/menu/04-arcade-candidates.md)), or
the MAME name (`param1 = mhavoc2`; see [docs/menu/06-aae-picker.md](docs/menu/06-aae-picker.md)), and
`needs = roms/aae/<name>.zip`. The AAE sections of `menu.default.ini` are a template.

## The 0.1.0 files

Version 0.1.0 had one file per entry (`menu/aae.x2d.ini` and so on) with an `[entry]` section. Those files still
work: they are read after `menu.default.ini`, and a game described both ways is listed once. The installer removes
the ones that are exactly as 0.1.0 shipped them, since their content is now in `menu.default.ini`, and keeps any you
changed. Whatever you changed in them is simplest to carry over by hand into `menu.ini`, as the few lines that
differ. (`docs/tools/entries_to_menu_ini.py <folder> <file>` turns a folder of such files into one file.)

## Controls

(The user view is in [docs/README.md](docs/README.md).) Stick up/down moves, right or Button 4 enters or starts,
left goes back, Button 2 starts a game under ALL ROMS with the other emulator or, on an entry with `control` lines,
shows its CONTROLS page, Button 1 is Button 4 plus "load the last saved state" (the Vectrex emulators only do that
with `HANDLE_STATES = 1` in `vectrex.ini`; the menu reads that line and only then says AND SAVED STATE). The menu
ignores the controls until they have rested for 15 frames, so a Button 4 held at power-on does not open anything.

## Changing how it looks

Through the menu file you can change every title, category, note, controls line, what is hidden, where things are
listed, their order and which emulator a Vectrex game prefers. You cannot change the text size or the number of
rows without rebuilding the loader:

| What | Where |
|---|---|
| Rows shown (7), list position and spacing, notes position, button hints | `NM_ROWS` in `newMenuLogic.h`; `NM_LIST_Y`, `NM_LIST_STEP`, `NM_NOTE_Y`, `NM_NOTE_STEP` and the `v_printString` calls in `newMenuUi.h` (size 7 is the large text, 5 the small) |
| Built-in help texts for the categories | `nmHelp()` in `newMenuLogic.h` (a category's `text` lines replace them) |
| Built-in folder rules (videos, songs) | `nmDefaultFolderRules()` in `newMenuLogic.h` |
| Sorting | `nmTitleCmp`, `nmSortChildren` in `newMenuLogic.h` |
| Limits | 700 menu nodes in total (entries, folders and ROMs), 400 sections in all menu files together (64 KB of note and control lines), 128 KB per menu file, 320 files in `menu/`, 160 files in `piZero1/` and in a folder-rule folder, 48 programs found without a section, 400 Vectrex games in at most 23 folders, 16 top-level categories, 4 category levels (a fifth cannot be opened), 128 rows in a list. In `menuFile.h`, `newMenuLogic.h` and `newMenuUi.h` (`NM_MAX_*`) |

Build the loader with `docs/tools/build.sh zero1 pitrex/loader` (or `zero2`) in the `pitrex-dev` container (see
the top of that script) and put `loader.pit` in `piZero1/` (or `piZero2/`) on the card. The menu code also runs on a
PC against Roger Boesch's PiTrex simulator, so a layout change can be looked at without a PiTrex:
[docs/menu/08-menu-preview.md](docs/menu/08-menu-preview.md). The logic has host tests: `docs/tools/test_new_menu.c`.

## loader.ini

| Line | Effect |
|---|---|
| `NEW_MENU = 1` | use the new menu (remove it for the original menu) |
| `NEW_MENU_SHOW_MISSING = 1` | also list arcade games whose ROM is not on the card |
| `NEW_MENU_LOG = 1` | write `NMLOG.TXT` on the card: the first controls read, launches, settings steps, menu sections that did not fit (for tracking down a problem; off by default because it writes to the card at every start) |

## Regenerating the shipped menu file

`menu.default.ini` is generated from one file per entry in `docs/menu/sidecar/menu`, so a change to all entries is
one edit in a script:

```
python3 -I docs/tools/export_sidecars.py ...        # from the original menu's list (was run once)
python3 -I docs/tools/more_sidecars.py  docs/menu/sidecar/menu   # the other AAE versions, QB3, sample programs, fixes
python3 -I docs/tools/controls_sidecars.py docs/menu/sidecar/menu   # the CONTROLS lines
python3 -I docs/tools/entries_to_menu_ini.py docs/menu/sidecar/menu docs/menu/sidecar/menu.default.ini
```

`make_release.sh` runs the last step itself. The ROM tables in `docs/README.md` come from
`docs/tools/gen_rom_tables.py`.

## Where the code is

| File | What it does |
|---|---|
| `pitrex/loader/menuFile.h` | reading the menu files into records, keys, folder-rule helpers. No hardware calls, tested on a PC. |
| `pitrex/loader/newMenuLogic.h` | building the tree: entries, folder rules, program and ROM discovery, hiding, sorting, navigation, help texts. No hardware calls, tested on a PC. |
| `pitrex/loader/newMenuUi.h` | drawing, buttons, launching, reading the card, the staged start-up |
| `pitrex/loader/menuSidecar.h`, `menuSidecarIo.h` | the keys of a section; the original menu's notes files |
| `pitrex/loader/loaderMain.c` | calls the new menu each frame when `NEW_MENU = 1` (the original compiled menu is untouched) |
| `aae/aaeSelect.h`, `aae/aaemain.c` | the arcade game picker |
| `docs/` | install guide, findings, format specs, tests, the build script and the desktop preview ([docs/DEVELOPMENT.md](docs/DEVELOPMENT.md)) |

Licensing of the upstream code: [docs/LICENSING.md](docs/LICENSING.md).
