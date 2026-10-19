# PiTrex new menu @VERSION@

A replacement menu for Malban's July 2025 PiTrex SD card, plus the rebuilt emulators and arcade programs it
needs.

* **Start with `README.md`** (plug and play): what to download, how to install, which ROM files go where.
* `NEW_MENU_README.md` is for people who want to change how the menu looks, add their own games or make collections
  (FAVORITES and the like) in the menu file.

Quick start: unzip Malban's `PiTrex_sd_Jul29_25.zip`, then

    ./install.sh /path/to/PiTrex_sd_Jul29_25            (Linux, macOS)
    install.bat C:\path\to\PiTrex_sd_Jul29_25            (Windows; or the SD card, e.g. install.bat E:\)

or build a whole card, arcade ROMs included, from Malban's zip and the RpiTeaM Edition image:

    python3 make_card.py PiTrex_sd_Jul29_25.zip PitreX_Baremetal_RpiTeaM_Edition.img my_card

`--dry-run` shows what would change. `--restore` undoes it. No ROMs or BIOS files are included.

## New in 0.2.0

* **The whole menu is one file**, `menu/menu.default.ini`, and **your changes go in `menu/menu.ini`**, read last:
  only the lines you want different (rename, move, hide, notes), and `also = FAVORITES` to list a game in
  collections of your own. Updates replace `menu.default.ini` and never touch `menu.ini`, so new games still appear.
* **Works without any menu file:** every Vectrex game, video, song and program on the card is listed. Videos in
  `vectrex/movies/` and songs in `vectrex/waves/` appear under VIDEO and AUDIO by themselves.
* Vectrex games may be `.vec` files, and only real cartridges (files starting with `g GCE`) are listed.
* Folder rules (every file in a folder becomes an entry), categories with their own help text, hiding and a
  custom order, games and programs placed in categories of your own.
* A FAVORITES collection (Vectorblade, Asteroids, the Vectrex Armor Attack) in `menu/favorites.ini`, written as an
  example to copy: edit it to keep your own favorites; updates never replace it.
* Vectorblade is listed under PITREX.
* Upgrading from 0.1.0: the installer removes the 0.1.0 entry files you did not change (saved in the backup) and
  keeps the ones you did; they are still read.
* From the review of 0.1.0: the AAE picker no longer says ROM NOT FOUND!, "saved state" is only promised when
  `vectrex.ini` enables it, `NMLOG.TXT` only with `NEW_MENU_LOG = 1`, `install.bat E:\` and `make_card.py` with a
  trailing slash work.

## What is in the package

| Folder | |
|---|---|
| `README.md`, `NEW_MENU_README.md`, `CHANGELOG.md` | the plug-and-play guide, the guide to changing the menu, and what changed in each version |
| `install.sh`, `install.ps1`, `install.bat`, `make_card.py` | the installers |
| `payload/` | bootstrap (`pitrex.img`, `pitrex7.img`), loader, AAE with the game picker, Vectrex emulators and players, Vectorblade, rebuilt Star Wars / Tempest / Black Widow (both Pi models), and the menu file `menu/menu.default.ini` |
| `payload-if-missing/` | `menu/favorites.ini`, an example FAVORITES collection: installed only if the card has no `favorites.ini` yet, so your own list is never overwritten |
| `settings/` | the tuned `vectrexInterface.ini` (used with `--settings tuned`, or if yours is missing or empty) and `vectrex.ini` (only with `--settings tuned`) |
| `arcade-tweaks/` | single-controller `blackwidow.ini` and `battlezone.ini` (only with `--arcade-tweaks`) |

`SHA256SUMS` lists every file. `BUILDINFO.txt` says which source each program was built from.
`legacy-menu-files.sha256` lets the installer recognise unchanged 0.1.0 entry files.

## Tested

| | |
|---|---|
| Pi Zero W in a PiTrex V.1 R.3 | the menu, Vectrex ROMs (Speedy and Exact), settings saving, video and audio players, Star Wars, Tempest, Asteroids, Black Widow, Red Baron, Gravitar, Space Duel, Lunar Lander |
| A card built by `make_card.py` from the 2025 zip and the RpiTeaM image | boots and shows the menu (Pi Zero W, 2026-10-18) |
| Hiding games whose ROM is missing, the entries for the other arcade versions, QB3 and the sample programs | tested on a PC (unit tests and the desktop preview); the menu starts on the PiTrex, the rest is **not yet tried there** |
| A 0.2.0 card built by `make_card.py` (menu file, FAVORITES, VIDEO and AUDIO from folder rules) | boots and the menu shows as expected (Pi Zero W, 2026-10-19) |
| The original menu (`NEW_MENU = 0` in `loader.ini`) with the 0.2.0 loader | boots and shows the original menu (Pi Zero W, 2026-10-19) |
| Changing the menu with `menu.ini`, collections, the no-metadata mode, `.vec` files | tested on a PC (unit tests and the desktop preview); **not yet tried on the PiTrex** |
| Upgrading a 0.1.0 card with `install.sh` | tested on a PC copy of a 0.1.0 card: unchanged entry files removed, changed ones and `menu.ini` kept, `--restore` gives the 0.1.0 card back byte for byte |
| Pi Zero 2 | built, **not tested on hardware** |
| `install.sh` | tested against a fresh unzip of the 2025 card: install, repeat, `--settings tuned`, empty settings file, restore |
| `make_card.py` | tested on Linux against the real 2025 zip and RpiTeaM image |
| `install.ps1`, `install.bat` | **not yet run on Windows**. Try `--dry-run` first |

## Credits and source

Everything here is built from Malban's <https://github.com/malbanGit/pitrex-baremetal> (thanks also to Kevin
Koster for the PiTrex hardware and low-level code). Our changes are on the `menu-redesign` branch of
`emmulator/pitrex-baremetal` (menu code in `pitrex/loader/`).
