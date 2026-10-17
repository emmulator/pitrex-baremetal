// The new list menu: drawing, input, launching, and scanning the card.
// Shared by the loader (loaderMain.c) and the desktop preview (docs/tools/menu_preview).
//
// The includer provides before this header: MenuItemNew and the compiled-in tree
// (vectrexMenu, arcadeMenu, computerMenu, videoMenu, audioMenu, exactPickOne), newMenuLogic.h,
// menuSidecar.h, menuSidecarIo.h, the PiTrex library calls v_printString, v_directReadButtons,
// v_SettingsGUI, v_saveIni, the globals currentButtonState, currentJoy1X/Y, currentMenuItem,
// selectionMade, the functions loadAndStart(), testRom(), displayMenu(), opendir/readdir/closedir
// with DT_DIR, and the macros IMG_FILE_PREFIX and NM_IMG_DIR. NM_CARD is the prefix of paths
// on the card ("0:/" on the PiTrex, "" on the desktop). Documentation: docs/menu/07-new-menu.md
#ifndef NEW_MENU_UI_H
#define NEW_MENU_UI_H

#ifndef NM_CARD
#define NM_CARD "0:/"
#endif
#ifndef RASPPI
#define RASPPI 1
#endif

// ---------------------------------------------------------------------------
// The new menu (loader.ini: NEW_MENU = 1). A vertical list of categories; you drill down.
// The logic is in newMenuLogic.h (tested on a PC), the entries come from the compiled
// menu above plus the programs found in the image folder. Documentation: docs/menu/07-new-menu.md
// ---------------------------------------------------------------------------
#if RASPPI != 1
#define NM_IMG_DIR NM_CARD "piZero2"
#else
#define NM_IMG_DIR NM_CARD "piZero1"
#endif

#define NM_MAX_FILES 160
static NmTree nmTree;
static NmNav nmNav;
static NmNode *nmSettingsNode = 0;
static int nmReady = 0;
static int nmBrowse = 0;                       // 1 while the old PICK A GAME screen is in charge
static char nmKnown[NM_MAX_IMAGES][NM_NAME_LEN];
static char nmRomPaths[NM_MAX_ROMS][NM_ROM_PATH];
static char nmFound[NM_MAX_PROGS][NM_NAME_LEN];
static char nmPaths[NM_MAX_PROGS][56];
static char nmFileNames[NM_MAX_FILES][32];
static const char *nmFilePtr[NM_MAX_FILES];
static char nmSideNames[320][36];               // names of the files in menu/ (without .ini), lower case
static int nmSideCount = 0;
static MenuItemNew nmLaunchItem;               // stand-in for a program found on the card
static NmNode *nmNotesOwner = 0;
static int nmHeld = 0;                         // frames the stick has been held up/down
static int nmSideLatch = 0;                    // stick left/right already acted on
static uint8_t nmLastButtons = 0;

// Remember which sidecar files exist, so a missing one is never opened: every failed open
// writes a line to the serial console and costs time.
static void nmListSidecars(void)
{
  nmSideCount = 0;
  DIR *dp = opendir(NM_CARD "menu");
  if (dp == 0) return;
  dirent *fi;
  while ((fi = readdir(dp)) != 0 && nmSideCount < 320)
  {
    int len = (int) strlen(fi->d_name);
    if (fi->d_type == DT_DIR || len <= 4 || len > 40) continue;
    if (!nmEqNoCase(fi->d_name + len - 4, ".ini")) continue;
    for (int i = 0; i < len - 4 && i < 35; i++) nmSideNames[nmSideCount][i] = nmLower(fi->d_name[i]);
    nmSideNames[nmSideCount][len - 4 > 35 ? 35 : len - 4] = 0;
    nmSideCount++;
  }
  closedir(dp);
}

static int nmHasSidecar(const char *key)
{
  for (int i = 0; i < nmSideCount; i++)
    if (strcmp(nmSideNames[i], key) == 0) return 1;
  return 0;
}

static void nmApplySidecars(void)
{
  SidecarText t;
  for (int i = 0; i < nmTree.used; i++)
  {
    NmNode *n = &nmTree.pool[i];
    if (n->img == 0 || n->id == 9999) continue;
    char key[96];
    if (n->discovered) nmCopy(key, n->key, (int) sizeof key);
    else if (!sidecarKey(n->img, n->param1, n->param2, key, sizeof key)) continue;
    if (!nmHasSidecar(key)) continue;
    if (!sidecarReadKey(key, &t)) continue;
    if (t.hide) n->hidden = 1;
    if (t.found)                               // a sidecar's notes replace the compiled ones, so do their caveats
    {
      n->tags = 0;
      for (int r = 0; r < t.rows; r++) n->tags |= nmTagsOfLine(t.row[r]);
    }
    if (n->discovered && t.title[0] != 0) nmCopy(n->title, t.title, (int) sizeof n->title);
    if (t.emulator != 0) nmChooseEmulator(n, t.emulator);
  }
}

// All .bin files under vectrex/ (two folder levels), as paths relative to vectrex/.
// Only one directory is open at a time: the file layer has a single directory handle.
static int nmScanRoms(void)
{
  static char dirs[24][NM_ROM_PATH];
  int ndirs = 1;
  int di = 0;
  int nroms = 0;
  dirs[0][0] = 0;
  while (di < ndirs)
  {
    char abs[112];
    nmCopy(abs, NM_CARD "vectrex", (int) sizeof abs);
    if (dirs[di][0] != 0)
    {
      strcat(abs, "/");
      strcat(abs, dirs[di]);
    }
    int depth = 0;
    for (const char *q = dirs[di]; *q != 0; q++) if (*q == '/') depth++;
    if (dirs[di][0] != 0) depth++;

    DIR *dp = opendir(abs);
    if (dp != 0)
    {
      dirent *fi;
      while ((fi = readdir(dp)) != 0)
      {
        if (fi->d_name[0] == '.') continue;
        char path[NM_ROM_PATH + 32];
        path[0] = 0;
        if (dirs[di][0] != 0)
        {
          nmCopy(path, dirs[di], (int) sizeof path);
          strcat(path, "/");
        }
        strncat(path, fi->d_name, sizeof(path) - strlen(path) - 1);
        if (strlen(path) >= NM_ROM_PATH) continue;
        if (fi->d_type == DT_DIR)
        {
          if (ndirs < 24 && depth < 2) nmCopy(dirs[ndirs++], path, NM_ROM_PATH);
        }
        else if (nroms < NM_MAX_ROMS && nmRomPathOk(path))
        {
          nmCopy(nmRomPaths[nroms++], path, NM_ROM_PATH);
        }
      }
      closedir(dp);
    }
    di++;
  }
  return nroms;
}

static void newMenuInit(void)
{
  MenuItemNew *roots[5] = { &vectrexMenu, &arcadeMenu, &computerMenu, &videoMenu, &audioMenu };
  nmBuildTree(&nmTree, roots, 5);

  // programs in the image folder that the compiled menu does not start
  int nfiles = 0;
  DIR *dp = opendir(NM_IMG_DIR);
  if (dp != 0)
  {
    dirent *fi;
    while ((fi = readdir(dp)) != 0 && nfiles < NM_MAX_FILES)
    {
      if (fi->d_type == DT_DIR) continue;
      nmCopy(nmFileNames[nfiles], fi->d_name, (int) sizeof nmFileNames[0]);
      nmFilePtr[nfiles] = nmFileNames[nfiles];
      nfiles++;
    }
    closedir(dp);
  }
  int nknown = nmCollectImages(nmTree.tops, nmTree.ntops, nmKnown, NM_MAX_NODES);
  int nfound = nmDiscover(nmFilePtr, nfiles, nmKnown, nknown, nmFound, NM_MAX_PROGS);
  if (nfound > 0)
  {
    NmNode *programs = nmAddTop(&nmTree, "PROGRAMS", 9001);
    if (programs != 0) nmAddPrograms(&nmTree, programs, IMG_FILE_PREFIX, nmFound, nfound, nmPaths);
  }
  nmSettingsNode = nmAddTop(&nmTree, "SETTINGS", 0);

  // Vectrex ROMs on the card: VECTREX / ALL ROMS, started with Exact (Button 2: Speedy)
  int nroms = nmScanRoms();
  if (nroms > 0)
  {
    NmNode *romsNode = nmNewNode(&nmTree);
    if (romsNode != 0)
    {
      nmCopy(romsNode->title, "ALL ROMS", (int) sizeof romsNode->title);
      nmAppendChild(nmTree.tops[0], romsNode);
      nmAddRoms(&nmTree, romsNode, nmRomPaths, nroms, IMG_FILE_PREFIX "vectrexexact.img", IMG_FILE_PREFIX "vectrexspeedy.img");
    }
  }

  nmListSidecars();
  nmApplySidecars();
  nmInit(&nmNav, &nmTree);
  nmNotesOwner = 0;
  printf("New menu: %i nodes, %i programs, %i ROMs found\n\r", nmTree.used, nfound, nroms);
}

static void nmLaunch(NmNode *n, int button, int useOtherEmulator)
{
  if (n->src != 0)
  {
    loadAndStart(n->src, button);              // compiled entry: exactly as the old menu does it
  }
  else
  {
    nmLaunchItem.id = 0;
    nmLaunchItem.img = (char *) (useOtherEmulator ? nmOtherEmulator(n) : n->img);
    nmLaunchItem.param1 = (char *) n->param1;
    nmLaunchItem.param2 = (char *) n->param2;
    loadAndStart(&nmLaunchItem, button);
  }
}

// button: 4 or 1 (the button that was pressed), 2 = start a ROM with the other emulator, 0 = stick
static void nmActivate(int button)
{
  if (button == 2)
  {
    NmNode *c = nmCurrent(&nmNav);
    if (c != 0 && !nmHasVisibleChild(c) && c->altImg != 0) nmLaunch(c, 4, 1);
    return;
  }
  NmNode *n = nmEnter(&nmNav);                 // goes into a category, else returns the entry
  if (n == 0) return;
  if (n == nmSettingsNode)
  {
    while ((v_directReadButtons() & 0x0f) != 0) { }
    v_SettingsGUI(1);
    v_saveIni("vectrexInterface.ini");
    return;
  }
  if (n->id == 9999)                           // PICK A GAME: the old file browser takes over
  {
    nmBrowse = 1;
    currentMenuItem = &exactPickOne;
    selectionMade = 1;                         // ignore the stick until it is centred
    return;
  }
  if (n->img == 0) return;                     // an empty category
  nmLaunch(n, button == 1 ? 1 : 4, 0);
}

static void nmDrawNotes(NmNode *n)
{
  if (n == 0 || n->img == 0) return;
  if (nmNotesOwner != n)
  {
    nmNotesOwner = n;
    sidecar.found = 0; sidecar.skip = 0; sidecar.rows = 0;
    char key[96];
    if (n->discovered)
    {
      if (nmHasSidecar(n->key)) sidecarReadKey(n->key, &sidecar);
    }
    else if (n->id != 9999 && sidecarKey(n->img, n->param1, n->param2, key, sizeof key))
    {
      if (nmHasSidecar(key)) sidecarReadKey(key, &sidecar);
    }
  }
  int y = -52;
  if (n->src != 0 && !testRom(n->src))
  {
    v_printString(-120, y, "ROM NOT FOUND!", 5, 0x5f);
    y -= 14;
  }
  if (sidecar.found)
  {
    for (int r = 0; r < sidecar.rows && y > -118; r++)
    {
      if (sidecar.row[r][0] == 0) continue;
      v_printString(-120, y, sidecar.row[r], 5, 0x48);
      y -= 14;
    }
  }
  else if (n->src != 0 && n->src->text != 0)
  {
    for (int r = 0; n->src->text[r] != 0 && y > -118; r++)
    {
      if (n->src->text[r][0] == 0) continue;
      v_printString(-120, y, n->src->text[r], 5, 0x48);
      y -= 14;
    }
  }
  if (n->altImg != 0)                          // a ROM: say what Button 2 does
  {
    char *emu = strstr(nmOtherEmulator(n), "speedy") != 0 ? "SPEEDY" : "EXACT";
    char hint[40];
    strcpy(hint, "BUTTON 2 STARTS WITH ");
    strcat(hint, emu);
    v_printString(-120, -118, hint, 5, 0x40);
  }
}

void newMenuFrame(void)
{
  if (!nmReady)
  {
    newMenuInit();
    nmReady = 1;
  }

  if (nmBrowse)                                // old PICK A GAME screen; leave when it goes up/sideways
  {
    displayMenu();
    if (currentMenuItem != &exactPickOne) nmBrowse = 0;
    return;
  }

  char line[56];
  char path[64];
  nmPath(&nmNav, path, (int) sizeof path);
  v_printString(-120, 86, path[0] ? path : "MAIN MENU", 5, 0x5f);

  // The PiTrex vector font has A-Z, 0-9, ".", "!", "<" and ">" (drawn as arrows) and "$" (a folder).
  // Most other punctuation prints as a blank, so: ">" marks the selection, "$" a category, and the
  // caveat tag goes in its own column instead of in brackets.
  for (int r = 0; r < NM_ROWS && nmNav.top + r < nmNav.count; r++)
  {
    NmNode *n = nmNav.list[nmNav.top + r];
    int selected = nmNav.top + r == nmNav.sel;
    int c = 0;
    line[c++] = selected ? '>' : ' ';
    line[c++] = nmHasVisibleChild(n) ? '$' : ' ';
    line[c++] = ' ';
    for (int k = 0; n->title[k] != 0 && c < 33; k++) line[c++] = n->title[k];
    line[c] = 0;
    int y = 68 - r * 14;
    v_printString(-120, y, line, 5, selected ? 0x5f : 0x40);
    const char *tag = nmTagText(n->tags);
    if (tag[0] != 0) v_printString(48, y, (char *) tag, 5, selected ? 0x50 : 0x30);
  }
  if (nmNav.top > 0) v_printString(84, 86, "MORE", 5, 0x30);
  if (nmNav.top + NM_ROWS < nmNav.count) v_printString(84, 68 - NM_ROWS * 14, "MORE", 5, 0x30);

  NmNode *cur = nmCurrent(&nmNav);
  if (!nmHasVisibleChild(cur != 0 ? cur : &nmTree.pool[0])) nmDrawNotes(cur);

  // input: stick up/down moves (hold to repeat), right/button 4 enters or starts, left goes back
  uint8_t buttons = currentButtonState & 0x0f;
  int8_t jy = currentJoy1Y;                    // negative = down
  int8_t jx = currentJoy1X;
  if (jy > -30 && jy < 30)
  {
    nmHeld = 0;
  }
  else
  {
    if (nmHeld == 0 || (nmHeld > 25 && (nmHeld % 6) == 0)) nmMove(&nmNav, jy < 0 ? 1 : -1);
    nmHeld++;
  }
  if (jx > -40 && jx < 40)
  {
    nmSideLatch = 0;
  }
  else if (!nmSideLatch)
  {
    nmSideLatch = 1;
    if (jx < 0) nmBack(&nmNav);
    else if (cur != 0 && nmHasVisibleChild(cur)) nmActivate(0);
  }
  if (buttons != nmLastButtons)                // act on the press, not while held
  {
    if (buttons == 0x08) nmActivate(4);
    else if (buttons == 0x01) nmActivate(1);
    else if (buttons == 0x02) nmActivate(2);
  }
  nmLastButtons = buttons;
}

#endif
