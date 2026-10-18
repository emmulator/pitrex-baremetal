/* Host test for pitrex/loader/newMenuLogic.h: the menu is built from entry files, nothing is compiled in.
 *
 *   gcc -c -w -Wno-error=implicit-function-declaration -include stdio.h -include string.h -include ctype.h \
 *       -include stdlib.h -include errno.h -I pitrex/vectrex -o /tmp/ini.o pitrex/vectrex/ini.c
 *   gcc -Wall -Wextra -Wno-unused-function -I pitrex/loader -I pitrex/vectrex -o /tmp/t docs/tools/test_new_menu.c /tmp/ini.o
 *   python3 -I docs/tools/make_rom_test_data.py <card>/vectrex <homebrew dir> /tmp/data <menu folder> <card>/piZero1
 *   /tmp/t <menu folder> /tmp/data/img_listing.txt /tmp/data/expected_programs.txt /tmp/data/rom_paths.txt /tmp/data/rom_expected.txt
 *
 * <menu folder> is docs/menu/sidecar/menu (the exporter's output) or the card's menu/.
 * Set NM_DUMP=1 to print the tree.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "ini.h"
#include "newMenuLogic.h"

/* ini.c's file-based ini_parse() pulls in the baremetal file layer; stub what it names. */
char lastAccessedIni[255];
int __filelength(FILE *fd) { (void) fd; return 0; }
const char *getErrorText(int e) { (void) e; return ""; }
int f_getcwd(char *b, int n) { (void) b; (void) n; return 0; }

static int fails = 0;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: " __VA_ARGS__); printf("\n"); fails++; } } while (0)

static const char *menuDir;

static int handler(void *user, const char *section, const char *name, const char *value)
{
  (void) section;
  sidecarLine((SidecarText *) user, name, value);
  return 1;
}

static int hostRead(const char *key, SidecarText *t)
{
  char path[512];
  snprintf(path, sizeof path, "%s/%s.ini", menuDir, key);
  FILE *f = fopen(path, "rb");
  if (!f) return 0;
  static char buf[4096];
  int len = (int) fread(buf, 1, sizeof buf - 1, f);
  fclose(f);
  buf[len] = 0;
  memset(t, 0, sizeof *t);
  ini_parse_string(buf, handler, t);
  return 1;
}

static char sideNames[320][NM_KEY_LEN];
static int nside = 0;

static void listSidecars(void)
{
  DIR *d = opendir(menuDir);
  struct dirent *e;
  while (d && (e = readdir(d)) != 0 && nside < 320)
  {
    int len = (int) strlen(e->d_name);
    if (len <= 4 || strcmp(e->d_name + len - 4, ".ini") != 0) continue;
    memset(sideNames[nside], 0, NM_KEY_LEN);
    strncpy(sideNames[nside], e->d_name, (size_t) len - 4);
    nside++;
  }
  if (d) closedir(d);
}

static NmTree tree;
static NmNav nav;
static NmScratch scratch;
static char listing[256][64];
static const char *files[256];
static int nf = 0;
static char romPaths[NM_MAX_ROMS][NM_ROM_PATH];
static int nroms = 0;
static int nentries, nfound;

static const char *present[8];                  /* paths the fake card has; 0 = no checking at all */
static int npresent = -1;
static int fakeExists(const char *p)
{
  for (int i = 0; i < npresent; i++) if (strcmp(present[i], p) == 0) return 1;
  return 0;
}

static NmNode *build(const char *const *fl, int nfl)
{
  memset(&tree, 0, sizeof tree);
  return nmAssemble(&tree, &scratch, sideNames, nside, hostRead, "piZero1/", fl, nfl, romPaths, nroms,
                    "piZero1/vectrexexact.img", "piZero1/vectrexspeedy.img", npresent >= 0 ? fakeExists : 0, &nentries, &nfound);
}

static int selectTitle(const char *title)
{
  for (int i = 0; i < nav.count; i++)
    if (strcmp(nav.list[i]->title, title) == 0) { nav.sel = i; nmFixWindow(&nav); return 1; }
  return 0;
}

static NmNode *findTitle(NmNode *parent, const char *title)
{
  return nmFindChildByTitle(parent, title);
}

int main(int argc, char **argv)
{
  if (argc < 3) { fprintf(stderr, "usage: %s <menu folder> <image listing> [expected programs] [rom paths] [rom expected]\n", argv[0]); return 2; }
  menuDir = argv[1];
  listSidecars();

  /* ---- sidecar parameter decoding ---- */
  {
    char o[40];
    sidecarParam(o, sizeof o, "0x2d");
    CHECK(o[0] == 0x2d && o[1] == 0, "0x2d is one byte");
    sidecarParam(o, sizeof o, "0x0a0b");
    CHECK(o[0] == 0x0a && o[1] == 0x0b && o[2] == 0, "0x0a0b is two bytes");
    sidecarParam(o, sizeof o, "original/BERZERK.BIN");
    CHECK(strcmp(o, "original/BERZERK.BIN") == 0, "a path stays a path");
    sidecarParam(o, sizeof o, "0xzz");
    CHECK(strcmp(o, "0xzz") == 0, "bad hex stays literal");
    sidecarParam(o, sizeof o, "0x2");
    CHECK(strcmp(o, "0x2") == 0, "odd hex stays literal");
  }

  FILE *f = fopen(argv[2], "r");
  if (!f) { perror(argv[2]); return 2; }
  while (nf < 256 && fgets(listing[nf], 64, f))
  {
    listing[nf][strcspn(listing[nf], "\r\n")] = 0;
    files[nf] = listing[nf];
    nf++;
  }
  fclose(f);

  if (argc > 4)
  {
    f = fopen(argv[4], "r");
    if (!f) { perror(argv[4]); return 2; }
    char l[128];
    while (nroms < NM_MAX_ROMS && fgets(l, sizeof l, f))
    {
      l[strcspn(l, "\r\n")] = 0;
      if (l[0] != 0) nmCopy(romPaths[nroms++], l, NM_ROM_PATH);
    }
    fclose(f);
  }

  NmNode *settings = build(files, nf);
  int entryFiles = 0;
  for (int i = 0; i < nside; i++) { SidecarText st; if (hostRead(sideNames[i], &st) && st.image[0]) entryFiles++; }
  printf("sidecar files: %d, entry files: %d, nodes: %d, programs: %d, ROMs: %d\n", nside, entryFiles, tree.used, nfound, nroms);
  CHECK(settings != 0 && settings->id == NM_ID_SETTINGS, "settings node");
  CHECK(nentries == entryFiles, "every entry file became an entry: %d of %d", nentries, entryFiles);
  CHECK(entryFiles >= 60, "entry files found: %d", entryFiles);

  /* ---- top level: categories alphabetically, then PROGRAMS, then SETTINGS ---- */
  {
    int sorted = 1;
    int i = 0;
    for (; i < tree.ntops && nmTopRank(tree.tops[i]) == 0; i++)
      if (i > 0 && nmTitleCmp(tree.tops[i - 1]->title, tree.tops[i]->title) > 0) sorted = 0;
    CHECK(sorted, "top categories are alphabetical");
    CHECK(tree.tops[tree.ntops - 1] == settings, "SETTINGS is last");
    CHECK(nmFindTop(&tree, "ARCADE") && nmFindTop(&tree, "VECTREX") && nmFindTop(&tree, "PITREX") &&
          nmFindTop(&tree, "VIDEO") && nmFindTop(&tree, "AUDIO"), "the five categories come from the entry files");
  }

  nmInit(&nav, &tree);
  CHECK(selectTitle("ARCADE") && nmEnter(&nav) == 0, "enter ARCADE");
  char path[96];
  nmPath(&nav, path, sizeof path);
  CHECK(strcmp(path, "ARCADE") == 0, "breadcrumb ARCADE, got '%s'", path);
  CHECK(selectTitle("ALL AAE GAMES"), "ALL AAE GAMES under ARCADE (an entry file, not compiled)");
  NmNode *aae = nmCurrent(&nav);
  CHECK(aae->img != 0 && strcmp(aae->img, "piZero1/aae.img") == 0 && aae->param1 == 0, "ALL AAE GAMES starts aae.img without a parameter");
  CHECK(selectTitle("ATARI") && nmEnter(&nav) == 0, "enter ATARI");
  nmPath(&nav, path, sizeof path);
  CHECK(strcmp(path, "ARCADE > ATARI") == 0, "breadcrumb, got '%s'", path);
  CHECK(selectTitle("MAJOR HAVOC"), "MAJOR HAVOC under ATARI");
  NmNode *mh = nmEnter(&nav);
  CHECK(mh != 0 && mh->param1 != 0 && mh->param1[0] == 0x2d && mh->param1[1] == 0, "Major Havoc passes game byte 0x2d");
  CHECK(mh != 0 && strcmp(mh->needs, "roms/aae/mhavoc.zip") == 0, "Major Havoc needs its zip, got '%s'", mh ? mh->needs : "");
  CHECK(selectTitle("STAR WARS"), "Star Wars is called STAR WARS");
  NmNode *sw = nmEnter(&nav);
  CHECK(sw != 0 && strcmp(sw->img, "piZero1/swsbt.img") == 0, "Star Wars starts swsbt.img");
  CHECK(nav.count > NM_ROWS, "ATARI has more rows than fit on screen (%d)", nav.count);
  nav.sel = 0; nav.top = 0;
  for (int i = 0; i < nav.count + 3; i++) nmMove(&nav, 1);   /* moving past the end stops at the last row */
  CHECK(nav.sel >= nav.top && nav.sel < nav.top + NM_ROWS, "window follows the selection");
  CHECK(nmBack(&nav) == 1 && nav.depth == 1, "back to ARCADE");
  CHECK(strcmp(nmCurrent(&nav)->title, "ATARI") == 0, "selection restored to ATARI, got %s", nmCurrent(&nav)->title);
  CHECK(nmBack(&nav) == 1 && nav.depth == 0, "back to the top");
  CHECK(nmBack(&nav) == 0, "cannot go above the top");

  /* speedy / exact groups under VECTREX */
  {
    NmNode *vx = nmFindTop(&tree, "VECTREX");
    CHECK(vx != 0, "VECTREX");
    NmNode *sp = findTitle(vx, "SPEEDY VECTREX");
    CHECK(sp != 0 && sp->child != 0, "SPEEDY VECTREX group exists");
    NmNode *ww = sp ? findTitle(sp, "WEB WARS") : 0;
    CHECK(ww != 0 && strcmp(ww->img, "piZero1/vectrexspeedy.img") == 0 && strcmp(ww->param1, "original/WEBWARS.BIN") == 0, "Web Wars starts Speedy with its ROM");
    NmNode *ex = findTitle(vx, "EXACT VECTREX");
    CHECK(ex != 0 && findTitle(ex, "MR BOSTON") != 0, "EXACT VECTREX has Mr Boston");
    CHECK(ex != 0 && findTitle(ex, "PICK A GAME") == 0, "there is no PICK A GAME");
    CHECK(findTitle(vx, "ALL ROMS") != 0 || nroms == 0, "ALL ROMS is under VECTREX");
    NmNode *vid = nmFindTop(&tree, "VIDEO");
    NmNode *ba = vid ? findTitle(vid, "BAD APPLE") : 0;
    CHECK(ba != 0 && strcmp(ba->param1, "vectrex/movies") == 0 && strcmp(ba->param2, "BadApple.bin") == 0, "Bad Apple passes both parameters");
  }

  /* ---- everything below the top level is alphabetical (ALL ROMS keeps folders first) ---- */
  {
    int unsorted = 0;
    for (int i = 0; i < tree.used; i++)
    {
      NmNode *n = &tree.pool[i];
      if (n->id == NM_ID_ALL_ROMS || n->child == 0) continue;
      for (NmNode *c = n->child; c != 0 && c->right != 0; c = c->right)
        if (nmTitleCmp(c->title, c->right->title) > 0) { unsorted++; printf("   not sorted under %s: %s > %s\n", n->title, c->title, c->right->title); }
    }
    CHECK(unsorted == 0, "%d lists not alphabetical", unsorted);
  }

  /* ---- titles: no brackets, SBT is not part of a name ---- */
  {
    int bad = 0;
    for (int i = 0; i < tree.used; i++)
    {
      const char *t = tree.pool[i].title;
      int len = (int) strlen(t);
      if (strchr(t, '(') || strchr(t, ')') || (len > 4 && strcmp(t + len - 4, " SBT") == 0)) { bad++; printf("   bad title: %s\n", t); }
    }
    CHECK(bad == 0, "%d titles with brackets or SBT", bad);
  }

  /* ---- programs: images on the card that no entry file mentions ---- */
  if (argc > 3)
  {
    static char exp[NM_MAX_PROGS][NM_NAME_LEN];
    int ne = 0;
    f = fopen(argv[3], "r");
    if (!f) { perror(argv[3]); return 2; }
    char l[64];
    while (ne < NM_MAX_PROGS && fgets(l, sizeof l, f)) { l[strcspn(l, "\r\n")] = 0; if (l[0]) strcpy(exp[ne++], l); }
    fclose(f);
    CHECK(ne == nfound, "discovered %d programs, expected %d", nfound, ne);
    for (int i = 0; i < ne && i < nfound; i++) CHECK(strcmp(exp[i], scratch.found[i]) == 0, "discovered[%d] '%s' != expected '%s'", i, scratch.found[i], exp[i]);
    NmNode *pr = nmFindTop(&tree, "PROGRAMS");
    CHECK((ne > 0) == (pr != 0), "PROGRAMS exists exactly when something was found");
    if (pr != 0)
    {
      CHECK(pr->child != 0 && pr->child->discovered && strncmp(pr->child->img, "piZero1/", 8) == 0, "first program is discovered with its image path");
      CHECK(nmTopRank(pr) == 1, "PROGRAMS ranks after the categories");
    }
  }

  /* ---- an entry whose image is not on the card is hidden, and so is an emptied category ---- */
  {
    const char *fewer[256];
    int nfew = 0;
    for (int i = 0; i < nf; i++) if (strcasecmp(files[i], "tempest.img") != 0) fewer[nfew++] = files[i];
    CHECK(nfew == nf - 1 || nf == 0, "tempest.img was in the listing");
    build(fewer, nfew);
    NmNode *atari = findTitle(nmFindTop(&tree, "ARCADE"), "ATARI");
    NmNode *t = atari ? findTitle(atari, "TEMPEST") : 0;
    CHECK(t != 0 && t->hidden, "TEMPEST is hidden when tempest.img is missing");
    NmNode *bw = atari ? findTitle(atari, "BLACK WIDOW") : 0;
    CHECK(bw != 0 && !bw->hidden, "the other games stay");
    /* no images at all except one: the other categories disappear */
    const char *one[1] = { "swsbt.img" };
    build(one, 1);
    nmInit(&nav, &tree);
    CHECK(selectTitle("ARCADE") && !selectTitle("VIDEO") && !selectTitle("AUDIO") && selectTitle("SETTINGS"), "empty categories are hidden, SETTINGS stays");
    build(files, nf);
  }

  /* ---- caveat tags ---- */
  CHECK(nmTagsOfLine("NO SOUND!") == NM_TAG_NOSOUND, "no sound");
  CHECK(nmTagsOfLine("AAE EMU / No SOUND") == NM_TAG_NOSOUND, "no sound in other case");
  CHECK(nmTagsOfLine("WORK IN PROGRESS!!") == NM_TAG_WIP, "wip");
  CHECK(nmTagsOfLine("INPUT NOT WORKING") == NM_TAG_BROKEN, "not working");
  CHECK(nmTagsOfLine("CRASHING") == NM_TAG_BROKEN, "crashing");
  CHECK(nmTagsOfLine("WORKING GOOD!") == 0, "working good has no tag");
  CHECK(nmTagsOfLine("") == 0 && nmTagsOfLine("SORT OF...") == 0, "plain notes have no tag");
  CHECK(strcmp(nmTagText(NM_TAG_NOSOUND | NM_TAG_WIP), "WIP") == 0, "wip outranks no sound");
  CHECK(strcmp(nmTagText(NM_TAG_BROKEN | NM_TAG_WIP), "BROKEN") == 0, "broken outranks wip");
  CHECK(nmTagText(0)[0] == 0, "no tag, no text");
  {
    struct { const char *title; int tags; } expect[] = {
      { "LUNAR LANDER", 0 }, { "BLACK WIDOW", NM_TAG_WIP }, { "SPEEDFREAK", NM_TAG_NOSOUND },
      { "SPACE WARS", NM_TAG_BROKEN }, { "OMEGA RACE", NM_TAG_WIP | NM_TAG_NOSOUND }, { "TEMPEST", NM_TAG_WIP | NM_TAG_NOSOUND },
      { "GRAVITAR", NM_TAG_WIP | NM_TAG_NOSOUND }, { "BARRIER", 0 }, { "DEMON", 0 }, { "ZBLAST", 0 }, { "REBEL DEFENDER", NM_TAG_NOSOUND },
      { "BZ2 BETA", NM_TAG_NOSOUND }, { "ASTEROIDS", 0 } };
    int tagged = 0, total = 0;
    for (int i = 0; i < tree.used; i++)
    {
      NmNode *n = &tree.pool[i];
      if (n->img != 0 && !n->discovered) { total++; if (n->tags) tagged++; }
      for (unsigned k = 0; k < sizeof expect / sizeof expect[0]; k++)
        if (n->img != 0 && !n->discovered && strcmp(n->title, expect[k].title) == 0)
          CHECK(n->tags == expect[k].tags, "tags of %s: %d, expected %d", n->title, n->tags, expect[k].tags);
    }
    printf("entries with a caveat tag: %d of %d\n", tagged, total);
    CHECK(tagged >= 20 && total >= 60, "caveats: %d of %d", tagged, total);
  }

  /* ---- Vectrex ROMs (optional arguments 4 and 5) ---- */
  if (argc > 5)
  {
    NmNode *vx = nmFindTop(&tree, "VECTREX");
    NmNode *roms = vx ? findTitle(vx, "ALL ROMS") : 0;
    CHECK(roms != 0 && roms->id == NM_ID_ALL_ROMS, "ALL ROMS exists");
    static char want[800][96];
    int nw = 0;
    f = fopen(argv[5], "r");
    if (!f) { perror(argv[5]); return 2; }
    char l[128];
    while (nw < 800 && fgets(l, sizeof l, f)) { l[strcspn(l, "\r\n")] = 0; if (l[0]) strcpy(want[nw++], l); }
    fclose(f);
    int got = 0, bad = 0;
    for (NmNode *c = roms ? roms->child : 0; c != 0; c = c->right)
    {
      char line[128];
      if (c->child != 0)
      {
        snprintf(line, sizeof line, "G:%s", c->title);
        if (got < nw && strcmp(line, want[got]) != 0) { printf("   tree[%d] got '%s' want '%s'\n", got, line, want[got]); bad++; }
        got++;
        for (NmNode *e = c->child; e != 0; e = e->right)
        {
          snprintf(line, sizeof line, "E:%s", e->title);
          if (got < nw && strcmp(line, want[got]) != 0) { printf("   tree[%d] got '%s' want '%s'\n", got, line, want[got]); bad++; }
          got++;
        }
      }
      else
      {
        snprintf(line, sizeof line, "R:%s", c->title);
        if (got < nw && strcmp(line, want[got]) != 0) { printf("   tree[%d] got '%s' want '%s'\n", got, line, want[got]); bad++; }
        got++;
      }
    }
    printf("ROM paths read: %d, tree rows: %d (expected %d)\n", nroms, got, nw);
    CHECK(got == nw, "tree rows %d != %d", got, nw);
    CHECK(bad == 0, "%d rows differ", bad);

    NmNode *first = 0;
    for (NmNode *c = roms ? roms->child : 0; c != 0 && first == 0; c = c->right) first = c->child ? c->child : c;
    CHECK(first != 0 && first->discovered && first->param1 != 0, "first ROM entry");
    if (first != 0)
    {
      CHECK(strstr(first->img, "vectrexexact.img") != 0, "ROMs start Exact by default");
      CHECK(strcmp(nmOtherEmulator(first), "piZero1/vectrexspeedy.img") == 0, "Button 2 gives Speedy");
      nmChooseEmulator(first, 's');
      CHECK(strstr(first->img, "vectrexspeedy.img") != 0 && strstr(first->altImg, "vectrexexact.img") != 0, "sidecar can switch to Speedy");
      nmChooseEmulator(first, 's');
      CHECK(strstr(first->img, "vectrexspeedy.img") != 0, "choosing the same emulator twice changes nothing");
      nmChooseEmulator(first, 'e');
      CHECK(strstr(first->img, "vectrexexact.img") != 0, "and back to Exact");
      CHECK(strncmp(first->key, "rom.", 4) == 0, "ROM sidecar key starts with rom.");
    }
    char k[40];
    nmRomKey("original/MR_BOSTON.BIN", k, sizeof k);
    CHECK(strcmp(k, "rom.original_mr_boston_bin") == 0, "key for original/MR_BOSTON.BIN is '%s'", k);
    CHECK(nmRomPathOk("A.BIN") && nmRomPathOk("sub/dir/deep.bin") && nmRomPathOk("x.Bin"), "paths that are listed");
    CHECK(!nmRomPathOk(".hidden.bin") && !nmRomPathOk("sub/.hidden.bin") && !nmRomPathOk("readme.txt") &&
          !nmRomPathOk("bios/x.bin") && !nmRomPathOk("movies/m.bin") && !nmRomPathOk("a/waves/w.bin") && !nmRomPathOk(".bin") && !nmRomPathOk("x.bi"), "paths that are not listed");

    /* the DEMOS group gets the demos found in vectrex/demos that no entry file has */
    NmNode *demos = vx ? findTitle(vx, "DEMOS") : 0;
    CHECK(demos != 0, "DEMOS group exists");
    int before = 0, after = 0;
    for (NmNode *c = demos ? demos->child : 0; c != 0; c = c->right) before++;
    CHECK(findTitle(demos, "ZED") != 0, "a demo found only in vectrex/demos was merged into DEMOS");
    static char dpaths[4][NM_ROM_PATH] = { "demos/epic.bin", "DEMOS/Brand.New.bin", "demos/sub/deep.bin", "other/x.bin" };
    int added = demos ? nmAddMissingRoms(&tree, demos, dpaths, 4, "demos/", "piZero1/vectrexexact.img", "piZero1/vectrexspeedy.img") : 0;
    for (NmNode *c = demos ? demos->child : 0; c != 0; c = c->right) after++;
    CHECK(added == 1 && after == before + 1, "only the missing demo (Brand New) is added: added=%d", added);
  }

  /* ---- help texts: only characters the PiTrex font draws ---- */
  {
    const char *cats[] = { "VECTREX", "ARCADE", "PITREX", "VIDEO", "AUDIO", "SPEEDY VECTREX", "EXACT VECTREX", "DEMOS" };
    for (unsigned i = 0; i < sizeof cats / sizeof cats[0]; i++)
    {
      NmNode n;
      memset(&n, 0, sizeof n);
      strcpy(n.title, cats[i]);
      const char *lines[6];
      int nl = nmHelp(&n, lines, 6);
      CHECK(nl > 0, "help for %s", cats[i]);
      for (int l = 0; l < nl; l++)
      {
        CHECK(strlen(lines[l]) <= 46, "help line fits (%s): %s", cats[i], lines[l]);
        for (const char *c = lines[l]; *c; c++)
          CHECK((*c >= 'A' && *c <= 'Z') || (*c >= '0' && *c <= '9') || *c == ' ' || *c == '.' || *c == '!',
                "help uses a drawable character, got '%c' in: %s", *c, lines[l]);
      }
    }
    const int ids[] = { NM_ID_PROGRAMS, NM_ID_ALL_ROMS, NM_ID_ROM_FOLDER, NM_ID_SETTINGS };
    for (unsigned i = 0; i < sizeof ids / sizeof ids[0]; i++)
    {
      NmNode n;
      memset(&n, 0, sizeof n);
      n.id = ids[i];
      const char *lines[6];
      CHECK(nmHelp(&n, lines, 6) > 0, "help for id %d", ids[i]);
    }
    NmNode entry;
    memset(&entry, 0, sizeof entry);
    entry.img = "x.img";
    const char *lines[6];
    CHECK(nmHelp(&entry, lines, 6) == 0, "a game has no category help");
  }

  /* ---- entries whose ROM is not on the card are hidden ---- */
  {
    npresent = 0;
    build(files, nf);
    NmNode *arc = nmFindTop(&tree, "ARCADE");
    CHECK(arc == 0 || arc->hidden, "no ROMs: no ARCADE category at all");
    int shown = 0;
    for (int i = 0; i < tree.used; i++)
      if (tree.pool[i].needs[0] != 0 && !tree.pool[i].hidden) shown++;
    CHECK(shown == 0, "no ROMs: %d entries with a needs file still show", shown);

    present[0] = "roms/aae/mhavoc.zip";
    npresent = 1;
    build(files, nf);
    shown = 0;
    int picker = 0;
    for (int i = 0; i < tree.used; i++)
    {
      const NmNode *n = &tree.pool[i];
      if (n->hidden || n->needs[0] == 0) continue;
      if (n->needs[strlen(n->needs) - 1] == '/') picker++; else { shown++; CHECK(strcmp(n->title, "MAJOR HAVOC") == 0, "only Major Havoc shows, not %s", n->title); }
    }
    CHECK(shown == 1, "one ROM: one game shows (%d)", shown);
    CHECK(picker == 1, "one AAE ROM: the AAE picker shows");

    present[0] = "roms/bzone.zip";
    build(files, nf);
    picker = 0;
    for (int i = 0; i < tree.used; i++)
      if (!tree.pool[i].hidden && tree.pool[i].needs[0] != 0 && tree.pool[i].needs[strlen(tree.pool[i].needs) - 1] == '/') picker++;
    CHECK(picker == 0, "a dedicated emulator's ROM does not bring the AAE picker back");
    npresent = -1;
    build(files, nf);                           /* later checks use the unfiltered tree */
  }

  if (getenv("NM_DUMP"))
    for (int i = 0; i < tree.ntops; i++)
    {
      printf("TOP %s\n", tree.tops[i]->title);
      int k = 0;
      for (NmNode *c = tree.tops[i]->child; c != 0; c = c->right, k++)
      {
        printf("  [%d] %s%s\n", k, c->title, c->hidden ? " (hidden)" : "");
        int j = 0;
        if (strcmp(c->title, "ALL ROMS") != 0) for (NmNode *g = c->child; g != 0; g = g->right, j++) printf("      [%d] %s%s\n", j, g->title, g->hidden ? " (hidden)" : "");
      }
    }

  printf(fails ? "FAIL (%d)\n" : "PASS\n", fails);
  return fails != 0;
}
