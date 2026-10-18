# PiTrex new menu @VERSION@

A replacement menu for Malban's July 2025 PiTrex SD card, plus the rebuilt emulators and arcade programs it
needs.

* **Start with `README.md`** (plug and play): what to download, how to install, which ROM files go where.
* `NEW_MENU_README.md` is for people who want to change how the menu looks, add their own games or edit the
  entry files.

Quick start: unzip Malban's `PiTrex_sd_Jul29_25.zip`, then

    ./install.sh /path/to/PiTrex_sd_Jul29_25            (Linux, macOS)
    install.bat C:\path\to\PiTrex_sd_Jul29_25            (Windows)

or build a whole card, arcade ROMs included, from Malban's zip and the RpiTeaM Edition image:

    python3 make_card.py PiTrex_sd_Jul29_25.zip PitreX_Baremetal_RpiTeaM_Edition.img my_card

`--dry-run` shows what would change. `--restore` undoes it. No ROMs or BIOS files are included.

## What is in the package

| Folder | |
|---|---|
| `README.md`, `NEW_MENU_README.md` | the plug-and-play guide, and the guide to changing the menu |
| `install.sh`, `install.ps1`, `install.bat`, `make_card.py` | the installers |
| `payload/` | bootstrap (`pitrex.img`, `pitrex7.img`), loader, AAE with the game picker, Vectrex emulators and players, rebuilt Star Wars / Tempest / Black Widow, 114 entry files in `menu/` (both Pi models) |
| `settings/` | the tuned `vectrexInterface.ini` (used only with `--settings tuned`, or if yours is missing or empty) |
| `arcade-tweaks/` | single-controller `blackwidow.ini` and `battlezone.ini` (only with `--arcade-tweaks`) |

`SHA256SUMS` lists every file.

## Tested

| | |
|---|---|
| Pi Zero W in a PiTrex V.1 R.3 | the menu, Vectrex ROMs (Speedy and Exact), settings saving, video and audio players, Star Wars, Tempest, Asteroids, Black Widow, Red Baron, Gravitar, Space Duel, Lunar Lander |
| Hiding games whose ROM is missing, and the entry files for the other arcade versions, QB3 and the sample programs | tested on a PC (unit tests and the desktop preview), **not yet on the PiTrex** |
| Pi Zero 2 | built, **not tested on hardware** |
| `install.sh` | tested against a fresh unzip of the 2025 card: install, repeat, `--settings tuned`, empty settings file, restore |
| `make_card.py` | tested on Linux against the real 2025 zip and RpiTeaM image |
| `install.ps1`, `install.bat` | **not yet run on Windows**. Try `--dry-run` first |

## Credits and source

Everything here is built from Malban's <https://github.com/malbanGit/pitrex-baremetal> (thanks also to Kevin
Koster for the PiTrex hardware and low-level code). Our changes are on the `menu-redesign` branch of
`emmulator/pitrex-baremetal` (menu code in `pitrex/loader/`).
