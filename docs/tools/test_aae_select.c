/* Host test for aae/aaeSelect.h using the real AAE driver names.
 *
 *   python3 -I docs/tools/gen_aae_table.py aae/aaemain.c > /tmp/aae_table.inc
 *   gcc -Wall -Wextra -I aae -I /tmp -o /tmp/t docs/tools/test_aae_select.c && /tmp/t <file with zip names, one per line>
 *
 * The file list is, for example, `ls <card>/roms/aae`.
 */
#include <stdio.h>
#include <stdlib.h>
#include "aaeSelect.h"
#include "aae_table.inc"

static int fails = 0;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: " __VA_ARGS__); printf("\n"); fails++; } } while (0)

static int idx(const char *name)
{
  for (int i = 0; i < TCOUNT; i++) if (strcmp(tNames[i], name) == 0) return i;
  return -99;
}

int main(int argc, char **argv)
{
  printf("driver entries: %d\n", TCOUNT);

  /* parameter forms */
  CHECK(aaeParamToIndex("", tNames, TCOUNT) == AAE_PICK, "empty -> picker");
  CHECK(aaeParamToIndex(0, tNames, TCOUNT) == AAE_PICK, "null -> picker");
  CHECK(aaeParamToIndex("\x2d", tNames, TCOUNT) == 0x2d, "legacy number 0x2d");
  CHECK(strcmp(tNames[0x2d], "mhavoc") == 0, "0x2d is mhavoc");
  CHECK(aaeParamToIndex("\x36", tNames, TCOUNT) == 0x36, "legacy 0x36 (quantum)");
  CHECK(aaeParamToIndex("\x46", tNames, TCOUNT) == TCOUNT - 1, "last legacy number");
  CHECK(aaeParamToIndex("\x47", tNames, TCOUNT) == AAE_BAD, "number past the table");
  CHECK(aaeParamToIndex("\xff", tNames, TCOUNT) == AAE_BAD, "0xff is not negative");
  CHECK(aaeParamToIndex("mhavoc", tNames, TCOUNT) == idx("mhavoc"), "name mhavoc");
  CHECK(aaeParamToIndex("MHAVOC", tNames, TCOUNT) == idx("mhavoc"), "name is case-insensitive");
  CHECK(aaeParamToIndex("mhavoc2", tNames, TCOUNT) == idx("mhavoc2"), "mhavoc2 is not mhavoc");
  CHECK(aaeParamToIndex("mhavo", tNames, TCOUNT) == AAE_BAD, "prefix is not a match");
  CHECK(aaeParamToIndex("nosuchgame", tNames, TCOUNT) == AAE_BAD, "unknown name");
  for (int i = 0; i < TCOUNT; i++)                      /* every name finds itself */
    if (strlen(tNames[i]) > 1) CHECK(aaeParamToIndex(tNames[i], tNames, TCOUNT) == i, "name %s", tNames[i]);

  /* zip names */
  char b[48];
  CHECK(aaeZipBase("MHavoc.ZIP", b, sizeof b) && strcmp(b, "mhavoc") == 0, "zip base");
  CHECK(!aaeZipBase(".hidden.zip", b, sizeof b), "hidden file");
  CHECK(!aaeZipBase("readme.txt", b, sizeof b), "not a zip");
  CHECK(!aaeZipBase("x.zi", b, sizeof b), "short");
  CHECK(!aaeZipBase(".zip", b, sizeof b), "no base name");
  CHECK(!aaeZipBase("abcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghij.zip", b, sizeof b), "too long");

  /* collection from a real directory listing */
  if (argc > 1)
  {
    static char lines[256][64];
    const char *files[256];
    int nf = 0;
    FILE *f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 2; }
    char l[64];
    while (nf < 256 && fgets(l, sizeof l, f))
    {
      l[strcspn(l, "\r\n")] = 0;
      strcpy(lines[nf], l);
      files[nf] = lines[nf];
      nf++;
    }
    fclose(f);
    files[nf] = "mhavoc.ZIP"; nf++;          /* duplicate in other case */
    int out[AAE_MAX_GAMES];
    int n = aaeCollect(files, nf, tNames, tDescs, TCOUNT, out, AAE_MAX_GAMES);

    int expect = 0;
    for (int i = 0; i < TCOUNT; i++)
      for (int k = 0; k < nf; k++)
      {
        char base[48];
        if (aaeZipBase(files[k], base, sizeof base) && strcmp(base, tNames[i]) == 0) { expect++; break; }
      }
    printf("zip files listed: %d, games found: %d (expected %d)\n", nf, n, expect);
    CHECK(n == expect, "count %d != %d", n, expect);
    for (int a = 1; a < n; a++) CHECK(aaeDescCmp(tDescs[out[a - 1]], tDescs[out[a]]) <= 0, "order at %d", a);
    for (int a = 0; a < n; a++)
      for (int c = a + 1; c < n; c++) CHECK(out[a] != out[c], "duplicate %s", tNames[out[a]]);
    for (int a = 0; a < n; a++) printf("  %-9s %s\n", tNames[out[a]], aaeTrim(tDescs[out[a]]));
  }
  printf(fails ? "FAIL (%d)\n" : "PASS\n", fails);
  return fails != 0;
}
