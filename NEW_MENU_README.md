# The new PiTrex menu

A replacement for the PiTrex bare-metal menu (Malban's July 2025 SD card). It is a plain list: pick a
category, go in, pick a game, start it. **Nothing is compiled in**: the menu is built every time the PiTrex
starts from what is on the SD card, so a game appears when its files are there and disappears when they are
not. Notes, warnings and a controls page for each game are small text files you can edit.

This package does not contain any ROMs. You supply the ROM files you own; the lists in
[Putting the ROMs on the card](#putting-the-roms-on-the-card) say exactly which files the menu looks for and
where they go, so you can see what is still missing for a complete set.

## Installing

1. Download Malban's card from <http://vide.malban.de/pitrex/pitrex-baremetal-download>
   (`PiTrex_sd_Jul29_25.zip`) and unzip it. This package is a set of files that go **over** that card.
2. Unzip this package and run the installer, giving it the folder from step 1:

   | System | Command |
   |---|---|
   | Linux, macOS | `./install.sh /path/to/PiTrex_sd_Jul29_25` |
   | Windows | `install.bat C:\path\to\PiTrex_sd_Jul29_25` |

3. Copy the folder to a FAT32 SD card (or run the installer on the card itself) and add your ROMs.

Options: `--dry-run` shows what would change and writes nothing. `--settings tuned` replaces your screen
settings (see [Settings](#settings)). `--arcade-tweaks` also adds the single-controller files for Black Widow
and Battle Zone and sets the original author's Star Wars and Lunar Lander calibration files aside (they are
for his Vectrex). `--restore` undoes the newest install.

The installer saves every file it replaces in `backup-menu-<date>/` on the card first, never touches ROMs, BIOS
files, `pitrex.img` or your own settings, and can be run twice safely. It replaces these files: the loader
(`piZero1/loader.pit`, `piZero2/loader.pit`), the Vectrex emulators and players, three rebuilt arcade programs
(Star Wars, Tempest, Black Widow), the entry files in `menu/`, and `loader.ini` (it gains `NEW_MENU = 1`).
The July 2025 emulators show a blank screen on a Pi Zero W, which is why they are replaced.

To go back to the original menu: delete the `NEW_MENU = 1` line from `loader.ini`.

## Using the menu

The menu opens at the top when the PiTrex starts (it ignores the controls until they have rested for a moment).

| You press | What happens |
|---|---|
| Stick **up / down** | Move the `>` marker. Hold to repeat. |
| Stick **right**, or **Button 4** | Open a category or start the highlighted game. |
| Stick **left** | Go back up one level. |
| **Button 2** on a Vectrex game | Start it with the *other* emulator (Button 4 uses Exact, Button 2 uses Speedy). |
| **Button 2** on an arcade game | Show how its controls work. Any button or the stick comes back. |
| **Button 1** | Like Button 4, but a Vectrex emulator loads the last saved state. |

`$` in front of a row is a category (folder), `!` means there is a caveat (read the notes under the list).
The PiTrex font only draws letters, digits, `.`, `!` and arrows, so the menu and any text you write for it
use only those.

Games are listed alphabetically. **A game whose ROM is not on the card is not listed**; it appears by itself once
the file is there. (To list every game anyway with a "ROM NOT FOUND!" note, add `NEW_MENU_SHOW_MISSING = 1` to
`loader.ini`.)

Speedy and Exact are one Vectrex emulator built two ways and either runs any game. Exact is as close to a real
Vectrex as the author could make it; Speedy runs games 30-50% faster and skips some BIOS work, so a few games
misbehave. Start with Exact and press Button 2 if a game is too slow or looks wrong.

## Putting the ROMs on the card

ROM files are not provided here. Sets from MAME with the file names below work; the arcade emulators read
the zip files themselves and only check that the files inside exist, so a wrong version starts but misbehaves.

### Vectrex games

| What | Where | Notes |
|---|---|---|
| Vectrex games | `vectrex/*.bin`, or in sub-folders such as `vectrex/original/` (two levels) | Any `.bin`. They show under VECTREX > ALL ROMS, each folder as a group named `<FOLDER> FOLDER`. A game's title is its file name. Rename `.vec` files to `.bin`. |
| Vectrex BIOS | `vectrex/bios/` | Malban's card already has four. `vectrex.ini` (`ROM_FILE`) picks the one Exact uses. |
| Music and video | `vectrex/waves/*.bin`, `vectrex/movies/*.bin` | Come with Malban's card. Turn the Vectrex volume all the way up: the sound goes through the video DAC and is quiet. |
| Players | `vectrex/WAVPLAYER.NIB`, `MOVIEPLAYER.NIB`, `VECTORBLADE.NIB` | Come with Malban's card. |

Games the menu has notes for (Berzerk, Dark Tower, Minestorm 2, Mr Boston, Scramble, Web Wars, the demos, ...)
are listed in their emulator's group as well as in ALL ROMS. Games it has no notes for are still listed, just
without notes.

### Arcade games

Arcade games run on two kinds of program. Some games have their own emulator (faster, and where the
input is set up); the rest run on AAE ("Another Arcade Emulator"). The two kinds read **different zip folders**.

#### Games with their own emulator: ROM zips go in `roms/`

| Zip | Game | Menu entry | Program |
|---|---|---|---|
| `astdelux.zip` | Asteroids Deluxe Sim (older emulator) | Atari | `deluxe.img` |
| `asteroid.zip` | Asteroids Sim (older emulator, also the fallback set for Asteroids) | Atari | `asteroids.img` |
| `asteroid2.zip` | Asteroids | Atari | `asteroids_sbt.img` |
| `barrier.zip` | Barrier | Cinematronics | `cine.img` |
| `bwidow.zip` | Black Widow | Atari | `blackwidow.img` |
| `bzone.zip` | Battle Zone | Atari | `battlezone.img` |
| `demon.zip` | Demon | Cinematronics | `cine.img` |
| `gravitar.zip` | Gravitar | Atari | `gravitar.img` |
| `llander.zip` | Lunar Lander | Atari | `lunar.img` |
| `qb3.zip` | QB3 (Q*bert's Qubes prototype) | Other Arcade | `cine.img` |
| `redbaron.zip` | Red Baron | Atari | `redbaron.img` |
| `spacduel.zip` | Space Duel | Atari | `spaceduel.img` |
| `spacewar.zip` | Space Wars | Cinematronics | `cine.img` |
| `tailg.zip` | Tailgunner | Cinematronics | `tailgunner.img` |
| `tempest.zip` | Tempest | Atari | `tempest.img` |

#### Games that run on AAE: ROM zips go in `roms/aae/`

| Zip | Game | Where in the menu |
|---|---|---|
| `llander1.zip` | Lunar Lander (Revision 1) | Atari > Lunar Lander Versions > Lunar Lander Rev 1 |
| `llander.zip` | Lunar Lander (Revision 2) | picker only (the same game also has its own emulator above) |
| `meteorts.zip` | Meteorites (Asteroids Bootleg) | Atari > Asteroids Versions > Meteorites Bootleg |
| `asterock.zip` | Asterock (Asteroids Bootleg) | Atari > Asteroids Versions > Asterock Bootleg |
| `asteroib.zip` | Asteroids (Bootleg on Lunar Lander Hardware) | Atari > Asteroids Versions > Asteroids Lunar Hw Bootleg |
| `asteroi1.zip` | Asteroids (Revision 1) | Atari > Asteroids Versions > Asteroids Rev 1 |
| `asteroid.zip` | Asteroids (Revision 2) | Atari > Asteroids Versions > Asteroids Rev 2 |
| `astdelu1.zip` | Asteroids Deluxe (Revision 1) | Atari > Asteroids Versions > Asteroids Deluxe Rev 1 |
| `astdelu2.zip` | Asteroids Deluxe (Revision 2) | Atari > Asteroids Versions > Asteroids Deluxe Rev 2 |
| `astdelux.zip` | Asteroids Deluxe (Revision 3) | Atari > Asteroids Deluxe |
| `omegrace.zip` | Omega Race | Other Arcade > Omega Race |
| `deltrace.zip` | Delta Race (Omega Race Bootleg) | Other Arcade > Delta Race |
| `bzone.zip` | Battlezone (Revision 1) | picker only (the same game also has its own emulator above) |
| `bzone2.zip` | Battlezone (Revision 2) | Atari > Battle Zone Versions > Battle Zone Rev 2 |
| `bzonec.zip` | Battlezone Cocktail Proto | Atari > Battle Zone Versions > Battle Zone Cocktail |
| `bzonep.zip` | Battlezone Plus (Clay Cowgill) | Atari > Battle Zone Versions > Battle Zone Plus |
| `redbaron.zip` | Red Baron | picker only (the same game also has its own emulator above) |
| `bradley.zip` | Bradley Trainer | Atari > Bradley Trainer |
| `spacduel.zip` | Space Duel | picker only (the same game also has its own emulator above) |
| `bwidow.zip` | Black Widow | picker only (the same game also has its own emulator above) |
| `gravitar.zip` | Gravitar (Revision 3) | picker only (the same game also has its own emulator above) |
| `gravitr2.zip` | Gravitar (Revision 2) | Atari > Gravitar Versions > Gravitar Rev 2 |
| `gravp.zip` | Gravitar (Prototype) | Atari > Gravitar Versions > Gravitar Prototype |
| `lunarbat.zip` | Lunar Battle (Prototype, Late) | Atari > Lunar Lander Versions > Lunar Battle Late |
| `lunarba1.zip` | Lunar Battle (Prototype, Early) | Atari > Lunar Lander Versions > Lunar Battle Early |
| `tempestm.zip` | Tempest Multigame (1999 Clay Cowgill) | Atari > Tempest Versions > Tempest Multigame |
| `tempest.zip` | Tempest (Revision 3) | picker only (the same game also has its own emulator above) |
| `tempest3.zip` | Tempest (Revision 2B) | Atari > Tempest Versions > Tempest Rev 2B |
| `tempest2.zip` | Tempest (Revision 2A) | Atari > Tempest Versions > Tempest Rev 2A |
| `tempest1.zip` | Tempest (Revision 1) | Atari > Tempest Versions > Tempest Rev 1 |
| `temptube.zip` | Tempest (Revision 1) | Atari > Tempest Versions > Tempest Rev 1 Tubes |
| `alienst.zip` | Aliens (Tempest Alpha) | Atari > Tempest Versions > Aliens Tempest Alpha |
| `vbreak.zip` | Vector Breakout (1999 Clay Cowgill) | Other Arcade > Vector Breakout |
| `vortex.zip` | Vortex (Tempest Beta) | Atari > Tempest Versions > Vortex Tempest Beta |
| `zektor.zip` | Zektor | Sega > Zektor |
| `tacscan.zip` | Tac/Scan | Sega > Tac Scan |
| `startrek.zip` | Star Trek | Sega > Star Trek |
| `spacfury.zip` | Space Fury (Revision C) | Sega > Space Fury |
| `spacfura.zip` | Space Fury (Revision A) | Sega > Space Fury Versions > Space Fury Rev A |
| `spacfurb.zip` | Space Fury (Revision B) | Sega > Space Fury Versions > Space Fury Rev B |
| `elim2.zip` | Eliminator (2 Player Set 1) | Sega > Eliminator |
| `elim2a.zip` | Eliminator (2 Player Set 2A) | Sega > Eliminator Versions > Eliminator 2P Set 2A |
| `elim2c.zip` | Eliminator (2 Player Set 2C) | Sega > Eliminator Versions > Eliminator 2P Set 2C |
| `elim4.zip` | Eliminator (4 Player) | Sega > Eliminator Versions > Eliminator 4 Player |
| `elim4p.zip` | Eliminator (4 Player Prototype) | Sega > Eliminator Versions > Eliminator 4P Prototype |
| `mhavoc.zip` | Major Havoc (Revision 3) | Atari > Major Havoc |
| `mhavoc2.zip` | Major Havoc (Revision 2) | Atari > Major Havoc Versions > Major Havoc Rev 2 |
| `mhavocrv.zip` | Major Havoc (Return To VAX - Mod by Jeff Askey) | Atari > Major Havoc Versions > Major Havoc Return To Vax |
| `mhavocp.zip` | Major Havoc (Prototype) | Atari > Major Havoc Versions > Major Havoc Prototype |
| `alphaone.zip` | Alpha One (Major Havoc Prototype - 3 Lives) | Atari > Major Havoc Versions > Alpha One 3 Lives |
| `alphaona.zip` | Alpha One (Major Havoc Prototype - 5 Lives) | Atari > Major Havoc Versions > Alpha One 5 Lives |
| `starwar1.zip` | Star Wars (Revision 1) | Atari > Star Wars Rev 1 Aae |
| `starwars.zip` | Star Wars (Revision 2) | Atari > Star Wars (its own program, `swsbt.img`, reads this zip) |
| `quantum1.zip` | Quantum (Revision 1) | Atari > Quantum Versions > Quantum Rev 1 |
| `quantum.zip` | Quantum (Revision 2) | Atari > Quantum |
| `quantump.zip` | Quantum (Prototype) | Atari > Quantum Versions > Quantum Prototype |
| `solarq.zip` | Solar Quest | Cinematronics > Solar Quest |
| `starcas.zip` | Star Castle | Cinematronics > Star Castle |
| `armora.zip` | Armor Attack | Cinematronics > Armor Attack |
| `barrier.zip` | Barrier | picker only (the same game also has its own emulator above) |
| `sundance.zip` | Sundance | Cinematronics > Sundance |
| `warrior.zip` | Warrior | Cinematronics > Warrior |
| `ripoff.zip` | RipOff | Cinematronics > Ripoff |
| `tailg.zip` | TailGunner | picker only (the same game also has its own emulator above) |
| `starhawk.zip` | StarHawk (Missing Some Samples) | Cinematronics > Star Hawk |
| `spacewar.zip` | SpaceWar | picker only (the same game also has its own emulator above) |
| `speedfrk.zip` | Speed Freak (No Sound, Input) | Cinematronics > Speedfreak |
| `demon.zip` | Demon (No Sound) | picker only (the same game also has its own emulator above) |
| `boxingb.zip` | Boxing Bugs (No Sound, Input) | Cinematronics > Boxing Bugs |
| `wotw.zip` | War of the Worlds (No Sound) | Cinematronics > War Of The Worlds |

Not in the list, and not supported: Cosmic Chasm (the Cinematronics emulator names it, but that part of the
code is an empty stub), Aztarac, The Empire Strikes Back, and Galaga (the AAE game table has a Galaga slot
but it is not a vector game and has no driver ROMs). The other Cinematronics sets that Malban's `cine.img` knows
(Armor Attack, Boxing Bugs, Ripoff, Solar Quest, Star Castle, Star Hawk, Speed Freak, Sundance, War of the
Worlds, Warrior) are run through AAE in this menu, so for them the `roms/aae/` zip is the one that counts.

ARCADE > ALL AAE GAMES is a picker that lists every zip in `roms/aae/` that AAE has a driver for, including
the ones the menu has no entry for. Button 4 plays, Button 1 goes back.

How well they run (tested on a Pi Zero W, Vectrex with a Buzz-type BIOS, 2026-10-17/18):

| Game | Result |
|---|---|
| Star Wars, Tempest | work (these are the rebuilt programs; the July 2025 ones show a blank screen) |
| Asteroids | works; the explosion sound is missing |
| Black Widow | works, some sound. One controller: hold Button 4 to fire the way the stick last pointed |
| Red Baron, Gravitar, Space Duel | work, with sound |
| Lunar Lander | works; sound not confirmed |
| Asteroids Deluxe, Major Havoc, Quantum (AAE) | work, no sound |
| Other AAE games, the Cinematronics emulator, QB3, the two older "SIM" Asteroids programs | **not tested**. The menu notes copy the original author's remarks (NO SOUND, WORK IN PROGRESS...) |
| Zero 2 builds | built, **not tested on hardware** |

Most of the older Atari emulations (Star Wars, Asteroids, Tailgunner...) are static binary translations
(SBT): the game code was translated ahead of time instead of being emulated, which is why they are fast.

## Settings

SETTINGS (last in the list) opens the screen calibration pages. Press Button 4 to leave: the values are written
to `vectrexInterface.ini` and the screen says **SETTINGS SAVED** (or **SETTINGS NOT SAVED!**). They are read at
every start, so you only set them once per Vectrex.

The installer **keeps your existing `vectrexInterface.ini`**. If the file is missing or empty (the original
menu can leave it empty), it installs the tuned file instead. If the picture or the menu text is unreadable
on your Vectrex with the file from Malban's card, run `install.sh --settings tuned`: it replaces your file
with the one this package was developed with (your original is saved in the backup, `--restore` brings it
back). Those values were tuned on one Vectrex, so treat them as a better starting point, not as the answer.

If vectors look slanted or skewed, use the six CALIBRATION values in SETTINGS (they correct vectors of length
5, 10, 15, 20, 50 and 100); Vectrex monitors differ. `XCAL_VALUE` and `YCAL_VALUE` in the file add a small
correction proportional to the distance from the centre. Background:
[AtariAge calibration guide](https://forums.atariage.com/topic/339338-vectrex-complete-screen-calibration-and-more-definitive-guide/).
Some games also read `<game>.ini` from the card root after `vectrexInterface.ini`, and it overrides the global
values; the ones on Malban's card carry his own calibration.

`vectrex.ini` (section `[VECTREX_EXACT]`): `ROM_FILE` picks the BIOS, `HANDLE_STATES = 0` stops Exact saving a
game state when you leave, `INTERNAL_STRING_DISPLAY = 1` draws the BIOS text with the PiTrex routines.

## Adding your own entries

A new Vectrex `.bin` needs nothing. A new program image (`something.img`, copied into `piZero1/` and
`piZero2/`) shows up under PROGRAMS. To put anything where you want it, or to give it notes, create
`menu/<any name>.ini`:

```
[entry]
title = MY GAME                ; shown in the list (capitals)
category = COMPUTER            ; where it is listed; "ARCADE / ATARI" makes a sub-folder
image = mygame.img             ; the program image in piZero1/
param1 = original/X.BIN        ; first parameter (0x2d = the single byte 0x2d); param2 for a second
needs = roms/mygame.zip        ; hide the entry while this file is missing; a folder ending in / means any entry needing a file there
text = WORKS GREAT.            ; note lines under the list (max 12). NO SOUND, WIP, BROKEN add a "!"
control = FIRE  BUTTON 4       ; lines of the CONTROLS page (Button 2), max 12
hide = 1                       ; leave it out
```

Delete the file and the entry is gone. A file without an `image` line changes something found on the card
instead: `menu/rom.<path with _ for other characters>.ini` for a Vectrex game (for example
`menu/rom.homebrew_e_srg_bin.ini`), `menu/<image name>.ini` for a program, `menu/dir.<folder>.ini` to rename
a ROM folder group. In those, `emulator = speedy` makes Button 4 use Speedy for that game. Write text in capitals
using only letters, digits, `.` and `!`.

## Known problems

* Audio (Queen, Prince) and video: the player loads the whole 3-10 MB file before it starts, so for several
  seconds the screen shows the Vectrex start screen and then WAVPLAYER; it is not frozen. The sound is quiet:
  turn the Vectrex volume all the way up. The video player plays sound too. Queen is a little scratchy.
* The menu music slows a little on the busiest screens.
* Vectrex BIOS text can shimmer slightly with the 2025 emulators; try the calibration pages.
* The July 2021 and 2023 emulators cannot draw the BIOS text at all (dots), so do not use them.
* The AAE picker does not show the caveat marks.
* `NMLOG.TXT` on the card records the first controls, launches and settings steps; look at it first if something
  freezes.

## If something goes wrong

1. Menu odd or unusable: delete the `NEW_MENU = 1` line in `loader.ini`; the original menu returns.
2. PiTrex does not boot or a game is worse: `install.sh --restore <card folder>` puts the replaced files back.
3. A Vectrex game misbehaves: Button 2 on it (the other emulator).

## Build it yourself

The loader, emulators and arcade programs in this package were built from the source in this repository with
`gcc-arm-none-eabi` 10.3.1 (`docs/tools/build.sh zero1 pitrex/loader vecx.exact vecx.speedy`; `zero2` for the Pi
Zero 2). Asteroids cannot be rebuilt from the repository (its translation step needs the original ROM
`035127-02.np3`), so its image stays the one from Malban's card. Design notes and the test tools are in
[`docs/`](docs/README.md); licensing notes are in [docs/LICENSING.md](docs/LICENSING.md).
