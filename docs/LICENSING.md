# Licensing notes

Not legal advice. Findings from reading the repository on 2026-10-17 (upstream `457c5a9`).

## The upstream project

* **No license for the author's own code.** There is no top-level `LICENSE` or `COPYING`,
  GitHub reports no license for `malbanGit/pitrex-baremetal`, and the author's own sources
  (`pitrex/loader/loaderMain.c`, `pitrex/vectrex/vectrexInterface.c`, `aae/aaemain.c`,
  `vectrexBase/`) carry no license header. Without a license, copyright is reserved by default.
* What GitHub's Terms of Service do allow for a public repository: others may view and fork it
  on GitHub. That is not a licence to modify and redistribute elsewhere.
* The README points to the author's pages (`vide.malban.de/pitrex`); those were read on 2026-10-18,
  see the next section.

## What the author's pages say (read 2026-10-18)

Read in full: the PiTrex baremetal download page (including its changelog and the reader comments
on it), the baremetal FAQ (`PiTrexBM_FAQ.txt`, 34 questions), the PiTrex project page
`vide.malban.de/pitrex`, the blog front page, the "Git - mainly" post, Kevin Koster's PiTrex page
(`ombertech.com/pitrex.php`) and the PiTrex wiki's Software_Ports page; and `gtoal/pitrex` on GitHub.

* **No license text, no terms of use, no redistribution rule** appears on any of them. The blog footer
  has only a WordPress copyright line. `gtoal/pitrex` has no LICENSE file either (the URL returns 404).
* **Statements of intent, not licenses:**
  * Malban's PiTrex page: "The PCB design is to be open source ... The Software that runs on the pi
    will also be open source."
  * `gtoal/pitrex` README: "PiTrex is a open source/open hardware project."
  * Kevin Koster's page: "PiTrex is an open-source project documented at the PiTrex Wiki and with code
    hosted at GitHub."
* **The FAQ invites changing the menu** but gives no terms. Question 13, "Can I change the selection of
  games, or add favorites?": "No, not yet, you need to recompile the loader. If you are into that (not
  too complicated), instructions can be found at: http://vide.malban.de/ (pitrex menu)."
* **The author collaborates through GitHub:** the "Git - mainly" post (2020) says he made the
  repository and will add people as collaborators so they can push.
* **Third-party pieces are credited with their own terms:** GS BASIC (MIT, Robert E. Alexander, stated on
  the download page), VxTRON (Jaymz Julian, "slightly modified"), the ZBLAST music (Roald Strauss,
  given to the author). The arcade ROMs are never included; the FAQ tells users to get them from MAME.
* A third-party menu fork (`mrchrisster/pitrex`) has been public since 2021 saying it is based on
  Malban's code; no objection was visible in what was read.

**Conclusion:** the project is described as open source by everyone involved, and changing the menu is
encouraged, but no licence is stated anywhere. That is not the same as permission to redistribute
modified builds. Asking him (done 2026-10-18 on the download page) is still the right step.
Not read: the Vectrex blog's older posts, the wiki pages other than Software_Ports, and the separate
arcade-emulator authors' terms.

### The pitrex-dev group archive (read 2026-10-18)

`groups.io/g/pitrex-dev` has a public archive: 130 topics, 2020 to May 2026 (about 735 KB of text).
All 130 were downloaded and searched for licence, GPL, open source, copyright, redistribution,
permission, fork and contribution wording, and the menu-related threads were read.

* **No licence for the baremetal code is stated or discussed.** The only hits were unrelated: a MAME
  copyright disclaimer screen, a request for open-source GUI software, Open Source Hardware
  certification for the PCB (Graham Toal), a 3D printer.
* **Kevin Koster** (hardware, PiTrexCore on Linux) says developers are expected to contribute to the
  baremetal repo or build their own images. In March 2026 he wrote that adding games to the menu
  without editing it "wouldn't be hard to do ... for Bare-Metal executables too, but for free games,
  maybe developers would be happy to just keep adding new games to the base Bare-Metal ZIP with
  hard-coded menu entries".
* **Brandon Yates** (author of the Tensor Valley game, March 2026) asked how to add a game to the
  baremetal menu without forking, and wished the menu "just scanned a folder for .img and listed
  them" with icons. It got no answer on the list. This is the feature the new menu provides.
* **Roger Boesch** (March 2026) suggested a download store for PiTrex games; see
  `docs/menu/10-other-work.md` for his later post.
* **Graham Toal** (March 2026) on a free game with paid carts: free download is preferred, and "no-one
  has ever objected" to voluntary-donation downloads. Shows a relaxed community attitude, nothing more.
* Malban posts in the group mostly about builds and bugs (the toolchain VM, save-state settings). I
  found nothing from him about others reusing or redistributing his code.

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
