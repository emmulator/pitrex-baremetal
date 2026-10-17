# Fork documentation

Notes for this fork of [malbanGit/pitrex-baremetal](https://github.com/malbanGit/pitrex-baremetal).
Nothing here is part of upstream.

## Menu redesign (`docs/menu/`)

| File | Contents |
|---|---|
| [01-existing-menu-structure.md](menu/01-existing-menu-structure.md) | The loader menu as it exists today, generated from `pitrex/loader/loaderMain.c` |
| [02-findings.md](menu/02-findings.md) | How the loader and the emulators behave, with source references; open questions |
| [03-redesign-plan.md](menu/03-redesign-plan.md) | Goals, constraints, card conventions and phases for a dynamic, hierarchical menu |
| [04-arcade-candidates.md](menu/04-arcade-candidates.md) | Arcade titles that could be added, UVMC2 comparison, MAME/VTerm |

`docs/tools/menu_tables.py` regenerates the tables in file 01 from the source.

Status markers used in these docs: **Verified** = read directly in the source;
**Unverified** = inferred or not yet tested on real PiTrex hardware.
