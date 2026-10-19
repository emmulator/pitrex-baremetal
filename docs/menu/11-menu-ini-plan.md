# Plan for 0.2.0: one menu file, folder rules, a menu that works without metadata

Status: **plan**, agreed in outline on 2026-10-18; nothing below is implemented yet.

## Goals

1. **Self-sufficient.** With no metadata at all, the menu lists everything on the card that it can start: Vectrex
   games, programs, arcade games, videos and songs.
2. **Flexible with metadata.** With metadata, people can rename, describe, hide and arrange everything, put things
   in categories of their own, and add games, without recompiling anything.
3. **Easy to edit for non-technical people.** One file to open, in a format that is hard to break.

## What the menu finds with no metadata

| On the card | How it is recognized | Listed as |
|---|---|---|
| Vectrex games | `.bin` or `.vec` under `vectrex/` (two folder levels) **and** the file starts with the cartridge marker `g GCE` (what the Vectrex BIOS checks) | VECTREX > ALL ROMS, as today; Button 4 = Exact, Button 2 = Speedy |
| Programs | `piZero1/*.img` (`piZero2/` on a Pi Zero 2), minus the boot and emulator images | PROGRAMS, as today |
| Arcade games | `aae.img` started without a parameter opens its own game picker; the dedicated emulators read their zips themselves | PROGRAMS (as today; not every dedicated emulator has been checked started this way) |
| Videos | every file in `vectrex/movies/` | VIDEO, each file started with `vectrexvideo.img` (new built-in default) |
| Songs | every file in `vectrex/waves/` | AUDIO, each file started with `vectrexaudio.img` (new built-in default) |

Notes:

* The `g GCE` check replaces today's skip-by-folder-name for `bios/`, `movies/` and `waves/`: the BIOS files and
  the video/sound data do not start with it (checked 2026-10-18 on Malban's card: all 7 demos and the games start
  with `g GCE <year>`; `movies/*.bin` start with `ZZZZ` or random bytes, `waves/*.bin` with zeros). A mislabelled
  file anywhere is then skipped and a real game in an oddly named folder is still found. Cost: one 11-byte read per
  file at start-up; to be measured on the card (about 100 files).
* `.vec` is the same raw cartridge image as `.bin` under another name. `.NIB` stays excluded: the three `.NIB`
  files on Malban's card are the programs built into the players and Vectorblade (`MOVIEPLAYER.NIB`,
  `WAVPLAYER.NIB`, `VECTORBLADE.NIB`), and the two player programs do nothing without their data.
* Videos and songs have no signature, so for them the folder name is the only clue. The two defaults are the one
  place where the menu knows a folder name; a folder rule in the menu file (below) overrides them. No entries are
  compiled in.
* `VECTORBLADE.NIB` (262144 bytes, `g GCE 2020`) is the plain 256 KB Vectorblade cartridge; Exact already detects
  that size ("Vectorblade style" banking, `vectrexBase/main.c`). Renamed to `.bin` it would be listed under ALL ROMS
  and might run in Exact; **untested**. The `vectorblade.img` program adds high scores saved on the card and its own
  fonts.

## The menu file: `menu/menu.ini`

One file describes and arranges everything. Format: INI, because it is the format PiTrex owners already edit
(`loader.ini`, `vectrex.ini`), it needs no quotes, commas or indentation, `;` starts a comment, and the loader
already has an INI parser. JSON (quotes, commas, braces, no comments), YAML (meaningful indentation, values that
silently change type) and TOML (quoting, `[[...]]` tables) are all easier to break by hand and would need a new
parser in the loader.

INI has no agreed way to nest, so the hierarchy is a path in a value, as today: `category = ARCADE / ATARI`. Moving
an entry is changing one line; a new path creates the category.

```
; menu/menu.ini: the whole menu. A ; starts a comment.

[MAJOR HAVOC]                       ; a game started through a program
category = ARCADE / ATARI
image    = aae.img
param1   = mhavoc
needs    = roms/aae/mhavoc.zip
text     = WORK IN PROGRESS!!
control  = FIRE  BUTTON 4

[BERZERK]                           ; notes for a file found on the card
file     = vectrex/original/BERZERK.BIN
category = VECTREX / SPEEDY VECTREX
emulator = speedy

[VIDEOS]                            ; a folder rule: every file in the folder becomes an entry
folder   = vectrex/movies
category = VIDEO
image    = vectrexvideo.img
param1   = {dir}
param2   = {file}

[BAD APPLE]                         ; title and notes for one file of that folder
file     = vectrex/movies/BadApple.bin
text     = THE CLIP EVERY MACHINE MUST SHOW!

[ARCADE / ATARI]                    ; settings of a category
order    = listed                   ; keep the order of this file instead of sorting
```

### Sections

The section name is the title shown in the menu. The kind of section is given by one key:

| Key | The section is | Keys it takes |
|---|---|---|
| `image =` | an entry that starts a program | `category`, `param1`, `param2`, `needs`, `text`, `control`, `hide`, `title` |
| `file =` | the description of a file the menu found (a ROM, a video, a program image) | `category` (moves it; it then also stays in ALL ROMS for ROMs), `emulator`, `text`, `control`, `hide`, `title` |
| `folder =` | a rule for every file in a folder | `category`, `image`, `param1`/`param2` with `{dir}` (the folder, e.g. `vectrex/movies`) and `{file}` (the file name), `files` (extensions, default all), `text` (shown for every file), `hide`, `title` (the group name, replaces `menu/dir.<folder>.ini`) |
| none, name contains ` / ` or matches a category | settings of a category | `order = listed` or `sorted` (default), `hide`, `text` (help shown under it) |

`title =` inside a section overrides the section name, so two entries can show the same title (SOLAR QUEST is
both a Vectrex game and an arcade game): `[SOLAR QUEST ARCADE]` with `title = SOLAR QUEST`.

### Which wins

1. `menu/menu.ini`, then any other `menu/*.ini` in alphabetical order; a later section with the same `file`,
   `folder` or section name changes the earlier one key by key. A homebrew author can ship `mygame.ini`; people who
   like one file per game keep that.
2. Folder rules.
3. The built-in discovery above.

A file that an `image =` entry already starts (same image and parameters) is not added a second time by a folder
rule, as `nmAddMissingRoms()` does for DEMOS today.

### Reading it

* The loader reads `menu.ini` with its own small line reader, not `ini_parse_string()`: the shared parser cannot
  tell two sections with the same name apart, and it is used by the settings code, which must not change.
* Size: everything shipped today is about 55 KB as separate files. Read into one buffer of up to 128 KB (today
  2 KB per file); entries keep pointers into it instead of copying fixed-size strings.
* Old files keep working: a section named `[entry]` takes its title from `title =`, and the old notes-file names
  (`rom.<slug>.ini`, `<image>.ini`, `dir.<folder>.ini`) are still matched. 0.1.0 cards upgrade without edits.
* Lines over 200 characters, unknown keys and broken lines are skipped, not fatal; the menu writes what it skipped
  to `NMLOG.TXT` when `NEW_MENU_LOG = 1`.

## Installing and upgrading

`menu/menu.ini` is the file people will edit, so the installers must not overwrite an edited one:

* The package ships `menu/menu.default.ini` (always replaced) and installs it as `menu/menu.ini` only if there is
  no `menu.ini` yet. An existing one is kept and the installer says so, with the command to take the new default.
* `menu.default.ini` is not read by the loader, it is a reference copy (or: read only when `menu.ini` is missing;
  to decide).
* The 114 separate entry files of 0.1.0 are removed on upgrade (they are in the backup) once their content is in
  `menu.default.ini`, unless the user changed them (compare with the 0.1.0 checksums).

## Tools

* The generators (`more_sidecars.py`, `controls_sidecars.py`) write `menu.default.ini` instead of separate files.
* A converter: separate entry files to one `menu.ini`, for people who edited 0.1.0 files.
* Host tests: the reader (sections, repeated titles, `{dir}`/`{file}`, precedence, old `[entry]` files, broken
  lines), discovery with no metadata at all (`g GCE`, `.vec`, movies/waves defaults), no doubled entries.
* Preview scenarios with no `menu/` folder at all, with only `menu.ini`, and with `menu.ini` plus extra files.

## Docs

* `docs/README.md`: "no metadata needed": what shows up with no menu file; where videos and songs go.
* `NEW_MENU_README.md`: rewritten around `menu.ini` with examples for each kind of section; the old separate-file
  format kept as "also works".

## Order of work

1. Discovery without metadata: `g GCE` check, `.vec`, movies/waves defaults (useful on its own; release-worthy).
2. The `menu.ini` reader with old-format compatibility; the generators write `menu.default.ini`.
3. Installer upgrade logic; converter.
4. Docs, tests, preview, hardware test, release 0.2.0.

## Open questions

* `menu.default.ini`: reference copy only, or read when there is no `menu.ini`?
* Do the dedicated arcade emulators all behave when started from PROGRAMS without a parameter?
* Start-up time with the `g GCE` reads, measured on the card.
* Should Vectorblade's plain cartridge (`VECTORBLADE.NIB`) be offered under ALL ROMS too (rename or an entry), once
  it is tried in Exact?

## Done for 0.2.0 already

* Vectorblade is listed under PITREX (issue #1): it only runs as its own PiTrex program. Changed in
  `more_sidecars.py`; the entry file now says `category = PITREX`.
