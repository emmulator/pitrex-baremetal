# The new PiTrex menu

This fork adds an optional **list menu** to the PiTrex baremetal loader. It does not replace the
original menu: it is switched on with one line in `loader.ini`, and removing that line brings the
original back. Everything the original menu starts is still started the same way.

What it adds:

* A simple list: pick a category, go in, pick a game, start it. No icons to decode.
* **Games are found on the card.** Copy a `.bin` into `vectrex/` and it shows up. No recompiling.
* Notes and warnings (NO SOUND, WORK IN PROGRESS...) live in small text files on the card, and you
  can edit them.
* A picker for the arcade emulator (AAE) that lists the arcade games whose ROM files you have.
* VECTORBURNER under COMPUTER. That one is a compiled-in entry (added to the loader, so the original menu has it
  too). Any *other* program image you drop into `piZero1/` that has no compiled entry shows up by itself under
  PROGRAMS. Everything is rebuilt and sorted at every boot, so new games and programs land in alphabetical order.
* On-screen help: the line under the list always says what the buttons do.

Status: tested on a real PiTrex (Pi Zero W, Vectrex with a Buzz-type BIOS) on 2026-10-17. The list,
settings saving, ROM discovery, and starting games from Exact and Speedy work, **provided the emulator
images are built from this source** (see [Emulator images](#emulator-images-important)).

Design notes, findings and the build tooling are in [`docs/`](docs/README.md).

---

## Turning it on and off

On the SD card, `loader.ini` (in the root of the card):

```
NEW_MENU = 1
```

Remove the line (or set `0`) and the original menu is back. Nothing else needs to change.
Installing from the `pitrex-builds` folder with `card.sh install <set> <card> --new-menu` does
this for you and keeps a backup of everything it replaced (`card.sh restore` undoes it).

## Controls

The menu opens at the top (the category list) when the PiTrex boots. It ignores the controls until they
have rested for 15 frames. After a game returns to the loader it may remember where you were.

| You press | What happens |
|---|---|
| Stick **up / down** | Move the `>` marker. Hold to repeat. |
| Stick **right**, or **Button 4** | Open a category, or start the highlighted entry. |
| Stick **left** | Go back up one level. |
| **Button 4** on a Vectrex game file | Start it with the default emulator (Exact). |
| **Button 2** on a Vectrex game file | Start the same game with the **other** emulator (Speedy). |
| **Button 2** on an arcade game or program that has a controls page | Show how the controls work (the hint under the list says "2 SHOWS THE CONTROLS"). Any button or stick left comes back. |
| **Button 1** | Same as Button 4, but tells the emulator to load the last saved state (the original menu's behaviour, see FAQ 34). Only Vectrex emulators use it. |

Buttons 1 and 2 are **not** "emulator one and emulator two". Button 4 is the normal start,
Button 2 swaps the emulator, Button 1 asks for a saved state.

### What the screen means

```
VECTREX > ALL ROMS > DEMOS        <- where you are
   BRESENHAM
 > EPIC                           <- ">" is the highlighted entry
 $ SOMETHING                      <- "$" is a category (a folder): go in with 4 or right
   BERZERK                     !  <- "!" means a caveat, read the notes under the list
 ...
 (notes or help for the highlighted entry)
 4 STARTS WITH EXACT              <- what the buttons do right now
 2 STARTS WITH SPEEDY
```

The PiTrex's built-in font only draws letters, digits, `.`, `!` and a few arrows. Brackets,
slashes, dashes and colons print as blanks, so the menu uses `>`, `$` and `!` instead, and so
should any text you write for it.

When you start something, a "STARTING ..." screen shows for a moment and names the program and
the emulator. If the screen then goes blank, the program itself is what failed, not the menu.

## What is in the menu

```
SETTINGS            screen settings and calibration
VECTREX
  VECTORBLADE
  SPEEDY VECTREX    8 games the original author set up for Speedy
  EXACT VECTREX     3 games set up for Exact
  DEMOS             the demos: the 5 the author set up plus any others in vectrex/demos
  ALL ROMS          every .bin found in vectrex/ (see below)
ARCADE
  ATARI, CINEMATRONICS, ... compiled-in arcade games
  ALL AAE GAMES     the picker: only the games whose ROM zip is in roms/aae/
COMPUTER            ZBlast, Hyperoids, GS BASIC, Elite, VTerm, VECTORBURNER ...
VIDEO               video players (need files in vectrex/movies)
AUDIO               music players (need files in vectrex/waves)
PROGRAMS            programs found in piZero1/ that the menu does not know by name
```

Highlight a category and the lines under the list explain it. All lists below the top level are in
alphabetical order (inside ALL ROMS the folders come first, then the loose files).

### The Vectrex games: preset groups and ALL ROMS

* **SPEEDY VECTREX**, **EXACT VECTREX** and **DEMOS** are listed alphabetically. The old PICK A GAME
  browser is gone: ALL ROMS does its job (and PICK A GAME started a game by itself).
* **SPEEDY VECTREX** and **EXACT VECTREX** are the original author's presets: games he tested with
  that emulator. They are always listed in the group of the emulator he chose.
* **ALL ROMS** lists **every** `.bin` file on the card under `vectrex/`, including the preset
  games. So yes: a preset game appears twice, once in its emulator's group and once in ALL ROMS.
  Both start the same file.
* Games in ALL ROMS start with **Exact** on Button 4 and **Speedy** on Button 2. You can change the
  default per game, see the sidecar section.
* Sub-folders of `vectrex/` (up to two levels) become groups inside ALL ROMS, so you can sort
  your own games into folders. A group is named after the folder with FOLDER added (`ORIGINAL FOLDER`,
  `HOMEBREW FOLDER`): the name says where the file is on the card and **nothing about the kind of game**.
  `original/` is just the author's folder for the games his menu presets use; it also holds a few
  newer homebrew releases. To rename a group, put `title = ...` in `menu/dir.<folder>.ini`
  (`menu/dir.original.ini`). Files not in any folder are listed after the groups. A loose file whose
  name also exists in a folder is treated as a copy and left out. `bios`, `movies` and `waves` are ignored.
* A game's name in the list is its file name: upper-cased, `_` and `-` turned into spaces, `.bin`
  dropped. `E-SRG.bin` shows as `E SRG`.
* Only files ending in `.bin` are listed. Rename `.vec` files to `.bin`.

### Speedy or Exact?

They are the same emulator built two ways, and **either one can run any `.bin`**.

| | Speedy | Exact |
|---|---|---|
| Speed | Games run about 30-50% faster than a real Vectrex, output at 50 Hz | As close to a real Vectrex as the author could make it |
| Risk | Skips some BIOS work, so a few games misbehave | Known weak spot: 3D Imager games |
| BIOS | built-in | loads the file named by `ROM_FILE` in `vectrex.ini` (in `vectrex/bios/`), else uses a built-in one |

Start with Exact. If a game is too slow or looks wrong, press Button 2 to try the other one.

### Arcade games

ARCADE > ALL AAE GAMES is a picker built into the arcade emulator. It lists the games it knows
that have a zip in `roms/aae/` (70 of its 71 known games have one on the prepared card). Button 4
plays, Button 1 backs out. The emulator reads only `roms/aae/`, and the zip file name must be the
MAME name (`mhavoc.zip`, `alphaone.zip`). It never checks file contents, so a wrong ROM set
starts but misbehaves. The author's notes say most AAE games have **no sound**; the menu shows
those notes and a `!`.

---

## Putting your own game on the card

1. Copy the `.bin` into `vectrex/` (or a sub-folder such as `vectrex/homebrew/`).
2. Put the card in the PiTrex. It appears under VECTREX > ALL ROMS.

That is all. The list is built every time the menu starts, so a long list makes the menu take a
moment longer to appear.

## Arcade games on this PiTrex

What was seen working or not on the test PiTrex (Pi Zero W, 2026-10-17):

| Game | Result |
|---|---|
| Asteroids (SBT) | works; explosion sounds missing (other sounds work, cause not found) |
| Asteroids Deluxe, Major Havoc, Quantum (AAE) | work, no sound (as the notes say) |
| Black Widow | works, some sound. Needs two sticks: see CONTROLS. One-stick mode added. |
| Red Baron | works, some sound |
| Gravitar, Space Duel | work, with sound |
| Lunar Lander | works; the code plays samples (`samples/lunar/*.raw`), whether they play was not confirmed |
| Video and Audio entries | did not start with the shipped player images; rebuilt (not yet confirmed) |
| Star Wars, Tempest | **went blank** with the shipped images. Rebuilt from source and the author's `starwars.ini` calibration moved aside; result not yet known. |

SBT = *static binary translation*: the original game code was translated to C ahead of time instead of
being emulated, which is why these run fast. Star Wars, Asteroids and Tailgunner are done this way (and
some of the others use translated pieces), so the menu does not put "SBT" in the titles.

**Black Widow** is a two-stick game (one moves, one fires in a direction). With one controller the
original port 2 stick reads as pushed fully up, so it fired straight up all the time. `blackwidow.ini`
in the card root now has `ONE_STICK = 1` (the default): move with the stick, hold Button 4 to fire the
way the stick last pointed. Set `ONE_STICK = 0` if a second stick is plugged into port 2.

**Per-game ini files and calibration.** Games read `<game>.ini` from the card root *after*
`vectrexInterface.ini`, and it overrides the global values. The files that came on the July 2025 card
(`lunar.ini`, `starwars.ini`) carry the *author's own* screen calibration (offsets, sizes, calibration
numbers for his Vectrex) and can put the picture off-screen or slanted on another Vectrex. They were renamed
`*.ini.author-calibration` on the test card. `battlezone.ini` was set for the author's special
two-controller board; it now has `ONE_STICK`-style single stick settings (`ONLY_ONE_JOYSTICK = 1`,
`STRAMASH_INPUT_2 = 0`), the original is `battlezone.ini.author`.

**Slanted pictures.** The SETTINGS pages have six CALIBRATION values (5, 10, 15, 20, 50, 100): the
correction for vectors of that length, and the on-screen help says "if vectors are displayed skewed, try
use calibration to improve the look; Button 3 displays different vector lists". The global values on the test
card were left near zero. Vectrex monitors differ, so these are per-machine. Also `XCAL_VALUE` and
`YCAL_VALUE` apply a small correction proportional to the distance from the centre, and X SIZE / Y SIZE
scale. General background for real Vectrex alignment (aged integrator capacitors, DAC offset) is in the
[AtariAge calibration guide thread](https://forums.atariage.com/topic/339338-vectrex-complete-screen-calibration-and-more-definitive-guide/);
nothing PiTrex-specific was found online.

## Notes, hiding and the default emulator: sidecar files

For each entry the menu looks for a small text file in `menu/` on the card. If it exists, it
changes how the entry is shown. Delete the file to get the default back.

The file name is a **key** plus `.ini`:

| Entry | Key | Example |
|---|---|---|
| A Vectrex game file | `rom.` + path with every run of other characters as `_`, lower case | `vectrex/homebrew/E-SRG.bin` is `menu/rom.homebrew_e_srg_bin.ini` |
| A program (`piZero1/foo.img`) | image name, lower case | `menu/foo.ini` |
| A compiled entry with parameters | image name + `.` + parameter | `menu/vectrexspeedy.original_webwars_bin.ini`, `menu/aae.x2d.ini` |

All keys of the entries that ship with the menu are listed in `docs/menu/sidecar/keys.tsv`; the
66 files that came with this build already sit in `menu/` on the card, use them as examples.

Contents (each line is `name = value`; `;` starts a comment):

```
[entry]
text = WORKS GREAT.            ; a note line, repeat for more lines (max 12)
text = "  KEEPS LEADING SPACES" ; quote a line to keep spaces
skip = 2                       ; blank lines before the first note line
hide = 1                       ; leave this entry out of the menu
title = MY GAME                ; rename a game found on the card (ROMs and programs)
control = FIRE  BUTTON 4       ; a line of the CONTROLS page (Button 2), repeat for more lines
emulator = speedy              ; which emulator Button 4 uses for a ROM: speedy or exact
```

Write notes in capitals using only letters, digits, `.` and `!`: other punctuation will not draw.
A note containing NO SOUND, WIP / WORK IN PROGRESS, or BROKEN / NOT WORKING / CRASHING makes the
entry show a `!` in the list. Full format: [docs/menu/05-sidecar-format.md](docs/menu/05-sidecar-format.md).

## Settings

SETTINGS (top of the list) opens the screen-calibration pages. Press Button 4 to leave; the values
are written to `vectrexInterface.ini` on the card and the screen then says **SETTINGS SAVED** (or
**SETTINGS NOT SAVED!** if the file did not get written). Those values are read every time the
PiTrex starts, so they only need setting once per Vectrex.

---

## Emulator images (important)

The Vectrex emulators (`vectrexexact.img`, `vectrexspeedy.img`) that came on the July 2025 SD card
image **went to a blank, frozen screen as soon as a game started** on a Pi Zero W. The same emulators
built from the source in this repository run Berzerk, Armor Attack, Mr Boston and E-SRG. The shipped
images were built from slightly older source than the repository's last commit.
The December 2021 and April 2023 emulators also run games but cannot draw the BIOS text (it shows as
dots, scores are missing); only the 2025 source has `INTERNAL_STRING_DISPLAY`.

**Where the shipped images came from.** The card used for testing was made from `PiTrex_sd_Jul29_25.zip`
(a PiTrex bare-metal SD card image; its `changelog.txt` is dated 23 May 2025 and the files in it are dated
16-23 May 2025, so it predates the repository's last upstream commit of 24 May 2025). Its FAQ points to
[rpiteam.carrd.co](https://rpiteam.carrd.co) and the author's site <http://vide.malban.de/>; the download
location was not recorded, so please add it here. The same card also got arcade ROM sets and the
Vectorburner images.

Images rebuilt from this source and installed on the test card so far: `vectrexexact`, `vectrexspeedy`,
`vectrexvideo`, `vectrexaudio`, `vectrexlineart`, `vectorblade`, `swsbt` (Star Wars), `tempest` and
`blackwidow`. The reasons: the shipped Exact and Speedy blanked; Star Wars and Tempest blanked; the
Video and Audio entries did not start; Black Widow needed the one-stick fire. Games that worked from the
shipped images (Asteroids, Gravitar, Space Duel, Red Baron, Lunar Lander...) were left alone. A rebuild of
every other program in the tree is available for comparison (`pitrex-builds/all-fork-source-gcc10.3.1`).

Build them yourself (needs the `pitrex-dev` container, see `docs/tools/build.sh`):

```
docs/tools/build.sh zero1 pitrex/loader vecx.exact vecx.speedy      # Pi Zero / Zero W
docs/tools/build.sh zero2 pitrex/loader vecx.exact vecx.speedy      # Pi Zero 2
```

and copy `vecx.exact/vectrexexact.img`, `vecx.speedy/vectrexspeedy.img` from the scratch folder the
script prints into `piZero1/` (or `piZero2/`) on the card. Zero 2 builds are untested. Other emulators
(`vectrexlineart`, `vectrexvideo`, `vectrexaudio`, `vectorblade`) were not rebuilt; they may have the same
problem.

`vectrex.ini` settings that matter (file in the card root, section `[VECTREX_EXACT]`):

| Setting | Effect |
|---|---|
| `ROM_FILE = bios/7ADB.bin` | which BIOS Exact loads from `vectrex/bios/` (the card also has `FASTBOOT.IMG`) |
| `HANDLE_STATES = 0` | do not save the game state when you leave with Button 4 (saving can freeze on slow cards) |
| `INTERNAL_STRING_DISPLAY = 1 or 0` | draw the BIOS text with the PiTrex routines (1) or from the emulated Vectrex (0). Compare both: the text can wobble. |
| `CYCLE_EQUIVALENT = 648` | emulation timing; also settable in SETTINGS |

## Known problems

* The BIOS text in Vectrex games is readable with the 2025 emulators but can shimmer: it flickers
  between bright and dim and shifts sideways between frames. Not yet understood; try the settings above
  and the calibration pages in SETTINGS.
* The 2021 and 2023 emulators cannot draw BIOS text at all, so do not use them.
* Speedy was only seen working from the debug-logging build; the clean build is untested.
* The menu ignores the controls for the first moments (until they have rested for 15 frames). Button 4
  reads as pressed at power-on if you hold it to skip the Vectrex intro; the wait keeps that from
  opening VECTREX.
* After you leave a game the list may start at the top again (not checked).
* The CONTROLS page (Button 2) used to stay blank while the button was held and left the menu music's last
  tone sounding; it now shows at once and silences the tone.
* Caveat marks are not shown in the AAE picker.
* The menu reads `.bin` files two folder levels deep only.
* `NMLOG.TXT` in the card root records the controls on the first frame, the launches and the SETTINGS
  steps. It is rewritten on every boot; look at it first when something freezes.

Fixed during the hardware test: the settings were never saved and leaving SETTINGS froze the PiTrex.
Cause: the upstream ini writer printed a `uint8_t` with `%f` (`vectrexInterface_settings.c`, now `%i`).
It left an empty `vectrexInterface.ini`, and an unreadable ini makes later saves do nothing.

## If something goes wrong

1. Menu odd or unusable: take the card to a computer, delete the `NEW_MENU = 1` line in
   `loader.ini`. The original menu returns.
2. PiTrex does not boot: `card.sh restore <card>` puts back the images that were replaced
   (they were copied to `backup-<date>/` on the card at install time).
3. A game misbehaves: Button 2 on it, or `emulator = ` in its sidecar.

## Where the code is

| File | What it does |
|---|---|
| `pitrex/loader/newMenuLogic.h` | the tree, discovery, navigation, help texts. No hardware calls, tested on a PC. |
| `pitrex/loader/newMenuUi.h` | drawing, buttons, launching, scanning the card |
| `pitrex/loader/menuSidecar.h`, `menuSidecarIo.h` | sidecar key rules and file reading |
| `pitrex/loader/loaderMain.c` | hooks the new menu in behind `NEW_MENU`, adds the VECTORBURNER and ALL AAE GAMES entries |
| `aae/aaeSelect.h`, `aae/aaemain.c` | the arcade game picker |
| `docs/` | findings, the format specs, tests, the build script and the desktop preview |

Licensing of the upstream code is discussed in [docs/LICENSING.md](docs/LICENSING.md).
