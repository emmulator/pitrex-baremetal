// The menu files, read in layers: menu/menu.default.ini (shipped), then every other menu/*.ini in alphabetical order,
// then menu/menu.ini (the user's changes, last so it has the final word). Read into records in memory; the menu
// asks for a record by key.
// No hardware dependencies, so it can be unit tested on a PC (new_menu/tools/test_new_menu.c).
//
// One section is one record. The section name is the title shown in the menu, unless "title =" says otherwise.
// What the section is follows from one key:
//   image  = ...   an entry that starts a program                     key "e.<section>"
//   file   = ...   the description of a file the menu found           key from the path (nmPathKey)
//   folder = ...   with image: every file in the folder is an entry   key "f.<folder>"
//                  without image: settings of a ROM folder group      key "dir.<folder under vectrex/>"
//   none           settings of a category ("ARCADE / ATARI")          key "cat.<path>"
// A section called [entry] is the 0.1.0 format (one file per entry): its key is the file name, as before.
// A later section with the same section name, file or folder changes the earlier one: what it sets replaces what
// was there, the rest stays. A later section with only a name and changes (no image, file or folder) changes the
// earlier section of that name, whatever it was. "also = A, B" lists a copy in further categories.
// Format and examples: NEW_MENU_README.md, new_menu/menu/11-menu-ini-plan.md
#ifndef MENU_FILE_H
#define MENU_FILE_H

#include <string.h>
#include "menuSidecar.h"

#ifndef NM_KEY_LEN
#define NM_KEY_LEN 64
#endif
#define NM_MAX_RECS 400
#define NM_POOL 65536
#define NM_REC_PATH 64

#define NM_REC_ENTRY 1
#define NM_REC_FILE 2
#define NM_REC_FOLDER 3
#define NM_REC_CATEGORY 4
#define NM_REC_NOTES 5                 // 0.1.0 notes file (no image): matched by its file name

typedef struct
{
  char kind, hide, order, emulator, skip, nrows, nctl, hasRows;
  char key[NM_KEY_LEN];
  char path[NM_REC_PATH];              // file= or folder=
  char files[24];                      // folder rule: extensions, "bin vec" (empty: every file)
  char title[32];
  char image[40];
  char category[56];
  char p1[48];
  char p2[48];
  char needs[56];
  char also[96];                       // also listed in these categories (comma-separated)
  char name[40];                       // the section name, so a later section of the same name changes this one
  int seq;                             // the last time a section mentioned it (order = listed sorts by this)
  int rows;                            // offset of the first note line in the pool (lines follow, 0-separated)
  int ctl;                             // offset of the first control line
} NmRec;

typedef struct
{
  NmRec rec[NM_MAX_RECS];
  int n;
  char pool[NM_POOL];
  int used;
  int dropped;                         // records that did not fit
  int seqNext;
} NmRecs;

// ---- keys --------------------------------------------------------------------------------------------------

// "rom." + slug: lower case, every run of other characters one '_' (the 0.1.0 notes-file names)
static void nmSlugKey(const char *pre, const char *s, char *out, int max)
{
  int n = 0;
  for (int i = 0; pre[i] != 0 && n < max - 1; i++) out[n++] = pre[i];
  int start = n;
  for (; *s != 0 && n < max - 1; s++)
  {
    char c = *s;
    if (c >= 'A' && c <= 'Z') c = (char) (c + 32);
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) out[n++] = c;
    else if (n > start && out[n - 1] != '_') out[n++] = '_';
  }
  if (n > start && out[n - 1] == '_') n--;
  out[n] = 0;
}

static int nmStartsNoCase(const char *s, const char *pre)
{
  for (; *pre != 0; s++, pre++)
  {
    char a = *s, b = *pre;
    if (a >= 'A' && a <= 'Z') a = (char) (a + 32);
    if (b >= 'A' && b <= 'Z') b = (char) (b + 32);
    if (a != b) return 0;
  }
  return 1;
}

// The key of a file on the card, the same one the 0.1.0 notes files used:
//   vectrex/original/BERZERK.BIN -> rom.original_berzerk_bin     piZero1/voom.img -> voom     other -> file.<slug>
static void nmPathKey(const char *path, char *out, int max)
{
  if (nmStartsNoCase(path, "vectrex/")) { nmSlugKey("rom.", path + 8, out, max); return; }
  if ((nmStartsNoCase(path, "piZero1/") || nmStartsNoCase(path, "piZero2/")))
  {
    const char *b = path + 8;
    int len = (int) strlen(b);
    if (len > 4 && nmStartsNoCase(b + len - 4, ".img") && len - 4 < max)
    {
      for (int i = 0; i < len - 4; i++) out[i] = (b[i] >= 'A' && b[i] <= 'Z') ? (char) (b[i] + 32) : b[i];
      out[len - 4] = 0;
      return;
    }
  }
  nmSlugKey("file.", path, out, max);
}

// ---- records -----------------------------------------------------------------------------------------------

static NmRec *nmFindRec(NmRecs *r, const char *key)
{
  for (int i = 0; i < r->n; i++)
    if (strcmp(r->rec[i].key, key) == 0) return &r->rec[i];
  return 0;
}

// The first record that came from a section with this name (not counting categories), or 0.
static const NmRec *nmFindRecByName(NmRecs *r, const char *name)
{
  for (int i = 0; i < r->n; i++)
  {
    const NmRec *c = &r->rec[i];
    if (c->kind == NM_REC_CATEGORY || c->name[0] == 0) continue;
    const char *a = c->name, *b = name;
    while (*a != 0 && *b != 0)
    {
      char x = *a++, y = *b++;
      if (x >= 'a' && x <= 'z') x = (char) (x - 32);
      if (y >= 'a' && y <= 'z') y = (char) (y - 32);
      if (x != y) break;
      if (*a == 0 && *b == 0) return c;
    }
  }
  return 0;
}

static int nmPoolAdd(NmRecs *r, const char *s)
{
  int len = (int) strlen(s);
  if (r->used + len + 1 > NM_POOL) return -1;
  int at = r->used;
  memcpy(r->pool + at, s, (size_t) len + 1);
  r->used += len + 1;
  return at;
}

// The record as the menu code uses it (the same struct a 0.1.0 notes file was read into). Returns 1 if found.
static int nmRecToSidecar(NmRecs *r, const char *key, SidecarText *t)
{
  memset(t, 0, sizeof *t);
  const NmRec *c = nmFindRec(r, key);
  if (c == 0) return 0;
  t->found = c->hasRows;
  t->skip = c->skip;
  t->hide = c->hide;
  t->emulator = c->emulator;
  t->order = c->order;
  sidecarCopy(t->title, (int) sizeof t->title, c->title);
  sidecarCopy(t->image, (int) sizeof t->image, c->image);
  sidecarCopy(t->category, (int) sizeof t->category, c->category);
  memcpy(t->p1, c->p1, sizeof t->p1 < sizeof c->p1 ? sizeof t->p1 : sizeof c->p1);
  t->p1[sizeof t->p1 - 1] = 0;
  memcpy(t->p2, c->p2, sizeof t->p2 < sizeof c->p2 ? sizeof t->p2 : sizeof c->p2);
  t->p2[sizeof t->p2 - 1] = 0;
  sidecarCopy(t->needs, (int) sizeof t->needs, c->needs);
  sidecarCopy(t->also, (int) sizeof t->also, c->also);
  t->hideSet = 1;
  t->seq = c->seq;
  const char *s = r->pool + c->rows;
  for (int i = 0; i < c->nrows && i < SC_MAX_ROWS; i++) { sidecarCopy(t->row[i], SC_ROW_LEN, s); s += strlen(s) + 1; }
  t->rows = c->nrows;
  s = r->pool + c->ctl;
  for (int i = 0; i < c->nctl && i < SC_MAX_ROWS; i++) { sidecarCopy(t->ctl[i], SC_ROW_LEN, s); s += strlen(s) + 1; }
  t->nctl = c->nctl;
  return 1;
}

// One section, collected while its lines are read.
typedef struct
{
  char name[64];
  char path[NM_REC_PATH];
  char isFile, isFolder, orderSet;
  char files[24];
  SidecarText st;
} NmSection;

static void nmTrimEnd(char *s)
{
  int n = (int) strlen(s);
  while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t')) s[--n] = 0;
}

// Stores the section that has been read. stem: the file name without ".ini" (for [entry] sections).
static void nmEndSection(NmRecs *r, NmSection *sec, const char *stem)
{
  if (sec->name[0] == 0) return;
  NmRec c;
  memset(&c, 0, sizeof c);
  SidecarText *st = &sec->st;
  int legacy = nmStartsNoCase(sec->name, "entry") && sec->name[5] == 0;
  if (legacy)
  {
    c.kind = st->image[0] != 0 ? NM_REC_ENTRY : NM_REC_NOTES;
    sidecarCopy(c.key, NM_KEY_LEN, stem);
  }
  else if (st->image[0] != 0 && !sec->isFolder)
  {
    c.kind = NM_REC_ENTRY;
    nmSlugKey("e.", sec->name, c.key, NM_KEY_LEN);
  }
  else if (sec->isFile)
  {
    c.kind = NM_REC_FILE;
    nmPathKey(sec->path, c.key, NM_KEY_LEN);
  }
  else if (sec->isFolder && st->image[0] != 0)
  {
    c.kind = NM_REC_FOLDER;
    nmSlugKey("f.", sec->path, c.key, NM_KEY_LEN);
  }
  else if (sec->isFolder)
  {
    c.kind = NM_REC_NOTES;                    // a ROM folder group: renamed or hidden like a 0.1.0 dir.<folder>.ini
    nmSlugKey("dir.", nmStartsNoCase(sec->path, "vectrex/") ? sec->path + 8 : sec->path, c.key, NM_KEY_LEN);
  }
  else
  {
    const NmRec *same = nmFindRecByName(r, sec->name);   // no image, file or folder: changes what an earlier section
    if (same != 0) { c.kind = same->kind; sidecarCopy(c.key, NM_KEY_LEN, same->key); }   // of that name described ...
    else { c.kind = NM_REC_CATEGORY; nmSlugKey("cat.", sec->name, c.key, NM_KEY_LEN); }  // ... else it is a category
  }
  if (!legacy && st->title[0] == 0 && c.kind != NM_REC_CATEGORY && nmFindRec(r, c.key) == 0)
    sidecarCopy(st->title, (int) sizeof st->title, sec->name);

  NmRec *d = nmFindRec(r, c.key);
  if (d == 0)
  {
    if (r->n >= NM_MAX_RECS) { r->dropped++; return; }
    d = &r->rec[r->n++];
    *d = c;
    if (!legacy) sidecarCopy(d->name, (int) sizeof d->name, sec->name);
  }
  else if (d->kind == NM_REC_NOTES && c.kind != NM_REC_NOTES) d->kind = c.kind;
  // what this section sets replaces what an earlier one with the same key said
  d->seq = ++r->seqNext;
  sidecarCopy(d->path, NM_REC_PATH, sec->path[0] ? sec->path : d->path);
  if (sec->files[0]) sidecarCopy(d->files, (int) sizeof d->files, sec->files);
  if (st->title[0]) sidecarCopy(d->title, (int) sizeof d->title, st->title);
  if (st->image[0]) sidecarCopy(d->image, (int) sizeof d->image, st->image);
  if (st->category[0]) sidecarCopy(d->category, (int) sizeof d->category, st->category);
  if (st->p1[0]) memcpy(d->p1, st->p1, sizeof d->p1 < sizeof st->p1 ? sizeof d->p1 : sizeof st->p1);
  if (st->p2[0]) memcpy(d->p2, st->p2, sizeof d->p2 < sizeof st->p2 ? sizeof d->p2 : sizeof st->p2);
  d->p1[sizeof d->p1 - 1] = 0;
  d->p2[sizeof d->p2 - 1] = 0;
  if (st->needs[0]) sidecarCopy(d->needs, (int) sizeof d->needs, st->needs);
  if (st->hideSet) d->hide = (char) st->hide;
  if (st->also[0]) sidecarCopy(d->also, (int) sizeof d->also, st->also);
  if (st->emulator) d->emulator = st->emulator;
  if (sec->orderSet) d->order = (char) st->order;
  if (st->found)
  {
    d->hasRows = 1;
    d->skip = (char) st->skip;
    d->nrows = 0;
    d->rows = r->used;
    for (int i = 0; i < st->rows; i++) if (nmPoolAdd(r, st->row[i]) >= 0) d->nrows++;
  }
  if (st->nctl > 0)
  {
    d->nctl = 0;
    d->ctl = r->used;
    for (int i = 0; i < st->nctl; i++) if (nmPoolAdd(r, st->ctl[i]) >= 0) d->nctl++;
  }
}

// Reads one menu file (its whole text, changed in place). stem: its name without ".ini".
static void nmParseMenuText(NmRecs *r, char *text, const char *stem)
{
  static NmSection sec;                       // static: large, and the loader's stack is small
  memset(&sec, 0, sizeof sec);
  char *p = text;
  while (*p != 0)
  {
    char *line = p;
    while (*p != 0 && *p != '\n' && *p != '\r') p++;
    if (*p != 0) *p++ = 0;
    while (*line == ' ' || *line == '\t') line++;
    if (*line == 0 || *line == ';' || *line == '#') continue;
    if (*line == '[')
    {
      char *end = strchr(line, ']');
      if (end == 0) continue;                 // a broken line is skipped
      *end = 0;
      nmEndSection(r, &sec, stem);
      memset(&sec, 0, sizeof sec);
      char *name = line + 1;
      while (*name == ' ') name++;
      nmTrimEnd(name);
      sidecarCopy(sec.name, (int) sizeof sec.name, name);
      continue;
    }
    if (sec.name[0] == 0) continue;           // lines before the first section
    char *eq = strchr(line, '=');
    if (eq == 0) continue;
    *eq = 0;
    char *name = line, *value = eq + 1;
    nmTrimEnd(name);
    while (*value == ' ' || *value == '\t') value++;
    for (char *c = value; *c != 0; c++)        // " ; comment" ends the value (as in the other PiTrex ini files)
      if (*c == ';' && c > value && (c[-1] == ' ' || c[-1] == '\t')) { *c = 0; break; }
    nmTrimEnd(value);
    if (strcmp(name, "file") == 0) { sidecarCopy(sec.path, NM_REC_PATH, value); sec.isFile = 1; }
    else if (strcmp(name, "folder") == 0)
    {
      sidecarCopy(sec.path, NM_REC_PATH, value);
      int n = (int) strlen(sec.path);
      while (n > 0 && sec.path[n - 1] == '/') sec.path[--n] = 0;
      sec.isFolder = 1;
    }
    else if (strcmp(name, "files") == 0) sidecarCopy(sec.files, (int) sizeof sec.files, value);
    else
    {
      if (strcmp(name, "order") == 0) sec.orderSet = 1;
      sidecarLine(&sec.st, name, value);
    }
  }
  nmEndSection(r, &sec, stem);
}

// "{dir}" and "{file}" in a folder rule's parameter
static void nmSubst(char *out, int max, const char *pat, const char *dir, const char *file)
{
  int n = 0;
  for (const char *p = pat; *p != 0 && n < max - 1;)
  {
    const char *v = 0;
    if (strncmp(p, "{dir}", 5) == 0) { v = dir; p += 5; }
    else if (strncmp(p, "{file}", 6) == 0) { v = file; p += 6; }
    if (v != 0) { while (*v != 0 && n < max - 1) out[n++] = *v++; }
    else out[n++] = *p++;
  }
  out[n] = 0;
}

// Does the file name end in one of the extensions of the list ("bin vec", empty = any)?
static int nmExtOk(const char *name, const char *list)
{
  if (list[0] == 0) return 1;
  const char *dot = strrchr(name, '.');
  if (dot == 0) return 0;
  dot++;
  int dl = (int) strlen(dot);
  for (const char *p = list; *p != 0;)
  {
    while (*p == ' ' || *p == ',' || *p == '.') p++;
    const char *w = p;
    while (*p != 0 && *p != ' ' && *p != ',') p++;
    int wl = (int) (p - w);
    if (wl == dl && wl > 0)
    {
      int same = 1;
      for (int i = 0; i < wl; i++)
      {
        char a = w[i], b = dot[i];
        if (a >= 'A' && a <= 'Z') a = (char) (a + 32);
        if (b >= 'A' && b <= 'Z') b = (char) (b + 32);
        if (a != b) same = 0;
      }
      if (same) return 1;
    }
  }
  return 0;
}

// A Vectrex cartridge starts with "g GCE" (what the Vectrex BIOS checks at power-on).
static int nmIsCartridge(const unsigned char *head, int n)
{
  return n >= 5 && head[0] == 'g' && head[1] == ' ' && head[2] == 'G' && head[3] == 'C' && head[4] == 'E';
}

#endif
