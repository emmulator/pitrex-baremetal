# The new menu

Status: implemented on branch `menu-redesign`, **off by default**. Built for both Pi
targets; the logic is unit tested on a PC; **not run on hardware**.

## What it is

A plain vertical list instead of the icon carousel. The first screen lists every category at
once; Button 4 (or stick right) goes into a category, stick left comes back, and the
highlighted entry's notes appear below the list. Example path: `MAIN MENU` ->
`ARCADE` -> `ATARI` -> `MAJOR HAVOC`.

It is **off by default.** To use it, add this line to `loader.ini` on the card:

    NEW_MENU = 1

Remove the line (or set it to 0) to get the original menu back. The loader reads the key at
start-up and again when it comes back from a program.

## Where the entries come from

* **The compiled-in menu.** The new menu walks the same tree of entries as the old one, so
  every existing program starts with exactly the same image, parameters and ROM check
  (`loadAndStart` is unchanged). The five icon categories are named VECTREX, ARCADE,
  COMPUTER, VIDEO and AUDIO. Leading spaces in names are trimmed.
* **Programs found on the card.** Every `.img` in `piZero1/` (or `piZero2/` on a Zero 2) that
  the compiled menu does not start and that is not `pitrex.img`/`pitrex7.img` becomes an
  entry under a new **PROGRAMS** category, sorted by name. Drop a new program in the folder
  and it appears. On the current card that finds 8: asteroids, deluxe, gyrocks, hello, maze,
  pacman, perspective, window.
* **Vectrex ROMs.** Every `.bin` under `vectrex/` (two folder levels deep; `bios`, `movies`,
  `waves` and names starting with `.` are skipped) is listed under **VECTREX > ALL ROMS**:
  one group per folder (`ORIGINAL`, `DEMOS`, ...), then the files that sit directly in
  `vectrex/`, each sorted by name (`MR_BOSTON.BIN` shows as `MR BOSTON`). They start the
  **Exact** emulator with the ROM's path as the parameter, the same call the compiled-in entries
  make. **Button 2** starts the highlighted ROM with the **Speedy** emulator instead; the screen
  says what Button 2 will do. A sidecar can make Speedy the default (see below).
* **SETTINGS**, last in the list, opens the calibration screen as before.
* **ALL AAE GAMES** (ARCADE) starts the AAE picker (file 06).

## Controls

| Input | Action |
|---|---|
| stick up / down | move (hold to repeat) |
| stick right, or Button 4 | go into a category; on an entry, start it |
| stick left | back |
| Button 1 | start the entry and load its saved state (Vectrex games), as in the old menu |
| Button 2 | on a ROM under ALL ROMS: start it with the other emulator (Speedy if Exact is the default) |

PICK A GAME is handed to the old file browser: it opens the same screen as before and
returns to the list when you leave it with the stick.

## Sidecar files (file 05) in the new menu

* `hide = 1` in an entry's sidecar leaves it out of the list, compiled entries included.
* For a program found on the card, `menu/<image name>.ini` can set `title = ...` (default:
  the file name in capitals with `_` and `-` as spaces), `hide = 1`, and note lines
  (`text = ...`), exactly as for other entries. Example for `maze.img`: `menu/maze.ini`.
* For a ROM under ALL ROMS the file is `menu/rom.<path>.ini`, where the path is lower-cased
  and each run of other characters becomes one `_`: `original/MR_BOSTON.BIN` ->
  `menu/rom.original_mr_boston_bin.ini`. It can set `emulator = speedy` or `emulator = exact`
  (the default), `title`, `hide` and notes.
* Notes and "ROM NOT FOUND!" show under the list for the highlighted entry.
* **Caveat tags.** The list shows the most serious caveat from an entry's notes beside its
  name: BROKEN ("not working", "crash"), WIP ("work in progress") or NO SOUND. The list shows a
  `!` at the right edge of such an entry; the notes under the list carry the wording.
  The tags come from the compiled-in notes, or, when a sidecar supplies notes, from the
  sidecar's lines. 27 of the 69 compiled-in entries have one today. Fix a game, edit its
  sidecar, and the tag goes away.
* The loader reads the list of files in `menu/` once and only opens files that exist.

## Not done yet

* Which games run well under Speedy is not known: the default is Exact for everything, and the
  8 Speedy presets stay in their own group. Both emulators accept any `.bin` (file 02, F8).
* Icons.
* `.vec` files and folders deeper than two levels are not listed; a ROM whose path is 64
  characters or longer is skipped. At most 400 ROMs.
* Categories for programs (all go under PROGRAMS).
* Remembering your place after a program returns is not implemented. It may happen anyway,
  because the loader stays in memory while a program runs and its variables are kept; that is
  unverified.
* Whether the old menu's draw routine and this one disagree on anything the author relies
  on, such as the flashing title in the corner, is unverified.

## Test

`docs/tools/test_new_menu.c` builds the menu logic against the real compiled-in tree
(`docs/tools/gen_menu_tree.py` writes it as test data from `loaderMain.c`): 79 nodes, 75
launchable entries, titles, drilling down and back with selection restore, the scrolling
window, Major Havoc keeps its compiled entry and game byte, hiding, and the discovery of the 8
programs from the card's real file list, and the caveat tags of the compiled-in entries (27 of 69, the
same count an independent python scan of the source gives). For ROMs it uses a list of 158 real and made-up paths
(the card's `vectrex/` tree, the 44 homebrew files in Downloads, and junk such as `bios/x.bin`,
`.hidden.bin`, `readme.txt`): 135 are listed, grouped and ordered as an independent python
computation says, the reserved folders are left out, and the emulator switch works. It fails when the
hide logic or the reserved-folder rule is broken (both checked; the second one caught a real bug
while writing it). Not tested: drawing, the SD card calls, stick feel, and the hand-off to PICK A GAME.

The test files `rom_paths.txt` and `rom_expected.txt` are produced by the snippet in
`docs/tools/make_rom_test_data.py`.

## Screen layout and help (2026-10-17, after the first hardware test)

The first test showed the text was too small and that the PiTrex vector font draws almost no
punctuation. The list now uses size 7 text, 7 rows, and only `>` (selected), `$` (category) and
`!` (caveat). The breadcrumb joins levels with ` > `. Under the list: the notes of the highlighted
game, or for a category a few lines of plain-language help (`nmHelp`, in `newMenuLogic.h`, tested
for length and for using only drawable characters). The bottom lines say what the buttons do for
the highlighted entry (for a Vectrex game file: which emulator Button 4 and Button 2 use). Starting
anything first shows "STARTING <name> WITH <emulator>" for 45 frames, so a blank screen afterwards
can be told apart from a menu failure.

SETTINGS now points the ini writer at `vectrexInterface.ini` explicitly (the loader's re-init
path parses `loader.ini`, and `v_saveIni()` writes to whichever ini was parsed last), and reports
SETTINGS SAVED or SETTINGS NOT SAVED! after checking the file is not empty. Cause of the empty
`vectrexInterface.ini` seen on the first test: unknown. `v_saveIni()` does nothing when no ini was
parsed at start (a card without `vectrexInterface.ini`), which this also covers.
See `NEW_MENU_README.md` in the repository root for the user-facing guide.
