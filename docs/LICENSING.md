# Licensing notes

Not legal advice. Findings from reading the repository on 2026-10-17 (upstream `457c5a9`).

## The upstream project

* **No license for the author's own code.** There is no top-level `LICENSE` or `COPYING`,
  GitHub reports no license for `malbanGit/pitrex-baremetal`, and the author's own sources
  (`pitrex/loader/loaderMain.c`, `pitrex/vectrex/vectrexInterface.c`, `aae/aaemain.c`,
  `vectrexBase/`) carry no license header. Without a license, copyright is reserved by default.
* What GitHub's Terms of Service do allow for a public repository: others may view and fork it
  on GitHub. That is not a licence to modify and redistribute elsewhere.
* The README points to the author's pages (`vide.malban.de/pitrex`) for more information;
  those pages were not checked for licence terms.

## Bundled components that do have licences

| Path | Licence seen |
|---|---|
| `gyrocks/`, `xhyperoid/`, `zblast/` | GPL v2 |
| `ww_vi/` | GPL (sounds under their own terms, `ww_vi/sounds/Attribution.txt`) |
| `new.not_done/xinv3d/` | GPL |
| `gsBasic/` | MIT (Robert E. Alexander). `changes.txt` says "see MIT_licence.txt" |
| `rebel/` | BSD-style (V. Portet) |
| `pitrex/baremetal/ff.c` | FatFs, ChaN, BSD-style |
| `pitrex/vectrex/uspi/` | USPi, GNU GPL (`LICENSE` present) |
| `aae/musashi/` | Musashi 68k core, its own licence in `readme.txt` |
| `starwars/translate/asm6809-2.12/` | GPL |

`aae/` is a port of "AAE (Another Arcade Emulator)" whose source was released by its
author on the arcadecontrols forum. The repository does not say under what terms; that was
not looked up.

## Consequences for this fork

* Private use and local experiments are one thing; **publishing** changed code, or
  redistributing builds, is another. The safe route is to ask the author for permission or
  for a licence to be added (the commit email is on the repository's commits).
* Docs and tools written in this fork (`docs/`) are the fork owner's own work.
* Do not publish SD-card images, built binaries, or arcade ROM sets. ROMs are
  third-party copyrighted works regardless of the emulator's licence.
* Components under the GPL carry obligations when redistributed (source, same licence).
  Check each before shipping anything built from them.
