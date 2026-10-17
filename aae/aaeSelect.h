// Choosing which AAE game to run: from a number (the old way), from a name, or from a
// list of the ROM zips found on the card. No hardware dependencies, so it can be unit
// tested on a PC (docs/tools/test_aae_select.c). The on-screen picker is in aaemain.c.
//
// Format of the parameter passed by the loader (getLoadParameter(0)):
//   ""            -> AAE_PICK: show the picker
//   exactly 1 byte-> the game number (index into driver[]), as before
//   anything else -> the game's MAME name, e.g. "mhavoc" (case does not matter)
#ifndef AAE_SELECT_H
#define AAE_SELECT_H

#include <string.h>

#define AAE_PICK (-2)       // show the picker
#define AAE_BAD (-1)        // unknown game / nothing chosen
#define AAE_MAX_GAMES 128   // upper bound for driver[] entries and for listed games

static char aaeLower(char c)
{
  return (c >= 'A' && c <= 'Z') ? (char) (c + 32) : c;
}

// ASCII case-insensitive equality
static int aaeNameEq(const char *a, const char *b)
{
  while (*a != 0 && *b != 0)
  {
    if (aaeLower(*a) != aaeLower(*b)) return 0;
    a++;
    b++;
  }
  return *a == 0 && *b == 0;
}

// Maps the loader parameter to a driver index, AAE_PICK or AAE_BAD.
static int aaeParamToIndex(const char *param, const char *const *names, int count)
{
  if (param == 0 || param[0] == 0) return AAE_PICK;
  if (param[1] == 0)                           // one byte: the old numeric form
  {
    int n = (unsigned char) param[0];
    return n < count ? n : AAE_BAD;
  }
  for (int i = 0; i < count; i++)
    if (aaeNameEq(param, names[i])) return i;
  return AAE_BAD;
}

// "MHavoc.ZIP" -> "mhavoc". Returns 0 for anything that is not a .zip name.
static int aaeZipBase(const char *file, char *out, int max)
{
  int len = (int) strlen(file);
  if (len <= 4 || file[0] == '.') return 0;
  if (!aaeNameEq(file + len - 4, ".zip")) return 0;
  len -= 4;
  if (len >= max) return 0;
  for (int i = 0; i < len; i++) out[i] = aaeLower(file[i]);
  out[len] = 0;
  return 1;
}

static const char *aaeTrim(const char *s)
{
  while (*s == ' ') s++;
  return s;
}

// Compares two descriptions: case-insensitive, leading spaces ignored.
static int aaeDescCmp(const char *a, const char *b)
{
  a = aaeTrim(a);
  b = aaeTrim(b);
  while (*a != 0 && *b != 0 && aaeLower(*a) == aaeLower(*b))
  {
    a++;
    b++;
  }
  return (unsigned char) aaeLower(*a) - (unsigned char) aaeLower(*b);
}

// Given the file names found in the ROM folder, fills out[] with the driver indexes
// that have a zip there: each driver once, ordered by description. Returns how many.
static int aaeCollect(const char *const *files, int nfiles,
                      const char *const *names, const char *const *descs, int count,
                      int *out, int maxOut)
{
  int n = 0;
  for (int f = 0; f < nfiles; f++)
  {
    char base[48];
    if (!aaeZipBase(files[f], base, sizeof base)) continue;
    for (int i = 0; i < count; i++)
    {
      if (!aaeNameEq(base, names[i])) continue;
      int dup = 0;
      for (int k = 0; k < n; k++) if (out[k] == i) dup = 1;
      if (!dup && n < maxOut) out[n++] = i;
      break;
    }
  }
  for (int a = 1; a < n; a++)                  // insertion sort by description
  {
    int v = out[a];
    int b = a - 1;
    while (b >= 0 && aaeDescCmp(descs[out[b]], descs[v]) > 0)
    {
      out[b + 1] = out[b];
      b--;
    }
    out[b + 1] = v;
  }
  return n;
}

#endif
