# Other work on the PiTrex menu (searched 2026-10-18)

Searched the web (Tavily) for forks and posts about changing the PiTrex baremetal menu.

| Who | What | Relation to this fork |
|---|---|---|
| `malbanGit/pitrex-baremetal` | Upstream. GitHub lists **no forks** of it. | Base of this fork. |
| `mrchrisster/pitrex` (6 stars, 80 commits) | Menu customisation based on Malban's June 2021 release: new logos, random Vectrex game, demo picker, Star Wars (AAE) menu, new layout. Edits the compiled-in `loaderMain.c` and `icons.i`. Omits the music/video player and speedy emulator. | Same idea (a nicer menu) but static: entries are still compiled in. Nothing dynamic. |
| `gtoal/pitrex` (41 stars) | Linux-side work (VMMenu, MAME driving), not the baremetal loader. | Unrelated. |
| Roger Boesch, [Native ARM on the Vectrex](https://vectrex.substack.com/p/native-arm-on-the-vectrex) | Compares PiTrex and UVMC2. Calls the hard-coded loader menu the PiTrex's main usability gap and lists planned work: a NATIVE GAMES entry that reads folders from the card, one package for Zero and Zero 2, titles/authors/descriptions from companion text files, per-game folders, ROM pre-flight checks. Says he is "working on something" and to "stay tuned". No code published as far as the search showed. | **Overlaps strongly** with this fork (companion text files, missing-ROM check, dynamic lists). Worth contacting before releasing, to avoid two incompatible formats. He also wrote the PiTrex simulator the preview tool uses (MIT). |

Not found: any published fork or patch with a dynamic (card-driven) loader menu.
