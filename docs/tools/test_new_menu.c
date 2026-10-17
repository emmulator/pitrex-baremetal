/* Host test for pitrex/loader/newMenuLogic.h on the real compiled-in menu.
 *
 *   python3 -I docs/tools/gen_menu_tree.py pitrex/loader/loaderMain.c > /tmp/menu_tree.inc
 *   gcc -Wall -Wextra -I pitrex/loader -I /tmp -o /tmp/t docs/tools/test_new_menu.c
 *   /tmp/t <image folder listing, one name per line> <expected discovered bases, one per line>
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
  static char known[NM_MAX_NODES][NM_NAME_LEN];
  int nknown = nmCollectImages(tree.tops, tree.ntops, known, NM_MAX_NODES);
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
  CHECK(strcmp(path, "ARCADE / ATARI") == 0, "breadcrumb, got '%s'", path);
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

  printf(fails ? "FAIL (%d)\n" : "PASS\n", fails);
  return fails != 0;
}
