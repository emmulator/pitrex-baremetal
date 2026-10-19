# Changelog

Changes to the new PiTrex menu (this fork of [malbanGit/pitrex-baremetal](https://github.com/malbanGit/pitrex-baremetal)).
Releases: <https://github.com/emmulator/pitrex-baremetal/releases>. Install guide: [README.md](README.md); changing
the menu: [NEW_MENU_README.md](../NEW_MENU_README.md).

## 0.2.0 (not yet released)

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
