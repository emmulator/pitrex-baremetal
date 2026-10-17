#!/usr/bin/env python3
"""Writes the AAE driver names and descriptions from aae/aaemain.c as a C include, for
docs/tools/test_aae_select.c.   python3 -I docs/tools/gen_aae_table.py aae/aaemain.c > aae_table.inc"""
import re, sys
am = open(sys.argv[1], encoding='latin-1').read()
i = am.index('struct AAEDriver driver[] =')
d = 0; k = am.index('{', i); j = k
while True:
    c = am[j]
    if c == '{': d += 1
    elif c == '}':
        d -= 1
        if d == 0: break
    j += 1
blk = am[k:j]; ents = []; d = 0
for p, ch in enumerate(blk):
    if ch == '{':
        d += 1
        if d == 2: st = p
    elif ch == '}':
        if d == 2: ents.append(blk[st:p + 1])
        d -= 1
rows = [re.match(r'\{\s*"([^"]*)"\s*,\s*"([^"]*)"', e) for e in ents]
rows = [m.groups() for m in rows if m]
print('static const char *tNames[] = {' + ', '.join('"%s"' % n for n, _ in rows) + '};')
print('static const char *tDescs[] = {' + ', '.join('"%s"' % d for _, d in rows) + '};')
print('#define TCOUNT %d' % len(rows))
