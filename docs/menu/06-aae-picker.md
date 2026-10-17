# AAE game selection and picker

Status: implemented on branch `menu-redesign` (aae/aaeSelect.h, aae/aaemain.c, loader entry
"ALL AAE GAMES"). **Built for both Pi targets and unit tested on a PC; not run on hardware.**

## What changed

`aae.img` used to take only a game **number**, an unchecked index into its compiled driver
table. It now accepts three forms of the loader's first parameter:

| Parameter | Meaning |
|---|---|
| exactly one byte | the game number, as before (every existing menu entry still works); now range-checked |
| empty | **show the picker** |
| anything longer | the game's MAME name, such as `mhavoc2` (case does not matter) |

An unknown name or a number past the table shows "UNKNOWN GAME" for two seconds instead of
reading outside the table.

## The picker

A new menu entry, **ALL AAE GAMES** (ARCADE category, after OTHER ARCADE), starts `aae.img`
with no parameter. AAE then lists `roms/aae/*.zip` (case-insensitive `.zip`), keeps the zips
whose names match a game in its driver table, and shows them sorted by title, eight rows at
a time. Stick up/down moves (holding repeats), **Button 4 plays**, **Button 1 goes back**.
A game it has a driver for appears as soon as its zip is on the card. No rebuild.

Limits: 160 files read from the folder, 128 games listed, names up to 31 characters.

## Limits and open questions

* A zip is matched by name only. Whether it holds the right files is not checked, and AAE
  does not check checksums (file 04). A bad set shows AAE's own "SOME ROMS NOT FOUND" or runs
  wrongly.
* The author's notes ("NO SOUND", "FLICKER") are not shown in the picker. The sidecar notes
  (file 05) are keyed per menu entry and the picker's games have no entry.
* Going back (Button 1) or an unknown game ends `main` with `return 0`, the same path the
  program takes after a game. What the loader does after that has not been verified.
* The picker runs before AAE sets up its interrupt handler, using the same polled screen
  calls as the loader's own menu. Not verified on the Pi.
* A game AAE has no driver for (Aztarac, Empire Strikes Back) still needs emulator work.

## Test

`docs/tools/test_aae_select.c` builds `aaeSelect.h` against the real driver names (from
`docs/tools/gen_aae_table.py`) and checks: all parameter forms, every name finds itself, zip
name rules, and a real directory listing (the card's `roms/aae` plus the newly added sets):
70 of the 71 drivers have a zip, no duplicates, correct order. It fails when the sort is
broken (checked). Usage is at the top of the file. Not tested: the on-screen code.
