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

## Phases

| # | Phase | Result | Test |
|---|---|---|---|
| 0 | Toolchain and baseline | Pinned `arm-none-eabi-gcc` in a distrobox (F15); local build overrides instead of editing `Rules.mk` (F17); unmodified loader builds for zero1 and zero2 | Build; boot the unmodified loader on the PiTrex. Optional: UART cable for `printf` and serial upload (F17) |
| 1 | Prove the pipeline | One new compiled entry: Vectorburner under COMPUTER (F13) | Launch from the menu; check exit behaviour |
| 2 | Data-driven menu | Replace the linked structs with a table. Same entries, same behaviour | Every entry launches as before, spot-checked on hardware |
| 3 | Discovery | Scan `vectrex/` and `piZeroN/`, merge with the catalog, apply sidecars | Host unit tests for parsing and classification, then hardware |
| 4 | New UI | Hierarchical text menu | Hardware; compare against phase 2 behaviour |
| 5 | Polish | Docs, sidecar examples, SD layout notes | Fresh card from scratch |

Phases 1 and 2 are independently useful. If the redesign stalls, phase 1 alone fixes
the missing Vectorburner entry.

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
