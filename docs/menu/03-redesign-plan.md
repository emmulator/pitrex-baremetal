# Redesign plan

Status: **draft, nothing implemented.** Decisions marked *(proposed)* are open for the
owner to change. Facts referenced as F1..F15 are in [02-findings.md](02-findings.md).

## Goals

1. **Full compatibility.** Every existing program (emulators, arcade, native, video,
   audio) must launch exactly as it does today, with the same image and parameters (F2).
2. **Dynamic discovery.** New titles are added by copying files to the card. No compile.
3. **Simple hierarchical menu.** All top-level categories visible at once. Selecting a
   category drills down to its entries.

## Constraints

* Upstream programs read `loaderSettings` unchanged, so the launch path
  (`loadAndStart`, F2) stays as it is. Only the menu around it changes.
* The loader runs bare-metal on a Pi Zero. Scanning must be fast at boot and bounded in
  memory (the existing lists cap at 1000 files / 100 dirs, F5).
* No PiTrex emulator (F14). Pure logic gets host-side unit tests. Everything else is
  tested on hardware, one change at a time.
* Stay mergeable: keep changes inside `pitrex/loader/` where possible, and keep the
  fork buildable for both `zero1` and `zero2`.

## Proposed design

### Entry sources *(proposed)*

| Source | Found by | Category | Launched as |
|---|---|---|---|
| Built-in catalog | Compiled table (the entries in file 01) | As today | Same image + params as today |
| Vectrex ROMs | `vectrex/**/*.bin` (already scanned, F5) | VECTREX, folders become sub-categories | Exact by default, Speedy by opt-in |
| Native programs | `piZeroN/*.img` not in the built-in or system lists | PROGRAMS (or a sidecar-chosen category) | No parameters |
| Overrides | Optional sidecar file per entry | Any | Any |

* **Built-in entries are shown only if their image exists on the card**, so a card
  with fewer programs gives a shorter menu instead of dead entries. *(proposed)*
* **System images are hidden**: `loader.pit`, `pitrex.img`, `pitrex7.img`,
  `vectrexexact.img`, `vectrexspeedy.img`, `vectrexlineart.img`, `vectrexaudio.img`,
  `vectrexvideo.img`, `aae.img`, `cine.img`, and any other image that is only an
  emulator. The list is a table in code. A native program with a new name is discovered
  automatically. *(proposed)*
* **Sidecar convention** *(proposed, open question 4)*: `menu/<name>.ini` next to or
  beside the image with `name=`, `category=`, `emulator=speedy|exact`, `param1=`,
  `param2=`. Without it, the name is the file name and the category is the default.
* **Arcade ROM zips are out of scope for the first version** (F11): a game number lives
  in the emulator code, so adding an arcade title still needs code. The built-in catalog
  keeps the existing arcade entries.

### Speedy vs Exact for `.bin` files *(proposed)*

Default **Exact** (F10). A per-file or per-folder opt-in selects **Speedy**, either via
the sidecar `emulator=` key or a folder convention such as `vectrex/speedy/`. A
button-chosen emulator at launch time could be added later. Which games suit Speedy
needs to be tested on the PiTrex, so the first version ships with the 8 existing
presets marked as Speedy and everything else as Exact.

### Menu behaviour *(proposed)*

* **Level 1:** a plain scrolling list of categories, all visible: VECTREX, ARCADE,
  PROGRAMS, VIDEO, AUDIO, SETTINGS. Stick up/down moves, Button 4 enters.
* **Level 2 and below:** the entries of the category, with sub-categories first.
  Stick left (or Button 2) goes back. Button 4 launches. Button 1 launches and loads the
  saved state, as today (F3).
* Text only, using the existing vector font. Icons and scroll text are optional extras.
* **Keep the author's caveats (F18).** Today the entry screen shows its note lines ("NO SOUND",
  "WORK IN PROGRESS, FLICKER") and "ROM NOT FOUND!". The redesign must keep both: the
  built-in catalog carries its existing notes forward unchanged, discovered titles take
  their notes from the sidecar `note=` lines, and the list shows a short tag (for example
  `[NO SOUND]`) so the caveat is visible before opening the entry.

## Phases

| # | Phase | Result | Test |
|---|---|---|---|
| 0 | Toolchain and baseline (**toolchain done 2026-10-17**; hardware boot of our own build still pending) | Pinned `arm-none-eabi-gcc` in a distrobox (F15); local build overrides instead of editing `Rules.mk` (F17); unmodified loader builds for zero1 and zero2 | Build; boot the unmodified loader on the PiTrex. Optional: UART cable for `printf` and serial upload (F17) |
| 1 | Prove the pipeline | One new compiled entry: Vectorburner under COMPUTER (F13) | Launch from the menu; check exit behaviour |
| 2a | Notes as sidecar files (**done 2026-10-17**, hardware test pending; see file 05) | The loader reads `menu/<key>.ini` for each entry's note lines; the compiled notes remain as fallback | Host test passes; try on the PiTrex |
| 2b | AAE takes names and lists ROM zips (**done 2026-10-17**, hardware test pending; see file 06) | New `ALL AAE GAMES` entry; AAE picks from `roms/aae/*.zip` | Host test passes; try on the PiTrex |
| 3+4 | Discovery of programs and a new list menu, **off by default** (**done 2026-10-17**, hardware test pending; see file 07) | `NEW_MENU = 1` in `loader.ini` | Host test passes; try on the PiTrex |
| 2 | Data-driven menu | Replace the linked structs with a table. Same entries, same behaviour | Every entry launches as before, spot-checked on hardware |
| 3 | Discovery | Scan `vectrex/` and `piZeroN/`, merge with the catalog, apply sidecars | Host unit tests for parsing and classification, then hardware |
| 4 | New UI | Hierarchical text menu | Hardware; compare against phase 2 behaviour |
| 5 | Polish | Docs, sidecar examples, SD layout notes | Fresh card from scratch |

Phases 1 and 2 are independently useful. If the redesign stalls, phase 1 alone fixes
the missing Vectorburner entry.

**Where things stand (2026-10-17):** the redesigned menu exists as a first version behind a switch. What exists is the
existing menu plus a VECTORBURNER entry (phase 1) and sidecar notes (phase 2a). Phases 2, 3 and
4 are still to do.

## Risks

* Large menus: memory and scan time at boot on a Pi Zero. Mitigate with the existing
  caps and by measuring.
* Image classification is a guess without metadata. The sidecar and an explicit
  system-image list reduce it.
* Hardware-only testing slows iteration. Keep the logic host-testable.
* Divergence from upstream. Keep commits small and the layout of `pitrex/loader/` intact.

## Working rules for this fork

* Branch `menu-redesign`. Commits use the repo-local identity
  `emmulator <2083447+emmulator@users.noreply.github.com>`.
* Do not push without the owner's say-so.

## Arcade launcher (proposed, added 2026-10-17)

Goal: a new ROM zip for a game the emulators already support appears in the menu with
no recompile of the loader.

**The obstacle (F11).** `aae.img` is told which game to run by one byte, an index into a
table compiled into the emulator (`aae/aaemain.c:1369`, `driver[gamenum]`). The loader
cannot turn a zip name into that byte unless it carries a copy of the table.

**Option A: teach the emulator to take a name, and give it a picker *(preferred)*.**
1. `aae.img` accepts a game **name**. A one-character parameter stays a numeric index
   (existing menu entries keep working), a longer one is looked up in `driver[]` by name.
2. With no parameter, `aae.img` shows its own picker: it scans `roms/aae/` and `roms/` for
   `.zip` files whose names match a driver, and lists them with the driver's descriptive
   name. This is the same idea as the Exact emulator's PICK A GAME.
3. The loader gets one `ARCADE` entry that launches `aae.img` with no parameter.
* Pro: the driver table stays in one place. A game the emulator learns later shows up on
  its own. A new zip for an existing driver needs no rebuild at all.
* Con: one change to the AAE emulator, and `aae.img` must be rebuilt once. A game the
  emulator has no driver for still needs emulator work (Aztarac, Empire Strikes Back).
* `cine.img` already takes names, so it only needs the same picker.
* Needs a guard: today an out-of-range index is not checked.

**Option B: a manifest on the card *(complements A)*.** The loader reads an optional
`menu/arcade.ini` of `title | image | param` lines and adds those entries. This covers any
emulator that already takes a parameter (`cine.img qb3`, a future emulator) and lets the
owner pick display names, with no rebuild.

**Not covered by either:** whether a zip is a complete romset. Split sets need their parent
zip too (the ROM guide's Asteroids example). A missing file only shows up as an on-screen
ROM load error after launch. A picker could mark known parent/clone pairs.

**Order of work:** Vectorburner as a plain entry first (phase 1), then Option A, then Option B
only if still wanted. (Cosmic Chasm and QB3 are not candidates: the author marks them as
not working, see file 04.)
