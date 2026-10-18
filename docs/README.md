# PiTrex new menu: plug and play

A replacement menu for the PiTrex bare-metal SD card: a plain list (category, game, start) that is built from
what is actually on the card. A game appears when its files are there and is left out when they are not, and
every game has notes and a controls page. Nothing is compiled in.

This page is for installing it and getting your games running. If you want to change how the menu looks, add
your own games or edit the entry files, read [NEW_MENU_README.md](../NEW_MENU_README.md) instead.

No ROMs are included in this package. Below is exactly which ROM files the menu looks for, which of them come
with the RpiTeaM image, and which you would have to find elsewhere.

## What you need to download

| | From | Why |
|---|---|---|
| **Malban's 2025 card**, `PiTrex_sd_Jul29_25.zip` | <http://vide.malban.de/pitrex/pitrex-baremetal-download> | The PiTrex software itself, the newest (23 May 2025). It has the Vectrex games, BIOS files and demos, but **no arcade ROMs**. |
| **The RpiTeaM image**, `PitreX_Baremetal_RpiTeaM_Edition.img` (optional) | <https://rpiteam.carrd.co/> | Only needed for its arcade ROM zips (`roms/` and `roms/aae/`). Its software is from December 2021 and is older than Malban's; **nothing but the ROM zips is taken from it**. |
| **This package** | the release page | The new menu and the rebuilt programs. |

(Malban's FAQ also points to the RpiTeaM site for arcade ROMs.) The RpiTeaM image is large (about 4 GB).

## Installing

### A. You already have Malban's 2025 card (or its zip, unpacked)

Run the installer with the folder (or the SD card), then copy the folder to the card if it is not on it yet:

| System | Command |
|---|---|
| Linux, macOS | `./install.sh /path/to/PiTrex_sd_Jul29_25` |
| Windows | `install.bat C:\path\to\PiTrex_sd_Jul29_25` |

### B. Build a complete card from the two downloads

`make_card.py` (needs Python 3) unpacks Malban's zip, reads the arcade ROM zips out of the RpiTeaM image
(no mounting, no administrator rights), and lays the new menu over the result:

```
python3 make_card.py PiTrex_sd_Jul29_25.zip PitreX_Baremetal_RpiTeaM_Edition.img my_card
```

It writes the folder `my_card` and `my_card.zip`. Copy the **contents** of `my_card` to a FAT32 SD card. No
Python? Open the `.img` with 7-Zip (or mount it), copy its `roms` folder over the unpacked 2025 card, and use
route A.

### Options (both routes)

* `--dry-run` (route A only) shows what would change and writes nothing.
* `--settings tuned` replaces your screen settings with the tuned ones, see [Settings](#settings).
* `--arcade-tweaks` adds single-controller files for Black Widow and Battle Zone and sets the original author's
  Star Wars and Lunar Lander calibration files aside (they are for his Vectrex).
* `--restore` (route A) undoes the newest install.

The installer saves every file it replaces in `backup-menu-<date>/` first, never touches ROMs, BIOS files
or your own settings, and can be run twice safely. It replaces the boot files `pitrex.img` and `pitrex7.img` (they must match the loader), the loader, AAE (with its
game picker), the Vectrex emulators and players (the July 2025 emulators show a blank screen on a Pi Zero W), three rebuilt arcade programs (Star
Wars, Tempest, Black Widow), the entry files in `menu/`, and adds `NEW_MENU = 1` to `loader.ini`.
To go back to the original menu, delete that line.

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

Games are listed alphabetically. **A game whose ROM is not on the card is not listed**; it appears by itself once
the file is there. (To list every game anyway, with a "ROM NOT FOUND!" note, add `NEW_MENU_SHOW_MISSING = 1` to
`loader.ini`.)

Speedy and Exact are one Vectrex emulator built two ways and either runs any game. Exact is as close to a real
Vectrex as the author could make it; Speedy runs games 30-50% faster and skips some BIOS work, so a few games
misbehave. Start with Exact and press Button 2 if a game is too slow or looks wrong.

## Where the ROMs go

The arcade emulators read the zip files themselves and only check that the files inside exist, so a wrong
version starts but misbehaves. MAME-named zips work.

### Vectrex games

| What | Where |
|---|---|
| Vectrex games | `vectrex/*.bin`, or in sub-folders such as `vectrex/original/` or `vectrex/homebrew/` (two levels). Anything ending in `.bin` shows up under VECTREX > ALL ROMS. Rename `.vec` files to `.bin`. |
| Vectrex BIOS | `vectrex/bios/` (Malban's card has four; `ROM_FILE` in `vectrex.ini` picks the one Exact uses) |
| Music and video | `vectrex/waves/`, `vectrex/movies/` and the players: already on Malban's card. **Turn the Vectrex volume all the way up**, the sound goes through the video DAC and is quiet. |

### Arcade games

Some games have their own emulator; the rest run on AAE ("Another Arcade Emulator"). The two kinds read
**different zip folders**.

**Own emulator: zips go in `roms/`.** All of these are in the RpiTeaM image.

| Zip | Game | Menu | Program |
|---|---|---|---|
| `astdelux.zip` | Asteroids Deluxe (older emulator) | Atari | `deluxe.img` |
| `asteroid.zip` | Asteroids (older emulator; also the second set Asteroids reads) | Atari | `asteroids.img` |
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

**AAE: zips go in `roms/aae/`.** The last column says whether the RpiTeaM image has the zip. The ten that it
does not have are other versions of games it does have; everything else works without them.

| Zip | Game | Menu | In the RpiTeaM image |
|---|---|---|---|
| `llander1.zip` | Lunar Lander (Revision 1) | Atari > Lunar Lander Versions > Lunar Lander Rev 1 | yes |
| `llander.zip` | Lunar Lander (Revision 2) | picker only (the game has its own emulator above) | yes |
| `meteorts.zip` | Meteorites (Asteroids Bootleg) | Atari > Asteroids Versions > Meteorites Bootleg | yes |
| `asterock.zip` | Asterock (Asteroids Bootleg) | Atari > Asteroids Versions > Asterock Bootleg | yes |
| `asteroib.zip` | Asteroids (Bootleg on Lunar Lander Hardware) | Atari > Asteroids Versions > Asteroids Lunar Hw Bootleg | yes |
| `asteroi1.zip` | Asteroids (Revision 1) | Atari > Asteroids Versions > Asteroids Rev 1 | yes |
| `asteroid.zip` | Asteroids (Revision 2) | Atari > Asteroids Versions > Asteroids Rev 2 | yes |
| `astdelu1.zip` | Asteroids Deluxe (Revision 1) | Atari > Asteroids Versions > Asteroids Deluxe Rev 1 | yes |
| `astdelu2.zip` | Asteroids Deluxe (Revision 2) | Atari > Asteroids Versions > Asteroids Deluxe Rev 2 | yes |
| `astdelux.zip` | Asteroids Deluxe (Revision 3) | Atari > Asteroids Deluxe | yes |
| `omegrace.zip` | Omega Race | Other Arcade > Omega Race | yes |
| `deltrace.zip` | Delta Race (Omega Race Bootleg) | Other Arcade > Delta Race | yes |
| `bzone.zip` | Battlezone (Revision 1) | picker only (the game has its own emulator above) | yes |
| `bzone2.zip` | Battlezone (Revision 2) | Atari > Battle Zone Versions > Battle Zone Rev 2 | yes |
| `bzonec.zip` | Battlezone Cocktail Proto | Atari > Battle Zone Versions > Battle Zone Cocktail | yes |
| `bzonep.zip` | Battlezone Plus (Clay Cowgill) | Atari > Battle Zone Versions > Battle Zone Plus | yes |
| `redbaron.zip` | Red Baron | picker only (the game has its own emulator above) | yes |
| `bradley.zip` | Bradley Trainer | Atari > Bradley Trainer | no |
| `spacduel.zip` | Space Duel | picker only (the game has its own emulator above) | yes |
| `bwidow.zip` | Black Widow | picker only (the game has its own emulator above) | yes |
| `gravitar.zip` | Gravitar (Revision 3) | picker only (the game has its own emulator above) | yes |
| `gravitr2.zip` | Gravitar (Revision 2) | Atari > Gravitar Versions > Gravitar Rev 2 | no |
| `gravp.zip` | Gravitar (Prototype) | Atari > Gravitar Versions > Gravitar Prototype | no |
| `lunarbat.zip` | Lunar Battle (Prototype, Late) | Atari > Lunar Lander Versions > Lunar Battle Late | yes |
| `lunarba1.zip` | Lunar Battle (Prototype, Early) | Atari > Lunar Lander Versions > Lunar Battle Early | yes |
| `tempestm.zip` | Tempest Multigame (1999 Clay Cowgill) | Atari > Tempest Versions > Tempest Multigame | yes |
| `tempest.zip` | Tempest (Revision 3) | picker only (the game has its own emulator above) | yes |
| `tempest3.zip` | Tempest (Revision 2B) | Atari > Tempest Versions > Tempest Rev 2B | yes |
| `tempest2.zip` | Tempest (Revision 2A) | Atari > Tempest Versions > Tempest Rev 2A | yes |
| `tempest1.zip` | Tempest (Revision 1) | Atari > Tempest Versions > Tempest Rev 1 | yes |
| `temptube.zip` | Tempest (Revision 1) | Atari > Tempest Versions > Tempest Rev 1 Tubes | yes |
| `alienst.zip` | Aliens (Tempest Alpha) | Atari > Tempest Versions > Aliens Tempest Alpha | yes |
| `vbreak.zip` | Vector Breakout (1999 Clay Cowgill) | Other Arcade > Vector Breakout | yes |
| `vortex.zip` | Vortex (Tempest Beta) | Atari > Tempest Versions > Vortex Tempest Beta | yes |
| `zektor.zip` | Zektor | Sega > Zektor | yes |
| `tacscan.zip` | Tac/Scan | Sega > Tac Scan | yes |
| `startrek.zip` | Star Trek | Sega > Star Trek | yes |
| `spacfury.zip` | Space Fury (Revision C) | Sega > Space Fury | yes |
| `spacfura.zip` | Space Fury (Revision A) | Sega > Space Fury Versions > Space Fury Rev A | yes |
| `spacfurb.zip` | Space Fury (Revision B) | Sega > Space Fury Versions > Space Fury Rev B | yes |
| `elim2.zip` | Eliminator (2 Player Set 1) | Sega > Eliminator | yes |
| `elim2a.zip` | Eliminator (2 Player Set 2A) | Sega > Eliminator Versions > Eliminator 2P Set 2A | yes |
| `elim2c.zip` | Eliminator (2 Player Set 2C) | Sega > Eliminator Versions > Eliminator 2P Set 2C | yes |
| `elim4.zip` | Eliminator (4 Player) | Sega > Eliminator Versions > Eliminator 4 Player | yes |
| `elim4p.zip` | Eliminator (4 Player Prototype) | Sega > Eliminator Versions > Eliminator 4P Prototype | yes |
| `mhavoc.zip` | Major Havoc (Revision 3) | Atari > Major Havoc | yes |
| `mhavoc2.zip` | Major Havoc (Revision 2) | Atari > Major Havoc Versions > Major Havoc Rev 2 | no |
| `mhavocrv.zip` | Major Havoc (Return To VAX - Mod by Jeff Askey) | Atari > Major Havoc Versions > Major Havoc Return To Vax | no |
| `mhavocp.zip` | Major Havoc (Prototype) | Atari > Major Havoc Versions > Major Havoc Prototype | no |
| `alphaone.zip` | Alpha One (Major Havoc Prototype - 3 Lives) | Atari > Major Havoc Versions > Alpha One 3 Lives | no |
| `alphaona.zip` | Alpha One (Major Havoc Prototype - 5 Lives) | Atari > Major Havoc Versions > Alpha One 5 Lives | no |
| `starwar1.zip` | Star Wars (Revision 1) | Atari > Star Wars Rev 1 Aae | yes |
| `starwars.zip` | Star Wars (Revision 2) | Atari > Star Wars (its own program, `swsbt.img`, reads this zip) | yes |
| `quantum1.zip` | Quantum (Revision 1) | Atari > Quantum Versions > Quantum Rev 1 | no |
| `quantum.zip` | Quantum (Revision 2) | Atari > Quantum | yes |
| `quantump.zip` | Quantum (Prototype) | Atari > Quantum Versions > Quantum Prototype | no |
| `solarq.zip` | Solar Quest | Cinematronics > Solar Quest | yes |
| `starcas.zip` | Star Castle | Cinematronics > Star Castle | yes |
| `armora.zip` | Armor Attack | Cinematronics > Armor Attack | yes |
| `barrier.zip` | Barrier | picker only (the game has its own emulator above) | yes |
| `sundance.zip` | Sundance | Cinematronics > Sundance | yes |
| `warrior.zip` | Warrior | Cinematronics > Warrior | yes |
| `ripoff.zip` | RipOff | Cinematronics > Ripoff | yes |
| `tailg.zip` | TailGunner | picker only (the game has its own emulator above) | yes |
| `starhawk.zip` | StarHawk (Missing Some Samples) | Cinematronics > Star Hawk | yes |
| `spacewar.zip` | SpaceWar | picker only (the game has its own emulator above) | yes |
| `speedfrk.zip` | Speed Freak (No Sound, Input) | Cinematronics > Speedfreak | yes |
| `demon.zip` | Demon (No Sound) | picker only (the game has its own emulator above) | yes |
| `boxingb.zip` | Boxing Bugs (No Sound, Input) | Cinematronics > Boxing Bugs | yes |
| `wotw.zip` | War of the Worlds (No Sound) | Cinematronics > War Of The Worlds | yes |


Not supported, whatever ROMs you have: Cosmic Chasm (the Cinematronics emulator names it, but that part of the
code is an empty stub), Aztarac, The Empire Strikes Back, and Galaga. The other Cinematronics sets Malban's
`cine.img` knows (Armor Attack, Boxing Bugs, Ripoff, Solar Quest, Star Castle, Star Hawk, Speed Freak, Sundance,
War of the Worlds, Warrior) are run through AAE here, so for those the `roms/aae/` zip is the one that counts.
The same games are also in `roms/` in the RpiTeaM image; the menu does not use those copies.

ARCADE > ALL AAE GAMES is a picker that lists every zip in `roms/aae/` that AAE has a driver for, including
the ones the menu has no entry for. Button 4 plays, Button 1 goes back.

How well they run (tested on a Pi Zero W, Vectrex with a Buzz-type BIOS, 2026-10-17/18):

| Game | Result |
|---|---|
| Star Wars, Tempest | work (the rebuilt programs; the July 2025 ones show a blank screen) |
| Asteroids | works; the explosion sound is missing |
| Black Widow | works, some sound. One controller: hold Button 4 to fire the way the stick last pointed |
| Red Baron, Gravitar, Space Duel | work, with sound |
| Lunar Lander | works; sound not confirmed |
| Asteroids Deluxe, Major Havoc, Quantum (AAE) | work, no sound |
| Other AAE games and versions, the Cinematronics emulator, QB3, the two older "SIM" Asteroids programs | **not tested**. The menu notes copy the original author's remarks (NO SOUND, WORK IN PROGRESS...) |
| Pi Zero 2 | built, **not tested on hardware** |

## Settings

SETTINGS (last in the list) opens the screen calibration pages. Press Button 4 to leave: the values are written
to `vectrexInterface.ini` and the screen says **SETTINGS SAVED** (or **SETTINGS NOT SAVED!**). They are read at
every start, so you only set them once per Vectrex.

The installer **keeps your existing `vectrexInterface.ini`**. If the file is missing or empty (the original
menu can leave it empty), it installs the tuned file instead. If the picture or the menu text is unreadable
on your Vectrex with the file from Malban's card, install with `--settings tuned`: it replaces your
`vectrexInterface.ini` and `vectrex.ini` with the ones this package was developed with (your originals are saved in
the backup). Those values were tuned on one Vectrex, so treat them as a better starting point, not as the answer.

The tuned `vectrex.ini` differs from Malban's in the BIOS line: `ROM_FILE = bios/7ADB.bin` (the original Vectrex BIOS,
so the Vectrex intro plays) instead of `bios/FASTBOOT.IMG`. Both files have `INTERNAL_STRING_DISPLAY = 1` (the emulator
draws the BIOS text itself) and leave `CYCLE_EQUIVALENT` commented out. On the test Vectrex, the text of the Vectrex
boot screen and of games (scores, "GAME OVER") was steady with exactly that combination, and shimmered or was garbled
with `FASTBOOT.IMG` in an earlier test, and also with `7ADB.bin` once `CYCLE_EQUIVALENT = 648` was switched on in
`vectrex.ini` (that line overrides the one in `vectrexInterface.ini`). `FASTBOOT.IMG` (which skips the Vectrex intro and
boots faster) looked fine again in a later test, so it is worth trying if the intro bothers you. If your game text
shimmers, check those two lines first.

If vectors look slanted or skewed, use the six CALIBRATION values in SETTINGS (they correct vectors of length
5, 10, 15, 20, 50 and 100); Vectrex monitors differ. Background:
[AtariAge calibration guide](https://forums.atariage.com/topic/339338-vectrex-complete-screen-calibration-and-more-definitive-guide/).
Some games also read `<game>.ini` from the card root after `vectrexInterface.ini`, and it overrides the global
values; the ones on Malban's card carry his own calibration (that is what `--arcade-tweaks` sets aside).

## Known problems

* Audio (Queen, Prince) and video: the player loads the whole 3-10 MB file before it starts, so for several
  seconds the screen shows the Vectrex start screen and then WAVPLAYER; it is not frozen. The sound is quiet:
  turn the Vectrex volume all the way up. The video player plays sound too. Queen is a little scratchy.
* The menu music slows a little on the busiest screens.
* Vectrex BIOS text (boot screen, scores) can shimmer or be garbled depending on `ROM_FILE` and `CYCLE_EQUIVALENT` in
  `vectrex.ini`: see [Settings](#settings). `--settings tuned` installs a combination that was steady on the test Vectrex.
* The AAE picker does not show the caveat marks.
* `NMLOG.TXT` on the card records the first controls, launches and settings steps; look at it first if something
  freezes.

## If something goes wrong

1. Menu odd or unusable: delete the `NEW_MENU = 1` line in `loader.ini`; the original menu returns.
2. PiTrex does not boot, or a game is worse: `install.sh --restore <card folder>` puts the replaced files back.
3. A Vectrex game misbehaves: Button 2 on it (the other emulator).

## Credits

Built from Malban's open-source PiTrex bare-metal code (<https://github.com/malbanGit/pitrex-baremetal>), with
thanks to Kevin Koster for the PiTrex hardware and low-level code. Licensing notes: [LICENSING.md](LICENSING.md).
Developer notes for this fork: [DEVELOPMENT.md](DEVELOPMENT.md).
