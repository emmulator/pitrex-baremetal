#!/usr/bin/env python3
"""Adds "control = ..." lines (the CONTROLS page, Button 2 in the new menu) to sidecar files.

Usage: python3 -I docs/tools/controls_sidecars.py <menu folder>
The folder holds menu/<key>.ini files (docs/menu/sidecar/menu or the card's menu/). Existing control
lines are replaced; a missing file is created with only the control lines. Text uses only the characters
the PiTrex font draws (A-Z 0-9 space . !), at most 46 per line.

Where each mapping comes from (sources in this repository):
  sim games (Battle Zone, Black Widow, Red Baron, Lunar Lander, Gravitar, Space Duel, Tempest): sim/vx_interface.c
  Asteroids SBT: the defaults in asteroid_sbt.ini and PiTrexBM_FAQ.txt (22)
  AAE games: aae/input.h (coin = buttons 3 and 4, start = buttons 1 and 2) and the game files
"""
import os, re, sys

SIM_COIN = ["COIN  BUTTONS 2 AND 3 TOGETHER", "START  BUTTON 1 ONE PLAYER  BUTTON 2 TWO"]
AAE_GENERIC = [
    "COIN  BUTTONS 3 AND 4 TOGETHER",
    "START  BUTTONS 1 AND 2 TOGETHER",
    "THEN THE STICK AND BUTTONS ARE USED BY",
    "THE GAME. TRY THEM. NO SETTINGS FILE YET.",
]
BY_KEY = {
    'asteroids_sbt': [
        "ROTATE  STICK LEFT RIGHT OR BUTTON 1 2",
        "THRUST  STICK UP OR BUTTON 3",
        "FIRE  BUTTON 4",
        "HYPERSPACE  STICK DOWN",
        "START 1 PLAYER  BUTTONS 3 AND 4 TOGETHER",
        "SETTINGS  BUTTONS 1 AND 4 TOGETHER",
    ],
    'battlezone': [
        "ONE STICK MODE  SEE BATTLEZONE.INI",
        "DRIVE  STICK UP FORWARD DOWN BACK",
        "TURN  STICK LEFT RIGHT",
        "FIRE  BUTTON 4",
    ] + SIM_COIN,
    'blackwidow': [
        "MOVE  THE STICK",
        "FIRE  HOLD BUTTON 4. IT FIRES THE WAY",
        "THE STICK LAST POINTED.",
        "WITH A SECOND STICK IN PORT 2 SET ONE STICK",
        "TO 0 IN BLACKWIDOW.INI. IT AIMS THE FIRE.",
    ] + SIM_COIN,
    'redbaron': [
        "FLY AND AIM  THE STICK",
        "FIRE  BUTTON 4",
    ] + SIM_COIN,
    'lunar': [
        "TURN  STICK LEFT RIGHT",
        "THRUST  STICK UP. MORE UP MEANS MORE.",
        "COIN  BUTTON 3. HOLD IT A MOMENT.",
        "START  BUTTON 1",
    ],
    'gravitar': [
        "ROTATE  STICK LEFT RIGHT",
        "THRUST  BUTTON 3",
        "FIRE  BUTTON 4",
        "SHIELD AND TRACTOR  BUTTON 1",
    ] + SIM_COIN,
    'spaceduel': [
        "ROTATE  STICK LEFT RIGHT",
        "THRUST  BUTTON 3",
        "FIRE  BUTTON 4",
        "SHIELD  BUTTON 1",
        "PLAYER 2 USES THE SAME ON PORT 2",
    ] + SIM_COIN,
    'tempest': [
        "SPIN  STICK LEFT RIGHT",
        "FIRE  BUTTON 4",
        "TRY BUTTON 3 FOR THE SUPERZAPPER",
    ] + SIM_COIN,
}
# notes the user found to be wrong on hardware (2026-10-17): Black Widow does have sound
TEXT_BY_KEY = {
    'blackwidow': ["WORK IN PROGRESS!!", "FLICKER. SOME SOUND WORKS."],
}
BY_AAE_TITLE = {
    'MAJOR HAVOC': [
        "MOVE  THE STICK",
        "FIRE  BUTTON 4",
        "SHIELD  BUTTON 3",
    ] + AAE_GENERIC[:2],
}

folder = sys.argv[1]

def title_of(text):
    m = re.search(r'^title\s*=\s*(.*)$', text, re.M)
    return m.group(1).strip().upper() if m else ''

def image_of(text):
    m = re.search(r'^image\s*=\s*(.*)$', text, re.M)
    return m.group(1).strip().lower() if m else ''

todo = {}
for name in sorted(os.listdir(folder)):
    if not name.endswith('.ini'):
        continue
    key = name[:-4]
    text = open(os.path.join(folder, name), encoding='latin-1').read()
    if key in BY_KEY:
        todo[key] = BY_KEY[key]
    elif key.startswith('aae.') and image_of(text) == 'aae.img':
        todo[key] = BY_AAE_TITLE.get(title_of(text), AAE_GENERIC)
for key, lines in BY_KEY.items():
    todo.setdefault(key, lines)

for key, lines in todo.items():
    for l in lines:
        assert len(l) <= 46, (key, l)
        assert re.fullmatch(r'[A-Z0-9 .!]*', l), (key, l)
    path = os.path.join(folder, key + '.ini')
    text = open(path, encoding='latin-1').read() if os.path.exists(path) else '[entry]\n'
    text = re.sub(r'^control\s*=.*\n', '', text, flags=re.M)
    if key in TEXT_BY_KEY:
        text = re.sub(r'^text\s*=.*\n', '', text, flags=re.M)
        if not text.endswith('\n'):
            text += '\n'
        text += ''.join('text = %s\n' % l for l in TEXT_BY_KEY[key])
    if not text.endswith('\n'):
        text += '\n'
    text += ''.join('control = %s\n' % l for l in lines)
    open(path, 'w', encoding='latin-1').write(text)
print(len(todo), 'files updated')
