// The new list menu: drawing, input, launching, and scanning the card.
// Shared by the loader (loaderMain.c) and the desktop preview (new_menu/tools/menu_preview).
//
// The includer provides before this header: MenuItemNew (only as the launch record that
// loadAndStart() takes), newMenuLogic.h, menuSidecar.h, menuSidecarIo.h, the PiTrex library calls
// v_printString, v_directReadButtons, v_SettingsGUI, v_saveIni, v_WaitRecal, v_noSound, the globals
// currentButtonState, currentJoy1X/Y, the loader.ini flags newMenuShowMissing and newMenuLog, the function loadAndStart(), ensureMainPath(),
// opendir/readdir/closedir with DT_DIR, and the macros IMG_FILE_PREFIX and NM_IMG_DIR. NM_CARD is the
// prefix of paths on the card ("0:/" on the PiTrex, "" on the desktop). Nothing about the menu is compiled
// in: it is built from menu/*.ini, the image folder and vectrex/. Documentation: new_menu/menu/07-new-menu.md
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
// The logic is in newMenuLogic.h (tested on a PC). The entries come from the entry files in menu/ on the
// card, the programs found in the image folder and the ROMs found under vectrex/.
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
static char nmRomPaths[NM_MAX_ROMS][NM_ROM_PATH];
static char nmFileNames[NM_MAX_FILES][32];
static const char *nmFilePtr[NM_MAX_FILES];
static char nmSideNames[320][NM_KEY_LEN];       // names of the files in menu/ (without .ini), lower case
static int nmSideCount = 0;
static MenuItemNew nmLaunchItem;               // the record loadAndStart() takes
static int nmNeedsMissing = 0;                 // the highlighted entry's needed file is not on the card
static NmNode *nmNotesOwner = 0;
static int nmHeld = -100000;                    // frames the stick has been held up/down (starts "held": ignore the stick until it is centred once)
static int nmSideLatch = 1;                    // stick left/right already acted on (starts set, same reason)
static int nmNeutral = 0;                      // consecutive frames with the stick centred and no button down
static int nmFrames = 0;
static int nmArmed = 0;                        // input is ignored until the controls have been at rest for a moment
static uint8_t nmLastButtons = 0x0f;           // a button held at start does not count as a press

static void nmLog(const char *step);

// The menu files: menu.default.ini, the other *.ini in menu/, then menu.ini, read into records at start-up.
static NmRecs nmRecs;
static char nmMenuBuf[131072];                 // one menu file at a time
static int nmMenuTruncated = 0;
static NmRec nmDefRules[2];                    // built-in folder rules (videos, songs)
static char nmDirNames[NM_MAX_FILES][40];      // the files of one folder-rule folder
static const char *nmDirPtr[NM_MAX_FILES];

static int nmRecRead(const char *key, SidecarText *t)
{
  return nmRecToSidecar(&nmRecs, key, t);
}

// The names of the files in menu/ (without .ini), sorted, so the order in which they are read does not depend on
// the card's directory order.
static void nmListSidecars(void)
{
  nmSideCount = 0;
  DIR *dp = opendir(NM_CARD "menu");
  if (dp == 0) return;
  dirent *fi;
  while ((fi = readdir(dp)) != 0 && nmSideCount < 320)
  {
    int len = (int) strlen(fi->d_name);
    if (fi->d_type == DT_DIR || len <= 4 || len > NM_KEY_LEN - 1 + 4) continue;
    if (!nmEqNoCase(fi->d_name + len - 4, ".ini")) continue;
    for (int i = 0; i < len - 4 && i < NM_KEY_LEN - 1; i++) nmSideNames[nmSideCount][i] = nmLower(fi->d_name[i]);
    nmSideNames[nmSideCount][len - 4 > NM_KEY_LEN - 1 ? NM_KEY_LEN - 1 : len - 4] = 0;
    nmSideCount++;
  }
  closedir(dp);
  for (int a = 1; a < nmSideCount; a++)        // insertion sort
  {
    char v[NM_KEY_LEN];
    nmCopy(v, nmSideNames[a], NM_KEY_LEN);
    int b = a - 1;
    while (b >= 0 && strcmp(nmSideNames[b], v) > 0) { nmCopy(nmSideNames[b + 1], nmSideNames[b], NM_KEY_LEN); b--; }
    nmCopy(nmSideNames[b + 1], v, NM_KEY_LEN);
  }
  // reading order: menu.default.ini first, then the other files, then the user's menu.ini, so it has the last word
  int haveDef = 0, haveMain = 0, o = 0;
  for (int i = 0; i < nmSideCount; i++)
  {
    if (strcmp(nmSideNames[i], "menu.default") == 0) { haveDef = 1; continue; }
    if (strcmp(nmSideNames[i], "menu") == 0) { haveMain = 1; continue; }
    if (o != i) nmCopy(nmSideNames[o], nmSideNames[i], NM_KEY_LEN);
    o++;
  }
  nmSideCount = o;
  if (haveDef)
  {
    for (int i = nmSideCount; i > 0; i--) nmCopy(nmSideNames[i], nmSideNames[i - 1], NM_KEY_LEN);
    nmCopy(nmSideNames[0], "menu.default", NM_KEY_LEN);
    nmSideCount++;
  }
  if (haveMain) nmCopy(nmSideNames[nmSideCount++], "menu", NM_KEY_LEN);
}

// Reads menu/<stem>.ini into the records.
static void nmReadMenuFile(const char *stem)
{
  char path[96];
  nmCopy(path, "menu/", (int) sizeof path);
  strncat(path, stem, sizeof path - strlen(path) - 5);
  strcat(path, ".ini");
  ensureMainPath();
  FILE *f = fopen(path, "rb");
  if (f == 0) return;
  int len = (int) fread(nmMenuBuf, 1, sizeof(nmMenuBuf) - 1, f);
  if (len == (int) sizeof(nmMenuBuf) - 1) nmMenuTruncated = 1;
  fclose(f);
  if (len <= 0) return;
  nmMenuBuf[len] = 0;
  nmParseMenuText(&nmRecs, nmMenuBuf, stem);
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
          char full[NM_ROM_PATH + 16];                     // only real cartridges: they start with "g GCE"
          unsigned char head[8];
          int got = 0;
          nmCopy(full, "vectrex/", (int) sizeof full);
          strncat(full, path, sizeof full - strlen(full) - 1);
          ensureMainPath();
          FILE *rf = fopen(full, "rb");
          if (rf != 0) { got = (int) fread(head, 1, sizeof head, rf); fclose(rf); }
          if (nmIsCartridge(head, got)) nmCopy(nmRomPaths[nroms++], path, NM_ROM_PATH);
        }
      }
      closedir(dp);
    }
    di++;
  }
  return nroms;
}

static NmScratch nmScratch;
static int nmInitStage = 0;                    // reading the card takes a while: done in steps, one screen refresh each
static int nmInitFiles = 0, nmInitRoms = 0, nmInitDone = 0, nmInitEntries = 0, nmFound = 0;

// One step of building the menu. Returns 1 when the menu is ready. While it is not, the screen says
// READING SD CARD (the PITREX logo and the music go on, because the loader's main loop keeps running).
// A ROM zip an entry needs: is it on the card?
static int nmFileExists(const char *path)
{
  ensureMainPath();
  FILE *f = fopen(path, "rb");
  if (f == 0) return 0;
  fclose(f);
  return 1;
}

// The Vectrex emulators only load a saved state on Button 1 when vectrex.ini has HANDLE_STATES = 1
// (section VECTREX_EXACT; a missing key means 1). The menu only promises a saved state then.
static int nmStatesOn = 1;
static int nmStatesHandler(void *user, const char *section, const char *name, const char *value)
{
  (void) user;
  if (strcmp(section, "VECTREX_EXACT") == 0 && strcmp(name, "HANDLE_STATES") == 0) nmStatesOn = atoi(value) == 1;
  return 1;
}

static void nmReadStatesFlag(void)
{
  nmStatesOn = 1;
  ensureMainPath();
  FILE *f = fopen("vectrex.ini", "rb");
  if (f == 0) return;
  int len = (int) fread(sidecarFile, 1, sizeof(sidecarFile) - 1, f);   // the sidecar buffer is free during start-up
  fclose(f);
  if (len <= 0) return;
  for (int i = 0; i < len; i++) if (sidecarFile[i] == '\r') sidecarFile[i] = '\n';
  sidecarFile[len] = 0;
  ini_parse_string(sidecarFile, nmStatesHandler, 0);
}

static int newMenuInitStep(void)
{
  switch (nmInitStage)
  {
    case 0:
      memset(&nmTree, 0, sizeof nmTree);
      memset(&nmRecs, 0, sizeof nmRecs);
      nmListSidecars();
      nmReadStatesFlag();
      break;
    case 1:                                    // the images in the image folder
    {
      DIR *dp = opendir(NM_IMG_DIR);
      if (dp != 0)
      {
        dirent *fi;
        while ((fi = readdir(dp)) != 0 && nmInitFiles < NM_MAX_FILES)
        {
          if (fi->d_type == DT_DIR) continue;
          nmCopy(nmFileNames[nmInitFiles], fi->d_name, (int) sizeof nmFileNames[0]);
          nmFilePtr[nmInitFiles] = nmFileNames[nmInitFiles];
          nmInitFiles++;
        }
        closedir(dp);
      }
      break;
    }
    case 2:
      nmInitRoms = nmScanRoms();
      break;
    case 3:                                    // the menu files, a few per refresh (menu.default.ini alone in the first)
    {
      int n = nmInitDone == 0 ? 1 : nmSideCount - nmInitDone;
      if (n > 6) n = 6;
      for (int i = 0; i < n && nmInitDone < nmSideCount; i++) nmReadMenuFile(nmSideNames[nmInitDone++]);
      if (nmInitDone < nmSideCount) return 0;  // stay in this stage
      break;
    }
    case 4:                                    // the entries, then every folder rule's files
    {
      nmInitEntries = nmAddRecEntries(&nmTree, &nmRecs, IMG_FILE_PREFIX);
      int ndef = nmDefaultFolderRules(&nmRecs, nmDefRules, 2);
      for (int i = 0; i < nmRecs.n + ndef; i++)
      {
        const NmRec *rule = i < nmRecs.n ? &nmRecs.rec[i] : &nmDefRules[i - nmRecs.n];
        if (rule->kind != NM_REC_FOLDER) continue;
        char dir[NM_REC_PATH + 4];
        nmCopy(dir, NM_CARD, (int) sizeof dir);
        strncat(dir, rule->path, sizeof dir - strlen(dir) - 1);
        int nf = 0;
        DIR *dp = opendir(dir);
        if (dp == 0) continue;
        dirent *fi;
        while ((fi = readdir(dp)) != 0 && nf < NM_MAX_FILES)
        {
          if (fi->d_type == DT_DIR) continue;
          nmCopy(nmDirNames[nf], fi->d_name, (int) sizeof nmDirNames[0]);
          nmDirPtr[nf] = nmDirNames[nf];
          nf++;
        }
        closedir(dp);
        nmInitEntries += nmAddFolderFiles(&nmTree, &nmRecs, rule, IMG_FILE_PREFIX, nmDirPtr, nf, nmRecRead);
      }
      break;
    }
    default:
      nmSettingsNode = nmAssembleRest(&nmTree, &nmScratch, nmRecRead, IMG_FILE_PREFIX, nmFilePtr, nmInitFiles,
                                      nmRomPaths, nmInitRoms, IMG_FILE_PREFIX "vectrexexact.img",
                                      IMG_FILE_PREFIX "vectrexspeedy.img",
                                      newMenuShowMissing ? 0 : nmFileExists, &nmFound);
      nmInit(&nmNav, &nmTree);
      nmNotesOwner = 0;
      printf("New menu: %i nodes, %i entries, %i records, %i programs, %i ROMs found\n\r", nmTree.used, nmInitEntries, nmRecs.n, nmFound, nmInitRoms);
      if (nmRecs.dropped || nmMenuTruncated)
      {
        char b[80];
        sprintf(b, "menu files: %i sections did not fit, truncated=%i", nmRecs.dropped, nmMenuTruncated);
        nmLog(b);
      }
      return 1;
  }
  nmInitStage++;
  return 0;
}

// A breadcrumb file for problems that freeze the PiTrex: NMLOG.TXT on the card lists the last steps
// that were reached. Rewritten on every call, so it is complete up to the freeze.
static char nmLogText[700];
static void nmLog(const char *step)
{
  if (!newMenuLog) return;                     // loader.ini NEW_MENU_LOG = 1: off by default, it writes to the card
  if (strlen(nmLogText) + strlen(step) + 2 < sizeof nmLogText)
  {
    strcat(nmLogText, step);
    strcat(nmLogText, "\n");
  }
  FILE *f = fopen("NMLOG.TXT", "wb");
  if (f == 0) return;
  fwrite(nmLogText, 1, strlen(nmLogText), f);
  fclose(f);
}

// Says what is about to start, for a moment. If the screen then goes blank, the program
// itself is what failed and not the menu.
static void nmSplash(const char *what, const char *how)
{
  char l1[56], l2[56];
  v_noSound();
  nmCopy(l1, "STARTING", (int) sizeof l1);
  nmCopy(l2, how, (int) sizeof l2);
  for (int f = 0; f < 45; f++)
  {
    v_WaitRecal();
    v_printString(-120, 40, l1, 7, 0x5f);
    v_printString(-120, 16, (char *) what, 7, 0x5f);
    if (l2[0] != 0) v_printString(-120, -12, l2, 5, 0x48);
  }
}

static void nmLaunch(NmNode *n, int button, int useOtherEmulator)
{
  char how[40];
  how[0] = 0;
  const char *img = useOtherEmulator ? nmOtherEmulator(n) : n->img;
  if (n->altImg != 0)                          // a ROM: name the emulator
  {
    strcpy(how, strstr(img, "speedy") != 0 ? "WITH SPEEDY" : "WITH EXACT");
    if (button == 1 && nmStatesOn) strcat(how, " AND SAVED STATE");
  }
  char shown[36];
  nmCopy(shown, n->title, (int) sizeof shown);
  if (shown[0] == 0) nmCopy(shown, "PROGRAM", (int) sizeof shown);
  shown[17] = 0;                               // size 7 text: 17 characters fit the screen
  {
    char b[120];
    sprintf(b, "launch %s %s %s btn=%i", n->title, img, n->param1 != 0 ? n->param1 : "-", button);
    nmLog(b);
  }
  nmSplash(shown, how);

  nmLaunchItem.id = 0;
  nmLaunchItem.img = (char *) img;
  nmLaunchItem.param1 = (char *) n->param1;
  nmLaunchItem.param2 = (char *) n->param2;
  loadAndStart(&nmLaunchItem, button);
}

// After the settings screen: look at vectrexInterface.ini and tell the user whether it was written.
static void nmCheckSettingsFile(void)
{
  int len = -1;
  FILE *f = fopen("vectrexInterface.ini", "rb");
  if (f != 0)
  {
    len = (int) __filelength(f);
    fclose(f);
  }
  if (len > 100)
  {
    for (int f = 0; f < 60; f++)
    {
      v_WaitRecal();
      v_printString(-120, 20, "SETTINGS SAVED", 7, 0x5f);
    }
  }
  else
  {
    v_message("SETTINGS NOT SAVED!");        // waits for Button 1 and 4 together
  }
}

// Button 2 on a game that has "control = ..." lines in its sidecar: a page about the controls.
static void nmShowControls(NmNode *n)
{
  v_noSound();                                 // the menu music would hold its last tone while this page is up
  int armed = 0;                               // the button that opened the page may still be down: it must be let go first
  for (int f = 0; f < 100000; f++)
  {
    v_WaitRecal();
    v_readButtons();
    v_readJoystick1Analog();
    char shown[36];
    nmCopy(shown, n->title, (int) sizeof shown);
    shown[17] = 0;
    v_printString(-120, 100, "CONTROLS", 5, 0x5f);
    v_printString(-120, 80, shown, 7, 0x5f);
    int y = 54;
    for (int r = 0; r < sidecar.nctl && y > -100; r++)
    {
      v_printString(-120, y, sidecar.ctl[r], 5, 0x48);
      y -= 13;
    }
    v_printString(-120, -124, "ANY BUTTON OR STICK LEFT GOES BACK.", 5, 0x40);
    int down = (currentButtonState & 0x0f) != 0;
    int jx = currentJoy1X;
    if (!down && jx > -60) armed = 1;
    if (armed && (down || jx < -60)) break;
  }
  nmLastButtons = currentButtonState & 0x0f;   // a button still held now is not a new press in the menu
  nmSideLatch = 1;
}

// button: 4 or 1 (the button that was pressed), 2 = start a ROM with the other emulator, 0 = stick
static void nmActivate(int button)
{
  if (button == 2)
  {
    NmNode *c = nmCurrent(&nmNav);
    if (c != 0 && !nmHasVisibleChild(c) && c->altImg != 0) nmLaunch(c, 4, 1);
    else if (c != 0 && nmNotesOwner == c && sidecar.nctl > 0) nmShowControls(c);
    return;
  }
  NmNode *n = nmEnter(&nmNav);                 // goes into a category, else returns the entry
  if (n == 0) return;
  if (n == nmSettingsNode)
  {
    while ((v_directReadButtons() & 0x0f) != 0) { }
    // v_saveIni() writes to the ini that was parsed last. After a program returned to the
    // loader that is loader.ini, so name the target explicitly.
    strcpy(lastAccessedIni, "vectrexInterface.ini");
    nmLog("settings gui opened");
    v_SettingsGUI(1);                          // saves the ini itself when left with Button 4
    nmLog("settings gui left");
    nmCheckSettingsFile();
    nmLog("checked");
    return;
  }
  if (n->img == 0) return;                     // an empty category
  nmLaunch(n, button == 1 ? 1 : 4, 0);
}

// Layout (screen is about -127..127 both ways; the loader's own PITREX logo sits at the top):
//   y  84        where you are (breadcrumb)
//   y  66..-30   the list, 7 rows of size 7 text
//   y -50..-86   notes of the highlighted entry, or help for a category
//   y -100..-124 what the buttons do right now
#define NM_LIST_Y 66
#define NM_LIST_STEP 16
#define NM_NOTE_Y (-50)
#define NM_NOTE_STEP 12

static void nmDrawNotes(NmNode *n)
{
  if (n == 0) return;
  int y = NM_NOTE_Y;
  const char *help[6];
  int nh = 0;
  if (n->img == 0)                             // a category: its text in the menu file, else the built-in help
  {
    static SidecarText cat;
    if (nmRecRead(n->key, &cat) && cat.rows > 0)
    {
      for (int r = 0; r < cat.rows && r < 6; r++) help[nh++] = cat.row[r];
    }
    else nh = nmHelp(n, help, 6);
  }
  if (nh > 0)                                  // a category (or SETTINGS): say what it is
  {
    for (int r = 0; r < nh; r++)
    {
      v_printString(-120, y, (char *) help[r], 5, 0x48);
      y -= NM_NOTE_STEP;
    }
    return;
  }
  if (n->img == 0) return;
  if (nmNotesOwner != n)
  {
    nmNotesOwner = n;
    memset(&sidecar, 0, sizeof sidecar);
    nmNeedsMissing = 0;
    if (n->key[0] != 0) nmRecRead(n->key, &sidecar);
    size_t nl = strlen(n->needs);
    if (nl > 0 && n->needs[nl - 1] != '/')     // a folder ("roms/aae/", the AAE picker) cannot be opened as a file
    {
      ensureMainPath();
      FILE *f = fopen(n->needs, "rb");
      if (f == 0) nmNeedsMissing = 1; else fclose(f);
    }
  }
  if (nmNeedsMissing)
  {
    v_printString(-120, y, "ROM NOT FOUND!", 5, 0x5f);
    y -= NM_NOTE_STEP;
  }
  for (int r = 0; r < sidecar.rows && y > -90; r++)
  {
    if (sidecar.row[r][0] == 0) continue;
    v_printString(-120, y, sidecar.row[r], 5, 0x48);
    y -= NM_NOTE_STEP;
  }
}

// The line(s) at the bottom: what the buttons do for the highlighted entry.
static void nmDrawHint(NmNode *cur)
{
  if (cur == 0) return;
  if (nmHasVisibleChild(cur))
  {
    v_printString(-120, -124, "4 OR RIGHT OPENS.  LEFT GOES BACK.", 5, 0x40);
  }
  else if (cur->altImg != 0)                   // a Vectrex game file
  {
    int otherSpeedy = strstr(nmOtherEmulator(cur), "speedy") != 0;
    int mainSpeedy = strstr(cur->img, "speedy") != 0;
    char l1[48], l2[48];
    strcpy(l1, "4 STARTS WITH ");
    strcat(l1, mainSpeedy ? "SPEEDY" : "EXACT");
    strcpy(l2, "2 STARTS WITH ");
    strcat(l2, otherSpeedy ? "SPEEDY" : "EXACT");
    v_printString(-120, -100, l1, 5, 0x5f);
    v_printString(-120, -112, l2, 5, 0x5f);
    v_printString(-120, -124, "LEFT GOES BACK.", 5, 0x40);
  }
  else if (cur->img != 0 || cur->id == 0)
  {
    if (nmNotesOwner == cur && sidecar.nctl > 0)
      v_printString(-120, -112, "2 SHOWS THE CONTROLS", 5, 0x5f);
    v_printString(-120, -124, "4 STARTS.  LEFT GOES BACK.", 5, 0x40);
  }
}

void newMenuFrame(void)
{
  if (!nmReady)
  {
    v_printString(-120, 20, "READING SD CARD", 7, 0x5f);
    if (nmInitStage == 3 && nmSideCount > 0)
    {
      char b[24];
      sprintf(b, "%i OF %i", nmInitDone, nmSideCount);
      v_printString(-120, -10, b, 5, 0x48);
    }
    if (newMenuInitStep()) nmReady = 1;
    return;
  }

  char line[56];
  char path[64];
  nmPath(&nmNav, path, (int) sizeof path);
  v_printString(-120, 84, path[0] ? path : "MAIN MENU", 5, 0x5f);

  // The PiTrex vector font has A-Z, 0-9, ".", "!", "<" and ">" (drawn as arrows) and "$" (a folder).
  // Most other punctuation prints as a blank, so: ">" marks the selection, "$" a category, and "!"
  // at the right edge an entry with a caveat (NO SOUND, WIP, BROKEN: see its notes below).
  for (int r = 0; r < NM_ROWS && nmNav.top + r < nmNav.count; r++)
  {
    NmNode *n = nmNav.list[nmNav.top + r];
    int selected = nmNav.top + r == nmNav.sel;
    int c = 0;
    line[c++] = selected ? '>' : ' ';
    line[c++] = nmHasVisibleChild(n) ? '$' : ' ';
    line[c++] = ' ';
    for (int k = 0; n->title[k] != 0 && c < 31; k++) line[c++] = n->title[k];
    line[c] = 0;
    int y = NM_LIST_Y - r * NM_LIST_STEP;
    v_printString(-120, y, line, 7, selected ? 0x5f : 0x40);
    if (n->tags != 0) v_printString(108, y, "!", 7, selected ? 0x5f : 0x40);
  }
  if (nmNav.top > 0) v_printString(84, 84, "MORE", 5, 0x30);
  if (nmNav.top + NM_ROWS < nmNav.count) v_printString(84, NM_LIST_Y - NM_ROWS * NM_LIST_STEP + 6, "MORE", 5, 0x30);

  NmNode *cur = nmCurrent(&nmNav);
  nmDrawNotes(cur);
  nmDrawHint(cur);

  // Controls are ignored until they have rested for 15 frames (a stick that reads off-centre or a
  // button seen as down at power-on must not open VECTREX by itself). Gives up waiting after 600 frames.
  {
    int jxs = currentJoy1X, jys = currentJoy1Y;
    int rest = (jxs > -40 && jxs < 40 && jys > -30 && jys < 30 && (currentButtonState & 0x0f) == 0);
    nmFrames++;
    nmNeutral = rest ? nmNeutral + 1 : 0;
    if (nmFrames == 1 || (!nmArmed && (nmNeutral == 15 || nmFrames == 600)))
    {
      char b[90];
      sprintf(b, "%s frame %i btn=%x jx=%i jy=%i depth=%i", nmArmed ? "-" : (nmFrames == 1 ? "first" : "armed"), nmFrames,
              currentButtonState & 0x0f, jxs, jys, nmNav.depth);
      nmLog(b);
    }
    if (!nmArmed && (nmNeutral >= 15 || nmFrames >= 600)) { nmArmed = 1; nmLastButtons = currentButtonState & 0x0f; }
    if (!nmArmed) return;
  }

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
