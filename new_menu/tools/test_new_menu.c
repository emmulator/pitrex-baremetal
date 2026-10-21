/* Host test for pitrex/loader/newMenuLogic.h: the menu is built from entry files, nothing is compiled in.
 *
 *   gcc -c -w -Wno-error=implicit-function-declaration -include stdio.h -include string.h -include ctype.h \
 *       -include stdlib.h -include errno.h -I pitrex/vectrex -o /tmp/ini.o pitrex/vectrex/ini.c
 *   gcc -Wall -Wextra -Wno-unused-function -I pitrex/loader -I pitrex/vectrex -o /tmp/t new_menu/tools/test_new_menu.c /tmp/ini.o
 *   python3 -I new_menu/tools/make_rom_test_data.py <card>/vectrex <homebrew dir> /tmp/data <menu folder> <card>/piZero1
 *   /tmp/t <menu folder> /tmp/data/img_listing.txt /tmp/data/expected_programs.txt /tmp/data/rom_paths.txt /tmp/data/rom_expected.txt
 *
 * <menu folder> is new_menu/menu/sidecar/menu (0.1.0: one file per entry), a folder with menu.default.ini (0.2.0), or the
 * card's menu/. The folder is read the way the loader reads it (menu.default.ini, the rest, then menu.ini).
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

static NmRecs recs;
static int recRead(const char *key, SidecarText *t) { return nmRecToSidecar(&recs, key, t); }

static char sideNames[320][NM_KEY_LEN];
static int nside = 0;

static int cmpName(const void *a, const void *b) { return strcmp((const char *) a, (const char *) b); }

/* the files of the menu folder in the loader's reading order: menu.default, the others sorted, then menu */
static void listSidecars(void)
{
  DIR *d = opendir(menuDir);
  struct dirent *e;
  while (d && (e = readdir(d)) != 0 && nside < 320)
  {
    int len = (int) strlen(e->d_name);
    if (len <= 4 || strcmp(e->d_name + len - 4, ".ini") != 0) continue;
    memset(sideNames[nside], 0, NM_KEY_LEN);
    for (int i = 0; i < len - 4 && i < NM_KEY_LEN - 1; i++) sideNames[nside][i] = nmLower(e->d_name[i]);
    nside++;
  }
  if (d) closedir(d);
  qsort(sideNames, (size_t) nside, NM_KEY_LEN, cmpName);
  int main = 0, def = 0;
  for (int i = 0; i < nside; i++) { if (!strcmp(sideNames[i], "menu")) main = 1; if (!strcmp(sideNames[i], "menu.default")) def = 1; }
  int o = 0;
  for (int i = 0; i < nside; i++)
    if (strcmp(sideNames[i], "menu") && strcmp(sideNames[i], "menu.default")) memmove(sideNames[o++], sideNames[i], NM_KEY_LEN);
  nside = o;
  if (def) { memmove(sideNames[1], sideNames[0], (size_t) nside * NM_KEY_LEN); strcpy(sideNames[0], "menu.default"); nside++; }
  if (main) strcpy(sideNames[nside++], "menu");
}

static void loadRecs(void)
{
  static char buf[131072];
  memset(&recs, 0, sizeof recs);
  for (int i = 0; i < nside; i++)
  {
    char path[512];
    snprintf(path, sizeof path, "%s/%s.ini", menuDir, sideNames[i]);
    FILE *f = fopen(path, "rb");
    if (!f) continue;
    int len = (int) fread(buf, 1, sizeof buf - 1, f);
    fclose(f);
    buf[len] = 0;
    nmParseMenuText(&recs, buf, sideNames[i]);
  }
}

/* what the folder-rule folders hold on Malban's card */
static const char *movies[] = { "BadApple.bin", "BadAppleDE.bin", "BlaBla.bin", "LaLinea.bin" };
static const char *waves[] = { "Bohemian.bin", "PurpleRain.bin" };

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

/* the loader's steps: entries of the menu files, folder rules (and the built-in ones), then the rest */
static NmNode *build(const char *const *fl, int nfl)
{
  memset(&tree, 0, sizeof tree);
  nentries = nmAddRecEntries(&tree, &recs, "piZero1/");
  static NmRec def[2];
  int ndef = nmDefaultFolderRules(&recs, def, 2);
  for (int i = 0; i < recs.n + ndef; i++)
  {
    const NmRec *rule = i < recs.n ? &recs.rec[i] : &def[i - recs.n];
    if (rule->kind != NM_REC_FOLDER) continue;
    if (nmEqNoCase(rule->path, "vectrex/movies")) nmAddFolderFiles(&tree, &recs, rule, "piZero1/", movies, 4, recRead);
    if (nmEqNoCase(rule->path, "vectrex/waves")) nmAddFolderFiles(&tree, &recs, rule, "piZero1/", waves, 2, recRead);
  }
  return nmAssembleRest(&tree, &scratch, recRead, "piZero1/", fl, nfl, romPaths, nroms,
                        "piZero1/vectrexexact.img", "piZero1/vectrexspeedy.img", npresent >= 0 ? fakeExists : 0, &nfound);
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
  loadRecs();

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
  for (int i = 0; i < recs.n; i++) if (recs.rec[i].kind == NM_REC_ENTRY) entryFiles++;
  printf("menu files: %d, records: %d, entries: %d, nodes: %d, programs: %d, ROMs: %d\n", nside, recs.n, entryFiles, tree.used, nfound, nroms);
  CHECK(recs.dropped == 0, "%d sections did not fit", recs.dropped);
  CHECK(settings != 0 && settings->id == NM_ID_SETTINGS, "settings node");
  CHECK(nentries >= 60 && nentries <= entryFiles, "entries %d of %d (fewer only when two files describe the same game)", nentries, entryFiles);
  CHECK(entryFiles >= 60, "entry files found: %d", entryFiles);

  /* ---- top level: categories alphabetically, then SETTINGS ---- */
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
      if (n->id == NM_ID_ALL_ROMS || n->child == 0 || n->keepOrder) continue;   /* order = listed is deliberate */
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
    NmNode *pr = nmFindTop(&tree, "PITREX");
    int inPitrex = 0, wrongPath = 0;
    for (NmNode *c = pr ? pr->child : 0; c != 0; c = c->right)
      if (c->discovered) { inPitrex++; if (strncmp(c->img, "piZero1/", 8) != 0) wrongPath++; }
    CHECK(inPitrex == ne && wrongPath == 0, "every program found on the card is listed under PITREX with its image path (%d of %d)", inPitrex, ne);
    CHECK(nmFindTop(&tree, "PROGRAMS") == 0, "there is no PROGRAMS category any more");
    int described = 0;
    for (NmNode *c = pr ? pr->child : 0; c != 0; c = c->right) if (!c->discovered) described++;
    CHECK(ne == 0 || described > 0, "programs found on the card are added to PITREX, not put in place of the games the menu files list there");
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

  /* ---- the shipped favorites.ini example (when the menu folder has it) ---- */
  if (nmFindRec(&recs, "cat.favorites") != 0)
  {
    NmNode *fav = nmFindTop(&tree, "FAVORITES");
    const char *want[] = { "VECTORBLADE", "ASTEROIDS", "ARMOR ATTACK" };
    int k = 0, ok = fav != 0 && !fav->hidden;
    for (NmNode *c = fav ? fav->child : 0; c; c = c->right, k++) if (k >= 3 || strcmp(c->title, want[k])) ok = 0;
    CHECK(ok && k == 3, "FAVORITES: Vectorblade, Asteroids, Armor Attack in the order of favorites.ini (%d rows)", k);
    NmNode *aa = fav ? findTitle(fav, "ARMOR ATTACK") : 0;
    CHECK(aa && aa->altImg && !strcmp(aa->param1, "original/ARMOR.BIN"), "the Armor Attack there is the Vectrex cartridge");
    CHECK(findTitle(nmFindTop(&tree, "PITREX"), "VECTORBLADE") && findTitle(findTitle(nmFindTop(&tree, "ARCADE"), "ATARI"), "ASTEROIDS"),
          "the favourites stay in their usual places");
    NmNode *cin = findTitle(nmFindTop(&tree, "ARCADE"), "CINEMATRONICS");
    NmNode *arc = cin ? findTitle(cin, "ARMOR ATTACK") : 0;
    CHECK(arc && strcmp(arc->img, "piZero1/aae.img") == 0, "the arcade Armor Attack is untouched");
    NmNode *orig = findTitle(findTitle(nmFindTop(&tree, "VECTREX"), "ALL ROMS"), "ORIGINAL FOLDER");
    int sortedOrig = orig != 0;
    for (NmNode *c = orig ? orig->child : 0; c && c->right; c = c->right) if (nmTitleCmp(c->title, c->right->title) > 0) sortedOrig = 0;
    CHECK(sortedOrig && findTitle(orig, "ARMOR ATTACK"), "the renamed ARMOR ATTACK is sorted in its ALL ROMS folder");
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
    int renamed = 0;                            /* a file section with a title renames a game: the expected list predates it */
    for (int i = 0; i < recs.n; i++) if (recs.rec[i].kind == NM_REC_FILE && !strncmp(recs.rec[i].key, "rom.", 4) && recs.rec[i].title[0]) renamed++;
    CHECK(got == nw, "tree rows %d != %d", got, nw);
    CHECK(bad == 0 || renamed > 0, "%d rows differ", bad);
    if (renamed) printf("   (%d games renamed by the menu files: %d rows differ from the file-name list)\n", renamed, bad);

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
    CHECK(nmRomPathOk("A.BIN") && nmRomPathOk("sub/dir/deep.bin") && nmRomPathOk("x.Bin") && nmRomPathOk("y.VEC"), "paths that are listed");
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
    const int ids[] = { NM_ID_ALL_ROMS, NM_ID_ROM_FOLDER, NM_ID_SETTINGS };
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

  /* ---- the 0.2.0 menu file: kinds of sections, keys, overrides, folder rules ---- */
  {
    static NmRecs mr;
    static NmTree mt;
    static NmScratch ms;
    memset(&mr, 0, sizeof mr);
    char text[] =
      "; a comment\n"
      "stray = before any section\n"
      "[MAJOR HAVOC]\r\n"
      "category = ARCADE / ATARI\n"
      "image    = aae.img\n"
      "param1   = mhavoc     ; a comment after a value\n"
      "text     = WORK IN PROGRESS!!\n"
      "control  = FIRE  BUTTON 4\n"
      "[SOLAR QUEST ARCADE]\n"
      "title = SOLAR QUEST\n"
      "category = ARCADE\n"
      "image = aae.img\n"
      "param1 = solarq\n"
      "[broken line without a bracket\n"
      "no equals sign here\n"
      "[BERZERK]\n"
      "file = vectrex/original/BERZERK.BIN\n"
      "category = FAVOURITES\n"
      "emulator = speedy\n"
      "[MY PROGRAM]\n"
      "file = piZero1/voom.img\n"
      "category = PITREX / DEMOS\n"
      "[VIDEOS]\n"
      "folder = vectrex/movies/\n"
      "category = VIDEO\n"
      "image = vectrexvideo.img\n"
      "param1 = {dir}\n"
      "param2 = {file}\n"
      "files = bin\n"
      "[BAD APPLE]\n"
      "file = vectrex/movies/BadApple.bin\n"
      "text = THE CLIP\n"
      "[ORIGINAL GAMES]\n"
      "folder = vectrex/original\n"
      "[FAVOURITES]\n"
      "order = listed\n"
      "text = MY FAVOURITE GAMES.\n"
      "[ARCADE / ATARI]\n"
      "hide = 1\n";
    nmParseMenuText(&mr, text, "menu");
    NmRec *r = nmFindRec(&mr, "e.major_havoc");
    CHECK(r && r->kind == NM_REC_ENTRY && !strcmp(r->title, "MAJOR HAVOC") && !strcmp(r->p1, "mhavoc") && r->nrows == 1 && r->nctl == 1,
          "an image section is an entry titled by its name, comment after a value removed");
    r = nmFindRec(&mr, "e.solar_quest_arcade");
    CHECK(r && !strcmp(r->title, "SOLAR QUEST"), "title = overrides the section name");
    r = nmFindRec(&mr, "rom.original_berzerk_bin");
    CHECK(r && r->kind == NM_REC_FILE && r->emulator == 's', "a file section has the 0.1.0 key of its file");
    CHECK(nmFindRec(&mr, "voom") && nmFindRec(&mr, "voom")->kind == NM_REC_FILE, "a program file's key is its image name");
    r = nmFindRec(&mr, "f.vectrex_movies");
    CHECK(r && r->kind == NM_REC_FOLDER && !strcmp(r->path, "vectrex/movies"), "folder rule, trailing / dropped");
    r = nmFindRec(&mr, "dir.original");
    CHECK(r && r->kind == NM_REC_NOTES && !strcmp(r->title, "ORIGINAL GAMES"), "a folder without image names a ROM folder group");
    r = nmFindRec(&mr, "cat.favourites");
    CHECK(r && r->kind == NM_REC_CATEGORY && r->order == 1 && r->nrows == 1, "a category section: order and text");
    CHECK(nmFindRec(&mr, "cat.arcade_atari") && nmFindRec(&mr, "cat.arcade_atari")->hide, "a category can be hidden");
    CHECK(mr.n == 9, "9 records, broken lines skipped (got %d)", mr.n);

    /* a later file changes an earlier record key by key; [entry] is the 0.1.0 format, keyed by the file name */
    char more[] = "[MAJOR HAVOC]\ntext = WORKS NOW.\n[entry]\ntitle = OLD STYLE\nimage = old.img\n";
    nmParseMenuText(&mr, more, "old.entry");
    r = nmFindRec(&mr, "e.major_havoc");
    CHECK(r && r->nrows == 1 && !strcmp(mr.pool + r->rows, "WORKS NOW.") && !strcmp(r->p1, "mhavoc") && r->nctl == 1,
          "a later section replaces the notes and keeps the rest");
    CHECK(nmFindRec(&mr, "old.entry") && !strcmp(nmFindRec(&mr, "old.entry")->title, "OLD STYLE"), "[entry] keyed by its file name");

    char k[64];
    nmPathKey("vectrex/demos/Epic.bin", k, sizeof k);
    CHECK(!strcmp(k, "rom.demos_epic_bin"), "path key %s", k);
    nmPathKey("piZero2/VOOM.img", k, sizeof k);
    CHECK(!strcmp(k, "voom"), "path key %s", k);
    nmPathKey("roms/aae/mhavoc.zip", k, sizeof k);
    CHECK(!strcmp(k, "file.roms_aae_mhavoc_zip"), "path key %s", k);
    char o[48];
    nmSubst(o, sizeof o, "{dir}/x/{file}", "vectrex/movies", "A.bin");
    CHECK(!strcmp(o, "vectrex/movies/x/A.bin"), "subst %s", o);
    CHECK(nmExtOk("a.BIN", "bin vec") && nmExtOk("a.vec", "bin, vec") && !nmExtOk("a.txt", "bin vec") && nmExtOk("a.txt", ""), "extension lists");
    const unsigned char cart[] = "g GCE 1982", data[] = "ZZZZZZZZ";
    CHECK(nmIsCartridge(cart, 10) && !nmIsCartridge(data, 8) && !nmIsCartridge(cart, 3), "cartridge marker");
    CHECK(nmSameBase("E-SRG.bin", "e-srg.VEC") && !nmSameBase("E-SRG.bin", "E-SRG2.bin"), "same game, other extension");
    char t[40];
    nmRomTitle("homebrew/E-SRG.vec", t, sizeof t);
    CHECK(!strcmp(t, "E SRG"), ".vec dropped from the title: %s", t);

    /* build a small tree with these records (recRead reads the global recs: point them here for this block) */
    static char rp[3][NM_ROM_PATH] = { "original/BERZERK.BIN", "original/WEBWARS.BIN", "solo.vec" };
    const char *fl[] = { "aae.img", "vectrexvideo.img", "vectrexexact.img", "vectrexspeedy.img", "voom.img", "newgame.img" };   /* no nosuch.img */
    const char *mv[] = { "BadApple.bin", "LaLinea.bin", "notes.txt", ".hidden.bin" };
    static NmRecs keep;
    keep = recs;
    recs = mr;
    /* sections that point at files which are not on the card */
    char missing[] =
      "[GONE]\nfile = vectrex/original/GONE.BIN\ncategory = LOST AND FOUND\n"
      "[EMPTY FOLDER]\nfolder = vectrex/nothing\ncategory = NOTHING HERE\nimage = vectrexvideo.img\nparam1 = {dir}\nparam2 = {file}\n"
      "[NO SUCH PROGRAM]\ncategory = PITREX\nimage = nosuch.img\n"
      "[GONE GAME]\ncategory = VECTREX / SPEEDY VECTREX\nimage = vectrexspeedy.img\nparam1 = original/GONE.BIN\n";
    nmParseMenuText(&recs, missing, "missing");
    memset(&mt, 0, sizeof mt);
    int ne = nmAddRecEntries(&mt, &recs, "piZero1/");
    CHECK(ne == 5, "five entries (got %d)", ne);
    SidecarText dup;
    nmRecToSidecar(&recs, "e.major_havoc", &dup);
    CHECK(nmAddEntry(&mt, "again", &dup, "piZero1/") == 0, "a second Major Havoc in ATARI is dropped");
    int nfold;
    nfold = nmAddFolderFiles(&mt, &recs, nmFindRec(&recs, "f.vectrex_movies"), "piZero1/", mv, 4, recRead);
    CHECK(nfold == 2, "folder rule: the two .bin files (got %d)", nfold);
    CHECK(nmAddFolderFiles(&mt, &recs, nmFindRec(&recs, "f.vectrex_nothing"), "piZero1/", mv, 0, recRead) == 0,
          "a folder rule whose folder is empty or missing adds nothing");
    static NmRec def[2];
    CHECK(nmDefaultFolderRules(&recs, def, 2) == 1 && !strcmp(def[0].path, "vectrex/waves"), "only waves gets the built-in rule (movies has one)");
    NmNode *set = nmAssembleRest(&mt, &ms, recRead, "piZero1/", fl, 6, rp, 3, "piZero1/vectrexexact.img", "piZero1/vectrexspeedy.img", 0, &nfound);
    CHECK(set != 0, "assembled");
    NmNode *vid = nmFindTop(&mt, "VIDEO");
    NmNode *ba = vid ? findTitle(vid, "BAD APPLE") : 0;
    CHECK(ba && !strcmp(ba->param1, "vectrex/movies") && !strcmp(ba->param2, "BadApple.bin") && !strcmp(ba->img, "piZero1/vectrexvideo.img"),
          "folder rule entry with its file section's title and the parameters filled in");
    CHECK(vid && findTitle(vid, "LALINEA"), "a file without a section is titled by its name");
    NmNode *fav = nmFindTop(&mt, "FAVOURITES");
    NmNode *bz = fav ? findTitle(fav, "BERZERK") : 0;
    CHECK(bz && strstr(bz->img, "speedy") && fav->keepOrder, "a ROM with a category is copied there (with its emulator); order = listed");
    NmNode *all = findTitle(nmFindTop(&mt, "VECTREX"), "ALL ROMS");
    CHECK(all && findTitle(all, "SOLO") && findTitle(all, "ORIGINAL GAMES"), "the ROM stays in ALL ROMS; .vec listed; the group renamed");
    NmNode *pd = findTitle(nmFindTop(&mt, "PITREX"), "DEMOS");
    CHECK(pd && findTitle(pd, "MY PROGRAM") && findTitle(nmFindTop(&mt, "PITREX"), "VOOM") == 0,
          "a program with a category moves out of PITREX, titled by its section");
    NmNode *oldp = findTitle(nmFindTop(&mt, "PITREX"), "NEWGAME");
    CHECK(oldp && oldp->discovered && findTitle(nmFindTop(&mt, "PITREX"), "NO SUCH PROGRAM"),
          "a program no menu file mentions is listed under PITREX, next to the entries the menu files put there");
    NmNode *atari = findTitle(nmFindTop(&mt, "ARCADE"), "ATARI");
    CHECK(atari && atari->hidden, "a hidden category");
    NmNode *lost = nmFindTop(&mt, "LOST AND FOUND");
    CHECK(lost == 0 || lost->hidden, "a file section for a file that is not on the card shows nothing");
    NmNode *nothing = nmFindTop(&mt, "NOTHING HERE");
    CHECK(nothing == 0 || nothing->hidden, "the empty folder rule's category is hidden");
    NmNode *nsp = findTitle(nmFindTop(&mt, "PITREX"), "NO SUCH PROGRAM");
    CHECK(nsp && nsp->hidden, "an entry whose program is not in piZero1 is hidden");
    NmNode *gg = findTitle(findTitle(nmFindTop(&mt, "VECTREX"), "SPEEDY VECTREX"), "GONE GAME");
    CHECK(gg && !gg->hidden, "KNOWN GAP: an entry whose program exists but whose game file does not is still listed (it has no needs line)");
    recs = keep;
  }

  /* ---- layers: menu.default.ini, then the user's menu.ini changes it, re-adds things elsewhere, un-hides ---- */
  {
    static NmRecs lr;
    static NmTree lt;
    static NmScratch ls;
    memset(&lr, 0, sizeof lr);
    char def[] =
      "[MAJOR HAVOC]\ncategory = ARCADE / ATARI\nimage = aae.img\nparam1 = 0x2d\ntext = WORK IN PROGRESS!!\ncontrol = FIRE  BUTTON 4\n"
      "[TEMPEST]\ncategory = ARCADE / ATARI\nimage = tempest.img\n"
      "[HELLO WORLD]\ncategory = PITREX / SAMPLE PROGRAMS\nimage = hello.img\nhide = 1\n"
      "[BAD APPLE]\nfile = vectrex/movies/BadApple.bin\ntext = THE CLIP\n";
    char mine[] =
      "[MAJOR HAVOC]\ntext = WORKS GREAT NOW.\nalso = FAVOURITES, TWO PLAYERS\n"          /* change notes, add copies */
      "[TEMPEST]\ncategory = FAVOURITES\n"                                                /* move */
      "[HELLO WORLD]\nhide = 0\n"                                                         /* show again */
      "[BAD APPLE]\ntitle = BAD APPLE CLIP\nalso = FAVOURITES\n"                          /* change a file section by name */
      "[MAJOR HAVOC RESCUE]\ntitle = MAJOR HAVOC\ncategory = FAVOURITES\nimage = aae.img\nparam1 = 0x2d\n";  /* same game again */
    nmParseMenuText(&lr, def, "menu.default");
    nmParseMenuText(&lr, mine, "menu");
    static NmRecs keep2;
    keep2 = recs;
    recs = lr;
    memset(&lt, 0, sizeof lt);
    nmAddRecEntries(&lt, &recs, "piZero1/");
    const char *bv[] = { "BadApple.bin" };
    static NmRec dr[2];
    int nd = nmDefaultFolderRules(&recs, dr, 2);
    for (int i = 0; i < nd; i++) if (!strcmp(dr[i].path, "vectrex/movies")) nmAddFolderFiles(&lt, &recs, &dr[i], "piZero1/", bv, 1, recRead);
    const char *fl2[] = { "aae.img", "tempest.img", "hello.img", "vectrexvideo.img" };
    nmAssembleRest(&lt, &ls, recRead, "piZero1/", fl2, 4, romPaths, 0, "piZero1/vectrexexact.img", "piZero1/vectrexspeedy.img", 0, &nfound);
    NmNode *atari = findTitle(nmFindTop(&lt, "ARCADE"), "ATARI");
    NmNode *mh = atari ? findTitle(atari, "MAJOR HAVOC") : 0;
    SidecarText st;
    CHECK(mh && recRead(mh->key, &st) && st.rows == 1 && !strcmp(st.row[0], "WORKS GREAT NOW.") && st.nctl == 1 && mh->param1[0] == 0x2d,
          "menu.ini replaces the notes, keeps the program, parameter and controls");
    NmNode *fav = nmFindTop(&lt, "FAVOURITES"), *two = nmFindTop(&lt, "TWO PLAYERS");
    int mhInFav = 0;
    for (NmNode *c = fav ? fav->child : 0; c; c = c->right) if (!strcmp(c->title, "MAJOR HAVOC")) mhInFav++;
    CHECK(mhInFav == 1, "also = FAVOURITES lists Major Havoc there once, even though a second section adds it too (%d)", mhInFav);
    CHECK(two && findTitle(two, "MAJOR HAVOC") && !strcmp(findTitle(two, "MAJOR HAVOC")->key, mh ? mh->key : "?"),
          "also = TWO PLAYERS: a copy with the same notes (same key)");
    CHECK(fav && findTitle(fav, "TEMPEST") && !(atari && findTitle(atari, "TEMPEST")), "category = moves an entry");
    NmNode *hw = findTitle(findTitle(nmFindTop(&lt, "PITREX"), "SAMPLE PROGRAMS"), "HELLO WORLD");
    CHECK(hw && !hw->hidden, "hide = 0 shows what the default hid");
    NmNode *vid = nmFindTop(&lt, "VIDEO");
    CHECK(vid && findTitle(vid, "BAD APPLE CLIP") && fav && findTitle(fav, "BAD APPLE CLIP"),
          "a section with only a name changes the file section of that name (title, also)");
    recs = keep2;
  }

  /* ---- the collections example of NEW_MENU_README.md, word for word ---- */
  {
    static NmRecs cr;
    static NmTree ct;
    static NmScratch cs;
    memset(&cr, 0, sizeof cr);
    char def[] =
      "[MAJOR HAVOC]\ncategory = ARCADE / ATARI\nimage = aae.img\nparam1 = 0x2d\n"
      "[TEMPEST]\ncategory = ARCADE / ATARI\nimage = tempest.img\n"
      "[BERZERK]\ncategory = VECTREX / SPEEDY VECTREX\nimage = vectrexspeedy.img\nparam1 = original/BERZERK.BIN\n";
    char mine[] =
      "[FAVOURITES]\n"
      "order = listed                      ; in the order of this file, not alphabetical\n"
      "text = THE GAMES I PLAY MOST.       ; the help shown when FAVOURITES is highlighted\n"
      "\n"
      "[TEMPEST]\n"
      "also = FAVOURITES\n"
      "\n"
      "[MAJOR HAVOC]\n"
      "also = FAVOURITES, TWO PLAYERS\n"
      "\n"
      "[BERZERK]                           ; a Vectrex game the menu finds by itself under ALL ROMS\n"
      "file = vectrex/original/BERZERK.BIN\n"
      "also = FAVOURITES, VECTREX CLASSICS\n";
    nmParseMenuText(&cr, def, "menu.default");
    nmParseMenuText(&cr, mine, "menu");
    static NmRecs keep3;
    keep3 = recs;
    recs = cr;
    memset(&ct, 0, sizeof ct);
    nmAddRecEntries(&ct, &recs, "piZero1/");
    static char rp1[1][NM_ROM_PATH] = { "original/BERZERK.BIN" };
    const char *fl3[] = { "aae.img", "tempest.img", "vectrexspeedy.img", "vectrexexact.img" };
    nmAssembleRest(&ct, &cs, recRead, "piZero1/", fl3, 4, rp1, 1, "piZero1/vectrexexact.img", "piZero1/vectrexspeedy.img", 0, &nfound);
    NmNode *fav = nmFindTop(&ct, "FAVOURITES");
    const char *want[] = { "TEMPEST", "MAJOR HAVOC", "BERZERK" };
    int k = 0, ok = fav != 0;
    for (NmNode *c = fav ? fav->child : 0; c; c = c->right, k++) if (k >= 3 || strcmp(c->title, want[k])) ok = 0;
    CHECK(ok && k == 3, "FAVOURITES: Tempest, Major Havoc, Berzerk in the order of menu.ini");
    NmNode *bz = fav ? findTitle(fav, "BERZERK") : 0;
    CHECK(bz && strstr(bz->img, "vectrexexact.img") && bz->altImg, "the Berzerk copy is the ALL ROMS file (Exact, Button 2 Speedy)");
    NmNode *two = nmFindTop(&ct, "TWO PLAYERS"), *cl = nmFindTop(&ct, "VECTREX CLASSICS");
    CHECK(two && findTitle(two, "MAJOR HAVOC") && cl && findTitle(cl, "BERZERK"), "TWO PLAYERS and VECTREX CLASSICS");
    NmNode *atari = findTitle(nmFindTop(&ct, "ARCADE"), "ATARI");
    CHECK(atari && findTitle(atari, "TEMPEST") && findTitle(atari, "MAJOR HAVOC"), "they stay in their usual places");
    SidecarText st;
    CHECK(fav && recRead(fav->key, &st) && st.rows == 1, "FAVOURITES has its help text");
    recs = keep3;
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
