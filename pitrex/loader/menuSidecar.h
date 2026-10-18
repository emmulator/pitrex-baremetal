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
  int hide;                        // 1: leave the entry out of the new menu
  char title[32];                  // display name of a program found on the card (new menu)
  char emulator;                   // 's' or 'e': which Vectrex emulator starts a ROM (new menu)
  char image[40];                  // entry files: the image to start (without the piZero1/ folder)
  char category[56];               // entry files: where it is listed, "ARCADE / ATARI"
  char p1[40];                     // entry files: parameter 1 ("0x2d" is one byte)
  char p2[24];                     // entry files: parameter 2
  char needs[56];                  // entry files: a file that must exist (roms/aae/mhavoc.zip)
  int nctl;                        // number of control lines ("control = ..."), shown by Button 2 (new menu)
  char ctl[SC_MAX_ROWS][SC_ROW_LEN];
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

// "0x2d" is the single byte 0x2d (the AAE game number); anything else is taken as it is.
static void sidecarParam(char *out, int max, const char *value)
{
  int len = (int) strlen(value);
  if (len >= 4 && (len % 2) == 0 && value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
  {
    int n = 0;
    int ok = 1;
    for (int i = 2; i + 1 < len && n < max - 1; i += 2)
    {
      int v = 0;
      for (int k = 0; k < 2; k++)
      {
        char c = value[i + k];
        int d = (c >= '0' && c <= '9') ? c - '0' : (c >= 'a' && c <= 'f') ? c - 'a' + 10 : (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;
        if (d < 0) ok = 0;
        v = v * 16 + (d < 0 ? 0 : d);
      }
      out[n++] = (char) v;
    }
    if (ok && n > 0) { out[n] = 0; return; }
  }
  int l = len > max - 1 ? max - 1 : len;
  memcpy(out, value, (size_t) l);
  out[l] = 0;
}

static void sidecarCopy(char *out, int max, const char *value)
{
  int l = (int) strlen(value);
  if (l > max - 1) l = max - 1;
  memcpy(out, value, (size_t) l);
  out[l] = 0;
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
  else if (strcmp(name, "hide") == 0)
  {
    t->hide = atoi(value) != 0;
  }
  else if (strcmp(name, "emulator") == 0)
  {
    char c = value[0];
    if (c >= 'A' && c <= 'Z') c = (char) (c + 32);
    t->emulator = (c == 's' || c == 'e') ? c : 0;
  }
  else if (strcmp(name, "image") == 0) sidecarCopy(t->image, (int) sizeof t->image, value);
  else if (strcmp(name, "category") == 0) sidecarCopy(t->category, (int) sizeof t->category, value);
  else if (strcmp(name, "param1") == 0) sidecarParam(t->p1, (int) sizeof t->p1, value);
  else if (strcmp(name, "param2") == 0) sidecarParam(t->p2, (int) sizeof t->p2, value);
  else if (strcmp(name, "needs") == 0) sidecarCopy(t->needs, (int) sizeof t->needs, value);
  else if (strcmp(name, "title") == 0)
  {
    int len = (int) strlen(value);
    if (len > (int) sizeof(t->title) - 1) len = (int) sizeof(t->title) - 1;
    memcpy(t->title, value, (size_t) len);
    t->title[len] = 0;
  }
  else if (strcmp(name, "control") == 0)
  {
    if (t->nctl >= SC_MAX_ROWS) return;
    int len = (int) strlen(value);
    if (len >= 2 && value[0] == '"' && value[len - 1] == '"')
    {
      value++;
      len -= 2;
    }
    if (len > SC_ROW_LEN - 1) len = SC_ROW_LEN - 1;
    memcpy(t->ctl[t->nctl], value, (size_t) len);
    t->ctl[t->nctl][len] = 0;
    t->nctl++;
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
