// The "new menu": a plain vertical list of categories that you drill down into.
// This file is the logic only (building the tree, navigation, finding programs on the
// card). It has no hardware dependencies, so it can be unit tested on a PC
// (docs/tools/test_new_menu.c). Drawing and file access are in loaderMain.c.
//
// Documentation: docs/menu/07-new-menu.md
//
// Needs the MenuItemNew type from loaderMain.c (or a copy of it in the test) before it.
#ifndef NEW_MENU_LOGIC_H
#define NEW_MENU_LOGIC_H

#include <string.h>

#define NM_MAX_NODES 700
#define NM_MAX_IMAGES 64
#define NM_MAX_ROMS 400
#define NM_ROM_PATH 64
#define NM_MAX_LIST 128
#define NM_MAX_DEPTH 4
#define NM_MAX_TOPS 8
#define NM_ROWS 7
#define NM_MAX_PROGS 48
#define NM_NAME_LEN 40

typedef struct NmNode
{
  char title[36];
  int id;                       // id of the compiled entry (0 = settings, 9999 = PICK A GAME)
  const char *img;              // image to launch (0 for categories)
  const char *param1;
  const char *param2;
  const char *altImg;           // for a ROM: the other emulator's image (Button 2 starts with it)
  struct NmNode *child;         // first child
  struct NmNode *right;         // next sibling
  struct NmNode *parent;
  MenuItemNew *src;             // the compiled entry this came from, 0 for discovered programs
  int tags;                     // NM_TAG_* found in the notes
  int hidden;                   // set from a sidecar (hide = 1)
  int discovered;               // found on the card, not compiled in
  char key[NM_NAME_LEN];        // for discovered programs: image base name, used for the sidecar
} NmNode;

typedef struct
{
  NmNode pool[NM_MAX_NODES];
  int used;
  NmNode *tops[NM_MAX_TOPS];
  int ntops;
} NmTree;

typedef struct
{
  NmTree *tree;
  NmNode *parent[NM_MAX_DEPTH];     // the category we are inside, per depth
  int savedSel[NM_MAX_DEPTH];       // selection to restore when going back
  int savedTop[NM_MAX_DEPTH];
  int depth;
  NmNode *list[NM_MAX_LIST];
  int count;
  int sel;
  int top;
} NmNav;

#define NM_TAG_NOSOUND 1
#define NM_TAG_WIP 2
#define NM_TAG_BROKEN 4

// ---- small text helpers -------------------------------------------------------------

static char nmUpper(char c) { return (c >= 'a' && c <= 'z') ? (char) (c - 32) : c; }
static char nmLower(char c) { return (c >= 'A' && c <= 'Z') ? (char) (c + 32) : c; }

static void nmCopy(char *dst, const char *src, int max)
{
  int i = 0;
  for (; src[i] != 0 && i < max - 1; i++) dst[i] = src[i];
  dst[i] = 0;
}

// "  WEB WARS" -> "WEB WARS" (leading spaces are used for alignment in the old menu)
static void nmCopyTrimmed(char *dst, const char *src, int max)
{
  while (*src == ' ') src++;
  nmCopy(dst, src, max);
  int n = (int) strlen(dst);
  while (n > 0 && dst[n - 1] == ' ') dst[--n] = 0;
}

static int nmContainsNoCase(const char *s, const char *needle)
{
  int nl = (int) strlen(needle);
  for (; *s != 0; s++)
  {
    int i = 0;
    while (i < nl && s[i] != 0 && nmLower(s[i]) == nmLower(needle[i])) i++;
    if (i == nl) return 1;
  }
  return nl == 0;
}

// Caveats in one line of an entry's notes: "NO SOUND", "WORK IN PROGRESS", "NOT WORKING"/"CRASH".
static int nmTagsOfLine(const char *line)
{
  int tags = 0;
  if (nmContainsNoCase(line, "no sound")) tags |= NM_TAG_NOSOUND;
  if (nmContainsNoCase(line, "work in progress")) tags |= NM_TAG_WIP;
  if (nmContainsNoCase(line, "not working") || nmContainsNoCase(line, "crash")) tags |= NM_TAG_BROKEN;
  return tags;
}

// Ids of nodes that the menu adds itself (compiled ids are below 9000, 9999 is PICK A GAME).
#define NM_ID_PROGRAMS 9001
#define NM_ID_ALL_ROMS 9002
#define NM_ID_ROM_FOLDER 9003

// Plain-language help shown under a highlighted category. Only the characters the PiTrex
// vector font draws are used: A-Z, 0-9, space, "." and "!". Returns the number of lines.
static int nmHelp(const NmNode *n, const char **out, int max)
{
  static const char *vectrex[] = {
    "GAMES FOR THE VECTREX EMULATORS.",
    "SPEEDY AND EXACT EACH LIST A FEW GAMES.",
    "ALL ROMS LISTS EVERY GAME ON THE CARD.",
    "DEMOS ARE SMALL VECTREX SHOWS.", 0 };
  static const char *speedy[] = {
    "SPEEDY RUNS GAMES FASTER THAN A REAL VECTREX.",
    "IT SKIPS SOME BIOS WORK TO GO FAST.",
    "A FEW GAMES MISBEHAVE. THEN TRY EXACT.", 0 };
  static const char *exact[] = {
    "EXACT RUNS GAMES THE WAY A REAL VECTREX DOES.",
    "IT IS THE SAFE CHOICE FOR ANY GAME.",
    "PICK A GAME LETS YOU BROWSE THE CARD.", 0 };
  static const char *demos[] = {
    "SMALL SHOWS MADE FOR THE VECTREX.", 0 };
  static const char *allRoms[] = {
    "EVERY GAME FILE FOUND IN THE VECTREX FOLDER",
    "OF THE CARD. THE GAMES UNDER SPEEDY AND",
    "EXACT ARE LISTED HERE TOO. FOLDERS COME FIRST.",
    "FILES NOT IN A FOLDER ARE LISTED AFTER THEM.",
    "4 STARTS WITH EXACT. 2 WITH SPEEDY.", 0 };
  static const char *folder[] = {
    "A FOLDER ON THE CARD UNDER VECTREX.",
    "THE NAME IS THE FOLDER NAME. NOT A GAME TYPE.",
    "4 STARTS A GAME WITH EXACT.",
    "2 STARTS IT WITH SPEEDY.", 0 };
  static const char *arcade[] = {
    "ARCADE GAMES RUN BY THE PITREX EMULATORS.",
    "MOST NEED THEIR ROM FILES ON THE CARD.",
    "ALL AAE GAMES LISTS THE ONES YOU HAVE.", 0 };
  static const char *computer[] = {
    "PROGRAMS AND GAMES WRITTEN FOR THE PITREX.", 0 };
  static const char *video[] = {
    "VIDEO PLAYBACK ON THE VECTREX SCREEN.", 0 };
  static const char *audio[] = {
    "MUSIC PLAYBACK. THE PICTURE DANCES TO IT.", 0 };
  static const char *programs[] = {
    "PROGRAMS FOUND ON THE CARD THAT THE MENU",
    "DOES NOT KNOW BY NAME.", 0 };
  static const char *settings[] = {
    "SCREEN SETTINGS AND CALIBRATION.",
    "4 OR RIGHT OPENS THEM. THEY ARE SAVED IN",
    "VECTREXINTERFACE.INI ON THE CARD.", 0 };
  const char **t = 0;
  switch (n->id)
  {
    case 1: t = vectrex; break;
    case 2: t = arcade; break;
    case 3: t = computer; break;
    case 4: t = video; break;
    case 5: t = audio; break;
    case 102: t = speedy; break;
    case 107: t = exact; break;
    case 108: t = demos; break;
    case NM_ID_PROGRAMS: t = programs; break;
    case NM_ID_ALL_ROMS: t = allRoms; break;
    case NM_ID_ROM_FOLDER: t = folder; break;
    default: if (n->id == 0 && n->img == 0 && n->child == 0) t = settings; break;
  }
  int c = 0;
  for (; t != 0 && t[c] != 0 && c < max; c++) out[c] = t[c];
  return c;
}

// The most serious tag as a short word, "" for none.
static const char *nmTagText(int tags)
{
  if (tags & NM_TAG_BROKEN) return "BROKEN";
  if (tags & NM_TAG_WIP) return "WIP";
  if (tags & NM_TAG_NOSOUND) return "NO SOUND";
  return "";
}

static int nmEqNoCase(const char *a, const char *b)
{
  while (*a != 0 && *b != 0)
  {
    if (nmLower(*a) != nmLower(*b)) return 0;
    a++;
    b++;
  }
  return *a == 0 && *b == 0;
}

// "VECTOR_BURNER" -> "VECTOR BURNER": upper case, '_' and '-' become spaces
static void nmPrettyTitle(const char *base, char *out, int max)
{
  int n = 0;
  for (; base[n] != 0 && n < max - 1; n++)
  {
    char c = base[n];
    out[n] = (c == '_' || c == '-') ? ' ' : nmUpper(c);
  }
  out[n] = 0;
}

// Image path -> base name without folder and ".img", lower case. 0 if there is no image.
static int nmImageBase(const char *img, char *out, int max)
{
  if (img == 0 || *img == 0) return 0;
  const char *base = img;
  for (const char *p = img; *p != 0; p++)
    if (*p == '/') base = p + 1;
  int len = (int) strlen(base);
  if (len > 4 && nmEqNoCase(base + len - 4, ".img")) len -= 4;
  if (len <= 0 || len >= max) return 0;
  for (int i = 0; i < len; i++) out[i] = nmLower(base[i]);
  out[len] = 0;
  return 1;
}

// ---- building the tree from the compiled-in menu ------------------------------------

// The category slots have no name in the compiled data (they are drawn as icons).
static const char *nmFallbackTitle(int id)
{
  switch (id)
  {
    case 0: return "SETTINGS";
    case 1: return "VECTREX";
    case 2: return "ARCADE";
    case 3: return "COMPUTER";
    case 4: return "VIDEO";
    case 5: return "AUDIO";
    case 102: return "SPEEDY VECTREX";
    case 107: return "EXACT VECTREX";
    default: return "?";
  }
}

static NmNode *nmNewNode(NmTree *t)
{
  if (t->used >= NM_MAX_NODES) return 0;
  NmNode *n = &t->pool[t->used++];
  memset(n, 0, sizeof *n);
  return n;
}

static NmNode *nmCloneNode(NmTree *t, MenuItemNew *m, NmNode *parent, int depth)
{
  NmNode *n = nmNewNode(t);
  if (n == 0) return 0;
  n->id = m->id;
  n->src = m;
  n->parent = parent;
  if (m->DISPLAYNAME != 0 && m->DISPLAYNAME[0] != 0)
    nmCopyTrimmed(n->title, m->DISPLAYNAME, (int) sizeof n->title);
  else
    nmCopy(n->title, nmFallbackTitle(m->id), (int) sizeof n->title);
  if (m->img != 0 && m->img[0] != 0)
  {
    n->img = m->img;
    n->param1 = m->param1;
    n->param2 = m->param2;
    for (int i = 0; i < 12 && m->text[i] != 0; i++)       // the notes of an entry
      n->tags |= nmTagsOfLine(m->text[i]);
  }
  if (depth < 4)
  {
    NmNode *prev = 0;
    int guard = 0;
    for (MenuItemNew *c = (MenuItemNew *) m->child; c != 0 && guard < NM_MAX_LIST; c = (MenuItemNew *) c->right, guard++)
    {
      // a category with no image and no children is empty: leave it out
      if ((c->img == 0 || c->img[0] == 0) && c->child == 0) continue;
      NmNode *k = nmCloneNode(t, c, n, depth + 1);
      if (k == 0) break;
      if (prev != 0) prev->right = k; else n->child = k;
      prev = k;
    }
  }
  return n;
}

// roots: the compiled top-level category entries, in the order they should be listed.
static void nmBuildTree(NmTree *t, MenuItemNew *const *roots, int nroots)
{
  memset(t, 0, sizeof *t);
  for (int i = 0; i < nroots && t->ntops < NM_MAX_TOPS; i++)
  {
    NmNode *n = nmCloneNode(t, roots[i], 0, 0);
    if (n != 0) t->tops[t->ntops++] = n;
  }
}

// A category node that is not in the compiled menu (SETTINGS, PROGRAMS).
static NmNode *nmAddTop(NmTree *t, const char *title, int id)
{
  if (t->ntops >= NM_MAX_TOPS) return 0;
  NmNode *n = nmNewNode(t);
  if (n == 0) return 0;
  nmCopy(n->title, title, (int) sizeof n->title);
  n->id = id;
  t->tops[t->ntops++] = n;
  return n;
}

// Every image the compiled menu launches, as lower-case base names.
static int nmCollectImages(NmNode *const *tops, int ntops, char (*out)[NM_NAME_LEN], int max)
{
  int count = 0;
  // walk the pool order via explicit stack to avoid recursion
  NmNode *stack[NM_MAX_NODES];
  int sp = 0;
  for (int i = 0; i < ntops && sp < NM_MAX_NODES; i++) stack[sp++] = tops[i];
  while (sp > 0)
  {
    NmNode *n = stack[--sp];
    char base[NM_NAME_LEN];
    if (nmImageBase(n->img, base, sizeof base))
    {
      int dup = 0;
      for (int k = 0; k < count; k++) if (strcmp(out[k], base) == 0) dup = 1;
      if (!dup && count < max) nmCopy(out[count++], base, NM_NAME_LEN);
    }
    for (NmNode *c = n->child; c != 0 && sp < NM_MAX_NODES; c = c->right) stack[sp++] = c;
  }
  return count;
}

// ---- finding programs on the card ---------------------------------------------------

// Images that are part of the boot chain and never a program to start.
static int nmIsSystemImage(const char *base)
{
  return strcmp(base, "pitrex") == 0 || strcmp(base, "pitrex7") == 0;
}

// files: names found in the image folder (piZero1 or piZero2). Fills out[] with the base
// names of the .img files that the compiled menu does not already launch, sorted.
static int nmDiscover(const char *const *files, int nfiles,
                      char (*known)[NM_NAME_LEN], int nknown,
                      char (*out)[NM_NAME_LEN], int max)
{
  int n = 0;
  for (int f = 0; f < nfiles; f++)
  {
    const char *name = files[f];
    if (name[0] == '.') continue;
    int len = (int) strlen(name);
    if (len <= 4 || !nmEqNoCase(name + len - 4, ".img")) continue;
    char base[NM_NAME_LEN];
    if (!nmImageBase(name, base, sizeof base)) continue;
    if (nmIsSystemImage(base)) continue;
    int skip = 0;
    for (int k = 0; k < nknown; k++) if (strcmp(known[k], base) == 0) skip = 1;
    for (int k = 0; k < n; k++) if (strcmp(out[k], base) == 0) skip = 1;
    if (skip || n >= max) continue;
    nmCopy(out[n++], base, NM_NAME_LEN);
  }
  for (int a = 1; a < n; a++)                   // insertion sort
  {
    char v[NM_NAME_LEN];
    nmCopy(v, out[a], NM_NAME_LEN);
    int b = a - 1;
    while (b >= 0 && strcmp(out[b], v) > 0)
    {
      nmCopy(out[b + 1], out[b], NM_NAME_LEN);
      b--;
    }
    nmCopy(out[b + 1], v, NM_NAME_LEN);
  }
  return n;
}

// Adds the discovered programs as children of node "into". image: e.g. "piZero1/".
// Returns how many were added.
static int nmAddPrograms(NmTree *t, NmNode *into, const char *imageDir,
                         char (*bases)[NM_NAME_LEN], int nbases,
                         char (*imgPaths)[56])
{
  NmNode *prev = 0;
  int added = 0;
  for (int i = 0; i < nbases && i < NM_MAX_PROGS; i++)
  {
    NmNode *n = nmNewNode(t);
    if (n == 0) break;
    nmCopy(imgPaths[i], imageDir, 56);
    int dl = (int) strlen(imgPaths[i]);
    nmCopy(imgPaths[i] + dl, bases[i], 56 - dl);
    dl = (int) strlen(imgPaths[i]);
    nmCopy(imgPaths[i] + dl, ".img", 56 - dl);
    nmPrettyTitle(bases[i], n->title, (int) sizeof n->title);
    nmCopy(n->key, bases[i], NM_NAME_LEN);
    n->img = imgPaths[i];
    n->parent = into;
    n->discovered = 1;
    if (prev != 0) prev->right = n; else into->child = n;
    prev = n;
    added++;
  }
  return added;
}

// ---- Vectrex ROMs found under vectrex/ ------------------------------------------------

static void nmAppendChild(NmNode *parent, NmNode *child)
{
  child->parent = parent;
  child->right = 0;
  if (parent->child == 0) { parent->child = child; return; }
  NmNode *last = parent->child;
  while (last->right != 0) last = last->right;
  last->right = child;
}

// true if the first "len" characters of seg equal word, ignoring case
static int nmSegIs(const char *seg, int len, const char *word)
{
  if ((int) strlen(word) != len) return 0;
  for (int i = 0; i < len; i++) if (nmLower(seg[i]) != nmLower(word[i])) return 0;
  return 1;
}

// A relative path such as "original/BERZERK.BIN" is listed when it is a .bin file and no
// folder on the way is one the loader keeps out of the list (bios, movies, waves) or hidden.
static int nmRomPathOk(const char *path)
{
  int len = (int) strlen(path);
  if (len <= 4 || len >= NM_ROM_PATH) return 0;
  if (!nmEqNoCase(path + len - 4, ".bin")) return 0;
  const char *seg = path;
  for (const char *p = path;; p++)
  {
    if (*p == '/' || *p == 0)
    {
      int sl = (int) (p - seg);
      if (sl == 0 || seg[0] == '.') return 0;
      if (*p == '/')
      {
        if (nmSegIs(seg, sl, "bios") || nmSegIs(seg, sl, "movies") || nmSegIs(seg, sl, "waves")) return 0;
      }
      if (*p == 0) break;
      seg = p + 1;
    }
  }
  return 1;
}

// Folder part of a path ("" for a file directly in vectrex/).
static void nmRomDir(const char *path, char *out, int max)
{
  int last = -1;
  for (int i = 0; path[i] != 0; i++) if (path[i] == '/') last = i;
  int n = last < 0 ? 0 : last;
  if (n > max - 1) n = max - 1;
  memcpy(out, path, (size_t) n);
  out[n] = 0;
}

// "original/MR_BOSTON.BIN" -> "MR BOSTON": file name without ".bin", upper case, '_' and '-' as spaces.
static void nmRomTitle(const char *path, char *out, int max)
{
  const char *name = path;
  for (const char *p = path; *p != 0; p++) if (*p == '/') name = p + 1;
  int len = (int) strlen(name);
  if (len > 4 && nmEqNoCase(name + len - 4, ".bin")) len -= 4;
  int n = 0;
  for (int i = 0; i < len && n < max - 1; i++)
  {
    char c = name[i];
    out[n++] = (c == '_' || c == '-') ? ' ' : nmUpper(c);
  }
  out[n] = 0;
}

// Slug of a ROM path for the sidecar key: "rom." + the same slug rules as menuSidecar.h
// (lower case, runs of other characters become one '_').
static void nmRomKey(const char *path, char *out, int max)
{
  int n = 0;
  const char *pre = "rom.";
  for (int i = 0; pre[i] != 0 && n < max - 1; i++) out[n++] = pre[i];
  int startSlug = n;
  for (const char *p = path; *p != 0 && n < max - 1; p++)
  {
    char c = nmLower(*p);
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) out[n++] = c;
    else if (n > startSlug && out[n - 1] != '_') out[n++] = '_';
  }
  if (n > startSlug && out[n - 1] == '_') n--;
  out[n] = 0;
}

static int nmTitleCmp(const char *a, const char *b)
{
  while (*a != 0 && *b != 0 && nmLower(*a) == nmLower(*b)) { a++; b++; }
  return (unsigned char) nmLower(*a) - (unsigned char) nmLower(*b);
}

// Builds the tree under "into": one category node per folder (alphabetical, listed first),
// then the files that sit directly in vectrex/. Entries are sorted by title. Every entry
// starts imgDefault and passes its path as the parameter. Returns how many ROMs were added.
// paths must stay valid: the nodes point into it.
static int nmAddRoms(NmTree *t, NmNode *into, char (*paths)[NM_ROM_PATH], int npaths,
                     const char *imgDefault, const char *imgAlt)
{
  int order[NM_MAX_ROMS];
  int n = 0;
  for (int i = 0; i < npaths && n < NM_MAX_ROMS; i++)
  {
    if (!nmRomPathOk(paths[i])) continue;
    if (strchr(paths[i], '/') == 0)             // a loose file that also sits in a folder is a copy: list it once
    {
      int copy = 0;
      for (int k = 0; k < npaths && !copy; k++)
      {
        const char *slash = strrchr(paths[k], '/');
        if (slash != 0 && nmRomPathOk(paths[k]) && nmEqNoCase(slash + 1, paths[i])) copy = 1;
      }
      if (copy) continue;
    }
    order[n++] = i;
  }

  // sort by folder, then by title
  for (int a = 1; a < n; a++)
  {
    int v = order[a];
    int b = a - 1;
    while (b >= 0)
    {
      char da[NM_ROM_PATH], dv[NM_ROM_PATH], ta[40], tv[40];
      nmRomDir(paths[order[b]], da, sizeof da);
      nmRomDir(paths[v], dv, sizeof dv);
      nmRomTitle(paths[order[b]], ta, sizeof ta);
      nmRomTitle(paths[v], tv, sizeof tv);
      int c = nmTitleCmp(da, dv);
      if (c == 0) c = nmTitleCmp(ta, tv);
      if (c <= 0) break;
      order[b + 1] = order[b];
      b--;
    }
    order[b + 1] = v;
  }

  int added = 0;
  // folders first: all entries with a folder, in order; then the root files
  for (int pass = 0; pass < 2; pass++)
  {
    NmNode *group = 0;
    char groupDir[NM_ROM_PATH];
    groupDir[0] = 0;
    for (int k = 0; k < n; k++)
    {
      char dir[NM_ROM_PATH];
      nmRomDir(paths[order[k]], dir, sizeof dir);
      int isRoot = dir[0] == 0;
      if ((pass == 0) == isRoot) continue;
      NmNode *parent = into;
      if (!isRoot)
      {
        if (group == 0 || strcmp(groupDir, dir) != 0)
        {
          group = nmNewNode(t);
          if (group == 0) return added;
          group->id = NM_ID_ROM_FOLDER;
          nmPrettyTitle(dir, group->title, (int) sizeof group->title - 8);
          for (int i = 0; group->title[i] != 0; i++) if (group->title[i] == '/') group->title[i] = '>';
          strcat(group->title, " FOLDER");               // a folder of the card, not a kind of game
          nmRomKey(dir, group->key, (int) sizeof group->key);
          memcpy(group->key, "dir.", 4);                 // sidecar menu/dir.<folder>.ini can rename it
          nmAppendChild(into, group);
          nmCopy(groupDir, dir, sizeof groupDir);
        }
        parent = group;
      }
      NmNode *e = nmNewNode(t);
      if (e == 0) return added;
      nmRomTitle(paths[order[k]], e->title, (int) sizeof e->title);
      nmRomKey(paths[order[k]], e->key, (int) sizeof e->key);
      e->img = imgDefault;
      e->altImg = imgAlt;
      e->param1 = paths[order[k]];
      e->discovered = 1;
      nmAppendChild(parent, e);
      added++;
    }
  }
  return added;
}

// Sorts the children of p by title (case-insensitive, stable).
static void nmSortChildren(NmNode *p)
{
  NmNode *sorted = 0;
  NmNode *c = p->child;
  while (c != 0)
  {
    NmNode *next = c->right;
    c->right = 0;
    if (sorted == 0 || nmTitleCmp(c->title, sorted->title) < 0)
    {
      c->right = sorted;
      sorted = c;
    }
    else
    {
      NmNode *q = sorted;
      while (q->right != 0 && nmTitleCmp(c->title, q->right->title) >= 0) q = q->right;
      c->right = q->right;
      q->right = c;
    }
    c = next;
  }
  p->child = sorted;
}

// Adds the ROMs of one folder (prefix "demos/", files directly inside it) to a compiled group
// when the group does not already list them (same parameter, ignoring case).
static int nmAddMissingRoms(NmTree *t, NmNode *into, char (*paths)[NM_ROM_PATH], int npaths,
                            const char *prefix, const char *imgDefault, const char *imgAlt)
{
  int added = 0;
  int plen = (int) strlen(prefix);
  for (int i = 0; i < npaths; i++)
  {
    const char *p = paths[i];
    if ((int) strlen(p) <= plen) continue;
    int same = 1;
    for (int k = 0; k < plen; k++) if (nmLower(p[k]) != nmLower(prefix[k])) same = 0;
    if (!same || strchr(p + plen, '/') != 0 || !nmRomPathOk(p)) continue;
    int have = 0;
    for (NmNode *c = into->child; c != 0; c = c->right)
      if (c->param1 != 0 && nmEqNoCase(c->param1, p)) have = 1;
    if (have) continue;
    NmNode *e = nmNewNode(t);
    if (e == 0) return added;
    nmRomTitle(p, e->title, (int) sizeof e->title);
    nmRomKey(p, e->key, (int) sizeof e->key);
    e->img = imgDefault;
    e->altImg = imgAlt;
    e->param1 = paths[i];
    e->discovered = 1;
    nmAppendChild(into, e);
    added++;
  }
  return added;
}

// The first node with this id, searching the whole tree (0 if none).
static NmNode *nmFindById(NmNode *n, int id)
{
  for (; n != 0; n = n->right)
  {
    if (n->id == id) return n;
    NmNode *f = nmFindById(n->child, id);
    if (f != 0) return f;
  }
  return 0;
}

// Button 2 on a ROM: start it with the other emulator. Returns the image to use.
static const char *nmOtherEmulator(const NmNode *n)
{
  return n->altImg != 0 ? n->altImg : n->img;
}

// sidecar "emulator = speedy|exact": make the entry start with the one named
static void nmChooseEmulator(NmNode *n, char which)
{
  if (n->altImg == 0 || (which != 's' && which != 'e')) return;
  int isSpeedy = strstr(n->img, "speedy") != 0;
  if ((which == 's') != isSpeedy)
  {
    const char *t = n->img;
    n->img = n->altImg;
    n->altImg = t;
  }
}

// ---- navigation ---------------------------------------------------------------------

static void nmFixWindow(NmNav *n)
{
  if (n->count == 0) { n->sel = 0; n->top = 0; return; }
  if (n->sel >= n->count) n->sel = n->count - 1;
  if (n->sel < 0) n->sel = 0;
  if (n->sel < n->top) n->top = n->sel;
  if (n->sel >= n->top + NM_ROWS) n->top = n->sel - NM_ROWS + 1;
  if (n->top < 0) n->top = 0;
}

// Fills list[] for the current depth, leaving out hidden nodes.
static void nmRebuild(NmNav *n)
{
  n->count = 0;
  if (n->depth == 0)
  {
    for (int i = 0; i < n->tree->ntops && n->count < NM_MAX_LIST; i++)
      if (!n->tree->tops[i]->hidden) n->list[n->count++] = n->tree->tops[i];
  }
  else
  {
    NmNode *p = n->parent[n->depth - 1];
    for (NmNode *c = p->child; c != 0 && n->count < NM_MAX_LIST; c = c->right)
      if (!c->hidden) n->list[n->count++] = c;
  }
  nmFixWindow(n);
}

static void nmInit(NmNav *n, NmTree *t)
{
  memset(n, 0, sizeof *n);
  n->tree = t;
  nmRebuild(n);
}

static NmNode *nmCurrent(NmNav *n)
{
  return (n->count > 0) ? n->list[n->sel] : 0;
}

static void nmMove(NmNav *n, int dir)
{
  if (n->count == 0) return;
  n->sel += dir;
  if (n->sel < 0) n->sel = 0;
  if (n->sel >= n->count) n->sel = n->count - 1;
  nmFixWindow(n);
}

static int nmHasVisibleChild(NmNode *c)
{
  for (c = c->child; c != 0; c = c->right) if (!c->hidden) return 1;
  return 0;
}

// Enter the selected category, or return the selected entry so the caller can start it.
// Returns 0 when it went into a category (or there is nothing selected).
static NmNode *nmEnter(NmNav *n)
{
  NmNode *c = nmCurrent(n);
  if (c == 0) return 0;
  if (nmHasVisibleChild(c))
  {
    if (n->depth >= NM_MAX_DEPTH) return 0;
    n->parent[n->depth] = c;
    n->savedSel[n->depth] = n->sel;
    n->savedTop[n->depth] = n->top;
    n->depth++;
    n->sel = 0;
    n->top = 0;
    nmRebuild(n);
    return 0;
  }
  return c;
}

// Go up one level. Returns 1 if it moved.
static int nmBack(NmNav *n)
{
  if (n->depth == 0) return 0;
  n->depth--;
  n->sel = n->savedSel[n->depth];
  n->top = n->savedTop[n->depth];
  nmRebuild(n);
  return 1;
}

// "ARCADE > ATARI" for the current level (the PiTrex font draws ">" as an arrow; it has no "/")
static void nmPath(NmNav *n, char *out, int max)
{
  out[0] = 0;
  int len = 0;
  for (int d = 0; d < n->depth; d++)
  {
    if (d > 0 && len + 3 < max) { out[len++] = ' '; out[len++] = '>'; out[len++] = ' '; out[len] = 0; }
    const char *t = n->parent[d]->title;
    for (int i = 0; t[i] != 0 && len < max - 1; i++) out[len++] = t[i];
    out[len] = 0;
  }
}

#endif
