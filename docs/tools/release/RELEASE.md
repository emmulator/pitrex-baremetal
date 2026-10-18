# PiTrex new menu @VERSION@

A replacement menu for Malban's PiTrex baremetal SD card. The menu is built from the card itself:
drop a game, a ROM or a small `menu/<name>.ini` text file on the card and it appears, sorted, with
its own help text and a CONTROLS page. Nothing is compiled in.

This is **not** a full SD card image. It is a set of files that go over Malban's July 2025 card.
It contains no ROMs, no BIOS and nothing of his that we did not rebuild.

## Install

1. Download Malban's card from <http://vide.malban.de/pitrex/pitrex-baremetal-download>
   (`PiTrex_sd_Jul29_25.zip`) and unzip it. Put your own ROMs in its `vectrex/` folder as usual.
2. Unzip this release.
3. Run the installer with the folder from step 1 (or the SD card, if you already copied it there):

   Linux / macOS:

       ./install.sh /path/to/PiTrex_sd_Jul29_25

   Windows (command prompt in the release folder):

       install.bat C:\path\to\PiTrex_sd_Jul29_25

   Add `--dry-run` to see what would change without writing anything.
4. Copy the folder's contents to the SD card (FAT32, label does not matter) if you have not already.

Every file the installer replaces is first copied to `backup-menu-<date>/` in the same folder.
To undo: `./install.sh --restore <folder>` (Windows: `install.bat --restore <folder>`).
Running the installer twice is harmless.

### Optional: arcade tweaks

`--arcade-tweaks` also:

- adds `blackwidow.ini` (one controller: Button 4 fires the way the stick last pointed),
- replaces `battlezone.ini` with single-stick settings,
- renames the author's `starwars.ini` and `lunar.ini` to `*.author-calibration` (they carry his own
  screen calibration and can put the picture off-screen on another Vectrex).

Undone by `--restore` as well.

## What it changes

| Files | What |
|---|---|
| `piZero1/loader.pit`, `piZero2/loader.pit` | the new menu |
| `piZero*/vectrexspeedy.img`, `vectrexexact.img` | the two Vectrex emulators, rebuilt (the July 2025 images show a blank screen on a Pi Zero W) |
| `piZero*/vectrexvideo.img`, `vectrexaudio.img`, `vectrexlineart.img`, `vectorblade.img` | the video, audio, line-art and VectorBlade players, rebuilt |
| `piZero*/swsbt.img`, `tempest.img`, `blackwidow.img` | rebuilt arcade programs |
| `menu/*.ini` | entry files: titles, categories, help text, controls |
| `loader.ini` | gets `NEW_MENU = 1` |
| `NEW_MENU_README.md` | the user guide (read this next) |

Not touched: `pitrex.img`, ROMs, BIOS, your `vectrex.ini` and `vectrexInterface.ini`.
`SHA256SUMS` lists every file in this release.

## Tested

| | |
|---|---|
| Pi Zero W (V1.1) in a PiTrex V.1 R.3, Vectrex | menu, Vectrex ROMs (speedy and exact), settings, video and audio players, Asteroids, Lunar Lander, Black Widow and most other Atari games |
| Pi Zero 2 | builds only, **not tested on hardware** |
| Star Wars, Tempest (rebuilt) | result not confirmed yet |
| `install.sh` | tested against a fresh unzip of the 2025 card (install, repeat, restore) |
| `install.ps1`, `install.bat` | **not yet run on Windows**. Try `--dry-run` first |

Known problems are in `NEW_MENU_README.md` (audio is quiet: turn the Vectrex volume all the way up;
menu music slows a little on the busiest screens).

## Credits and source

Everything here is built from Malban's open source <https://github.com/malbanGit/pitrex-baremetal>
(thanks also to Kevin Koster for the PiTrex hardware and low-level code). Our changes are on the
`menu-redesign` branch of the fork at `emmulator/pitrex-baremetal` (menu code in `pitrex/loader/`).
