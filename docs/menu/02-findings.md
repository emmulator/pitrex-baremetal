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
are not linked into the menu (see the end of file 01). The author's notes explain the two
Cinematronics ones: Cosmic Chasm "NOT WORKING NEEDS 68000", QB3 "CRASHING". They were left out
because they do not work. The `id == 6000` special case in
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

**F14. There is no PiTrex emulator. Verified for this repo; see also F16.** Nothing in it
builds the loader for a PC. QEMU's Pi Zero machine could run the ARM code but has no VIA or
Vectrex behind the GPIO pins, so there would be no vector output.
Consequences: logic that does not touch hardware (name parsing, classification, menu
data, list handling) can be written as plain C and unit-tested on a PC. Anything that
draws or launches must be tried on the PiTrex.

**F15. ARM toolchain: set up and working. Verified (2026-10-17).**
`distrobox` container `pitrex-dev` (Ubuntu 22.04) with Ubuntu's `gcc-arm-none-eabi`
10.3.1 (2021.07) and newlib. The unmodified upstream sources build cleanly with it:
loader for zero1 (`pitrex.img` 196,700 bytes, `loader.pit` 689,356) and zero2
(`loader.pit` 778,196, `pitrex7.img` 182,404), and `aae.img` for zero1 (1,484,564).
These are within about 1% of the images on the card (built from a slightly later source
state with other toolchain flags) but **not byte-identical**, so the first test of our own
build on hardware is still needed. The author's warning about the newest Arm toolchain
(`_kill` linker error) did not apply to this older release. `make` needs
`MAKE_IMAGE= SERIAL_TRANSFER=` on the command line, since `Rules.mk` otherwise tries to
copy to the author's folders and use his serial port. The repo tracks some build
artifacts (59 `.img`/`.a`/`.o` files), so `docs/tools/build.sh` builds in a scratch copy.

**F16. The "development image" in the README is a VM, not an emulator. Verified.**
The README and the quick-start page (`vide.malban.de/pitrex/pitrex-baremetal-quick-start-unfinished`,
plain HTTP; the site's HTTPS fails here) point to `PiTrexBaremetalDev_Ubuntu_16.04.07.ova`,
a VirtualBox appliance (user and password `ubuntu`) with Ubuntu 16.04, the GNU Arm
cross-compiler and the sources. It builds images. It does not run them, and the page
mentions no simulator. This repo's `sim/` directory is a 6502 arcade simulator and
translator for the Atari ports, not a PiTrex or loader emulator. Not downloaded or
inspected.

**F17. The author's dev loop is serial upload plus UART printf. Verified (docs and
Makefile), Unverified on this hardware.** Instead of swapping the SD card, `make` can
push an image over a USB-serial cable to the running PiTrex loader with
`PiTrexSerial/piserial` (`SERIAL_TRANSFER`, `Rules.mk`). Wiring is GND, TX and RX to the
Pi's UART. The same cable shows every `printf` in a terminal. Your card already ships
`piserial.lin` / `piserial.win` and `vterm.ini` (VTerm, the PC-to-PiTrex serial link).
`Rules.mk` is configured for the author's machine (`MAKE_IMAGE = yes`, a hard-coded FTDI
serial path, copy targets under `/media/sf_*`). `HowToCompile.txt` says to comment out
`MAKE_IMAGE` or fix the paths or the build breaks. A fork should move these to local
overrides.

## Open questions

1. Which `.bin` games, if any, should default to Speedy? Per-game testing needed.
2. Should discovery also cover arcade ROMs (needs a name-to-game-number table in code)?
3. Do bare-metal `.img` programs return to the loader on exit, and how?
4. How should a program supply its display name and category: file name, sidecar file, or
   a header in the image?
5. Keep the icon-based category slots, or drop icons for a plain text list?
