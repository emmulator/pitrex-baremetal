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
| [05-sidecar-format.md](menu/05-sidecar-format.md) | Menu notes as editable `menu/<key>.ini` files on the SD card |
| [06-aae-picker.md](menu/06-aae-picker.md) | AAE accepts game names and lists the ROM zips on the card |
| [07-new-menu.md](menu/07-new-menu.md) | The new list menu (`NEW_MENU = 1`), program and ROM discovery |

Tools in `docs/tools/`: `menu_tables.py` (tables in file 01), `export_sidecars.py` (sidecar files), `test_sidecar_key.c` and `test_aae_select.c` and `test_new_menu.c` (host tests), `gen_aae_table.py`, `gen_menu_tree.py`, `make_rom_test_data.py`, `build.sh` (ARM build in the container).

Status markers used in these docs: **Verified** = read directly in the source;
**Unverified** = inferred or not yet tested on real PiTrex hardware.
