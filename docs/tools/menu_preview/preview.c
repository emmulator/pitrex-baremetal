/* Desktop preview of the PiTrex loader's new menu.
 *
 * Compiles the SAME menu code as the loader (pitrex/loader/newMenuLogic.h, newMenuUi.h,
 * menuSidecarIo.h, menuSidecar.h) against Roger Boesch's PiTrex simulator (MIT licence,
 * https://rogerboesch.games/learn/gong-rider/), which supplies the screen, the stick and the
 * buttons. The compiled-in menu is the real one, read from loaderMain.c by gen_menu_tree.py.
 * The SD card is a folder. Starting a program prints a line instead of jumping to it.
 *
 * What it does NOT show: the PiTrex vector font (the simulator draws text in its own 5x7 font at
 * about the same size), the flashing title animation, the old PICK A GAME browser (replaced by a
 * stub), anything about timing or the real FAT driver.
 *
 *   preview <card folder>      with the simulator's own options as environment variables:
 *   PITREX_SHOT=out.bmp PITREX_FRAMES=60 PITREX_SCRIPT="down@10,right@20" preview card
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>

#include <vectrex/vectrexInterface.h>   /* the simulator's copy */
#include <vectrex/ini.h>                /* the loader's own INI parser */
#include "sim_extra.h"

#define RASPPI 1
#define IMG_FILE_PREFIX "piZero1/"
#define NM_CARD ""

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

#include "menu_tree.inc"                /* the compiled-in menu: mock[], vectrexMenu, ... exactPickOne */

/* ini.c's file-based ini_parse() names these; the menu only uses ini_parse_string(), so stubs do */
char lastAccessedIni[255];
int __filelength(FILE *fd) { (void) fd; return 500; }   /* pretend the settings file was written */
const char *getErrorText(int e) { (void) e; return ""; }
int f_getcwd(char *b, int n) { (void) b; (void) n; return 0; }

/* ---- stand-ins for the loader's globals and hardware calls ---- */
MenuItemNew *currentMenuItem;
int selectionMade = 0;
int scrollReset = 1;
static char cardRoot[PATH_MAX];
typedef struct dirent dirent;

void ensureMainPath(void) { if (chdir(cardRoot) != 0) perror("chdir card"); }
/* the loader waits here for a button to be released; the preview has no live hardware to poll */
uint8_t v_directReadButtons(void) { return 0; }
void v_SettingsGUI(int own) { (void) own; printf("SETTINGS GUI opened\n"); fflush(stdout); }
void v_saveIni() { printf("save ini %s\n", lastAccessedIni); fflush(stdout); }
void v_message(char *m) { printf("MESSAGE %s\n", m); fflush(stdout); }
int testRom(MenuItemNew *m) { (void) m; return 1; }

static void showParam(const char *p)
{
  if (p == 0) { printf("-"); return; }
  int printable = 1;
  for (const char *q = p; *q; q++) if ((unsigned char) *q < 32 || (unsigned char) *q > 126) printable = 0;
  if (printable) { printf("\"%s\"", p); return; }
  printf("0x");
  for (const char *q = p; *q; q++) printf("%02x", (unsigned char) *q);
}

void loadAndStart(MenuItemNew *item, int button)
{
  printf("LAUNCH image=%s param1=", item->img);
  showParam(item->param1);
  printf(" param2=");
  showParam(item->param2);
  printf(" button=%d id=%d\n", button, item->id);
  fflush(stdout);
}

void displayMenu(void)      /* the old PICK A GAME browser: a stub that leaves on stick left */
{
  v_printString(-100, 40, "OLD FILE BROWSER", 5, 0x50);
  v_printString(-100, 20, "(STUB IN THE PREVIEW)", 5, 0x40);
  if (currentJoy1X < -50) currentMenuItem = &vectrexMenu;
}

#include "menuSidecar.h"
#include "newMenuLogic.h"
#include "menuSidecarIo.h"
#include "newMenuUi.h"

int main(int argc, char **argv)
{
  const char *card = argc > 1 ? argv[1] : "sdcard";
  if (realpath(card, cardRoot) == 0) { perror(card); return 2; }
  ensureMainPath();
  mockInit();
  mockTopsInit();
  currentMenuItem = &vectrexMenu;

  vectrexinit(1);
  v_init();
  v_setRefresh(50);
  char title[] = "PITREX";
  int b = 20, bDir = 1;
  for (;;)
  {
    v_WaitRecal();
    v_readButtons();
    v_readJoystick1Analog();
    b += bDir;
    if (b == 70) bDir = -1;
    if (b == 20) bDir = 1;
    v_printString(-100, 100, title, 5, (uint8_t) b);
    newMenuFrame();
  }
}
