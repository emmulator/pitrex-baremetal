# Hardware test log (2026-10-17)

Pi Zero W (Zero-1 builds) in a PiTrex V.1 cartridge, new 58.6 GB SD card.
Menu builds are the `candidate-N` sets in the `pitrex-builds` folder next to the repository (not tracked).

| Test | Result |
|---|---|
| Original 3.94 GB card in the Vectrex | Vectrex did not power on. Cause was the card (a new card fixed it), not the cart. |
| New card, July 2025 images, original menu | Boots. Screen needed calibration (SETTINGS = stick left at the main menu). |
| New menu (candidate-7) | List shown. Text too small, settings did not save, leaving SETTINGS froze (F21, F22), games blank (F19). |
| Larger text, help lines, button hints | OK. |
| Settings fix (candidate-9) | Settings save. |
| Menu opens in VECTREX | Button 4 held at power-on (F23). Fixed by waiting for the controls to rest. |
| 2021 emulators | Games draw; Exact text is dots, Speedy is wobbly with missing vectors. |
| 2023 emulators | Same as 2021. |
| Own build of the 2025 source (debug-logging build) | Armor Attack and Berzerk run. Text readable but shimmers. |

Open: the text shimmer, the Zero 2 builds, the clean (non-logging) Speedy build, the other
Vectrex-based emulators (lineart, video, audio, vectorblade) which were not rebuilt.

The log helpers: `NMLOG.TXT` (menu, written by `nmLog()` in `newMenuUi.h`) and `EMUDBG.TXT` (emulator,
`new_menu/tools/emulator-debug-breadcrumbs.patch`).
