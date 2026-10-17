# Existing menu structure

Generated from `pitrex/loader/loaderMain.c` at upstream commit `457c5a9`
(2025-06-24) with `docs/tools/menu_tables.py`. Status: **Verified** against the source;
not re-checked on hardware.

## How to read this

* The menu is a linked structure of `MenuItemNew` records compiled into the loader.
  Each record has: id, display name, directory, **image**, **param1**, **param2**,
  icons, and parent/child/left/right pointers.
* **Image launched** is `IMG_FILE_PREFIX` + file. `IMG_FILE_PREFIX` is `piZero1/` for the
  original Pi Zero / Zero W build of the loader and `piZero2/` for the Zero 2 build
  (shown here as `piZeroN/`).
* **Parameters** are copied into the loader's shared settings and read by the launched
  program (`getLoadParameter(n)`). For the Vectrex emulators param1 is a ROM path
  relative to `vectrex/`. For the `aae.img` arcade emulator it is a **single byte game
  number**, shown as `game #N`. For `cine.img` it is a MAME-style game name.
* Top row, left to right: settings, then five category slots. Left/right moves between
  siblings, down enters a category, up goes back to the parent.
* Category slots have no name in the data. They are drawn from an icon.
  The names used below (VECTREX, ARCADE, COMPUTER, VIDEO, AUDIO) are the ones used in
  the FAQ and in the code comments.
* Pressing **Button 4** on any entry that has an image launches it. This includes the
  category nodes `speedMenu` and `exactMenu`, which launch their emulator with no ROM.

## Top row

`settingsMenu` | `vectrexMenu` | `arcadeMenu` | `computerMenu` | `videoMenu` | `audioMenu`

The settings slot is the far-left item. Button 4 on it opens the calibration GUI
(`v_SettingsGUI`) and then saves `vectrexInterface.ini`.

### Top-level slot: `settingsMenu` (id 0)

| Entry | C symbol | id | Image launched | Parameters passed |
|---|---|---|---|---|
| PLEASE ENTER SETTINGS | `settingsMenu` | 0 | — | — |

### Top-level slot: `vectrexMenu` (id 1)

| Entry | C symbol | id | Image launched | Parameters passed |
|---|---|---|---|---|
| (category icon) | `vectrexMenu` | 1 | — | — |
| &nbsp;&nbsp;VECTORBLADE | `vectorbladeMenu` | 100 | `piZeroN/vectorblade.img` | — |
| &nbsp;&nbsp;(category icon) | `speedMenu` | 102 | `piZeroN/vectrexspeedy.img` | — |
| &nbsp;&nbsp;&nbsp;&nbsp;WEB WARS | `webWarsSpeedMenu` | 1020 | `piZeroN/vectrexspeedy.img` | original/WEBWARS.BIN |
| &nbsp;&nbsp;&nbsp;&nbsp;BERZERK | `berzerkSpeedMenu` | 1021 | `piZeroN/vectrexspeedy.img` | original/BERZERK.BIN |
| &nbsp;&nbsp;&nbsp;&nbsp;MINESTORM 2 | `mine2SpeedMenu` | 1022 | `piZeroN/vectrexspeedy.img` | original/MSTORM2.BIN |
| &nbsp;&nbsp;&nbsp;&nbsp;DARK TOWER | `darkTowerSpeedMenu` | 1023 | `piZeroN/vectrexspeedy.img` | original/DKTOWER.BIN |
| &nbsp;&nbsp;&nbsp;&nbsp;STAR CASTLE | `starCastleSpeedMenu` | 1024 | `piZeroN/vectrexspeedy.img` | original/CASTLE.BIN |
| &nbsp;&nbsp;&nbsp;&nbsp;FORTRESS OF NARZOD | `narzodSpeedMenu` | 1025 | `piZeroN/vectrexspeedy.img` | original/NARZOD.BIN |
| &nbsp;&nbsp;&nbsp;&nbsp;SCRAMBLE | `scrambleSpeedMenu` | 1026 | `piZeroN/vectrexspeedy.img` | original/SCRAMBLE.BIN |
| &nbsp;&nbsp;&nbsp;&nbsp;SOLAR QUEST | `solarSpeedMenu` | 1027 | `piZeroN/vectrexspeedy.img` | original/SOLAR.BIN |
| &nbsp;&nbsp;(category icon) | `exactMenu` | 107 | `piZeroN/vectrexexact.img` | — |
| &nbsp;&nbsp;&nbsp;&nbsp;KARL QUAPPE | `exactkarlQuappeMenu` | 1070 | `piZeroN/vectrexexact.img` | KARL_QUAPPE.BIN |
| &nbsp;&nbsp;&nbsp;&nbsp;RELEASE | `exactMenuRelease` | 1071 | `piZeroN/vectrexexact.img` | RELEASE.BIN |
| &nbsp;&nbsp;&nbsp;&nbsp;MR BOSTON | `exactmrBoston` | 1072 | `piZeroN/vectrexexact.img` | original/MR_BOSTON.BIN |
| &nbsp;&nbsp;&nbsp;&nbsp;PICK A GAME | `exactPickOne` | 9999 | `piZeroN/vectrexexact.img` | — |
| &nbsp;&nbsp;DEMOS | `demoMenu` | 108 | — | — |
| &nbsp;&nbsp;&nbsp;&nbsp;RAIDING PARTY | `demo1Menu` | 1080 | `piZeroN/vectrexexact.img` | demos/RaidingParty.bin |
| &nbsp;&nbsp;&nbsp;&nbsp;BRESENHAM | `demo2Menu` | 1081 | `piZeroN/vectrexexact.img` | demos/bresenham.bin |
| &nbsp;&nbsp;&nbsp;&nbsp;EPIC REALTIME | `demo3Menu` | 1082 | `piZeroN/vectrexexact.img` | demos/epic.bin |
| &nbsp;&nbsp;&nbsp;&nbsp;WHATPG | `demo4Menu` | 1083 | `piZeroN/vectrexexact.img` | demos/whatpg.bin |
| &nbsp;&nbsp;&nbsp;&nbsp;LINEART | `demo5Menu` | 1084 | `piZeroN/vectrexlineart.img` | demos/lineart8banks.bin |

### Top-level slot: `arcadeMenu` (id 2)

| Entry | C symbol | id | Image launched | Parameters passed |
|---|---|---|---|---|
| (category icon) | `arcadeMenu` | 2 | — | — |
| &nbsp;&nbsp;ATARI | `atariMenu` | 200 | — | — |
| &nbsp;&nbsp;&nbsp;&nbsp;Star Wars (SBT) | `swsbtMenu` | 2000 | `piZeroN/swsbt.img` | — |
| &nbsp;&nbsp;&nbsp;&nbsp;ASTEROIDS | `asteroidsMenu` | 2001 | `piZeroN/asteroids_sbt.img` | — |
| &nbsp;&nbsp;&nbsp;&nbsp;BATTLE ZONE | `battleZoneMenu` | 2002 | `piZeroN/battlezone.img` | — |
| &nbsp;&nbsp;&nbsp;&nbsp;BLACK WIDOW | `blackWidowMenu` | 2003 | `piZeroN/blackwidow.img` | — |
| &nbsp;&nbsp;&nbsp;&nbsp;RED BARON | `redBaronMenu` | 2004 | `piZeroN/redbaron.img` | — |
| &nbsp;&nbsp;&nbsp;&nbsp;LUNAR LANDER | `lunarMenu` | 2005 | `piZeroN/lunar.img` | — |
| &nbsp;&nbsp;&nbsp;&nbsp;GRAVITAR | `gravitarMenu` | 2006 | `piZeroN/gravitar.img` | — |
| &nbsp;&nbsp;&nbsp;&nbsp;SPACE DUEL | `spaceDuelMenu` | 2007 | `piZeroN/spaceduel.img` | — |
| &nbsp;&nbsp;&nbsp;&nbsp;TEMPEST | `tempestMenu` | 2008 | `piZeroN/tempest.img` | — |
| &nbsp;&nbsp;&nbsp;&nbsp;MAJOR HAVOC | `majorHavocMenu` | 2009 | `piZeroN/aae.img` | game #45 (0x2d) |
| &nbsp;&nbsp;&nbsp;&nbsp;ASTEROIDS DELUXE | `asteroidsDeluxeMenu` | 2010 | `piZeroN/aae.img` | game #9 (0x09) |
| &nbsp;&nbsp;&nbsp;&nbsp;QUANTUM | `quantumMenu` | 2011 | `piZeroN/aae.img` | game #54 (0x36) |
| &nbsp;&nbsp;CINEMATRONICS | `allCinematronicsMenu` | 201 | — | — |
| &nbsp;&nbsp;&nbsp;&nbsp;TAILGUNNER | `tailgunnerMenu` | 3000 | `piZeroN/tailgunner.img` | — |
| &nbsp;&nbsp;&nbsp;&nbsp;SPEEDFREAK | `speedfreakMenu` | 3001 | `piZeroN/aae.img` | game #66 (0x42) |
| &nbsp;&nbsp;&nbsp;&nbsp;ARMOR ATTACK | `armorAttackMenu` | 3002 | `piZeroN/aae.img` | game #58 (0x3a) |
| &nbsp;&nbsp;&nbsp;&nbsp;BOXING BUGS | `boxingbugsMenu` | 3003 | `piZeroN/aae.img` | game #68 (0x44) |
| &nbsp;&nbsp;&nbsp;&nbsp;DEMON | `demonMenu` | 3004 | `piZeroN/cine.img` | demon |
| &nbsp;&nbsp;&nbsp;&nbsp;RIPOFF | `ripoffMenu` | 3005 | `piZeroN/aae.img` | game #62 (0x3e) |
| &nbsp;&nbsp;&nbsp;&nbsp;SPACE WARS | `spaceWarsMenu` | 3006 | `piZeroN/cine.img` | spacewars |
| &nbsp;&nbsp;&nbsp;&nbsp;STAR CASTLE | `starcastleMenu` | 3007 | `piZeroN/aae.img` | game #57 (0x39) |
| &nbsp;&nbsp;&nbsp;&nbsp;SUNDANCE | `sundanceMenu` | 3008 | `piZeroN/aae.img` | game #60 (0x3c) |
| &nbsp;&nbsp;&nbsp;&nbsp;WAR OF THE WORLDS | `waroftheworldsMenu` | 3009 | `piZeroN/aae.img` | game #69 (0x45) |
| &nbsp;&nbsp;&nbsp;&nbsp;BARRIER | `barrierMenu` | 3010 | `piZeroN/cine.img` | barrier |
| &nbsp;&nbsp;&nbsp;&nbsp;SOLAR QUEST | `solarquestMenu` | 3013 | `piZeroN/aae.img` | game #56 (0x38) |
| &nbsp;&nbsp;&nbsp;&nbsp;STAR HAWK | `starhawkMenu` | 3014 | `piZeroN/aae.img` | game #64 (0x40) |
| &nbsp;&nbsp;&nbsp;&nbsp;WARRIOR | `warriorMenu` | 3015 | `piZeroN/aae.img` | game #61 (0x3d) |
| &nbsp;&nbsp;SEGA | `segaMenu` | 202 | — | — |
| &nbsp;&nbsp;&nbsp;&nbsp;Zektor | `zektorMenu` | 4000 | `piZeroN/aae.img` | game #34 (0x22) |
| &nbsp;&nbsp;&nbsp;&nbsp;TACSCAN | `tacscanMenu` | 4001 | `piZeroN/aae.img` | game #35 (0x23) |
| &nbsp;&nbsp;&nbsp;&nbsp;STARTREK | `starTrekMenu` | 4002 | `piZeroN/aae.img` | game #36 (0x24) |
| &nbsp;&nbsp;&nbsp;&nbsp;SPACFURY | `spaceFuryMenu` | 4003 | `piZeroN/aae.img` | game #37 (0x25) |
| &nbsp;&nbsp;&nbsp;&nbsp;ELEMINATOR | `eleminatorMenu` | 4004 | `piZeroN/aae.img` | game #40 (0x28) |
| &nbsp;&nbsp;OTHER ARCADE | `otherArcadeMenu` | 203 | — | — |
| &nbsp;&nbsp;&nbsp;&nbsp;OMEGA RACE | `omegaMenu` | 5000 | `piZeroN/aae.img` | game #10 (0x0a) |

### Top-level slot: `computerMenu` (id 3)

| Entry | C symbol | id | Image launched | Parameters passed |
|---|---|---|---|---|
| (category icon) | `computerMenu` | 3 | — | — |
| &nbsp;&nbsp;ZBLAST | `zBlastMenu` | 300 | `piZeroN/zblast.img` | — |
| &nbsp;&nbsp;HYPEROIDS | `hyperoidsMenu` | 301 | `piZeroN/vhyperoid.img` | — |
| &nbsp;&nbsp;GS BASIC | `basicMenu` | 302 | `piZeroN/gsbasic.img` | — |
| &nbsp;&nbsp;ELITE (BETA) | `newKindMenu` | 303 | `piZeroN/newkind.img` | — |
| &nbsp;&nbsp;WW VI (BETA) | `wwviMenu` | 304 | `piZeroN/ww_vi.img` | — |
| &nbsp;&nbsp;BZ2 (BETA) | `bz2Menu` | 305 | `piZeroN/bz2.img` | — |
| &nbsp;&nbsp;VOOM (DEMO) | `voomMenu` | 306 | `piZeroN/voom.img` | — |
| &nbsp;&nbsp;REBEL DEFENDER | `rebelMenu` | 308 | `piZeroN/rebel.img` | — |
| &nbsp;&nbsp;VTERM | `testMenu` | 309 | `piZeroN/vterm.img` | — |

### Top-level slot: `videoMenu` (id 4)

| Entry | C symbol | id | Image launched | Parameters passed |
|---|---|---|---|---|
| (category icon) | `videoMenu` | 4 | — | — |
| &nbsp;&nbsp;BAD APPLE | `badAppleMenu` | 300 | `piZeroN/vectrexvideo.img` | vectrex/movies, BadApple.bin |
| &nbsp;&nbsp;BLA BLA | `blablaMenu` | 301 | `piZeroN/vectrexvideo.img` | vectrex/movies, BlaBla.bin |
| &nbsp;&nbsp;LA LINEA | `laLineaMenu` | 302 | `piZeroN/vectrexvideo.img` | vectrex/movies, LaLinea.bin |
| &nbsp;&nbsp;BAD APPLE DE | `badAppleDeMenu` | 303 | `piZeroN/vectrexvideo.img` | vectrex/movies, BadAppleDE.bin |

### Top-level slot: `audioMenu` (id 5)

| Entry | C symbol | id | Image launched | Parameters passed |
|---|---|---|---|---|
| (category icon) | `audioMenu` | 5 | — | — |
| &nbsp;&nbsp;QUEEN | `bohemianMenu` | 400 | `piZeroN/vectrexaudio.img` | vectrex/waves, Bohemian.bin |
| &nbsp;&nbsp;PRINCE | `purpleMenu` | 401 | `piZeroN/vectrexaudio.img` | vectrex/waves, PurpleRain.bin |

### Defined but not reachable from the top row

| Entry | C symbol | id | Image launched | Parameters passed |
|---|---|---|---|---|
| COSMIC CHASM | `cosmicchasmMenu` | 3011 | `piZeroN/cine.img` | cosmicchasm |
| QB3 | `qb3Menu` | 3012 | `piZeroN/cine.img` | qb3 |
| TAILGUNNER | `tailgunner2Menu` | 2000 | `piZeroN/tailgunner.img` | — |
| AAE | `aaeMenu` | 6000 | `piZeroN/aae.img` | — |
