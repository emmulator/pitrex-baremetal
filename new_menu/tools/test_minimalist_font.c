/* Host test for pitrex/loader/newMenuFont.h: the glyph table of the minimalist font.
 * Every point is two digits on the 0-4 by 0-6 grid, a path has two points at least, and every character the menu
 * can show (A-Z 0-9 and the punctuation of its own texts) has a glyph. */
#include <stdio.h>
#include <string.h>
#include "newMenuFont.h"

static int bad = 0;
#define CHECK(c, ...) do { if (!(c)) { bad++; printf("FAIL  " __VA_ARGS__); printf("\n"); } } while (0)

int main(void)
{
  for (int c = NM_MIN_FIRST; c <= NM_MIN_LAST; c++)
  {
    const char *g = nmMinGlyph(c);
    CHECK(g != 0 && *g, "'%c' has no glyph", c);
    if (!g) continue;
    int digits = 0;
    for (const char *p = g;; p++)
    {
      if (*p == ' ' || *p == 0)
      {
        CHECK(digits >= 4 && digits % 2 == 0, "'%c': a path of %d digits", c, digits);
        digits = 0;
        if (!*p) break;
        continue;
      }
      CHECK((digits % 2 == 0) ? (*p >= '0' && *p <= '4') : (*p >= '0' && *p <= '6'), "'%c': %c is off the grid", c, *p);
      digits++;
    }
  }
  const char *shown = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.!<>$,-:/'()?+";
  for (const char *s = shown; *s; s++) CHECK(nmMinGlyph(*s) != 0, "'%c' (shown by the menu) has no glyph", *s);
  CHECK(nmMinGlyph('a') == nmMinGlyph('A'), "lower case is not drawn as upper case");
  CHECK(nmMinGlyph(' ') == 0 && nmMinGlyph(200) == 0, "a space or a high character has a glyph");
  printf(bad ? "%d failed\n" : "all passed\n", bad);
  return bad != 0;
}
