# PiTrex new menu @VERSION@

A replacement menu for Malban's July 2025 PiTrex SD card, plus the rebuilt emulators and arcade programs it
needs.

* **Start with `README.md`** (plug and play): what to download, how to install, which ROM files go where.
* `NEW_MENU_README.md` is for people who want to change how the menu is arranged, add their own games or make
  collections (FAVORITES and the like) in the menu file. The installer also puts it on the card, in `menu/`.

Quick start: unzip Malban's `PiTrex_sd_Jul29_25.zip`, then

    ./install.sh /path/to/PiTrex_sd_Jul29_25            (Linux, macOS)
    install.bat C:\path\to\PiTrex_sd_Jul29_25            (Windows; or the SD card, e.g. install.bat E:\)

or build a whole card, arcade ROMs included, from Malban's zip and the RpiTeaM Edition image:

    python3 make_card.py PiTrex_sd_Jul29_25.zip PitreX_Baremetal_RpiTeaM_Edition.img my_card

`--dry-run` shows what would change. `--restore` undoes it. No ROMs or BIOS files are included.

## New in 0.2.3

* **Four versions of the screen settings.** If the picture on your Vectrex is garbled, badly scaled or too faint to read
  with the settings file that came with Malban's 2025 card, you can now try three others: Malban's March 2021 file, the
  RpiTeaM Edition's (December 2021) file, and the author's own. They are in the `settings/` folder of the package, as
  unchanged copies with names that say where they come from, and `settings/README.txt` describes each. `install.sh
  --settings malban-2021` (also `rpiteam-2021`, `malban-2025`, `alt`) puts one on the card; run it again with another
  value to switch. Nothing else on the card changes (`alt` also replaces `vectrex.ini`), and `--restore` goes back.
  `README.md` explains which one to try and why.
* The folders `settings-alt/` and `settings-default/` of 0.2.2 are now the one folder `settings/`.
* **`README.md` explains the menu font** (`NEW_MENU_FONT = DEFAULT` or `MINIMALIST` in `loader.ini`): why one might choose
  the minimalist font (fewer lines per letter, so a screen full of text may flicker less), with photos of both to look
  at first.
* `README.md` no longer says that Speedy and Exact each run any game: the games under EXACT VECTREX need Exact.
* The programs and the menu are the same as in 0.2.2; only the installers, the settings files and the guides changed.

## New in 0.2.2

* **A second font for the menu text.** Add `NEW_MENU_FONT = MINIMALIST` to `loader.ini` to draw the text with a font
  that takes fewer lines per letter, so a screen with a lot of text may flicker less. `DEFAULT` (or no line) keeps the
  PiTrex font. The installer adds `NEW_MENU_FONT = DEFAULT` to `loader.ini`, with comments that explain the two
  values, if the file has no such line.
* **Shorter help texts and control lines**, and the groups under VECTREX > ALL ROMS that stand for a folder of the card
  are named after the folder, without the word FOLDER (the folder sign says it).
* **Screen settings:** Malban's `vectrexInterface.ini` stays the starting point. `--settings tuned` is now
  `--settings alt` (`tuned` still works): it installs the settings of the author's Vectrex, which are no better than
  Malban's in general. A card with no `vectrexInterface.ini`, or an empty one, now gets Malban's file.
* **Upgrading from 0.1.0** removes the 0.1.0 entry files you did not change, as 0.2.0 and 0.2.1 said it would (their
  list of those files was empty).

## New in 0.2.1

* **A program you copy to the card is listed under PITREX** (an `.img` in `piZero1/` or `piZero2/`), named after its
  file, with no menu entry needed. The separate PROGRAMS category is gone. Give it a name, notes or another place
  with a few lines in `menu/menu.ini`; the guide has an example, including a game that has a different file for
  each Pi model.
* **The guide is on the card, in `menu/`** (`menu/NEW_MENU_README.md`), next to `menu.ini`. The installer removes the
  old copy from the card's root if you have not changed it.
* **Vectorburner** is no longer in the default menu (it is not part of Malban's card). It is listed under PITREX by
  itself when its image is on the card.
* The default menu no longer carries "not tested" notes. The notes of Red Baron, Gravitar and Space Duel no longer
  say NO SOUND: they have sound on a real PiTrex.

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
| `payload/` | bootstrap (`pitrex.img`, `pitrex7.img`), loader, AAE with the game picker, Vectrex emulators and players, Vectorblade, rebuilt Star Wars / Tempest / Black Widow (both Pi models), the menu file `menu/menu.default.ini` and the guide `menu/NEW_MENU_README.md` |
| `payload-if-missing/` | `menu/favorites.ini`, an example FAVORITES collection: installed only if the card has no `favorites.ini` yet, so your own list is never overwritten |
| `settings/` | four versions of `vectrexInterface.ini` (Malban 2021-03, RpiTeaM 2021-12, Malban 2025-07, and the author's `alt` one, with its `vectrex-alt.ini`) and a `README.txt` that says which is which. Installed only with `--settings <version>`; Malban's 2025 file also when the card has none or an empty one |
| `arcade-tweaks/` | single-controller `blackwidow.ini` and `battlezone.ini` (only with `--arcade-tweaks`) |

`SHA256SUMS` lists every file. `BUILDINFO.txt` says which source each program was built from.
`legacy-menu-files.sha256` and `legacy-files.sha256` let the installer recognise unchanged files of earlier versions.

## Credits and source

Everything here is built from Malban's <https://github.com/malbanGit/pitrex-baremetal> (thanks also to Kevin
Koster for the PiTrex hardware and low-level code). Our changes are on the `menu-redesign` branch of
`emmulator/pitrex-baremetal` (menu code in `pitrex/loader/`).
