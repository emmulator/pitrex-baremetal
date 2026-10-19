// The "new menu": a plain vertical list of categories that you drill down into.
// This file is the logic only (building the tree, navigation, finding programs on the
// card). It has no hardware dependencies, so it can be unit tested on a PC
// (docs/tools/test_new_menu.c). Drawing and file access are in loaderMain.c.
//
// Documentation: docs/menu/07-new-menu.md
//
// The menu has nothing compiled in: it is built from the entry files in menu/ on the card, the
// programs found in the image folder and the ROMs found under vectrex/.
#ifndef NEW_MENU_LOGIC_H
#define NEW_MENU_LOGIC_H

#include <string.h>
#include "menuSidecar.h"
#include "menuFile.h"

#define NM_MAX_NODES 700
#define NM_MAX_IMAGES 200
#define NM_MAX_ROMS 400
#define NM_ROM_PATH 64
#define NM_MAX_LIST 128
#define NM_MAX_DEPTH 4
#define NM_MAX_TOPS 16
#define NM_ROWS 7
#define NM_MAX_PROGS 48
#define NM_NAME_LEN 40
#ifndef NM_KEY_LEN
#define NM_KEY_LEN 64
#endif
#define NM_MAX_ENTRIES 160

typedef struct NmNode
{
  char title[36];
  int id;                       // NM_ID_* for nodes the menu adds itself, else 0
  const char *img;              // image to launch (0 for categories); points at imgBuf or a constant
  const char *param1;
  const char *param2;
  const char *altImg;           // for a ROM: the other emulator's image (Button 2 starts with it)
  struct NmNode *child;         // first child
  struct NmNode *right;         // next sibling
  struct NmNode *parent;
  int tags;                     // NM_TAG_* found in the notes
  int hidden;                   // hide = 1 in a sidecar, or the entry's image is not on the card
  int discovered;               // found on the card (a ROM or a program), not described by an entry file
  int keepOrder;                // a category with "order = listed": its entries are not sorted
  char key[NM_KEY_LEN];         // name of the sidecar file (without .ini): notes, controls and the entry itself
  char imgBuf[48];              // the entry's image path, e.g. piZero1/swsbt.img
  char p1[40];                  // the entry's parameters (a parameter can be one raw byte)
  char p2[48];
  char needs[56];               // a file the game needs; "ROM NOT FOUND!" is shown when it is missing
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

// Ids of nodes that the menu adds itself.
#define NM_ID_SETTINGS 9004
#define NM_ID_PROGRAMS 9001
#define NM_ID_ALL_ROMS 9002
#define NM_ID_ROM_FOLDER 9003

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

// Plain-language help shown under a highlighted category. Only the characters the PiTrex
// vector font draws are used: A-Z, 0-9, space, "." and "!". Returns the number of lines.
static int nmHelp(const NmNode *n, const char **out, int max)
{
  static const char *vectrex[] = {
    "GAMES FOR THE VECTREX EMULATORS.",
    "SPEEDY AND EXACT LIST GAMES SET UP FOR THEM.",
    "ALL ROMS LISTS EVERY GAME ON THE CARD.",
    "DEMOS ARE SMALL VECTREX SHOWS.", 0 };
  static const char *speedy[] = {
    "SPEEDY RUNS GAMES FASTER THAN A REAL VECTREX.",
    "IT SKIPS SOME BIOS WORK TO GO FAST.",
    "A FEW GAMES MISBEHAVE. THEN TRY EXACT.", 0 };
  static const char *exact[] = {
    "EXACT RUNS GAMES THE WAY A REAL VECTREX DOES.",
    "IT IS THE SAFE CHOICE FOR ANY GAME.",
    "ALL ROMS LISTS EVERY GAME ON THE CARD.", 0 };
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
  static const char *pitrexOwn[] = {
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
    case NM_ID_PROGRAMS: t = programs; break;
    case NM_ID_ALL_ROMS: t = allRoms; break;
    case NM_ID_ROM_FOLDER: t = folder; break;
    case NM_ID_SETTINGS: t = settings; break;
    default:
      if (n->img != 0) break;                    // a game: its own notes are shown instead
      if (nmEqNoCase(n->title, "VECTREX")) t = vectrex;
      else if (nmEqNoCase(n->title, "ARCADE")) t = arcade;
      else if (nmEqNoCase(n->title, "PITREX") || nmEqNoCase(n->title, "COMPUTER")) t = pitrexOwn;   // COMPUTER: the old name
      else if (nmEqNoCase(n->title, "VIDEO")) t = video;
      else if (nmEqNoCase(n->title, "AUDIO")) t = audio;
      else if (nmEqNoCase(n->title, "SPEEDY VECTREX")) t = speedy;
      else if (nmEqNoCase(n->title, "EXACT VECTREX")) t = exact;
      else if (nmEqNoCase(n->title, "DEMOS")) t = demos;
      break;
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

// Upper case, brackets dropped, any other character the PiTrex font cannot draw becomes a space,
// double spaces collapse, a trailing "SBT" is dropped: "Star Wars (SBT)" -> "STAR WARS".
static void nmCleanTitle(char *t)
{
  int o = 0;
  for (int i = 0; t[i] != 0; i++)
  {
    char c = nmUpper(t[i]);
    if (c == '(' || c == ')' || c == '[' || c == ']') continue;
    if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '!')) c = ' ';
    if (c == ' ' && (o == 0 || t[o - 1] == ' ')) continue;
    t[o++] = c;
  }
  while (o > 0 && t[o - 1] == ' ') o--;
  t[o] = 0;
  // "SBT" (static binary translation) is how a game was ported, not part of its name; several are
  if (o > 4 && strcmp(t + o - 4, " SBT") == 0) t[o - 4] = 0;
}

static NmNode *nmNewNode(NmTree *t)
{
  if (t->used >= NM_MAX_NODES) return 0;
  NmNode *n = &t->pool[t->used++];
  memset(n, 0, sizeof *n);
  return n;
}

// A top-level category node.
static NmNode *nmAddTop(NmTree *t, const char *title, int id)
{
  if (t->ntops >= NM_MAX_TOPS) return 0;
  NmNode *n = nmNewNode(t);
  if (n == 0) return 0;
  nmCopy(n->title, title, (int) sizeof n->title);
  nmSlugKey("cat.", title, n->key, NM_KEY_LEN);
  n->id = id;
  t->tops[t->ntops++] = n;
  return n;
}

// Every image any node of the tree starts, as lower-case base names.
static int nmCollectImages(const NmTree *t, char (*out)[NM_NAME_LEN], int max)
{
  int count = 0;
  for (int i = 0; i < t->used; i++)
  {
    char base[NM_NAME_LEN];
    if (!nmImageBase(t->pool[i].img, base, sizeof base)) continue;
    int dup = 0;
    for (int k = 0; k < count; k++) if (strcmp(out[k], base) == 0) dup = 1;
    if (!dup && count < max) nmCopy(out[count++], base, NM_NAME_LEN);
  }
  return count;
}

// ---- finding programs on the card ---------------------------------------------------

// Images that are part of the boot chain or the Vectrex ROM engines (started through ALL ROMS),
// never listed as a program.
static int nmIsSystemImage(const char *base)
{
  return strcmp(base, "pitrex") == 0 || strcmp(base, "pitrex7") == 0 ||
         strcmp(base, "vectrexexact") == 0 || strcmp(base, "vectrexspeedy") == 0;   // the engines that start ROMs
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

// A Vectrex game file: .bin, or .vec (the same raw cartridge image under another name)
static int nmIsRomName(const char *name, int len)
{
  return len > 4 && (nmEqNoCase(name + len - 4, ".bin") || nmEqNoCase(name + len - 4, ".vec"));
}

// A relative path such as "original/BERZERK.BIN" is listed when it is a .bin or .vec file and no
// folder on the way is one the loader keeps out of the list (bios, movies, waves) or hidden.
static int nmRomPathOk(const char *path)
{
  int len = (int) strlen(path);
  if (len <= 4 || len >= NM_ROM_PATH) return 0;
  if (!nmIsRomName(path, len)) return 0;
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

// "original/MR_BOSTON.BIN" -> "MR BOSTON": file name without its extension, upper case, '_' and '-' as spaces.
static void nmRomTitle(const char *path, char *out, int max)
{
  const char *name = path;
  for (const char *p = path; *p != 0; p++) if (*p == '/') name = p + 1;
  int len = (int) strlen(name);
  const char *dot = strrchr(name, '.');
  if (dot != 0 && dot > name) len = (int) (dot - name);
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

// "E-SRG.bin" and "e-srg.VEC" are the same game: names equal without their extensions, ignoring case
static int nmSameBase(const char *a, const char *b)
{
  const char *da = strrchr(a, '.'), *db = strrchr(b, '.');
  int la = da ? (int) (da - a) : (int) strlen(a), lb = db ? (int) (db - b) : (int) strlen(b);
  if (la != lb) return 0;
  for (int i = 0; i < la; i++) if (nmLower(a[i]) != nmLower(b[i])) return 0;
  return 1;
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
        if (slash != 0 && nmRomPathOk(paths[k]) && nmSameBase(slash + 1, paths[i])) copy = 1;
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
      nmCleanTitle(e->title);
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
    nmCleanTitle(e->title);
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

// Sorts the children of every category below the top level by title, except inside ALL ROMS
// (its folders come first, then the loose files).
static void nmSortAll(NmNode *n)
{
  for (; n != 0; n = n->right)
  {
    if (n->id == NM_ID_ALL_ROMS) continue;
    if (n->child != 0)
    {
      if (!n->keepOrder) nmSortChildren(n);
      nmSortAll(n->child);
    }
  }
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

// ---- building the tree from the entry files ------------------------------------------

static NmNode *nmFindChildByTitle(NmNode *p, const char *title)
{
  for (NmNode *c = p->child; c != 0; c = c->right)
    if (nmEqNoCase(c->title, title)) return c;
  return 0;
}

static NmNode *nmFindTop(NmTree *t, const char *title)
{
  for (int i = 0; i < t->ntops; i++)
    if (nmEqNoCase(t->tops[i]->title, title)) return t->tops[i];
  return 0;
}

// The category for a path such as "ARCADE / ATARI": found or created, with its parents. 0 if the
// tree is full.
static NmNode *nmCategory(NmTree *t, const char *path)
{
  NmNode *cur = 0;
  const char *p = path;
  char sofar[NM_KEY_LEN];                     // "ARCADE / ATARI": the key of the category is cat.arcade_atari
  sofar[0] = 0;
  while (*p != 0)
  {
    char seg[36];
    int n = 0;
    while (*p != 0 && *p != '/') { if (n < 35) seg[n++] = *p; p++; }
    if (*p == '/') p++;
    seg[n] = 0;
    nmCleanTitle(seg);
    if (seg[0] == 0) continue;
    if (sofar[0] != 0) strncat(sofar, " ", sizeof sofar - strlen(sofar) - 1);
    strncat(sofar, seg, sizeof sofar - strlen(sofar) - 1);
    NmNode *next = cur != 0 ? nmFindChildByTitle(cur, seg) : nmFindTop(t, seg);
    if (next == 0)
    {
      if (cur == 0 && t->ntops >= NM_MAX_TOPS) return 0;
      next = nmNewNode(t);
      if (next == 0) return cur;
      nmCopy(next->title, seg, (int) sizeof next->title);
      nmSlugKey("cat.", sofar, next->key, NM_KEY_LEN);
      if (cur != 0) nmAppendChild(cur, next);
      else t->tops[t->ntops++] = next;
    }
    cur = next;
  }
  return cur;
}

// PROGRAMS: entries without a category and programs found on the card.
static NmNode *nmProgramsNode(NmTree *t)
{
  for (int i = 0; i < t->ntops; i++)
    if (t->tops[i]->id == NM_ID_PROGRAMS) return t->tops[i];
  return nmAddTop(t, "PROGRAMS", NM_ID_PROGRAMS);
}

static int nmParamEq(const char *a, const char *b)
{
  if (a == 0 || a[0] == 0) return b == 0 || b[0] == 0;
  return b != 0 && nmEqNoCase(a, b);
}

// A child of parent that starts the same image (prefix + image) with the same parameters, or 0. Two files that
// describe the same game (an upgraded card with old entry files, a folder rule and an entry) give one entry.
static NmNode *nmFindSame(NmNode *parent, const char *prefix, const char *image, const char *p1, const char *p2)
{
  char img[48];
  nmCopy(img, prefix, (int) sizeof img);
  int pl = (int) strlen(img);
  nmCopy(img + pl, image, (int) sizeof img - pl);
  for (NmNode *c = parent->child; c != 0; c = c->right)
    if (c->img != 0 && nmEqNoCase(c->img, img) && nmParamEq(c->param1, p1) && nmParamEq(c->param2, p2)) return c;
  return 0;
}

// Adds the game that an entry file describes. key = file name without ".ini", prefix = image folder
// ("piZero1/"). Returns the node, or 0 if the file has no image.
static NmNode *nmAddEntry(NmTree *t, const char *key, const SidecarText *s, const char *prefix)
{
  if (s->image[0] == 0) return 0;
  NmNode *parent = s->category[0] != 0 ? nmCategory(t, s->category) : 0;
  if (parent == 0) parent = nmProgramsNode(t);
  if (parent == 0) return 0;
  if (nmFindSame(parent, prefix, s->image, s->p1, s->p2) != 0) return 0;   // the same game twice in one category
  NmNode *n = nmNewNode(t);
  if (n == 0) return 0;
  if (s->title[0] != 0) nmCopy(n->title, s->title, (int) sizeof n->title);
  else { char base[NM_NAME_LEN]; if (nmImageBase(s->image, base, sizeof base)) nmPrettyTitle(base, n->title, (int) sizeof n->title); }
  nmCleanTitle(n->title);
  nmCopy(n->key, key, (int) sizeof n->key);
  nmCopy(n->imgBuf, prefix, (int) sizeof n->imgBuf);
  int pl = (int) strlen(n->imgBuf);
  nmCopy(n->imgBuf + pl, s->image, (int) sizeof n->imgBuf - pl);
  n->img = n->imgBuf;
  if (s->p1[0] != 0) { memcpy(n->p1, s->p1, sizeof n->p1); n->param1 = n->p1; }
  if (s->p2[0] != 0) { memcpy(n->p2, s->p2, sizeof n->p2); n->param2 = n->p2; }
  nmCopy(n->needs, s->needs, (int) sizeof n->needs);
  for (int r = 0; r < s->rows; r++) n->tags |= nmTagsOfLine(s->row[r]);
  n->hidden = s->hide;
  nmAppendChild(parent, n);
  return n;
}

typedef int (*NmReadFn)(const char *key, SidecarText *t);

// Reads every sidecar file in names[] (keys, no ".ini") and adds those that describe a game.
static int nmAddEntries(NmTree *t, char (*names)[NM_KEY_LEN], int nnames, NmReadFn rd, const char *prefix)
{
  SidecarText st;
  int added = 0;
  for (int i = 0; i < nnames; i++)
    if (rd(names[i], &st) && st.image[0] != 0 && nmAddEntry(t, names[i], &st, prefix) != 0) added++;
  return added;
}

typedef int (*NmExistsFn)(const char *path);

// An entry whose "needs" file (a ROM zip) is not on the card cannot be played: hide it, so the menu lists what
// the card can run. It comes back by itself when the file is added. A needs that ends in "/" (the AAE picker:
// "roms/aae/") means "any other entry that needs a file in this folder is still showing".
static void nmHideMissingNeeds(NmTree *t, NmExistsFn exists)
{
  if (exists == 0) return;
  for (int i = 0; i < t->used; i++)
  {
    NmNode *n = &t->pool[i];
    size_t l = strlen(n->needs);
    if (l == 0 || n->needs[l - 1] == '/' || n->hidden) continue;
    if (!exists(n->needs)) n->hidden = 1;
  }
  for (int i = 0; i < t->used; i++)
  {
    NmNode *n = &t->pool[i];
    size_t l = strlen(n->needs);
    if (l == 0 || n->needs[l - 1] != '/' || n->hidden) continue;
    int any = 0;
    for (int k = 0; k < t->used && !any; k++)
    {
      const NmNode *o = &t->pool[k];
      if (o != n && !o->hidden && strncmp(o->needs, n->needs, l) == 0 && o->needs[l] != 0) any = 1;
    }
    if (!any) n->hidden = 1;
  }
}

// An entry whose image is not in the image folder cannot start: hide it. files = names in that folder.
static void nmHideMissingImages(NmTree *t, const char *const *files, int nfiles)
{
  if (nfiles <= 0) return;                       // listing failed: leave everything as it is
  for (int i = 0; i < t->used; i++)
  {
    NmNode *n = &t->pool[i];
    if (n->imgBuf[0] == 0) continue;
    const char *name = strrchr(n->imgBuf, '/');
    name = name != 0 ? name + 1 : n->imgBuf;
    int found = 0;
    for (int f = 0; f < nfiles && !found; f++) if (nmEqNoCase(files[f], name)) found = 1;
    if (!found) n->hidden = 1;
  }
}

static int nmTopRank(const NmNode *n)
{
  return n->id == NM_ID_SETTINGS ? 2 : (n->id == NM_ID_PROGRAMS ? 1 : 0);
}

// Top level: the categories alphabetically, then PROGRAMS, then SETTINGS.
static void nmSortTops(NmTree *t)
{
  for (int a = 1; a < t->ntops; a++)
  {
    NmNode *v = t->tops[a];
    int b = a - 1;
    while (b >= 0 && (nmTopRank(t->tops[b]) > nmTopRank(v) ||
                      (nmTopRank(t->tops[b]) == nmTopRank(v) && nmTitleCmp(t->tops[b]->title, v->title) > 0)))
    {
      t->tops[b + 1] = t->tops[b];
      b--;
    }
    t->tops[b + 1] = v;
  }
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

// A category with nothing visible in it is hidden too (all its games hidden or missing).
static void nmHideEmpty(NmNode *n)
{
  for (; n != 0; n = n->right)
  {
    nmHideEmpty(n->child);
    if (n->img == 0 && n->id != NM_ID_SETTINGS && !nmHasVisibleChild(n)) n->hidden = 1;
  }
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

static void nmUnlink(NmNode *n)
{
  NmNode *p = n->parent;
  if (p == 0) return;
  if (p->child == n) p->child = n->right;
  else for (NmNode *c = p->child; c != 0; c = c->right) if (c->right == n) { c->right = n->right; break; }
  n->right = 0;
  n->parent = 0;
}

// What the menu file says about what was found on the card (ROMs, programs, ROM folders, folder-rule files) and
// about categories. Entries were applied when they were read.
//   ROM with "category": also listed there (it stays in ALL ROMS)    program with "category": moved there
//   category: hide, order = listed
static void nmApplySidecars(NmTree *t, NmReadFn rd)
{
  SidecarText st;
  int end = t->used;                          // copies made below are not looked at again
  for (int i = 0; i < end; i++)
  {
    NmNode *n = &t->pool[i];
    if (!n->discovered && n->id != NM_ID_ROM_FOLDER) continue;
    if (n->key[0] == 0 || !rd(n->key, &st)) continue;
    if (n->id == NM_ID_ROM_FOLDER)
    {
      if (st.title[0] != 0) { nmCopy(n->title, st.title, (int) sizeof n->title); nmCleanTitle(n->title); }
      if (st.hide) n->hidden = 1;
      continue;
    }
    if (st.hide) n->hidden = 1;
    if (st.found)                               // a sidecar's notes carry their own caveats
    {
      n->tags = 0;
      for (int r = 0; r < st.rows; r++) n->tags |= nmTagsOfLine(st.row[r]);
    }
    if (st.title[0] != 0) { nmCopy(n->title, st.title, (int) sizeof n->title); nmCleanTitle(n->title); }
    if (st.emulator != 0) nmChooseEmulator(n, st.emulator);
    if (st.category[0] == 0) continue;
    NmNode *to = nmCategory(t, st.category);
    if (to == 0 || to == n->parent) continue;
    if (n->altImg != 0)                         // a ROM: a copy in the category
    {
      if (nmFindSame(to, "", n->img, n->param1, n->param2) != 0) continue;
      NmNode *c = nmNewNode(t);
      if (c == 0) continue;
      *c = *n;
      c->child = 0;
      nmAppendChild(to, c);
    }
    else if (n->parent != 0 && n->parent->id == NM_ID_PROGRAMS)
    {
      nmUnlink(n);
      nmAppendChild(to, n);
    }
  }
  for (int i = 0; i < t->used; i++)           // categories, including those made just above
  {
    NmNode *n = &t->pool[i];
    if (n->img != 0 || strncmp(n->key, "cat.", 4) != 0 || !rd(n->key, &st)) continue;
    if (st.hide) n->hidden = 1;
    if (st.order) n->keepOrder = 1;
  }
}

// A folder rule (menu file section with folder = and image =): every file in the folder becomes an entry.
// files: the names in that folder (no sub-folders). A file that an entry already starts is not added twice.
// Returns how many entries were added.
static int nmAddFolderFiles(NmTree *t, NmRecs *recs, const NmRec *rule, const char *prefix,
                            const char *const *files, int nfiles, NmReadFn rd)
{
  int added = 0;
  NmNode *parent = rule->category[0] != 0 ? nmCategory(t, rule->category) : 0;
  if (parent == 0) parent = nmProgramsNode(t);
  if (parent == 0) return 0;
  SidecarText st;
  for (int f = 0; f < nfiles; f++)
  {
    const char *name = files[f];
    if (name[0] == '.' || !nmExtOk(name, rule->files)) continue;
    char p1[48], p2[48], path[NM_REC_PATH + 40], key[NM_KEY_LEN];
    nmSubst(p1, (int) sizeof p1, rule->p1, rule->path, name);
    nmSubst(p2, (int) sizeof p2, rule->p2, rule->path, name);
    if (nmFindSame(parent, prefix, rule->image, p1, p2) != 0) continue;
    nmCopy(path, rule->path, (int) sizeof path);
    strncat(path, "/", sizeof path - strlen(path) - 1);
    strncat(path, name, sizeof path - strlen(path) - 1);
    nmPathKey(path, key, NM_KEY_LEN);
    int have = rd(key, &st);
    NmNode *n = nmNewNode(t);
    if (n == 0) break;
    if (have && st.title[0] != 0) nmCopy(n->title, st.title, (int) sizeof n->title);
    else nmRomTitle(name, n->title, (int) sizeof n->title);
    nmCleanTitle(n->title);
    nmCopy(n->key, key, (int) sizeof n->key);
    nmCopy(n->imgBuf, prefix, (int) sizeof n->imgBuf);
    int pl = (int) strlen(n->imgBuf);
    nmCopy(n->imgBuf + pl, rule->image, (int) sizeof n->imgBuf - pl);
    n->img = n->imgBuf;
    if (p1[0] != 0) { nmCopy(n->p1, p1, (int) sizeof n->p1); n->param1 = n->p1; }
    if (p2[0] != 0) { nmCopy(n->p2, p2, (int) sizeof n->p2); n->param2 = n->p2; }
    if (have)
    {
      for (int r = 0; r < st.rows; r++) n->tags |= nmTagsOfLine(st.row[r]);
      n->hidden = st.hide;
    }
    nmAppendChild(parent, n);
    added++;
  }
  (void) recs;
  return added;
}

// The entries of the menu file(s), in the order they were read. Returns how many were added.
static int nmAddRecEntries(NmTree *t, NmRecs *recs, const char *prefix)
{
  int added = 0;
  SidecarText st;
  for (int i = 0; i < recs->n; i++)
  {
    if (recs->rec[i].kind != NM_REC_ENTRY) continue;
    nmRecToSidecar(recs, recs->rec[i].key, &st);
    if (nmAddEntry(t, recs->rec[i].key, &st, prefix) != 0) added++;
  }
  return added;
}

// The built-in folder rules: videos and songs on Malban's card, used when the menu file has no rule for the folder.
static int nmDefaultFolderRules(const NmRecs *recs, NmRec *out, int max)
{
  static const char *def[2][3] = { { "vectrex/movies", "VIDEO", "vectrexvideo.img" },
                                   { "vectrex/waves", "AUDIO", "vectrexaudio.img" } };
  int n = 0;
  for (int d = 0; d < 2 && n < max; d++)
  {
    int have = 0;
    for (int i = 0; i < recs->n; i++)
      if (recs->rec[i].kind == NM_REC_FOLDER && nmEqNoCase(recs->rec[i].path, def[d][0])) have = 1;
    if (have) continue;
    NmRec *r = &out[n++];
    memset(r, 0, sizeof *r);
    r->kind = NM_REC_FOLDER;
    nmCopy(r->path, def[d][0], (int) sizeof r->path);
    nmCopy(r->category, def[d][1], (int) sizeof r->category);
    nmCopy(r->image, def[d][2], (int) sizeof r->image);
    nmCopy(r->p1, "{dir}", (int) sizeof r->p1);
    nmCopy(r->p2, "{file}", (int) sizeof r->p2);
  }
  return n;
}

typedef struct
{
  char known[NM_MAX_IMAGES][NM_NAME_LEN];
  char found[NM_MAX_PROGS][NM_NAME_LEN];
  char paths[NM_MAX_PROGS][56];
} NmScratch;

// The second half of building the tree, after the entry files have been added with nmAddEntries():
// hides entries with no image, finds programs and ROMs, sorts. Returns the SETTINGS node.
static NmNode *nmAssembleRest(NmTree *t, NmScratch *sc, NmReadFn rd, const char *prefix,
                              const char *const *files, int nfiles, char (*romPaths)[NM_ROM_PATH], int nroms,
                              const char *exactImg, const char *speedyImg, NmExistsFn exists, int *nfound)
{
  nmHideMissingImages(t, files, nfiles);
  nmHideMissingNeeds(t, exists);

  int nknown = nmCollectImages(t, sc->known, NM_MAX_IMAGES);
  *nfound = nmDiscover(files, nfiles, sc->known, nknown, sc->found, NM_MAX_PROGS);
  if (*nfound > 0)
  {
    NmNode *programs = nmProgramsNode(t);
    if (programs != 0) nmAddPrograms(t, programs, prefix, sc->found, *nfound, sc->paths);
  }

  if (nroms > 0)                                // VECTREX / ALL ROMS: Exact starts a game, Button 2 starts Speedy
  {
    NmNode *vectrex = nmCategory(t, "VECTREX");
    NmNode *romsNode = nmNewNode(t);
    if (vectrex != 0 && romsNode != 0)
    {
      nmCopy(romsNode->title, "ALL ROMS", (int) sizeof romsNode->title);
      romsNode->id = NM_ID_ALL_ROMS;
      nmAppendChild(vectrex, romsNode);
      nmAddRoms(t, romsNode, romPaths, nroms, exactImg, speedyImg);
      NmNode *demos = nmFindChildByTitle(vectrex, "DEMOS");   // a DEMOS group that entry files made also gets vectrex/demos
      if (demos != 0) nmAddMissingRoms(t, demos, romPaths, nroms, "demos/", exactImg, speedyImg);
    }
  }

  nmApplySidecars(t, rd);
  for (int i = 0; i < t->ntops; i++)             // every list below the top level is alphabetical
    if (t->tops[i]->child != 0) { if (!t->tops[i]->keepOrder) nmSortChildren(t->tops[i]); nmSortAll(t->tops[i]->child); }
  NmNode *settings = nmAddTop(t, "SETTINGS", NM_ID_SETTINGS);
  for (int i = 0; i < t->ntops; i++) nmHideEmpty(t->tops[i]);
  nmSortTops(t);
  return settings;
}

// Builds the whole tree in one go (the loader does it in steps, one screen refresh each, see newMenuUi.h).
// Inputs: the names of the files in menu/ (sideNames) and a reader for them, the image folder ("piZero1/" prefix,
// file names in files[]), the ROM paths found under vectrex/ and the two emulator images.
static NmNode *nmAssemble(NmTree *t, NmScratch *sc, char (*sideNames)[NM_KEY_LEN], int nside, NmReadFn rd,
                          const char *prefix, const char *const *files, int nfiles,
                          char (*romPaths)[NM_ROM_PATH], int nroms,
                          const char *exactImg, const char *speedyImg, NmExistsFn exists, int *nentries, int *nfound)
{
  *nentries = nmAddEntries(t, sideNames, nside, rd, prefix);
  return nmAssembleRest(t, sc, rd, prefix, files, nfiles, romPaths, nroms, exactImg, speedyImg, exists, nfound);
}


#endif
