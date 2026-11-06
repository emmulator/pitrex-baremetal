# Changelog

Changes to the new PiTrex menu (this fork of [malbanGit/pitrex-baremetal](https://github.com/malbanGit/pitrex-baremetal)).
Releases: <https://github.com/emmulator/pitrex-baremetal/releases>. Install guide: [README.md](README.md); changing
the menu: [NEW_MENU_README.md](../NEW_MENU_README.md).

## 0.2.3 (not released yet)

### Installers
* Four versions of the screen settings file are in the package's `settings/` folder (replacing `settings-alt/` and
  `settings-default/`): Malban's March 2021 file, the RpiTeaM Edition's December 2021 file, Malban's July 2025 file
  and the author's own (`alt`). `--settings malban-2021 | rpiteam-2021 | malban-2025 | alt` chooses one (`tuned` still
  means `alt`). Running the installer again with another value changes only `vectrexInterface.ini` (`alt` also
  `vectrex.ini`). The 2021 files are the first thing to try when the 2025 file gives a garbled picture: a user
  reported that. `settings/README.txt` and the Settings section of `README.md` explain the versions.

### Docs
* `README.md` has a section on the menu font (`NEW_MENU_FONT`): why one might choose the minimalist font, and where
  to look at photos of both before choosing. It no longer says that Speedy and Exact each run any game: the games
  under EXACT VECTREX need Exact.

### Repository
* The repository README starts with photos of the menu in the default and the minimalist font.

## 0.2.2 (2026-11-04)

### Menu
* The groups under VECTREX > ALL ROMS that stand for a folder of the card are no longer named "... FOLDER": the folder
  sign in front of them says so.
* `NEW_MENU_FONT = MINIMALIST` in `loader.ini` draws the menu text with the minimalist font (the font of uvmc2-menu): fewer strokes
  and jumps per letter, so a screen with a lot of text takes less beam time and may flicker less. The PiTrex font
  stays the default. The installer adds `NEW_MENU_FONT = DEFAULT` to `loader.ini`, with comment lines that explain the
  two values, if the file has no such line yet. The glyph table is `pitrex/loader/newMenuFont.h`, checked by `new_menu/tools/test_minimalist_font.c`.

### Installers
* The option `--settings tuned` is now `--settings alt` (`-Settings alt` on Windows; `tuned` still works). The word
  "tuned" made the author's calibration sound like an improvement over Malban's, and it is not: it is what one Vectrex
  ended up with. The folder `settings/` of the package is `settings-alt/`.
* If the card has no `vectrexInterface.ini`, or an empty one, the installer now puts Malban's file in
  (`settings-default/`), no longer the alternative one.
* Fix: the list of 0.1.0 entry files that the installer may remove (`legacy-menu-files.sha256`) was empty in 0.2.0 and
  0.2.1 (it was made from a path that moved when `docs/` became `new_menu/`), so upgrading a 0.1.0 card left the old
  `menu/*.ini` entry files in place. They did no harm (the menu file still wins), but they were not removed as
  described. The list is now a file in the repository.

### Repository
* The 0.1.0 entry files (`new_menu/menu/sidecar/menu/`) and the scripts that made them are gone; `menu.default.ini`
  is edited directly. They are at the `v0.2.1` tag.

## 0.2.1 (2026-10-20)

### Menu
* A PiTrex program the menu files do not mention (an `.img` you copied into `piZero1/` or `piZero2/`) is listed
  under **PITREX**, with the other PiTrex games. The separate PROGRAMS category is gone.
* The PITREX help text says that a program you copy to the card is listed there and where the guide is.
* Vectorburner is no longer in the default menu file: it is not part of Malban's card, so it is listed under PITREX
  by itself when its image is on the card, and `menu.ini` can give it a name and notes.
* The notes of Red Baron, Gravitar and Space Duel no longer say NO SOUND: they have sound on a real PiTrex (Black Widow's
  note was already corrected in 0.2.0). The other notes are Malban's own.
* The default menu file no longer says NOT TESTED about games: whether a game has been tried is not part of the
  release. (`test-log.ini`, kept by whoever tests, can put those notes back for their own card.)

### Guide
* `NEW_MENU_README.md` is now `menu/NEW_MENU_README.md` on the card, next to `menu.ini`, instead of in the card's
  root. The installer removes the old copy from the root when it is unchanged.
* It tells how to add a game with a different file name for each Pi model (one entry per file; the one whose file is
  not on that Pi is not shown).

## 0.2.0 (2026-10-19)

### The menu is one file, with your changes on top
* All entries, notes, controls and categories are in one file, `menu/menu.default.ini`, instead of one file per
  entry. Updates replace it.
* Your changes go in `menu/menu.ini`, read last: only the lines you want different (rename, move, hide, notes).
  Updates never touch it, so new games still appear on a customised card.
* Other `.ini` files in `menu/` are read in between, so a game can come with its own small file.
* Collections: `also = FAVORITES, TWO PLAYERS` lists a game in more categories while it stays in its usual place;
  `order = listed` keeps a category in your order; a category can have its own help text and can be hidden;
  `hide = 0` shows what an earlier file hid.
* `menu/favorites.ini`: an example FAVORITES collection (Vectorblade, Asteroids, the Vectrex Armor Attack) to edit
  into your own list. It is installed only when the card has none, so yours is never overwritten.

### Works without any menu file
* Every Vectrex game, video, song and program on the card is listed even with no menu file at all.
* Videos in `vectrex/movies/` and songs in `vectrex/waves/` appear under VIDEO and AUDIO by themselves. Folder
  rules (`folder = ...` with `{dir}` and `{file}`) do the same for any folder of files one program plays.
* Vectrex games may be `.vec` files, and only real cartridges (files starting with `g GCE`) are listed.

### Menu
* Vectorblade is listed under PITREX (it only runs as its own PiTrex program).
* The curated Vectrex lists (SPEEDY, EXACT, DEMOS) hide an entry whose game file is gone, instead of starting the
  emulator with nothing to load.
* A game renamed by the menu file is sorted under its new name in ALL ROMS.
* The ALL AAE GAMES picker no longer says ROM NOT FOUND!.
* The STARTING screen only says AND SAVED STATE when `vectrex.ini` enables saved states (`HANDLE_STATES = 1`).
* The button hint on a game says "4 STARTS" (stick right only opens categories; it never started games).
* `NMLOG.TXT` is only written with `NEW_MENU_LOG = 1` in `loader.ini`.

### Installers
* Upgrading from 0.1.0 removes the old entry files that are unchanged (saved in the backup) and keeps any you
  changed; they are still read.
* `--restore` keeps a file the install added if you have changed it since.
* `install.bat E:\` (a drive root) and `make_card.py` with a trailing slash in the output folder work.
* The package has `BUILDINFO.txt` (which source each program was built from) and this changelog.

### Repository
* The fork's documentation and tools moved from `docs/` to `new_menu/`. **Known issue:** the 0.1.0 release page and
  the guides inside the 0.1.0 zip link to files under `docs/` on the `menu-redesign` branch; those links now show
  "page not found". The same files are still there at the `v0.1.0` tag:
  <https://github.com/emmulator/pitrex-baremetal/tree/v0.1.0/docs>.

## 0.1.0 (2026-10-18)

First release.

* A new list menu (`NEW_MENU = 1` in `loader.ini`): categories you drill into, built at every start from the
  entry files in `menu/`, the programs in `piZero1/` and the ROMs under `vectrex/`. Nothing is compiled in.
* Notes and a CONTROLS page (Button 2) for every game; caveat marks (`!`) for NO SOUND, WORK IN PROGRESS and
  broken games.
* Entry files for every game the emulators can run, including 39 more AAE versions. Games whose ROM is not on the
  card are hidden (`NEW_MENU_SHOW_MISSING = 1` lists them).
* VECTREX > ALL ROMS lists every Vectrex game on the card; Button 4 starts Exact, Button 2 Speedy.
* The AAE game picker (ARCADE > ALL AAE GAMES) lists the ROM zips found in `roms/aae/`.
* Rebuilt Vectrex emulators, video, audio and line-art players, Vectorblade, Star Wars, Tempest and Black Widow (the
  July 2025 images show a blank screen on a Pi Zero W); Black Widow plays with one controller.
* Fixes: the settings screen no longer freezes when saving; the menu music slows down much less on busy screens.
* Installers for Linux, macOS and Windows with backup and `--restore`; `make_card.py` builds a whole card from
  Malban's zip and the RpiTeaM image; a tuned settings option.
