#!/usr/bin/env python3
# Regenerates the menu tables in docs/menu/01-existing-menu-structure.md
# Usage: python3 -I docs/tools/menu_tables.py pitrex/loader/loaderMain.c > tables.md
import re,sys
src=open(sys.argv[1],encoding='latin-1').read()
items={}
for m in re.finditer(r'MenuItemNew\s+(\w+)\s*=\s*\{(.*?)\n\};',src,re.S):
    body=re.sub(r'//[^\n]*','',m.group(2))
    f=[];cur='';q=False;d=0
    for ch in body:
        if ch=='"':q=not q
        if not q:
            if ch in '{(':d+=1
            if ch in '})':d-=1
            if ch==',' and d==0:
                f.append(cur.strip());cur='';continue
        cur+=ch
    f.append(cur.strip())
    items[m.group(1)]=f
def lit(x):
    x=x.strip()
    return x[1:-1] if x.startswith('"') and x.endswith('"') else ''
def img(x):
    x=x.strip()
    m=re.match(r'IMG_FILE_PREFIX\s*"([^"]*)"',x)
    return 'piZeroN/'+m.group(1) if m else ('' if x in ('0','""') else x)
def prm(x):
    v=lit(x)
    v=v.replace('\\\\x','\\x')
    m=re.fullmatch(r'\\x([0-9a-fA-F]{2})',v)
    if m: return f'game #{int(m.group(1),16)} (0x{m.group(1)})'
    return v
def ref(x):
    x=x.strip().lstrip('&')
    return '' if x=='0' else x
def chain(first):
    out=[];n=first;seen=set()
    while n and n in items and n not in seen:
        seen.add(n);out.append(n);n=ref(items[n][11])
    return out
def name(n):
    f=items[n]; return lit(f[1]).strip() or '(category icon)'
top=chain('settingsMenu')
visited=set()
w=sys.stdout.write
def row(n,depth):
    f=items[n]
    p1=prm(f[4]);p2=prm(f[5])
    params=', '.join(x for x in (p1,p2) if x)
    notes=' / '.join(x for x in re.findall(r'"((?:[^"\\]|\\.)*)"',' '.join(f[13:])) if x.strip())
    w(f"| {'&nbsp;&nbsp;'*depth}{name(n)} | `{n}` | {f[0]} | {('`'+img(f[3])+'`') if img(f[3]) else '—'} | {params or '—'} | {notes or '—'} |\n")
    visited.add(n)
    for c in chain(ref(f[9])): row(c,depth+1)
for t in top:
    f=items[t]
    w(f"\n### Top-level slot: `{t}` (id {f[0]})\n\n")
    w("| Entry | C symbol | id | Image launched | Parameters passed | Author's note in the menu |\n|---|---|---|---|---|---|\n")
    row(t,0)
orph=[n for n in items if n not in visited]
if orph:
    w("\n### Defined but not reachable from the top row\n\n")
    w("| Entry | C symbol | id | Image launched | Parameters passed | Author's note in the menu |\n|---|---|---|---|---|---|\n")
    for n in orph: row(n,0)
