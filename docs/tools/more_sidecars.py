#!/usr/bin/env python3
"""Writes the entry files for the games that export_sidecars.py does not cover: the other versions of games
that AAE can run (one entry file each, AAE game number as param1), Q*bert's Qubes on cine.img, the vecsim
Asteroids programs and the small sample programs. Run it after export_sidecars.py and before controls_sidecars.py.

Usage: python3 -I docs/tools/more_sidecars.py <menu folder>

Not included on purpose: the main sets of the games that have their own emulator (Battle Zone, Black Widow,
Gravitar, Lunar Lander, Red Baron, Space Duel, Tempest, Star Wars, Barrier, Demon, Space Wars, Tailgunner). They
are in the menu already and also in the AAE picker. Cosmic Chasm: cine.img accepts the name but loadCosmicChasm()
in cinematronics/cinemu.c is an empty stub (the game is a 68000 board), so there is nothing to start.
"""
import os, sys

HEAD = ("; PiTrex menu entry: the new menu is built from files like this one.\n"
        "; Delete the file and the entry is gone from the menu (a program is then listed under PROGRAMS).\n"
        "; Format: docs/menu/05-sidecar-format.md\n[entry]\n")

# (AAE game number, MAME name, title, category). Numbers are positions in the driver table of aae/aaemain.c.
AAE = [
    (0x02, 'meteorts', 'METEORITES BOOTLEG', 'ARCADE / ATARI / ASTEROIDS VERSIONS'),
    (0x03, 'asterock', 'ASTEROCK BOOTLEG', 'ARCADE / ATARI / ASTEROIDS VERSIONS'),
    (0x04, 'asteroib', 'ASTEROIDS LUNAR HW BOOTLEG', 'ARCADE / ATARI / ASTEROIDS VERSIONS'),
    (0x05, 'asteroi1', 'ASTEROIDS REV 1', 'ARCADE / ATARI / ASTEROIDS VERSIONS'),
    (0x06, 'asteroid', 'ASTEROIDS REV 2', 'ARCADE / ATARI / ASTEROIDS VERSIONS'),
    (0x07, 'astdelu1', 'ASTEROIDS DELUXE REV 1', 'ARCADE / ATARI / ASTEROIDS VERSIONS'),
    (0x08, 'astdelu2', 'ASTEROIDS DELUXE REV 2', 'ARCADE / ATARI / ASTEROIDS VERSIONS'),
    (0x0d, 'bzone2', 'BATTLE ZONE REV 2', 'ARCADE / ATARI / BATTLE ZONE VERSIONS'),
    (0x0e, 'bzonec', 'BATTLE ZONE COCKTAIL', 'ARCADE / ATARI / BATTLE ZONE VERSIONS'),
    (0x0f, 'bzonep', 'BATTLE ZONE PLUS', 'ARCADE / ATARI / BATTLE ZONE VERSIONS'),
    (0x15, 'gravitr2', 'GRAVITAR REV 2', 'ARCADE / ATARI / GRAVITAR VERSIONS'),
    (0x16, 'gravp', 'GRAVITAR PROTOTYPE', 'ARCADE / ATARI / GRAVITAR VERSIONS'),
    (0x00, 'llander1', 'LUNAR LANDER REV 1', 'ARCADE / ATARI / LUNAR LANDER VERSIONS'),
    (0x17, 'lunarbat', 'LUNAR BATTLE LATE', 'ARCADE / ATARI / LUNAR LANDER VERSIONS'),
    (0x18, 'lunarba1', 'LUNAR BATTLE EARLY', 'ARCADE / ATARI / LUNAR LANDER VERSIONS'),
    (0x19, 'tempestm', 'TEMPEST MULTIGAME', 'ARCADE / ATARI / TEMPEST VERSIONS'),
    (0x1b, 'tempest3', 'TEMPEST REV 2B', 'ARCADE / ATARI / TEMPEST VERSIONS'),
    (0x1c, 'tempest2', 'TEMPEST REV 2A', 'ARCADE / ATARI / TEMPEST VERSIONS'),
    (0x1d, 'tempest1', 'TEMPEST REV 1', 'ARCADE / ATARI / TEMPEST VERSIONS'),
    (0x1e, 'temptube', 'TEMPEST REV 1 TUBES', 'ARCADE / ATARI / TEMPEST VERSIONS'),
    (0x1f, 'alienst', 'ALIENS TEMPEST ALPHA', 'ARCADE / ATARI / TEMPEST VERSIONS'),
    (0x21, 'vortex', 'VORTEX TEMPEST BETA', 'ARCADE / ATARI / TEMPEST VERSIONS'),
    (0x2e, 'mhavoc2', 'MAJOR HAVOC REV 2', 'ARCADE / ATARI / MAJOR HAVOC VERSIONS'),
    (0x2f, 'mhavocrv', 'MAJOR HAVOC RETURN TO VAX', 'ARCADE / ATARI / MAJOR HAVOC VERSIONS'),
    (0x30, 'mhavocp', 'MAJOR HAVOC PROTOTYPE', 'ARCADE / ATARI / MAJOR HAVOC VERSIONS'),
    (0x31, 'alphaone', 'ALPHA ONE 3 LIVES', 'ARCADE / ATARI / MAJOR HAVOC VERSIONS'),
    (0x32, 'alphaona', 'ALPHA ONE 5 LIVES', 'ARCADE / ATARI / MAJOR HAVOC VERSIONS'),
    (0x35, 'quantum1', 'QUANTUM REV 1', 'ARCADE / ATARI / QUANTUM VERSIONS'),
    (0x37, 'quantump', 'QUANTUM PROTOTYPE', 'ARCADE / ATARI / QUANTUM VERSIONS'),
    (0x33, 'starwar1', 'STAR WARS REV 1 AAE', 'ARCADE / ATARI'),
    (0x11, 'bradley', 'BRADLEY TRAINER', 'ARCADE / ATARI'),
    (0x20, 'vbreak', 'VECTOR BREAKOUT', 'ARCADE / OTHER ARCADE'),
    (0x0b, 'deltrace', 'DELTA RACE', 'ARCADE / OTHER ARCADE'),
    (0x26, 'spacfura', 'SPACE FURY REV A', 'ARCADE / SEGA / SPACE FURY VERSIONS'),
    (0x27, 'spacfurb', 'SPACE FURY REV B', 'ARCADE / SEGA / SPACE FURY VERSIONS'),
    (0x29, 'elim2a', 'ELIMINATOR 2P SET 2A', 'ARCADE / SEGA / ELIMINATOR VERSIONS'),
    (0x2a, 'elim2c', 'ELIMINATOR 2P SET 2C', 'ARCADE / SEGA / ELIMINATOR VERSIONS'),
    (0x2b, 'elim4', 'ELIMINATOR 4 PLAYER', 'ARCADE / SEGA / ELIMINATOR VERSIONS'),
    (0x2c, 'elim4p', 'ELIMINATOR 4P PROTOTYPE', 'ARCADE / SEGA / ELIMINATOR VERSIONS'),
]

def write(folder, key, lines):
    for l in lines:
        v = l.split('=', 1)[1].strip()
        assert len(v) <= 46, (key, l)
    open(os.path.join(folder, key + '.ini'), 'w').write(HEAD + '\n'.join(lines) + '\n')

folder = sys.argv[1]
for num, name, title, cat in AAE:
    assert len(title) <= 28, title
    write(folder, 'aae.' + name, [
        'title = ' + title, 'category = ' + cat, 'image = aae.img', 'param1 = 0x%02x' % num,
        'needs = roms/aae/%s.zip' % name, 'skip = 4',
        'text = ANOTHER VERSION OF THE GAME',
        'text = RUNS ON AAE. NOT TESTED.',
    ])

write(folder, 'cine.qb3', [
    'title = QB3', 'category = ARCADE / OTHER ARCADE', 'image = cine.img', 'param1 = qb3',
    'needs = roms/qb3.zip', 'skip = 4',
    'text = CINEMATRONICS EMU',
    'text = Q BERTS QUBES PROTOTYPE. NOT TESTED.',
])
write(folder, 'asteroids.sim', [  # key must be the image's own name to be found as an entry; see below
])
os.remove(os.path.join(folder, 'asteroids.sim.ini'))
write(folder, 'asteroids', [
    'title = ASTEROIDS SIM', 'category = ARCADE / ATARI', 'image = asteroids.img',
    'needs = roms/asteroid.zip', 'skip = 4',
    'text = OLDER EMULATOR. NOT TESTED.',
    'text = ASTEROIDS SBT IS THE FASTER ONE.',
])
write(folder, 'deluxe', [
    'title = ASTEROIDS DELUXE SIM', 'category = ARCADE / ATARI', 'image = deluxe.img',
    'needs = roms/astdelux.zip', 'skip = 4',
    'text = OLDER EMULATOR. NOT TESTED.',
])
write(folder, 'gyrocks', [
    'title = GYROCKS', 'category = COMPUTER', 'image = gyrocks.img', 'skip = 4',
    'text = NOT TESTED.',
])
for img, title in (('maze', 'MAZE'), ('pacman', 'PACMAN DEMO'), ('perspective', 'PERSPECTIVE')):
    write(folder, img, [
        'title = ' + title, 'category = COMPUTER / SAMPLE PROGRAMS', 'image = %s.img' % img, 'skip = 4',
        'text = SAMPLE PROGRAM FROM THE SOURCE. NOT TESTED.',
    ])
for img, title in (('hello', 'HELLO WORLD'), ('window', 'WINDOW')):
    write(folder, img, [
        'title = ' + title, 'category = COMPUTER / SAMPLE PROGRAMS', 'image = %s.img' % img, 'hide = 1', 'skip = 4',
        'text = TEST PROGRAM. HIDDEN. REMOVE THE HIDE LINE',
        'text = TO SHOW IT.',
    ])

# titles the original menu spelled badly
FIX = {'aae.x28': ('ELEMINATOR', 'ELIMINATOR'), 'aae.x25': ('SPACFURY', 'SPACE FURY'), 'aae.x24': ('STARTREK', 'STAR TREK'),
       'aae.x23': ('TACSCAN', 'TAC SCAN'), 'aae.x22': ('Zektor', 'ZEKTOR')}
for key, (old, new) in FIX.items():
    p = os.path.join(folder, key + '.ini')
    if os.path.exists(p):
        t = open(p).read()
        if 'title = ' + old + '\n' in t:
            open(p, 'w').write(t.replace('title = ' + old + '\n', 'title = ' + new + '\n'))

# the AAE picker is only shown while at least one AAE game entry is
p = os.path.join(folder, 'aae.ini')
t = open(p).read()
if 'needs' not in t:
    t = t.replace('image = aae.img\n', 'image = aae.img\nneeds = roms/aae/\n')
    open(p, 'w').write(t)
print(len(AAE) + 9, 'entry files written')
