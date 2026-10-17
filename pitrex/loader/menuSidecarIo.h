// Reading menu sidecar files from the SD card and the cached text for the entry on screen.
// Shared by the loader (loaderMain.c) and the desktop preview (docs/tools/menu_preview).
//
// The includer provides: MenuItemNew, menuSidecar.h, <vectrex/ini.h> (ini_parse_string),
// fopen/fread/fclose for the card, ensureMainPath() (go to the card's top folder) and
// the global scrollReset. See docs/menu/05-sidecar-format.md.
#ifndef MENU_SIDECAR_IO_H
#define MENU_SIDECAR_IO_H

// ---------------------------------------------------------------------------
// Menu sidecar files: "menu/<key>.ini" replaces the note lines of a menu entry.
// See menuSidecar.h and docs/menu/05-sidecar-format.md.
// The stock ini_parse() is NOT used: it keeps a 10 KB buffer on the stack and records
// the file name in lastAccessedIni, which the settings screen uses to decide where to
// save. A sidecar must never become that file.
// ---------------------------------------------------------------------------
static SidecarText sidecar;
static MenuItemNew *sidecarOwner = 0;       // entry the cached text belongs to
static char sidecarFile[2048];

static int sidecarIniHandler(void *user, const char *section, const char *name, const char *value)
{
  sidecarLine((SidecarText *) user, name, value);
  return 1;
}

// Reads menu/<key>.ini into *t. Returns 1 if the file exists.
static int sidecarReadKey(const char *key, SidecarText *t)
{
  memset(t, 0, sizeof *t);
  char path[128];
  strcpy(path, "menu/");
  strcat(path, key);
  strcat(path, ".ini");

  ensureMainPath();
  FILE *f = fopen(path, "rb");
  if (f == 0) return 0;                        // no sidecar: keep the compiled notes
  int len = (int) fread(sidecarFile, 1, sizeof(sidecarFile) - 1, f);
  fclose(f);
  if (len <= 0) return 0;
  for (int i = 0; i < len; i++)
    if (sidecarFile[i] == '\r') sidecarFile[i] = '\n';
  sidecarFile[len] = 0;
  ini_parse_string(sidecarFile, sidecarIniHandler, t);
  return 1;
}

static void loadSidecarText(MenuItemNew *m)
{
  sidecar.found = 0;
  sidecar.skip = 0;
  sidecar.rows = 0;
  if (m == 0 || m->id == 9999) return;         // 9999 = PICK A GAME (browses the card)

  char key[96];
  if (!sidecarKey(m->img, m->param1, m->param2, key, sizeof(key))) return;
  sidecarReadKey(key, &sidecar);
}

#endif
