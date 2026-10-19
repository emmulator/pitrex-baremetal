#!/usr/bin/env python3
"""Prints the two ROM tables of new_menu/README.md (Markdown) from the entry files.

    python3 -I new_menu/tools/gen_rom_tables.py new_menu/menu/sidecar/menu aae_names.txt

aae_names.txt: the 71 MAME names of the AAE driver table, then the 71 descriptions, one per line
(python3 -I new_menu/tools/gen_aae_table.py aae/aaemain.c prints them as C arrays; `tNames` first, `tDescs` second).
RPITEAM lists the zips the RpiTeaM Edition image contains; every other zip needs another source.
"""
import glob, os, re, sys

# roms/ in PitreX_Baremetal_RpiTeaM_Edition.img: 26 zips; roms/aae/: 60 zips (checked 2026-10-18)
NOT_IN_RPITEAM = {'bradley', 'mhavoc2', 'mhavocrv', 'mhavocp', 'alphaone', 'alphaona', 'quantum1', 'quantump', 'gravitr2', 'gravp'}
DEDICATED = {'llander', 'bzone', 'redbaron', 'spacduel', 'bwidow', 'gravitar', 'tempest', 'starwars', 'barrier', 'tailg', 'spacewar', 'demon'}
NICE = {'qb3.zip': 'QB-3 (Rock-Ola prototype)', 'astdelux.zip': 'Asteroids Deluxe (older emulator)',
        'asteroid.zip': 'Asteroids (older emulator; also the second set Asteroids reads)', 'asteroid2.zip': 'Asteroids'}

folder, namesfile = sys.argv[1], sys.argv[2]
names = open(namesfile).read().split('\n')
half = len(names) // 2
drv, desc = names[:half], names[half:]
ents = {}
for f in glob.glob(os.path.join(folder, '*.ini')):
    t = open(f, encoding='latin-1').read()
    g = lambda k: (re.findall(r'(?m)^' + k + r'\s*=\s*(.*)$', t) or [''])[0].strip()
    ents[os.path.basename(f)[:-4]] = dict(title=g('title'), cat=g('category'), img=g('image'), p1=g('param1'), needs=g('needs'))
byaae = {int(e['p1'], 16): e for e in ents.values() if e['img'] == 'aae.img' and e['p1'].startswith('0x')}

def where(e):
    return ' > '.join(p.strip().title() for p in e['cat'].replace('ARCADE / ', '').split('/')) + ' > ' + e['title'].title()

print('| Zip | Game | Menu | Program |\n|---|---|---|---|')
rows = []
for e in ents.values():
    n = e['needs']
    if n.startswith('roms/') and not n.startswith('roms/aae/') and n.endswith('.zip'):
        z = n[5:]
        rows.append((z, NICE.get(z, e['title'].title()), ' > '.join(p.strip().title() for p in e['cat'].replace('ARCADE / ', '').split('/')), e['img']))
for z, t, c, img in sorted(rows):
    print('| `%s` | %s | %s | `%s` |' % (z, t, c, img))
print()
print('| Zip | Game | Menu | In the RpiTeaM image |\n|---|---|---|---|')
for n, (name, d) in enumerate(zip(drv, desc)):
    if name == 'galaga':
        continue
    e = byaae.get(n)
    if e: w = where(e)
    elif name == 'starwars': w = 'Atari > Star Wars (its own program, `swsbt.img`, reads this zip)'
    elif name in DEDICATED: w = 'picker only (the game has its own emulator above)'
    else: w = 'picker only'
    print('| `%s.zip` | %s | %s | %s |' % (name, d.strip(), w, 'no' if name in NOT_IN_RPITEAM else 'yes'))
