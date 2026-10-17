// Menu sidecar files: menu/<key>.ini on the SD card overrides the note lines
// that the loader shows for a menu entry. No hardware dependencies, so it can be
// unit tested on a PC (see docs/tools/test_sidecar_key.c).
//
// Format and rules: docs/menu/05-sidecar-format.md
#ifndef MENU_SIDECAR_H
#define MENU_SIDECAR_H

#include <string.h>
#include <stdlib.h>

#define SC_MAX_ROWS 12
#define SC_ROW_LEN 48

typedef struct
{
  int found;                       // 1 if a sidecar file supplied text
  int skip;                        // blank rows before the first line
  int rows;                        // number of lines in row[]
  char row[SC_MAX_ROWS][SC_ROW_LEN];
} SidecarText;

// A parameter that is exactly one byte (the AAE game number) is written as xNN.
// Any other parameter is lower-cased, every run of characters that are not a-z/0-9
// becomes one '_', and leading/trailing '_' are dropped.
static void sidecarSlug(char *out, int max, const char *s)
{
  static const char hex[] = "0123456789abcdef";
  int n = 0;
  if (s[0] != 0 && s[1] == 0)
  {
    unsigned char b = (unsigned char) s[0];
    if (max >= 4)
    {
      out[n++] = 'x';
      out[n++] = hex[b >> 4];
      out[n++] = hex[b & 15];
    }
    out[n] = 0;
    return;
  }
  for (; *s != 0 && n < max - 1; s++)
  {
    char c = *s;
    if (c >= 'A' && c <= 'Z') c = (char) (c + 32);
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
    {
      out[n++] = c;
    }
    else if (n > 0 && out[n - 1] != '_')
    {
      out[n++] = '_';
    }
  }
  if (n > 0 && out[n - 1] == '_') n--;
  out[n] = 0;
}

// key = <image base name without directory and ".img"> [ "." slug(param1) ] [ "." slug(param2) ]
// Returns 1 and fills out, or 0 when the entry has no image (a pure category) or the key
// would not fit.
static int sidecarKey(const char *img, const char *param1, const char *param2, char *out, int max)
{
  if (img == 0 || *img == 0) return 0;
  const char *base = img;
  for (const char *p = img; *p != 0; p++)
    if (*p == '/') base = p + 1;
  int len = (int) strlen(base);
  if (len > 4 && strcmp(base + len - 4, ".img") == 0) len -= 4;
  if (len <= 0 || len >= max) return 0;
  int n = 0;
  for (int i = 0; i < len; i++)
  {
    char c = base[i];
    if (c >= 'A' && c <= 'Z') c = (char) (c + 32);
    out[n++] = c;
  }
  out[n] = 0;
  const char *params[2] = { param1, param2 };
  for (int k = 0; k < 2; k++)
  {
    if (params[k] == 0 || *params[k] == 0) continue;
    char slug[64];
    sidecarSlug(slug, (int) sizeof slug, params[k]);
    if (slug[0] == 0) continue;
    int sl = (int) strlen(slug);
    if (n + 1 + sl >= max) return 0;
    out[n++] = '.';
    memcpy(out + n, slug, (size_t) sl);
    n += sl;
    out[n] = 0;
  }
  return 1;
}

// Handles one name=value line of a sidecar file. Unknown names are ignored on purpose,
// so files may carry human-readable fields (title, image, ...).
// A value wrapped in double quotes keeps its inner spaces.
static void sidecarLine(SidecarText *t, const char *name, const char *value)
{
  if (strcmp(name, "skip") == 0)
  {
    int v = atoi(value);
    if (v < 0) v = 0;
    if (v > SC_MAX_ROWS) v = SC_MAX_ROWS;
    t->skip = v;
    t->found = 1;
  }
  else if (strcmp(name, "text") == 0)
  {
    if (t->rows >= SC_MAX_ROWS) return;
    int len = (int) strlen(value);
    if (len >= 2 && value[0] == '"' && value[len - 1] == '"')
    {
      value++;
      len -= 2;
    }
    if (len > SC_ROW_LEN - 1) len = SC_ROW_LEN - 1;
    memcpy(t->row[t->rows], value, (size_t) len);
    t->row[t->rows][len] = 0;
    t->rows++;
    t->found = 1;
  }
}

#endif
