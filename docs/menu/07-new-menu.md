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
* **SETTINGS**, last in the list, opens the calibration screen as before.
* **ALL AAE GAMES** (ARCADE) starts the AAE picker (file 06).

## Controls

| Input | Action |
|---|---|
| stick up / down | move (hold to repeat) |
| stick right, or Button 4 | go into a category; on an entry, start it |
| stick left | back |
| Button 1 | start the entry and load its saved state (Vectrex games), as in the old menu |

PICK A GAME is handed to the old file browser: it opens the same screen as before and
returns to the list when you leave it with the stick.

## Sidecar files (file 05) in the new menu

* `hide = 1` in an entry's sidecar leaves it out of the list, compiled entries included.
* For a program found on the card, `menu/<image name>.ini` can set `title = ...` (default:
  the file name in capitals with `_` and `-` as spaces), `hide = 1`, and note lines
  (`text = ...`), exactly as for other entries. Example for `maze.img`: `menu/maze.ini`.
* Notes and "ROM NOT FOUND!" show under the list for the highlighted entry.

## Not done yet

* Vectrex `.bin` files are not listed in the new menu; use PICK A GAME or the original menu.
* Speedy-vs-Exact choice for new ROMs, caveat tags in the list (`[NO SOUND]`), icons.
* Categories for programs (all go under PROGRAMS).
* The list does not remember where you were after a program returns.
* Whether the old menu's draw routine and this one disagree on anything the author relies
  on, such as the flashing title in the corner, is unverified.

## Test

`docs/tools/test_new_menu.c` builds the menu logic against the real compiled-in tree
(`docs/tools/gen_menu_tree.py` writes it as test data from `loaderMain.c`): 79 nodes, 75
launchable entries, titles, drilling down and back with selection restore, the scrolling
window, Major Havoc keeps its compiled entry and game byte, hiding, and the discovery of the 8
programs from the card's real file list. It fails when the hide logic is broken (checked).
Not tested: drawing, the SD card calls, stick feel, and the hand-off to PICK A GAME.
