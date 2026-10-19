#!/usr/bin/env python3
"""Turns a folder of 0.1.0 entry files (one menu/<name>.ini per game) into one menu file.

    python3 -I docs/tools/entries_to_menu_ini.py <menu folder> <output .ini>

Used to make the shipped menu/menu.default.ini from docs/menu/sidecar/menu, and by anyone who edited 0.1.0 files
and wants them in one menu.ini. Every entry becomes a section named after its title (a second entry with the same
title gets a number and a title line). Entries that play a file from a folder with the video or audio player become
one folder rule for the folder plus a short section per file. skip lines are left out: only the original menu used
them, and it reads the separate files.
"""
import os, re, sys

PLAYERS = ('vectrexvideo.img', 'vectrexaudio.img')
ORDER = ['title', 'category', 'image', 'param1', 'param2', 'needs', 'emulator', 'hide', 'text', 'control']

HEAD = """; menu.ini: the whole PiTrex menu in one file.
;
; How to change the menu: copy this file to menu.ini in the same folder (menu/) and edit that copy. The
; PiTrex reads menu.ini when it exists, otherwise this file; an update of the package replaces this file but
; never your menu.ini. A line starting with ; is a comment.
;
; Each [SECTION] is one entry; its name is the title shown in the menu. The line that says what it is:
;   image  = ...   a program to start (with param1, param2 if it takes them)
;   file   = ...   notes, a title or a category for a file the menu finds by itself
;   folder = ...   with image: every file in that folder becomes an entry ({dir} and {file} are filled in)
;   none           settings of a category: [ARCADE / ATARI] with hide = 1, order = listed, text = ...
; Other lines: category = ARCADE / ATARI (where it is listed; a new path makes a new folder), needs = a file that
; must be on the card, text = a note line (up to 12), control = a line of the CONTROLS page (Button 2), hide = 1,
; emulator = speedy or exact (Vectrex games). Text: A-Z, 0-9, . and ! only; about 46 characters per line.
; Full description: NEW_MENU_README.md
"""


def read_entry(path):
    vals = []
    for line in open(path, encoding='latin-1'):
        line = line.strip()
        if not line or line[0] in ';#[':
            continue
        if '=' not in line:
            continue
        k, v = line.split('=', 1)
        k = k.strip()
        v = re.sub(r'\s;.*$', '', v.strip()).strip()
        vals.append((k, v))
    return vals


def get(vals, key):
    for k, v in vals:
        if k == key:
            return v
    return ''


def section(name, vals, out, title_line=False):
    out.append('')
    out.append('[%s]' % name)
    width = max(len(k) for k in ORDER)
    for key in ORDER:
        for k, v in vals:
            if k != key or (k == 'title' and not title_line):
                continue
            out.append('%-*s = %s' % (width, k, v))


def main(folder, dest):
    entries = []
    for f in sorted(os.listdir(folder)):
        if f.endswith('.ini') and f not in ('menu.ini', 'menu.default.ini'):
            vals = read_entry(os.path.join(folder, f))
            if get(vals, 'image'):
                entries.append(vals)
    media = [e for e in entries if get(e, 'image') in PLAYERS and get(e, 'param1').startswith('vectrex/') and get(e, 'param2')]
    games = [e for e in entries if e not in media]
    games.sort(key=lambda e: (get(e, 'category').upper(), get(e, 'title').upper()))

    out = [HEAD.rstrip('\n')]
    used = {}
    cat = None
    for e in games:
        c = get(e, 'category') or 'PROGRAMS'
        if c != cat:
            out.append('')
            out.append('; ' + '-' * 20 + ' ' + c)
            cat = c
        title = get(e, 'title')
        n = used.get(title.upper(), 0) + 1
        used[title.upper()] = n
        if n == 1:
            section(title, e, out)
        else:
            section('%s %d' % (title, n), e, out, title_line=True)

    rules = {}
    for e in media:
        rules.setdefault((get(e, 'param1'), get(e, 'image'), get(e, 'category')), []).append(e)
    for (folder_path, image, category), files in sorted(rules.items()):
        out.append('')
        out.append('; ' + '-' * 20 + ' %s: every file in %s' % (category, folder_path))
        name = {'vectrexvideo.img': 'VIDEOS', 'vectrexaudio.img': 'SONGS'}.get(image, folder_path.upper())
        rule = [('folder', folder_path), ('category', category), ('image', image), ('param1', '{dir}'), ('param2', '{file}')]
        out.append('')
        out.append('[%s]' % name)
        for k, v in rule:
            out.append('%-8s = %s' % (k, v))
        for e in sorted(files, key=lambda e: get(e, 'title').upper()):
            out.append('')
            out.append('[%s]' % get(e, 'title'))
            out.append('%-8s = %s/%s' % ('file', folder_path, get(e, 'param2')))
            for k, v in e:
                if k in ('text', 'control', 'hide'):
                    out.append('%-8s = %s' % (k, v))
    open(dest, 'w', newline='\r\n').write('\n'.join(out) + '\n')
    print('%s: %d entries, %d folder rules with %d files' % (dest, len(games), len(rules), len(media)))


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
