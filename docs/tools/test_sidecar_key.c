/* Host test for pitrex/loader/menuSidecar.h against the exporter's output.
 *
 *   gcc -c -w -Wno-error=implicit-function-declaration -include stdio.h -include string.h \
 *       -include ctype.h -include stdlib.h -include errno.h \
 *       -I pitrex/vectrex -o /tmp/ini.o pitrex/vectrex/ini.c
 *   gcc -Wall -Wextra -I pitrex/loader -I pitrex/vectrex -o /tmp/t \
 *       docs/tools/test_sidecar_key.c /tmp/ini.o && /tmp/t docs/menu/sidecar
 *
 * 1. every (image, param1, param2) in keys.tsv produces the key the exporter wrote
 * 2. every sidecar file, parsed with the loader's real ini parser (ini_parse_string),
 *    yields exactly the rows the exporter expects, also with CRLF line endings
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

  snprintf(path, sizeof path, "%s/expected.tsv", argv[1]);
  f = fopen(path, "r");
  if (!f) { perror(path); return 2; }
  int files = 0;
  size_t biggest = 0;
  while (fgets(line, sizeof line, f))
  {
    line[strcspn(line, "\n")] = 0;
    char *tab1 = strchr(line, '\t');
    char *tab2 = tab1 ? strchr(tab1 + 1, '\t') : 0;
    if (!tab2) { fprintf(stderr, "bad expected line\n"); fail++; continue; }
    *tab1 = 0; *tab2 = 0;
    const char *key = line;
    int skip = atoi(tab1 + 1);
    const char *rows = tab2 + 1;

    char file[1100];
    snprintf(file, sizeof file, "%s/menu/%s.ini", argv[1], key);
    FILE *g = fopen(file, "rb");
    if (!g) { printf("MISSING FILE %s\n", file); fail++; continue; }
    static char buf[4096];
    size_t len = fread(buf, 1, sizeof buf - 1, g);
    fclose(g);
    if (len > biggest) biggest = len;

    for (int crlf = 0; crlf < 2; crlf++)
    {
      static char data[8192];
      size_t o = 0;
      for (size_t i = 0; i < len; i++)
      {
        if (crlf && buf[i] == '\n') data[o++] = '\r';
        data[o++] = buf[i];
      }
      data[o] = 0;
      SidecarText t; memset(&t, 0, sizeof t);
      ini_parse_string(data, handler, &t);

      /* build the expected rows */
      char exp[SC_MAX_ROWS][SC_ROW_LEN]; int er = 0;
      const char *p = rows;
      if (*rows)
        for (;;)
        {
          const char *e = strchr(p, 0x1f);
          size_t l = e ? (size_t) (e - p) : strlen(p);
          memcpy(exp[er], p, l); exp[er][l] = 0; er++;
          if (!e) break;
          p = e + 1;
        }
      int same = t.found && t.skip == skip && t.rows == er;
      for (int r = 0; same && r < er; r++) if (strcmp(t.row[r], exp[r]) != 0) same = 0;
      if (!same)
      {
        printf("PARSE MISMATCH %s (%s): found=%d skip=%d/%d rows=%d/%d\n", key, crlf ? "CRLF" : "LF", t.found, t.skip, skip, t.rows, er);
        for (int r = 0; r < t.rows; r++) printf("   got  [%s]\n", t.row[r]);
        for (int r = 0; r < er; r++) printf("   want [%s]\n", exp[r]);
        fail++;
      }
    }
    files++;
  }
  fclose(f);
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
  printf("sidecar files parsed: %d (largest %zu bytes; loader buffer is %d)\n", files, biggest, 2047);
  if (biggest > 2047) { printf("FILE TOO LARGE FOR LOADER BUFFER\n"); fail++; }
  printf(fail ? "FAIL (%d)\n" : "PASS\n", fail);
  return fail != 0;
}
