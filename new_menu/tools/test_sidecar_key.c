/* Host test for pitrex/loader/menuSidecar.h against the keys the 0.1.0 exporter wrote.
 *
 *   gcc -c -w -Wno-error=implicit-function-declaration -include stdio.h -include string.h \
 *       -include ctype.h -include stdlib.h -include errno.h \
 *       -I pitrex/vectrex -o /tmp/ini.o pitrex/vectrex/ini.c
 *   gcc -Wall -Wextra -I pitrex/loader -I pitrex/vectrex -o /tmp/t \
 *       new_menu/tools/test_sidecar_key.c /tmp/ini.o && /tmp/t new_menu/menu/sidecar
 *
 * every (image, param1, param2) in keys.tsv produces the key the 0.1.0 exporter wrote (the entry files of 0.1.0 were
 * named by it), and hide/title lines parse as the new menu needs
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ini.h"
#include "menuSidecar.h"

/* ini.c's file-based ini_parse() pulls in the baremetal file layer; stub what it names. */
char lastAccessedIni[255];
int __filelength(FILE *fd) { (void) fd; return 0; }
const char *getErrorText(int e) { (void) e; return ""; }
int f_getcwd(char *b, int n) { (void) b; (void) n; return 0; }

static int handler(void *user, const char *section, const char *name, const char *value)
{
  (void) section;
  sidecarLine((SidecarText *) user, name, value);
  return 1;
}

static int unhex(const char *h, char *out, int max)
{
  int n = 0;
  for (; h[0] && h[1] && n < max - 1; h += 2)
  {
    char b[3] = { h[0], h[1], 0 };
    out[n++] = (char) strtol(b, 0, 16);
  }
  out[n] = 0;
  return n;
}

int main(int argc, char **argv)
{
  if (argc < 2) { fprintf(stderr, "usage: %s <exporter outdir>\n", argv[0]); return 2; }
  char path[512];
  int fail = 0, checked = 0;

  snprintf(path, sizeof path, "%s/keys.tsv", argv[1]);
  FILE *f = fopen(path, "r");
  if (!f) { perror(path); return 2; }
  char line[1024];
  fclose(f);

  /* the manual split, preserving empty fields */
  f = fopen(path, "r");
  while (fgets(line, sizeof line, f))
  {
    line[strcspn(line, "\n")] = 0;
    char *c[4]; int n = 0; c[n++] = line;
    for (char *p = line; *p && n < 4; p++) if (*p == '\t') { *p = 0; c[n++] = p + 1; }
    if (n != 4) { fprintf(stderr, "bad line: %s\n", line); fail++; continue; }
    char p1[256], p2[256], key[128];
    unhex(c[1], p1, sizeof p1);
    unhex(c[2], p2, sizeof p2);
    int ok = sidecarKey(c[0], p1, p2, key, sizeof key);
    checked++;
    if (!ok || strcmp(key, c[3]) != 0)
    {
      printf("KEY MISMATCH: %s -> C '%s', exporter '%s'\n", c[0], ok ? key : "(none)", c[3]);
      fail++;
    }
  }
  fclose(f);
  printf("keys checked: %d\n", checked);

  /* hide and title (used by the new menu) */
  {
    SidecarText t; memset(&t, 0, sizeof t);
    ini_parse_string("[entry]\ntitle = MY PROGRAM\nhide = 1\n", handler, &t);
    if (!(t.hide == 1 && strcmp(t.title, "MY PROGRAM") == 0 && t.found == 0))
    { printf("hide/title not parsed: hide=%d title='%s' found=%d\n", t.hide, t.title, t.found); fail++; }
    memset(&t, 0, sizeof t);
    ini_parse_string("hide = 0\n", handler, &t);
    if (t.hide != 0) { printf("hide = 0 should not hide\n"); fail++; }
  }
  printf(fail ? "FAIL (%d)\n" : "PASS\n", fail);
  return fail != 0;
}
