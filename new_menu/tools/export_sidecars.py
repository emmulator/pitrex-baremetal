#!/usr/bin/env python3
"""Export the compiled-in menu notes of pitrex/loader/loaderMain.c as sidecar files.

Usage: python3 -I new_menu/tools/export_sidecars.py pitrex/loader/loaderMain.c OUTDIR
Writes OUTDIR/menu/<key>.ini for every reachable entry that starts an image (the entry files the new
menu is built from: title, category, image, parameters, the ROM files it needs, notes), and
OUTDIR/keys.tsv (image, param1 hex, param2 hex, key) for the host test
new_menu/tools/test_sidecar_key.c. The key rules must match pitrex/loader/menuSidecar.h.
Format: new_menu/menu/05-sidecar-format.md
"""
import re, sys, os

src = open(sys.argv[1], encoding='latin-1').read()
out = sys.argv[2]

items = {}
for m in re.finditer(r'MenuItemNew\s+(\w+)\s*=\s*\{(.*?)\n\};', src, re.S):
    body = re.sub(r'//[^\n]*', '', m.group(2))
    f = []; cur = ''; q = False; d = 0; esc = False
    for ch in body:
        if q:
            cur += ch
            if esc: esc = False
            elif ch == '\\': esc = True
            elif ch == '"': q = False
            continue
        if ch == '"': q = True
        if ch in '{(': d += 1
        if ch in '})': d -= 1
        if ch == ',' and d == 0:
            f.append(cur.strip()); cur = ''; continue
        cur += ch
    f.append(cur.strip())
    items[m.group(1)] = f

STR = r'"((?:[^"\\]|\\.)*)"'

def cdecode(s):
    """Decode the C escapes used in the menu strings into bytes."""
    b = bytearray(); i = 0
    while i < len(s):
        c = s[i]
        if c == '\\' and i + 1 < len(s):
            n = s[i + 1]
            if n == 'x':
                j = i + 2
                while j < len(s) and s[j] in '0123456789abcdefABCDEF': j += 1
                b.append(int(s[i + 2:j], 16) & 255); i = j; continue
            b.extend({'n': b'\n', 't': b'\t', '\\': b'\\', '"': b'"'}.get(n, n.encode())); i += 2; continue
        b.extend(c.encode('latin-1')); i += 1
    return bytes(b)

def strfield(x):
    m = re.fullmatch(STR, x.strip())
    return cdecode(m.group(1)) if m else b''

def imgfield(x):
    m = re.match(r'IMG_FILE_PREFIX\s*' + STR, x.strip())
    return 'piZero1/' + m.group(1) if m else ''

def slug(bs):
    if len(bs) == 1:
        return 'x%02x' % bs[0]
    o = ''
    for c in bs:
        ch = chr(c).lower() if c < 128 else '_'
        if ch.isascii() and (ch.isalpha() or ch.isdigit()): o += ch
        elif o and o[-1] != '_': o += '_'
    return o.rstrip('_')

def key(img, p1, p2):
    if not img: return None
    base = img.split('/')[-1]
    if base.endswith('.img'): base = base[:-4]
    k = base.lower()
    for p in (p1, p2):
        if p:
            s = slug(p)
            if s: k += '.' + s
    return k

def ref(x):
    x = x.strip().lstrip('&')
    return '' if x == '0' else x

def chain(first):
    res = []; n = first; seen = set()
    while n and n in items and n not in seen:
        seen.add(n); res.append(n); n = ref(items[n][11])
    return res

# files each entry needs (testOneRomId in the loader): id -> path
needs = {}
m = re.search(r'int testOneRomId\(int id\)(.*?)\n}\n', src, re.S)
if m:
    for cm in re.finditer(r'case\s+(\d+)\s*:\s*(?://[^\n]*)?\s*\{?\s*(?://[^\n]*)?\s*testPath\s*=\s*"([^"]+)"', m.group(1)):
        needs[cm.group(1)] = cm.group(2)

TOP = {'1': 'VECTREX', '2': 'ARCADE', '3': 'PITREX', '4': 'VIDEO', '5': 'AUDIO'}
def label(n):
    f = items[n]
    nm = strfield(f[1]).decode('latin-1').strip()
    return nm or TOP.get(f[0]) or ({'102': 'SPEEDY VECTREX', '107': 'EXACT VECTREX'}.get(f[0], '?'))

reached = []
def walk(n, path):
    f = items[n]
    reached.append((n, path))
    for c in chain(ref(f[9])):
        walk(c, path + [label(n)])
for t in chain('settingsMenu'):
    walk(t, [])

def rows_of(f):
    strings = [cdecode(s).decode('latin-1') for s in re.findall(STR, ' '.join(f[13:]))]
    skip = 0
    while skip < len(strings) and strings[skip] == '': skip += 1
    rows = strings[skip:]
    while rows and rows[-1] == '': rows.pop()
    return skip, rows

os.makedirs(out + '/menu', exist_ok=True)
manifest = []; written = {}; problems = []; expected = []
for n, path in reached:
    f = items[n]
    img = imgfield(f[3]); p1 = strfield(f[4]); p2 = strfield(f[5])
    if f[0] in ('9999', '102', '107'):   # PICK A GAME and the two emulator slots are not games
        continue
    k = key(img, p1, p2)
    if k is None: continue
    manifest.append((img, p1, p2, k))
    skip, rows = rows_of(f)
    for r in rows:
        if ';' in r or '#' in r: problems.append(f'{n}: ";" or "#" in note {r!r}')
        if len(r) >= 48: problems.append(f'{n}: note longer than 47 chars {r!r}')
        if any(ord(c) > 126 or ord(c) < 32 for c in r): problems.append(f'{n}: non-ASCII in {r!r}')
    if len(rows) > 12: problems.append(f'{n}: more than 12 rows')
    title = strfield(f[1]).decode('latin-1').strip() or label(n)
    def show(bs):
        if len(bs) == 1: return '0x%02x' % bs[0]
        return bs.decode('latin-1')
    lines = ['; PiTrex menu entry: the new menu is built from files like this one.',
             '; Delete the file and the entry is gone from the menu (a program is then listed under PITREX).',
             '; Format: new_menu/menu/05-sidecar-format.md',
             '[entry]', f'title = {title}',
             'category = ' + ' / '.join(x for x in path if x)]
    lines.append('image = ' + img.split('/')[-1])
    if p1: lines.append('param1 = ' + show(p1))
    if p2: lines.append('param2 = ' + show(p2))
    if f[0] in needs: lines.append('needs = ' + needs[f[0]])
    if skip: lines.append(f'skip = {skip}')
    for r in rows:
        q = '"%s"' % r if (r != r.strip()) else r
        lines.append('text = ' + q if q else 'text =')
    body = '\n'.join(lines) + '\n'
    if k in written and written[k] != body:
        problems.append(f'duplicate key {k} with different text ({n})')
        continue
    written[k] = body
    expected.append((k, skip, rows))
    open(f'{out}/menu/{k}.ini', 'w').write(body)

# entries that are plain data and were never in the compiled menu (Vectorburner is not on Malban's card: a program
# the card has is listed under PITREX by itself)
EXTRA = {
    'aae': ['title = ALL AAE GAMES', 'category = ARCADE', 'image = aae.img', 'skip = 4',
            'text = LISTS THE ROM ZIPS', 'text = FOUND IN ROMS/AAE'],
}
for k, body in EXTRA.items():
    if k not in written:
        text = '; PiTrex menu entry: the new menu is built from files like this one.\n[entry]\n' + '\n'.join(body) + '\n'
        written[k] = text
        open(f'{out}/menu/{k}.ini', 'w').write(text)

with open(out + '/keys.tsv', 'w') as fh:
    for img, p1, p2, k in manifest:
        fh.write(f'{img}\t{p1.hex()}\t{p2.hex()}\t{k}\n')

with open(out + '/expected.tsv', 'w') as fh:
    for k, skip, rows in expected:
        fh.write(f'{k}\t{skip}\t' + '\x1f'.join(rows) + '\n')

ks = [m[3] for m in manifest]
dups = sorted({k for k in ks if ks.count(k) > 1})
print(f'reachable entries: {len(reached)}; with an image: {len(manifest)}; sidecar files written: {len(written)}')
print('keys shared by more than one reachable entry:', dups or 'none')
for p in problems: print('PROBLEM', p)
