# Other work on the PiTrex menu (searched 2026-10-18)

Searched the web (Tavily) for forks and posts about changing the PiTrex baremetal menu.

| Who | What | Relation to this fork |
|---|---|---|
| `malbanGit/pitrex-baremetal` | Upstream. GitHub lists **no forks** of it. | Base of this fork. |
| `mrchrisster/pitrex` (6 stars, 80 commits) | Menu customisation based on Malban's June 2021 release: new logos, random Vectrex game, demo picker, Star Wars (AAE) menu, new layout. Edits the compiled-in `loaderMain.c` and `icons.i`. Omits the music/video player and speedy emulator. | Same idea (a nicer menu) but static: entries are still compiled in. Nothing dynamic. |
| `gtoal/pitrex` (41 stars) | Linux-side work (VMMenu, MAME driving), not the baremetal loader. | Unrelated. |
| Roger Boesch, [Native ARM on the Vectrex](https://vectrex.substack.com/p/native-arm-on-the-vectrex) | Compares PiTrex and UVMC2. Calls the hard-coded loader menu the PiTrex's main usability gap and lists planned work: a NATIVE GAMES entry that reads folders from the card, one package for Zero and Zero 2, titles/authors/descriptions from companion text files, per-game folders, ROM pre-flight checks. Says he is "working on something" and to "stay tuned". No code published as far as the search showed. | **Overlaps strongly** with this fork (companion text files, missing-ROM check, dynamic lists). Worth contacting before releasing, to avoid two incompatible formats. He also wrote the PiTrex simulator the preview tool uses (MIT). |

Not found: any published fork or patch with a dynamic (card-driven) loader menu.

## pitrex-dev group (searched 2026-10-18)

All 130 topics of the public archive were read for menu-related talk. Findings:

* March 2026, "Adding a game to PiTrex baremetal": Brandon Yates, who wrote the Tensor Valley game, asks
  for a way to add a game to the menu without forking, ideally by scanning a folder for `.img` files
  with an optional icon. No answer. (Tensor Valley itself is a native PiTrex game; its audio and text
  issues are discussed in the newest topics.)
* March 2026, "Pitrex game update": Kevin Koster says the Linux PiTrexCore already adds games dropped
  on the card to its menu, that doing the same for bare-metal "wouldn't be hard", but suggests
  developers keep adding hard-coded entries to the base ZIP. Roger Boesch proposes a download store.
* Nobody in the archive reports a dynamic baremetal menu. The new menu is the answer to the question
  Brandon asked.
