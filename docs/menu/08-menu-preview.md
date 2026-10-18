# Desktop preview of the new menu

`docs/tools/menu_preview/` compiles the real `newMenuUi.h` and `newMenuLogic.h` against Roger
Boesch's PiTrex simulator (MIT licence, a small SDL2 beam model from the Gong Rider source
package, <https://rogerboesch.games/learn/gong-rider/>). The simulator is downloaded into
`~/.cache/pitrex-preview` when needed and is not stored in this repository.

Needs the `pitrex-dev` distrobox with `libsdl2-dev g++ imagemagick` (see `docs/tools/build.sh`).

```
docs/tools/menu_preview/build_preview.sh                       # builds the preview
docs/tools/menu_preview/make_card.sh <real card> <out> <real card>/menu   # placeholder card with the real file names
docs/tools/menu_preview/shot.sh <out> <png dir> <name> <frames> "down@10,right@20"   # screenshot
docs/tools/menu_preview/scenarios.sh <out>                     # scripted checks, prints ok/FAIL
```

Input scripts are `name@frame` joined by commas (`up down left right b1 b2 b4`); each input is held
for 4 frames. `PITREX_FRAMES` ends the run, `PITREX_SHOT` saves the last frame.

What it proves: which program, parameters and button the loader would be asked to start; the list
layout and navigation; sidecar effects; SETTINGS saving to `vectrexInterface.ini`.

What it does **not** show: the real PiTrex font (the simulator draws a different one, so use
the preview for layout only, never for which characters are drawable: see file 07), real timing,
the SD card code, the old PICK A GAME browser (stubbed), or anything about the emulators.
