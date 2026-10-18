#!/usr/bin/env python3
"""Writes rom_paths.txt and rom_expected.txt for docs/tools/test_new_menu.c.
   python3 -I docs/tools/make_rom_test_data.py <dir with a vectrex/ tree> <dir with .bin/.vec files> <outdir> [<menu folder> <piZero1 folder>]
The expected file is computed independently of the C code: it lists, in menu order, the
rows "G:<group>", "E:<entry in a group>" and "R:<entry directly in vectrex/>"."""
import os, re, sys, glob
vroot, extra, out = sys.argv[1], sys.argv[2], sys.argv[3]
menudir = sys.argv[4] if len(sys.argv) > 4 else None     # optional: entry files folder and image folder
imgdir = sys.argv[5] if len(sys.argv) > 5 else None
paths = []
for r, ds, fs in os.walk(vroot):
    for f in fs: paths.append(os.path.relpath(os.path.join(r, f), vroot))
for f in sorted(glob.glob(extra + '/*')):
    n = os.path.basename(f)
    if n.lower().endswith(('.bin', '.vec')) and len(n) < 60: paths.append(n)
paths += ['.hidden.bin', 'readme.txt', 'bios/x.bin', 'movies/m.bin', 'waves/w.bin', 'UPPER.BIN',
          'sub/dir/deep.bin', 'sub/Another_One.bin', 'demos/Zed.bin', 'BIOS/y.bin']
def ok(p):
    if len(p) <= 4 or len(p) >= 64 or not p.lower().endswith('.bin'): return False
    segs = p.split('/')
    if any(s == '' or s.startswith('.') for s in segs): return False
    return not any(s.lower() in ('bios', 'movies', 'waves') for s in segs[:-1])
def title(p): return p.split('/')[-1][:-4].upper().replace('_', ' ').replace('-', ' ')
def clean(t):   # same rules as nmCleanTitle in newMenuLogic.h
    t = re.sub(r'[()\[\]]', '', t.upper())
    t = re.sub(r'[^A-Z0-9 .!]', ' ', t)
    t = re.sub(r' +', ' ', t).strip()
    return t[:-4] if len(t) > 4 and t.endswith(' SBT') else t
def d(p): return '/'.join(p.split('/')[:-1])
okp = [p for p in paths if ok(p)]
folderfiles = {p.split('/')[-1].lower() for p in okp if '/' in p}
good = sorted((p for p in okp if '/' in p or p.lower() not in folderfiles), key=lambda p: (d(p).lower(), title(p).lower()))
rows = []; cur = None
for p in good:
    if d(p):
        if d(p) != cur:
            cur = d(p); rows.append('G:' + d(p).upper().replace('_', ' ').replace('-', ' ').replace('/', '>') + ' FOLDER')
        rows.append('E:' + clean(title(p)))
rows += ['R:' + clean(title(p)) for p in good if not d(p)]
os.makedirs(out, exist_ok=True)
open(out + '/rom_paths.txt', 'w').write('\n'.join(paths) + '\n')
open(out + '/rom_expected.txt', 'w').write('\n'.join(rows) + '\n')
print(len(paths), 'paths;', len(good), 'listed;', len(rows), 'rows')

if menudir and imgdir:      # programs: images that no entry file mentions (independent of the C code)
    names = sorted(n for n in os.listdir(imgdir) if os.path.isfile(os.path.join(imgdir, n)) and not n.startswith('.'))
    open(out + '/img_listing.txt', 'w').write('\n'.join(names) + '\n')
    used = set()
    for n in os.listdir(menudir):
        if n.endswith('.ini'):
            m = re.search(r'^image\s*=\s*(\S+)', open(os.path.join(menudir, n), encoding='latin-1').read(), re.M)
            if m: used.add(m.group(1).lower().rsplit('/', 1)[-1][:-4] if m.group(1).lower().endswith('.img') else m.group(1).lower())
    system = {'pitrex', 'pitrex7', 'vectrexexact', 'vectrexspeedy'}
    progs = sorted({n[:-4].lower() for n in names if n.lower().endswith('.img')} - used - system)
    open(out + '/expected_programs.txt', 'w').write('\n'.join(progs) + '\n')
    print(len(names), 'images;', len(progs), 'programs expected')
