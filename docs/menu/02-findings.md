# Findings

Facts about how the loader and its launched programs behave, with the source that
shows it. **Verified** = read in the source at upstream `457c5a9`. **Unverified** =
inferred, or needs a test on real hardware (there is no PiTrex emulator, see F14).

## Loader

**F1. The menu is compiled in. Verified.** `pitrex/loader/loaderMain.c` defines every
entry as a static `MenuItemNew`. The only runtime config is `loader.ini`, which accepts
just `SKIP_WAV` and `USE_SMP` (`loaderIniHandler`, ~line 323). The upstream FAQ (#13)
confirms: changing the game selection or adding favorites needs a loader recompile.

**F2. An entry launches an image and passes it two strings. Verified.**
`loadAndStart()` (~line 156) reads `piZeroN/<img>` into memory at `0x8000`, copies
`param1` and `param2` (max 126 chars each) into the shared `loaderSettings`, records
which button started it in `parameter3[0]`, and jumps to it. Programs read the strings
with `getLoadParameter(n)`. Any new launch mechanism must keep this contract so
existing programs keep working unchanged.

**F3. Navigation. Verified.** Stick left/right moves between siblings, down enters a
child, up returns to the parent. Button 4 (exactly, no other button) starts the entry
or, on the settings slot, opens the calibration GUI. Button 1 (exactly) starts an entry
too and, per FAQ #34, makes the Vectrex emulators load the last saved state.

**F4. Category nodes can launch too. Verified (code), behaviour Unverified.** Button 4
launches whenever `img != 0`. `speedMenu` and `exactMenu` have an image and no ROM
parameter, so pressing Button 4 on them starts the emulator with no ROM. The changelog
says that starting the exact emulator without a cart shows a small ROM-selection menu
inside the emulated Vectrex.

**F5. A dynamic list already exists: PICK A GAME. Verified.** `loadFileNames()` /
`isNameOk()` (~line 2668 to 2790) list a directory under `vectrex/`. Accepted: files
ending in `.bin` (case-insensitive) and any directory, except names starting with `.`
and the reserved directories `bios`, `movies`, `waves`. Limits: 1000 files and 100
directories per folder (`MAX_V_FILES`, `MAX_V_DIRS`). Selecting a file launches
`vectrexexact.img` with `param1` = sub-path + file name (`loadAndStart`, id 9999).
This is the only runtime discovery in the loader. It covers `.bin` ROMs only.

**F6. The card has no registry of programs. Verified.** `piZeroN/` holds both programs
(`zblast.img`, `vectorblade.img`...) and system images (`loader.pit`, `vectrexexact.img`,
`aae.img`...). Nothing marks which is which, so a discovery scheme needs either a list
of known system images or a naming/sidecar convention (see the plan).

## Emulators

**F7. Speedy and Exact are two builds of the same emulator. Verified.** `vecx.speedy`
and `vecx.exact` compile the same sources (`vectrexBase/`) with different switches in
`vectrexConfig.h`:

| | Speedy (`vectrexspeedy.img`) | Exact (`vectrexexact.img`) |
|---|---|---|
| Distinct switches | `USE_VIA_QUICK_ACCESS`, `EMULATE_KNOWN_PRINTSTR`, `WAIT_EMULATION` | `EXACT_WAIT_EMULATION`, `ALLOW_ROM_WRITE`, `WAIT_EMULATION` |
| Behaviour (FAQ #7, #8) | Games run about 30-50% faster than a real Vectrex and output at 50 Hz | As exact as the author knows how; most games should behave as on a real Vectrex. Known exception: 3D Imager games (VIA interrupt not supported) |
| Default Pi target | zero1 | zero2 (both build for either) |

**F8. Both accept any `.bin`. Verified.** Both call `loadVectrexBin(param1)`, which opens
`vectrex/` + `param1` (`vectrexBase/main.c:700`). The emulator is not tied to a particular
game. The compiled-in Speedy list (8 games under `vectrex/original/`) is only the loader
menu. Exact also has a "forced ROM" path (`bootUpName`) that can load a file from
anywhere.

**F9. Speedy has no picker. Verified.** Only `exactPickOne` (id 9999) browses the card.
Speedy's menu is exactly 8 fixed entries (see file 01).

**F10. Which emulator should a new `.bin` use? Mixed.**
Verified: either can run it. Reasoning (**Unverified, needs per-game testing**): Exact is
the safe default because it is the faithful one. Speedy is an opt-in for games that
benefit from the speed-up and are known to tolerate it. Speedy changes timing and
shortcuts known BIOS print routines, so timing-sensitive or BIOS-quirky games may
misbehave. The 8 Speedy presets are the only games the author lists for it.

## Arcade emulators

**F11. `aae.img` takes a single byte, not a name. Verified.** `aae/aaemain.c:1369`:
`gamenum = getLoadParameter(0)[0]`. The menu encodes it as `"\x2d"` and so on. A new
arcade game is therefore not just a ROM zip on the card. It needs support in the AAE
code and a number. The ROM guide's folder rules apply (`roms/` vs `roms/aae/`).
`cine.img` instead takes a MAME-style name (`demon`, `spacewars`, `barrier`, `qb3`,
`cosmicchasm`).

**F12. Some defined entries are unreachable. Verified.** `cosmicchasmMenu` (3011),
`qb3Menu` (3012), `tailgunner2Menu` (2000), `aaeMenu` (6000) exist in the source but
are not linked into the menu (see the end of file 01). The `id == 6000` special case in
`loadAndStart` suggests an AAE game selector that is not wired up. **Unverified**
whether it works.

## Native programs

**F13. Vectorburner fits the existing COMPUTER category. Verified (package), integration
Unverified.** It is a bare-metal PiTrex program built against this SDK, like ZBlast and
Hyperoids, with no parameters (`piZero1/vectorburner.img` and `piZero2/vectorburner.img`
in the package). Today's menu has no entry for it, and the package's own notes say it
launches by editing `config.txt` (`kernel=piZero1/vectorburner.img`), not from the menu.
Unknown: whether it returns to the loader on exit.

## Process

**F14. There is no PiTrex emulator. Verified for this repo.** Nothing in it builds the
loader for a PC. QEMU's Pi Zero machine could run the ARM code but has no VIA or
Vectrex behind the GPIO pins, so there would be no vector output.
Consequences: logic that does not touch hardware (name parsing, classification, menu
data, list handling) can be written as plain C and unit-tested on a PC. Anything that
draws or launches must be tried on the PiTrex.

**F15. Toolchain not yet checked. Unverified.** The build needs `arm-none-eabi-gcc`
(`HowToCompile.txt`). Whether it is installed here, and whether the unmodified loader
builds, is the first thing to establish.

## Open questions

1. Which `.bin` games, if any, should default to Speedy? Per-game testing needed.
2. Should discovery also cover arcade ROMs (needs a name-to-game-number table in code)?
3. Do bare-metal `.img` programs return to the loader on exit, and how?
4. How should a program supply its display name and category: file name, sidecar file, or
   a header in the image?
5. Keep the icon-based category slots, or drop icons for a plain text list?
