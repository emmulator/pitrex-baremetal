/* Host test for pitrex/loader/newMenuLogic.h on the real compiled-in menu.
 *
 *   python3 -I docs/tools/gen_menu_tree.py pitrex/loader/loaderMain.c > /tmp/menu_tree.inc
 *   gcc -Wall -Wextra -I pitrex/loader -I /tmp -o /tmp/t docs/tools/test_new_menu.c
 *   /tmp/t <image folder listing, one name per line> <expected discovered bases, one per line> \
 *          <ROM paths under vectrex/, one per line> <expected ROM tree, as written by the python snippet in docs/menu/07-new-menu.md>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* the fields of the loader's MenuItemNew that the menu logic uses (same names and order) */
typedef struct MenuItemNew
{
  int id;
  char *DISPLAYNAME;
  char *dir;
  char *img;
  char *param1;
  char *param2;
  const signed char *icon;
  const signed char *smallIcon;
  void *parent;
  void *child;
  void *left;
  void *right;
  char *scrolltext;
  char *text[12];
} MenuItemNew;

#include "newMenuLogic.h"
#include "menu_tree.inc"

static int fails = 0;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: " __VA_ARGS__); printf("\n"); fails++; } } while (0)

static NmTree tree;
static NmNav nav;

static int selectTitle(const char *title)
{
  for (int i = 0; i < nav.count; i++)
    if (strcmp(nav.list[i]->title, title) == 0) { nav.sel = i; nmFixWindow(&nav); return 1; }
  return 0;
}

static int countLeaves(NmNode *n)
{
  int c = 0;
  for (; n != 0; n = n->right)
  {
    if (n->child != 0) c += countLeaves(n->child);
    else if (n->img != 0) c++;
  }
  return c;
}

int main(int argc, char **argv)
{
  mockInit();
  mockTopsInit();
  nmBuildTree(&tree, mockTops, 5);
  printf("compiled nodes: %d, nodes in the tree: %d\n", MOCK_COUNT, tree.used);

  CHECK(tree.ntops == 5, "five compiled top categories");
  const char *want[] = { "VECTREX", "ARCADE", "COMPUTER", "VIDEO", "AUDIO" };
  for (int i = 0; i < 5; i++) CHECK(strcmp(tree.tops[i]->title, want[i]) == 0, "top %d is %s", i, tree.tops[i]->title);
  CHECK(tree.used <= MOCK_COUNT, "no more nodes than compiled entries");

  NmNode *settings = nmAddTop(&tree, "SETTINGS", 0);
  CHECK(settings != 0, "settings node");

  /* programs found on the card */
  static char known[NM_MAX_IMAGES][NM_NAME_LEN];
  int nknown = nmCollectImages(tree.tops, tree.ntops, known, NM_MAX_IMAGES);
  printf("distinct images launched by the compiled menu: %d\n", nknown);

  static char listing[256][64];
  const char *files[256];
  int nf = 0;
  if (argc > 1)
  {
    FILE *f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 2; }
    while (nf < 256 && fgets(listing[nf], 64, f))
    {
      listing[nf][strcspn(listing[nf], "\r\n")] = 0;
      files[nf] = listing[nf];
      nf++;
    }
    fclose(f);
  }
  static char found[NM_MAX_PROGS][NM_NAME_LEN];
  int nfound = nmDiscover(files, nf, known, nknown, found, NM_MAX_PROGS);
  printf("files listed: %d, programs discovered: %d\n", nf, nfound);
  if (argc > 2)
  {
    static char exp[NM_MAX_PROGS][NM_NAME_LEN];
    int ne = 0;
    FILE *f = fopen(argv[2], "r");
    if (!f) { perror(argv[2]); return 2; }
    char l[64];
    while (ne < NM_MAX_PROGS && fgets(l, sizeof l, f)) { l[strcspn(l, "\r\n")] = 0; if (l[0]) strcpy(exp[ne++], l); }
    fclose(f);
    CHECK(ne == nfound, "discovered %d, expected %d", nfound, ne);
    for (int i = 0; i < ne && i < nfound; i++) CHECK(strcmp(exp[i], found[i]) == 0, "discovered[%d] '%s' != expected '%s'", i, found[i], exp[i]);
    for (int i = 0; i < nfound; i++) printf("   program: %s\n", found[i]);
  }
  CHECK(nmDiscover(0, 0, known, nknown, found, NM_MAX_PROGS) == 0, "no files, no programs");

  static char paths[NM_MAX_PROGS][56];
  NmNode *programs = nmAddTop(&tree, "PROGRAMS", 6000 + 1);
  int added = nmAddPrograms(&tree, programs, "piZero1/", found, nfound, paths);
  CHECK(added == nfound, "programs added");
  if (added > 0)
  {
    CHECK(programs->child != 0 && programs->child->discovered, "first program is discovered");
    CHECK(strncmp(programs->child->img, "piZero1/", 8) == 0, "program image path");
  }

  /* navigation on the real tree */
  nmInit(&nav, &tree);
  CHECK(nav.count == 7, "level 1 lists 7 categories, got %d", nav.count);
  CHECK(strcmp(nav.list[0]->title, "VECTREX") == 0, "VECTREX first");
  CHECK(strcmp(nav.list[nav.count - 1]->title, "PROGRAMS") == 0 || strcmp(nav.list[nav.count - 1]->title, "SETTINGS") == 0, "last is PROGRAMS or SETTINGS");

  CHECK(selectTitle("ARCADE"), "find ARCADE");
  CHECK(nmEnter(&nav) == 0, "entering ARCADE goes in");
  CHECK(nav.depth == 1, "depth 1");
  char path[96];
  nmPath(&nav, path, sizeof path);
  CHECK(strcmp(path, "ARCADE") == 0, "breadcrumb ARCADE, got '%s'", path);
  CHECK(selectTitle("ATARI"), "ATARI under ARCADE");
  CHECK(selectTitle("ALL AAE GAMES"), "ALL AAE GAMES under ARCADE");
  NmNode *aae = nmCurrent(&nav);
  CHECK(aae->img != 0 && strstr(aae->img, "aae.img") != 0 && aae->param1 == 0, "ALL AAE GAMES launches aae.img with no parameter");
  CHECK(selectTitle("ATARI"), "back to ATARI");
  CHECK(nmEnter(&nav) == 0, "entering ATARI");
  nmPath(&nav, path, sizeof path);
  CHECK(strcmp(path, "ARCADE > ATARI") == 0, "breadcrumb, got '%s'", path);
  CHECK(selectTitle("MAJOR HAVOC"), "MAJOR HAVOC under ATARI");
  NmNode *mh = nmEnter(&nav);
  CHECK(mh != 0 && mh->param1 != 0 && mh->param1[0] == 0x2d && mh->param1[1] == 0, "Major Havoc passes game byte 0x2d");
  CHECK(mh != 0 && mh->src != 0 && mh->src->id == 2009, "Major Havoc keeps its compiled entry (id 2009)");
  CHECK(nav.count == 12 || nav.count > 8, "ATARI has more rows than fit on screen (%d)", nav.count);
  nav.sel = 0; nav.top = 0;
  for (int i = 0; i < nav.count + 3; i++) nmMove(&nav, 1);   /* moving past the end stops at the last row */
  CHECK(nav.sel >= nav.top && nav.sel < nav.top + NM_ROWS, "window follows the selection");
  CHECK(nmBack(&nav) == 1 && nav.depth == 1, "back to ARCADE");
  CHECK(strcmp(nmCurrent(&nav)->title, "ATARI") == 0, "selection restored to ATARI, got %s", nmCurrent(&nav)->title);
  CHECK(nmBack(&nav) == 1 && nav.depth == 0, "back to the top");
  CHECK(nmBack(&nav) == 0, "cannot go above the top");

  /* speedy / exact groups under VECTREX */
  CHECK(selectTitle("VECTREX") && nmEnter(&nav) == 0, "enter VECTREX");
  CHECK(selectTitle("SPEEDY VECTREX"), "speedy group is named");
  CHECK(nmEnter(&nav) == 0, "enter speedy");
  CHECK(selectTitle("WEB WARS"), "leading spaces trimmed");
  NmNode *ww = nmEnter(&nav);
  CHECK(ww != 0 && strstr(ww->img, "vectrexspeedy.img") != 0 && strcmp(ww->param1, "original/WEBWARS.BIN") == 0, "Web Wars launches speedy with its ROM");
  CHECK(nmBack(&nav) == 1 && nav.depth == 1, "back from speedy to VECTREX");
  CHECK(selectTitle("EXACT VECTREX") && nmEnter(&nav) == 0, "enter exact");
  CHECK(selectTitle("PICK A GAME"), "PICK A GAME present");
  NmNode *pick = nmEnter(&nav);
  CHECK(pick != 0 && pick->id == 9999, "PICK A GAME is a leaf with id 9999");
  nmBack(&nav);
  nmBack(&nav);

  /* hiding */
  CHECK(selectTitle("VIDEO"), "find VIDEO");
  tree.tops[3]->hidden = 1;
  nmRebuild(&nav);
  CHECK(!selectTitle("VIDEO"), "hidden category disappears");
  tree.tops[3]->hidden = 0;
  nmRebuild(&nav);
  CHECK(selectTitle("VIDEO"), "unhidden category returns");

  /* every launchable entry is reachable by walking the tree */
  int leaves = 0;
  for (int i = 0; i < tree.ntops; i++) leaves += tree.tops[i]->child ? countLeaves(tree.tops[i]->child) : 0;
  printf("launchable entries in the tree: %d\n", leaves);
  CHECK(leaves >= 66, "launchable entries: %d", leaves);

  /* ---- caveat tags from the author's notes ---- */
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
      { "LUNAR LANDER", 0 }, { "BLACK WIDOW", NM_TAG_WIP | NM_TAG_NOSOUND }, { "SPEEDFREAK", NM_TAG_NOSOUND },
      { "SPACE WARS", NM_TAG_BROKEN }, { "OMEGA RACE", NM_TAG_WIP | NM_TAG_NOSOUND }, { "TEMPEST", NM_TAG_WIP | NM_TAG_NOSOUND },
      { "GRAVITAR", NM_TAG_WIP | NM_TAG_NOSOUND }, { "BARRIER", 0 }, { "DEMON", 0 }, { "ZBLAST", 0 }, { "REBEL DEFENDER", NM_TAG_NOSOUND },
      { "BZ2 BETA", NM_TAG_NOSOUND }, { "ASTEROIDS", 0 } };
    int tagged = 0, total = 0;
    for (int i = 0; i < tree.used; i++)
    {
      NmNode *n = &tree.pool[i];
      if (n->img != 0 && !n->discovered) { total++; if (n->tags) tagged++; }
      for (unsigned k = 0; k < sizeof expect / sizeof expect[0]; k++)
        if (n->img != 0 && strcmp(n->title, expect[k].title) == 0)
          CHECK(n->tags == expect[k].tags, "tags of %s: %d, expected %d", n->title, n->tags, expect[k].tags);
    }
    printf("compiled entries with a caveat tag: %d of %d\n", tagged, total);
    CHECK(tagged == 27 && total == 69, "27 of 69 compiled entries carry a caveat, got %d of %d", tagged, total);
  }

  /* ---- Vectrex ROMs (optional arguments 3 and 4) ---- */
  if (argc > 4)
  {
    static char rpaths[NM_MAX_ROMS][NM_ROM_PATH];
    int nr = 0;
    FILE *f = fopen(argv[3], "r");
    if (!f) { perror(argv[3]); return 2; }
    char l[256];
    while (nr < NM_MAX_ROMS && fgets(l, sizeof l, f))
    {
      l[strcspn(l, "\r\n")] = 0;
      if (strlen(l) < NM_ROM_PATH) strcpy(rpaths[nr++], l);
    }
    fclose(f);

    NmNode *vectrex = tree.tops[0];
    NmNode *roms = nmNewNode(&tree);
    nmCopy(roms->title, "ALL ROMS", (int) sizeof roms->title);
    nmAppendChild(vectrex, roms);
    int nadded = nmAddRoms(&tree, roms, rpaths, nr, "piZero1/vectrexexact.img", "piZero1/vectrexspeedy.img");

    /* expected: one line per node, G: group, E: entry in a group, R: entry in vectrex/ */
    static char want[NM_MAX_NODES][64];
    int nw = 0;
    f = fopen(argv[4], "r");
    if (!f) { perror(argv[4]); return 2; }
    while (nw < NM_MAX_NODES && fgets(want[nw], 64, f)) { want[nw][strcspn(want[nw], "\r\n")] = 0; nw++; }
    fclose(f);

    int got = 0, bad = 0;
    for (NmNode *c = roms->child; c != 0; c = c->right)
    {
      char line[80];
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
    printf("ROM paths read: %d, ROMs listed: %d, tree rows: %d (expected %d)\n", nr, nadded, got, nw);
    CHECK(got == nw, "tree rows %d != %d", got, nw);
    CHECK(bad == 0, "%d rows differ", bad);

    /* parameters, images, keys */
    NmNode *first = 0;
    for (NmNode *c = roms->child; c != 0 && first == 0; c = c->right) first = c->child ? c->child : c;
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
  }



  /* compiled game groups: demos found in vectrex/demos are added, then SPEEDY, EXACT and DEMOS are sorted */
  {
    NmNode *demos = nmFindById(tree.tops[0], 108);
    CHECK(demos != 0, "DEMOS group found");
    int before = 0, after = 0;
    for (NmNode *c = demos->child; c != 0; c = c->right) before++;
    static char dpaths[4][NM_ROM_PATH] = { "demos/epic.bin", "DEMOS/Zed.bin", "demos/sub/deep.bin", "other/x.bin" };
    int added = nmAddMissingRoms(&tree, demos, dpaths, 4, "demos/", "piZero1/vectrexexact.img", "piZero1/vectrexspeedy.img");
    for (NmNode *c = demos->child; c != 0; c = c->right) after++;
    CHECK(added == 1 && after == before + 1, "only the missing demo (Zed) is added: added=%d", added);
    nmSortChildren(demos);
    int sorted = 1;
    for (NmNode *c = demos->child; c != 0 && c->right != 0; c = c->right)
      if (nmTitleCmp(c->title, c->right->title) > 0) sorted = 0;
    CHECK(sorted, "DEMOS is alphabetical");
    NmNode *speedy = nmFindById(tree.tops[0], 102);
    nmSortChildren(speedy);
    CHECK(strcmp(speedy->child->title, "BERZERK") == 0, "SPEEDY starts with BERZERK, got %s", speedy->child->title);
  }


  /* everything below the top level is alphabetical after nmSortAll (ALL ROMS keeps folders first) */
  {
    for (int i = 0; i < tree.ntops; i++)
      if (tree.tops[i]->child != 0) { nmSortChildren(tree.tops[i]); nmSortAll(tree.tops[i]->child); }
    int unsorted = 0;
    for (int i = 0; i < tree.used; i++)
    {
      NmNode *n = &tree.pool[i];
      if (n->id == NM_ID_ALL_ROMS || n->child == 0) continue;
      for (NmNode *c = n->child; c != 0 && c->right != 0; c = c->right)
        if (nmTitleCmp(c->title, c->right->title) > 0) { unsorted++; printf("   not sorted under %s: %s > %s\n", n->title, c->title, c->right->title); }
    }
    CHECK(unsorted == 0, "%d lists not alphabetical", unsorted);
    if (getenv("NM_DUMP"))
      for (int i = 0; i < tree.ntops; i++)
      {
        printf("TOP %s\n", tree.tops[i]->title);
        int k = 0;
        for (NmNode *c = tree.tops[i]->child; c != 0; c = c->right, k++)
        {
          printf("  [%d] %s\n", k, c->title);
          int j = 0;
          if (strcmp(c->title, "ALL ROMS") != 0) for (NmNode *g = c->child; g != 0; g = g->right, j++) printf("      [%d] %s\n", j, g->title);
        }
      }
  }


  /* titles: no brackets, and "SBT" is not part of a name */
  {
    int bad = 0, sw = 0;
    for (int i = 0; i < tree.used; i++)
    {
      const char *t = tree.pool[i].title;
      int len = (int) strlen(t);
      if (strchr(t, '(') || strchr(t, ')') || (len > 4 && strcmp(t + len - 4, " SBT") == 0)) bad++;
      if (strcmp(t, "STAR WARS") == 0) sw++;
    }
    CHECK(bad == 0, "%d titles with brackets or SBT", bad); if (bad) for (int i = 0; i < tree.used; i++) { const char *t = tree.pool[i].title; if (strchr(t, 0x28) || strchr(t, 0x29)) printf("   bracket title: %s\n", t); }
    CHECK(sw == 1, "Star Wars is listed as STAR WARS");
  }

  /* help texts: every category says what it is, using only characters the PiTrex font draws */
  {
    const int ids[] = { 1, 2, 3, 4, 5, 102, 107, 108, NM_ID_PROGRAMS, NM_ID_ALL_ROMS, NM_ID_ROM_FOLDER, 0 };
    for (unsigned i = 0; i < sizeof ids / sizeof ids[0]; i++)
    {
      NmNode n;
      memset(&n, 0, sizeof n);
      n.id = ids[i];
      const char *lines[6];
      int nl = nmHelp(&n, lines, 6);
      CHECK(nl > 0, "help for id %d", ids[i]);
      for (int l = 0; l < nl; l++)
      {
        CHECK(strlen(lines[l]) <= 46, "help line fits (id %d): %s", ids[i], lines[l]);
        for (const char *c = lines[l]; *c; c++)
          CHECK((*c >= 'A' && *c <= 'Z') || (*c >= '0' && *c <= '9') || *c == ' ' || *c == '.' || *c == '!',
                "help uses a drawable character, got '%c' in: %s", *c, lines[l]);
      }
    }
    NmNode entry;
    memset(&entry, 0, sizeof entry);
    entry.id = 1070;
    entry.img = "x.img";
    const char *lines[6];
    CHECK(nmHelp(&entry, lines, 6) == 0, "a game entry has no category help");
  }

  printf(fails ? "FAIL (%d)\n" : "PASS\n", fails);
  return fails != 0;
}
