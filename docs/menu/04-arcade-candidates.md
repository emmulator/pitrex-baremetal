# Arcade candidates

Which arcade titles the PiTrex can run, which are not in the menu, and how that compares
with the UVMC2 collection. Status: **Verified** = read in source or checked on disk;
**Unverified** = not tried on hardware (quality, speed and sound are unknown until then).
Checked 2026-10-17 against upstream `457c5a9` and the owner's SD card layout.

## Four kinds of arcade entry (not all are AAE)

| Kind | Launched with | Games | Notes |
|---|---|---|---|
| Dedicated, optimised programs | own `.img` | Asteroids, Battle Zone, Black Widow, Red Baron, Lunar Lander, Gravitar, Space Duel, Tempest, Tailgunner, Star Wars (SBT) | Per FAQ #9/#10 only Vectrex, Tailgunner, Asteroids and Battlezone are hand-optimised with sound |
| AAE (generic emulator) | `aae.img` + one-byte game number (F11) | Asteroids Deluxe, Major Havoc, Quantum, Omega Race, Speed Freak, Armor Attack, Boxing Bugs, Ripoff, Star Castle, Sundance, War of the Worlds, Solar Quest, Star Hawk, Warrior, Zektor, Tac/Scan, Star Trek, Space Fury, Eliminator | 71 game variants in the driver table (`aae/aaemain.c`). Menu notes mark some "work in progress, flicker, no sound" |
| Cinematronics emulator | `cine.img` + MAME-style name | Demon, Space Wars, Barrier (linked); Cosmic Chasm, QB3 (**defined but not linked**) | Name, not number |
| Native programs | own `.img` | ZBlast, Hyperoids, GS BASIC, Elite, WW VI, BZ2, Voom, Rebel Defender, Vectorburner (not yet in menu) | Not arcade ROMs |

## Candidates to add

Games from the AAE driver table that have no menu entry. "On card" = a ROM zip exists in
`roms/` or `roms/aae/` on the card built on 2026-10-17. All **Unverified** on hardware.

### Ready now: ROM on the card, emulator support exists

| Game | Driver / how | Why it is interesting |
|---|---|---|
| Cosmic Chasm | `cine.img`, `cosmicchasm` (entry exists, unlinked) | Cinematronics vector game |
| QB3 (Q*bert's Qubes) | `cine.img`, `qb3` (entry exists, unlinked) | Cinematronics vector game |
| Vector Breakout | AAE `vbreak` (0x20) | Breakout on vector hardware (Clay Cowgill, 1999) |
| Battlezone Plus | AAE `bzonep` (0x0f) | Clay Cowgill's enhanced Battlezone. The existing Battle Zone entry is the plain game |
| Vortex, Aliens | AAE `vortex` (0x21), `alienst` (0x1f) | Tempest prototypes |
| Tempest Multigame | AAE `tempestm` (0x19) | Clay Cowgill multi-game ROM |
| Lunar Battle | AAE `lunarbat` (0x17), `lunarba1` (0x18) | Lunar Lander-hardware prototypes |
| Delta Race | AAE `deltrace` (0x0b) | Omega Race bootleg |

Revisions and bootlegs of games that are already in the menu (Asteroids Deluxe rev 1/2,
Space Fury A/B, Eliminator sets, Star Wars rev 1, Tempest 1/2A/2B, Meteorites, Asterock...)
are probably not worth separate entries. A "variants" sub-menu could hold them.

### Supported, but the ROM is not on the card

| Game | Driver | Missing |
|---|---|---|
| Bradley Trainer | `bradley` (0x11) | `bradley.zip` |
| Major Havoc variants | `mhavoc2`, `mhavocrv` (Return to VAX), `mhavocp`, `alphaone`, `alphaona` | their zips |
| Quantum variants, Gravitar variants | `quantum1`, `quantump`, `gravitr2`, `gravp` | their zips |

### Not supported by the PiTrex emulators

Aztarac and Empire Strikes Back have no AAE driver entry. Adding them would be emulator
work, not menu work. The driver table also lists `galaga` (Namco, 0x46) with no ROM here
and unknown status.

## Comparison with the UVMC2 collection (`~/Downloads/Vectrex`)

Observations from file names and the collection's own `roms/README.md`; archive contents
were listed but nothing was run.

* **The arcade ports are UVMC2-specific.** They are `.um2` files, some with a companion
  `roms/<game>.zip` romset: OutRun, Donkey Kong, Major Havoc, Star Wars, Empire Strikes
  Back, Quantum, Aztarac, Tac/Scan, plus self-contained `0_Xa` builds of Asteroids,
  Battle Zone, Centipede, Omega Race, Red Baron and Wizard of Wor. Other archives are
  named for Amidar, Crazy Climber, Defender, Missile Command, Moon Cresta, Pac-Man,
  Phoenix, Q*bert, Tempest Tube and Way of the Exploding Fist. The PiTrex cannot load
  `.um2`.
* **Overlap:** Asteroids, Battle Zone, Red Baron, Omega Race, Star Wars, Major Havoc,
  Quantum, Tac/Scan.
* **UVMC2 only (vector):** Aztarac, Empire Strikes Back. Also raster-game ports
  (Centipede, Defender, Pac-Man, and so on), which the PiTrex has no equivalent for.
* **PiTrex only:** Lunar Lander, Black Widow, Gravitar, Space Duel, Tempest, Tailgunner,
  most of the Cinematronics and Sega catalogue, Asteroids Deluxe, Cosmic Chasm, QB3.
* **Vectrex `.bin` homebrew is shared.** The collection has 42 `.bin` and 2 `.vec` files
  (for example Wireout, Minestorm variants, Zappy Bird, Neon Arena, Samurai, Scramble
  Wars). A byte comparison found **none of the 44 already on the card**. All can be copied
  to `vectrex/` and run through the Exact emulator's PICK A GAME. `.vec` files must be
  renamed to `.bin` first (ROM guide). **Unverified:** that each runs well on PiTrex.
  3D Imager games are a known exception (FAQ #8).

## Does the PiTrex run MAME?

**No, not on the Pi.** Two related things exist:

* **VTerm (verified in source).** `mame.277.src/` holds a patched MAME 0.277 vector
  device (`src/devices/video/vector.cpp`) that sends vector data over a serial cable at up
  to 921600 baud to the PiTrex's `vterm.img`, so the PiTrex acts as the display for MAME
  running on a PC. The sources say the Linux side is not included ("Since I cannot compile
  MAME with linux at the moment"). The card's `Mame277.win` is a Windows build. Using this
  from Linux would mean building the patch ourselves. **Unverified.**
* **AAE** is the PiTrex's own arcade emulator (not MAME). Its ROM sets are MAME-format
  zips, which is why `roms/` holds MAME names.

## ROM sets added to the card (2026-10-17)

Source: the owner's own collection (`~/Development/emulators/roms/mame_wrong_version/`).
Copied byte-for-byte into `roms/aae/` on the card and verified.

| Set | Result |
|---|---|
| `mhavoc2`, `mhavocrv`, `mhavocp`, `alphaone` | File names match what AAE loads; all CRCs match MAME 0.289 |
| `gravitr2`, `gravp`, `alphaona` | File names match AAE; MAME 0.289 no longer has these sets, so no CRC cross-check was possible |
| `quantum1`, `quantump` | In the zip; `quantump` CRCs match MAME 0.289. `quantum1` could not be cross-checked |
| `bradley` | Rebuilt: the original set lacked the one file AAE wants (`036408.01.bin`). It is the Battlezone PROM (`036408-01.k7`, CRC 5903af03) taken from the card's own `roms/bzone.zip` and stored under the old name. All other files match MAME 0.289 |
| `galaga` | **Not added.** The collection's zip is a different Galaga set (different files and CRCs) from the one AAE expects (`gg1-*` names). The right set is not in the collection. It is also a raster game, so it is low priority |

AAE does not check CRCs (`aae/loaders.c` only looks files up by name), so a name match does not
prove the contents are right. All of these are **Unverified on hardware**.
